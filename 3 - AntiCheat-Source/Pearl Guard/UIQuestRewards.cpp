#include "UIQuestRewards.h"

CQuestRewards::CQuestRewards()
{
	btn_close = NULL;
	str_quest = NULL;
	str_zone = NULL;
	str_hunt = NULL;
	str_hunt_count = NULL;
	hunt_area = NULL;
	for (uint8 i = 0; i < 4; i++)
	{
		rewardBase[i].icon = NULL;
	}
}

CQuestRewards::~CQuestRewards()
{

}

bool CQuestRewards::Load(HANDLE hFile)
{
	if (CN3UIBase::Load(hFile) == false) return false;

	std::string find = xorstr("btn_close");
	btn_close = (CN3UIButton*)GetChildByID(find);
	find = xorstr("text_quest_name");
	str_quest = (CN3UIString*)GetChildByID(find);

	find = xorstr("text_zone");
	str_zone = (CN3UIString*)GetChildByID(find);

	find = xorstr("Hunt");
	CN3UIBase* hunt = (CN3UIBase*)GetChildByID(find);

	find = xorstr("text_hunt_name");
	str_hunt = (CN3UIString*)hunt->GetChildByID(find);
	find = xorstr("text_hunt_count");
	str_hunt_count = (CN3UIString*)hunt->GetChildByID(find);
	find = xorstr("hunt_area");
	hunt_area = (CN3UIArea*)hunt->GetChildByID(find);

	for (uint8 i = 0; i < 4; i++)
	{
		find = string_format(xorstr("Reward%d"), i+1);
		rewardBase[i].base = (CN3UIBase*)GetChildByID(find);

		find = xorstr("str_name");
		rewardBase[i].str_name = (CN3UIString*)rewardBase[i].base->GetChildByID(find);
		find = xorstr("str_count");
		rewardBase[i].str_count = (CN3UIString*)rewardBase[i].base->GetChildByID(find);
		find = xorstr("area");
		rewardBase[i].area = (CN3UIArea*)rewardBase[i].base->GetChildByID(find);
	}

	SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, Engine->m_UiMgr->GetScreenCenter(this).y);

	return true;
}

extern std::string GetZoneName(uint16 zoneID);

void CQuestRewards::OnKill(uint16 mobID)
{

///*
//	std::map<uint8, DailyQuest>::iterator it;
//	for (auto it : Engine->uiQuestPage->kcbq_quests)
//	{
//		if (it.Mob[0] == mobID || it->Mob[1] == mobID || it.Mob[2] == mobID || it.Mob[3] == mobID) {
//			it.MyCount++;
//			if(cMob == it.Mob[0] || cMob == it.Mob[1] || cMob == it.Mob[2] || cMob == it.Mob[3]) LoadInfo(it);
//
//			Engine->uiQuestPage->InitQuests(255);
//			break;
//		}
//	}*/
}

void CQuestRewards::LoadInfo(DailyQuest* quest)
{
	cMob = quest->Mob[0];

	for (uint8 i = 0; i < i; i++)
	{
		rewardBase[i].icon = NULL;
	}

	std::string zone = GetZoneName(quest->ZoneID);
	str_zone->SetString(zone);
	if (auto mob = Engine->tblMgr->getMobData(quest->Mob[0]))
	{
		str_quest->SetString(string_format(xorstr("%s"), mob->strName.c_str()));
		str_hunt->SetString(mob->strName);
	}
	else {
		if (quest->Mob[0] == 1) {
			str_quest->SetString(xorstr("<Kill Enemy>"));
			str_hunt->SetString(xorstr("<Kill Enemy>"));
		}
		else{
			str_quest->SetString(xorstr("<Unknown>"));
			str_hunt->SetString(xorstr("<Unknown>"));
		}
	}

	str_hunt_count->SetString(string_format(xorstr("%d / %d"), quest->MyCount, quest->KillTarget));

	float fUVAspect = (float)45.0f / (float)64.0f;
	CN3UIIcon* icon = new CN3UIIcon;
	icon->Init(hunt_area);
	
	icon->SetUVRect(0, 0, fUVAspect, fUVAspect);
	icon->SetUIType(UI_TYPE_ICON);
	icon->SetStyle(UISTYLE_ICON_ITEM | UISTYLE_ICON_CERTIFICATION_NEED);
	icon->SetVisible(true);

	uint32 OriginItemID = 900005000;

	TABLE_ITEM_BASIC * tbl = Engine->tblMgr->getItemData(OriginItemID);
	if (TABLE_ITEM_EXT* ext = Engine->tblMgr->getExtData(tbl->extNum, OriginItemID))
	{
		DWORD dwIconID = tbl->dxtID;
		if (ext->dxtID > 0)
			dwIconID = ext->dxtID;

		std::string dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);

		icon->SetTex(dxt);
	}
	else {
		std::vector<char> buffer(256, NULL);

		sprintf(&buffer[0], "ui\\itemicon_%.1d_%.4d_%.2d_%.1d.dxt",
			(tbl->dxtID / 10000000),
			(tbl->dxtID / 1000) % 10000,
			(tbl->dxtID / 10) % 100,
			tbl->dxtID % 10);

		std::string szIconFN = &buffer[0];

		icon->SetTex(szIconFN);
	}

	icon->SetRegion(hunt_area->GetRegion());

	// Rewards
	int rewardCount = 0;
	for (uint8 i = 0; i < 4; i++)
	{
		if (quest->Reward[i])
		{
			rewardCount++;
		}
	}

	for (uint8 i = 0; i < 4; i++)
	{
		if (quest->Reward[i])
		{
			rewardBase[i].nItemID = quest->Reward[i];
			rewardBase[i].base->SetVisible(true);
			rewardBase[i].icon = new CN3UIIcon;
			rewardBase[i].icon->Init(rewardBase[i].area);
			rewardBase[i].icon->SetUVRect(0, 0, fUVAspect, fUVAspect);
			rewardBase[i].icon->SetUIType(UI_TYPE_ICON);
			rewardBase[i].icon->SetStyle(UISTYLE_ICON_ITEM | UISTYLE_ICON_CERTIFICATION_NEED);
			rewardBase[i].icon->SetRegion(rewardBase[i].area->GetRegion());
			rewardBase[i].icon->SetVisible(true);

			uint32 OriginItemID = quest->Reward[i];

			if (TABLE_ITEM_BASIC * tbl = Engine->tblMgr->getItemData(OriginItemID)) {
				rewardBase[i].tbl = tbl;
				if (TABLE_ITEM_EXT* ext = Engine->tblMgr->getExtData(tbl->extNum, OriginItemID))
				{
					DWORD dwIconID = tbl->dxtID;
					if (ext->dxtID > 0)
						dwIconID = ext->dxtID;
					std::string dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);
					rewardBase[i].icon->SetTex(dxt);
				}
				else {
					std::vector<char> buffer(256, NULL);
					sprintf(&buffer[0], "ui\\itemicon_%.1d_%.4d_%.2d_%.1d.dxt",(tbl->dxtID / 10000000),(tbl->dxtID / 1000) % 10000,(tbl->dxtID / 10) % 100,tbl->dxtID % 10);
					std::string szIconFN = &buffer[0];
					rewardBase[i].icon->SetTex(szIconFN);
				}
				rewardBase[i].str_name->SetString(tbl->strName);
				rewardBase[i].str_count->SetString(to_string(quest->Count[i]));
			}
		}
		else rewardBase[i].base->SetVisible(false);
	}

}

bool CQuestRewards::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		 if (pSender == btn_close)
			Close();
	}

	return true;
}

uint32_t CQuestRewards::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (Engine->m_UiMgr->m_FocusedUI == this && Engine->m_UiMgr->m_pUITooltipDlg != NULL && IsVisible())
	{
		bool showTooltip = false;

		for (int i = 0; i < 4; i++)
		{
			if (rewardBase[i].icon != NULL) {
				if (rewardBase[i].icon->IsIn(ptCur.x, ptCur.y) && rewardBase[i].tbl != nullptr && rewardBase[i].nItemID != 0)
				{
					Engine->m_UiMgr->m_pUITooltipDlg->DisplayTooltipsEnable(ptCur.x, ptCur.y, GetItemStruct(rewardBase[i].icon, rewardBase[i].icon->FileName(), rewardBase[i].tbl, rewardBase[i].nItemID), this, false, true, true);
					showTooltip = true;
					break;
				}
			}
		}

		Engine->m_UiMgr->m_pUITooltipDlg->SetVisible(showTooltip);
	}
	dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
	return dwRet;
}

bool CQuestRewards::OnKeyPress(int iKey)
{
	if (Engine->m_UiMgr->uiQuestRewards == NULL)
		return CN3UIBase::OnKeyPress(iKey);

	if (!IsVisible())
		return CN3UIBase::OnKeyPress(iKey);

	switch (iKey)
	{
	case DIK_ESCAPE:
		Close();
		return true;
		break;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

void CQuestRewards::Open()
{
	SetVisible(true);
}

void CQuestRewards::Close()
{
	SetVisible(false);
}


__IconItemSkill * CQuestRewards::GetItemStruct(CN3UIImage* img, std::string fileSz, __TABLE_ITEM_BASIC* tbl, uint32 realID)
{
	__IconItemSkill* newStruct = new __IconItemSkill;
	newStruct->pUIIcon = img;
	newStruct->szIconFN = fileSz;
	newStruct->index = 0;

	__TABLE_ITEM_BASIC* newItemBasic = new __TABLE_ITEM_BASIC;
	__TABLE_ITEM_BASIC* itemOrg = tbl;
	newItemBasic->Num = itemOrg->Num;
	newItemBasic->extNum = itemOrg->extNum;
	newItemBasic->strName = itemOrg->strName;
	newItemBasic->Description = itemOrg->Description;

	newItemBasic->Class = itemOrg->Class;

	newItemBasic->Attack = itemOrg->Attack;
	newItemBasic->Delay = itemOrg->Delay;
	newItemBasic->Range = itemOrg->Range;
	newItemBasic->Weight = itemOrg->Weight;
	newItemBasic->Duration = itemOrg->Duration;
	newItemBasic->repairPrice = itemOrg->repairPrice;
	newItemBasic->sellingPrice = itemOrg->sellingPrice;
	newItemBasic->AC = itemOrg->AC;
	newItemBasic->isCountable = itemOrg->isCountable;
	newItemBasic->ReqLevelMin = itemOrg->ReqLevelMin;

	newItemBasic->ReqStr = itemOrg->ReqStr;
	newItemBasic->ReqHp = itemOrg->ReqHp;
	newItemBasic->ReqDex = itemOrg->ReqDex;
	newItemBasic->ReqInt = itemOrg->ReqInt;
	newItemBasic->ReqMp = itemOrg->ReqMp;

	newItemBasic->SellingGroup = itemOrg->SellingGroup;

	newStruct->pItemBasic = newItemBasic;

	__TABLE_ITEM_EXT* newItemExt = Engine->tblMgr->getExtData(itemOrg->extNum, realID);

	if (newItemExt == nullptr)
	{
		newItemExt = new __TABLE_ITEM_EXT;
		newItemExt->extensionID = itemOrg->extNum;
		newItemExt->szHeader = itemOrg->strName;
		newItemExt->byMagicOrRare = itemOrg->ItemGrade;
		newItemExt->siDamage = 0;
		newItemExt->siAttackIntervalPercentage = 100;
		newItemExt->siHitRate = 0;
		newItemExt->siEvationRate = 0;
		newItemExt->siMaxDurability = 0;
		newItemExt->siDefense = 0;
		newItemExt->siDefenseRateDagger = 0;
		newItemExt->siDefenseRateSword = 0;
		newItemExt->siDefenseRateBlow = 0;
		newItemExt->siDefenseRateAxe = 0;
		newItemExt->siDefenseRateSpear = 0;
		newItemExt->siDefenseRateArrow = 0;
		newItemExt->byDamageFire = 0;
		newItemExt->byDamageIce = 0;
		newItemExt->byDamageThuner = 0;
		newItemExt->byDamagePoison = 0;
		newItemExt->byStillHP = 0;
		newItemExt->byDamageMP = 0;
		newItemExt->byStillMP = 0;
		newItemExt->siBonusStr = 0;
		newItemExt->siBonusSta = 0;
		newItemExt->siBonusHP = 0;
		newItemExt->siBonusDex = 0;
		newItemExt->siBonusMSP = 0;
		newItemExt->siBonusInt = 0;
		newItemExt->siBonusMagicAttak = 0;
		newItemExt->siRegistFire = 0;
		newItemExt->siRegistIce = 0;
		newItemExt->siRegistElec = 0;
		newItemExt->siRegistMagic = 0;
		newItemExt->siRegistPoison = 0;
		newItemExt->siRegistCurse = 0;
		newItemExt->siNeedLevel = 0;
	}

	newStruct->pItemExt = newItemExt;

	return newStruct;
}