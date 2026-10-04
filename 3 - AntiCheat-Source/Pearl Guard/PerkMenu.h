#pragma once
class CPerkMenu;
#include "stdafx.h"
#include "Pearl Engine.h"
#include "Timer.h"
#include "N3BASE/N3UIBase.h"
#include "N3BASE/N3UIString.h"
#include "N3BASE/N3UIButton.h"

struct perkInfo
{
	CN3UIBase* base;
	CN3UIString* str_perk, * str_descp;
	perkInfo()
	{
		base = NULL;
		str_perk = NULL;
		str_descp = NULL;
	}
};

class CPerkMenu : public CN3UIBase
{
public:
	uint16 perkType[PERK_COUNT];
	uint16 cPage, pageCount;
	CN3UIString* str_info, * str_Nick, * str_page;
	CN3UIButton* btn_left, * btn_close, * btn_right;

	perkInfo PerksBonus[UI_PERKCOUNT];
public:
	CPerkMenu();
	~CPerkMenu();
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	bool Load(HANDLE hFile);
	void LoadInfo(uint16 perk[PERK_COUNT], uint16 targetID);
	void reOrderPerk(uint16 page = 1);
	void Open();
	void Close();
	bool OnKeyPress(int iKey);
private:
};