#pragma once
class CUITaskbarSubPlug;
#include "stdafx.h"
#include "Pearl Engine.h"

class CUITaskbarSubPlug
{
public:
	DWORD m_dVTableAddr;
	DWORD m_btnQuestion;
	DWORD m_btnPowerUPStore;
	DWORD m_btnHotkeyH;
	DWORD m_btnGlobalMap;
	DWORD m_btnInviteParty;
	DWORD m_btnExitGame;
	DWORD m_mobdrop;
	DWORD btn_wof;
	DWORD m_dailyquest;
	DWORD m_btnbug;
public:
	CUITaskbarSubPlug();
	~CUITaskbarSubPlug();
	void ParseUIElements();
	bool ReceiveMessage(DWORD * pSender, uint32_t dwMsg);
	void InitReceiveMessage();
	void OpenGlobalMap();
	void OpenInviteParty();
	void OpenExitGame();
	void OpenHotkeyH();
	void OpenSeedHelper();
	void OpenDailyQuest();
	void OpenMobSearch();
	void OpenSupportReport();
private:

};
