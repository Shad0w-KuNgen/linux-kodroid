#include "PerkMenu.h"
#include "HDRReader.h"

extern std::string GetName(DWORD obj);
CPerkMenu::CPerkMenu()
{
	str_info = str_Nick = str_page = NULL;
	btn_left = btn_close = btn_right = NULL;
	for (int i = 0; i < UI_PERKCOUNT; i++)
		PerksBonus[i] = perkInfo();
	cPage = pageCount = 1;
	memset(perkType, 0, sizeof(perkType));
}

CPerkMenu::~CPerkMenu()
{

}

bool CPerkMenu::Load(HANDLE hFile)
{
	if (CN3UIBase::Load(hFile) == false)
		return false;

	str_info = (CN3UIString*)GetChildByID(xorstr("str_info"));
	str_Nick = (CN3UIString*)GetChildByID(xorstr("str_Nick"));
	str_page = (CN3UIString*)GetChildByID(xorstr("str_page"));

	btn_left = (CN3UIButton*)GetChildByID(xorstr("btn_left"));
	btn_close = (CN3UIButton*)GetChildByID(xorstr("btn_close"));
	btn_right = (CN3UIButton*)GetChildByID(xorstr("btn_right"));
	for (int i = 0; i < UI_PERKCOUNT; i++) {
		PerksBonus[i].base = (CN3UIBase*)GetChildByID(string_format("bonus%d", i + 1));
		PerksBonus[i].str_perk = (CN3UIString*)PerksBonus[i].base->GetChildByID(xorstr("str_perk"));
		PerksBonus[i].str_descp = (CN3UIString*)PerksBonus[i].base->GetChildByID(xorstr("str_descp"));
		PerksBonus[i].str_perk->SetColor(D3DCOLOR_RGBA(255, 60, 60, 255));
	}

	SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, Engine->m_UiMgr->GetScreenCenter(this).y);
	Engine->m_UiMgr->SetFocusedUI(this);
	return true;
}

void CPerkMenu::Close()
{
	SetVisible(false);
}

void CPerkMenu::Open() {

	SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, Engine->m_UiMgr->GetScreenCenter(this).y);

	SetVisible(true);
	Engine->m_UiMgr->SetFocusedUI(this);
}

void CPerkMenu::reOrderPerk(uint16 page)
{
	for (int i = 0; i < UI_PERKCOUNT; i++)
	{
		PerksBonus[i].base->SetVisible(false);
		PerksBonus[i].str_descp->SetString("");
		PerksBonus[i].str_perk->SetString("");
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
	str_page->SetString(string_format("%d/%d", cPage, pageCount));

	int p = (page - 1);
	if (p < 0) p = 0;
	size_t i = abs(p * UI_PERKCOUNT);

	uint8 c_slot = 0; uint16 count = 0;
	foreach(itr, mPerks)
	{
		count++;
		if (count <= i)
			continue;

		PerksBonus[c_slot].base->SetVisible(true);
		PerksBonus[c_slot].str_descp->SetString(itr->strDescp);
		PerksBonus[c_slot].str_perk->SetString(std::to_string(perkType[itr->pIndex]));

		c_slot++;
		if (c_slot >= UI_PERKCOUNT)
			break;
	}
}

void CPerkMenu::LoadInfo(uint16 perk[PERK_COUNT], uint16 targetID)
{
	DWORD target = Engine->GetTarget(targetID);
	std::string name = "";
	if (target) name = GetName(target);

	str_Nick->SetString(name);
	str_info->SetString(string_format("%s's perk information.", name.c_str()));
	memcpy(perkType, perk, sizeof(perkType));
	cPage = pageCount = 1;
	reOrderPerk();
	Open();
}

bool CPerkMenu::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	if (pSender == btn_close) {
		Close();
	}
	else if (pSender == btn_right)
	{
		if (cPage >= pageCount)
			return true;

		cPage++;
		str_page->SetString(string_format("%d/%d", cPage, pageCount));
		reOrderPerk(cPage);
		return true;
	}
	else if (pSender == btn_left)
	{
		if (cPage > 1) {
			cPage--;
			str_page->SetString(string_format("%d/%d", cPage, pageCount));
			reOrderPerk(cPage);
		}
		return true;
	}
	return true;
}

bool CPerkMenu::OnKeyPress(int iKey)
{
	if (!IsVisible())
		return CN3UIBase::OnKeyPress(iKey);

	switch (iKey)
	{
	case DIK_ESCAPE:
		Close();
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}
