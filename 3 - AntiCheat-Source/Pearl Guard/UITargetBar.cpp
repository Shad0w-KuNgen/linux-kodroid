#include "UITargetBar.h"
#include <math.h>

CUITargetBarPlug::CUITargetBarPlug()
{
	vector<int>offsets;
	offsets.push_back(0x220);
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);

	m_dTextTargetHp = NULL;
	m_btnDrop = NULL;
	m_user_info = NULL; 
	ParseUIElements();
	InitReceiveMessage();
}

CUITargetBarPlug::~CUITargetBarPlug()
{
}

void CUITargetBarPlug::ParseUIElements()
{
	str_name = Engine->GetChildByID(m_dVTableAddr, "name");
#if (HOOK_SOURCE_VERSION != 1098)
	m_btnDrop = Engine->GetChildByID(m_dVTableAddr, "Btn_Drops");
	m_dTextTargetHp = Engine->GetChildByID(m_dVTableAddr, "Text_Health");
	Engine->SetVisible(m_btnDrop, false);
	Engine->SetState(m_btnDrop, UI_STATE_BUTTON_DISABLE);
#endif
}

void CUITargetBarPlug::SetInfo(DWORD color, uint8 lvl, std::string name)
{
	if(str_name) Engine->SetString(str_name, string_format("%s Lv. %d", name.c_str(), lvl));  // moblarýn isimlerinin yanýnda level görünüyor.

}

DWORD uiTargetBarVTable;
DWORD Func_TargetBar = 0;

bool CUITargetBarPlug::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	uiTargetBarVTable = m_dVTableAddr;
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	Engine->m_zMob = *(DWORD*)((*(DWORD*)KO_PTR_CHR) + KO_OFF_MOB);
	uint16 sIID = 0;
	DWORD target = Engine->KOGetTarget();
	if (target > 0)
		sIID = *(uint16*)(target + KO_SSID);

	if (pSender == (DWORD*)m_btnDrop)
	{
		Packet pkt(XSafe);
		pkt << uint8(XSafeOpCodes::DROP_REQUEST) << uint8(3) << sIID;
		Engine->Send(&pkt);
	}
	else if (pSender == (DWORD*)m_user_info) 
	{
		Packet result(WIZ_USER_INFO, uint8(0x05));
		result << uint16(Engine->m_zMob);
		Engine->Send(&result);
	}
	return true;
}

void __stdcall UiTargetBarReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiTargetBar->ReceiveMessage(pSender, dwMsg);
}

void CUITargetBarPlug::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_TargetBar = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)UiTargetBarReceiveMessage_Hook;
}
__inline DWORD PRDWORD(DWORD ulBase)
{
	if (!IsBadReadPtr((VOID*)ulBase, sizeof(DWORD)))
	{
		return(*(DWORD*)(ulBase));
	}
	return 0;
}

void CUITargetBarPlug::TargetNameSet(uint16 GetID)
{
	DWORD base = PRDWORD(PRDWORD(KO_DLG) + KO_OFF_PTBASE);
	DWORD buffer = PRDWORD(base + KO_OFF_PT);
	DWORD partyCount = PRDWORD(base + KO_OFF_PTCOUNT);
	bool isparty = false;
	for (int i = 0; i < partyCount; i++)
	{
		if (partyCount > 0)
		{
			buffer = PRDWORD(buffer);
			if (buffer)
			{
				USHORT Id = *(USHORT*)(buffer + 0x8);

				if (GetID == Id)
				{
					Engine->SetStringColor(str_name, 0xFFFFFF00);
					isparty = true;
					break;
				}
			}
		}
	}

	if (!isparty)
	{
		DWORD target = Engine->KOGetTarget();
		if (target > 0)
		{
			if (*(uint16*)(target + KO_OFF_ID) < NPC_BAND)
			{
				uint8 myLvl = *(uint8*)(*(DWORD*)KO_PTR_CHR + KO_OFF_LEVEL);
				if (myLvl < 30)
					Engine->SetStringColor(str_name, 0xFFFFFFFF);
			}
		}
	}
}

void CUITargetBarPlug::SetTargetHp(Packet & pkt)
{
	uint16 tid, damage, ssid;
	uint8 echo, prototype;
	int32 maxhp, hp;
	pkt >> tid >> echo >> maxhp >> hp >> damage >> ssid;


	pkt >> prototype;
	/*
		1.monster
		2.npc
		3.bot
		4.real user
	*/

	if (ssid == tid)
		return;

	if (tid < NPC_BAND || prototype == 3)
	{
		Engine->SetVisible(m_user_info, true); 
		Engine->SetVisible(m_btnDrop, false);
		Engine->SetState(m_user_info, UI_STATE_BUTTON_NORMAL);
		Engine->SetState(m_btnDrop, UI_STATE_BUTTON_DISABLE);
		TargetNameSet(tid);
	}
	else if (tid > NPC_BAND)
	{
		if (prototype == 2)
			Engine->SetVisible(m_btnDrop, false);
		else
			Engine->SetVisible(m_btnDrop, true);

		Engine->KOGetSSID = ssid;
		Engine->SetVisible(m_user_info, false);
		
		Engine->SetState(m_user_info, UI_STATE_BUTTON_DISABLE);
		Engine->SetState(m_btnDrop, UI_STATE_BUTTON_NORMAL);
	}
	//if (Engine->m_PlayerBase && !Engine->m_PlayerBase->isInPKZone()) {
		int percent = (int)ceil((hp * 100) / maxhp);
		std::string max = Engine->StringHelper->NumberFormat(maxhp);
		std::string cur = Engine->StringHelper->NumberFormat(hp);
		std::string str = xorstr("%s/%s (%d%%)");
		Engine->SetString(m_dTextTargetHp, string_format(str, cur.c_str(), max.c_str(), percent));
	//}
}