#include "stdafx.h"
#include "UIGenieMain.h"
extern bool isLeaderAttack;
CUIGenieMain::CUIGenieMain()
{
	vector<int>offsets;
	offsets.push_back(0x50);
	offsets.push_back(0);
	offsets.push_back(0x518);
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);

	btn_auto_r_attack_on = NULL;
	btn_auto_r_attack_off = NULL;
	btn_party_leader_on = NULL;
	btn_party_leader_off = NULL;
	btn_basic_attack_off = NULL;

	ParseUIElements();
	InitReceiveMessage();
}

CUIGenieMain::~CUIGenieMain()
{
}

void CUIGenieMain::ParseUIElements()
{
	genie = Engine->GetChildByID(m_dVTableAddr, "genie");
	etc = Engine->GetChildByID(genie, "etc");
	attack = Engine->GetChildByID(genie, "attack");
	btn_basic_attack_off = Engine->GetChildByID(attack, "btn_basic_attack_off");
	btn_auto_r_attack_on = Engine->GetChildByID(attack, "btn_auto_r_attack_on");
	btn_auto_r_attack_off = Engine->GetChildByID(attack, "btn_auto_r_attack_off");
	scr_att_range = Engine->GetChildByID(attack, "scr_att_range");
	scr_act_range = Engine->GetChildByID(attack, "scr_act_range");
	btn_party_leader_on = Engine->GetChildByID(attack, "btn_party_leader_on");
	btn_party_leader_off = Engine->GetChildByID(attack, "btn_party_leader_off");
}

extern bool __genieAttackBlock;
extern bool __genieAttackMove;

void CUIGenieMain::Tick()
{
	static bool bSet = false;
	if (!bSet && Engine->IsVisible(attack))
	{
		if (__genieAttackBlock) {
			Engine->SetState(btn_auto_r_attack_on, UI_STATE_BUTTON_NORMAL);
			Engine->SetState(btn_auto_r_attack_off, UI_STATE_BUTTON_DOWN);
		} else {
			Engine->SetState(btn_auto_r_attack_on, UI_STATE_BUTTON_DOWN);
			Engine->SetState(btn_auto_r_attack_off, UI_STATE_BUTTON_NORMAL);
		}
		
		if (__genieAttackMove) {
			Engine->SetState(btn_basic_attack_off, UI_STATE_BUTTON_NORMAL);
		}
		else {
			Engine->SetState(btn_basic_attack_off, UI_STATE_BUTTON_DOWN);
			
		}
		if (isLeaderAttack) {
			Engine->SetState(btn_party_leader_on, UI_STATE_BUTTON_DOWN);
			Engine->SetState(btn_party_leader_off, UI_STATE_BUTTON_NORMAL);
		}
		else {
			Engine->SetState(btn_party_leader_on, UI_STATE_BUTTON_NORMAL);
			Engine->SetState(btn_party_leader_off, UI_STATE_BUTTON_DOWN);
		}
		bSet = true;
	}
	else if (!Engine->IsVisible(attack)) bSet = false;
}

void CUIGenieMain::SendBasicAttack(bool status)
{
	Packet newpkt(WIZ_GENIE, uint8(25));
	newpkt << uint8(status);
	Engine->Send(&newpkt);
}

DWORD uiGenieMainVTable;
bool CUIGenieMain::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	uiGenieMainVTable = m_dVTableAddr;
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	std::string msg = "";
	if (pSender == (DWORD*)btn_party_leader_off)
	{
		isLeaderAttack = false;
		Engine->SetState(btn_party_leader_on, UI_STATE_BUTTON_NORMAL);
		Engine->SetState(btn_party_leader_off, UI_STATE_BUTTON_DOWN);
	}
	if (pSender == (DWORD*)btn_basic_attack_off)
	{
		__genieAttackMove = !__genieAttackMove;
		Engine->SetState(btn_basic_attack_off, (__genieAttackMove ? UI_STATE_BUTTON_NORMAL : UI_STATE_BUTTON_DOWN));
		SendBasicAttack(__genieAttackMove);

	}
	if (pSender == (DWORD*)btn_party_leader_on)
	{
		isLeaderAttack = true;
		Engine->SetState(btn_party_leader_on, UI_STATE_BUTTON_DOWN);
		Engine->SetState(btn_party_leader_off, UI_STATE_BUTTON_NORMAL);
	}
	if (pSender == (DWORD*)btn_auto_r_attack_on)
	{
		__genieAttackBlock = false;
		Engine->SetState(btn_auto_r_attack_on, UI_STATE_BUTTON_DOWN);
		Engine->SetState(btn_auto_r_attack_off, UI_STATE_BUTTON_NORMAL);
	}
	else if (pSender == (DWORD*)btn_auto_r_attack_off)
	{
		__genieAttackBlock = true;
		Engine->SetState(btn_auto_r_attack_on, UI_STATE_BUTTON_NORMAL);
		Engine->SetState(btn_auto_r_attack_off, UI_STATE_BUTTON_DOWN);
	}

	return true;
}

DWORD Func_GenieMain = 0;
void __stdcall UiGenieMainReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiGenieMain->ReceiveMessage(pSender, dwMsg);
	__asm
	{
		MOV ECX, uiGenieMainVTable
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_GenieMain
		CALL EAX
	}
}

void CUIGenieMain::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_GenieMain = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)UiGenieMainReceiveMessage_Hook;
}