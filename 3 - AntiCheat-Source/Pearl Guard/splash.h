#pragma once

#include <windows.h>
#include <stdio.h>
#include "resource.h"

class SPLASH
{
private:
	HWND m_hWnd;
	HWND m_hWndParent;

public:
	BOOL m_bShowing;

	SPLASH()
	{
		m_hWnd = m_hWndParent = NULL;
		m_bShowing = FALSE;
	}

	~SPLASH()
	{
		Hide();
		DestroyWindow(m_hWnd);
		m_hWnd = NULL;
	}

	BOOL Init(HWND hWndParent, HINSTANCE hInst, int nResID)
	{
		RECT desktop;
		const HWND hDesktop = GetDesktopWindow();
		GetWindowRect(hDesktop, &desktop);

		m_hWndParent = hWndParent;
		m_hWnd = CreateWindowEx(WS_EX_WINDOWEDGE, "STATIC", NULL,
			WS_POPUP | WS_DLGFRAME | SS_BITMAP, desktop.right/2/*-201*/, desktop.bottom/2/*-347*/, 195, 300, m_hWndParent, NULL, hInst, NULL);
		
		if (m_hWnd) {
			SendMessage(m_hWnd, STM_SETIMAGE, IMAGE_BITMAP,
				(LPARAM)LoadBitmap(hInst, MAKEINTRESOURCE(nResID)));
			return TRUE;
		}
		return FALSE;
	}

	void Show()
	{
		RECT rcThis, rcParent;
		POINT pt;
		if (GetWindowRect(m_hWnd, &rcThis) && GetWindowRect(m_hWndParent, &rcParent)) {
			pt.x = (((rcParent.right - rcParent.left) - (rcThis.right - rcThis.left)) / 2);
			pt.y = (((rcParent.bottom - rcParent.top) - (rcThis.bottom - rcThis.top)) / 2);
			SetWindowPos(m_hWnd, HWND_TOPMOST, pt.x, pt.y, 0, 0, SWP_NOSIZE);
		}
		
		ShowWindow(m_hWnd, SW_SHOWNORMAL);
		UpdateWindow(m_hWnd);
		m_bShowing = TRUE;
	}

	void Hide()
	{
		ShowWindow(m_hWnd, SW_HIDE);
		m_bShowing = FALSE;
	}

};