// KoTouchOverlay.cpp — dokunmatik kontrol kaplaması (bkz. KoTouchOverlay.h).
#include "StdAfx.h"
#include "KoTouchOverlay.h"
#include <cstdio>
#include "KoPlatformInput.h"

#include "GameProcedure.h"
#include "GameProcMain.h"
#include "GameEng.h"
#include "GameDef.h"

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

void KoTouchOverlay::Layout(int w, int h)
{
	m_w = w;
	m_h = h;
	float u = (float) h / 768.0f;
	m_u     = u;
	m_joyR  = 80.0f * u;
	m_joyHomeX = m_joyR + 70.0f * u;
	m_joyHomeY = h - m_joyR - 120.0f * u;
	m_joyCx = m_joyHomeX;
	m_joyCy = m_joyHomeY;
	m_knobX = m_joyCx;
	m_knobY = m_joyCy;
	m_joyFinger = -1;

	m_buttons.clear();
	auto circle = [&](Action a, int dik, const char* label, float cx, float cy, float r, uint32_t col) {
		Button b;
		b.action = a; b.dik = dik; b.label = label; b.cx = cx; b.cy = cy; b.r = r; b.color = col;
		m_buttons.push_back(b);
	};
	auto rect = [&](int dik, const char* label, float cx, float cy, float bw, float bh) {
		Button b;
		b.shape = Shape::Rect; b.dik = dik; b.label = label; b.cx = cx; b.cy = cy; b.w = bw; b.h = bh; b.color = COL_BAR_BTN;
		m_buttons.push_back(b);
	};

	// --- Sağ alt: saldırı + beceri yayı + hedef (KO Mobile düzeni) ---
	float atkR  = 46.0f * u;
	float atkCx = w - 110.0f * u, atkCy = h - 170.0f * u;
	circle(Action::Key, KM_TOGGLE_ATTACK, "SALDIR", atkCx, atkCy, atkR, COL_ATTACK);
	circle(Action::Key, KM_TARGET_NEAREST_ENEMY, "HEDEF", atkCx + 58.0f * u, atkCy - 70.0f * u, 24.0f * u, COL_TARGET);
	// 8 beceri yuvası: iç yay 4, dış yay 4 (sol-üst çeyrek)
	const int hotkeys[8] = {KM_HOTKEY1, KM_HOTKEY2, KM_HOTKEY3, KM_HOTKEY4, KM_HOTKEY5, KM_HOTKEY6, KM_HOTKEY7, KM_HOTKEY8};
	const char* names[8] = {"1", "2", "3", "4", "5", "6", "7", "8"};
	float slotR = 28.0f * u;
	for (int i = 0; i < 4; ++i)
	{
		float a = DegreesToRadians(185.0f + i * 28.0f);
		float rr = atkR + slotR + 26.0f * u;
		Button b;
		b.shape = Shape::Ring; b.dik = hotkeys[i]; b.label = names[i];
		b.cx = atkCx + std::cos(a) * rr; b.cy = atkCy + std::sin(a) * rr; b.r = slotR; b.color = COL_SLOT;
		m_buttons.push_back(b);
	}
	for (int i = 0; i < 4; ++i)
	{
		float a = DegreesToRadians(190.0f + i * 26.0f);
		float rr = atkR + slotR * 3.0f + 46.0f * u;
		Button b;
		b.shape = Shape::Ring; b.dik = hotkeys[4 + i]; b.label = names[4 + i];
		b.cx = atkCx + std::cos(a) * rr; b.cy = atkCy + std::sin(a) * rr; b.r = slotR; b.color = COL_SLOT;
		m_buttons.push_back(b);
	}

	// --- Sağ üst: kamera kümesi ---
	float camR = 22.0f * u, camY = 70.0f * u, camX0 = w - 40.0f * u, step = 56.0f * u;
	circle(Action::Key, KM_CAMERA_CHANGE, "KAM", camX0, camY, camR, COL_CAM);
	circle(Action::Yaw180, 0, "180", camX0 - step, camY, camR, COL_CAM);
	circle(Action::ZoomIn, 0, "+", camX0 - 2 * step, camY, camR, COL_CAM);
	circle(Action::ZoomOut, 0, "-", camX0 - 3 * step, camY, camR, COL_CAM);
	circle(Action::Key, KM_TOGGLE_RUN, "KOS", camX0 - 4 * step, camY, camR, COL_CAM);

	// --- Alt çubuk ---
	const struct { int dik; const char* label; } bar[] = {
		{KM_TOGGLE_INVENTORY, "CANTA"}, {KM_TOGGLE_STATE, "KARAKTER"}, {KM_TOGGLE_SKILL, "BECERI"},
		{KM_TOGGLE_SITDOWN, "OTUR"}, {KM_TOGGLE_MINIMAP, "HARITA"}, {KM_DROPPED_ITEM_OPEN, "AL"},
		{DIK_RETURN, "SOHBET"}, {DIK_ESCAPE, "MENU"}};
	int n = (int) (sizeof(bar) / sizeof(bar[0]));
	float bw = 96.0f * u, bh = 26.0f * u, gap = 10.0f * u;
	float total = n * bw + (n - 1) * gap;
	float x0 = (w - total) / 2.0f + bw / 2.0f, y0 = h - 20.0f * u;
	for (int i = 0; i < n; ++i)
		rect(bar[i].dik, bar[i].label, x0 + i * (bw + gap), y0, bw, bh);

	// --- HP / MP pot düğmeleri (kısayol yuvası ayarlanabilir) ---
	const int slotKeys[8] = {KM_HOTKEY1, KM_HOTKEY2, KM_HOTKEY3, KM_HOTKEY4, KM_HOTKEY5, KM_HOTKEY6, KM_HOTKEY7, KM_HOTKEY8};
	int hp = std::clamp(m_tuning.hpSlot, 1, 8) - 1, mp = std::clamp(m_tuning.mpSlot, 1, 8) - 1;
	circle(Action::Key, slotKeys[hp], "HP", w - 300.0f * u, h - 150.0f * u, 24.0f * u, 0x90A02828);
	circle(Action::Key, slotKeys[mp], "MP", w - 300.0f * u, h - 92.0f * u, 24.0f * u, 0x902848A0);

	// --- En az dokunma boyutu (~48dp): küçük düğmeleri büyüt ---
	if (m_tuning.minTouchPx > 0.0f)
	{
		float minR = m_tuning.minTouchPx / 2.0f;
		for (Button& b : m_buttons)
		{
			if (b.shape == Shape::Rect)
				b.h = std::max(b.h, m_tuning.minTouchPx);
			else
				b.r = std::max(b.r, minR);
		}
		m_joyR = std::max(m_joyR, m_tuning.minTouchPx * 1.4f);
	}

	for (CDFont* f : m_fonts)
		delete f;
	m_fonts.clear();
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
	return x < m_w * 0.45f && y > m_h * 0.2f && y < m_h - 45.0f * m_u;
}

void KoTouchOverlay::OnFingerDown(int64_t id, int x, int y)
{
	KoInputState& in = KoInput();
	Finger f {id, Role::Pending, x, y, x, y, -1, timeGetTime()};
	bool overlayActive = m_enabled && (IsInGame() || m_forceVisible) && CN3UIBase::GetFocusedEdit() == nullptr;
	if (overlayActive)
	{
		int b = HitButton(x, y);
		if (b >= 0 && m_buttons[b].finger < 0)
		{
			f.role        = Role::Button;
			f.buttonIndex = b;
			m_buttons[b].finger = id;
			m_buttons[b].down   = true;
			m_buttons[b].fired  = false;
			m_fingers.push_back(f);
			return;
		}
		if (m_joyFinger < 0 && InJoystickZone(x, y))
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
		case Role::Pending:
			if (std::abs(x - f->startX) > TAP_SLOP || std::abs(y - f->startY) > TAP_SLOP)
			{
				bool camera = m_enabled && IsInGame();
				f->role     = camera ? Role::Camera : Role::LeftDrag;
				if (camera)
					in.rbDown = true;
				else
					in.lbDown = true;
				in.mouseX = f->startX;
				in.mouseY = f->startY;
			}
			break;
		case Role::Camera:
			// Kamera hassasiyeti: parmağın başlangıca göre kaymasını çarpanla ilet (oyun kare
			// farkını kullanır, mutlak ölçekleme aynı sonucu verir)
			in.mouseX = f->startX + (int) std::lround((x - f->startX) * m_tuning.camSens);
			in.mouseY = f->startY + (int) std::lround((y - f->startY) * m_tuning.camSens);
			break;
		case Role::LeftDrag:
			in.mouseX = x;
			in.mouseY = y;
			break;
		default:
			break;
	}
}

void KoTouchOverlay::ReleaseFinger(Finger& f)
{
	KoInputState& in = KoInput();
	switch (f.role)
	{
		case Role::Button:
			if (f.buttonIndex >= 0 && f.buttonIndex < (int) m_buttons.size())
			{
				m_buttons[f.buttonIndex].down   = false;
				m_buttons[f.buttonIndex].finger = -1;
			}
			break;
		case Role::Joystick:
			m_joyFinger = -1;
			m_joyCx = m_knobX = m_joyHomeX;
			m_joyCy = m_knobY = m_joyHomeY;
			break;
		case Role::Pending:
			in.taps.push_back({f.x, f.y, false}); // CLocalInput::Tick kare kare işler
			break;
		case Role::Camera:
			in.rbDown   = false;
			m_pinchDist = 0.0f;
			break;
		case Role::LeftDrag:
			in.lbDown = false;
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
				in.taps.push_back({f.x, f.y, true});
				f.role = Role::Done;
			}
		}
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
				if (ny < -0.35f) in.virtualKeysDIK[KM_MOVE_FOWARD] = 0x80;
				if (ny > 0.35f) in.virtualKeysDIK[KM_MOVE_BACKWARD] = 0x80;
				if (nx < -0.45f) in.virtualKeysDIK[KM_ROTATE_LEFT] = 0x80;
				if (nx > 0.45f) in.virtualKeysDIK[KM_ROTATE_RIGHT] = 0x80;
			}
		}
	}

	// Tuşlar
	CGameEng* eng = CGameProcedure::s_pEng;
	for (Button& b : m_buttons)
	{
		if (!b.down)
			continue;
		switch (b.action)
		{
			case Action::Key: in.virtualKeysDIK[b.dik] = 0x80; break;
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
		}
	}

	// İki parmakla yakınlaştırma
	std::vector<Finger*> free;
	for (auto& f : m_fingers)
		if (f.role == Role::Camera || f.role == Role::Pending || f.role == Role::LeftDrag)
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

void KoTouchOverlay::Render(IDirect3DDevice9* dev)
{
	RenderFps(dev);
	if (!m_enabled || !dev)
		return;
	if (!IsInGame() && !m_forceVisible)
		return;
	if (CN3UIBase::GetFocusedEdit() != nullptr && !m_forceVisible)
		return; // sohbet yazarken kaplamayı gizle

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
	setup();
	float u = m_u;

	// Joystick (aktifken parmağın altında, değilken soluk ipucu)
	bool joyActive = m_joyFinger >= 0;
	uint32_t base  = joyActive ? COL_JOY_BASE : (COL_JOY_BASE & 0x00FFFFFF) | 0x18000000;
	uint32_t ring  = joyActive ? COL_JOY_RING : (COL_JOY_RING & 0x00FFFFFF) | 0x40000000;
	DrawCircle(dev, m_joyCx, m_joyCy, m_joyR, base, 48);
	DrawRing(dev, m_joyCx, m_joyCy, m_joyR, 3.0f * u, ring, 48);
	DrawCircle(dev, m_knobX, m_knobY, m_joyR * 0.38f, joyActive ? COL_KNOB : (COL_KNOB & 0x00FFFFFF) | 0x40000000);

	// Alt çubuk arka planı
	float barH = 40.0f * u;
	DrawRect(dev, 0, m_h - barH, (float) m_w, barH, COL_BAR);
	DrawRect(dev, 0, m_h - barH, (float) m_w, 2.0f * u, COL_SLOT_RING);

	// Tuşlar
	for (const Button& b : m_buttons)
	{
		uint32_t col = b.down ? COL_DOWN : b.color;
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
	for (size_t i = 0; i < m_buttons.size(); ++i)
	{
		const Button& b = m_buttons[i];
		int height      = b.shape == Shape::Rect ? 9 : (b.r > 40.0f * u ? 12 : 9);
		DrawLabel(dev, (int) i, b.label, b.cx, b.cy, COL_TEXT, height);
		setup(); // DFont durumları değiştirir
	}
}
