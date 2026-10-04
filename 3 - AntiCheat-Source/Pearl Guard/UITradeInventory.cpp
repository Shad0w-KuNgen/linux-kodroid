#include "stdafx.h"
#include "UITradeInventory.h"

CUITradeInventoryPlug::CUITradeInventoryPlug()
{
	vector<int>offsets;
	offsets.push_back(0x2C4);
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);

	m_txtTotal = 0;
	val = 0;
	lastState = 0;
	base_mid = 0;
	base_bot = 0;
	base_mid = 0;
	lastVisible = false;

	ParseUIElements();
}

CUITradeInventoryPlug::~CUITradeInventoryPlug()
{
}

void CUITradeInventoryPlug::ParseUIElements()
{
	base_top = Engine->GetChildByID(m_dVTableAddr, "base_top");
	base_bot = Engine->GetChildByID(m_dVTableAddr, "base_bot");
	base_mid = Engine->GetChildByID(m_dVTableAddr, "base_mid");
	text_sell_KnightCash = Engine->GetChildByID(base_mid, "text_sell_KnightCash");
	text_sell_money = Engine->GetChildByID(base_mid, "text_sell_money");

	if (base_top) {
		for (int i = 0; i < 12; i++)
		{
			DWORD curVTable = Engine->GetChildByID(base_top, string_format(xorstr("at%d"), i));
			if (curVTable != 0)
			{
				CMerchantInvSlot* newSlot = new CMerchantInvSlot(i, curVTable);
				m_Slots.push_back(newSlot);
			}
		}
	}
}

void CUITradeInventoryPlug::OnOpen()
{
	if (lastState == 0) val = Engine->Player.KnightCash;
	UpdateTotal(val, lastState == 0 ? true : false, false);
}

void CUITradeInventoryPlug::OnClose()
{
	val = 0;
}

void CUITradeInventoryPlug::UpdateTotal(int32 value, bool iskc, bool add)
{
	if ((lastState == 0 && !iskc) || (lastState == 1 && iskc)) {
		val = 0;
		lastState = !lastState;
	}

	if (add && iskc)
		val += value;
	else if (iskc)
		val = value;

	std::string strValue = Engine->StringHelper->NumberFormat(val);
	Engine->SetString(m_txtTotal, strValue);
}

void CUITradeInventoryPlug::Tick()
{
	if (m_dVTableAddr != 0) {
		if (lastVisible != Engine->IsVisible(m_dVTableAddr))
		{
			lastVisible = !lastVisible;
			if (lastVisible)
				OnOpen();
			else
				OnClose();
		}
	}
}

DWORD CUITradeInventoryPlug::GetSlotVTable(int id)
{
	for (MerchantInvSlotListItor itor = m_Slots.begin(); m_Slots.end() != itor; ++itor)
	{
		CMerchantInvSlot* slot = (*itor);

		if (slot->m_iID == id)
			return slot->m_dVTableAddr;
	}

	return 0;
}