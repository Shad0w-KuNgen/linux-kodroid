#include "CollectionRace.h"
extern std::string GetZoneName(uint16 zoneID);
CUICollectionRacePlug::CUICollectionRacePlug()
{
	m_dVTableAddr = NULL;
	minimaliaze = false;
	vector<int>offsets;
	offsets.push_back(0x558);  // co_battle_score Satýr : 250
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);
	m_Timer = NULL;
	m_iRemainingTime = m_iCompleteUserCount = TotalParticipantUser = 0;
	EventName = "";
	group_top = NULL;
	btn_min = NULL;
	btn_max = NULL;
	btn_close = NULL;
	calibreCount = 0;
	text_event_zone = NULL;
	group_time = NULL;
	text_winners = text_time = text_winners_label = text_time_label = NULL;
	text_event_name = NULL;
	for (int32_t i = 0; i < 3; i++)
	{
		requital[i] = NULL;
		group[i] = NULL;
	}

	for (int i = 0; i < 2; i++)
		group_middle[i] = NULL;

	m_bIsStarted = false;
	ParseUIElements();
	InitReceiveMessage();
}

DWORD Func_cr;
CUICollectionRacePlug::~CUICollectionRacePlug()
{

}

void CUICollectionRacePlug::ParseUIElements()
{
	group_top = Engine->GetChildByID(m_dVTableAddr, "group_top");
	btn_min = Engine->GetChildByID(group_top, "btn_min");
	btn_max = Engine->GetChildByID(group_top, "btn_max");
	btn_close = Engine->GetChildByID(group_top, "btn_close");

	group_time = Engine->GetChildByID(m_dVTableAddr, "group_time");
	text_winners_label = Engine->GetChildByID(group_time, "text_winners_label");
	text_time_label = Engine->GetChildByID(group_time, "text_time_label");
	text_time = Engine->GetChildByID(group_time, "text_time");
	text_event_name = Engine->GetChildByID(group_time, "text_event_name");
	text_event_zone = Engine->GetChildByID(group_time, "text_event_zone");
	text_winners = Engine->GetChildByID(group_time, "text_winners");

	group_middle[0] = Engine->GetChildByID(m_dVTableAddr, "group_middle_0");
	items = Engine->GetChildByID(m_dVTableAddr, "items");
	for (int i = 0; i < 3; i++)
	{
		requital[i] = Engine->GetChildByID(items, string_format(xorstr("requital_%d"), i));
		m_Items[i].txt_return_first = Engine->GetChildByID(requital[i], "txt_return_first");
		m_Items[i].txt_return_second = Engine->GetChildByID(requital[i], "txt_return_second");
		m_Items[i].txt_rate = Engine->GetChildByID(requital[i], "txt_rate");
		m_Items[i].area_return = Engine->GetChildByID(requital[i], "icon");
		m_Items[i].btn_return = Engine->GetChildByID(requital[i], "btn_return");
		group[i] = Engine->GetChildByID(m_dVTableAddr, string_format(xorstr("group_%d"), i));
		CollectionRaceEvent[i].txt_completion = Engine->GetChildByID(group[i], "txt_needs_first");
		CollectionRaceEvent[i].txt_target = Engine->GetChildByID(group[i], "txt_needs_second");
		CollectionRaceEvent[i].area_needs = Engine->GetChildByID(group[i], "icon");
		CollectionRaceEvent[i].btn_needs = Engine->GetChildByID(group[i], "btn_needs");
	}

	group_middle[1] = Engine->GetChildByID(items, "group_middle_1");
	bases = Engine->GetChildByID(m_dVTableAddr, "bases");
	for (uint8 i = 0; i < 3; i++)
	{
		base[i] = Engine->GetChildByID(bases, string_format(xorstr("base_%d"), i + 1));
	}
}

DWORD CollectionRace;

void CUICollectionRacePlug::setMin() {
	Engine->SetVisible(btn_min, false);
	Engine->SetVisible(btn_max, true);

	RECT a = Engine->GetUiRegion(group_top);
	RECT b = Engine->GetUiRegion(group_time);
	a.bottom = b.bottom;
	Engine->SetUiRegion(m_dVTableAddr, a);

	for (int i = 0; i < 3; i++)
	{
		Engine->SetVisible(requital[i], false);
		Engine->SetVisible(group[i], false);
		Engine->SetVisible(CollectionRaceEvent[i].img_needs, false);
		Engine->SetVisible(m_Items[i].img_return, false);
	}

	for (int i = 0; i < 2; i++)
		Engine->SetVisible(group_middle[i], false);
}

void CUICollectionRacePlug::setMax() {
	Engine->SetVisible(btn_min, true);
	Engine->SetVisible(btn_max, false);

	RECT a = Engine->GetUiRegion(group_top);
	RECT b = Engine->GetUiRegion(group_time);
	a.bottom = b.bottom;
	Engine->SetUiRegion(m_dVTableAddr, a);

	for (int i = 0; i < calibreCount + 1; i++)
	{
		if (i > 2)
			continue;

		//Engine->SetVisible(requital[i], true);
		Engine->SetVisible(group[i], true);
		Engine->SetVisible(CollectionRaceEvent[i].img_needs, true);
		Engine->SetVisible(m_Items[i].img_return, true);
	}

	for (int i = 0; i < 3; i++)
	{
		if (m_Items[i].ItemID > 0)
		{
			Engine->SetVisible(requital[i], true);
			Engine->SetVisible(m_Items[i].img_return, true);
		}
	}

	for (int i = 0; i < 2; i++)
		Engine->SetVisible(group_middle[i], true);
}

void CUICollectionRacePlug::Open()
{
	Engine->SetVisible(m_dVTableAddr, true);
}

bool CUICollectionRacePlug::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	CollectionRace = m_dVTableAddr;
	if (!pSender || pSender == (DWORD*)0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	if (pSender == (DWORD*)btn_min)
	{
		minimaliaze = true;
		setMin();
	}
	else if (pSender == (DWORD*)btn_max)
	{
		minimaliaze = false;
		setMax();
		Calibrate();
	}
	else if (pSender == (DWORD*)btn_close)
	{
		Close();

		if (Engine->uiTopRightNewPlug != NULL)
			Engine->uiTopRightNewPlug->OpenCollectionRaceButton();
	}
	else {

		for (int i = 0; i < 3; i++)
		{
			if (pSender == (DWORD*)m_Items[i].btn_return)
			{
				if (m_Items[i].ItemID == 900004000)
				{
					Packet pkt(XSafe, uint8(XSafeOpCodes::DROP_REQUEST));
					pkt << uint8(4) << uint8(i);
					Engine->Send(&pkt);
				}

			}
		}
	}

	return true;
}

void __stdcall CollectionRaceReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiCollection->ReceiveMessage(pSender, dwMsg);
	__asm
	{
		MOV ECX, CollectionRace
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_cr
		CALL EAX
	}
}

void CUICollectionRacePlug::Tick()
{
	if (Engine->IsCRActive)
	{
		//Engine->SetVisible(m_dVTableAddr, m_iRemainingTime > 0);
		if (m_Timer->IsElapsedSecond())
			m_iRemainingTime--;

		uint16_t remainingMinutes = (uint16_t)ceil(m_iRemainingTime / 60);
		uint16_t remainingSeconds = m_iRemainingTime - (remainingMinutes * 60);

		std::string remainingTime;
		if (remainingMinutes < 10 && remainingSeconds < 10)
			remainingTime = string_format(xorstr("0%d : 0%d"), remainingMinutes, remainingSeconds);
		else if (remainingMinutes < 10)
			remainingTime = string_format(xorstr("0%d : %d"), remainingMinutes, remainingSeconds);
		else if (remainingSeconds < 10)
			remainingTime = string_format(xorstr("%d : 0%d"), remainingMinutes, remainingSeconds);
		else
			remainingTime = string_format(xorstr("%d : %d"), remainingMinutes, remainingSeconds);

		Engine->SetString(text_time, string_format(xorstr("%s"), remainingTime.c_str()));
	}
}

void CUICollectionRacePlug::Start()
{
	minimaliaze = false;
	uint32_t OriginItemID = 0;

	m_iRemainingTime = Engine->pCollectionRaceEvent.EventTimer;
	m_iCompleteUserCount = Engine->pCollectionRaceEvent.m_iCompleteUserCount;
	TotalParticipantUser = Engine->pCollectionRaceEvent.TotalParticipantUser;
	EventName = Engine->pCollectionRaceEvent.EventName;

	Engine->SetString(text_winners, string_format("%d of %d", m_iCompleteUserCount, TotalParticipantUser));
	Engine->SetString(text_event_name, string_format("%s", EventName.c_str()));
	Engine->SetString(text_event_zone, string_format("Zone : %s", GetZoneName(Engine->pCollectionRaceEvent.sZone).c_str()));

	uint8 baseCount = 0;

	for (int i = 0; i < 3; i++)
	{
		CollectionRaceEvent[i].nTargetID = Engine->pCollectionRaceEvent.sSid[i];
		CollectionRaceEvent[i].targetCount = Engine->pCollectionRaceEvent.killTarget[i];
		CollectionRaceEvent[i].Nation = Engine->pCollectionRaceEvent.Nation;

		TABLE_MOB* mobData = Engine->tblMgr->getMobData(CollectionRaceEvent[i].nTargetID);
		if (mobData != nullptr)
		{
			Engine->SetString(CollectionRaceEvent[i].txt_completion, string_format(xorstr("Hunt - 0 / %d"), CollectionRaceEvent[i].targetCount));
			Engine->SetString(CollectionRaceEvent[i].txt_target, mobData->strName);
			Engine->SetState(CollectionRaceEvent[i].btn_needs, eUI_STATE::UI_STATE_BUTTON_ON);

			OriginItemID = 900005000;

			TABLE_ITEM_BASIC* tbl = Engine->tblMgr->getItemData(OriginItemID);
			if (TABLE_ITEM_EXT* ext = Engine->tblMgr->getExtData(tbl->extNum, OriginItemID))
			{
				DWORD dwIconID = tbl->dxtID;
				if (ext->dxtID > 0)
					dwIconID = ext->dxtID;

				std::string dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);

				Engine->SetTexImage(CollectionRaceEvent[i].area_needs, dxt);
				baseCount++;
			}

			else {
				std::vector<char> buffer(256, NULL);

				sprintf(&buffer[0], "ui\\itemicon_%.1d_%.4d_%.2d_%.1d.dxt",
					(tbl->dxtID / 10000000),
					(tbl->dxtID / 1000) % 10000,
					(tbl->dxtID / 10) % 100,
					tbl->dxtID % 10);

				std::string szIconFN = &buffer[0];


				Engine->SetTexImage(CollectionRaceEvent[i].area_needs, szIconFN);
				baseCount++;
			}

		}
		else if (CollectionRaceEvent[i].nTargetID == 1 && CollectionRaceEvent[i].Nation != 0)
		{
			OriginItemID = 900008000;
			TABLE_ITEM_BASIC* tbl = Engine->tblMgr->getItemData(OriginItemID);
			if (TABLE_ITEM_EXT* ext = Engine->tblMgr->getExtData(tbl->extNum, OriginItemID))
			{
				DWORD dwIconID = tbl->dxtID;
				if (ext->dxtID > 0)
					dwIconID = ext->dxtID;

				std::string dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);


				Engine->SetTexImage(CollectionRaceEvent[i].area_needs, dxt);
				baseCount++;
			}
			else {
				std::vector<char> buffer(256, NULL);

				sprintf(&buffer[0], "ui\\itemicon_%.1d_%.4d_%.2d_%.1d.dxt",
					(tbl->dxtID / 10000000),
					(tbl->dxtID / 1000) % 10000,
					(tbl->dxtID / 10) % 100,
					tbl->dxtID % 10);

				std::string szIconFN = &buffer[0];

				Engine->SetTexImage(CollectionRaceEvent[i].area_needs, szIconFN);

				baseCount++;
			}

			Engine->SetString(CollectionRaceEvent[i].txt_completion, string_format(xorstr("0 / %d"), CollectionRaceEvent[i].targetCount));
			Engine->SetString(CollectionRaceEvent[i].txt_target, CollectionRaceEvent[i].Nation == 1 ? xorstr("<El Morad>") : xorstr("<Karus>"));
			Engine->SetState(CollectionRaceEvent[i].btn_needs, eUI_STATE::UI_STATE_BUTTON_ON);

		}
		else
		{
			Engine->SetString(CollectionRaceEvent[i].txt_completion, string_format(xorstr(""), CollectionRaceEvent[i].targetCount));
			Engine->SetString(CollectionRaceEvent[i].txt_target, xorstr(""));
		}
	}

	LoadItems(Engine->pCollectionRaceEvent.RewardsItemID,
		Engine->pCollectionRaceEvent.RewardsItemCount, Engine->pCollectionRaceEvent.RewardsItemRate);

	calibreCount = baseCount;

	Calibrate();

	m_Timer = new CTimer(false);
	Engine->SetVisible(m_dVTableAddr, true);
	
	if(Engine->uiTopRightNewPlug)
		Engine->uiTopRightNewPlug->OpenCollectionRaceButton();

	if (minimaliaze)
		setMin();
	else
	{
		setMax();
		Calibrate();
	}
}

uint32_t CUICollectionRacePlug::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	bool showTooltip = false;

	for (auto it : m_Items)
	{
		if (Engine->IsVisible(btn_min) && Engine->IsIn(it.img_return, ptCur.x, ptCur.y) && it.tbl != nullptr)
		{
			Engine->ShowToolTipEx(it.ItemID, ptCur.x, ptCur.y);
			break;
		}
	}

	return dwRet;
}

void CUICollectionRacePlug::Calibrate()
{
	POINT ret = Engine->GetUiPos(base[calibreCount - 1]);
	Engine->SetUIPos(items, ret);
	//items->SetPos(base[calibreCount - 1]->GetPos().x, base[calibreCount - 1]->GetPos().y);

	for (uint8 i = 0; i < 3; i++)
		Engine->SetVisible(group[i], false);

	for (uint8 i = 0; i < calibreCount; i++)
		Engine->SetVisible(group[i], true);

	for (uint8 i = calibreCount; i < 3; i++)
		if (i < 3) 	Engine->SetVisible(group[i], false);

	RECT minReg = Engine->GetUiRegion(group_top);
	RECT group_times = Engine->GetUiRegion(group_time);
	minReg.bottom += group_times.bottom - group_times.top;
	RECT baseSize = Engine->GetUiRegion(group[0]);
	RECT itemSize = Engine->GetUiRegion(items);
	minReg.bottom += itemSize.bottom - itemSize.top;
	minReg.bottom += calibreCount * (baseSize.bottom - baseSize.top);
	Engine->SetUiRegion(m_dVTableAddr, minReg);
}

void CUICollectionRacePlug::Update()
{
	int completionCount = 0;
	for (int i = 0; i < 3; i++)
	{
		TABLE_MOB* mobData = Engine->tblMgr->getMobData(CollectionRaceEvent[i].nTargetID);
		if (mobData != nullptr)
			Engine->SetString(CollectionRaceEvent[i].txt_completion, string_format(xorstr("Hunt - %d / %d"), Engine->pCollectionRaceEvent.killCount[i], CollectionRaceEvent[i].targetCount));
		else if (CollectionRaceEvent[i].nTargetID == 1 && CollectionRaceEvent[i].Nation != 0)
		{
			Engine->SetString(CollectionRaceEvent[i].txt_completion, string_format(xorstr("%d / %d"), Engine->pCollectionRaceEvent.killCount[i], CollectionRaceEvent[i].targetCount));
			Engine->SetString(CollectionRaceEvent[i].txt_target, CollectionRaceEvent[i].Nation == 1 ? xorstr("<El Morad>") : xorstr("<Karus>"));
		}

		if (Engine->pCollectionRaceEvent.killCount[i] >= CollectionRaceEvent[i].targetCount)
		{
			Engine->SetStringColor(CollectionRaceEvent[i].txt_completion, D3DCOLOR_RGBA(255, 99, 71, 255));
			completionCount++;
		}
	}

	//if (completionCount >= 3)
	//	Close();
}

void CUICollectionRacePlug::UpdateFinishUserCounter()
{
	m_iCompleteUserCount = Engine->pCollectionRaceEvent.m_iCompleteUserCount;
	Engine->SetString(text_winners, string_format(xorstr("%d of %d"), m_iCompleteUserCount, TotalParticipantUser));
}

void CUICollectionRacePlug::LoadItems(uint32 Items[3], uint32 Counts[3], uint8 nRate[3])
{
	for (int i = 0; i < 3; i++)
	{
		m_Items[i].ItemID = Items[i];

		TABLE_ITEM_BASIC* tbl = Engine->tblMgr->getItemData(Items[i]);
		if (tbl != nullptr)
		{
			m_Items[i].tbl = tbl;

			Engine->SetString(m_Items[i].txt_return_first, tbl->strName);
			Engine->SetString(m_Items[i].txt_return_second, to_string(Counts[i]));
			Engine->SetString(m_Items[i].txt_rate, "%" + to_string(nRate[i]));

			if (TABLE_ITEM_EXT* ext = Engine->tblMgr->getExtData(tbl->extNum, Items[i]))
			{
				DWORD dwIconID = tbl->dxtID;
				if (ext->dxtID > 0)
					dwIconID = ext->dxtID;

				std::string dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);

				Engine->SetTexImage(m_Items[i].area_return, dxt);
			}
			else {
				std::vector<char> buffer(256, NULL);

				sprintf(&buffer[0], "ui\\itemicon_%.1d_%.4d_%.2d_%.1d.dxt",
					(tbl->dxtID / 10000000),
					(tbl->dxtID / 1000) % 10000,
					(tbl->dxtID / 10) % 100,
					tbl->dxtID % 10);

				std::string szIconFN = &buffer[0];

				Engine->SetTexImage(m_Items[i].area_return, szIconFN);
			}

			Engine->SetState(m_Items[i].btn_return, eUI_STATE::UI_STATE_BUTTON_ON);
		}
		else
		{
			m_Items[i].ItemID = m_Items[i].ItemCount = 0;
			m_Items[i].tbl = nullptr;
			Engine->SetString(m_Items[i].txt_return_first, xorstr(""));
			Engine->SetString(m_Items[i].txt_return_second, xorstr(""));
			Engine->SetString(m_Items[i].txt_rate, xorstr(""));
			Engine->SetVisible(requital[i], false);
		}
	}
}
void CUICollectionRacePlug::Close()
{
	Engine->SetVisible(m_dVTableAddr, false);
}

void CUICollectionRacePlug::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_cr = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)CollectionRaceReceiveMessage_Hook;
}