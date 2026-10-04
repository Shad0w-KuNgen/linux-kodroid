#include "stdafx.h"
#include "UIInformationWind.h"
#include "Pearl Engine.h"

void UIMerchantWindTick()
{
	while (true)
	{
		if (Engine->m_bGameStart && Engine != NULL && Engine->uiInformationWind != NULL && Engine->uiInformationWind->merchant_state)
			Engine->uiInformationWind->MerchantBtnSetState();

		Sleep(500);
	}
}

void UINoticeWindTick()
{
	while (true)
	{
		if (Engine->m_bGameStart && Engine != NULL && Engine->uiInformationWind != NULL && Engine->uiInformationWind->notice_state)
			Engine->uiInformationWind->NoticeBtnSetState();

		Sleep(500);
	}
}

CUIInformationWindPlug::CUIInformationWindPlug()
{
	vector<int>offsets;
	offsets.push_back(0x214);
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);

	if (m_dVTableAddr == 0)
		return;

	merchant_state = notice_state = false;
	ParseUIElements();
	InitReceiveMessage();
	CreateThread(NULL, NULL, LPTHREAD_START_ROUTINE(UINoticeWindTick), NULL, 0, 0);
	CreateThread(NULL, NULL, LPTHREAD_START_ROUTINE(UIMerchantWindTick), NULL, 0, 0);
}

CUIInformationWindPlug::~CUIInformationWindPlug()
{

}

void CUIInformationWindPlug::ParseUIElements()
{
	DWORD parent = 0;
	parent = Engine->GetChildByID(m_dVTableAddr, "base_msgwnd");
	btn_merchant = Engine->GetChildByID(parent, "btn_Merchant");
	btn_Notice = Engine->GetChildByID(parent, "btn_notice");
}

DWORD uiInformationWindVTable;
bool CUIInformationWindPlug::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	uiInformationWindVTable = m_dVTableAddr;
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	if (pSender == (DWORD*)btn_merchant)
	{
		merchant_state = false;
		Engine->uiNoticeWind->Open(true);
		Engine->SetState(btn_merchant, UI_STATE_BUTTON_NORMAL);
	}
	else if (pSender == (DWORD*)btn_Notice)
	{
		notice_state = false;
		Engine->uiNoticeWind->Open();
				Engine->SetState(btn_Notice, UI_STATE_BUTTON_NORMAL);
	}

	return true;
}

DWORD Func_InfoBox = 0;

void __stdcall UIInformationWindReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiInformationWind->ReceiveMessage(pSender, dwMsg);
	__asm
	{
		MOV ECX, uiInformationWindVTable
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_InfoBox
		CALL EAX
	}
}

void CUIInformationWindPlug::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_InfoBox = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)UIInformationWindReceiveMessage_Hook;
}

void CUIInformationWindPlug::MerchantBtnSetState()
{
	Engine->SetState(btn_merchant, UI_STATE_BUTTON_DOWN);
	Sleep(1000);
	Engine->SetState(btn_merchant, UI_STATE_BUTTON_NORMAL);
}

void CUIInformationWindPlug::NoticeBtnSetState()
{
	Engine->SetState(btn_Notice, UI_STATE_BUTTON_DOWN);
	Sleep(1200);
	Engine->SetState(btn_Notice, UI_STATE_BUTTON_NORMAL);
}