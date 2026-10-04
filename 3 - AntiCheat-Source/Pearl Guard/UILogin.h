#pragma once
#include "stdafx.h"
class CUILogin;
#include "Pearl Engine.h"

class CUILogin
{
public:
	CUILogin();
	~CUILogin();

	void ParseUIElements();

	DWORD m_dVTableAddr;

	DWORD m_bGroupLogin;
	DWORD Group_ServerList_01;
	DWORD Group_introduction_ka;
	DWORD Group_introduction_el;
	DWORD premium;
	DWORD copyright;
	DWORD str_warning1;

	DWORD m_btnRememberID;
	DWORD m_btnLogin;

	DWORD m_EditUID;
	DWORD m_EditPW;

	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void InitReceiveMessage();

private:

};