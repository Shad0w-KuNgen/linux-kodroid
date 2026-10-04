#pragma once
class CUITradeItemDisplaySpecialPlug;
#include "Models/MerchantInvSlot.h"
#include "Pearl Engine.h"

class CUITradeItemDisplaySpecialPlug
{
public:
	DWORD m_dVTableAddr;
	MerchantInvSlotList m_Slots;

public:
	CUITradeItemDisplaySpecialPlug();
	~CUITradeItemDisplaySpecialPlug();
	void ParseUIElements();
	DWORD GetSlotVTable(int id);

private:

};