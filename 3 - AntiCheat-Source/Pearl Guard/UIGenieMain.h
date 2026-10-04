#pragma once
class CUIGenieMain;
#include "stdafx.h"
#include "Pearl Engine.h"

class CUIGenieMain
{
public:
	DWORD m_dVTableAddr;
	DWORD genie;
	DWORD attack, btn_auto_r_attack_on, btn_auto_r_attack_off, etc, btn_party_leader_off, btn_party_leader_on, btn_basic_attack_off;
	DWORD scr_act_range, scr_att_range;
public:
	CUIGenieMain();
	~CUIGenieMain();
	void ParseUIElements();
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void InitReceiveMessage();
	void Tick();
	void SendBasicAttack(bool status);
private:

};