#pragma once
class HookPM;
#include "stdafx.h"
#include "Pearl Engine.h"

class HookPM
{
public:
	HookPM(DWORD base, bool bOpenWindow);
	~HookPM();
	void ParseUIElements();

	DWORD el_Base;
	DWORD exit_id;
	DWORD btn_close;
	DWORD str_msgcount;
	DWORD str_lastseen;
	string user_id;
	time_t m_LastTick;
	time_t m_LastSeenSend;
	bool m_bOpenWindow;
	bool m_canTick;
	bool m_bcolored;
	void Tick();
private:

};