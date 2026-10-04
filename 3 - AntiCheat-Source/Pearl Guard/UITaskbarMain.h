#pragma once
class CUITaskbarMainPlug;
#include "stdafx.h"
#include "Pearl Engine.h"

class CUITaskbarMainPlug
{
public:
	DWORD m_dVTableAddr;
	DWORD base_menu;
	DWORD base_exp;
	DWORD base_TaskBar;
	DWORD base_oldbuttons;
	DWORD m_btnMenu;
	DWORD m_btnPPCard;
	DWORD m_btnRank;
	DWORD m_mobdrop;
	DWORD m_dailyquest;
	DWORD btn_wof;
	DWORD btn_daily_event;
	DWORD m_texthp;
	DWORD m_btnoption;
	DWORD str_exp;
	DWORD m_btn00Stand;
	DWORD m_btn01Sit;
	DWORD m_btn02Seek;
	DWORD m_btn03Trade;
	DWORD m_btn04SkillK;
	DWORD m_btn05Character;
	DWORD m_btn06inventory;
	DWORD m_btn07town;
	//1098 UI
	DWORD m_btninventory;
	DWORD m_btncharacter;
	DWORD m_btnskill;
	DWORD m_btntrade;
	DWORD m_btncommand;
	DWORD m_btninvite;
	DWORD m_btnattack;
	DWORD m_btnwalk;
	DWORD m_btnrun;
	DWORD m_btnsit;
	DWORD m_btnstand;
	DWORD m_btncamera;
	DWORD m_btndisband;
	DWORD m_btnexit;
	DWORD m_btnmap;
	DWORD m_btnQuestion;
	DWORD m_btnPartySettings;
public:
	CUITaskbarMainPlug();
	~CUITaskbarMainPlug();
	void ParseUIElements();
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void InitReceiveMessage();
	void UpdatePosition();
	void OpenPPCard();
	//1299 UI
	void OpenMapButton();
	void OpenInventory();
	void OpenCharacterU();
	void OpenSkill();
	void OpenStand();
	void OpenSit();
	void OpenTrade();
	void OpenSeed();
	void OpenInvite();
	void OpenDisband();
	void OpenExit();
private:

};