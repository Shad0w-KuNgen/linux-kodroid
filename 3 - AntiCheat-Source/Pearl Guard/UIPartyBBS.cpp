#include "stdafx.h"
#include "UIPartyBBS.h"

CUIPartyBBSPlug::CUIPartyBBSPlug()
{
	vector<int>offsets;
	offsets.push_back(0x50);
	offsets.push_back(0);
	offsets.push_back(0x1E8);
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);

	if (m_dVTableAddr == 0)
		return;

	ParseUIElements();
	InitReceiveMessage();
}

CUIPartyBBSPlug::~CUIPartyBBSPlug()
{
}

void CUIPartyBBSPlug::ParseUIElements()
{
	btn_disband = Engine->GetChildByID(m_dVTableAddr, "btn_party");
	for (int i = 0; i < 8; i++)
	{
		slot[i].baseAddr = Engine->GetChildByID(m_dVTableAddr, string_format(xorstr("Base_Party%d"), i + 1));
		slot[i].strName = Engine->GetChildByID(slot[i].baseAddr, string_format(xorstr("str_name%d"), i + 1));
	}
}

DWORD uiPartyBBSVTable;
bool CUIPartyBBSPlug::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	uiPartyBBSVTable = m_dVTableAddr;
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	return true;
}

DWORD Func_Party = 0;

void __stdcall UIPatyBBSReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiPartyBBS->ReceiveMessage(pSender, dwMsg);
	__asm
	{
		MOV ECX, uiPartyBBSVTable
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_Party
		CALL EAX
	}
}

void CUIPartyBBSPlug::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_Party = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)UIPatyBBSReceiveMessage_Hook;
}

void CUIPartyBBSPlug::OpenPartyDisband()
{
	UIPatyBBSReceiveMessage_Hook((DWORD*)btn_disband, UIMSG_BUTTON_CLICK);
}

void CUIPartyBBSPlug::UpdatePartyUserHPBAR(Packet& pkt)
{
	if (Engine->uiPartyBBS == NULL)
		return;

	int max_hp, hp;
	std::string username;
	pkt.DByte();
	pkt >> username >> max_hp >> hp;
	std::string hptext = xorstr("x");
	for (size_t i = 0; i < 8; i++)
	{
		std::string name = Engine->GetString(slot[i].strName);
		if (username == name)
		{
			hptext = string_format(xorstr("%d / %d"), hp, max_hp);
			Engine->SetString(Engine->uiPartyBBS->slot[i].strHP, hptext.c_str());
			return;
		}
	}
}
__inline DWORD rRDWORD(DWORD ulBase)
{
	if (!IsBadReadPtr((VOID*)ulBase, sizeof(DWORD)))
	{
		return(*(DWORD*)(ulBase));
	}
	return 0;
}

bool CUIPartyBBSPlug::PartyFind(uint16 GetID)
{
	DWORD base = rRDWORD(rRDWORD(KO_DLG) + KO_OFF_PTBASE);
	DWORD buffer = rRDWORD(base + KO_OFF_PT);
	DWORD partyCount = rRDWORD(base + KO_OFF_PTCOUNT);

	for (int i = 0; i < partyCount; i++)
	{
		if (partyCount > 0)
		{
			buffer = rRDWORD(buffer);
			if (buffer)
			{
				USHORT Id = *(USHORT*)(buffer + 0x8);
				if (GetID == Id)
					return true;
			}
		}
	}

	/*if (!Engine->IsVisible(m_dVTableAddr))
		return false;

		DWORD base, buffer, partyCount;
		base = rRDWORD(rRDWORD(KO_DLG) + OFF_PTBASE_1);
		buffer = rRDWORD(base + OFF_PT_1);
		partyCount = rRDWORD(base + OFF_PTCOUNT_1);

	for (DWORD i = 0; i < partyCount; i++)
	{
		std::string name = Engine->GetPartyString(slot[i].strName).c_str();
		if (username == name)
			return true;
	}*/

	return false;
}