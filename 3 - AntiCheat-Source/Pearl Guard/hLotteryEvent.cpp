#include "stdafx.h"
#include "hLotteryEvent.h"

CUILotteryEventPlug::CUILotteryEventPlug()
{
	vector<int>offsets;
	offsets.push_back(0x5A0);   // co_report_questionnaire Satýr : 276
	offsets.push_back(0);
	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);
	for (int i = 0; i < 9; i++)
		itemIDs[i] = 0;
	m_Timer = NULL;
	str_remaining = NULL;
	ticket_buy_count = NULL;
	str_participant = NULL;
	ParseUIElements();
	InitReceiveMessage();
}

CUILotteryEventPlug::~CUILotteryEventPlug()
{
}

void CUILotteryEventPlug::ParseUIElements()
{
	base_default = Engine->GetChildByID(m_dVTableAddr, "base_default");
	group_reqs = Engine->GetChildByID(m_dVTableAddr, "group_reqs");
	group_rewards = Engine->GetChildByID(m_dVTableAddr, "group_rewards");
	group_details = Engine->GetChildByID(m_dVTableAddr, "group_details");
	btn_close = Engine->GetChildByID(m_dVTableAddr, "btn_exit");
	ticket_buy_count = Engine->GetChildByID(group_reqs, "text_ticket_buy_count");
	reqSlots = Engine->GetChildByID(group_reqs, "slot1");

	for (int i = 0; i < 4; i++) {
		rewardSlots[i]=Engine->GetChildByID(group_rewards, string_format("icon%d", i + 1));
		Engine->SetVisible(rewardSlots[i], false);
	}

	str_participant = Engine->GetChildByID(group_details, "str_participant");
	str_ticket = Engine->GetChildByID(group_details, "str_ticket");
	str_remaining = Engine->GetChildByID(m_dVTableAddr, "str_remaining");
	btn_join = Engine->GetChildByID(group_reqs, "btn_join");
	for (int i = 0; i < 9; i++)
		itemTbl[i] = nullptr;

	Engine->UIScreenCenter(m_dVTableAddr);
}

void CUILotteryEventPlug::Update(uint32 reqs[5], uint32 reqsCount[5], uint32 rewards[4], bool reset)
{
	if (reset)
	{
		Engine->SetVisible(reqSlots, false);
		
		for (int i = 0; i < 5; i++) {
			itemIDs[i] = 0;
			itemTbl[i] = nullptr;
		}
	}

	for (int i = 0; i < 5; i++)
	{
		itemIDs[i] = reqs[i];
		if (reqs[i] == 0)
			continue;

		TABLE_ITEM_BASIC* tbl = Engine->tblMgr->getItemData(reqs[i]);
		if (!tbl)
		{
			itemTbl[i] = nullptr;
			continue;
		}

		auto pItemExt = Engine->tblMgr->getExtData(tbl->extNum, reqs[i]);
		if (TABLE_ITEM_EXT* ext = Engine->tblMgr->getExtData(tbl->extNum, rewards[i]))
		{
			DWORD dwIconID = tbl->dxtID;
			if (ext->dxtID > 0)
				dwIconID = ext->dxtID;

			std::string dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);

			Engine->SetTexImage(reqSlots,dxt);
		}
		else {
			std::vector<char> buffer(256, NULL);

			sprintf(&buffer[0], "ui\\itemicon_%.1d_%.4d_%.2d_%.1d.dxt",
				(tbl->dxtID / 10000000),
				(tbl->dxtID / 1000) % 10000,
				(tbl->dxtID / 10) % 100,
				tbl->dxtID % 10);

			std::string szIconFN = &buffer[0];


				Engine->SetTexImage(reqSlots, szIconFN);
		}
		Engine->SetVisible(reqSlots,true);
		itemTbl[i] = tbl;
	}

	for (int i = 0; i < 4; i++)
	{
		itemIDs[i + 5] = rewards[i];
		if (rewards[i] == 0)
			continue;

		TABLE_ITEM_BASIC* tbl = Engine->tblMgr->getItemData(rewards[i]);
		if (!tbl)
		{
			itemTbl[i + 5] = nullptr;
			continue;
		}


		if (TABLE_ITEM_EXT* ext = Engine->tblMgr->getExtData(tbl->extNum, rewards[i]))
		{
			DWORD dwIconID = tbl->dxtID;
			if (ext->dxtID > 0)
				dwIconID = ext->dxtID;

			std::string dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);

			Engine->SetTexImage(rewardSlots[i], dxt);
		
		}
		else {
			std::vector<char> buffer(256, NULL);

			sprintf(&buffer[0], "ui\\itemicon_%.1d_%.4d_%.2d_%.1d.dxt",
				(tbl->dxtID / 10000000),
				(tbl->dxtID / 1000) % 10000,
				(tbl->dxtID / 10) % 100,
				tbl->dxtID % 10);

			std::string szIconFN = &buffer[0];

	
			Engine->SetTexImage(rewardSlots[i], szIconFN);
		}

		Engine->SetVisible(rewardSlots[i],true);
		itemTbl[i + 5] = tbl;
	}
}

DWORD uiLottery;
DWORD Func_Lottery;

bool CUILotteryEventPlug::ReceiveMessage(DWORD * pSender, uint32_t dwMsg)
{
	uiLottery = m_dVTableAddr;

	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender ==(DWORD*) btn_close)
		{
			Close();

			if (Engine->uiTopRightNewPlug != NULL)
			{
				if (!Engine->IsVisible(Engine->uiTopRightNewPlug->m_lottery) && Engine->uiTopRightNewPlug->m_lottery)
				{
					Engine->SetVisible(Engine->uiTopRightNewPlug->m_lottery, true);
					Engine->SetState(Engine->uiTopRightNewPlug->m_lottery, UI_STATE_BUTTON_NORMAL);
				}
			}
		}
		else if (pSender == (DWORD*)btn_join)
		{
			Packet pkt(XSafe, uint8(XSafeOpCodes::LOTTERY));
			pkt << uint8(3);
			Engine->Send(&pkt);
		}
	}
	

	return true;
}

void CUILotteryEventPlug::Tick()
{
	if (m_Timer == NULL)
		m_Timer = new CTimer();

	if (Engine->lotteryEvent.remainingTime > 0) {
		if (m_Timer->IsElapsedSecond() && Engine->lotteryEvent.remainingTime > 0)
			Engine->lotteryEvent.remainingTime--;


		uint16_t remainingMinutes = (uint16_t)ceil(Engine->lotteryEvent.remainingTime / 60);
		uint16_t remainingSeconds = Engine->lotteryEvent.remainingTime - (remainingMinutes * 60);

		std::string remainingTime;
		if (remainingMinutes < 10 && remainingSeconds < 10)
			remainingTime = string_format(xorstr("0%d : 0%d"), remainingMinutes, remainingSeconds);
		else if (remainingMinutes < 10)
			remainingTime = string_format(xorstr("0%d : %d"), remainingMinutes, remainingSeconds);
		else if (remainingSeconds < 10)
			remainingTime = string_format(xorstr("%d : 0%d"), remainingMinutes, remainingSeconds);
		else
			remainingTime = string_format(xorstr("%d : %d"), remainingMinutes, remainingSeconds);

		Engine->SetString(str_remaining, string_format(xorstr("%s"), remainingTime.c_str()));
	}
}

void CUILotteryEventPlug::OpenLottery()
{
	Engine->SetVisible(m_dVTableAddr, true);
	Engine->UIScreenCenter(m_dVTableAddr);

}
uint32_t CUILotteryEventPlug::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (Engine->IsVisible(m_dVTableAddr))
	{
		bool showTooltip = false;

		for (int i = 0; i < 4; i++)
		{
			if (Engine->IsVisible(rewardSlots[i]) && Engine->IsIn(rewardSlots[i],ptCur.x, ptCur.y) && itemTbl[i+5]) {
				Engine->ShowToolTipEx(itemIDs[i + 5], ptCur.x, ptCur.y);
				showTooltip = true;
			}
		}
		if (!showTooltip)
		{
			if (Engine->IsVisible(reqSlots) && Engine->IsIn(reqSlots, ptCur.x, ptCur.y) && itemTbl[0]) {
				Engine->ShowToolTipEx(itemIDs[0], ptCur.x, ptCur.y);
				showTooltip = true;
			}
		}
	}

	return dwRet;
}

void CUILotteryEventPlug::Open()
{
	Engine->SetVisible(m_dVTableAddr,true);
}

void CUILotteryEventPlug::Close()
{
	Engine->SetVisible(m_dVTableAddr,false);
}
void __stdcall uiLotteryReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiLottery->ReceiveMessage(pSender, dwMsg);

	__asm
	{
		MOV ECX, uiLottery
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_Lottery
		CALL EAX
	}
}

void CUILotteryEventPlug::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_Lottery = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)uiLotteryReceiveMessage_Hook;
}