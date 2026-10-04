#include "stdafx.h"
#include "UITradePrice.h"

CUITradePricePlug::CUITradePricePlug()
{
	vector<int>offsets;
	offsets.push_back(0x50);
	offsets.push_back(0);
	offsets.push_back(0x274);
	offsets.push_back(0x334);
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);

	m_bIsKC = false;
	m_btnKC = 0;

	ParseUIElements();
	InitReceiveMessage();
}

CUITradePricePlug::~CUITradePricePlug()
{
}

void CUITradePricePlug::Reset()
{
	m_bIsKC = false;
}

void CUITradePricePlug::ParseUIElements()
{
	m_btnKC = Engine->GetChildByID(m_dVTableAddr, "btn_iskc");
	m_edit_money = Engine->GetChildByID(m_dVTableAddr, "edit_money");
	m_txtMoney = Engine->GetChildByID(m_edit_money, "text_money");
}

DWORD merchantTradePriceVTable;
bool CUITradePricePlug::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	merchantTradePriceVTable = m_dVTableAddr;
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	if (pSender == (DWORD*)m_btnKC)
	{
		m_bIsKC = !m_bIsKC;
	}

	return true;
}

bool updateRequired = false;

void CUITradePricePlug::Tick()
{
	if (Engine->IsVisible(m_dVTableAddr))
	{
		if (updateRequired) {
			updateRequired = false;
			Engine->SetState(m_btnKC, m_bIsKC ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);
		}
	} else updateRequired = true;
}

void __stdcall MerchantTradePriceReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiTradePrice->ReceiveMessage(pSender, dwMsg);

	__asm
	{
		MOV ECX, merchantTradePriceVTable
		PUSH dwMsg
		PUSH pSender
		MOV EAX, KO_UI_MERCHANT_PRICE_RECEIVE_MESSAGE_FUNC
		CALL EAX
	}
}

void CUITradePricePlug::InitReceiveMessage()
{
	*(DWORD*)KO_UI_MERCHANT_PRICE_RECEIVE_MESSAGE_PTR = (DWORD)MerchantTradePriceReceiveMessage_Hook;
}