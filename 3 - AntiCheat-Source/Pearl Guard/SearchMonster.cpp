#include "SearchMonster.h"
#include <cctype>

SearchMonster::SearchMonster()
{
	needToReset = true;
	page = 0;
	btn_close = NULL;
	int i = 0;
	oldLen = 0;
}

CN3UIString* SearchMonster::GetItemText(CN3UIButton* item)
{
	return (CN3UIString*)item->GetChildByID("txt_mob");
}

SearchMonster::~SearchMonster()
{
	btn_close = NULL;
}

extern std::string GetName(DWORD obj);
extern tGetObjectBase Func_GetObjectBase;


bool SearchMonster::Load(HANDLE hFile)
{
	if (CN3UIBase::Load(hFile) == false) return false;

	std::string find = xorstr("btn_close");
	btn_close = (CN3UIButton*)GetChildByID(find);
	find = xorstr("edit_search");
	edit_search = (CN3UIEdit*)GetChildByID(find);
	find = xorstr("edit_search");
	mob_search = (CN3UIString*)edit_search->GetChildByID(find);
	find = xorstr("btn_search");
	btn_search = (CN3UIButton*)GetChildByID(find);
	find = xorstr("btn_next");
	btn_next = (CN3UIButton*)GetChildByID(find);
	find = xorstr("btn_previously");
	btn_previously = (CN3UIButton*)GetChildByID(find);
	find = xorstr("btn_drops");
	btn_drops = (CN3UIButton*)GetChildByID(find);
	find = xorstr("Drop_list");
	Drop_list = (CN3UIList*)GetChildByID(find);
	find = xorstr("text_page");
	text_page = (CN3UIString*)GetChildByID(find);
	float fUVAspect = (float)45.0f / (float)64.0f;
	for (uint8 i = 0; i < 12; i++) {
		items[i] = (CN3UIArea*)GetChildByID(string_format("item%d", i));
		CN3UIIcon* nIcon = new CN3UIIcon();
		nIcon->Init(this);
		nIcon->SetUIType(UI_TYPE_ICON);
		nIcon->SetStyle(UISTYLE_ICON_ITEM | UISTYLE_ICON_CERTIFICATION_NEED);
		nIcon->SetRegion(items[i]->GetRegion());
		nIcon->SetUVRect(0, 0, fUVAspect, fUVAspect);

		ItemInfo * inf = new ItemInfo();
		inf->icon = nIcon;
		inf->tbl = NULL;
		inf->nItemID = 0;

		itemsx.push_back(inf);
	}

	find = xorstr("txt_monster_name");
	txt_monster_name = (CN3UIString*)GetChildByID(find);
	find = xorstr("str_title");
	str_title = (CN3UIString*)GetChildByID(find);

	SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, Engine->m_UiMgr->GetScreenCenter(this).y);
	SetupAnimation();

	std::thread th = std::thread([&]() {
		while (true) {
			if (edit_search->GetString().empty())
			{
				auto mobs = Engine->tblMgr->GetMobTable();
				Func_GetObjectBase = (tGetObjectBase)KO_FMBS;
				monsters.clear();
				for (uint16 i = 10000; i < 15000; i++) {
					if (DWORD mob = Func_GetObjectBase(*(DWORD*)KO_FLDB, i, 1))
					{
						std::string mobName = GetName(mob);
						if (mobName.empty())
							continue;

						uint16 protoID = *(uint16*)(mob + KO_SSID);

						if (mobs.find(protoID) == mobs.end())
							continue;

						if (std::find_if(monsters.begin(), monsters.end(), [&](const MONSTER_INFO &a) { return a.sID == protoID; }) == monsters.end())
							monsters.push_back(MONSTER_INFO(protoID, mobName));
					}
				}

				if (oldLen != monsters.size()) {
					int iSelPrev = Drop_list->GetCurSel();
					Drop_list->ResetContent();
					for (auto &mob : monsters)
						Drop_list->AddString(mob.strName);
					oldLen = monsters.size();
					Drop_list->SetCurSel(iSelPrev);
				}
			}
			Sleep(1000);
		}
		
	});

	th.detach();

	return true;
}

bool findStringIC(const std::string& strHaystack, const std::string& strNeedle)
{
	auto it = std::search(
		strHaystack.begin(), strHaystack.end(),
		strNeedle.begin(), strNeedle.end(),
		[](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
	);
	return (it != strHaystack.end());
}

bool SearchMonster::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == btn_close)
			Close();
		else if (pSender == btn_search)
		{
			std::string query = mob_search->GetString().c_str();
			if (query.length() < 3)
				return true;

			monsters.clear();
			auto mobs = Engine->tblMgr->GetMobTable();
			map<uint32, TABLE_MOB*>::iterator it;
			for (it = mobs.begin(); it != mobs.end(); it++)
			{
				TABLE_MOB mob = *it->second;
				if (findStringIC(mob.strName, query))
					monsters.push_back(MONSTER_INFO(mob.ID, mob.strName));
			}
			Drop_list->ResetContent();
			for (auto &mob : monsters)
				Drop_list->AddString(mob.strName);
			needToReset = true;
		}
		else if (pSender == btn_drops)
		{
			auto index = Drop_list->GetCurSel();
			if (index < 0)
				return true;
			if (index >= monsters.size())
				return true;

			auto mob = monsters[index];

			Packet pkt(XSafe, uint8(XSafeOpCodes::DROP_REQUEST));
			pkt << uint8(3) << uint16(mob.sID);
			Engine->Send(&pkt);
		}
		else if (pSender == btn_previously && m_CurrentPage > 1)
		{
			Order(--m_CurrentPage);
			if (text_page)
				text_page->SetString(string_format(xorstr("%d/%d"), m_CurrentPage, m_MaxPage));
		}
		else if (pSender == btn_next && m_CurrentPage < m_MaxPage)
		{
			Order(++m_CurrentPage);
			if (text_page)
				text_page->SetString(string_format(xorstr("%d/%d"), m_CurrentPage, m_MaxPage));
		}
	}
	return true;
}


void SearchMonster::Update(uint16 target, uint8 isMonster, vector<DropItem> drops)
{
	TABLE_MOB * mob = Engine->tblMgr->getMobData(target);
	if (mob != nullptr) {
		txt_monster_name->SetString(mob->strName.c_str());
	}
	else {
		txt_monster_name->SetString(xorstr("<unknown>"));
	}

	m_Drops = drops;
	Order();
}

uint32_t SearchMonster::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;

	if (Engine->m_UiMgr->m_FocusedUI == this)
	{
		bool showTooltip = false;

		for (auto it : itemsx)
		{
			if (it->icon->IsIn(ptCur.x, ptCur.y) && it->tbl != nullptr)
			{
				Engine->m_UiMgr->m_pUITooltipDlg->DisplayTooltipsEnable(ptCur.x, ptCur.y, GetItemStruct(it->icon, it->icon->FileName(), it->tbl, it->nItemID), this, false, true, true);
				showTooltip = true;
				break;
			}
		}

		Engine->m_UiMgr->m_pUITooltipDlg->SetVisible(showTooltip);
	}

	dwRet = CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
	return dwRet;
}

bool SearchMonster::OnKeyPress(int iKey)
{
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

void SearchMonster::OpenWithAnimation(uint32 frameTime)
{
	if (!IsVisible()) {
		auto mobs = Engine->tblMgr->GetMobTable();
		Func_GetObjectBase = (tGetObjectBase)KO_FMBS;
		monsters.clear();
		for (uint16 i = 10000; i < 15000; i++) {
			if (DWORD mob = Func_GetObjectBase(*(DWORD*)KO_FLDB, i, 1))
			{
				std::string mobName = GetName(mob);
				if (mobName.empty())
					continue;

				uint16 protoID = *(uint16*)(mob + KO_SSID);

				if (mobs.find(protoID) == mobs.end())
					continue;

				if (std::find_if(monsters.begin(), monsters.end(), [&](const MONSTER_INFO &a) { return a.sID == protoID; }) == monsters.end())
					monsters.push_back(MONSTER_INFO(protoID, mobName));
			}
		}

		int iSelPrev = Drop_list->GetCurSel();
		Drop_list->ResetContent();
		for (auto &mob : monsters)
			Drop_list->AddString(mob.strName);
		oldLen = monsters.size();
		Drop_list->SetCurSel(iSelPrev);

		fps = frameTime;
		frame = 0;
		frameCounter = 0;
		animated = true;
		SetPos(startRect.left, startRect.top);
		SetVisible(true);
	}
}

void SearchMonster::Open()
{
	if (!IsVisible()) {
		auto mobs = Engine->tblMgr->GetMobTable();
		Func_GetObjectBase = (tGetObjectBase)KO_FMBS;
		monsters.clear();
		for (uint16 i = 10000; i < 15000; i++) {
			if (DWORD mob = Func_GetObjectBase(*(DWORD*)KO_FLDB, i, 1))
			{
				std::string mobName = GetName(mob);
				if (mobName.empty())
					continue;

				uint16 protoID = *(uint16*)(mob + KO_SSID);

				if (mobs.find(protoID) == mobs.end())
					continue;

				if (std::find_if(monsters.begin(), monsters.end(), [&](const MONSTER_INFO &a) { return a.sID == protoID; }) == monsters.end())
					monsters.push_back(MONSTER_INFO(protoID, mobName));
			}
		}

		int iSelPrev = Drop_list->GetCurSel();
		Drop_list->ResetContent();
		for (auto &mob : monsters)
			Drop_list->AddString(mob.strName);
		oldLen = monsters.size();
		Drop_list->SetCurSel(iSelPrev);

		SetVisible(true);
	}
}

void SearchMonster::Close()
{
	monsters.clear();
	Drop_list->ResetContent();
	edit_search->SetString("");
	edit_search->KillFocus();
	for (auto& it : itemsx)
	{
		it->nItemID = NULL;
		it->tbl = nullptr;
		it->icon->SetTex("");
		txt_monster_name->SetString("");
	}
	SetVisible(false);
}

void SearchMonster::OpenSearchMonster()
{
#if (HOOK_SOURCE_VERSION == 1098)
	uint8 Nation = *(uint8*)(*(DWORD*)KO_PTR_CHR + KO_OFF_NATION);
	if (Nation == 1)
	{
		std::string name = Engine->dcpUIF(xorstr("Msoft\\Ka_SearchMonsterDrop.Msoft"));
		Engine->m_UiMgr->uiSearchMonster = new SearchMonster();
		Engine->m_UiMgr->uiSearchMonster->Init(this);
		Engine->m_UiMgr->uiSearchMonster->LoadFromFile(Engine->m_BasePath + name.c_str(), N3FORMAT_VER_1068);
		Engine->m_UiMgr->AddChild(Engine->m_UiMgr->uiSearchMonster);
		remove(name.c_str());
}
	else if (Nation == 2)
	{
		std::string name = Engine->dcpUIF(xorstr("Msoft\\El_SearchMonsterDrop.Msoft"));
		Engine->m_UiMgr->uiSearchMonster = new SearchMonster();
		Engine->m_UiMgr->uiSearchMonster->Init(this);
		Engine->m_UiMgr->uiSearchMonster->LoadFromFile(Engine->m_BasePath + name.c_str(), N3FORMAT_VER_1068);
		Engine->m_UiMgr->AddChild(Engine->m_UiMgr->uiSearchMonster);
		remove(name.c_str());
	}
#else
	std::string name = Engine->dcpUIF(xorstr("Msoft\\SearchMonsterDrop.Msoft"));
	Engine->m_UiMgr->uiSearchMonster = new SearchMonster();
	Engine->m_UiMgr->uiSearchMonster->Init(this);
	Engine->m_UiMgr->uiSearchMonster->LoadFromFile(Engine->m_BasePath + name.c_str(), N3FORMAT_VER_1068);
	Engine->m_UiMgr->AddChild(Engine->m_UiMgr->uiSearchMonster);
	remove(name.c_str());
#endif
}

__IconItemSkill* SearchMonster::GetItemStruct(CN3UIImage* img, std::string fileSz, __TABLE_ITEM_BASIC* tbl, uint32 realID)
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