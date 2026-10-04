#include "EventShowList.h"

CEventShowList::CEventShowList()
{
	text_page = 0;
	text_server_time = 0;
	btn_up = NULL;
	btn_down = NULL;
	m_Timer = NULL;

	page = 1;
	pageCount = 1;
}

CEventShowList::~CEventShowList()
{
	m_Timer = NULL;
}

bool CEventShowList::Load(HANDLE hFile)
{
	if (CN3UIBase::Load(hFile) == false)
		return false;

	std::string find = xorstr("btn_previously");
	btn_down = (CN3UIButton*)GetChildByID(find);
	
	find = xorstr("btn_next");
	btn_up = (CN3UIButton*)GetChildByID(find);
	
	find = xorstr("text_page");
	text_page = (CN3UIString*)GetChildByID(find);
	
	find = xorstr("btn_close");
	btn_close = (CN3UIButton*)GetChildByID(find);
	
	find = xorstr("txt_server_time");
	text_server_time = (CN3UIString*)GetChildByID(find);

	for (int i = 0; i < 10; i++) {
		find = string_format(xorstr("base_event%d"), i+1);
		baseGroup[i] = (CN3UIBase*)GetChildByID(find);
	}

	for (int i = 0; i < 10; i++) {
		CN3UIBase* detailgroup = baseGroup[i];

		find = string_format(xorstr("txt_event_time"));
		txt_event_time[i] = (CN3UIString*)detailgroup->GetChildByID(find);

		find = string_format(xorstr("txt_event_name"));
		txt_event_name[i] = (CN3UIString*)detailgroup->GetChildByID(find);

		txt_event_time[i]->SetString(string_format("Timer-%d", i));
		txt_event_name[i]->SetString(string_format("name-%d", i));
	}
	SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, Engine->m_UiMgr->GetScreenCenter(this).y);
	m_Timer = new CTimer(true, 1000);
	return true;
}

bool CEventShowList::OnKeyPress(int iKey)
{
	if (Engine->m_UiMgr->uiEventShowList == NULL)
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

bool CEventShowList::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	if (pSender == btn_close) Close();
	else if (pSender == btn_up) {
		if (page < pageCount) {
			page++;
			InitList(page);
		}
	}
	else if (pSender == btn_down) {
		if (page > 1) {
			page--;
			InitList(page);
		}
	}
	return true;
}

void CEventShowList::InitList(uint8 p)
{
	if (Engine->m_UiMgr && Engine->m_UiMgr->uiEventShowList) mshowlist = Engine->m_UiMgr->m_eventshowlist;
	if (mshowlist.empty()) return;

	if (p == 255) p = page;

	std::sort(mshowlist.begin(),
		mshowlist.end(), [](auto const& a, auto const& b) { return a.hour < b.hour;});

	pageCount = abs(ceil((double)mshowlist.size() / (double)10));
	if (page > pageCount) page = 1;

	text_server_time->SetString(string_format("%s:%s:%s", to_string(Engine->serverhour).c_str(),
		to_string(Engine->serverminute).c_str(),
		to_string(Engine->serversecond).c_str()));

	int begin = (p - 1) * 10;
	int j = -1;
	for (int i = begin; i < begin + 10; i++) {
		j++;
		if (j > 9) break;
		if (i > mshowlist.size() - 1) { baseGroup[j]->SetVisible(false);continue; }

		txt_event_name[j]->SetString(mshowlist[i].name);
		txt_event_time[j]->SetString(mshowlist[i].time);

		baseGroup[j]->SetVisible(true);
		text_page->SetString(to_string(page));
	}
}

void CEventShowList::Tick()
{
	if (!Engine || !m_Timer->IsElapsedSecond()) return;

	Engine->serversecond++;
	if (Engine->serversecond > 59) { 
		Engine->serverminute++; 
		Engine->serversecond = 0;
	}
	if (Engine->serverminute > 59) {
		Engine->serverhour++;
		Engine->serverminute = 0;
		Engine->serversecond = 0;
	}
	if (Engine->serverhour > 23) {
		Engine->serverhour = 0;
		Engine->serverminute = 0;
		Engine->serversecond = 0;
	}

	text_server_time->SetString(string_format("%s:%s:%s", to_string(Engine->serverhour).c_str(),
		to_string(Engine->serverminute).c_str(),
		to_string(Engine->serversecond).c_str()));
}

void CEventShowList::Open()
{
	if (Engine->m_UiMgr->uiEventShowList && Engine->m_UiMgr->uiEventShowList->IsVisible())
		Engine->m_UiMgr->uiEventShowList->SetVisible(false);

	SetVisible(true);
	SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, Engine->m_UiMgr->GetScreenCenter(this).y);
	Engine->m_UiMgr->SetFocusedUI(this);
}

void CEventShowList::Close()
{
	if (Engine->m_UiMgr->uiEventShowList && !Engine->m_UiMgr->uiEventShowList->IsVisible())
		Engine->m_UiMgr->uiEventShowList->SetVisible(true);

	SetVisible(false);
	SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, Engine->m_UiMgr->GetScreenCenter(this).y);
}

void CEventShowList::OpenScheduler() {
#if (HOOK_SOURCE_VERSION == 1098)
	uint8 Nation = *(uint8*)(*(DWORD*)KO_PTR_CHR + KO_OFF_NATION);
	if (Nation == 1)
	{
		std::string name = Engine->dcpUIF(xorstr("Msoft\\Ka_DailyEvent.Msoft"));
		Engine->m_UiMgr->uiEventShowList = new CEventShowList();
		Engine->m_UiMgr->uiEventShowList->Init(this);
		Engine->m_UiMgr->uiEventShowList->LoadFromFile(Engine->m_BasePath + name.c_str(), N3FORMAT_VER_1068);
		Engine->m_UiMgr->AddChild(Engine->m_UiMgr->uiEventShowList);
		remove(name.c_str());
	}
	else if (Nation == 2)
	{
		std::string name = Engine->dcpUIF(xorstr("Msoft\\El_DailyEvent.Msoft"));
		Engine->m_UiMgr->uiEventShowList = new CEventShowList();
		Engine->m_UiMgr->uiEventShowList->Init(this);
		Engine->m_UiMgr->uiEventShowList->LoadFromFile(Engine->m_BasePath + name.c_str(), N3FORMAT_VER_1068);
		Engine->m_UiMgr->AddChild(Engine->m_UiMgr->uiEventShowList);
		remove(name.c_str());
	}
#else
	std::string name = Engine->dcpUIF(xorstr("Msoft\\DailyEvent.Msoft"));
	Engine->m_UiMgr->uiEventShowList = new CEventShowList();
	Engine->m_UiMgr->uiEventShowList->Init(this);
	Engine->m_UiMgr->uiEventShowList->LoadFromFile(Engine->m_BasePath + name.c_str(), N3FORMAT_VER_1068);
	Engine->m_UiMgr->AddChild(Engine->m_UiMgr->uiEventShowList);
	remove(name.c_str());
#endif
}