#pragma once
#include "stdafx.h"
class CSealToolTip;
#include "Pearl Engine.h"

struct _perkimsi
{
	DWORD base;
	DWORD view_bonus_perk;
	DWORD view_bonus_descp;
};

struct _perkimci
{
	_perkimsi perkInfo[13];
	DWORD base_perks_view;
	DWORD btn_back_view;
	DWORD str_page;
	DWORD btn_left;
	DWORD btn_right;
	DWORD base_bonus;
};

class CSealToolTip
{
public:

	DWORD area[42];
	uint16 pageCount,cPage;
	CSealToolTip();
	~CSealToolTip();

	uint16 perkType[PERK_COUNT];

	DWORD btn_view_perks;
	_perkimci perkInfo;

	bool BasladimiBaslamadimi;
	void OrderPerk(uint16 page = 1);

	DWORD m_dVTableAddr;
	DWORD base_skill_view;

		DWORD m_70_quest_off;
		DWORD m_70_quest_reset_off;
		DWORD warrior_70_off;
		DWORD warrior_75_off;
		DWORD warrior_80_off;
		DWORD warrior_berserk_off;
		DWORD Rogue_70_off;
		DWORD Rogue_72_off;
		DWORD Rogue_75_off;
		DWORD Rogue_80_off;
		DWORD Mage_70_off;
		DWORD Mage_72_off;
		DWORD Mage_75_off;
		DWORD Mage_80_off;
		DWORD Priest_70_off;
		DWORD Priest_72_off;
		DWORD Priest_74_off;
		DWORD Priest_75_off;
		DWORD Priest_76_off;
		DWORD Priest_78_off;
		DWORD Priest_80_off;



	void ParseUIElements();
	void InitReceiveMessage();
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void DropResultStatus(bool status);
	void QuestCheck(Packet& pkt);
	DWORD m_btnsituation;
	DWORD m_btndropresult;
	DWORD m_btnbug;
	DWORD btn_lottery;
};