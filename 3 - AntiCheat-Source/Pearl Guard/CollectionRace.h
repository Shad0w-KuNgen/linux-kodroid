#pragma once
#include "stdafx.h"
class CUICollectionRacePlug;
#include "Pearl Engine.h"
#include "Timer.h"
class CUICollectionRacePlug
{
	uint8 calibreCount;
	struct CollectionRaceEventData
	{
		uint32 nTargetID;
		uint16 targetCount;
		uint8 Nation;
		DWORD txt_target;
		DWORD bases;
		DWORD txt_completion;
		DWORD area_needs;
		DWORD btn_needs;
		DWORD img_needs;

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
		uint8  nRate;
		TABLE_ITEM_BASIC* tbl;
		DWORD txt_return_first;
		DWORD txt_return_second;
		DWORD txt_rate;
		DWORD area_return;
		DWORD btn_return;
		DWORD img_return;

		CRItemData()
		{
			ItemID = ItemCount = 0;
			tbl = nullptr;
			txt_return_first = NULL;
			txt_return_second = NULL;
			area_return = NULL;
			btn_return = NULL;
			img_return = NULL;
			txt_rate = NULL;
			nRate = 0;
		}
	};

	uint16 m_iRemainingTime, m_iCompleteUserCount, TotalParticipantUser;

	std::string EventName;
	bool minimaliaze;
	DWORD group_top;
	DWORD btn_min;
	DWORD btn_max;
	DWORD btn_close;
	DWORD bases;
	DWORD text_event_zone;
	DWORD group_time;
	DWORD text_winners;
	DWORD text_time;
	DWORD text_winners_label;
	DWORD text_time_label;
	DWORD text_event_name;
	DWORD requital[3];
	DWORD group[3];
	DWORD group_middle[2];
	DWORD base[3];
	DWORD items;
	CollectionRaceEventData CollectionRaceEvent[3];
	CRItemData m_Items[3];
	CTimer* m_Timer;
	bool m_bIsStarted;
public:


	CUICollectionRacePlug();
	~CUICollectionRacePlug();

	void ParseUIElements();
	void InitReceiveMessage();


	DWORD m_dVTableAddr;
	void Open();
	void setMin();
	void setMax();
	void Close();
	void Start();
	void Update();
	void UpdateFinishUserCounter();
	void LoadItems(uint32 Items[3], uint32 Counts[3], uint8 nRate[3]);
	void OpenCR();
	void Calibrate();

	void Tick();

	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);

};