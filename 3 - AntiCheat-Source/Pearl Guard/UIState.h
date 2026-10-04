#pragma once
class CUIStatePlug;
#include "stdafx.h"
#include "Pearl Engine.h"

struct basePerksBonus
{
	DWORD base;
	DWORD btn_plus, str_perk, str_maxPerk, str_descp;
	int32 index;
	basePerksBonus()
	{
		base = NULL;
		btn_plus = NULL;
		str_perk = NULL;
		str_maxPerk = NULL;
		str_descp = NULL;
		index = 0;
	}
};

struct basePerks
{
	DWORD base;
	DWORD str_page, btn_right, btn_left;
	DWORD base_bonus;
	basePerksBonus _basePerksBonus[UI_PERKCOUNT];
	basePerks()
	{
		base = NULL;
		str_page = NULL;
		btn_right = NULL;
		btn_left = NULL;
		base_bonus = NULL;
		for (int i = 0; i < UI_PERKCOUNT; i++)
			_basePerksBonus[i] = basePerksBonus();
	}
};


class CUIStatePlug
{
public:
	DWORD m_dVTableAddr;
	//Buttons
	DWORD m_btnResetReb;
	//Texts
	DWORD m_txtCash;
	DWORD m_txtDagger;
	DWORD m_txtSword;
	DWORD m_txtClub;
	DWORD m_strFreeStatPoint;
	DWORD m_txtAxe;
	DWORD m_txtSpear;
	DWORD m_txtArrow;
	DWORD m_txtJamadar;
	DWORD m_txtTLBalance;
	int32 m_iCash;
	int32 m_iBalance;
	DWORD getStat;
	
	uint16 m_iDagger;
	uint16 m_iSword;
	uint16 m_iClub;
	uint16 m_iAxe;
	uint16 m_iSpear;
	uint16 m_iArrow;
	uint16 m_iJamadar;
	//-----------------------
	DWORD m_btnStr10;
	DWORD m_btnHp10;
	DWORD m_btnDex10;
	DWORD m_btnMp10;
	DWORD m_btnInt10;
	//-----------------------------

	DWORD m_btnReset;
	DWORD m_orgbtnReset;

	basePerks _basePerks;
	uint16 cPage, pageCount;
	DWORD str_remPerkCount;
	DWORD btn_perks;
	DWORD btn_reset_perks;
	DWORD back_character_page;
	ULONGLONG perkUseTick;

public:
	CUIStatePlug();
	~CUIStatePlug();
	void ParseUIElements();
	void UpdateKC(uint32 cash,uint32 balance);
	void SetAntiDefInfo(Packet& pkt);
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void InitReceiveMessage();
	void UpdateUI();
	bool UpdatePointButton(bool visible);
	void Tick();

	void resetPerks();
	void perksOpen();
	void perkPlus(Packet& pkt);
	void reOrderPerk(uint16 page = 1);
	void resetPerkPoint(Packet& pkt);
	void setOptions();
	void resetSelect();
	void perkUseItem(Packet& pkt);
	time_t amktick;
};