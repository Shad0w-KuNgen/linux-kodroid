// KoTouchOverlay.cpp — dokunmatik kontrol kaplaması (bkz. KoTouchOverlay.h).
#include "StdAfx.h"
#include "KoTouchOverlay.h"
#include "UIManager.h"
#include <cstdio>
#include <string>
#include "KoPlatformInput.h"

#include "GameProcedure.h"
#include "GameProcMain.h"
#include "GameProcLogIn_1298.h"
#include "GameProcCharacterCreate.h"
#include "GameProcCharacterSelect.h"
#include "UICharacterCreate.h"
#include "PlayerMySelf.h"
#include <N3Base/LogWriter.h>
#include "GameEng.h"
#include "GameDef.h"
#include "UIHotKeyDlg.h"
#include "MagicSkillMng.h"
#include <N3Base/N3Texture.h>

#include <N3Base/N3Base.h>
#include <N3Base/N3UIBase.h>
#include <N3Base/DFont.h>

#include <d3d9.h>
#include <dinput.h>

#include <algorithm>
#include <cmath>

namespace
{
constexpr int TAP_SLOP           = 14;         // piksel: bundan az kayarsa "tık"
constexpr uint32_t DOUBLE_TAP_MS = 450;        // arayüz ikonunda çift dokunuş = sağ tık
constexpr uint32_t COL_JOY_BASE  = 0x38FFFFFF;
constexpr uint32_t COL_JOY_RING  = 0x90E8D8A0;
constexpr uint32_t COL_KNOB      = 0xA0FFFFFF;
constexpr uint32_t COL_SLOT      = 0x50101010;
constexpr uint32_t COL_SLOT_RING = 0xC0D9B25C; // altın
constexpr uint32_t COL_ATTACK    = 0x90B02020;
constexpr uint32_t COL_ATTACK_RING = 0xE0FFD070;
constexpr uint32_t COL_TARGET    = 0x90204080;
constexpr uint32_t COL_CAM       = 0x80303030;
constexpr uint32_t COL_CAM_RING  = 0xC0E0C070;
constexpr uint32_t COL_BAR       = 0xA0241C14;
constexpr uint32_t COL_BAR_BTN   = 0xC0463828;
constexpr uint32_t COL_DOWN      = 0xC0FFC040;
constexpr uint32_t COL_TEXT      = 0xFFF4E6B8;
constexpr uint32_t COL_PAGE_ON   = 0xD0B07A1C; // seçili beceri sayfası (F1..F8)

struct RhwVertex
{
	float x, y, z, rhw;
	D3DCOLOR color;
};
} // namespace

KoTouchOverlay& KoTouch()
{
	static KoTouchOverlay o;
	return o;
}

KoTouchOverlay::KoTouchOverlay() = default;

KoTouchOverlay::~KoTouchOverlay()
{
	for (CDFont* f : m_fonts)
		delete f;
}

bool KoTouchOverlay::IsInGame() const
{
	return CGameProcedure::s_pProcActive != nullptr && CGameProcedure::s_pProcActive == CGameProcedure::s_pProcMain
		   && CGameProcedure::s_pPlayer != nullptr;
}

bool KoTouchOverlay::IsServerSelect() const
{
	return CGameProcedure::s_pProcLogIn != nullptr && CGameProcedure::s_pProcActive == CGameProcedure::s_pProcLogIn
		   && CGameProcedure::s_pProcLogIn->IsServerListOpen();
}

bool KoTouchOverlay::IsCharacterCreate() const
{
	return CGameProcedure::s_pProcCharacterCreate != nullptr
		   && CGameProcedure::s_pProcActive == (CGameProcedure*) CGameProcedure::s_pProcCharacterCreate
		   && CGameProcedure::s_pProcCharacterCreate->m_pUICharacterCreate != nullptr;
}

bool KoTouchOverlay::IsCharacterSelect() const
{
	return CGameProcedure::s_pProcCharacterSelect != nullptr
		   && CGameProcedure::s_pProcActive == (CGameProcedure*) CGameProcedure::s_pProcCharacterSelect;
}

// Yazı tipi dizinleri sabit: DrawLabel metni ilk çizimde önbelleğe alır, her etiketin kendi dizini olmalı
namespace
{
constexpr int PRE_FONT_BASE = 100;
enum PreLabel { PL_EL_BA = 0, PL_EL_MAN, PL_EL_WOMAN, PL_KA_AT, PL_KA_TU, PL_KA_WT, PL_KA_PT, PL_WARRIOR, PL_ROGUE, PL_MAGE,
	PL_PRIEST, PL_AUTO, PL_CREATE, PL_CANCEL, PL_START, PL_NEW, PL_COUNT };
const char* PRE_TEXT[PL_COUNT] = { "Barbar", "Erkek", "Kadin", "Arktuarek", "Tuarek", "Kirisik Tuarek", "Puri Tuarek", "Savasci", "Hirsiz",
	"Buyucu", "Rahip", "OTO PUAN", "OLUSTUR", "GERI", "BASLA", "YENI KARAKTER" };
} // namespace

void KoTouchOverlay::LayoutPreButtons()
{
	m_preButtons.clear();
	float u        = m_u;
	auto add       = [&](PreAction a, int arg, int lbl, float x, float y, float w, float h, bool sel) {
        m_preButtons.push_back({ a, arg, PRE_TEXT[lbl], PRE_FONT_BASE + lbl, x, y, w, h, sel });
	};
	if (IsCharacterCreate())
	{
		CUICharacterCreate* pUI = CGameProcedure::s_pProcCharacterCreate->m_pUICharacterCreate;
		e_Nation eNation        = CGameProcedure::s_pPlayer ? CGameProcedure::s_pPlayer->m_InfoBase.eNation : NATION_ELMORAD;
		int iRace               = pUI->SelectedRaceIndex();
		int iClass              = pUI->SelectedClassIndex();
		float bw = 190.0f * u, bh = 56.0f * u, gap = 12.0f * u;
		// Sol sütun: ırklar
		int raceLbls[4];
		int nRaces = 0;
		if (eNation == NATION_KARUS)
		{
			raceLbls[0] = PL_KA_AT; raceLbls[1] = PL_KA_TU; raceLbls[2] = PL_KA_WT; raceLbls[3] = PL_KA_PT; nRaces = 4;
		}
		else
		{
			raceLbls[0] = PL_EL_BA; raceLbls[1] = PL_EL_MAN; raceLbls[2] = PL_EL_WOMAN; nRaces = 3;
		}
		float y0 = 120.0f * u;
		for (int i = 0; i < nRaces; i++)
			add(PreAction::Race, i, raceLbls[i], 24.0f * u, y0 + i * (bh + gap), bw, bh, iRace == i);
		// Sağ sütun: sınıflar
		int classLbls[4] = { PL_WARRIOR, PL_ROGUE, PL_MAGE, PL_PRIEST };
		for (int i = 0; i < 4; i++)
			add(PreAction::Class, i, classLbls[i], m_w - bw - 24.0f * u, y0 + i * (bh + gap), bw, bh, iClass == i);
		// Alt satır: oto puan, oluştur, geri
		float by = m_h - bh - 20.0f * u;
		add(PreAction::AutoBonus, 0, PL_AUTO, 24.0f * u, by, bw, bh, false);
		add(PreAction::Create, 0, PL_CREATE, m_w / 2.0f - bw - gap / 2, by, bw, bh, false);
		add(PreAction::Cancel, 0, PL_CANCEL, m_w / 2.0f + gap / 2, by, bw, bh, false);
	}
	else if (IsCharacterSelect())
	{
		float bw = 260.0f * u, bh = 64.0f * u, gap = 16.0f * u;
		float by = m_h - bh - 20.0f * u;
		add(PreAction::SelStart, 0, PL_START, m_w / 2.0f - bw - gap / 2, by, bw, bh, false);
		add(PreAction::SelNew, 0, PL_NEW, m_w / 2.0f + gap / 2, by, bw, bh, false);
	}
}

void KoTouchOverlay::DoPreAction(const PreButton& b)
{
	CUIManager* mgr = CGameProcedure::s_pUIMgr;
	if (mgr != nullptr && !mgr->EnableOperation())
	{
		CLogWriter::Write("Dokunmatik: '{}' yok sayıldı (sunucu yanıtı bekleniyor)", b.label);
		return;
	}
	if (IsCharacterCreate())
	{
		CUICharacterCreate* pUI = CGameProcedure::s_pProcCharacterCreate->m_pUICharacterCreate;
		switch (b.action)
		{
			case PreAction::Race: pUI->SelectRaceByIndex(b.arg); break;
			case PreAction::Class:
				if (!pUI->SelectClassByIndex(b.arg))
					CLogWriter::Write("Dokunmatik: önce ırk seçilmeli");
				break;
			case PreAction::AutoBonus: pUI->AutoAssignBonus(); break;
			case PreAction::Create:
				CLogWriter::Write("Dokunmatik OLUSTUR");
				pUI->RequestCreate();
				break;
			case PreAction::Cancel: pUI->Cancel(); break;
			default: break;
		}
	}
	else if (IsCharacterSelect())
	{
		CGameProcCharacterSelect* pSel = CGameProcedure::s_pProcCharacterSelect;
		int iSlot = -1;
		for (int i = 0; i < MAX_AVAILABLE_CHARACTER; i++)
		{
			bool bHas = pSel->m_pChrs[i] != nullptr;
			if ((b.action == PreAction::SelStart && bHas) || (b.action == PreAction::SelNew && !bHas))
			{
				iSlot = i;
				break;
			}
		}
		if (iSlot < 0)
		{
			CLogWriter::Write("Dokunmatik {}: uygun yuva yok", b.label);
			return;
		}
		CGameProcedure::s_iChrSelectIndex = iSlot;
		CLogWriter::Write("Dokunmatik {}: yuva {} ({})", b.label, iSlot, pSel->m_InfoChrs[iSlot].szID);
		pSel->CharacterSelectOrCreate();
	}
}

void KoTouchOverlay::Layout(int w, int h)
{
	m_w = w;
	m_h = h;
	float u = (float) h / 768.0f;
	m_u     = u;

	// Sunucu seçme ekranı: alt ortada büyük BAĞLAN düğmesi (dokunmatik "Enter")
	m_connW  = std::max(300.0f * u, m_tuning.minTouchPx * 5.0f);
	m_connH  = std::max(72.0f * u, m_tuning.minTouchPx * 1.4f);
	m_connCx = w / 2.0f;
	m_connCy = h - m_connH / 2.0f - 24.0f * u;

	// Tüm aralıklar tek bir "dokunma birimi"ne göre: en az ~48dp (DPI'dan) ya da 56 mantıksal px.
	// Düğmeler bu birime göre yerleşir, böylece büyütme üst üste binmeye yol açmaz.
	const float t   = std::max(56.0f * u, m_tuning.minTouchPx); // bir düğmenin kapladığı kare
	const float gap = std::max(8.0f * u, t * 0.14f);
	const float r   = t * 0.46f;                                 // yuvarlak düğme yarıçapı

	// Joystick: sol alt
	m_joyR     = std::max(80.0f * u, t * 1.3f);
	m_joyHomeX = m_joyR + 60.0f * u;
	m_joyHomeY = h - m_joyR - 110.0f * u;
	m_joyCx = m_joyHomeX;
	m_joyCy = m_joyHomeY;
	m_knobX = m_joyCx;
	m_knobY = m_joyCy;
	m_joyFinger = -1;

	m_buttons.clear();
	auto circle = [&](Action a, int dik, const char* label, float cx, float cy, float rr, uint32_t col) {
		Button b;
		b.action = a; b.dik = dik; b.label = label; b.cx = cx; b.cy = cy; b.r = rr; b.color = col;
		m_buttons.push_back(b);
	};
	auto ring = [&](Action a, int dik, const std::string& label, float cx, float cy, float rr, uint32_t col) {
		Button b;
		b.action = a; b.shape = Shape::Ring; b.dik = dik; b.label = label; b.cx = cx; b.cy = cy; b.r = rr; b.color = col;
		m_buttons.push_back(b);
	};
	auto rect = [&](int dik, const char* label, float cx, float cy, float bw, float bh) {
		Button b;
		b.shape = Shape::Rect; b.dik = dik; b.label = label; b.cx = cx; b.cy = cy; b.w = bw; b.h = bh; b.color = COL_BAR_BTN;
		m_buttons.push_back(b);
	};

	// --- Alt çubuk (en altta): CANTA ... KAPAT; yüksekliği dokunma birimine göre ---
	const float barH = std::max(40.0f * u, t * 0.7f);
	m_barH           = barH;
	{
		const struct { int dik; const char* label; } bar[] = {
			{KM_TOGGLE_INVENTORY, "CANTA"}, {KM_TOGGLE_STATE, "KARAKTER"}, {KM_TOGGLE_SKILL, "BECERI"},
			{KM_TOGGLE_SITDOWN, "OTUR"}, {KM_TOGGLE_MINIMAP, "HARITA"}, {KM_DROPPED_ITEM_OPEN, "AL"},
			{DIK_RETURN, "SOHBET"}, {KM_TOGGLE_PUS, "PUS"}, {KM_TOGGLE_CMDLIST, "MENU"}, {KM_TOGGLE_HELP, "YARDIM"}, {DIK_ESCAPE, "KAPAT"}};
		int n = (int) (sizeof(bar) / sizeof(bar[0]));
		float bgap = 6.0f * u;
		float bw   = std::min(110.0f * u, (w - 16.0f * u - (n - 1) * bgap) / n);
		float total = n * bw + (n - 1) * bgap;
		float x0 = (w - total) / 2.0f + bw / 2.0f, y0 = h - barH / 2.0f;
		for (int i = 0; i < n; ++i)
			rect(bar[i].dik, bar[i].label, x0 + i * (bw + bgap), y0, bw, barH - 8.0f * u);
	}

	// --- Sağ alt küme (çubuğun üstünde), sağdan sola: SALDIR | beceri ızgarası 2x4 | HP/MP ---
	const float bottom = h - barH - gap;          // kümenin alt kenarı
	const float atkR   = std::max(46.0f * u, t * 0.62f);
	const float atkCx  = w - gap - atkR, atkCy = bottom - atkR;
	circle(Action::Key, KM_TOGGLE_ATTACK, "SALDIR", atkCx, atkCy, atkR, COL_ATTACK);
	// Üst orta: kaplamayı gizle/göster (gizliyken yalnız bu düğme kalır)
	circle(Action::ToggleHide, 0, m_hidden ? "GOSTER" : "GIZLE", w / 2.0f, 16.0f * u + r * 0.7f, r * 0.7f, COL_CAM);

	// Beceri ızgarası: üst sıra 1-4, alt sıra 5-8; sağ kenarı SALDIR'ın solunda
	const int hotkeys[8] = {KM_HOTKEY1, KM_HOTKEY2, KM_HOTKEY3, KM_HOTKEY4, KM_HOTKEY5, KM_HOTKEY6, KM_HOTKEY7, KM_HOTKEY8};
	const char* names[8] = {"1", "2", "3", "4", "5", "6", "7", "8"};
	const float gridRight = atkCx - atkR - gap * 1.5f;
	const float rowY1     = bottom - t / 2.0f;            // alt sıra (5-8)
	const float rowY0     = rowY1 - t - gap;              // üst sıra (1-4)
	for (int i = 0; i < 4; ++i)
	{
		float cx = gridRight - (3 - i) * (t + gap) - t / 2.0f;
		ring(Action::Key, hotkeys[i], names[i], cx, rowY0, r, COL_SLOT);
		m_buttons.back().hotkeySlot = i;
		ring(Action::Key, hotkeys[4 + i], names[4 + i], cx, rowY1, r, COL_SLOT);
		m_buttons.back().hotkeySlot = 4 + i;
	}
	const float gridLeft = gridRight - 4 * (t + gap) + gap;

	// Beceri sayfaları F1..F8: ızgaranın üstünde sekiz ayrı düğme (oyundaki F1-F8 gibi), seçili sayfa vurgulu
	{
		const float gridW = 4 * t + 3 * gap;
		const float pgap  = 4.0f * u;
		const float pw    = (gridW - 7 * pgap) / 8.0f, ph = std::max(24.0f * u, t * 0.5f);
		const float py    = rowY0 - t / 2.0f - gap - ph / 2.0f;
		m_clusterTop      = py - ph / 2.0f;
		for (int i = 0; i < 8; ++i)
		{
			Button b;
			b.action = Action::SkillPage; b.shape = Shape::Rect; b.dik = KM_SKILL_PAGE_1 + i;
			b.label = "F" + std::to_string(i + 1);
			b.cx = gridLeft + pw / 2.0f + i * (pw + pgap); b.cy = py; b.w = pw; b.h = ph; b.color = COL_BAR_BTN;
			m_buttons.push_back(b);
		}
	}

	// HP / MP: ızgaranın solunda, iki sıraya hizalı
	const int slotKeys[8] = {KM_HOTKEY1, KM_HOTKEY2, KM_HOTKEY3, KM_HOTKEY4, KM_HOTKEY5, KM_HOTKEY6, KM_HOTKEY7, KM_HOTKEY8};
	int hp = std::clamp(m_tuning.hpSlot, 1, 8) - 1, mp = std::clamp(m_tuning.mpSlot, 1, 8) - 1;
	const float potX = gridLeft - gap * 1.5f - t / 2.0f;
	circle(Action::Key, slotKeys[hp], "HP", potX, rowY0, r, 0x90A02828);
	circle(Action::Key, slotKeys[mp], "MP", potX, rowY1, r, 0x902848A0);

	// --- Hedef sütunu: SALDIR'ın üstünde, sağ kenara yaslı, aşağıdan yukarı HEDEF, NPC, PARTI, DOST ---
	{
		const float tr = r * 0.8f, step = tr * 2.0f + gap;
		const float cx = w - gap - tr;
		float cy       = atkCy - atkR - gap - tr;
		CGameProcedure::s_iTouchInsetRight = (int) (w - (cx - tr) + gap); // hedef sütunu: oyun pencereleri bunun soluna
		circle(Action::Key, KM_TARGET_NEAREST_ENEMY, "HEDEF", cx, cy, tr, COL_TARGET); cy -= step;
		circle(Action::Key, KM_TARGET_NEAREST_NPC, "NPC", cx, cy, tr, COL_TARGET); cy -= step;
		circle(Action::Key, KM_TARGET_NEAREST_PARTY, "PARTI", cx, cy, tr, COL_TARGET); cy -= step;
		circle(Action::Key, KM_TARGET_NEAREST_FRIEND, "DOST", cx, cy, tr, COL_TARGET);
	}

	// --- Sağ üst: kamera kümesi (KAM, 180, +, -, KOS), sağdan sola ---
	{
		const float cr = r * 0.8f, step = cr * 2.0f + gap;
		const float cy = gap + cr + 40.0f * u; // durum çubuğu/mini harita altı
		float cx       = w - gap - cr;
		CGameProcedure::s_iTouchInsetTop    = (int) (cy + cr + gap); // kamera düğmelerinin altı
		CGameProcedure::s_iTouchInsetBottom = (int) barH;
		circle(Action::Key, KM_CAMERA_CHANGE, "KAM", cx, cy, cr, COL_CAM); cx -= step;
		circle(Action::Yaw180, 0, "180", cx, cy, cr, COL_CAM); cx -= step;
		circle(Action::ZoomIn, 0, "+", cx, cy, cr, COL_CAM); cx -= step;
		circle(Action::ZoomOut, 0, "-", cx, cy, cr, COL_CAM); cx -= step;
		circle(Action::Key, KM_TOGGLE_RUN, "KOS", cx, cy, cr, COL_CAM);
	}

	// --- Sürekli yürüme (E): joystick'in sağ üstünde ---
	circle(Action::Key, KM_TOGGLE_MOVE_CONTINOUS, "OTO", m_joyHomeX + m_joyR + r + gap, m_joyHomeY - m_joyR * 0.6f, r * 0.8f, COL_CAM);

	for (CDFont* f : m_fonts)
		delete f;
	m_fonts.clear();
	m_infoFontText.clear();
}

void KoTouchOverlay::ButtonBox(size_t i, float* x0, float* y0, float* x1, float* y1, std::string* label) const
{
	const Button& b = m_buttons[i];
	if (b.shape == Shape::Rect)
	{
		*x0 = b.cx - b.w / 2; *x1 = b.cx + b.w / 2; *y0 = b.cy - b.h / 2; *y1 = b.cy + b.h / 2;
	}
	else
	{
		*x0 = b.cx - b.r; *x1 = b.cx + b.r; *y0 = b.cy - b.r; *y1 = b.cy + b.r;
	}
	if (label)
		*label = b.label;
}

KoTouchOverlay::Finger* KoTouchOverlay::Find(int64_t id)
{
	for (auto& f : m_fingers)
		if (f.id == id)
			return &f;
	return nullptr;
}

int KoTouchOverlay::HitButton(int x, int y) const
{
	for (size_t i = 0; i < m_buttons.size(); ++i)
	{
		const Button& b = m_buttons[i];
		if (b.shape == Shape::Rect)
		{
			if (std::fabs(x - b.cx) <= b.w / 2 + 4 && std::fabs(y - b.cy) <= b.h / 2 + 6)
				return (int) i;
		}
		else
		{
			float dx = x - b.cx, dy = y - b.cy;
			if (dx * dx + dy * dy <= b.r * b.r * 1.25f)
				return (int) i;
		}
	}
	return -1;
}

bool KoTouchOverlay::InJoystickZone(int x, int y) const
{
	// Sol %45, üst %20'nin altı, alt çubuğun üstü
	return x < m_w * 0.45f && y > m_h * 0.2f && y < m_h - (m_barH > 0 ? m_barH + 4.0f * m_u : 45.0f * m_u);
}

void KoTouchOverlay::OnFingerDown(int64_t id, int x, int y)
{
	KoInputState& in = KoInput();
	Finger f {id, Role::Pending, x, y, x, y, -1, timeGetTime()};
	if (in.stickyDrag)
	{
		// Yapışkan sürükleme: ikonu buraya taşı ve bırak (bir kare LBDOWN, sonra LBCLICKED)
		in.mouseX            = x;
		in.mouseY            = y;
		in.dragHoldFrames    = 0;
		in.pendingLbUpFrames = 2;
		in.stickyDrag        = false;
		f.role               = Role::Done;
		m_fingers.push_back(f);
		return;
	}
	if (IsCharacterCreate() || IsCharacterSelect())
	{
		LayoutPreButtons();
		for (const PreButton& b : m_preButtons)
		{
			if (x >= b.x - 6 && x <= b.x + b.w + 6 && y >= b.y - 6 && y <= b.y + b.h + 6)
			{
				DoPreAction(b);
				f.role = Role::Done;
				m_fingers.push_back(f);
				return;
			}
		}
	}
	if (IsServerSelect() && std::fabs(x - m_connCx) <= m_connW / 2 + 8 && std::fabs(y - m_connCy) <= m_connH / 2 + 8)
	{
		m_connDown = true;
		CGameProcedure::s_pProcLogIn->RequestConnectSelected();
		f.role = Role::Done;
		m_fingers.push_back(f);
		return;
	}
	bool overlayActive = m_enabled && (IsInGame() || m_forceVisible) && CN3UIBase::GetFocusedEdit() == nullptr;
	if (overlayActive && IsInGame() && HitPlayerMenu(x, y))
	{
		f.role = Role::Done;
		m_fingers.push_back(f);
		return;
	}
	if (overlayActive)
	{
		int b = HitButton(x, y);
		if (m_hidden && b >= 0 && m_buttons[b].action != Action::ToggleHide)
			b = -1; // gizliyken yalnız GÖSTER düğmesi
		if (b >= 0 && m_buttons[b].finger < 0)
		{
			f.role        = Role::Button;
			f.buttonIndex = b;
			m_buttons[b].finger    = id;
			m_buttons[b].down      = true;
			m_buttons[b].fired     = false;
			m_buttons[b].tapQueued = m_buttons[b].hotkeySlot < 0; // beceri yuvası: tuş kalkışta (sürükleme ayrımı için)
			m_fingers.push_back(f);
			return;
		}
		if (!m_hidden && m_joyFinger < 0 && InJoystickZone(x, y) && !CGameProcedure::s_bTouchLockMove && !IsOverDialogUI(x, y))
		{
			f.role      = Role::Joystick;
			m_joyFinger = id;
			m_joyCx = m_knobX = (float) x; // yüzen joystick: parmağın bastığı yerde belirir
			m_joyCy = m_knobY = (float) y;
			m_fingers.push_back(f);
			return;
		}
	}
	in.mouseX = x;
	in.mouseY = y;
	m_fingers.push_back(f);
}

void KoTouchOverlay::OnFingerMotion(int64_t id, int x, int y)
{
	Finger* f = Find(id);
	if (!f)
		return;
	f->x = x;
	f->y = y;
	KoInputState& in = KoInput();
	switch (f->role)
	{
		case Role::Button:
			if (f->buttonIndex >= 0 && f->buttonIndex < (int) m_buttons.size() && m_buttons[f->buttonIndex].hotkeySlot >= 0
				&& (std::abs(x - f->startX) > TAP_SLOP || std::abs(y - f->startY) > TAP_SLOP))
				f->slotDrag = true;
			break;
		case Role::Pending:
			if (std::abs(x - f->startX) > TAP_SLOP || std::abs(y - f->startY) > TAP_SLOP)
			{
				// Arayüz penceresi üstünde başlayan sürükleme = sol tuş sürüklemesi (ikon taşıma,
				// pencere taşıma, kaydırma çubuğu); 3D dünyada = kamera (sağ tuş sürüklemesi)
				bool camera = m_enabled && IsInGame() && !IsOverUI(f->startX, f->startY) && !CGameProcedure::s_bTouchLockMove;
				if (camera)
				{
					f->role   = Role::Camera;
					in.rbDown = true;
					in.mouseX = f->startX;
					in.mouseY = f->startY;
				}
				else
					BeginLeftDrag(*f);
			}
			break;
		case Role::Camera:
			// Kamera hassasiyeti: parmağın başlangıca göre kaymasını çarpanla ilet (oyun kare
			// farkını kullanır, mutlak ölçekleme aynı sonucu verir)
			in.mouseX = f->startX + (int) std::lround((x - f->startX) * m_tuning.camSens);
			in.mouseY = f->startY + (int) std::lround((y - f->startY) * m_tuning.camSens);
			break;
		case Role::LeftDrag:
			if (in.dragHoldFrames == 0) // başlangıç kareleri bitmeden imleç kıpırdamaz
			{
				in.mouseX = x;
				in.mouseY = y;
			}
			break;
		default:
			break;
	}
}

void KoTouchOverlay::BeginLeftDrag(Finger& f)
{
	KoInputState& in  = KoInput();
	f.role            = Role::LeftDrag;
	in.mouseX         = f.startX;
	in.mouseY         = f.startY;
	in.lbDown         = true;
	in.dragHoldFrames = 2; // kare 1: LBCLICK ikon üstünde, kare 2: LBDOWN ikon üstünde, sonra takip
}

bool KoTouchOverlay::IsOverUI(int x, int y) const
{
	CUIManager* mgr = CGameProcedure::s_pUIMgr;
	if (!mgr || !mgr->IsVisible())
		return false;
	for (CN3UIBase* child : mgr->GetChildren())
	{
		if (child && child->IsVisible() && child->IsIn(x, y))
			return true;
	}
	return false;
}

void KoTouchOverlay::ReleaseFinger(Finger& f)
{
	KoInputState& in = KoInput();
	switch (f.role)
	{
		case Role::Button:
			if (f.buttonIndex >= 0 && f.buttonIndex < (int) m_buttons.size())
			{
				Button& b = m_buttons[f.buttonIndex];
				b.down    = false;
				b.finger  = -1;
				if (b.hotkeySlot >= 0)
				{
					CUIHotKeyDlg* pHK = (CGameProcedure::s_pProcMain != nullptr) ? CGameProcedure::s_pProcMain->m_pUIHotKeyDlg : nullptr;
					float dx = (float) f.x - b.cx, dy = (float) f.y - b.cy;
					float dist2   = dx * dx + dy * dy;
					uint32_t held = timeGetTime() - f.downTicks;
					bool bFarOut  = dist2 > (b.r * 2.4f) * (b.r * 2.4f);
					if (!f.slotDrag)
						b.tapQueued = true; // dokunuş (kısa kayma dahil) = beceriyi kullan
					else if (pHK != nullptr && held >= 300)
					{
						// Gerçek sürükleme (≥300 ms basılı + parmak hareket etti)
						int iOther = HitHotkeyRing(f.x, f.y);
						if (iOther >= 0 && iOther != b.hotkeySlot)
							pHK->SwapSlots(b.hotkeySlot, iOther); // başka yuvaya bırak = yer değiştir
						else if (bFarOut)
							pHK->ClearSlot(b.hotkeySlot);          // çok dışarı bırak = yuvadan kaldır
					}
				}
			}
			break;
		case Role::Joystick:
			m_joyFinger = -1;
			m_joyCx = m_knobX = m_joyHomeX;
			m_joyCy = m_knobY = m_joyHomeY;
			break;
		case Role::Pending:
		{
			// Arayüz ikonu üstünde çift dokunuş = sağ tık (KO'da eşya kullan / giy, beceri kullan;
			// ICON_RUP aynı yuvada RBCLICK+RBCLICKED ister). 3D dünyada çift dokunuş = oyunun çift
			// tıkı (hedefe saldır) olarak kalır.
			uint32_t now  = timeGetTime();
			bool dblTap   = (now - m_lastTapTicks) <= DOUBLE_TAP_MS && std::abs(f.x - m_lastTapX) <= TAP_SLOP * 2
							&& std::abs(f.y - m_lastTapY) <= TAP_SLOP * 2;
			bool overUI   = IsOverUI(f.x, f.y);
			in.taps.push_back({f.x, f.y, dblTap && overUI}); // CLocalInput::Tick kare kare işler
			m_lastTapTicks = dblTap ? 0 : now; // üçüncü dokunuş yeni dizi başlatsın
			m_lastTapX     = f.x;
			m_lastTapY     = f.y;
			break;
		}
		case Role::Camera:
			in.rbDown   = false;
			m_pinchDist = 0.0f;
			break;
		case Role::LeftDrag:
			if (f.longPressDrag && std::abs(f.x - f.startX) <= TAP_SLOP && std::abs(f.y - f.startY) <= TAP_SLOP)
			{
				// Uzun basışla alınan ikon, parmak kıpırdamadan kalktı: yapışkan sürükleme —
				// sol tuş basılı kalır, sonraki dokunuşta hedefe bırakılır
				in.stickyDrag = true;
			}
			else if (in.dragHoldFrames > 0)
				in.pendingLbUpFrames = in.dragHoldFrames + 1; // başlangıç kareleri işlendikten sonra bırak (yerinde)
			else
			{
				in.mouseX = f.x; // bırakma noktası = parmağın kalktığı yer
				in.mouseY = f.y;
				in.lbDown = false;
			}
			break;
		default:
			break;
	}
}

void KoTouchOverlay::OnFingerUp(int64_t id, int x, int y)
{
	Finger* f = Find(id);
	if (!f)
		return;
	f->x = x;
	f->y = y;
	ReleaseFinger(*f);
	m_fingers.erase(std::remove_if(m_fingers.begin(), m_fingers.end(), [&](const Finger& g) { return g.id == id; }), m_fingers.end());
}

void KoTouchOverlay::Update()
{
	KoInputState& in = KoInput();
	std::memset(in.virtualKeysDIK, 0, sizeof(in.virtualKeysDIK));

	// Uzun basış (parmak kıpırdamadan): sağ tık = NPC ile konuş / ceset-kutu aç / nesne olayı.
	// Çift dokunuş zaten iki hızlı sol tık → oyunun çift tık = hedefe saldır davranışı.
	{
		uint32_t now = timeGetTime();
		for (Finger& f : m_fingers)
		{
			if (f.role != Role::Pending)
				continue;
			if (now - f.downTicks >= m_tuning.longPressMs && std::abs(f.x - f.startX) <= TAP_SLOP && std::abs(f.y - f.startY) <= TAP_SLOP)
			{
				if (IsOverUI(f.startX, f.startY))
				{
					// Arayüz üstünde uzun basış = ikonu/pencereyi tut (sürükleme başlar); sağ tık
					// (eşyayı kullan) değil — kullanmak için çift dokunuş
					BeginLeftDrag(f);
					f.longPressDrag = true;
				}
				else
				{
					in.taps.push_back({f.x, f.y, true}); // 3D dünya: sağ tık (NPC, kapı, kutu)
					f.role = Role::Done;
				}
			}
		}
		// Sürüklenen parmağı izle (başlangıç kareleri bittikten sonra)
		for (Finger& f : m_fingers)
			if (f.role == Role::LeftDrag && in.dragHoldFrames == 0)
			{
				in.mouseX = f.x;
				in.mouseY = f.y;
			}
	}
	// Beceri sayfası tuşu birkaç kare basılı tutulur
	if (m_pageKeyFrames > 0)
	{
		in.virtualKeysDIK[m_pageKey] = 0x80;
		--m_pageKeyFrames;
	}
	if (!m_enabled)
		return;

	// Joystick → W/S/A/D
	if (m_joyFinger >= 0)
	{
		if (Finger* f = Find(m_joyFinger))
		{
			float dx = (float) f->x - m_joyCx, dy = (float) f->y - m_joyCy;
			float len = std::sqrt(dx * dx + dy * dy);
			if (len > m_joyR)
			{
				dx *= m_joyR / len;
				dy *= m_joyR / len;
				len = m_joyR;
			}
			m_knobX = m_joyCx + dx;
			m_knobY = m_joyCy + dy;
			if (len > m_joyR * m_tuning.joyDeadZone)
			{
				float nx = dx / len, ny = dy / len;
				CGameEng* pEng = CGameProcedure::s_pEng;
				CPlayerMySelf* pMe = CGameProcedure::s_pPlayer;
				if (m_tuning.joyTurnAndRun && pEng != nullptr && pMe != nullptr && IsInGame() && pMe->IsAlive())
				{
					// Mobil "dön ve koş": joystick yönü kameraya göre dünya yönüne çevrilir, karakter anında
					// o yöne döndürülür ve ileri koşar (60°/sn'lik A/D dönüşü yerine)
					__Vector3 vDir(nx, 0.0f, -ny);
					__Matrix44 mtxRot;
					mtxRot.RotationY(pEng->CameraYaw());
					vDir *= mtxRot;
					vDir.y = 0.0f;
					vDir.Normalize();
					pMe->RotateTo(::_Yaw2D(vDir.x, vDir.z), true);
					in.virtualKeysDIK[KM_MOVE_FOWARD] = 0x80;
				}
				else
				{
					// Klasik kip: yukarı/aşağı = ileri/geri, yatay = dönüş. Dönüş hızı joystick yatıklığıyla orantılı
					// (en fazla joyRotateDegPerSec), oyunun sabit 60°/sn A/D'sinden hızlı.
					if (ny < -0.35f) in.virtualKeysDIK[KM_MOVE_FOWARD] = 0x80;
					if (ny > 0.35f) in.virtualKeysDIK[KM_MOVE_BACKWARD] = 0x80;
					float fTurn = std::fabs(nx) > 0.25f ? nx : 0.0f;
					if (fTurn != 0.0f && pMe != nullptr && IsInGame() && pMe->IsAlive())
						pMe->RotAdd(fTurn * m_tuning.joyRotateDegPerSec * 3.14159265f / 180.0f);
				}
			}
		}
	}

	// Tuşlar
	CGameEng* eng = CGameProcedure::s_pEng;
	for (Button& b : m_buttons)
	{
		bool bPressed = b.down || b.tapQueued;
		b.tapQueued   = false;
		if (!bPressed)
			continue;
		switch (b.action)
		{
			case Action::Key:
				if (b.hotkeySlot >= 0 && b.down)
					break; // yuva basılı tutuluyor: sürükleme olabilir; tuş kalkışta (tapQueued) işlenir
				in.virtualKeysDIK[b.dik] = 0x80;
				break;
			case Action::ToggleHide:
				if (!b.fired)
				{
					m_hidden   = !m_hidden;
					m_relayout = true; // döngü içinde m_buttons değiştirilemez (referanslar geçersiz olurdu)
					CLogWriter::Write("Dokunmatik: kaplama {}", m_hidden ? "GİZLENDİ" : "GÖSTERİLDİ");
				}
				b.fired = true;
				break;
			case Action::Yaw180:
				if (!b.fired && eng && IsInGame())
					eng->CameraYawAdd(3.1415926f);
				b.fired = true;
				break;
			case Action::ZoomIn:
				if (eng && IsInGame()) eng->CameraZoom(0.6f);
				break;
			case Action::ZoomOut:
				if (eng && IsInGame()) eng->CameraZoom(-0.6f);
				break;
			case Action::SkillPage:
				if (!b.fired)
				{
					m_skillPage     = b.dik - KM_SKILL_PAGE_1 + 1; // F1..F8 ardışık
					m_pageKey       = b.dik;
					m_pageKeyFrames = 2;
					b.fired         = true;
				}
				break;
		}
	}

	// İki parmakla yakınlaştırma
	std::vector<Finger*> free;
	for (auto& f : m_fingers)
		if (f.role == Role::Camera || f.role == Role::Pending)
			free.push_back(&f);
	if (free.size() >= 2 && IsInGame() && eng)
	{
		float dx = (float) (free[0]->x - free[1]->x), dy = (float) (free[0]->y - free[1]->y);
		float dist = std::sqrt(dx * dx + dy * dy);
		if (m_pinchDist > 0.0f)
			eng->CameraZoom((m_pinchDist - dist) * 0.01f);
		m_pinchDist = dist;
		in.rbDown   = false;
	}
	else
		m_pinchDist = 0.0f;
	if (m_relayout)
	{
		m_relayout = false;
		Layout(m_w, m_h); // etiket GIZLE/GOSTER
	}
}

// --- Çizim yardımcıları --------------------------------------------------------
void KoTouchOverlay::DrawCircle(IDirect3DDevice9* dev, float cx, float cy, float r, uint32_t color, int segs)
{
	std::vector<RhwVertex> v;
	v.reserve(segs + 2);
	v.push_back({cx, cy, 0.5f, 1.0f, color});
	for (int i = 0; i <= segs; ++i)
	{
		float a = (float) i / segs * 6.2831853f;
		v.push_back({cx + std::cos(a) * r, cy + std::sin(a) * r, 0.5f, 1.0f, color});
	}
	dev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, segs, v.data(), sizeof(RhwVertex));
}

void KoTouchOverlay::DrawRing(IDirect3DDevice9* dev, float cx, float cy, float r, float t, uint32_t color, int segs)
{
	std::vector<RhwVertex> v;
	v.reserve((segs + 1) * 2);
	for (int i = 0; i <= segs; ++i)
	{
		float a = (float) i / segs * 6.2831853f;
		float c = std::cos(a), s = std::sin(a);
		v.push_back({cx + c * (r - t), cy + s * (r - t), 0.5f, 1.0f, color});
		v.push_back({cx + c * r, cy + s * r, 0.5f, 1.0f, color});
	}
	dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, segs * 2, v.data(), sizeof(RhwVertex));
}

void KoTouchOverlay::DrawRect(IDirect3DDevice9* dev, float x, float y, float w, float h, uint32_t color)
{
	RhwVertex v[4] = {{x, y, 0.5f, 1.0f, color}, {x + w, y, 0.5f, 1.0f, color}, {x, y + h, 0.5f, 1.0f, color}, {x + w, y + h, 0.5f, 1.0f, color}};
	dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, sizeof(RhwVertex));
}

void KoTouchOverlay::DrawLabel(IDirect3DDevice9* dev, int index, const std::string& text, float x, float y, uint32_t color, int height)
{
	while ((int) m_fonts.size() <= index)
		m_fonts.push_back(nullptr);
	CDFont*& f = m_fonts[index];
	if (!f)
	{
		f = new CDFont("Arial", (uint32_t) height, D3DFONT_BOLD);
		f->InitDeviceObjects(dev);
		f->RestoreDeviceObjects();
		f->SetText(text);
	}
	SIZE sz = f->GetSize();
	f->DrawText(x - sz.cx / 2.0f, y - sz.cy / 2.0f, color, 0);
}

void KoTouchOverlay::RenderFps(IDirect3DDevice9* dev)
{
	if (!m_showFps || !dev)
		return;
	uint32_t now = timeGetTime();
	++m_fpsFrames;
	if (m_fpsLastTick == 0)
		m_fpsLastTick = now;
	if (now - m_fpsLastTick >= 500)
	{
		m_fpsValue    = m_fpsFrames * 1000.0f / (float) (now - m_fpsLastTick);
		m_fpsFrames   = 0;
		m_fpsLastTick = now;
		char buf[96];
		std::snprintf(buf, sizeof(buf), "FPS %.1f  %dx%d  olcek %.0f%%", m_fpsValue, m_w, m_h, d3d9gles::GetRenderScale() * 100.0f);
		m_fpsText = buf;
	}
	if (m_fpsText.empty())
		return;
	dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	dev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	dev->SetRenderState(D3DRS_LIGHTING, FALSE);
	dev->SetRenderState(D3DRS_FOGENABLE, FALSE);
	dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	dev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	dev->SetTexture(0, nullptr);
	dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	dev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	float u = m_u > 0 ? m_u : 1.0f;
	DrawRect(dev, 4.0f * u, 4.0f * u, 230.0f * u, 18.0f * u, 0x99000000);
	if (!m_fpsFont)
	{
		m_fpsFont = new CDFont("Arial", (uint32_t) (11 * u), D3DFONT_BOLD);
		m_fpsFont->InitDeviceObjects(dev);
		m_fpsFont->RestoreDeviceObjects();
	}
	m_fpsFont->SetText(m_fpsText);
	m_fpsFont->DrawText(8.0f * u, 6.0f * u, m_fpsValue < 20.0f ? 0xFFFF6060 : 0xFFB0FFB0, 0);
}

namespace
{
// Kaplamanın değiştirdiği çizim durumlarını sakla/geri yükle: giriş ekranında UI çizimi (dokulu
// alfa) bu durumları her kare yeniden kurmuyor; sızan ALPHAOP/ALPHAARG/doku durumu beyaz kutular
// (alfa=0 bölgeler opak) çiziyordu
struct KoRenderStateGuard
{
	IDirect3DDevice9* dev;
	static constexpr D3DRENDERSTATETYPE RS[] = { D3DRS_ALPHABLENDENABLE, D3DRS_SRCBLEND, D3DRS_DESTBLEND, D3DRS_ZENABLE,
		D3DRS_LIGHTING, D3DRS_FOGENABLE, D3DRS_CULLMODE, D3DRS_ALPHATESTENABLE };
	static constexpr D3DTEXTURESTAGESTATETYPE TS[] = { D3DTSS_COLOROP, D3DTSS_COLORARG1, D3DTSS_ALPHAOP, D3DTSS_ALPHAARG1 };
	DWORD rs[8] = {}, ts0[4] = {}, ts1 = 0, fvf = 0;
	IDirect3DBaseTexture9* tex0 = nullptr;
	explicit KoRenderStateGuard(IDirect3DDevice9* d) : dev(d)
	{
		for (int i = 0; i < 8; i++)
			dev->GetRenderState(RS[i], &rs[i]);
		for (int i = 0; i < 4; i++)
			dev->GetTextureStageState(0, TS[i], &ts0[i]);
		dev->GetTextureStageState(1, D3DTSS_COLOROP, &ts1);
		dev->GetFVF(&fvf);
		dev->GetTexture(0, &tex0);
	}
	~KoRenderStateGuard()
	{
		for (int i = 0; i < 8; i++)
			dev->SetRenderState(RS[i], rs[i]);
		for (int i = 0; i < 4; i++)
			dev->SetTextureStageState(0, TS[i], ts0[i]);
		dev->SetTextureStageState(1, D3DTSS_COLOROP, ts1);
		dev->SetFVF(fvf);
		dev->SetTexture(0, tex0);
		if (tex0)
			tex0->Release();
	}
};
} // namespace

void KoTouchOverlay::Render(IDirect3DDevice9* dev)
{
	RenderFps(dev);
	if (!dev)
		return;
	bool bPreGame = IsServerSelect() || IsCharacterCreate() || IsCharacterSelect();
	if (!bPreGame && (!m_enabled || (!IsInGame() && !m_forceVisible)))
	{
		if (!m_lastRenderSkipped)
			CLogWriter::Write("Dokunmatik: kaplama çizilmiyor (etkin {}, oyunda {}, aktif süreç {})", m_enabled ? 1 : 0, IsInGame() ? 1 : 0,
				(void*) CGameProcedure::s_pProcActive);
		m_lastRenderSkipped = true;
		return;
	}
	if (m_lastRenderSkipped)
		CLogWriter::Write("Dokunmatik: kaplama yeniden çiziliyor");
	m_lastRenderSkipped = false;

	KoRenderStateGuard guard(dev);

	auto setup = [&]() {
		dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		dev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		dev->SetRenderState(D3DRS_LIGHTING, FALSE);
		dev->SetRenderState(D3DRS_FOGENABLE, FALSE);
		dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		dev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		dev->SetTexture(0, nullptr);
		dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
		dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		dev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	};
	if (IsCharacterCreate() || IsCharacterSelect())
	{
		LayoutPreButtons();
		setup();
		for (const PreButton& b : m_preButtons)
		{
			DrawRect(dev, b.x, b.y, b.w, b.h, b.selected ? 0xE0C09040 : 0xC0503820);
			DrawRect(dev, b.x, b.y, b.w, 2.5f * m_u, COL_SLOT_RING);
			DrawRect(dev, b.x, b.y + b.h - 2.5f * m_u, b.w, 2.5f * m_u, COL_SLOT_RING);
			DrawLabel(dev, b.fontIdx, b.label, b.x + b.w / 2, b.y + b.h / 2, 0xFFFFFFFF, (int) (20.0f * m_u));
			setup();
		}
		return;
	}

	if (IsServerSelect())
	{
		// Sunucu seçme ekranı: yalnızca BAĞLAN düğmesi (kaplama ayarından bağımsız)
		setup();
		float x0 = m_connCx - m_connW / 2, y0 = m_connCy - m_connH / 2;
		DrawRect(dev, x0, y0, m_connW, m_connH, m_connDown ? 0xE0C09040 : 0xC0805020);
		DrawRect(dev, x0, y0, m_connW, 3.0f * m_u, COL_SLOT_RING);
		DrawRect(dev, x0, y0 + m_connH - 3.0f * m_u, m_connW, 3.0f * m_u, COL_SLOT_RING);
		DrawLabel(dev, (int) m_buttons.size() + 2, "BAGLAN  (Enter)", m_connCx, m_connCy, 0xFFFFFFFF, (int) (26.0f * m_u));
		m_connDown = false;
		return;
	}

	if (!m_enabled)
		return;
	if (!IsInGame() && !m_forceVisible)
		return;
	if (CN3UIBase::GetFocusedEdit() != nullptr && !m_forceVisible)
		return; // sohbet yazarken kaplamayı gizle

	setup();
	float u = m_u;

	if (KoInput().stickyDrag)
	{
		DrawRect(dev, m_w / 2.0f - 170.0f * u, 48.0f * u, 340.0f * u, 24.0f * u, 0xB0203050);
		DrawLabel(dev, (int) m_buttons.size() + 1, "Birakmak icin hedef yuvaya dokun", m_w / 2.0f, 60.0f * u, COL_TEXT, 11);
		setup();
	}

	// Joystick (aktifken parmağın altında, değilken soluk ipucu)
	bool joyActive = m_joyFinger >= 0;
	if (!m_hidden)
	{
		uint32_t base = joyActive ? COL_JOY_BASE : (COL_JOY_BASE & 0x00FFFFFF) | 0x18000000;
		uint32_t ring = joyActive ? COL_JOY_RING : (COL_JOY_RING & 0x00FFFFFF) | 0x40000000;
		DrawCircle(dev, m_joyCx, m_joyCy, m_joyR, base, 48);
		DrawRing(dev, m_joyCx, m_joyCy, m_joyR, 3.0f * u, ring, 48);
		DrawCircle(dev, m_knobX, m_knobY, m_joyR * 0.38f, joyActive ? COL_KNOB : (COL_KNOB & 0x00FFFFFF) | 0x40000000);
	}

	// Alt çubuk arka planı
	float barH = m_barH > 0 ? m_barH : 40.0f * u;
	if (!m_hidden)
	{
		DrawRect(dev, 0, m_h - barH, (float) m_w, barH, COL_BAR);
		DrawRect(dev, 0, m_h - barH, (float) m_w, 2.0f * u, COL_SLOT_RING);
	}

	// Tuşlar
	for (const Button& b : m_buttons)
	{
		if (m_hidden && b.action != Action::ToggleHide)
			continue;
		uint32_t col = b.down ? COL_DOWN : b.color;
		if (b.action == Action::SkillPage && b.dik == KM_SKILL_PAGE_1 + m_skillPage - 1)
			col = b.down ? COL_DOWN : COL_PAGE_ON; // seçili beceri sayfası
		switch (b.shape)
		{
			case Shape::Rect:
				DrawRect(dev, b.cx - b.w / 2, b.cy - b.h / 2, b.w, b.h, col);
				DrawRect(dev, b.cx - b.w / 2, b.cy - b.h / 2, b.w, 1.5f * u, COL_SLOT_RING);
				DrawRect(dev, b.cx - b.w / 2, b.cy + b.h / 2 - 1.5f * u, b.w, 1.5f * u, COL_SLOT_RING);
				break;
			case Shape::Ring:
				DrawCircle(dev, b.cx, b.cy, b.r, col);
				DrawRing(dev, b.cx, b.cy, b.r, 2.5f * u, COL_SLOT_RING);
				break;
			case Shape::Circle:
				DrawCircle(dev, b.cx, b.cy, b.r, col, 40);
				DrawRing(dev, b.cx, b.cy, b.r, 3.0f * u, b.action == Action::Key && b.dik == KM_TOGGLE_ATTACK ? COL_ATTACK_RING : COL_CAM_RING, 40);
				break;
		}
	}
	if (!m_hidden)
	{
		DrawSkillIcons(dev);
		setup();
		DrawInfoLines(dev);
		setup();
	}
	// Oyuncu menüsü
	{
		std::vector<MenuRow> rows;
		LayoutPlayerMenu(rows);
		if (!rows.empty())
		{
			setup();
			int fontBase = 200;
			for (const MenuRow& r : rows)
			{
				DrawRect(dev, r.x, r.y, r.w, r.h, r.action == 4 ? 0xD0603030 : 0xD0203050);
				DrawRect(dev, r.x, r.y, r.w, 2.0f * m_u, COL_SLOT_RING);
				DrawLabel(dev, fontBase + r.action, r.label, r.x + r.w / 2, r.y + r.h / 2, 0xFFFFFFFF, (int) (18.0f * m_u));
				setup();
			}
		}
	}
	for (size_t i = 0; i < m_buttons.size(); ++i)
	{
		const Button& b = m_buttons[i];
		if (m_hidden && b.action != Action::ToggleHide)
			continue;
		int height      = b.shape == Shape::Rect ? 9 : (b.r > 40.0f * u ? 12 : 9);
		DrawLabel(dev, (int) i, b.label, b.cx, b.cy, COL_TEXT, height);
		setup(); // DFont durumları değiştirir
	}
}


// ---------------------------------------------------------------------------
// Beceri ikonları: oyunun kısayol penceresi (gizli) veri modelidir; seçili sayfadaki yuvaların ikonları
// kaplamadaki 1-8 halkalarının içine çizilir.
// ---------------------------------------------------------------------------
namespace
{
struct RhwTexVertex
{
	float x, y, z, rhw;
	D3DCOLOR color;
	float u, v;
};
} // namespace

void KoTouchOverlay::DrawTexturedQuad(IDirect3DDevice9* dev, float x, float y, float w, float h, void* pTexV, uint32_t color)
{
	LPDIRECT3DTEXTURE9 pTex = (LPDIRECT3DTEXTURE9) pTexV;
	if (pTex == nullptr)
		return;
	const float uv = 45.0f / 64.0f; // KO ikon dokuları 64² içinde 45² kullanır
	RhwTexVertex v[4] = {{x, y, 0.5f, 1.0f, color, 0, 0}, {x + w, y, 0.5f, 1.0f, color, uv, 0}, {x, y + h, 0.5f, 1.0f, color, 0, uv},
		{x + w, y + h, 0.5f, 1.0f, color, uv, uv}};
	dev->SetTexture(0, pTex);
	dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	dev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, sizeof(RhwTexVertex));
	dev->SetTexture(0, nullptr);
}

void KoTouchOverlay::DrawSkillIcons(IDirect3DDevice9* dev)
{
	if (!IsInGame() || CGameProcedure::s_pProcMain == nullptr)
		return;
	CUIHotKeyDlg* pHK = CGameProcedure::s_pProcMain->m_pUIHotKeyDlg;
	if (pHK == nullptr)
		return;
	const int hotkeys[8] = {KM_HOTKEY1, KM_HOTKEY2, KM_HOTKEY3, KM_HOTKEY4, KM_HOTKEY5, KM_HOTKEY6, KM_HOTKEY7, KM_HOTKEY8};
	int page = pHK->m_iCurPage;
	if (page < 0 || page >= MAX_SKILL_HOTKEY_PAGE)
		return;
	for (const Button& b : m_buttons)
	{
		if (b.action != Action::Key || b.shape != Shape::Ring)
			continue;
		int slot = -1;
		for (int i = 0; i < 8; i++)
			if (b.dik == hotkeys[i])
				slot = i;
		if (slot < 0 || slot >= MAX_SKILL_IN_HOTKEY)
			continue;
		__IconItemSkill* pItem = pHK->m_pMyHotkey[page][slot];
		if (pItem == nullptr || pItem->szIconFN.empty())
			continue;
		CN3Texture* pTex = CN3Base::s_MngTex.Get(pItem->szIconFN, false);
		if (pTex == nullptr || pTex->Get() == nullptr)
			continue;
		float s = b.r * 1.3f; // halkanın içine sığan kare
		DrawTexturedQuad(dev, b.cx - s / 2, b.cy - s / 2, s, s, pTex->Get(), b.down ? 0xFFFFFFFF : 0xE0FFFFFF);
	}
}

// ---------------------------------------------------------------------------
// Oyuncu menüsü (seçili dost oyuncuya ikinci dokunuş): PARTI / TICARET / FISILDA / ARKADAS / KAPAT
// ---------------------------------------------------------------------------
void KoTouchOverlay::LayoutPlayerMenu(std::vector<MenuRow>& rows) const
{
	rows.clear();
	CGameProcMain* pMain = CGameProcedure::s_pProcMain;
	if (pMain == nullptr || pMain->m_iTouchMenuPlayerID < 0)
		return;
	const float u = m_u, w = 180.0f * u, h = 40.0f * u, gap = 4.0f * u;
	const char* labels[5] = {"PARTI DAVET", "TICARET", "FISILDA", "ARKADAS EKLE", "KAPAT"};
	float x = std::clamp((float) pMain->m_iTouchMenuX - w / 2, 8.0f * u, (float) m_w - w - 8.0f * u);
	float y = std::clamp((float) pMain->m_iTouchMenuY - (5 * (h + gap)) - 10.0f * u, 60.0f * u, (float) m_h - 5 * (h + gap) - 60.0f * u);
	for (int i = 0; i < 5; i++)
		rows.push_back({i, x, y + i * (h + gap), w, h, labels[i]});
}

bool KoTouchOverlay::HitPlayerMenu(int x, int y)
{
	CGameProcMain* pMain = CGameProcedure::s_pProcMain;
	if (pMain == nullptr || pMain->m_iTouchMenuPlayerID < 0)
		return false;
	std::vector<MenuRow> rows;
	LayoutPlayerMenu(rows);
	for (const MenuRow& r : rows)
		if (x >= r.x - 4 && x <= r.x + r.w + 4 && y >= r.y - 4 && y <= r.y + r.h + 4)
		{
			pMain->TouchMenuAction(r.action);
			return true;
		}
	return false;
}


bool KoTouchOverlay::IsOverDialogUI(int x, int y) const
{
	CUIManager* mgr = CGameProcedure::s_pUIMgr;
	if (!mgr || !mgr->IsVisible())
		return false;
	CGameProcMain* pMain = CGameProcedure::s_pProcMain;
	for (CN3UIBase* child : mgr->GetChildren())
	{
		if (child == nullptr || !child->IsVisible() || !child->IsIn(x, y))
			continue;
		if (pMain != nullptr)
		{
			// Kalıcı HUD pencereleri joystick alanını kapatmaz
			if (child == (CN3UIBase*) pMain->m_pUIChatDlg || child == (CN3UIBase*) pMain->m_pUIMsgDlg
				|| child == (CN3UIBase*) pMain->m_pUIStateBarAndMiniMap || child == (CN3UIBase*) pMain->m_pUIHotKeyDlg
				|| child == (CN3UIBase*) pMain->m_pUICmd)
				continue;
		}
		return true;
	}
	return false;
}

int KoTouchOverlay::HitHotkeyRing(int x, int y) const
{
	for (const Button& b : m_buttons)
	{
		if (b.hotkeySlot < 0)
			continue;
		float dx = x - b.cx, dy = y - b.cy;
		if (dx * dx + dy * dy <= b.r * b.r * 1.3f)
			return b.hotkeySlot;
	}
	return -1;
}

void KoTouchOverlay::AddInfoLine(const std::string& text, uint32_t color)
{
	m_infoLines.push_back({text, color, (uint32_t) timeGetTime()});
	while (m_infoLines.size() > 6)
		m_infoLines.pop_front();
}

void KoTouchOverlay::DrawInfoLines(IDirect3DDevice9* dev)
{
	if (m_infoLines.empty() || !IsInGame())
		return;
	const uint32_t now = timeGetTime();
	while (!m_infoLines.empty() && now - m_infoLines.front().ticks > 9000)
		m_infoLines.pop_front();
	if (m_infoLines.empty())
		return;
	const float u = m_u, lh = 15.0f * u, pad = 6.0f * u;
	const float right = (float) m_w - (CGameProcedure::s_iTouchInsetRight > 0 ? (float) CGameProcedure::s_iTouchInsetRight : 90.0f * u);
	const float w     = std::min((float) m_w * 0.42f, 420.0f * u);
	float bottom      = (m_clusterTop > 0 ? m_clusterTop : m_h * 0.5f) - 6.0f * u;
	float top         = bottom - pad * 2 - lh * (float) m_infoLines.size();
	DrawRect(dev, right - w, top, w, bottom - top, 0x60000000); // yarı saydam zemin
	int idx = 300;
	float y = top + pad + lh / 2;
	for (const InfoLine& l : m_infoLines)
	{
		uint32_t age   = now - l.ticks;
		uint32_t alpha = age > 6000 ? (uint32_t) (255 * (9000 - age) / 3000) : 255;
		uint32_t col   = (alpha << 24) | (l.color & 0x00FFFFFF);
		while ((int) m_fonts.size() <= idx)
			m_fonts.push_back(nullptr);
		CDFont*& f = m_fonts[idx];
		if (!f)
		{
			f = new CDFont("Arial", (uint32_t) (12.0f * u), 0);
			f->InitDeviceObjects(dev);
			f->RestoreDeviceObjects();
		}
		while (m_infoFontText.size() <= (size_t) idx)
			m_infoFontText.emplace_back();
		if (m_infoFontText[idx] != l.text)
		{
			f->SetText(l.text); // yalnız metin değişince doku üretilir
			m_infoFontText[idx] = l.text;
		}
		f->DrawText(right - w + pad, y - lh / 2, col, 0);
		y += lh;
		idx++;
	}
}
