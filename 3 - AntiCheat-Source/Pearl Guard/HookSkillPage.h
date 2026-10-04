#pragma once
class HookSkillPage;
#include "stdafx.h"
#include "Pearl Engine.h"

class HookSkillPage
{
public:
	uintptr_t** vTable;
	DWORD m_dVTableAddr;
	// UI
	DWORD m_orgbtnResetSkill;
	DWORD m_btnResetSkill;
	DWORD m_btn4;
	DWORD m_btn5;
	DWORD m_btn6;
	DWORD m_btn7;
public:
	HookSkillPage();
	~HookSkillPage();
	void ParseUIElements();
	void Tick();
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void InitReceiveMessage();

private:

};