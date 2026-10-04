#include "HookSkillPage.h"

HookSkillPage::HookSkillPage()
{
	vector<int>offsets;
	offsets.push_back(0x50);
	offsets.push_back(0);
	offsets.push_back(0x1EC);
	offsets.push_back(0);
	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);
	m_orgbtnResetSkill = 0;
	m_btnResetSkill = 0;
	ParseUIElements();
	InitReceiveMessage();
}

HookSkillPage::~HookSkillPage()
{
}

void HookSkillPage::ParseUIElements()
{
	m_orgbtnResetSkill = Engine->GetChildByID(m_dVTableAddr, "btn_reset");
	m_btnResetSkill = Engine->GetChildByID(m_dVTableAddr, "btn_reset_skill");
	m_btn4 = Engine->GetChildByID(m_dVTableAddr, "btn_4");
	m_btn5 = Engine->GetChildByID(m_dVTableAddr, "btn_5");
	m_btn6 = Engine->GetChildByID(m_dVTableAddr, "btn_6");
	m_btn7 = Engine->GetChildByID(m_dVTableAddr, "btn_7");
}


DWORD el_skill_base;
DWORD Func_SendMessageSkillPage;
void __stdcall UiSkillTreeReceiveMessage_Hookx(DWORD* pSender, uint32_t dwMsg)
{
	__asm
	{
		MOV ECX, el_skill_base
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_SendMessageSkillPage
		CALL EAX
	}
}
bool HookSkillPage::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	el_skill_base = m_dVTableAddr;
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
	{
		if (pSender == (DWORD*)m_btnResetSkill) {
			std::string reqcoins = "Would you like to reset your skill points for :    " + std::to_string(Engine->Player.moneyreq) + " coins.";
			Engine->m_UiMgr->ShowMessageBox("Skill Reset", reqcoins, MsgBoxTypes::YesNo, ParentTypes::PARENT_SKILL_TREE);
		}

		if (dwMsg != 2)
		{
			if (pSender == (DWORD*)m_btn4)
				UiSkillTreeReceiveMessage_Hookx((DWORD*)m_btn4, UIMSG_BUTTON_CLICK);
			else if (pSender == (DWORD*)m_btn5)
				UiSkillTreeReceiveMessage_Hookx((DWORD*)m_btn5, UIMSG_BUTTON_CLICK);
			else if (pSender == (DWORD*)m_btn6)
				UiSkillTreeReceiveMessage_Hookx((DWORD*)m_btn6, UIMSG_BUTTON_CLICK);
			else if (pSender == (DWORD*)m_btn7)
				UiSkillTreeReceiveMessage_Hookx((DWORD*)m_btn7, UIMSG_BUTTON_CLICK);
		}

		return false;
	}

	return true;
}
void __stdcall UiSkillTreeReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiSkillPage->ReceiveMessage(pSender, dwMsg);

	__asm
	{
		MOV ECX, el_skill_base
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_SendMessageSkillPage
		CALL EAX
	}
}
long g1renderTick = 0;


void HookSkillPage::Tick()
{

}

void HookSkillPage::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_SendMessageSkillPage = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)UiSkillTreeReceiveMessage_Hook;
}