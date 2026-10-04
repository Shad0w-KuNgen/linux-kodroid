#pragma once
class CUITradeInventoryPlug;
#include "Models/MerchantInvSlot.h"
#include "Pearl Engine.h"

class CUITradeInventoryPlug
{
public:
	int64 val;
	uint8 lastState;
	DWORD m_dVTableAddr;
	DWORD base_top, base_bot, base_mid;
	DWORD m_txtTotal;
	DWORD text_sell_money;
	DWORD text_sell_KnightCash;
	MerchantInvSlotList m_Slots;
	bool lastVisible;
public:
	CUITradeInventoryPlug();
	~CUITradeInventoryPlug();
	void ParseUIElements();
	void UpdateTotal(int32 value, bool iskc = true, bool add = false);
	DWORD GetSlotVTable(int id);
	void Tick();
	void OnOpen();
	void OnClose();
private:

};