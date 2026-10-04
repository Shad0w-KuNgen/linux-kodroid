#include "stdafx.h"
#include "UIMiniMenu.h"

CUIMiniMenuPlug::CUIMiniMenuPlug()
{
	vector<int>offsets;
	offsets.push_back(0x398);
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);
	m_btngiftpus = 0;
	m_btnViewInfo = 0;

	ParseUIElements();
	InitReceiveMessage();
}

CUIMiniMenuPlug::~CUIMiniMenuPlug()
{
}

void CUIMiniMenuPlug::ParseUIElements()
{
	base_default = Engine->GetChildByID(m_dVTableAddr, "base_default");
	m_btngiftpus = Engine->GetChildByID(base_default, "btn_giftpus");
	m_btnViewInfo = Engine->GetChildByID(base_default, "btn_viewinfo");
}

DWORD uiMiniMenuVTable;
DWORD miniMenuSetVisibleFalse;
void __stdcall SetVisibleViaOGFuncAsm()
{
	_asm
	{
		MOV ECX, uiMiniMenuVTable
		MOV EAX, miniMenuSetVisibleFalse
		PUSH EAX
		CALL KO_SET_VISIBLE_MINIMENU_FUNC
		//RET
	}
}

bool CUIMiniMenuPlug::ReceiveMessage(DWORD * pSender, uint32_t dwMsg)
{
	uiMiniMenuVTable = m_dVTableAddr;
	miniMenuSetVisibleFalse = 0;
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	if (pSender == (DWORD*)m_btnViewInfo)
	{
		int16 target = -1;
		DWORD ko_adr = 0;

		ReadProcessMemory(GetCurrentProcess(), (LPVOID)KO_PTR_CHR, &ko_adr, sizeof(&ko_adr), 0);
		ReadProcessMemory(GetCurrentProcess(), (LPVOID)(ko_adr + KO_OFF_MOB), &target, sizeof(&target), 0);

		Packet result(WIZ_USER_INFO, uint8(0x05));
		result << target;
		Engine->Send(&result);
		SetVisibleViaOGFuncAsm();
	}
	else if (pSender == (DWORD*)m_btngiftpus) //14.11.2020 User Sag Tik Pus
	{
		if (Engine->uiPowerUpStore  == NULL)
		{
			Engine->uiPowerUpStore ->OpenPowerUpStore();
		}
		Engine->uiPowerUpStore ->UpdateItemList(Engine->m_UiMgr->item_list, Engine->m_UiMgr->cat_list);
		Engine->uiPowerUpStore ->UpdateRefundItemList(Engine->m_UiMgr->pusrefund_itemlist);
		Engine->uiPowerUpStore ->Open();
		SetVisibleViaOGFuncAsm();
	}

	return true;
}

void __stdcall UiMiniMenuReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiMiniMenuPlug->ReceiveMessage(pSender, dwMsg);

	__asm
	{
		MOV ECX, uiMiniMenuVTable
		PUSH dwMsg
		PUSH pSender
		MOV EAX, KO_UI_MINIMENU_RECEIVE_MESSAGE_FUNC
		CALL EAX
	}
}

void CUIMiniMenuPlug::InitReceiveMessage()
{
	*(DWORD*)KO_UI_MINIMENU_RECEIVE_MESSAGE_PTR = (DWORD)UiMiniMenuReceiveMessage_Hook;
}