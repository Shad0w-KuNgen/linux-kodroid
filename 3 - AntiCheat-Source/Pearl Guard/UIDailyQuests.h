#pragma once
class HookQuestPage;
#include "stdafx.h"
#include "Pearl Engine.h"

#define _COUNT 15

class HookQuestPage
{
	struct Reward
	{
		uint32 itemID;
		uint32 itemCount;
	};
	struct RewardBase
	{
		DWORD base;
		DWORD str_name;
		DWORD str_count;
		DWORD str_btn;
		DWORD icon;
		TABLE_ITEM_BASIC* tbl;
		uint32 nItemID;
		RewardBase()
		{
			base = NULL;
			str_count = NULL;
			str_name = NULL;
	
			icon = NULL;
			tbl = NULL;
			nItemID = NULL;
		}
	};

public:
	DWORD btn_close;
	DWORD str_quest, str_zone,  str_hunt,  str_hunt_count;
	DWORD hunt_area;
	DWORD hunt;

	RewardBase rewardBase[4];
	uint16 cMob;

	uint8 page;
	uint8 pagecountpageCount;
	int pageCount;
	DWORD m_dVTableAddr;
	DWORD group_quest;
	DWORD quest_rewards;
	DWORD m_btnOk;
	DWORD g_close;
	DWORD m_btnReview;
	DWORD m_btnCancel;
	DWORD text_page;
	DWORD btn_up;
	DWORD btn_down;
	uint16 selectQuestID;
	DWORD m_txtUprate;
	CTimer* m_Timer;
	DWORD slots[28];
	DWORD baseQuestButton[_COUNT];
	DWORD txt_quest_time[_COUNT];
	DWORD txt_quest_status[_COUNT];
	DWORD txt_quest_title[_COUNT];
	DWORD baseGroup[_COUNT];

	DWORD txt_quest_zone[_COUNT];
	DWORD txt_quest_killtype[_COUNT];
public:
	HookQuestPage();
	~HookQuestPage();
	vector<DailyQuest*> kcbq_quests;
	vector<DailyQuest*> filteredList;
	void InitQuests(uint8 p = 1);
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	void KillTrigger(uint8 questID, uint16 mob);
	void ParseUIElements();
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void InitReceiveMessage();
	void Tick();
	bool IsInSlots();
	void LoadInfo(DailyQuest* quest);
private:

};