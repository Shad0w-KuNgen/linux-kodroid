#include "stdafx.h"
#include "hEventShowList.h"

CEventShowListPlug::CEventShowListPlug()
{
	vector<int>offsets;
	offsets.push_back(0x444);  //co_force_join Satýr : 202
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);
	el_base = Engine->rdword(KO_DLG, offsets);
	btn_details = 0;
	base_news = 0;
	btn_news_close = 0;

	ParseUIElements();
	InitReceiveMessage();
}

CEventShowListPlug::~CEventShowListPlug()
{
}

void CEventShowListPlug::ParseUIElements()  
{
	btn_down = Engine->GetChildByID(m_dVTableAddr, "btn_previously");
	btn_up = Engine->GetChildByID(m_dVTableAddr, "btn_next");
	text_page = Engine->GetChildByID(m_dVTableAddr, "text_page");
	btn_close = Engine->GetChildByID(m_dVTableAddr, "btn_close");
	text_server_time = Engine->GetChildByID(m_dVTableAddr, "txt_server_time");
#if (HOOK_SOURCE_VERSION != 1098)
	base_news = Engine->GetChildByID(m_dVTableAddr, "base_news");
	btn_news_close = Engine->GetChildByID(base_news, "btn_news_close");
	btn_details = Engine->GetChildByID(m_dVTableAddr, "btn_details");
#endif

	for (int i = 0; i < 10; i++)
	{
		baseGroup[i] = Engine->GetChildByID(m_dVTableAddr, string_format(xorstr("base_event%d"), i + 1));
	}
	
	for (int i = 0; i < 10; i++) {
		DWORD detailgroup = baseGroup[i];

		txt_event_time[i] = Engine->GetChildByID(detailgroup, "txt_event_time");
		txt_event_name[i] = Engine->GetChildByID(detailgroup, "txt_event_name");
		Engine->SetString(txt_event_time[i],string_format("Timer-%d", i));
		Engine->SetString(txt_event_name[i],string_format("name-%d", i));
	}
	Engine->UIScreenCenter(el_base);
	m_Timer = new CTimer(true, 1000);

}

DWORD uiEventShow;


bool CEventShowListPlug::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	uiEventShow = el_base;

	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;


	if (pSender ==(DWORD*) btn_close) Close();
	else if (pSender == (DWORD*)btn_up) {
		if (page < pageCount) {
			page++;
			InitList(page);
		}
	}
	else if (pSender == (DWORD*)btn_down) {
		if (page > 1) {
			page--;
			InitList(page);
		}
	}

	if (pSender == (DWORD*)btn_details) {
		Engine->SetVisible(base_news, true);
	}
	if (pSender == (DWORD*)btn_news_close) {
		Engine->SetVisible(base_news, false);
	}

	return true;
}


DWORD Func_eventShow;
void CEventShowListPlug::InitList(uint8 p)
{
	if (Engine->uiEventShowList) mshowlist = Engine->m_UiMgr->m_eventshowlist;
	if (mshowlist.empty()) return;

	if (p == 255) p = page;

	std::sort(mshowlist.begin(),
		mshowlist.end(), [](auto const& a, auto const& b) { return a.hour < b.hour;});

	pageCount = abs(ceil((double)mshowlist.size() / (double)10));
	if (page > pageCount) page = 1;

	Engine->SetString(text_server_time,string_format("%s:%s:%s", to_string(Engine->serverhour).c_str(),
		to_string(Engine->serverminute).c_str(),
		to_string(Engine->serversecond).c_str()));

	int begin = (p - 1) * 10;
	int j = -1;
	for (int i = begin; i < begin + 10; i++) {
		j++;
		if (j > 9) break;
		if (i > mshowlist.size() - 1) { Engine->SetVisible(baseGroup[j],false);continue; }

		Engine->SetString(txt_event_name[j],mshowlist[i].name);
		Engine->SetString(txt_event_time[j],mshowlist[i].time);

		Engine->SetVisible(baseGroup[j],true);
		Engine->SetString(text_page,to_string(page));
	}
}

void CEventShowListPlug::Tick()
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

	Engine->SetString(text_server_time,string_format("%s:%s:%s", to_string(Engine->serverhour).c_str(),
		to_string(Engine->serverminute).c_str(),
		to_string(Engine->serversecond).c_str()));
}


void CEventShowListPlug::Open()
{
	if (Engine->uiEventShowList && Engine->IsVisible(el_base))
		Engine->SetVisible(el_base,false);


	Engine->SetVisible(el_base,true);
	Engine->SetVisible(base_news, false);
	Engine->UIScreenCenter(el_base);
}

void CEventShowListPlug::Close()
{
	if (Engine->uiEventShowList && !Engine->IsVisible(el_base))
		Engine->SetVisible(el_base, true);

	Engine->SetVisible(el_base,false);
	Engine->UIScreenCenter(el_base);
}
void CEventShowListPlug::OpenScheduler()
{

}
void __stdcall UiEventShowReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiEventShowList->ReceiveMessage(pSender, dwMsg);

	__asm
	{
		MOV ECX, uiEventShow
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_eventShow
		CALL EAX
	}
}

void CEventShowListPlug::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(el_base);
	Func_eventShow = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)UiEventShowReceiveMessage_Hook;
}