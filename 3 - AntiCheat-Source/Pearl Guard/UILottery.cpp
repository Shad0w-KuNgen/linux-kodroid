#include "UILottery.h"
#include "HDRReader.h"

RimaLottery::RimaLottery()
{
	for (int i = 0; i < 9; i++)
		itemIDs[i] = 0;
	m_Timer = NULL;
	str_remaining = NULL;

	ticket_buy_count = NULL;
}

RimaLottery::~RimaLottery()
{
	m_Timer = NULL;
}

bool RimaLottery::Load(HANDLE hFile)
{
	if (CN3UIBase::Load(hFile) == false)
		return false;

	std::string find = xorstr("group_reqs");
	group_reqs = GetChildByID(find);
	find = xorstr("group_rewards");
	group_rewards = GetChildByID(find);
	find = xorstr("group_details");
	group_details = GetChildByID(find);

	find = xorstr("str_remaining");
	str_remaining = (CN3UIString*)GetChildByID(find);

	find = xorstr("btn_close");
	btn_close = (CN3UIButton*)GetChildByID(find);

	find = xorstr("text_ticket_buy_count");
	ticket_buy_count = (CN3UIString*)group_reqs->GetChildByID(find);

	for (int i = 0; i < 5; i++) {
		find = string_format(xorstr("slot%d"), i + 1);
		reqSlots[i] = (CN3UIArea*)group_reqs->GetChildByID(find);
		itemIcons[i] = new CN3UIIcon();
		itemIcons[i]->Init(this);
		itemIcons[i]->SetUIType(UI_TYPE_ICON);
		itemIcons[i]->SetStyle(UISTYLE_ICON_ITEM | UISTYLE_ICON_CERTIFICATION_NEED);
		itemIcons[i]->SetUVRect(0, 0, fUVAspect, fUVAspect);
		itemIcons[i]->SetRegion(reqSlots[i]->GetRegion());
		itemIcons[i]->SetVisible(false);
	}

	for (int i = 0; i < 4; i++) {
		find = string_format(xorstr("slot%d"), i + 1);
		rewardSlots[i] = (CN3UIArea*)group_rewards->GetChildByID(find);
		itemIcons[i + 5] = new CN3UIIcon();
		itemIcons[i + 5]->Init(this);
		itemIcons[i + 5]->SetUIType(UI_TYPE_ICON);
		itemIcons[i + 5]->SetStyle(UISTYLE_ICON_ITEM | UISTYLE_ICON_CERTIFICATION_NEED);
		itemIcons[i + 5]->SetUVRect(0, 0, fUVAspect, fUVAspect);
		itemIcons[i + 5]->SetRegion(rewardSlots[i]->GetRegion());
		itemIcons[i + 5]->SetVisible(false);
	}

	find = xorstr("str_participant");
	str_participant = (CN3UIString*)group_details->GetChildByID(find);
	find = xorstr("str_ticket");
	str_ticket = (CN3UIString*)group_details->GetChildByID(find);
	find = xorstr("str_change");
	str_change = (CN3UIString*)group_details->GetChildByID(find);
	find = xorstr("btn_join");
	btn_join = (CN3UIButton*)group_reqs->GetChildByID(find);

	for (int i = 0; i < 9; i++)
		itemTbl[i] = nullptr;

	SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, Engine->m_UiMgr->GetScreenCenter(this).y);

	return true;
}

void RimaLottery::Update(uint32 reqs[5],uint32 reqsCount[5], uint32 rewards[4], bool reset)
{
	if (reset)
	{
		for (int i = 0; i < 5; i++)
		{
			itemIcons[i]->SetVisible(false);
		}
		for (int i = 0; i < 4; i++)
		{
			itemIcons[i + 5]->SetVisible(false);
		}
		for (int i = 0; i < 9; i++) {
			itemIDs[i] = 0;
			itemTbl[i] = nullptr;
		}
	}

	for (int i = 0; i < 5; i++)
	{
		itemIDs[i] = reqs[i];
		if (reqs[i] == 0)
			continue;

		TABLE_ITEM_BASIC * tbl = Engine->tblMgr->getItemData(reqs[i]);
		if (!tbl) 
		{
			itemTbl[i] = nullptr;
			continue;
		}

		auto pItemExt = Engine->tblMgr->getExtData(tbl->extNum, reqs[i]);
		if (TABLE_ITEM_EXT* ext = Engine->tblMgr->getExtData(tbl->extNum, rewards[i]))
		{
			DWORD dwIconID = tbl->dxtID;
			if (ext->dxtID > 0)
				dwIconID = ext->dxtID;

			std::string dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);

			itemIcons[i]->SetTex(dxt);
		}
		else {
			std::vector<char> buffer(256, NULL);

			sprintf(&buffer[0], "ui\\itemicon_%.1d_%.4d_%.2d_%.1d.dxt",
				(tbl->dxtID / 10000000),
				(tbl->dxtID / 1000) % 10000,
				(tbl->dxtID / 10) % 100,
				tbl->dxtID % 10);

			std::string szIconFN = &buffer[0];

			itemIcons[i]->SetTex(szIconFN);
		}
		itemIcons[i]->SetVisible(true);
		itemTbl[i] = tbl;
	}

	for (int i = 0; i < 4; i++)
	{
		itemIDs[i+5] = rewards[i];
		if (rewards[i] == 0)
			continue;

		TABLE_ITEM_BASIC * tbl = Engine->tblMgr->getItemData(rewards[i]);
		if (!tbl) 
		{
			itemTbl[i+5] = nullptr;
			continue;
		}

		
		if (TABLE_ITEM_EXT* ext = Engine->tblMgr->getExtData(tbl->extNum, rewards[i]))
		{
			DWORD dwIconID = tbl->dxtID;
			if (ext->dxtID > 0)
				dwIconID = ext->dxtID;

			std::string dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);

			itemIcons[i + 5]->SetTex(dxt);
		}
		else {
			std::vector<char> buffer(256, NULL);

			sprintf(&buffer[0], "ui\\itemicon_%.1d_%.4d_%.2d_%.1d.dxt",
				(tbl->dxtID / 10000000),
				(tbl->dxtID / 1000) % 10000,
				(tbl->dxtID / 10) % 100,
				tbl->dxtID % 10);

			std::string szIconFN = &buffer[0];

			itemIcons[i + 5]->SetTex(szIconFN);
		}

		itemIcons[i + 5]->SetVisible(true);
		itemTbl[i+5] = tbl;
	}
}

bool RimaLottery::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == btn_close)
		{ 
			Close();

			if (Engine->uiTopRightNewPlug != NULL)
			{
				if (!Engine->IsVisible(Engine->uiTopRightNewPlug->m_lottery) && Engine->uiTopRightNewPlug->m_lottery)
				{
					Engine->SetVisible(Engine->uiTopRightNewPlug->m_lottery, true);
					Engine->SetState(Engine->uiTopRightNewPlug->m_lottery, UI_STATE_BUTTON_NORMAL);
				}
			}			
		}
		else if (pSender == btn_join)
		{
			Packet pkt(XSafe, uint8(XSafeOpCodes::LOTTERY));
			pkt << uint8(3);
			Engine->Send(&pkt);
		}
	}

	return true;
}

uint32_t RimaLottery::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (Engine->m_UiMgr->m_FocusedUI == this && IsVisible())
	{
		bool showTooltip = false;

		for (int i = 0; i < 9; i++)
		{
			if (itemIcons[i]->IsIn(ptCur.x, ptCur.y) && itemTbl[i]) {
				Engine->m_UiMgr->m_pUITooltipDlg->DisplayTooltipsEnable(ptCur.x, ptCur.y, GetItemStruct((CN3UIImage*)itemIcons[i], itemIcons[i]->FileName(), itemTbl[i], itemIDs[i]), this, true, false, false);
				showTooltip = true;
			}
		}

		Engine->m_UiMgr->m_pUITooltipDlg->SetVisible(showTooltip);
	}
	dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
	return dwRet;
}

void RimaLottery::Tick()
{
	if (m_Timer == NULL)
		m_Timer = new CTimer();

	if (Engine->lotteryEvent.remainingTime > 0) {
		if (m_Timer->IsElapsedSecond())
			Engine->lotteryEvent.remainingTime--;

		std::string remainingTime;

		int h, m, s;

		h = (Engine->lotteryEvent.remainingTime / 3600);
		m = (Engine->lotteryEvent.remainingTime - (3600 * h)) / 60;
		s = (Engine->lotteryEvent.remainingTime - (3600 * h) - (m * 60));

		if (m < 10 && s < 10)
			remainingTime = string_format(xorstr("%d : 0%d : 0%d"), h, m, s);
		else if (m < 10)
			remainingTime = string_format(xorstr("%d : 0%d : %d"), h, m, s);
		else if (s < 10)
			remainingTime = string_format(xorstr("%d : %d : 0%d"), h, m, s);
		else
			remainingTime = string_format(xorstr("%d : %d : %d"), h, m, s);

		str_remaining->SetString(string_format(xorstr(" %s"), remainingTime.c_str()));
	}
}

bool RimaLottery::OnKeyPress(int iKey)
{
	if (!IsVisible())
		return CN3UIBase::OnKeyPress(iKey);
	if (iKey == DIK_ESCAPE)
		Close();

	return true;
}

void RimaLottery::Open()
{
	SetVisible(true);
}

void RimaLottery::Close()
{
	SetVisible(false);
}

__IconItemSkill * RimaLottery::GetItemStruct(CN3UIImage* img, std::string fileSz, __TABLE_ITEM_BASIC* tbl, uint32 realID)
{
	__IconItemSkill * newStruct = new __IconItemSkill;
	newStruct->pUIIcon = img;
	newStruct->szIconFN = fileSz;
	newStruct->index = 0;

	__TABLE_ITEM_BASIC* newItemBasic = new __TABLE_ITEM_BASIC;
	__TABLE_ITEM_BASIC* itemOrg = tbl;
	newItemBasic->Num = itemOrg->Num;
	newItemBasic->extNum = itemOrg->extNum;
	newItemBasic->strName = itemOrg->strName;
	newItemBasic->Description = itemOrg->Description;
	//newItemBasic->customStrings = itemOrg->customStrings;

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

	__TABLE_ITEM_EXT * newItemExt = Engine->tblMgr->getExtData(itemOrg->extNum, realID);

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

void RimaLottery::OpenLottery()
{
	std::string name = Engine->dcpUIF(xorstr("Msoft\\LotteryEvent.Msoft"));
	Engine->m_UiMgr->uiLottery = new RimaLottery();
	Engine->m_UiMgr->uiLottery->Init(this);
	Engine->m_UiMgr->uiLottery->LoadFromFile(Engine->m_BasePath + name.c_str(), N3FORMAT_VER_1068);
	Engine->m_UiMgr->AddChild(Engine->m_UiMgr->uiLottery);
	remove(name.c_str());
}