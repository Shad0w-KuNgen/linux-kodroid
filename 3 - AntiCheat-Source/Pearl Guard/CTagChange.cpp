#include "CTagChange.h"


CTagChange::CTagChange()
{
	vector<int>offsets;
	offsets.push_back(0x2F8);  // co_CastleUnion Satýr : 82
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);
	btn_cancel = NULL;
	btn_ok = NULL;
	edit_id = NULL;
	plate = NULL;
	r = 255;
	g = 255;
	b = 255;
	ParseUIElements();
	InitReceiveMessage();
}

DWORD Func_tag;
DWORD uiTag;
CTagChange::~CTagChange()
{

}

void CTagChange::ParseUIElements()
{
	txt_id = Engine->GetChildByID(m_dVTableAddr, "edit_id");
	btn_cancel = Engine->GetChildByID(m_dVTableAddr, "btn_cancel");
	btn_ok = Engine->GetChildByID(m_dVTableAddr, "btn_ok");
	text_title = Engine->GetChildByID(m_dVTableAddr, "Text_title");
	plate = Engine->GetChildByID(m_dVTableAddr, "plate");
	text_explanation = Engine->GetChildByID(m_dVTableAddr, "text_explanation");

	if (text_explanation) Engine->SetString(text_explanation,"Please enter the new your Tag ID.");
	
	Engine->UIScreenCenter(m_dVTableAddr);
}


void __stdcall UITagChangeReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiTagChange->ReceiveMessage(pSender, dwMsg);
	__asm
	{
		MOV ECX, uiTag
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_tag
		CALL EAX
	}
}
void CTagChange::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_tag = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)UITagChangeReceiveMessage_Hook;
}
bool CTagChange::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{

	uiTag = m_dVTableAddr;
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == (DWORD*)btn_ok) {
			if (!txt_id) return true;
			std::string id = Engine->GetEditString(txt_id);
			if (id.empty()) {
				Engine->WriteInfoMessageExt((char*)string_format(xorstr("Tag Change : Tag is empty or incorret word!")).c_str(), 0xa30000);
				return true;
			}
			if (id.size() > 40) {
				Engine->WriteInfoMessageExt((char*)string_format(xorstr("Tag Change : tag size soo long!")).c_str(), 0xa30000);
				return true;
			}

			Packet tag(XSafe, (uint8)XSafeOpCodes::TagInfo);
			tag << (uint8)tagerror::newtag << Engine->GetEditString(txt_id) << r << g << b;
			Engine->Send(&tag);
			Engine->SetVisible(m_dVTableAddr,false);
			Engine->EditKillFocus(txt_id);
			if (txt_id) Engine->SetEditString(txt_id, "");
			//Close();
		}
		else if (pSender == (DWORD*)btn_cancel) {
			if (txt_id) Engine->SetEditString(txt_id, "");
			Engine->EditKillFocus(txt_id);
			Engine->SetVisible(m_dVTableAddr, false);
			//Close();
		}
	}
	return true;
}

extern HWND window;

uint32_t CTagChange::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;

	if ((dwFlags & 0x4) && window && Engine->IsIn(plate,ptCur.x, ptCur.y))
	{
		HDC dc = GetDC(window);
		COLORREF color = GetPixel(dc, ptCur.x, ptCur.y);
		ReleaseDC(window, dc);
		r = GetRValue(color);
		g = GetGValue(color);
		b = GetBValue(color);
		Engine->SetStringColor(text_title,D3DCOLOR_ARGB(255, r, g, b));
	}


	return dwRet;
}


void CTagChange::Open()
{
	r = 255; g = 255; b = 255;
	Engine->SetVisible(m_dVTableAddr,true);
}

void CTagChange::Close()
{
	Engine->EditKillFocus(txt_id);
	Engine->SetVisible(m_dVTableAddr, false);
}

void CTagChange::OpenTagChange()
{
	Engine->UIScreenCenter(m_dVTableAddr);
	Engine->SetVisible(m_dVTableAddr,true);

}