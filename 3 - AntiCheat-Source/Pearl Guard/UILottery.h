#pragma once
class RimaLottery;
#include "stdafx.h"
#include "Pearl Engine.h"
#include "N3BASE/N3UIBase.h"
#include "N3BASE/N3UIString.h"
#include "N3BASE/N3UIButton.h"
#include "N3BASE\N3UIArea.h"

class RimaLottery : public CN3UIBase
{
	float fUVAspect = (float)45.0f / (float)64.0f;
public:
	CTimer * m_Timer;
	CN3UIBase* group_reqs, *group_rewards, *group_details;
	CN3UIArea* reqSlots[5], *rewardSlots[4];
	CN3UIString* str_remaining, *str_participant, *str_ticket, *str_change, *ticket_buy_count;
	CN3UIButton* btn_close, *btn_join;
	CN3UIIcon* itemIcons[9];

	__TABLE_ITEM_BASIC* itemTbl[9];

	RimaLottery();
	~RimaLottery();
	bool Load(HANDLE hFile);
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	bool OnKeyPress(int iKey);
	void Close();
	void Open();
	void Update(uint32 reqs[5],uint32 reqsCount[5], uint32 rewards[4], bool reset = false);
	void Tick();
	void OpenLottery();
	uint32 itemIDs[9];
	__IconItemSkill* GetItemStruct(CN3UIImage* img, std::string fileSz, __TABLE_ITEM_BASIC* tbl, uint32 realID);
};