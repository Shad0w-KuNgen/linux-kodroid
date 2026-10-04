#include "UIDailyQuests.h"

extern std::string GetZoneName(uint16 zoneID);

HookQuestPage::HookQuestPage()
{
	vector<int>offsets;
#if (HOOK_SOURCE_VERSION == 1098 || HOOK_SOURCE_VERSION == 1534)
	offsets.push_back(0x5E0);
	offsets.push_back(0);
#else
	offsets.push_back(0x250);   // 2369		//ka_change Satýr : 33
	offsets.push_back(0);
#endif

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);

	m_btnOk = 0;
	m_btnReview = 0;
	m_btnCancel = 0;
	m_txtUprate = 0;
	text_page = 0;
	btn_up = 0;
	btn_down = 0;
	selectQuestID = 0;
	page = 1;
	m_Timer = NULL;
	pageCount = 1;
	pagecountpageCount = 1;
	for (int i = 0; i < 15; i++) {
		baseQuestButton[i] = 0;
		txt_quest_time[i] = 0;
		txt_quest_status[i] = 0;
		txt_quest_title[i] = 0;
		baseGroup[i] = 0;
		txt_quest_zone[i] = 0;
		txt_quest_killtype[i] = 0;
	}
	ParseUIElements();
	InitReceiveMessage();
}

HookQuestPage::~HookQuestPage()
{
}

void HookQuestPage::ParseUIElements()
{
	group_quest = Engine->GetChildByID(m_dVTableAddr, "group_quest");
	quest_rewards = Engine->GetChildByID(m_dVTableAddr, "quest_rewards");
	text_page = Engine->GetChildByID(group_quest, "text_page");
	btn_down = Engine->GetChildByID(group_quest, "btn_previouspage");
	btn_up = Engine->GetChildByID(group_quest, "btn_nextpage");
	for (int i = 0; i <= 14; i++) {
		if (i > 9)
			baseGroup[i] = Engine->GetChildByID(group_quest, string_format(xorstr("group_quest_%d"), i));
		else
			baseGroup[i] = Engine->GetChildByID(group_quest, string_format(xorstr("group_quest_0%d"), i));
	}
	for (int i = 0; i <= 14; i++) {
		if (i > 9)
			baseQuestButton[i] = Engine->GetChildByID(baseGroup[i], string_format(xorstr("btn_selectquest%d"), i));
		else
			baseQuestButton[i] = Engine->GetChildByID(baseGroup[i], string_format(xorstr("btn_selectquest0%d"), i));

		txt_quest_title[i] = Engine->GetChildByID(baseGroup[i], "txt_quest_title");
		txt_quest_status[i] = Engine->GetChildByID(baseGroup[i], "txt_quest_status");
		txt_quest_time[i] = Engine->GetChildByID(baseGroup[i], "txt_quest_time");
		txt_quest_zone[i] = Engine->GetChildByID(baseGroup[i], "txt_quest_zone");
		txt_quest_killtype[i] = Engine->GetChildByID(baseGroup[i], "txt_quest_type");
	}
	for (int i = 0; i <= 14; i++) {
		Engine->SetString(txt_quest_time[i], string_format("Timer-%d", i));
		Engine->SetString(txt_quest_status[i], string_format("Status-%d", i));
		Engine->SetString(txt_quest_title[i], string_format("Title-%d", i));
	}
	
	btn_close = Engine->GetChildByID(quest_rewards, "btn_close");
	str_quest = Engine->GetChildByID(quest_rewards, "text_quest_name");
	str_zone = Engine->GetChildByID(quest_rewards, "text_zone");
	hunt = Engine->GetChildByID(quest_rewards, "Hunt");
	str_hunt = Engine->GetChildByID(hunt, "text_hunt_name");
	str_hunt_count = Engine->GetChildByID(hunt, "text_hunt_count");
	hunt_area = Engine->GetChildByID(hunt, "icon");
	for (uint8 i = 0; i < 4; i++)
	{
		rewardBase[i].base = Engine->GetChildByID(quest_rewards, string_format(xorstr("Reward%d"), i + 1));
		rewardBase[i].str_name = Engine->GetChildByID(rewardBase[i].base, "str_name");
		rewardBase[i].str_count = Engine->GetChildByID(rewardBase[i].base, "str_count");
		rewardBase[i].str_btn = Engine->GetChildByID(rewardBase[i].base, "reward");
		rewardBase[i].icon = Engine->GetChildByID(rewardBase[i].base, "icon");
	}

	Engine->SetVisible(quest_rewards, false);
	m_Timer = new CTimer(true, 600);
}

void HookQuestPage::LoadInfo(DailyQuest* quest)
{
	cMob = quest->Mob[0];

	std::string zone = GetZoneName(quest->ZoneID);
	Engine->SetString(str_zone,zone);
	if (auto mob = Engine->tblMgr->getMobData(quest->Mob[0]))
	{
		Engine->SetString(str_quest,string_format(xorstr("%s"), mob->strName.c_str()));
		Engine->SetString(str_hunt,mob->strName);
	}
	else {
		if (quest->Mob[0] == 1) {
			Engine->SetString(str_quest,xorstr("<Kill Enemy>"));
			Engine->SetString(str_hunt,xorstr("<Kill Enemy>"));
		}
		else {
			Engine->SetString(str_quest,xorstr("<Unknown>"));
			Engine->SetString(str_hunt,xorstr("<Unknown>"));
		}
	}

	Engine->SetString(str_hunt_count,string_format(xorstr("%d / %d"), quest->MyCount, quest->KillTarget));

	uint32 OriginItemID = 900005000;

	TABLE_ITEM_BASIC* tbl = Engine->tblMgr->getItemData(OriginItemID);
	if (TABLE_ITEM_EXT* ext = Engine->tblMgr->getExtData(tbl->extNum, OriginItemID))
	{
		DWORD dwIconID = tbl->dxtID;
		if (ext->dxtID > 0)
			dwIconID = ext->dxtID;

		std::string dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);
		  
		Engine->SetTexImage(hunt_area,dxt);
	}
	else {
		std::vector<char> buffer(256, NULL);

		sprintf(&buffer[0], "ui\\itemicon_%.1d_%.4d_%.2d_%.1d.dxt",
			(tbl->dxtID / 10000000),
			(tbl->dxtID / 1000) % 10000,
			(tbl->dxtID / 10) % 100,
			tbl->dxtID % 10);

		std::string szIconFN = &buffer[0];


		Engine->SetTexImage(hunt_area, szIconFN);
	}


	// Rewards
	int rewardCount = 0;
	for (uint8 i = 0; i < 4; i++)
	{
		if (quest->Reward[i])
		{
			rewardCount++;
		}
	}

	for (uint8 i = 0; i < 4; i++) {
		if (quest->Reward[i]) {
			rewardBase[i].nItemID = quest->Reward[i];
			Engine->SetVisible(rewardBase[i].base,true);
			Engine->SetVisible(rewardBase[i].icon, true);

			uint32 OriginItemID = quest->Reward[i];

			if (TABLE_ITEM_BASIC* tbl = Engine->tblMgr->getItemData(OriginItemID)) {
				rewardBase[i].tbl = tbl;

				TABLE_ITEM_EXT* ext = Engine->tblMgr->getExtData(tbl->extNum, OriginItemID);
				if (ext) {
					DWORD dwIconID = tbl->dxtID;
					if (ext->dxtID > 0) dwIconID = ext->dxtID;
					std::string dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);

					Engine->SetTexImage(rewardBase[i].icon, dxt);
				}
				else {
					std::vector<char> buffer(256, NULL);
					sprintf(&buffer[0], "ui\\itemicon_%.1d_%.4d_%.2d_%.1d.dxt", (tbl->dxtID / 10000000), (tbl->dxtID / 1000) % 10000, (tbl->dxtID / 10) % 100, tbl->dxtID % 10);
					std::string szIconFN = &buffer[0];
					Engine->SetTexImage(rewardBase[i].icon, szIconFN);
				}

				uint32 counta = quest->Count[i];

				Engine->SetString(rewardBase[i].str_name,tbl->strName);
				Engine->SetString(rewardBase[i].str_count,to_string(quest->Count[i]));
			}
		}
		else Engine->SetVisible(rewardBase[i].base,false);
	}
}

bool HookQuestPage::IsInSlots()
{
	//Engine->m_UiMgr->uiGetQuestInfo->Start(filteredList[selectQuestID].targetMob, filteredList[selectQuestID].targetCount, filteredList[selectQuestID].currentCount, Engine->m_UiMgr->uiDailyQuests->getZoneName(filteredList[selectQuestID].zone).c_str(), 0);
	return true;
}
#define MINUTE				60
#define HOUR				MINUTE * 60

void HookQuestPage::KillTrigger(uint8 questID, uint16 mob)
{
	foreach(itr, kcbq_quests)
	{
		if ((*itr)->index != questID
			|| (*itr)->Status == (uint8)DailyQuestStatus::comp
			|| (*itr)->Status == (uint8)DailyQuestStatus::timewait)
			continue;

		bool _v = false;
		for (int i = 0; i < 4; i++)
			if ((*itr)->Mob[i] == mob) _v = true;

		if (!_v) continue;
		(*itr)->MyCount++;
		if ((*itr)->MyCount >= (*itr)->KillTarget)
		{
			if ((*itr)->questtype == (uint8)DailyQuesttimetype::single)
			{
				(*itr)->Status = (uint8)DailyQuestStatus::comp;
				continue;
			}

			if ((*itr)->questtype == (uint8)DailyQuesttimetype::time && (*itr)->remtime == 0) {
				(*itr)->Status = (uint8)DailyQuestStatus::timewait;
				(*itr)->remtime = (*itr)->replaytime * 60 * 60;
			}
			else
			{
				(*itr)->MyCount = 0;
				(*itr)->remtime = 0;
				(*itr)->Status = (uint8)DailyQuestStatus::ongoing;
			}
		}
		if (Engine->IsVisible(quest_rewards))
			LoadInfo(filteredList[selectQuestID]);
		break;
	}

	InitQuests(page);
}

uint32_t HookQuestPage::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (Engine->IsVisible(quest_rewards))
	{
		bool showTooltip = false;

		for (int i = 0; i < 4; i++)
		{
			if (rewardBase[i].icon != NULL) {
				if (Engine->IsIn(rewardBase[i].icon,ptCur.x, ptCur.y) && rewardBase[i].tbl != nullptr && rewardBase[i].nItemID != 0)
				{
					Engine->ShowToolTipEx(rewardBase[i].nItemID, ptCur.x, ptCur.y);
					showTooltip = true;
					break;
				}
			}
		}
	}
	return dwRet;
}

DWORD uiQuestPage;
DWORD Func_SendMessageQuest;
void HookQuestPage::InitQuests(uint8 p)
{
	if (p == 255) p = page;
	filteredList.clear();

	uint8 nation = 0;
	DWORD chr = *(DWORD*)KO_PTR_CHR;
	if (!chr)
		return;

	nation = *(uint8*)(chr + KO_OFF_NATION);

	for (int i = 0; i < kcbq_quests.size(); i++)
	{
		if (kcbq_quests[i]->ZoneID) {
			if (kcbq_quests[i]->ZoneID == ZONE_MORADON && !isInMoradon(Engine->Player.ZoneID)) continue;
			else if (kcbq_quests[i]->ZoneID == ZONE_ELMORAD && !isInElmoradCastle(Engine->Player.ZoneID)) continue;
			else if (kcbq_quests[i]->ZoneID == ZONE_KARUS && !isInLufersonCastle(Engine->Player.ZoneID)) continue;
			else if (kcbq_quests[i]->ZoneID == ZONE_ELMORAD_ESLANT && !isInElmoradEslant(Engine->Player.ZoneID)) continue;
			else if (kcbq_quests[i]->ZoneID == ZONE_KARUS_ESLANT && !isInKarusEslant(Engine->Player.ZoneID)) continue;
			else { if (kcbq_quests[i]->ZoneID != Engine->Player.ZoneID) continue; }
		}

		if ((kcbq_quests[i]->ZoneID == 1 || kcbq_quests[i]->ZoneID == 2) && kcbq_quests[i]->ZoneID != nation) continue;

		if (Engine->m_PlayerBase->GetLevel() <  kcbq_quests[i]->MinLevel || Engine->m_PlayerBase->GetLevel() > kcbq_quests[i]->MaxLevel)
			continue;

		filteredList.push_back(kcbq_quests[i]);
	}

	if (filteredList.empty()) {
		for (int i = 0; i < 15; i++)Engine->SetVisible(baseGroup[i], false);
		return;
	}

	pageCount = abs(ceil((double)filteredList.size() / (double)15));
	if (page > pageCount)
		page = 1;

	int begin = (p - 1) * 15;
	int j = -1;
	for (int i = begin; i < begin + 15; i++)
	{
		j++;
		if (j > 14) break;
		if (i > filteredList.size() - 1)
		{
			Engine->SetVisible(baseGroup[j], false);
			continue;
		}

		if (filteredList[i]->Mob[0]) {
			if(auto data = Engine->tblMgr->getMobData(filteredList[i]->Mob[0]))
				Engine->SetString(txt_quest_title[j], string_format(xorstr("%s"), data->strName.c_str()));
			else 
				Engine->SetString(txt_quest_title[j], string_format(xorstr("%s"), "Kill Enemy"));
		}

		std::string zonename = GetZoneName(filteredList[i]->ZoneID);
		Engine->SetString(txt_quest_zone[j], string_format(xorstr("%s"), zonename.empty() ? "unknown" : zonename.c_str()));

		std::string killtypename = "-";
		if (filteredList[i]->killtype == 0)killtypename = "Solo";
		else if (filteredList[i]->killtype == 1)killtypename = "Party";
		else if (filteredList[i]->killtype == 2)killtypename = "Solo/Party";
		Engine->SetString(txt_quest_killtype[j], string_format(xorstr("%s"), killtypename.c_str()));

		Engine->SetVisible(baseGroup[j], true);
		Engine->SetString(text_page, to_string(page));

		switch (filteredList[i]->questtype)
		{
			case (uint8)DailyQuesttimetype::repeat:
				Engine->SetString(txt_quest_status[j], string_format("Repeatable"));
				Engine->SetStringColor(txt_quest_status[j], 0x129603);
				Engine->SetString(txt_quest_time[j], "-");
				break;
			case (uint8)DailyQuesttimetype::single:
				if (filteredList[i]->Status == (uint8)DailyQuestStatus::ongoing)
				{
					Engine->SetString(txt_quest_status[j], string_format("Ongoing"));
					Engine->SetStringColor(txt_quest_status[j], 0x129603);
					Engine->SetString(txt_quest_time[j], "-");
				}
				else {
					Engine->SetString(txt_quest_status[j], string_format("Finished"));
					Engine->SetStringColor(txt_quest_status[j], 0xff7a70);
					Engine->SetString(txt_quest_time[j], "-");
				}
				break;
			case (uint8)DailyQuesttimetype::time:
				if (filteredList[i]->Status == (uint8)DailyQuestStatus::ongoing)
				{
					Engine->SetString(txt_quest_status[j], string_format("Ongoing"));
					Engine->SetStringColor(txt_quest_status[j], 0x129603);
					Engine->SetString(txt_quest_time[j], "-");
				}
				else {
					Engine->SetString(txt_quest_status[j], string_format("Waiting"));
					Engine->SetStringColor(txt_quest_status[j], 0x129603);
				}
				break;
		}
	}
}

bool HookQuestPage::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	uiQuestPage = m_dVTableAddr;
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	std::string msg = "";
	if (pSender == (DWORD*)m_btnOk) {
		if (!filteredList.empty()) {
			LoadInfo(filteredList[selectQuestID]);
			Engine->SetVisible(quest_rewards, true);
		}
	}
	else if (pSender == (DWORD*)btn_up) {
		if (page < pageCount)
		{
			page++;
			InitQuests(page);
		}
	}
	else if (pSender == (DWORD*)btn_close)
	{
		Engine->SetVisible(quest_rewards, false);
		//Engine->SetVisible(el_Base, true);
	}
	else if (pSender == (DWORD*)btn_down) {
		if (page > 1)
		{
			page--;
			InitQuests(page);
		}
	}
	else {

		for (int i = 0; i <= 14; i++)
		{
			if (pSender == (DWORD*)baseQuestButton[i]) {
				selectQuestID = ((page - 1) * 15) + i;
				Engine->SetState(baseQuestButton[i], UI_STATE_BUTTON_DOWN);
			}
			else
				Engine->SetState(baseQuestButton[i], UI_STATE_BUTTON_NORMAL);
		}

		InitQuests(page);
		for (int i = 0;i < 4;i++) {
			if (pSender == (DWORD*)rewardBase[i].str_btn) {
				if (rewardBase[i].nItemID == 900004000) {
					Packet pkt(XSafe, uint8(XSafeOpCodes::DROP_REQUEST));
					pkt << uint8(4) << uint8(i);
					Engine->Send(&pkt);
				}
			}
		}

		if (!filteredList.empty()) {
			LoadInfo(filteredList[selectQuestID]);
			Engine->SetVisible(quest_rewards, true);
		}
	}
	return true;
}

std::string calcDiff(int h1, int m1, int s1, int h2, int m2, int s2) {
	std::chrono::seconds d = std::chrono::hours(h2 - h1)
		+ std::chrono::minutes(m2 - m1)
		+ std::chrono::seconds(s2 - s1);

	return string_format("%d:%d:%d", std::chrono::duration_cast<std::chrono::hours>(d).count(),
		std::chrono::duration_cast<std::chrono::minutes>(d % std::chrono::hours(1)).count(),
		std::chrono::duration_cast<std::chrono::seconds>(d % std::chrono::minutes(1)).count());
}

long long timePassed = 0;

void HookQuestPage::Tick()
{
	if (timePassed == 0) timePassed = clock();

	if (timePassed > clock() - 1000)
		return;

	timePassed = clock();

	for (size_t i = 0; i < kcbq_quests.size(); i++)
	{
		if (kcbq_quests[i]->remtime)
			kcbq_quests[i]->remtime--;
	}

	if (!Engine->IsVisible(m_dVTableAddr))
		return;
	
	if (filteredList.empty())
		return;

	if (page > pageCount)
		page = 1;

	int begin = (page - 1) * 15;
	int j = -1;
	for (int i = begin; i < begin + 15; i++)
	{
		j++;
		if (i > filteredList.size() - 1) continue;

		if (filteredList[i]->questtype == (uint8)DailyQuesttimetype::time && filteredList[i]->Status == (uint8)DailyQuestStatus::timewait) {
			
			if (filteredList[i]->remtime) {

				std::string remainingTime = "";
				uint32_t days = (uint32_t)floor(filteredList[i]->remtime / 86400);
				uint32_t hours = (uint32_t)floor(((filteredList[i]->remtime - days * 86400) / 3600) % 24);
				uint32_t minutes = (uint32_t)floor((((filteredList[i]->remtime - days * 86400) - hours * 3600) / 60) % 60);
				uint32_t seconds = (uint32_t)floor(((((filteredList[i]->remtime - days * 86400) - hours * 3600) - minutes * 60)) % 60);

				if (days)
					remainingTime += string_format(xorstr("%s%u:"), days < 10 ? "0" : "", days);

				if (hours)
					remainingTime += string_format(xorstr("%s%u:"), hours < 10 ? "0" : "", hours);
				else
					remainingTime += "00:";

				if (minutes)
					remainingTime += string_format(xorstr("%s%u:"), minutes < 10 ? "0" : "", minutes);
				else
					remainingTime += "00:";

				if (seconds)
					remainingTime += string_format(xorstr("%s%u"), seconds < 10 ? "0" : "", seconds);
				else
					remainingTime += "00";
				
				//printf("Time remaining: %u Days, %u Hours, %u Minutes, %u Seconds\n", days, hours, minutes, seconds);


				/*uint32 seconds = filteredList[i]->remtime;
				uint16_t remainingMinutes = (uint16_t)ceil(seconds / 60);
				uint16_t remainingSeconds = seconds - (remainingMinutes * 60);
				
				if (remainingMinutes < 10 && remainingSeconds < 10)
					remainingTime = string_format(xorstr("0%d : 0%d"), remainingMinutes, remainingSeconds);
				else if (remainingMinutes < 10)
					remainingTime = string_format(xorstr("0%d : %d"), remainingMinutes, remainingSeconds);
				else if (remainingSeconds < 10)
					remainingTime = string_format(xorstr("%d : 0%d"), remainingMinutes, remainingSeconds);
				else
					remainingTime = string_format(xorstr("%d : %d"), remainingMinutes, remainingSeconds);*/

				Engine->SetString(txt_quest_status[j], string_format("Waiting"));
				Engine->SetStringColor(txt_quest_status[j], 0x129603);
				Engine->SetString(txt_quest_time[j], remainingTime.c_str());
			}
			else {
				filteredList[i]->Status = (uint8)DailyQuestStatus::ongoing;
			}
		}
	}
		

}

void __stdcall UiQuestReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiQuestPage->ReceiveMessage(pSender, dwMsg);

	__asm
	{
		MOV ECX, uiQuestPage
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_SendMessageQuest
		CALL EAX
	}
}

void HookQuestPage::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_SendMessageQuest = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)UiQuestReceiveMessage_Hook;
}