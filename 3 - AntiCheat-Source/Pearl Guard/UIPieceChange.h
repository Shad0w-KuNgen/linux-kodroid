#pragma once

class CUIPieceChangePlug;

#include "stdafx.h"
#include "Pearl Engine.h"
#include "Timer.h"

class CUIPieceChangePlug
{
public:
	DWORD m_dVTableAddr;

	DWORD btn_bank, btnSell, btn_down, btn_up;
	bool  m_bank, m_sell;
	uint8 gemcount;
	
	DWORD m_btnStart;
	DWORD m_btnClose;
	DWORD m_btnStop;
	DWORD m_textExCount;

	DWORD m_slots[28];

	bool n_control;
	uint16 m_nObjectID;
	uint32 m_nExchangeItemID;
	int8_t m_nExchangeRobItemSlot;

public:
	CUIPieceChangePlug();
	~CUIPieceChangePlug();
	void ParseUIElements();
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void InitReceiveMessage();
	void ClickStop();
	void Tick();
};