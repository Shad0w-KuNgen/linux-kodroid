#pragma once

class CCollectionRace;

#include "stdafx.h"
#include "Pearl Engine.h"
#include "Timer.h"
#include "N3BASE/N3UIBase.h"
#include "N3BASE/N3UIString.h"
#include "N3BASE/N3UIButton.h"

class CCollectionRace : public CN3UIBase
{
	uint8 calibreCount;
	struct CollectionRaceEventData
	{
		uint32 nTargetID;
		uint16 targetCount;
		uint8 Nation;
		CN3UIString * txt_target;
		CN3UIString * txt_completion;
		CN3UIArea * area_needs;
		CN3UIButton* btn_needs;
		CN3UIImage* img_needs;

		CollectionRaceEventData()
		{
			nTargetID = 0;
			targetCount = 1;
			Nation = 0;
			txt_target = NULL;
			txt_completion = NULL;
			area_needs = NULL;
			btn_needs = NULL;
			img_needs = NULL;
		}
	};

	struct CRItemData
	{
		uint32 ItemID, ItemCount;
		TABLE_ITEM_BASIC* tbl;
		CN3UIString* txt_return_first;
		CN3UIString* txt_return_second;
		CN3UIArea* area_return;
		CN3UIButton* btn_return;
		CN3UIImage* img_return;

		CRItemData()
		{
			ItemID = ItemCount = 0;
			tbl = nullptr;
			txt_return_first = NULL;
			txt_return_second = NULL;
			area_return = NULL;
			btn_return = NULL;
			img_return = NULL;
		}
	};

	uint16 m_iRemainingTime, m_iCompleteUserCount, TotalParticipantUser;

	std::string EventName;

	CN3UIBase* group_top;
	CN3UIButton* btn_min;
	CN3UIButton* btn_max;
	CN3UIButton* btn_close;

	CN3UIBase* group_time;
	CN3UIString* text_winners;
	CN3UIString* text_time;
	CN3UIString* text_winners_label;
	CN3UIString* text_time_label;
	CN3UIString* text_event_name;

	CN3UIBase* requital[3];
	
	CN3UIBase* group[3];

	CN3UIBase* group_middle[2];

	CN3UIBase* base[3];
	CN3UIBase* items;

	CollectionRaceEventData CollectionRaceEvent[3];
	CRItemData m_Items[3];

	CTimer * m_Timer;
	bool m_bIsStarted;

public:
	CCollectionRace();
	~CCollectionRace();
	void Calibrate();
	bool Load(HANDLE hFile);
	void Tick();
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	bool OnKeyPress(int iKey);
	void Open();
	void Close();
	void Start();
	void Update();
	void UpdateFinishUserCounter();
	void LoadItems(uint32 Items[3], uint32 Counts[3]);
	void OpenCR();
	__IconItemSkill * GetItemStruct(CN3UIImage * img, std::string fileSz, __TABLE_ITEM_BASIC * tbl, uint32 realID);
};