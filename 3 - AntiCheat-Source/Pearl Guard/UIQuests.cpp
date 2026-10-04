#include "UIQuests.h"
#include <math.h>

CUIQuestsPlug::CUIQuestsPlug()
{
	vector<int>offsets;
	offsets.push_back(0x1D4);
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);

	
	for (uint8 i = 0; i < 12; i++)
	{
		btn_quest[i] = NULL;
		str_status[i] = NULL;
		str_time[i] = NULL;
	}

	ParseUIElements();
	InitReceiveMessage();
}

CUIQuestsPlug::~CUIQuestsPlug()
{
}

void CUIQuestsPlug::ParseUIElements()
{
	std::string find = xorstr("Btn_gkhn_info"); //14.11.2020 Hp Bar User Info Icon
	Engine->GetChildByID(m_dVTableAddr, find, m_gkhn_info);
	find = xorstr("Btn_Drops");
	Engine->GetChildByID(m_dVTableAddr, find, m_btnDrop);
	Engine->SetVisible(m_btnDrop, false);
	Engine->SetState(m_btnDrop, UI_STATE_BUTTON_DISABLE);
	find = xorstr("Text_Health");
	Engine->GetChildByID(m_dVTableAddr, find, m_dTextTargetHp);
}

DWORD uiQuestsVTable;

bool CUIQuestsPlug::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	uiQuestsVTable = m_dVTableAddr;
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	
	return true;
}

void __stdcall UiQuestsReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiTargetBar->ReceiveMessage(pSender, dwMsg);
}

void CUIQuestsPlug::InitReceiveMessage()
{
	*(DWORD*)KO_UI_TARGETBAR_RECEIVE_MESSAGE_PTR = (DWORD)UiQuestsReceiveMessage_Hook;
}
