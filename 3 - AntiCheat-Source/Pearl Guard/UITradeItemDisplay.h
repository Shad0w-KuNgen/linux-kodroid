#pragma once
class CUITradeItemDiplayPlug;
#include "stdafx.h"
#include "Pearl Engine.h"
#include "Models/MerchantInvSlot.h"

class CUITradeItemDiplayPlug
{
public:
	DWORD m_dVTableAddr;
	MerchantInvSlotList m_Slots;

public:
	CUITradeItemDiplayPlug();
	~CUITradeItemDiplayPlug();
	void __stdcall ParseUIElements();
	DWORD GetSlotVTable(int id);

private:

};