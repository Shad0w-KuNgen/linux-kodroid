#pragma once
class CUICindirellaPlug;
#include "stdafx.h"
#include "Pearl Engine.h"
#include "Timer.h"
class CUICindirellaPlug
{
public:
	CTimer* m_Timer;
	DWORD m_dVTableAddr;
	bool status, prepare, ison;
	uint64 remainingTime,selectedtime;
	bool t_test;
	int8 index;
	DWORD txt_select_time, txt_remaining_time, txt_your_death_count, txt_your_kill_count, txt_elmo_kill_count;
	DWORD txt_karus_kill_count;
	DWORD btn_close, btn_priest, btn_mage, btn_rogue, btn_warrior;
	DWORD btn_elmo, btn_karus;
public:

	CUICindirellaPlug();
	~CUICindirellaPlug();
	void UpdateTime();
	void CharacterSelect(uint8 index);
	void ParseUIElements();
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void InitReceiveMessage();
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	bool OnKeyPress(int iKey);
	void Close();
	void Open();
	void Tick();
	void Reset();
	void Start();
	void ShowPanel();
	void UpdateKDA();
private:

};