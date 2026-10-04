#include "UIHpBarMenu.h"

CUIHPBarMenuPlug::CUIHPBarMenuPlug()
{
	m_dVTableAddr = NULL;


	vector<int>offsets;
	offsets.push_back(0x54C);   // co_drop_off Satýr : 247
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);

	btn_daily_event = 0;
	btn_daily_quest = 0;
	btn_facebook = 0;
	btn_discord = 0; 
	btn_wof = 0;
	btn_searchdrop = 0;
	btn_ticket = 0;
	btn_rank = 0;
	btn_scheduler = 0;
	btn_pus = 0;
	text_online_time = 0;
	ParseUIElements();
	InitReceiveMessage();
}
DWORD Func_HpMenu;
CUIHPBarMenuPlug::~CUIHPBarMenuPlug()
{
}

void CUIHPBarMenuPlug::ParseUIElements()
{
#if (HOOK_SOURCE_VERSION == 1098)
	btn_wof = Engine->GetChildByID(m_dVTableAddr, "btn_wof");
	btn_pus = Engine->GetChildByID(m_dVTableAddr, "btn_pus");
	btn_searchdrop = Engine->GetChildByID(m_dVTableAddr, "btn_searchdrop");
	btn_ticket = Engine->GetChildByID(m_dVTableAddr, "btn_ticket");
	btn_scheduler = Engine->GetChildByID(m_dVTableAddr, "btn_scheduler");
	btn_rank = Engine->GetChildByID(m_dVTableAddr, "btn_dailyrank");
#else
	btn_facebook = Engine->GetChildByID(m_dVTableAddr, "btn_facebook");
	btn_discord = Engine->GetChildByID(m_dVTableAddr, "btn_discord");
	btn_ticket = Engine->GetChildByID(m_dVTableAddr, "btn_ticket");
	btn_daily_event = Engine->GetChildByID(m_dVTableAddr, "btn_daily_event");
#endif
}

DWORD uiHpMenu;
bool CUIHPBarMenuPlug::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	uiHpMenu = m_dVTableAddr;
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	if (pSender == (DWORD*)btn_pus)
	{
		if (Engine->uiPowerUpStore == NULL)
		{
			Engine->uiPowerUpStore->OpenPowerUpStore();
		}
		Engine->uiPowerUpStore->UpdateItemList(Engine->m_UiMgr->item_list, Engine->m_UiMgr->cat_list);
		Engine->uiPowerUpStore->Open();
	}
	else if (pSender == (DWORD*)btn_ticket)
	{
		if (Engine->uiSupport != NULL)
		{
			Engine->uiSupport->OpenSupport();
		}

	}
	else if (pSender == (DWORD*)btn_searchdrop)
	{
		if (Engine->uiSearchMonster != NULL)
		{
			Engine->uiSearchMonster->OpenWithAnimation();
		}
		
	}
	else if (pSender == (DWORD*)btn_wof)
	{
		Engine->uiWheel->OpenWheel();
	}
	else if (pSender == (DWORD*)btn_scheduler) {

		if (Engine->uiEventShowList)
		{
			Engine->uiEventShowList->InitList();
			Engine->uiEventShowList->Open();
		}
	}
	else if (pSender == (DWORD*)btn_daily_event) {

		if (Engine->uiEventShowList)
		{
			Engine->uiEventShowList->InitList();
			Engine->uiEventShowList->Open();
		}
	}
	//else if (pSender == (DWORD*)btn_daily_quest) {
	//	if (Engine->uiQuestPage != NULL)
	//	{

	//		Engine->uiQuestPage->InitQuests(Engine->uiQuestPage->page);
	//		Engine->SetVisible(Engine->uiQuestPage->el_Base, true);
	//		Engine->UIScreenCenter(Engine->uiQuestPage->el_Base);
	//	}
	//}
	else if (pSender == (DWORD*)btn_discord)
	{
		ShellExecute(NULL, "open", DiscordURL.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}
	else if (pSender == (DWORD*)btn_facebook)
	{
		ShellExecute(NULL, "open", FacebookURL.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}
#if (HOOK_SOURCE_VERSION == 1098)
	else if (pSender == (DWORD*)btn_rank)
		if (Engine->uiTaskbarMain)Engine->uiTaskbarMain->OpenSeed();
#endif


	return true;
}

void CUIHPBarMenuPlug::UpdateHpMenuVisible(Packet& pkt)
{

	uint8_t Facebook = 0, Discord = 0, Live = 0;

	pkt.DByte();
	pkt >> Facebook >> FacebookURL >> Discord >> DiscordURL >> Live >> LiveURL >> ResellerURL;

	Engine->SetVisible(btn_discord,Discord);


	Engine->SetVisible(btn_facebook,Facebook);

	if(Engine->uiTopRightNewPlug!=NULL)
	Engine->uiTopRightNewPlug->UpdateTopLeftVisible(Live, LiveURL);
}

void CUIHPBarMenuPlug::UpdatePosition()
{
	
	if (Engine->uiHPBarPlug != NULL)
	{
		POINT eventNoticePos;
		Engine->GetUiPos(Engine->uiHPBarPlug->m_dVTableAddr, eventNoticePos);

		RECT reg = Engine->GetUiRegion(Engine->uiHPBarPlug->m_dVTableAddr);

		POINT myPos;
#if (HOOK_SOURCE_VERSION == 1098)
		myPos.x = reg.left + (reg.right - reg.left) - 70;
		myPos.y = reg.top;
#else
		myPos.x = reg.left + (reg.right - reg.left) - 32;
		myPos.y = reg.top + 15;
#endif
		Engine->SetUIPos(m_dVTableAddr, myPos);
	}
}

void __stdcall UIHpMenuReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiHpMenuPlug->ReceiveMessage(pSender, dwMsg);
	__asm
	{
		MOV ECX, uiHpMenu
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_HpMenu
		CALL EAX
	}
}

void CUIHPBarMenuPlug::OpenHpBarMenu()
{
	
	Engine->SetVisible(m_dVTableAddr, true);
	UpdatePosition();
}
void CUIHPBarMenuPlug::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_HpMenu = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)UIHpMenuReceiveMessage_Hook;
}