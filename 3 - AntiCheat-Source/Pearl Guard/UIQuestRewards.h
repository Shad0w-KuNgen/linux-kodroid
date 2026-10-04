#pragma once
class CQuestRewards;
#include "stdafx.h"
#include "Pearl Engine.h"
#include "N3BASE/N3UIBase.h"
#include "N3BASE/N3UIString.h"
#include "N3BASE/N3UIButton.h"

class CQuestRewards : public CN3UIBase
{
	struct Reward
	{
		uint32 itemID;
		uint32 itemCount;
	};
	struct RewardBase
	{
		CN3UIBase* base;
		CN3UIString* str_name;
		CN3UIString* str_count;
		CN3UIArea* area;
		CN3UIIcon* icon;
		TABLE_ITEM_BASIC * tbl;
		uint32 nItemID;
		RewardBase()
		{
			base = NULL;
			str_count = NULL;
			str_name = NULL;
			area = NULL;
			icon = NULL;
			tbl = NULL;
			nItemID = NULL;
		}
	};

public:
	CN3UIButton* btn_close;
	CN3UIString* str_quest, * str_zone, * str_hunt, * str_hunt_count;
	CN3UIArea* hunt_area;

	RewardBase rewardBase[4];
	uint16 cMob;

public:
	CQuestRewards();
	~CQuestRewards();
	bool Load(HANDLE hFile);
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	bool OnKeyPress(int iKey);
	void Close();
	void Open();
	void LoadInfo(DailyQuest* quest);
	__IconItemSkill * GetItemStruct(CN3UIImage* img, std::string fileSz, __TABLE_ITEM_BASIC* tbl, uint32 realID);
	void OnKill(uint16 mobID);
};