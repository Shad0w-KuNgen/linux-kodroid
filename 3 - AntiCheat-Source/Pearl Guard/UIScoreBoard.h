#pragma once
class CUIScoreBoardPlug;
#include "stdafx.h"
#include "Pearl Engine.h"

class CUIScoreBoardPlug
{
public:
	DWORD m_dVTableAddr;
	CUIScoreBoardPlug();
	~CUIScoreBoardPlug();

	CTimer * m_Timer;

	DWORD base_clan;
	DWORD base_runa;
	DWORD base_time;

	DWORD Ename, Kname,EScore,KScore, RemTime;

	void ParseUIElements();
	void InitReceiveMessage();
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void Tick();
private:

};