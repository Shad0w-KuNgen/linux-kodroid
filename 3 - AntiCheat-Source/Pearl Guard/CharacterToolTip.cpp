#include "CharacterToolTip.h"

CSealToolTip::CSealToolTip()
{
	m_dVTableAddr = NULL;
	base_skill_view = NULL;
	m_btnsituation = NULL;
	m_btndropresult = NULL;
	m_btnbug = NULL;
	btn_lottery = NULL;
	pageCount = cPage = 1;
	BasladimiBaslamadimi = false;
	vector<int>offsets;
	offsets.push_back(0x42C);
	offsets.push_back(0);
	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);

	memset(perkType, 0, sizeof(perkType));

	ParseUIElements();
	InitReceiveMessage();
}

CSealToolTip::~CSealToolTip()
{
}

void CSealToolTip::QuestCheck(Packet& pkt)
{
	uint8 nSize = 19;
	uint8 Status = 0;
	uint16 nQuestID = 0;

	for (int i = 0; i < nSize;i++)
	{
		pkt >> nQuestID >> Status;

		if (nQuestID == 334)
		{
			if(Status==2)
				Engine->SetTexImage(warrior_70_off, string_format("ui\\mark_flag02.dxt"));
			else
				Engine->SetTexImage(warrior_70_off, string_format("ui\\mark_flag01.dxt"));
		}else if (nQuestID == 359)
		{
			if (Status == 2)
				Engine->SetTexImage(warrior_75_off, string_format("ui\\mark_flag02.dxt"));
			else
				Engine->SetTexImage(warrior_75_off, string_format("ui\\mark_flag01.dxt"));
			
		}
		else if (nQuestID == 365)
		{
			if (Status == 2)
				Engine->SetTexImage(warrior_80_off, string_format("ui\\mark_flag02.dxt"));
			else
				Engine->SetTexImage(warrior_80_off, string_format("ui\\mark_flag01.dxt"));

				}
		else if (nQuestID == 335)
		{
			if (Status == 2)
				Engine->SetTexImage(Rogue_70_off, string_format("ui\\mark_flag02.dxt"));
			else
				Engine->SetTexImage(Rogue_70_off, string_format("ui\\mark_flag01.dxt"));
		}
		else if (nQuestID == 347)
		{
			if (Status == 2)
				Engine->SetTexImage(Rogue_72_off, string_format("ui\\mark_flag02.dxt"));
			else
				Engine->SetTexImage(Rogue_72_off, string_format("ui\\mark_flag01.dxt"));

		}
		else if (nQuestID == 360)
		{
			if (Status == 2)
				Engine->SetTexImage(Rogue_75_off, string_format("ui\\mark_flag02.dxt"));
			else
				Engine->SetTexImage(Rogue_75_off, string_format("ui\\mark_flag01.dxt"));

		}
		else if (nQuestID == 366)
		{
			if (Status == 2)
				Engine->SetTexImage(Rogue_80_off, string_format("ui\\mark_flag02.dxt"));
			else
				Engine->SetTexImage(Rogue_80_off, string_format("ui\\mark_flag01.dxt"));

		}
		else if (nQuestID == 336)
		{
			if (Status == 2)
				Engine->SetTexImage(Mage_70_off, string_format("ui\\mark_flag02.dxt"));
			else
				Engine->SetTexImage(Mage_70_off, string_format("ui\\mark_flag01.dxt"));

		}
		else if (nQuestID == 348)
		{
			if (Status == 2)
				Engine->SetTexImage(Mage_72_off, string_format("ui\\mark_flag02.dxt"));
			else
				Engine->SetTexImage(Mage_72_off, string_format("ui\\mark_flag01.dxt"));

		}
		else if (nQuestID == 361)
		{
			if (Status == 2)
				Engine->SetTexImage(Mage_75_off, string_format("ui\\mark_flag02.dxt"));
			else
				Engine->SetTexImage(Mage_75_off, string_format("ui\\mark_flag01.dxt"));

		}
		else if (nQuestID == 367)
		{
			if (Status == 2)
				Engine->SetTexImage(Mage_80_off, string_format("ui\\mark_flag02.dxt"));
			else
				Engine->SetTexImage(Mage_80_off, string_format("ui\\mark_flag01.dxt"));

		}
		else if (nQuestID == 516)
		{
			//Engine->SetTexImage(warrior_80_off,  string_format("ui\\mark_flag_0%d.dxt",Status));
		}
		else if (nQuestID == 337)
		{
			if (Status == 2)
				Engine->SetTexImage(Priest_70_off, string_format("ui\\mark_flag02.dxt"));
			else
				Engine->SetTexImage(Priest_70_off, string_format("ui\\mark_flag01.dxt"));

		}
		else if (nQuestID == 349)
		{
		if (Status == 2)
			Engine->SetTexImage(Priest_72_off, string_format("ui\\mark_flag02.dxt"));
		else
			Engine->SetTexImage(Priest_72_off, string_format("ui\\mark_flag01.dxt"));

		}
		else if (nQuestID == 357)
		{
		if (Status == 2)
			Engine->SetTexImage(Priest_74_off, string_format("ui\\mark_flag02.dxt"));
		else
			Engine->SetTexImage(Priest_74_off, string_format("ui\\mark_flag01.dxt"));

		}
		else if (nQuestID == 362)
		{
		if (Status == 2)
			Engine->SetTexImage(Priest_75_off, string_format("ui\\mark_flag02.dxt"));
		else
			Engine->SetTexImage(Priest_75_off, string_format("ui\\mark_flag01.dxt"));

		}
		else if (nQuestID == 363)
		{
		if (Status == 2)
			Engine->SetTexImage(Priest_76_off, string_format("ui\\mark_flag02.dxt"));
		else
			Engine->SetTexImage(Priest_76_off, string_format("ui\\mark_flag01.dxt"));

		}
		else if (nQuestID == 364)
		{
		if (Status == 2)
			Engine->SetTexImage(Priest_78_off, string_format("ui\\mark_flag02.dxt"));
		else
			Engine->SetTexImage(Priest_78_off, string_format("ui\\mark_flag01.dxt"));

		}
		else if (nQuestID == 368)
		{
				if (Status == 2)
			Engine->SetTexImage(Priest_80_off, string_format("ui\\mark_flag02.dxt"));
		else
			Engine->SetTexImage(Priest_80_off, string_format("ui\\mark_flag01.dxt"));
			//Engine->SetTexImage(Priest_80_off,  string_format("ui\\mark_flag_0%d.dxt",Status));
		}
	}

	Engine->SetVisible(perkInfo.base_perks_view, false);
	uint16 perk[PERK_COUNT]{};
	for (int i = 0; i < PERK_COUNT; i++) perk[i] = pkt.read<uint16>();
	memcpy(perkType, perk, sizeof(perkType));
	OrderPerk();
}

void CSealToolTip::OrderPerk(uint16 page)
{
	for (int i = 0; i < 13; i++)
	{
		Engine->SetString(perkInfo.perkInfo[i].view_bonus_descp, "");
		Engine->SetString(perkInfo.perkInfo[i].view_bonus_perk, "");
		Engine->SetVisible(perkInfo.perkInfo[i].base, false);
	}

	std::vector< _PERKS> mPerks;
	foreach(itr, Engine->m_PerksArray)
		if (itr->second && itr->second->status)
			mPerks.push_back(*itr->second);

	if (mPerks.empty())
		return;

	std::sort(mPerks.begin(), mPerks.end(),
		[](auto const& a, auto const& b) { return a.pIndex < b.pIndex; });

	pageCount = abs(ceil((double)mPerks.size() / (double)UI_PERKCOUNT));
	if (pageCount < 1) pageCount = 1;
	Engine->SetString(perkInfo.str_page, string_format("%d/%d", cPage, pageCount));

	int p = (page - 1);
	if (p < 0) p = 0;
	size_t i = abs(p * UI_PERKCOUNT);

	uint8 c_slot = 0; uint16 count = 0;
	foreach(itr, mPerks)
	{
		count++;
		if (count <= i)
			continue;
		
		Engine->SetVisible(perkInfo.perkInfo[c_slot].base, true);
		Engine->SetString(perkInfo.perkInfo[c_slot].view_bonus_descp, itr->strDescp);
		Engine->SetString(perkInfo.perkInfo[c_slot].view_bonus_perk, std::to_string(perkType[itr->pIndex]));
		c_slot++;
		if (c_slot >= UI_PERKCOUNT)
			break;
	}
}

void CSealToolTip::ParseUIElements()
{

	btn_view_perks = Engine->GetChildByID(m_dVTableAddr, "btn_view_perks");
	perkInfo.base_perks_view = Engine->GetChildByID(m_dVTableAddr, "base_perks_view");
	perkInfo.base_bonus = Engine->GetChildByID(perkInfo.base_perks_view, "base_bonus");
	perkInfo.btn_back_view = Engine->GetChildByID(perkInfo.base_perks_view, "btn_back_view");
	perkInfo.btn_left = Engine->GetChildByID(perkInfo.base_perks_view, "btn_left");
	perkInfo.btn_right = Engine->GetChildByID(perkInfo.base_perks_view, "btn_right");
	perkInfo.str_page = Engine->GetChildByID(perkInfo.base_perks_view, "str_page");
	for (int i = 0; i < 13; i++)
	{
		perkInfo.perkInfo[i].base = Engine->GetChildByID(perkInfo.base_bonus, string_format(xorstr("base_perk_bonus%d"), i + 1));
		perkInfo.perkInfo[i].view_bonus_descp = Engine->GetChildByID(perkInfo.perkInfo[i].base, "view_bonus_descp");
		perkInfo.perkInfo[i].view_bonus_perk = Engine->GetChildByID(perkInfo.perkInfo[i].base, "view_bonus_perk");
	}
	Engine->SetVisible(perkInfo.base_perks_view, false);
	base_skill_view = Engine->GetChildByID(m_dVTableAddr, "base_skill_view");
	m_70_quest_off = Engine->GetChildByID(base_skill_view, "70_quest_off");
	m_70_quest_reset_off = Engine->GetChildByID(base_skill_view, "m_70_quest_reset_off");
	warrior_70_off = Engine->GetChildByID(base_skill_view, "warrior_70_off");
	warrior_75_off = Engine->GetChildByID(base_skill_view, "warrior_75_off");
	warrior_80_off = Engine->GetChildByID(base_skill_view, "warrior_80_off");
	warrior_berserk_off = Engine->GetChildByID(base_skill_view, "warrior_berserk_off");
	Rogue_70_off = Engine->GetChildByID(base_skill_view, "Rogue_70_off");
	Rogue_72_off = Engine->GetChildByID(base_skill_view, "Rogue_72_off");
	Rogue_75_off = Engine->GetChildByID(base_skill_view, "Rogue_75_off");
	Rogue_80_off = Engine->GetChildByID(base_skill_view, "Rogue_80_off");
	Mage_70_off = Engine->GetChildByID(base_skill_view, "Mage_70_off");
	Mage_72_off = Engine->GetChildByID(base_skill_view, "Mage_72_off");
	Mage_75_off = Engine->GetChildByID(base_skill_view, "Mage_75_off");
	Mage_80_off = Engine->GetChildByID(base_skill_view, "Mage_80_off");
	Priest_70_off = Engine->GetChildByID(base_skill_view, "Priest_70_off");
	Priest_72_off = Engine->GetChildByID(base_skill_view, "Priest_72_off");
	Priest_74_off = Engine->GetChildByID(base_skill_view, "Priest_74_off");
	Priest_75_off = Engine->GetChildByID(base_skill_view, "Priest_75_off");
	Priest_76_off = Engine->GetChildByID(base_skill_view, "Priest_76_off");
	Priest_78_off = Engine->GetChildByID(base_skill_view, "Priest_78_off");
	Priest_80_off = Engine->GetChildByID(base_skill_view, "Priest_80_off");
}

void CSealToolTip::DropResultStatus(bool status)
{
	if (this)
	{
		Engine->SetVisible(m_btndropresult, status);
		Engine->SetState(m_btndropresult, status ? UI_STATE_BUTTON_NORMAL : UI_STATE_BUTTON_DISABLE);
	}
}

DWORD uiTool;
DWORD Func_tool;
bool CSealToolTip::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	uiTool = m_dVTableAddr;
	if (!pSender || pSender == (DWORD*)0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	if (pSender == (DWORD*)btn_view_perks)
	{
		cPage = 1;
		OrderPerk();
		Engine->SetVisible(perkInfo.base_perks_view, true);
		return true;
	}
	else if (pSender == (DWORD*)perkInfo.btn_back_view)
	{
		Engine->SetVisible(perkInfo.base_perks_view, false);
		return true;
	}

	return true;
}

void __stdcall UISealTooltipReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiSealToolTip->ReceiveMessage(pSender, dwMsg);
	__asm
	{
		MOV ECX, uiTool
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_tool
		CALL EAX
	}
}


void CSealToolTip::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_tool = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)UISealTooltipReceiveMessage_Hook;
}
