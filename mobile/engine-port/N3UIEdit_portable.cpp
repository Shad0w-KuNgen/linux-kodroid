// N3UIEdit_portable.cpp — CN3UIEdit'in Win32 EDIT kontrolü yerine SDL metin girişiyle çalışması.
//
// Windows'ta oyun, görünmez bir EDIT penceresine odak verir ve metni EN_CHANGE'te geri okur.
// Burada aynı rol bir metin tamponuyla oynanır: platform katmanı SDL_TEXTINPUT/SDL_KEYDOWN
// olaylarını InputText/InputKey'e iletir; tampon odaklı CN3UIEdit'e yazılır.
#if !defined(_WIN32)

#include <N3Base/KoText.h>
#include <N3Base/StdAfxBase.h>
#include <N3Base/N3UIEdit.h>
#include <N3Base/N3UIString.h>

#include <algorithm>
#include <string>

#if __has_include(<iconv.h>) && !defined(__ANDROID__)
#include <iconv.h>
#define KO_HAVE_ICONV 1
#endif

namespace
{
std::string g_text;   // odaklı edit'in metni (oyun kodlaması: ASCII / CP949)
size_t g_caret = 0;   // bayt konumu

// Dizede iPos'taki baytın bir çift baytlı karakterin ikinci baytı olup olmadığı
bool IsTrailByte(const std::string& s, size_t pos)
{
	if (pos == 0 || pos >= s.size() || !((unsigned char) s[pos] & 0x80) || KoTextCodePage() == 1254)
		return false;
	bool middle = false;
	for (size_t i = 0; i < pos; ++i)
		if ((unsigned char) s[i] & 0x80)
			middle = !middle;
	return middle;
}

size_t PrevCharStart(const std::string& s, size_t pos)
{
	if (pos == 0)
		return 0;
	size_t p = pos - 1;
	if (IsTrailByte(s, p) && p > 0)
		--p;
	return p;
}

size_t NextCharEnd(const std::string& s, size_t pos)
{
	if (pos >= s.size())
		return s.size();
	size_t p = pos + 1;
	if (p < s.size() && IsTrailByte(s, p))
		++p;
	return p;
}

// UTF-8 → oyun kodlaması (ASCII doğrudan; diğerleri CP949'a iconv ile, yoksa atlanır)
std::string Utf8ToGame(const char* utf8)
{
	std::string in = utf8 ? utf8 : "";
	std::string out;
	bool ascii = std::all_of(in.begin(), in.end(), [](char c) { return !((unsigned char) c & 0x80); });
	if (ascii)
		return in;
	if (KoTextCodePage() == 1254)
		return KoTextUtf8To1254(in); // Türkçe klavye → Windows-1254
#if KO_HAVE_ICONV
	static iconv_t cd = iconv_open("CP949//IGNORE", "UTF-8");
	if (cd != (iconv_t) -1)
	{
		std::string buf(in.size() * 2 + 8, '\0');
		char* pin   = in.data();
		char* pout  = buf.data();
		size_t inl  = in.size(), outl = buf.size();
		iconv(cd, nullptr, nullptr, nullptr, nullptr);
		iconv(cd, &pin, &inl, &pout, &outl);
		buf.resize(buf.size() - outl);
		return buf;
	}
#endif
	for (char c : in)
		if (!((unsigned char) c & 0x80))
			out.push_back(c);
	return out;
}

void Commit()
{
	CN3UIEdit* edit = CN3UIBase::GetFocusedEdit();
	if (!edit)
		return;
	edit->SetString(g_text);
	// SetString azami uzunluğa göre kırpabilir; tamponu güncel metinle eşitle
	g_text  = edit->GetString();
	g_caret = std::min(g_caret, g_text.size());
	edit->SetCaretPos(g_caret);
}
} // namespace

BOOL CN3UIEdit::CreateEditWindow(HWND, RECT)
{
	return TRUE; // pencere yok; metin girişi platform katmanından gelir
}

LRESULT APIENTRY CN3UIEdit::EditWndProc(HWND, uint16_t, WPARAM, LPARAM)
{
	return 0;
}

void CN3UIEdit::UpdateTextFromEditCtrl()
{
	Commit();
}

void CN3UIEdit::UpdateCaretPosFromEditCtrl()
{
	CN3UIEdit* edit = CN3UIBase::GetFocusedEdit();
	if (!edit)
		return;
	g_caret = std::min(g_caret, g_text.size());
	edit->SetCaretPos(g_caret);
}

void CN3UIEdit::SetImeStatus(POINT, bool)
{
}

void CN3UIEdit::OnFocusGained(CN3UIEdit* pEdit)
{
	g_text  = pEdit ? pEdit->GetString() : std::string();
	g_caret = g_text.size();
}

void CN3UIEdit::OnFocusLost(CN3UIEdit*)
{
	g_text.clear();
	g_caret = 0;
}

bool CN3UIEdit::WantsTextInput()
{
	return CN3UIBase::GetFocusedEdit() != nullptr;
}

void CN3UIEdit::InputText(const char* utf8)
{
	if (!CN3UIBase::GetFocusedEdit())
		return;
	std::string ins = Utf8ToGame(utf8);
	if (ins.empty())
		return;
	g_caret = std::min(g_caret, g_text.size());
	g_text.insert(g_caret, ins);
	g_caret += ins.size();
	Commit();
}

void CN3UIEdit::InputKey(e_EditKey key)
{
	CN3UIEdit* edit = CN3UIBase::GetFocusedEdit();
	if (!edit)
		return;
	g_caret = std::min(g_caret, g_text.size());
	switch (key)
	{
		case EDITKEY_BACKSPACE:
			if (g_caret > 0)
			{
				size_t start = PrevCharStart(g_text, g_caret);
				g_text.erase(start, g_caret - start);
				g_caret = start;
				Commit();
			}
			break;
		case EDITKEY_DELETE:
			if (g_caret < g_text.size())
			{
				g_text.erase(g_caret, NextCharEnd(g_text, g_caret) - g_caret);
				Commit();
			}
			break;
		case EDITKEY_LEFT:
			g_caret = PrevCharStart(g_text, g_caret);
			edit->SetCaretPos(g_caret);
			break;
		case EDITKEY_RIGHT:
			g_caret = NextCharEnd(g_text, g_caret);
			edit->SetCaretPos(g_caret);
			break;
		case EDITKEY_HOME:
			g_caret = 0;
			edit->SetCaretPos(g_caret);
			break;
		case EDITKEY_END:
			g_caret = g_text.size();
			edit->SetCaretPos(g_caret);
			break;
		case EDITKEY_RETURN:
			if (edit->GetParent() != nullptr)
				edit->GetParent()->ReceiveMessage(edit, UIMSG_EDIT_RETURN);
			break;
		default:
			break;
	}
}

#endif // !_WIN32
