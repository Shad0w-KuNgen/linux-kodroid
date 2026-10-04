#include "stdafx.h"
#include "UIInventory.h"

bool viewEmblem = true, viewPathos = true;
bool minervaHook = false;
const DWORD KO_PATHOS_CHECK = 0x438C24;
const DWORD KO_EMBLEM_LOOP_ITERATE = 0x00585E60;
const DWORD KO_UI_INVENTORY_MINERVA_RESTORE = 0x00607B64;
DWORD KO_UI_INVENTORY_MINERVA_RESTORE_ORG = 0;

void MinervaRestore()
{
	if (viewEmblem && Engine->uiInventoryPlug) Engine->SetState(Engine->uiInventoryPlug->btn_EmblemView, UI_STATE_BUTTON_DOWN);
	if (viewPathos && Engine->uiInventoryPlug) Engine->SetState(Engine->uiInventoryPlug->btn_PathosView, UI_STATE_BUTTON_DOWN);
}

void __declspec(naked) hkOpenMinerva()
{
	__asm {
		pushad
		pushfd
		call MinervaRestore
		popfd
		popad
		jmp KO_UI_INVENTORY_MINERVA_RESTORE_ORG
	}
}

CUIInventoryPlug::CUIInventoryPlug()
{
	vector<int>offsets;
	offsets.push_back(0x204); // re_Inventory Satýr : 11
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);

	ParseUIElements();
	InitReceiveMessage();
}

CUIInventoryPlug::~CUIInventoryPlug()
{
}

void CUIInventoryPlug::ParseUIElements()
{
	btn_trash = Engine->GetChildByID(m_dVTableAddr, "btn_trash");
	base_cos = Engine->GetChildByID(m_dVTableAddr, "base_cos");
	btn_EmblemView = Engine->GetChildByID(base_cos, "btn_EmblemView");
	btn_PathosView = Engine->GetChildByID(base_cos, "btn_PathosView");

	if (!minervaHook)
	{
		minervaHook = true;
		KO_UI_INVENTORY_MINERVA_RESTORE_ORG = (DWORD)DetourFunction((PBYTE)KO_UI_INVENTORY_MINERVA_RESTORE, (PBYTE)hkOpenMinerva);
	}

	viewEmblem = GetPrivateProfileIntA(xorstr("Effect"), xorstr("EmblemView"), 1, string(Engine->m_BasePath + xorstr("Option.ini")).c_str()) == 1 ? true : false;
	*(uint8*)KO_EMBLEM_LOOP_ITERATE = viewEmblem ? 2 : 1;

	viewPathos = GetPrivateProfileIntA(xorstr("Effect"), xorstr("PathosView"), 1, string(Engine->m_BasePath + xorstr("Option.ini")).c_str()) == 1 ? true : false;
	*(uint8*)KO_PATHOS_CHECK = viewPathos ? (uint8)0x74 : (uint8)0xEB;

}

DWORD uiInventoryVTable;
bool CUIInventoryPlug::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	uiInventoryVTable = m_dVTableAddr;
	//if (!pSender || pSender == 0 || dwMsg != 67108864)
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	if (pSender == (DWORD*)btn_trash)
	{
		Packet result(XSafe);
		result << uint8_t(XSafeOpCodes::SendRepurchaseMsg);
		Engine->Send(&result);
	}
	else if (pSender == (DWORD*)btn_EmblemView)
	{
		viewEmblem = !viewEmblem;
		*(uint8*)KO_EMBLEM_LOOP_ITERATE = viewEmblem ? 2 : 1;

		WritePrivateProfileStringA(xorstr("Effect"), xorstr("EmblemView"), viewEmblem ? xorstr("1") : xorstr("0"), string(Engine->m_BasePath + xorstr("Option.ini")).c_str());
	}
	else if (pSender == (DWORD*)btn_PathosView)
	{
		viewPathos = !viewPathos;
		*(uint8*)KO_PATHOS_CHECK = viewPathos ? (uint8)0x74 : (uint8)0xEB;

		WritePrivateProfileStringA(xorstr("Effect"), xorstr("PathosView"), viewPathos ? xorstr("1") : xorstr("0"), string(Engine->m_BasePath + xorstr("Option.ini")).c_str());
	}
	return true;
}

DWORD Func_Inventory;
void __stdcall UIInventoryReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiInventoryPlug->ReceiveMessage(pSender, dwMsg);
	__asm
	{
		MOV ECX, uiInventoryVTable
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_Inventory
		CALL EAX
	}
}

void CUIInventoryPlug::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_Inventory = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)UIInventoryReceiveMessage_Hook;
}