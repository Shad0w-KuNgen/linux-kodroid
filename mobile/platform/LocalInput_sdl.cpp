// LocalInput_sdl.cpp — CLocalInput'un DirectInput yerine SDL/dokunmatik ile uygulanması.
// Fare bayrağı mantığı (tıklama/bırakma/çift tık/sürükleme) orijinal LocalInput.cpp ile aynıdır.
#include "StdAfx.h"
#include "LocalInput.h"
#include "GameProcMain.h"
#include "GameEng.h"

#include "KoPlatformInput.h"

CLocalInput::CLocalInput()
{
	m_lpDI          = nullptr;
	m_lpDIDKeyboard = nullptr;
	m_hWnd          = nullptr;
	m_bNoKeyDown    = FALSE;
	m_nMouseFlag    = 0;
	m_nMouseFlagOld = 0;
	m_dwTickLBDown  = 0;
	m_dwTickRBDown  = 0;
	m_ptCurMouse.x = m_ptCurMouse.y = 0;
	m_ptOldMouse.x = m_ptOldMouse.y = 0;
	SetRect(&m_rcLBDrag, 0, 0, 0, 0);
	SetRect(&m_rcMBDrag, 0, 0, 0, 0);
	SetRect(&m_rcRBDrag, 0, 0, 0, 0);
	memset(m_byCurKeys, 0, sizeof(m_byCurKeys));
	memset(m_byOldKeys, 0, sizeof(m_byOldKeys));
	memset(m_bKeyPresses, 0, sizeof(m_bKeyPresses));
	memset(m_bKeyPresseds, 0, sizeof(m_bKeyPresseds));
	memset(m_dwTickKeyPress, 0, sizeof(m_dwTickKeyPress));
}

CLocalInput::~CLocalInput() = default;

BOOL CLocalInput::Init(HINSTANCE, HWND hWnd)
{
	m_hWnd = hWnd;
	KeyboardFlushData();
	return TRUE;
}

void CLocalInput::SetActiveDevices(BOOL bKeyboard)
{
	if (!bKeyboard)
		KeyboardFlushData();
}

void CLocalInput::KeyboardFlushData()
{
	memset(m_byOldKeys, 0, NUMDIKEYS);
	memset(m_byCurKeys, 0, NUMDIKEYS);
}

void CLocalInput::MouseSetPos(int x, int y)
{
	m_ptCurMouse.x = x;
	m_ptCurMouse.y = y;
	KoInput().mouseX = x;
	KoInput().mouseY = y;
}

BOOL CLocalInput::KeyboardGetKeyState(int nDIKey)
{
	if (nDIKey < 0 || nDIKey >= NUMDIKEYS)
		return FALSE;
	return m_byCurKeys[nDIKey];
}

void CLocalInput::AcquireKeyboard()
{
	KeyboardFlushData();
}

void CLocalInput::UnacquireKeyboard()
{
	KeyboardFlushData();
}

void CLocalInput::Tick()
{
	KoInputState& in = KoInput();
	if (!in.windowFocused)
	{
		// Kenar bayrakları (CLICK/CLICKED) tekrar etmesin
		m_nMouseFlagOld = m_nMouseFlag;
		m_nMouseFlag &= (MOUSE_LBDOWN | MOUSE_MBDOWN | MOUSE_RBDOWN);
		return;
	}

	// Dokunmatik tık kuyruğu: önce önceki tıkın bırakma karesi, sonra sıradaki tıkın basma karesi
	if (in.pendingLbUpFrames > 0)
	{
		if (--in.pendingLbUpFrames == 0)
		{
			in.lbDown = false;
			in.rbDown = false;
		}
	}
	else if (!in.lbDown && !in.rbDown && !in.taps.empty())
	{
		KoInputState::Tap t = in.taps.front();
		in.taps.pop_front();
		in.mouseX = t.x;
		in.mouseY = t.y;
		if (t.right)
			in.rbDown = true;
		else
			in.lbDown = true;
		in.pendingLbUpFrames = 1; // bir sonraki karede bırakılır → LBCLICKED
	}

	// KLAVYE
	KoInputUpdateKeyboard();
	memcpy(m_byOldKeys, m_byCurKeys, NUMDIKEYS);
	memcpy(m_byCurKeys, in.keysDIK, NUMDIKEYS);
	m_bNoKeyDown = TRUE;
	for (int i = 0; i < NUMDIKEYS; i++)
	{
		m_bKeyPresses[i]  = (!m_byOldKeys[i] && m_byCurKeys[i]) ? TRUE : FALSE;
		m_bKeyPresseds[i] = (m_byOldKeys[i] && !m_byCurKeys[i]) ? TRUE : FALSE;
		if (m_byCurKeys[i])
			m_bNoKeyDown = FALSE;
	}

	// FARE / DOKUNMATİK
	m_ptOldMouse   = m_ptCurMouse;
	m_ptCurMouse.x = in.mouseX;
	m_ptCurMouse.y = in.mouseY;

	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	if (!PtInRect(&rcClient, m_ptCurMouse))
	{
		m_nMouseFlagOld = m_nMouseFlag;
		m_nMouseFlag &= (MOUSE_LBDOWN | MOUSE_MBDOWN | MOUSE_RBDOWN);
		return;
	}

	m_nMouseFlagOld = m_nMouseFlag;
	m_nMouseFlag    = 0;
	if (in.lbDown) m_nMouseFlag |= MOUSE_LBDOWN;
	if (in.mbDown) m_nMouseFlag |= MOUSE_MBDOWN;
	if (in.rbDown) m_nMouseFlag |= MOUSE_RBDOWN;

	if (!(m_nMouseFlagOld & MOUSE_LBDOWN) && (m_nMouseFlag & MOUSE_LBDOWN)) m_nMouseFlag |= MOUSE_LBCLICK;
	if (!(m_nMouseFlagOld & MOUSE_MBDOWN) && (m_nMouseFlag & MOUSE_MBDOWN)) m_nMouseFlag |= MOUSE_MBCLICK;
	if (!(m_nMouseFlagOld & MOUSE_RBDOWN) && (m_nMouseFlag & MOUSE_RBDOWN)) m_nMouseFlag |= MOUSE_RBCLICK;
	if ((m_nMouseFlagOld & MOUSE_LBDOWN) && !(m_nMouseFlag & MOUSE_LBDOWN)) m_nMouseFlag |= MOUSE_LBCLICKED;
	if ((m_nMouseFlagOld & MOUSE_MBDOWN) && !(m_nMouseFlag & MOUSE_MBDOWN)) m_nMouseFlag |= MOUSE_MBCLICKED;
	if ((m_nMouseFlagOld & MOUSE_RBDOWN) && !(m_nMouseFlag & MOUSE_RBDOWN)) m_nMouseFlag |= MOUSE_RBCLICKED;

	static DWORD dwDblClk = GetDoubleClickTime();
	if (m_nMouseFlag & MOUSE_LBCLICKED)
	{
		static DWORD dwClicked = 0;
		if (timeGetTime() < dwClicked + dwDblClk)
			m_nMouseFlag |= MOUSE_LBDBLCLK;
		dwClicked = timeGetTime();
	}
	if (m_nMouseFlag & MOUSE_MBCLICKED)
	{
		static DWORD dwClicked = 0;
		if (timeGetTime() < dwClicked + dwDblClk)
			m_nMouseFlag |= MOUSE_MBDBLCLK;
		dwClicked = timeGetTime();
	}
	if (m_nMouseFlag & MOUSE_RBCLICKED)
	{
		static DWORD dwClicked = 0;
		if (timeGetTime() < dwClicked + dwDblClk)
			m_nMouseFlag |= MOUSE_RBDBLCLK;
		dwClicked = timeGetTime();
	}

	if (m_nMouseFlag & MOUSE_LBDOWN) { m_rcLBDrag.right = m_ptCurMouse.x; m_rcLBDrag.bottom = m_ptCurMouse.y; }
	if (m_nMouseFlag & MOUSE_MBDOWN) { m_rcMBDrag.right = m_ptCurMouse.x; m_rcMBDrag.bottom = m_ptCurMouse.y; }
	if (m_nMouseFlag & MOUSE_RBDOWN) { m_rcRBDrag.right = m_ptCurMouse.x; m_rcRBDrag.bottom = m_ptCurMouse.y; }
	if (m_nMouseFlag & MOUSE_LBCLICK) { m_rcLBDrag.left = m_ptCurMouse.x; m_rcLBDrag.top = m_ptCurMouse.y; }
	if (m_nMouseFlag & MOUSE_MBCLICK) { m_rcMBDrag.left = m_ptCurMouse.x; m_rcMBDrag.top = m_ptCurMouse.y; }
	if (m_nMouseFlag & MOUSE_RBCLICK) { m_rcRBDrag.left = m_ptCurMouse.x; m_rcRBDrag.top = m_ptCurMouse.y; }
}
