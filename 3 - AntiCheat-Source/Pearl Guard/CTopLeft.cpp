#include "CTopLeft.h"
#include "RC5/RC5.h"

CTopLeft::CTopLeft()
{
	timeLapse = 0;
	posSet = false;
	btn_scheduler = 0;
	text_online_time = NULL;
}

CTopLeft::~CTopLeft()
{
	text_online_time = NULL;
}

bool CTopLeft::Load(HANDLE hFile)
{
	if (CN3UIBase::Load(hFile) == false)
		return false;

	std::string find = xorstr("");

#if (HOOK_SOURCE_VERSION == 1098)	
	find = xorstr("btn_pus");
	btn_pus = (CN3UIButton*)GetChildByID(find);
	find = xorstr("btn_searchdrop");
	btn_searchdrop = (CN3UIButton*)GetChildByID(find);
	find = xorstr("btn_ticket");
	btn_ticket = (CN3UIButton*)GetChildByID(find);
	
	find = xorstr("btn_scheduler");
	btn_scheduler = (CN3UIButton*)GetChildByID(find);
	find = xorstr("btn_wof");
	btn_wof = (CN3UIButton*)GetChildByID(find);

#else
	find = xorstr("btn_discord");
	btn_discord = (CN3UIButton*)GetChildByID(find);
	find = xorstr("btn_facebook");
	btn_facebook = (CN3UIButton*)GetChildByID(find);
	find = xorstr("text_online_time");
	text_online_time = (CN3UIString*)GetChildByID(find);
	btn_discord->SetVisible(false);
	btn_discord->SetState(UI_STATE_BUTTON_DISABLE);
	btn_facebook->SetVisible(false);
	btn_facebook->SetState(UI_STATE_BUTTON_DISABLE);
#endif
	SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, Engine->m_UiMgr->GetScreenCenter(this).y);
	//Engine->m_UiMgr->SetFocusedUI(this);

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
		myPos.x = reg.left + (reg.right - reg.left) - 25;
		myPos.y = reg.top + 15;
#endif
		SetPos(myPos.x, myPos.y);
	}
	return true;
}

void CTopLeft::Tick()
{
#if (HOOK_SOURCE_VERSION != 1098)	
	if (text_online_time && Engine->m_bGameStart)
	{
		if (timeLapse > clock() - 1000)
			return;

		timeLapse = clock();

		Engine->timesPassed++;

		if (Engine->timesPassed > 60 * 60)
			text_online_time->SetString(string_format("%d saat", Engine->timesPassed / 60 / 60));
		if(Engine->timesPassed > 60)
			text_online_time->SetString(string_format("%d dk", Engine->timesPassed / 60));
		else
			text_online_time->SetString(string_format("%d sn", Engine->timesPassed));
	}
#endif
}

bool CTopLeft::ReceiveMessage(CN3UIBase * pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
#if (HOOK_SOURCE_VERSION == 1098)	
		if (pSender == btn_pus)
		{
			if (Engine->m_UiMgr->uiPowerUpStore == NULL)
			{
				Engine->m_UiMgr->uiPowerUpStore->OpenPowerUpStore();
			}
			Engine->m_UiMgr->uiPowerUpStore->UpdateItemList(Engine->m_UiMgr->item_list, Engine->m_UiMgr->cat_list);
			Engine->m_UiMgr->uiPowerUpStore->Open();
		}
		else if (pSender == btn_ticket)
		{
			if (Engine->m_UiMgr->uiSupport == NULL)
			{
				Engine->m_UiMgr->uiSupport->OpenSupport();
			}
			Engine->m_UiMgr->uiSupport->OpenWithAnimation();
		}
		else if (pSender == btn_searchdrop)
		{
			if (Engine->m_UiMgr->uiSearchMonster == NULL)
			{
				Engine->m_UiMgr->OpenMonsterSearch();
			}
			Engine->m_UiMgr->uiSearchMonster->OpenWithAnimation();
		}
		else if (pSender == btn_wof)
		{
			Engine->m_UiMgr->uiWheel->OpenWheel();
	
	
	
		}
		else if (pSender == btn_scheduler) {
			if (Engine->m_UiMgr->uiEventShowList == NULL)
				Engine->m_UiMgr->OpenScheduler();

			Engine->m_UiMgr->uiEventShowList->InitList();
			Engine->m_UiMgr->uiEventShowList->Open();
		}

		/*else if (pSender == btn_dailyquest)
		{
			if (Engine->m_UiMgr->uixDailyQuest == NULL)
			{
				Engine->m_UiMgr->OpenDailyQuest();
			}
			Engine->m_UiMgr->uixDailyQuest->Open();
		}*/
#else
		if (pSender == btn_discord)
		{
			ShellExecute(NULL, "open", DiscordURL.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
		else if (pSender == btn_facebook)
		{
			ShellExecute(NULL, "open", FacebookURL.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
#endif
	}
	return true;
}

uint32_t CTopLeft::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
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
		myPos.x = reg.left + (reg.right - reg.left) - 25;
		myPos.y = reg.top + 15;
#endif
		SetPos(myPos.x, myPos.y);
	}
	dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
	return dwRet;
}

bool CTopLeft::OnKeyPress(int iKey)
{
	if (!IsVisible())
		return CN3UIBase::OnKeyPress(iKey);

	return true;
}

void CTopLeft::Open()
{

	SetVisible(true);
}

void CTopLeft::Close()
{
	SetVisible(false);
}

void CTopLeft::UpdatePosition()
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
		myPos.x = reg.left + (reg.right - reg.left) - 25;
		myPos.y = reg.top + 15;
#endif
		SetPos(myPos.x, myPos.y);
	}
}

void CTopLeft::UpdateTopLeftVisible(Packet& pkt)
{

	uint8_t Facebook = 0, Discord = 0, Live = 0;
	std::string LiveURL = "";
	pkt.DByte();
	pkt >> Facebook >> FacebookURL >> Discord >> DiscordURL >> Live >> LiveURL;

	if (Engine->uiTopRightNewPlug != NULL)
		Engine->uiTopRightNewPlug->UpdateTopLeftVisible(Live, LiveURL);
#if (HOOK_SOURCE_VERSION != 1098)
	btn_discord->SetVisible(Discord);
	btn_discord->SetState(Discord ? eUI_STATE::UI_STATE_BUTTON_NORMAL : eUI_STATE::UI_STATE_BUTTON_DISABLE);

	btn_facebook->SetVisible(Facebook);
	btn_facebook->SetState(Facebook ? eUI_STATE::UI_STATE_BUTTON_NORMAL : eUI_STATE::UI_STATE_BUTTON_DISABLE);
#endif
}

void CTopLeft::FPS()
{
	/*float fps = *(float*)0x00D894D8;
	if (Engine->m_UiMgr->uiTopLeft != NULL)
		text_FPS->SetString(string_format("fps : %d", static_cast<int>(fps)));*/
}

void CTopLeft::OpenTopLeft()
{
#if (HOOK_SOURCE_VERSION == 1098)
	uint8 Nation = *(uint8*)(*(DWORD*)KO_PTR_CHR + KO_OFF_NATION);
	if (Nation == 1)
	{
		std::string name = Engine->dcpUIF(xorstr("Msoft\\Ka_TopLeft.Msoft"));
		Engine->m_UiMgr->uiTopLeft = new CTopLeft();
		Engine->m_UiMgr->uiTopLeft->Init(this);
		Engine->m_UiMgr->uiTopLeft->LoadFromFile(Engine->m_BasePath + name.c_str(), N3FORMAT_VER_1068);
		Engine->m_UiMgr->AddChild(Engine->m_UiMgr->uiTopLeft);
		remove(name.c_str());
	}
	else if (Nation == 2)
	{
		std::string name = Engine->dcpUIF(xorstr("Msoft\\El_TopLeft.Msoft"));
		Engine->m_UiMgr->uiTopLeft = new CTopLeft();
		Engine->m_UiMgr->uiTopLeft->Init(this);
		Engine->m_UiMgr->uiTopLeft->LoadFromFile(Engine->m_BasePath + name.c_str(), N3FORMAT_VER_1068);
		Engine->m_UiMgr->AddChild(Engine->m_UiMgr->uiTopLeft);
		remove(name.c_str());
	}
#else
	std::string name = Engine->dcpUIF(xorstr("Msoft\\topleft.Msoft"));
	Engine->m_UiMgr->uiTopLeft = new CTopLeft();
	Engine->m_UiMgr->uiTopLeft->Init(this);
	Engine->m_UiMgr->uiTopLeft->LoadFromFile(Engine->m_BasePath + name.c_str(), N3FORMAT_VER_1068);
	Engine->m_UiMgr->AddChild(Engine->m_UiMgr->uiTopLeft);
	remove(name.c_str());
#endif
}
