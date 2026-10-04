#pragma once
class CUILotteryEventPlug;
#include "stdafx.h"
#include "Pearl Engine.h"
#include "Timer.h"
class CUILotteryEventPlug
{
public:
	CTimer* m_Timer;
	DWORD m_dVTableAddr;
	
	DWORD base_default;
	DWORD group_reqs,  group_rewards, group_details;
	DWORD reqSlots,  rewardSlots[4];
	DWORD str_remaining,  str_participant,  str_ticket,  str_change,  ticket_buy_count;
	DWORD btn_close,  btn_join;
	DWORD itemIcons[4];
	DWORD nReqItem;
	__TABLE_ITEM_BASIC* itemTbl[9];

public:

	CUILotteryEventPlug();
	~CUILotteryEventPlug();
	void ParseUIElements();
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void InitReceiveMessage();
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	bool OnKeyPress(int iKey);
	void Close();
	void Open();
	void Update(uint32 reqs[5], uint32 reqsCount[5], uint32 rewards[4], bool reset = false);
	void Tick();
	void OpenLottery();
	uint32 itemIDs[9];

private:

};