#include "stdafx.h"
#include "UITaskbarMain.h"

CUITaskbarMainPlug::CUITaskbarMainPlug()
{
	vector<int>offsets;
	offsets.push_back(0x3D8);
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);
	m_btnPPCard = 0;
	m_mobdrop = 0;
	m_dailyquest = 0;
	btn_wof = 0;
	base_menu = 0;
	ParseUIElements();
	InitReceiveMessage();
	UpdatePosition();
}
extern DWORD KO_UI_SCREEN_SIZE;
void CUITaskbarMainPlug::UpdatePosition()
{
#if (HOOK_SOURCE_VERSION == 1098)

	POINT koScreen = *(POINT*)KO_UI_SCREEN_SIZE;
	RECT region = Engine->GetUiRegion(m_dVTableAddr);
	POINT orta;
	LONG w = (region.right - region.left);
	orta.x = (koScreen.x / 2) - 513;
	orta.y = region.top;
	Engine->SetUIPos(m_dVTableAddr, orta);
#endif
}

CUITaskbarMainPlug::~CUITaskbarMainPlug()
{
}

void CUITaskbarMainPlug::ParseUIElements()
{
	DWORD parent = 0;

	base_menu = Engine->GetChildByID(m_dVTableAddr, "base_menu");
	str_exp = Engine->GetChildByID(m_dVTableAddr, "str_exp");
	m_btnMenu = Engine->GetChildByID(base_menu, "btn_menu");
	m_btnRank = Engine->GetChildByID(base_menu, "btn_rank");

#if (HOOK_SOURCE_VERSION != 1098)	
	m_mobdrop = Engine->GetChildByID(base_menu, "btn_mobsearch");
	btn_wof = Engine->GetChildByID(base_menu, "btn_wof");
	m_dailyquest = Engine->GetChildByID(base_menu, "btn_daily_quest");
#endif
	base_exp = Engine->GetChildByID(m_dVTableAddr, "base_exp");
	base_TaskBar = Engine->GetChildByID(m_dVTableAddr, "base_TaskBar");
	m_btn00Stand = Engine->GetChildByID(base_TaskBar, "btn_00");
	m_btn01Sit = Engine->GetChildByID(base_TaskBar, "btn_01");
	m_btn02Seek = Engine->GetChildByID(base_TaskBar, "btn_02");
	m_btn03Trade = Engine->GetChildByID(base_TaskBar, "btn_03");
	m_btn04SkillK = Engine->GetChildByID(base_TaskBar, "btn_04");
	m_btn05Character = Engine->GetChildByID(base_TaskBar, "btn_05");
	m_btn06inventory = Engine->GetChildByID(base_TaskBar, "btn_06");
#if (HOOK_SOURCE_VERSION == 1098)	
	base_oldbuttons = Engine->GetChildByID(m_dVTableAddr, "base_oldbuttons");
	m_btninventory = Engine->GetChildByID(base_oldbuttons, "btn_inventory");
	m_btncharacter = Engine->GetChildByID(base_oldbuttons, "btn_character");
	m_btnskill = Engine->GetChildByID(base_oldbuttons, "btn_skill");
	m_btntrade = Engine->GetChildByID(base_oldbuttons, "btn_trade");
	m_btncommand = Engine->GetChildByID(base_oldbuttons, "btn_command");
	m_btninvite = Engine->GetChildByID(base_oldbuttons, "btn_invite");
	Engine->SetVisible(m_btninvite, true);
	Engine->SetState(m_btninvite, UI_STATE_BUTTON_NORMAL);

	m_btnattack = Engine->GetChildByID(base_oldbuttons, "btn_attack");
	m_btnwalk = Engine->GetChildByID(base_oldbuttons, "btn_walk");
	m_btnrun = Engine->GetChildByID(base_oldbuttons, "btn_run");
	m_btnsit = Engine->GetChildByID(base_oldbuttons, "btn_sit");

	Engine->SetVisible(m_btnsit, true);
	Engine->SetState(m_btnsit, UI_STATE_BUTTON_NORMAL);

	m_btnstand = Engine->GetChildByID(base_oldbuttons, "btn_stand");

	Engine->SetVisible(m_btnstand, false);
	Engine->SetState(m_btnstand, UI_STATE_BUTTON_DISABLE);

	m_btncamera = Engine->GetChildByID(base_oldbuttons, "btn_camera");
	m_btndisband = Engine->GetChildByID(base_oldbuttons, "btn_disband");

	Engine->SetVisible(m_btndisband, false);
	Engine->SetState(m_btndisband, UI_STATE_BUTTON_DISABLE);

	m_btnexit = Engine->GetChildByID(base_oldbuttons, "btn_exit");
	m_btnmap = Engine->GetChildByID(base_oldbuttons, "btn_map");
#endif
}


DWORD uiTaskbarMainVTable;
DWORD Func_Taskbar = 0;
void __stdcall UITaskbarMainReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiTaskbarMain->ReceiveMessage(pSender, dwMsg);
	__asm
	{
		MOV ECX, uiTaskbarMainVTable
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_Taskbar
		CALL EAX
	}
}

void CUITaskbarMainPlug::OpenSeed()
{
	UITaskbarMainReceiveMessage_Hook((DWORD*)m_btnRank, UIMSG_BUTTON_CLICK);
}

bool CUITaskbarMainPlug::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	uiTaskbarMainVTable = m_dVTableAddr;
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

#if (HOOK_SOURCE_VERSION == 1098)	
	else if (pSender == (DWORD*)m_btnmap)
	{
		if (Engine->uiTaskbarSub != NULL)
		{
			if (Engine->uiMinimapPlug) {
				if(DWORD map = Engine->uiMinimapPlug->m_dVTableAddr) {
					if (*(bool*)(map + 0xF9))
					{
						__asm {
							pushad
							pushfd
							mov ecx, map
							mov eax, 0x846C40
							call eax
							popfd
							popad
						}
					}
					else {
						__asm {
							pushad
							pushfd
							mov ecx, map
							mov eax, 0x846E00
							call eax
							popfd
							popad
						}
					}
				}
			}
		}
	}
	else if (pSender == (DWORD*)m_btninventory)
	{
		if (Engine->uiTaskbarMain != NULL)
		{
			Engine->uiTaskbarMain->OpenInventory();
		}
	}
	else if (pSender == (DWORD*)m_btncharacter)
	{
		if (Engine->uiTaskbarMain != NULL)
		{
			Engine->uiTaskbarMain->OpenCharacterU();
		}
	}
	else if (pSender == (DWORD*)m_btnskill)
	{
		if (Engine->uiTaskbarMain != NULL)
		{
			Engine->uiTaskbarMain->OpenSkill();
		}
	}
	else if (pSender == (DWORD*)m_btnstand)
	{
		if (Engine->uiTaskbarMain != NULL)
		{
			Engine->uiTaskbarMain->OpenStand();
		}
	}
	else if (pSender == (DWORD*)m_btnsit)
	{
		if (Engine->uiTaskbarMain != NULL)
		{
			Engine->uiTaskbarMain->OpenSit();
		}
	}
	else if (pSender == (DWORD*)m_btntrade)
	{
		if (Engine->uiTaskbarMain != NULL)
		{
			Engine->uiTaskbarMain->OpenTrade();
		}
	}
	else if (pSender == (DWORD*)m_btncommand)
	{
		//--------------------------- F10 Setting açýlýyor.
		/*if (Engine->uiSeedHelperPlug == NULL || Engine->uiSeedHelperPlug->m_dVTableAddr == NULL)
			return true;

		DWORD tmp = Engine->uiSeedHelperPlug->m_dVTableAddr;
		__asm {
			push 1
			mov ecx, tmp
			mov eax, 0x804260
			call eax
		}*/

		if (Engine->uiTaskbarSub != NULL)
		{
			Engine->uiTaskbarSub->OpenHotkeyH();
		}
	}
	else if (pSender == (DWORD*)m_btninvite)
	{
		if (Engine->uiTaskbarSub != NULL)
		{
			Engine->uiTaskbarSub->OpenInviteParty();
		}
	}
	else if (pSender == (DWORD*)m_btnexit)
	{
		if (Engine->uiTaskbarSub != NULL)
		{
			Engine->uiTaskbarSub->OpenExitGame();
		}
	}
	else if (pSender == (DWORD*)m_btndisband)
	{
		if (Engine->uiPartyBBS != NULL)
		{
			Engine->uiPartyBBS->OpenPartyDisband();
		}
	}
	else if (pSender == (DWORD*)m_btnrun)
	{
		Engine->ChangeRunState();
		Engine->SetVisible(m_btnrun, false);
		Engine->SetVisible(m_btnwalk, true);
	}
	else if (pSender == (DWORD*)m_btnwalk)
	{
		Engine->ChangeRunState();
		Engine->SetVisible(m_btnrun, true);
		Engine->SetVisible(m_btnwalk, false);
	}
	else if (pSender == (DWORD*)m_btncamera)
	{
		Engine->CommandCameraChange();
	}
	else if (pSender == (DWORD*)m_btnattack)   
	{
		if (DWORD target = Engine->GetTarget()) {
			__asm {	
				push target
				push 1
				mov ecx, [KO_DLG]
				mov ecx, [ecx]
				mov eax, 0x00535550
				call eax
			}
		}
	}
#else
if (pSender == (DWORD*)m_mobdrop)
{
	if (Engine->uiSearchMonster != NULL)
		Engine->m_UiMgr->OpenMonsterSearch();

}
else if (pSender == (DWORD*)m_dailyquest) {
	if (Engine->uiQuestPage != NULL)
	{

		Engine->uiQuestPage->InitQuests(Engine->uiQuestPage->page);
		Engine->SetVisible(Engine->uiQuestPage->m_dVTableAddr, true);
		Engine->UIScreenCenter(Engine->uiQuestPage->m_dVTableAddr);
	}
}
else if (pSender == (DWORD*)btn_wof)
{
	Engine->uiWheel->OpenWheel();
}
#endif
	return true;
}

void CUITaskbarMainPlug::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_Taskbar = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)UITaskbarMainReceiveMessage_Hook;
}

void CUITaskbarMainPlug::OpenPPCard()
{
	UITaskbarMainReceiveMessage_Hook((DWORD*)m_btnPPCard, UIMSG_BUTTON_CLICK);
}

#if (HOOK_SOURCE_VERSION == 1098)	
void CUITaskbarSubPlug::OpenSeedHelper()
{
	UITaskbarMainReceiveMessage_Hook((DWORD*)m_btnQuestion, UIMSG_BUTTON_CLICK);
}

void CUITaskbarMainPlug::OpenMapButton()
{
	UITaskbarMainReceiveMessage_Hook((DWORD*)m_btnMenu, UIMSG_BUTTON_CLICK);
}
void CUITaskbarMainPlug::OpenStand()
{
	UITaskbarMainReceiveMessage_Hook((DWORD*)m_btn00Stand, UIMSG_BUTTON_CLICK);
}
void CUITaskbarMainPlug::OpenSit()
{
	UITaskbarMainReceiveMessage_Hook((DWORD*)m_btn01Sit, UIMSG_BUTTON_CLICK);
}
void CUITaskbarMainPlug::OpenTrade()
{
	UITaskbarMainReceiveMessage_Hook((DWORD*)m_btn03Trade, UIMSG_BUTTON_CLICK);
}
void CUITaskbarMainPlug::OpenSkill()
{
	UITaskbarMainReceiveMessage_Hook((DWORD*)m_btn04SkillK, UIMSG_BUTTON_CLICK);
}
void CUITaskbarMainPlug::OpenCharacterU()
{
	UITaskbarMainReceiveMessage_Hook((DWORD*)m_btn05Character, UIMSG_BUTTON_CLICK);
}
void CUITaskbarMainPlug::OpenInventory()
{
	UITaskbarMainReceiveMessage_Hook((DWORD*)m_btn06inventory, UIMSG_BUTTON_CLICK);
}
#endif