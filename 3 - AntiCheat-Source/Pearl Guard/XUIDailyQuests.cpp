#include "XUIDailyQuests.h"

extern std::string GetZoneName(uint16 zoneID);

XdailyQuest::XdailyQuest()
{
	m_btnOk = NULL;
	m_btnReview = NULL;
	m_btnCancel = NULL;
	m_txtUprate = NULL;
	text_page = 0;
	btn_up = NULL;
	btn_down = NULL;
	selectQuestID = 0;
	page = 1;
	m_Timer = NULL;
	pageCount = 1;
	pagecountpageCount = 1;
	for (int i = 0; i <= 11; i++) {
		baseQuestButton[i] = 0;
		txt_quest_time[i] = 0;
		txt_quest_status[i] = 0;
		txt_quest_title[i] = 0;
		txt_quest_zone[i] = 0;
		txt_quest_killtype[i] = 0;
 		baseGroup[i] = 0;
	}
}

XdailyQuest::~XdailyQuest()
{

}

bool XdailyQuest::Load(HANDLE hFile)
{
	if (CN3UIBase::Load(hFile) == false) return false;

	DWORD  base = 0;

	std::string find = xorstr("btnGetQuestDetails");
	CN3UIBase* detail = (CN3UIBase*)GetChildByID(find);

	find = xorstr("btnGetQuestDetails");
	m_btnOk = (CN3UIButton*)detail->GetChildByID(find);
	find = xorstr("btn_previouspage");
	btn_down = (CN3UIButton*)detail->GetChildByID(find);
	find = xorstr("btn_nextpage");
	btn_up = (CN3UIButton*)detail->GetChildByID(find);
	find = xorstr("text_page");
	text_page = (CN3UIString*)detail->GetChildByID(find);

	find = xorstr("btn_close");
	btn_close = (CN3UIButton*)GetChildByID(find);


	for (int i = 0; i <= 11; i++) {
		if (i > 9)
			find = string_format(xorstr("group_quest_%d"), i);
		else
			find = string_format(xorstr("group_quest_0%d"), i);

		baseGroup[i] = (CN3UIBase*)detail->GetChildByID(find);
	}
	for (int i = 0; i <= 11; i++) {
		CN3UIBase* detailgroup = baseGroup[i];

		if (i > 9)
			find = string_format(xorstr("btn_selectquest%d"), i);
		else
			find = string_format(xorstr("btn_selectquest0%d"), i);

		baseQuestButton[i] = (CN3UIButton*)detailgroup->GetChildByID(find);

		find = string_format(xorstr("txt_quest_title"));
		txt_quest_title[i] = (CN3UIString*)detailgroup->GetChildByID(find);

		find = string_format(xorstr("txt_quest_status"));
		txt_quest_status[i] = (CN3UIString*)detailgroup->GetChildByID(find);

		find = string_format(xorstr("txt_quest_time"));
		txt_quest_time[i] = (CN3UIString*)detailgroup->GetChildByID(find);

		find = string_format(xorstr("txt_quest_zone"));
		txt_quest_zone[i] = (CN3UIString*)detailgroup->GetChildByID(find);

		find = string_format(xorstr("txt_quest_type"));
		txt_quest_killtype[i] = (CN3UIString*)detailgroup->GetChildByID(find);

		txt_quest_time[i]->SetString(string_format("Timer-%d", i));
		txt_quest_status[i]->SetString(string_format("Status-%d", i));
		txt_quest_title[i]->SetString(string_format("Title-%d", i));
		txt_quest_zone[i]->SetString(string_format("Zone-%d", i));
		txt_quest_killtype[i]->SetString(string_format("Type-%d", i));
	}
	SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, Engine->m_UiMgr->GetScreenCenter(this).y);
	m_Timer = new CTimer(true, 600);
	return true;
}

bool XdailyQuest::IsInSlots()
{
	//Engine->m_UiMgr->uiGetQuestInfo->Start(filteredList[selectQuestID].targetMob, filteredList[selectQuestID].targetCount, filteredList[selectQuestID].currentCount, Engine->d3dUIEngine->uiDailyQuests->getZoneName(filteredList[selectQuestID].zone).c_str(), 0);
	return true;
}
#define MINUTE				60
#define HOUR				MINUTE * 60

void XdailyQuest::KillTrigger(uint16 mob)
{
	for (size_t i = 0; i < kcbq_quests.size(); i++) {

		if (kcbq_quests[i]->Status == (uint8)DailyQuestStatus::comp || kcbq_quests[i]->Status == (uint8)DailyQuestStatus::timewait)
			continue;

		if (kcbq_quests[i]->Mob[0] == mob || kcbq_quests[i]->Mob[1] == mob || kcbq_quests[i]->Mob[2] == mob || kcbq_quests[i]->Mob[3] == mob)
		{
			kcbq_quests[i]->MyCount++;

			if (kcbq_quests[i]->MyCount >= kcbq_quests[i]->KillTarget)
			{
				if (kcbq_quests[i]->questtype == (uint8)DailyQuesttimetype::single)
				{
					kcbq_quests[i]->Status = (uint8)DailyQuestStatus::comp;
					continue;
				}

				if (kcbq_quests[i]->questtype == (uint8)DailyQuesttimetype::time && kcbq_quests[i]->remtime == 0) {
					kcbq_quests[i]->Status = (uint8)DailyQuestStatus::timewait;
					kcbq_quests[i]->remtime = kcbq_quests[i]->replaytime * HOUR;
				}
			}
		}


		/*if (Engine->m_UiMgr->uiQuestRewards && Engine->m_UiMgr->uiQuestRewards->IsVisible() && Engine->m_UiMgr->uiQuestRewards->cMob == kcbq_quests[i]->Mob[j])
		{
			Engine->m_UiMgr->uiQuestRewards->LoadInfo(kcbq_quests[i]);
		}*/
	}
	InitQuests(page);
}

void XdailyQuest::InitQuests(uint8 p)
{
	if (Engine->m_UiMgr && Engine->m_UiMgr->uixDailyQuest) kcbq_quests = Engine->m_UiMgr->uixDailyQuest->kcbq_quests;

	if (p == 255) p = page;
	filteredList.clear();

	uint8 nation = 0;
	DWORD chr = *(DWORD*)KO_PTR_CHR;
	if (!chr)
		return;
	nation = *(uint8*)(chr + KO_OFF_NATION);

	for (int i = 0; i < kcbq_quests.size(); i++)
	{
		if ((kcbq_quests[i]->ZoneID == 1 || kcbq_quests[i]->ZoneID == 2) && kcbq_quests[i]->ZoneID != nation)
			continue;

		filteredList.push_back(kcbq_quests[i]);
	}

	if (filteredList.empty()) 
		return;

	pageCount = abs(ceil((double)filteredList.size() / (double)12));
	if (page > pageCount)
		page = 1;

	int begin = (p - 1) * 12;
	int j = -1;
	for (int i = begin; i < begin + 12; i++)
	{
		j++;
		if (j > 11) break;
		if (i > filteredList.size() - 1)
		{
			baseGroup[j]->SetVisible(false);
			continue;
		}

		if (filteredList[i]->Mob[0]) {
			if (auto data = Engine->tblMgr->getMobData(filteredList[i]->Mob[0]))
				txt_quest_title[j]->SetString(string_format(xorstr("%s"), data->strName.c_str()));
			else
				txt_quest_title[j]->SetString(string_format(xorstr("%s"), "Kill Enemy"));
		}

		std::string zonename = GetZoneName(filteredList[i]->ZoneID);
		txt_quest_zone[j]->SetString(string_format(xorstr("%s"), zonename.empty() ? "unknown" : zonename.c_str()));

		std::string killtypename = "-";
		if (filteredList[i]->killtype == 0)killtypename = "Solo";
		else if (filteredList[i]->killtype == 1)killtypename = "Party";
		else if (filteredList[i]->killtype == 2)killtypename = "Solo/Party";
		txt_quest_killtype[j]->SetString(string_format(xorstr("%s"), killtypename.c_str()));

		baseGroup[j]->SetVisible(true);
		text_page->SetString(to_string(page));

		switch (filteredList[i]->questtype)
		{
		case (uint8)DailyQuesttimetype::repeat:
			txt_quest_status[j]->SetString(string_format("Repeatable"));
			//Engine->SetStringColor(txt_quest_status[j], 0x129603);
			//txt_quest_status[j]->SetColor(0x129603);
			txt_quest_time[j]->SetString(string_format("-"));
			break;
		case (uint8)DailyQuesttimetype::single:
			if (filteredList[i]->Status == (uint8)DailyQuestStatus::ongoing)
			{
				txt_quest_status[j]->SetString(string_format("Ongoing"));
				//Engine->SetStringColor(txt_quest_status[j], 0x129603);
				//txt_quest_status[j]->SetColor(0x129603);
				txt_quest_time[j]->SetString(string_format("-"));
			}
			else {
				txt_quest_status[j]->SetString(string_format("Finished"));
				//Engine->SetStringColor(txt_quest_status[j], 0xff7a70);
				//txt_quest_status[j]->SetColor(0xff7a70);
				txt_quest_time[j]->SetString(string_format("-"));
			}
			break;
		case (uint8)DailyQuesttimetype::time:
			if (filteredList[i]->Status == (uint8)DailyQuestStatus::ongoing)
			{
				txt_quest_status[j]->SetString(string_format("Ongoing"));
				//Engine->SetStringColor(txt_quest_status[j], 0x129603);
				//txt_quest_status[j]->SetColor(0x129603);
				txt_quest_time[j]->SetString(string_format("-"));
			}
			else {
				txt_quest_status[j]->SetString(string_format("Waiting"));
				//Engine->SetStringColor(txt_quest_status[j], 0x129603);
				//txt_quest_status[j]->SetColor(0x129603);
			}
			break;
		}
	}
}

bool XdailyQuest::OnKeyPress(int iKey)
{
	if (Engine->m_UiMgr->uixDailyQuest == NULL)
		return CN3UIBase::OnKeyPress(iKey);

	if (!IsVisible())
		return CN3UIBase::OnKeyPress(iKey);

	switch (iKey)
	{
	case DIK_ESCAPE:
		Close();
		return true;
		break;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

void XdailyQuest::Open()
{
	if (Engine->m_UiMgr->uixDailyQuest && Engine->m_UiMgr->uixDailyQuest->IsVisible())
		Engine->m_UiMgr->uixDailyQuest->SetVisible(false);

	SetVisible(true);
	SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, Engine->m_UiMgr->GetScreenCenter(this).y);
	Engine->m_UiMgr->SetFocusedUI(this);
}

void XdailyQuest::Close()
{
	if (Engine->m_UiMgr->uixDailyQuest && !Engine->m_UiMgr->uixDailyQuest->IsVisible())
		Engine->m_UiMgr->uixDailyQuest->SetVisible(true);

	SetVisible(false);
	SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, Engine->m_UiMgr->GetScreenCenter(this).y);
}

bool XdailyQuest::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	std::string msg = "";
	if (pSender == m_btnOk) {

		if (!Engine->m_UiMgr->uiQuestRewards)
		{
			std::string name = Engine->dcpUIF(xorstr("Msoft\\DailyQuest_Reward.Msoft"));
			Engine->m_UiMgr->uiQuestRewards = new CQuestRewards();
			Engine->m_UiMgr->uiQuestRewards->Init(Engine->m_UiMgr);
			Engine->m_UiMgr->uiQuestRewards->LoadFromFile(Engine->m_BasePath + name.c_str(), N3FORMAT_VER_1068);
			Engine->m_UiMgr->AddChild(Engine->m_UiMgr->uiQuestRewards);
			remove(name.c_str());
		}

		Engine->m_UiMgr->uiQuestRewards->LoadInfo(filteredList[selectQuestID]);
		Engine->m_UiMgr->uiQuestRewards->Open();
	}
	else if (pSender == btn_close) {
		Close();
	}
	else if (pSender == btn_up) {
		if (page < pageCount)
		{
			page++;
			InitQuests(page);
		}
	}
	else if (pSender == btn_down) {
		if (page > 1)
		{
			page--;
			InitQuests(page);
		}
	}
	else {
		for (int i = 0; i <= 11; i++)
		{
			if (pSender == baseQuestButton[i]) {
				selectQuestID = ((page - 1) * 12) + i;
				baseQuestButton[i]->SetState(UI_STATE_BUTTON_DOWN);

			}
			else
				baseQuestButton[i]->SetState(UI_STATE_BUTTON_NORMAL);
		}

	}
	return true;
}

std::string calcDiff2(int h1, int m1, int s1, int h2, int m2, int s2) {
	std::chrono::seconds d = std::chrono::hours(h2 - h1)
		+ std::chrono::minutes(m2 - m1)
		+ std::chrono::seconds(s2 - s1);

	return string_format("%d:%d:%d", std::chrono::duration_cast<std::chrono::hours>(d).count(),
		std::chrono::duration_cast<std::chrono::minutes>(d % std::chrono::hours(1)).count(),
		std::chrono::duration_cast<std::chrono::seconds>(d % std::chrono::minutes(1)).count());
}

long long xtimePassed = 0;

void XdailyQuest::Tick()
{
	if (xtimePassed == 0) xtimePassed = clock();

	if (xtimePassed > clock() - 1000)
		return;

	xtimePassed = clock();

	for (size_t i = 0; i < kcbq_quests.size(); i++)
	{
		if (kcbq_quests[i]->remtime)
			kcbq_quests[i]->remtime--;
	}

	if (filteredList.empty())
		return;

	if (page > pageCount)
		page = 1;

	int begin = (page - 1) * 12;
	int j = -1;
	for (int i = begin; i < begin + 12; i++)
	{
		j++;
		if (i > filteredList.size() - 1) continue;

		if (filteredList[i]->questtype == (uint8)DailyQuesttimetype::time && filteredList[i]->Status == (uint8)DailyQuestStatus::timewait) {
			if (filteredList[i]->remtime) {
				uint16_t remainingMinutes = (uint16_t)ceil(filteredList[i]->remtime / 60);
				uint16_t remainingSeconds = filteredList[i]->remtime - (remainingMinutes * 60);

				std::string remainingTime;
				if (remainingMinutes < 10 && remainingSeconds < 10)
					remainingTime = string_format(xorstr("0%d : 0%d"), remainingMinutes, remainingSeconds);
				else if (remainingMinutes < 10)
					remainingTime = string_format(xorstr("0%d : %d"), remainingMinutes, remainingSeconds);
				else if (remainingSeconds < 10)
					remainingTime = string_format(xorstr("%d : 0%d"), remainingMinutes, remainingSeconds);
				else
					remainingTime = string_format(xorstr("%d : %d"), remainingMinutes, remainingSeconds);

				txt_quest_status[j]->SetString("Waiting");
				//txt_quest_status[j]->SetStringColor(0x129603);
				//txt_quest_status[j]->SetColor(0x129603);
				txt_quest_time[j]->SetString(remainingTime.c_str());
			}
			else {
				filteredList[i]->Status = (uint8)DailyQuestStatus::ongoing;
			}
		}
	}
}

void XdailyQuest::OpenDailyQuest()
{
	std::string name = Engine->dcpUIF(xorstr("Msoft\\DailyQuest_Main.Msoft"));
	Engine->m_UiMgr->uixDailyQuest = new XdailyQuest();
	Engine->m_UiMgr->uixDailyQuest->Init(this);
	Engine->m_UiMgr->uixDailyQuest->LoadFromFile(Engine->m_BasePath + name.c_str(), N3FORMAT_VER_1068);
	Engine->m_UiMgr->AddChild(Engine->m_UiMgr->uixDailyQuest);
	remove(name.c_str());
}