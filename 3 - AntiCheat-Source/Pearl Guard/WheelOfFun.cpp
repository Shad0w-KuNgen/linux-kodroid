#include "WheelOfFun.h"
#include "HDRReader.h"
WheelOfFun::WheelOfFun()
{

	lastItem = 0;
	KnightCash = NULL;
	base_item = NULL;
	btn_start = NULL;
	btn_close = NULL;
	btn_stop = NULL;
	base_item = NULL;
	TickSay = clock();
	isActiveWhile = false;


}

WheelOfFun::~WheelOfFun()
{

}
bool WheelOfFun::Load(HANDLE hFile)
{
	if (CN3UIBase::Load(hFile) == false) return false;


	std::string find = xorstr("base_item");


	char szBuf[64];
	for (int i = 0; i < 25; i++)
	{

		sprintf(szBuf, "item%.2d", i);
		nItem[i] = (CN3UIImage*)GetChildByID(szBuf);
	}
	find = xorstr("btn_stop");
	btn_stop = (CN3UIButton*)GetChildByID(find);
	find = xorstr("btn_start");
	btn_start = (CN3UIButton*)GetChildByID(find);
	find = xorstr("btn_close");
	btn_close = (CN3UIButton*)GetChildByID(find);
	find = xorstr("txt_cash");
	KnightCash = (CN3UIString*)GetChildByID(find);


	SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, Engine->m_UiMgr->GetScreenCenter(this).y);

	return true;
}
void WheelOfFun::ImageChange(int8 sID)
{
	auto it = Engine->WheelItemData.find(sID);
	if (it == Engine->WheelItemData.end())
		return;

	nItem[sID]->SetTex(Engine->GetItemDXT(it->second));
	nItem[lastItem]->SetTex("ui\\itemicon_noimage.dxt");
	lastItem = sID;
}
void WheelOfFun::StartChallange()
{
	for (int i = 0; i < 15; i++)
		nItem[i]->SetTex("ui\\itemicon_noimage.dxt");

	btn_start->SetState(UI_STATE_BUTTON_DISABLE);
	btn_stop->SetState(UI_STATE_BUTTON_NORMAL);
	isActiveWhile = true;

}
void WheelOfFun::GiveItemImage(uint32 nItemID)
{
	auto it = Engine->WheelItemData.begin();
	while (it != Engine->WheelItemData.end())
	{
		if (it->second == nItemID)
		{
			for (int i = 0; i < 15; i++)
				nItem[i]->SetTex("ui\\itemicon_noimage.dxt"); // hepsini gizle

			nItem[it->first]->SetTex(Engine->GetItemDXT(it->second)); // çýkan itemi göster

			break;
		}

		it++;
	}

}
void WheelOfFun::StopChallange()
{
	btn_start->SetState(UI_STATE_BUTTON_NORMAL);
	btn_stop->SetState(UI_STATE_BUTTON_DISABLE);
	isActiveWhile = false;
}
void WheelOfFun::IsActive(bool enable)
{
	SetVisible(enable);

	if (!isActiveWhile)
	{
		for (int i = 0; i < 15; i++)
		{
			auto it = Engine->WheelItemData.find(i);
			if (it == Engine->WheelItemData.end())
				return;

			nItem[i]->SetTex(Engine->GetItemDXT(it->second));

		}


	}
	if (enable)
	{
		Engine->m_UiMgr->SetFocusedUI(this);
		KnightCash->SetString(string_format("Knight Cash : %d", Engine->uiState->m_iCash).c_str());
		SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, Engine->m_UiMgr->GetScreenCenter(this).y);

	}
}
int myrands(int a, int b)
{
	int div = b - a;
	return a + (rand() % (b - a));
}
void WheelOfFun::Tick()
{
	if (!IsVisible() || !isActiveWhile)
		return;

	if (TickSay < clock() - 120)
	{
		TickSay = clock();

		int8 Rand = myrands(0, 24);

		ImageChange(Rand);
	}

}
bool WheelOfFun::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == btn_start)
			StartChallange();
		else if (pSender == btn_stop)
		{
			Packet pkt(XSafe, uint8_t(XSafeOpCodes::WheelData));
			pkt << uint8(1);
			Engine->Send(&pkt);
		}
		else if (pSender == btn_close)
			IsActive(false);
	}
	return true;

}
uint32_t WheelOfFun::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{

	if (!this)
		return false;
	if (!m_bVisible) return false;
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (Engine->m_UiMgr->m_FocusedUI == this)
	{
		bool showTooltip = false;

		int i = 0;
		for (auto it : Engine->WheelItemData)
		{
			if (i >= Engine->WheelItemData.size())
				break;


			if (nItem[i]->GetParent()->IsVisible() && nItem[i]->IsIn(ptCur.x, ptCur.y))
			{
				if (Engine->str_contains(nItem[i]->GetTexFN().c_str(), xorstr("itemicon_noimage.dxt")))
					continue;

				printf("%s \n", nItem[i]->GetTexFN().c_str());
				TABLE_ITEM_BASIC* item = Engine->tblMgr->getItemData(it.second);
				if (!item) {
					i++;
					continue;
				}

				Engine->m_UiMgr->m_pUITooltipDlg->DisplayTooltipsEnable(ptCur.x, ptCur.y, GetItemStruct((CN3UIImage*)nItem[i], nItem[i]->FileName(), item, it.second), this, true, false, false);
				showTooltip = true;
				break;
			}
			i++;
		}

		Engine->m_UiMgr->m_pUITooltipDlg->SetVisible(showTooltip);
	}
	dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
	return dwRet;
}
__IconItemSkill* WheelOfFun::GetItemStruct(CN3UIImage* img, std::string fileSz, __TABLE_ITEM_BASIC* tbl, uint32 realID)
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

void WheelOfFun::OpenWheel()
{
#if (HOOK_SOURCE_VERSION == 1098)
	uint8 Nation = *(uint8*)(*(DWORD*)KO_PTR_CHR + KO_OFF_NATION);
	if (Engine->m_UiMgr->uiWheel == NULL)
	{
		if (Nation == 1)
		{
			std::string name = Engine->dcpUIF(xorstr("Msoft\\Ka_WhellOffun.Msoft"));
			Engine->m_UiMgr->uiWheel = new WheelOfFun();
			Engine->m_UiMgr->uiWheel->Init(this);
			Engine->m_UiMgr->uiWheel->LoadFromFile(Engine->m_BasePath + name.c_str(), N3FORMAT_VER_1068);
			Engine->m_UiMgr->AddChild(Engine->m_UiMgr->uiWheel);
			remove(name.c_str());
		}
		else if (Nation == 2)
		{
			std::string name = Engine->dcpUIF(xorstr("Msoft\\El_WhellOffun.Msoft"));
			Engine->m_UiMgr->uiWheel = new WheelOfFun();
			Engine->m_UiMgr->uiWheel->Init(this);
			Engine->m_UiMgr->uiWheel->LoadFromFile(Engine->m_BasePath + name.c_str(), N3FORMAT_VER_1068);
			Engine->m_UiMgr->AddChild(Engine->m_UiMgr->uiWheel);
			remove(name.c_str());
		}
	}
	Engine->m_UiMgr->uiWheel->IsActive(true);
#else
	std::string name = Engine->dcpUIF(xorstr("Msoft\\WhellOffun.Msoft"));
	Engine->m_UiMgr->uiWheel = new WheelOfFun();
	Engine->m_UiMgr->uiWheel->Init(this);
	Engine->m_UiMgr->uiWheel->LoadFromFile(Engine->m_BasePath + name.c_str(), N3FORMAT_VER_1068);
	Engine->m_UiMgr->AddChild(Engine->m_UiMgr->uiWheel);
	remove(name.c_str());
#endif
}