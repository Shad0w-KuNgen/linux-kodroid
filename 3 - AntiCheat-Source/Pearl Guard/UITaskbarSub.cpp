#include "stdafx.h"
#include "UITaskbarSub.h"

CUITaskbarSubPlug::CUITaskbarSubPlug()
{
	vector<int>offsets;
	offsets.push_back(0x3DC);
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);
	m_btnPowerUPStore = 0;
	m_mobdrop = 0;
	btn_wof = 0;
	m_dailyquest = 0;
	m_btnbug = NULL;
	m_btnQuestion = 0;

	ParseUIElements();
	InitReceiveMessage();
}

CUITaskbarSubPlug::~CUITaskbarSubPlug()
{

}

void CUITaskbarSubPlug::ParseUIElements()
{
	m_btnPowerUPStore = Engine->GetChildByID(m_dVTableAddr, "btns_pus");
	m_btnHotkeyH = Engine->GetChildByID(m_dVTableAddr, "btn_hotkey");
	m_btnGlobalMap = Engine->GetChildByID(m_dVTableAddr, "btn_globalmap");
	m_btnInviteParty = Engine->GetChildByID(m_dVTableAddr, "btn_party");
	m_btnExitGame = Engine->GetChildByID(m_dVTableAddr, "btn_exit");	
}

DWORD uiTaskbarSubVTable;
bool CUITaskbarSubPlug::ReceiveMessage(DWORD * pSender, uint32_t dwMsg)
{
	uiTaskbarSubVTable = m_dVTableAddr;
	if (!pSender || pSender == 0)
		return false;

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
	if (pSender == (DWORD*)m_btnbug)
	{
		if (Engine->uiState == NULL)
		{
			Engine->m_UiMgr->ShowMessageBox(xorstr("Error"), xorstr("Support is available only after the game has started."), Ok);
			return true;
		}
		
		if (Engine->uiSupport)
			Engine->uiSupport->OpenSupport();
	}
	else if (pSender == (DWORD*)btn_wof)
	{
		Engine->uiWheel->OpenWheel();
	}

	return true;
}

void __stdcall UITaskbarSubReceiveMessage_Hook(DWORD * pSender, uint32_t dwMsg)
{
	Engine->uiTaskbarSub->ReceiveMessage(pSender, dwMsg);
	__asm
	{
		MOV ECX, uiTaskbarSubVTable
		PUSH dwMsg
		PUSH pSender
		MOV EAX, KO_UI_TASKBAR_SUB_RECEIVE_MESSAGE_FUNC
		CALL EAX
	}
}

void CUITaskbarSubPlug::InitReceiveMessage()
{
	
	*(DWORD*)KO_UI_TASKBAR_SUB_RECEIVE_MESSAGE_PTR = (DWORD)UITaskbarSubReceiveMessage_Hook;
}
void CUITaskbarSubPlug::OpenGlobalMap()
{
	UITaskbarSubReceiveMessage_Hook((DWORD*)m_btnGlobalMap, UIMSG_BUTTON_CLICK);
}
void CUITaskbarSubPlug::OpenInviteParty()
{
	UITaskbarSubReceiveMessage_Hook((DWORD*)m_btnInviteParty, UIMSG_BUTTON_CLICK);
}
void CUITaskbarSubPlug::OpenHotkeyH()
{
	UITaskbarSubReceiveMessage_Hook((DWORD*)m_btnHotkeyH, UIMSG_BUTTON_CLICK);
}
void CUITaskbarSubPlug::OpenExitGame()
{
	UITaskbarSubReceiveMessage_Hook((DWORD*)m_btnExitGame, UIMSG_BUTTON_CLICK);
}