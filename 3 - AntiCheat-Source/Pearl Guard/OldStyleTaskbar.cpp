#include "OldStyleTaskbar.h"
#if (HOOK_SOURCE_VERSION == 1299)
COldStyleTaskbar::COldStyleTaskbar()
{
	m_btninventory = NULL;
	m_btncharacter = NULL;
	m_btnskill = NULL;
	m_btntrade = NULL;
	m_btncommand = NULL;
	m_btninvite = NULL;
	m_btnattack = NULL;
	m_btnwalk = NULL;
	m_btnrun = NULL;
	m_btnsit = NULL;
	m_btnstand = NULL;
	m_btncamera = NULL;
	m_btndisband = NULL;
	m_btnexit = NULL;
	m_btnmap = NULL;
	Ayaktami = true;
	Oturuyormu = false;
}

COldStyleTaskbar::~COldStyleTaskbar(){}

bool COldStyleTaskbar::Load(HANDLE hFile)
{
	if (CN3UIBase::Load(hFile) == false) return false;

	CN3UIBase * base_oldbutton = NULL;
	std::string find = xorstr("base_oldbuttons");
	base_oldbutton = (CN3UIBase*)GetChildByID(find);

	find = xorstr("btn_inventory");
	m_btninventory = (CN3UIButton*)base_oldbutton->GetChildByID(find);

	find = xorstr("btn_character");
	m_btncharacter = (CN3UIButton*)base_oldbutton->GetChildByID(find);

	find = xorstr("btn_skill");
	m_btnskill = (CN3UIButton*)base_oldbutton->GetChildByID(find);

	find = xorstr("btn_trade");
	m_btntrade = (CN3UIButton*)base_oldbutton->GetChildByID(find);

	find = xorstr("btn_command");	
	m_btncommand = (CN3UIButton*)base_oldbutton->GetChildByID(find);

	find = xorstr("btn_invite");
	m_btninvite = (CN3UIButton*)base_oldbutton->GetChildByID(find);

	find = xorstr("btn_attack");
	m_btnattack = (CN3UIButton*)base_oldbutton->GetChildByID(find);

	find = xorstr("btn_walk");
	m_btnwalk = (CN3UIButton*)base_oldbutton->GetChildByID(find);

	find = xorstr("btn_run");
	m_btnrun = (CN3UIButton*)base_oldbutton->GetChildByID(find);

	find = xorstr("btn_sit");
	m_btnsit = (CN3UIButton*)base_oldbutton->GetChildByID(find);

	find = xorstr("btn_stand");
	m_btnstand = (CN3UIButton*)base_oldbutton->GetChildByID(find);

	find = xorstr("btn_camera");
	m_btncamera = (CN3UIButton*)base_oldbutton->GetChildByID(find);

	find = xorstr("btn_disband");
	m_btndisband = (CN3UIButton*)base_oldbutton->GetChildByID(find);

	find = xorstr("btn_exit");
	m_btnexit = (CN3UIButton*)base_oldbutton->GetChildByID(find);

	find = xorstr("btn_map");
	m_btnmap = (CN3UIButton*)base_oldbutton->GetChildByID(find);

	RECT screen = Engine->m_UiMgr->GetScreenRect();

	SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, screen.bottom - Engine->GetUiHeight(Engine->uiTaskbarMain->m_dVTableAddr) - 40);
	//SetPos(Engine->GetUiHeight(Engine->uiTaskbarMain->m_dVTableAddr), Engine->GetUiWidth(Engine->uiTaskbarMain->m_dVTableAddr) / 2);
	Engine->m_UiMgr->SetFocusedUI(this);
	return true;
}

bool COldStyleTaskbar::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_btnmap)
		{
			if (Engine->uiTaskbarSub != NULL)
			{
				Engine->uiTaskbarSub->OpenGlobalMap();
			}
		}
		else if (pSender == m_btninventory)
		{
			if (Engine->uiTaskbarMain != NULL)
			{
				Engine->uiTaskbarMain->OpenInventory();
			}
		}
		else if (pSender == m_btncharacter)
		{
			if (Engine->uiTaskbarMain != NULL)
			{
				Engine->uiTaskbarMain->OpenCharacterU();
			}
		}
		else if (pSender == m_btnskill)
		{
			if (Engine->uiTaskbarMain != NULL)
			{
				Engine->uiTaskbarMain->OpenSkill();
			}
		}
		else if (pSender == m_btnstand)
		{
			if (Engine->uiTaskbarMain != NULL)
			{
				Engine->uiTaskbarMain->OpenStand();
			}
		}
		else if (pSender == m_btnsit)
		{
			if (Engine->uiTaskbarMain != NULL)
			{
				Engine->uiTaskbarMain->OpenSit();				
			}
		}
		else if (pSender == m_btntrade)
		{
			if (Engine->uiTaskbarMain != NULL)
			{
				Engine->uiTaskbarMain->OpenTrade();
			}
		}
		else if (pSender == m_btncommand)
		{
			if (Engine->uiTaskbarSub != NULL)
			{
				Engine->uiTaskbarSub->OpenHotkeyH();
			}
		}
		else if (pSender == m_btninvite)
		{
			if (Engine->uiTaskbarSub != NULL)
			{
				Engine->uiTaskbarSub->OpenInviteParty();
			}
		}
		else if (pSender == m_btnexit)
		{
			if (Engine->uiTaskbarSub != NULL)
			{
				Engine->uiTaskbarSub->OpenExitGame();
			}
		}
		else if (pSender == m_btndisband)
		{
		}
	}
	return true;
}

bool posSet = false;

uint32_t COldStyleTaskbar::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	/*if (Engine->m_UiMgr && Engine->m_UiMgr->uiOldStyleTaskbar != NULL && !posSet)
	{
		POINT eventNoticePos;
		Engine->GetUiPos(Engine->m_UiMgr->uiOldStyleTaskbar->m_dVTableAddr, eventNoticePos);
		posSet = true;
		SetPos(Engine->GetUiHeight(Engine->m_UiMgr->uiOldStyleTaskbar->m_dVTableAddr) + 200, 0);
	}*/
	dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
	return dwRet;
}

bool COldStyleTaskbar::OnKeyPress(int iKey)
{
	if (!IsVisible())
		return CN3UIBase::OnKeyPress(iKey);

	return CN3UIBase::OnKeyPress(iKey);
}

void COldStyleTaskbar::UpdatePosition()
{
	posSet = false;
	if (Engine->uiTaskbarMain != NULL && !posSet)
	{
		POINT eventNoticePos;
		Engine->GetUiPos(Engine->uiTaskbarMain->m_dVTableAddr, eventNoticePos);
		posSet = true;
		RECT screen = Engine->m_UiMgr->GetScreenRect();

		SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, screen.bottom - Engine->GetUiHeight(Engine->uiTaskbarMain->m_dVTableAddr));
		//SetPos(Engine->GetUiHeight(Engine->uiTaskbarMain->m_dVTableAddr), Engine->GetUiWidth(Engine->uiTaskbarMain->m_dVTableAddr) / 2);
	}
}

void COldStyleTaskbar::SitAndStand()
{
	if (Ayaktami == true)
	{
		m_btnsit->SetVisible(false);
		m_btnsit->SetState(UI_STATE_BUTTON_DOWN);
		m_btnsit->SetVisible(true);
		m_btnsit->SetState(UI_STATE_BUTTON_NORMAL);
	}
	else if (Oturuyormu == true)
	{
		m_btnsit->SetVisible(false);
		m_btnsit->SetState(UI_STATE_BUTTON_DOWN);
		m_btnstand->SetVisible(true);
		m_btnstand->SetState(UI_STATE_BUTTON_NORMAL);
	}
	else if (Ayaktami == false)
	{
		m_btnsit->SetVisible(false);
		m_btnsit->SetState(UI_STATE_BUTTON_DOWN);
		m_btnsit->SetVisible(true);
		m_btnsit->SetState(UI_STATE_BUTTON_NORMAL);
	}
	else if (Oturuyormu == false)
	{
		m_btnstand->SetVisible(false);
		m_btnstand->SetState(UI_STATE_BUTTON_DOWN);
		m_btnsit->SetVisible(true);
		m_btnsit->SetState(UI_STATE_BUTTON_NORMAL);
	}
}
#endif