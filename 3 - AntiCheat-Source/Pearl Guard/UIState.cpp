#include "UIState.h"
#include "stdafx.h"

CUIStatePlug::CUIStatePlug()
{
	vector<int>offsets;
	offsets.push_back(0x50);
	offsets.push_back(0);
	offsets.push_back(0x1B8);
	offsets.push_back(0x134);
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);

	m_txtCash = 0;
	m_txtDagger = 0;
	m_txtSword = 0;
	m_txtClub = 0;
	m_txtAxe = 0;
	m_txtSpear = 0;
	m_txtArrow = 0;
	m_txtJamadar = 0;
	amktick = GetTickCount() + 150;
	m_iDagger = 0;
	m_iSword = 0;
	m_iClub = 0;
	m_iAxe = 0;
	m_iSpear = 0;
	m_iArrow = 0;
	m_iJamadar = 0;
	//m_btnResetReb = 0;
	//getStat = 0;
	//m_iCash = 0;
	////--------------------
	m_btnStr10 = 0;
	m_btnHp10 = 0;
	m_btnDex10 = 0;
	m_btnMp10 = 0;
	m_btnInt10 = 0;
	m_btnReset = 0;
	//--------------------

	cPage = pageCount = 1;
	_basePerks = basePerks();
	str_remPerkCount = 0;
	btn_perks = 0;
	back_character_page = 0;
	perkUseTick = 0;
	btn_reset_perks = 0;

	ParseUIElements();
	InitReceiveMessage();
 }

CUIStatePlug::~CUIStatePlug()
{
}

void CUIStatePlug::ParseUIElements()
{
	m_txtCash = Engine->GetChildByID(m_dVTableAddr, "Text_Cash");
	m_txtTLBalance = Engine->GetChildByID(m_dVTableAddr, "Text_tl_Cash");
	m_txtDagger = Engine->GetChildByID(m_dVTableAddr, "Text_DaggerAc");
	m_txtSword = Engine->GetChildByID(m_dVTableAddr, "Text_SwordAc");
	m_txtClub = Engine->GetChildByID(m_dVTableAddr, "Text_MaceAc");
	m_txtAxe = Engine->GetChildByID(m_dVTableAddr, "Text_AxeAc");
	m_txtSpear = Engine->GetChildByID(m_dVTableAddr, "Text_SpearAc");
	m_txtArrow = Engine->GetChildByID(m_dVTableAddr, "Text_BowAc");
	m_strFreeStatPoint = Engine->GetChildByID(m_dVTableAddr, "Text_BonusPoint");
	m_btnStr10 = Engine->GetChildByID(m_dVTableAddr, "btn_str");
	m_btnHp10 = Engine->GetChildByID(m_dVTableAddr, "btn_sta");
	m_btnDex10 = Engine->GetChildByID(m_dVTableAddr, "btn_Dex");
	m_btnMp10 = Engine->GetChildByID(m_dVTableAddr, "btn_Magic");
	m_btnInt10 = Engine->GetChildByID(m_dVTableAddr, "btn_Int");
	m_btnReset = Engine->GetChildByID(m_dVTableAddr, "btn_reset_stat");
	m_orgbtnReset = Engine->GetChildByID(m_dVTableAddr, "btn_preset");
	_basePerks.base = Engine->GetChildByID(m_dVTableAddr, "base_perks");
	_basePerks.str_page = Engine->GetChildByID(_basePerks.base, "str_page");
	_basePerks.btn_right = Engine->GetChildByID(_basePerks.base, "btn_right");
	_basePerks.btn_left = Engine->GetChildByID(_basePerks.base, "btn_left");
	str_remPerkCount = Engine->GetChildByID(_basePerks.base, "str_remPerkCount");
	back_character_page = Engine->GetChildByID(_basePerks.base, "back_character_page");
	btn_reset_perks = Engine->GetChildByID(_basePerks.base, "btn_reset_perks");
	_basePerks.base_bonus = Engine->GetChildByID(_basePerks.base, "base_bonus");

	for (int i = 0; i < UI_PERKCOUNT; i++) {
		_basePerks._basePerksBonus[i].base = Engine->GetChildByID(_basePerks.base_bonus, string_format("bonus%d", i + 1));
		_basePerks._basePerksBonus[i].btn_plus = Engine->GetChildByID(_basePerks._basePerksBonus[i].base, xorstr("btn_plus"));
		_basePerks._basePerksBonus[i].str_perk = Engine->GetChildByID(_basePerks._basePerksBonus[i].base, xorstr("str_perk"));
		_basePerks._basePerksBonus[i].str_maxPerk = Engine->GetChildByID(_basePerks._basePerksBonus[i].base, xorstr("str_maxPerk"));
		_basePerks._basePerksBonus[i].str_descp = Engine->GetChildByID(_basePerks._basePerksBonus[i].base, xorstr("str_descp"));
	}
	btn_perks = Engine->GetChildByID(m_dVTableAddr, "btn_perks");	
	resetPerks();
}

void CUIStatePlug::SetAntiDefInfo(Packet& pkt)
{
	pkt >> m_iDagger >> m_iSword >> m_iAxe >> m_iClub >> m_iSpear >> m_iArrow >> m_iJamadar >> Engine->moneyReq;

	Engine->SetString(m_txtDagger, std::to_string(m_iDagger));
	Engine->SetString(m_txtSword, std::to_string(m_iSword));
	Engine->SetString(m_txtAxe, std::to_string(m_iAxe));
	Engine->SetString(m_txtClub, std::to_string(m_iClub));
	Engine->SetString(m_txtSpear, std::to_string(m_iSpear));
	Engine->SetString(m_txtArrow, std::to_string(m_iArrow));
	//Engine->SetString(m_txtJamadar, std::to_string(m_iJamadar));
}

void CUIStatePlug::UpdateKC(uint32 cash, uint32 balance)
{
	Engine->SetString(m_txtCash, Engine->StringHelper->NumberFormat(cash));
	Engine->SetString(m_txtTLBalance, Engine->StringHelper->NumberFormat(balance));
}

bool CUIStatePlug::UpdatePointButton(bool visible)
{
	Engine->SetVisible(m_btnStr10, visible);
	Engine->SetVisible(m_btnHp10, visible);
	Engine->SetVisible(m_btnDex10, visible);
	Engine->SetVisible(m_btnInt10, visible);
	Engine->SetVisible(m_btnMp10, visible);
	return true;
}
void CUIStatePlug::UpdateUI()
{
	Engine->SetString(m_txtCash, Engine->StringHelper->NumberFormat(m_iCash));
	Engine->SetString(m_txtTLBalance, Engine->StringHelper->NumberFormat(m_iBalance));
	Engine->SetString(m_txtDagger, std::to_string(m_iDagger));
	Engine->SetString(m_txtSword, std::to_string(m_iSword));
	Engine->SetString(m_txtAxe, std::to_string(m_iAxe));
	Engine->SetString(m_txtClub, std::to_string(m_iClub));
	Engine->SetString(m_txtSpear, std::to_string(m_iSpear));
	Engine->SetString(m_txtArrow, std::to_string(m_iArrow));
	//Engine->SetString(m_txtJamadar, std::to_string(m_iJamadar));
}

DWORD uiStateVTable;
bool CUIStatePlug::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	uiStateVTable = m_dVTableAddr;
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
	{
		if (pSender == (DWORD*)m_btnReset) {
			std::string reqcoins = "Would you like to reset your stat points for :    " + std::to_string(Engine->Player.moneyreq) + " coins.";
			Engine->m_UiMgr->ShowMessageBox("Stat Reset", reqcoins, MsgBoxTypes::YesNo, ParentTypes::PARENT_PAGE_STATE);
		}
		else if (pSender == (DWORD*)m_btnStr10)
		{
			Packet pkt(WIZ_POINT_CHANGE, uint8(1));
			pkt << uint8(1);
			Engine->Send(&pkt);
		}
		else if (pSender == (DWORD*)m_btnHp10)
		{
			Packet pkt(WIZ_POINT_CHANGE, uint8(2));
			pkt << uint8(2);
			Engine->Send(&pkt);
		}
		else if(pSender == (DWORD*)m_btnDex10)
		{
			Packet pkt(WIZ_POINT_CHANGE, uint8(3));
			pkt << uint8(3);
			Engine->Send(&pkt);
		}
		else if(pSender == (DWORD*)m_btnInt10)
		{
			Packet pkt(WIZ_POINT_CHANGE, uint8(4));
			pkt << uint8(4);
			Engine->Send(&pkt);
		}
		else if (pSender == (DWORD*)m_btnMp10)
		{
			Packet pkt(WIZ_POINT_CHANGE, uint8(5));
			pkt << uint8(5);
			Engine->Send(&pkt);
		}
		return false;
	}

	if (pSender && dwMsg == 0x00000001)
	{
		if (pSender == (DWORD*)btn_perks) {
			perksOpen();
			return true;
		}
		else if (pSender == (DWORD*)back_character_page) {
			resetPerks();
			return true;
		}
		else if (pSender == (DWORD*)btn_reset_perks) {

			uint32 perk = 0;
			for (int i = 0; i < PERK_COUNT; i++)
				perk += Engine->pPerks.perkType[i];

			if (perk == 0) {
				Engine->WriteInfoMessageExt((char*)string_format(xorstr("There are no Perk points to reset.")).c_str(), 0xf2224c);
				return true;
			}

			Engine->m_UiMgr->ShowMessageBox(xorstr("Perk Reset"), string_format("Do you want to reset your PERKS POINT for %s ?",
				Engine->StringHelper->NumberFormat(Engine->pPerks.perkCoins).c_str(), "Coins"), YesNo, PARENT_PERK_RESET);
			return true;
		}
		else if (pSender == (DWORD*)_basePerks.btn_right) {
			if (cPage >= pageCount)
				return true;

			cPage++;
			Engine->SetString(_basePerks.str_page, string_format(xorstr("%d/%d"), cPage, pageCount));
			reOrderPerk(cPage);
			return true;
		}
		else if (pSender == (DWORD*)_basePerks.btn_left) {
			if (cPage > 1) {
				cPage--;
				Engine->SetString(_basePerks.str_page, string_format(xorstr("%d/%d"), cPage, pageCount));
				reOrderPerk(cPage);
			}
			return true;
		}

		for (int i = 0; i < UI_PERKCOUNT; i++)
		{
			if (pSender == (DWORD*)_basePerks._basePerksBonus[i].btn_plus)
			{
				if (!Engine->pPerks.remPerk) {
					Engine->WriteInfoMessageExt((char*)string_format(xorstr("You do not have perk rights to use.")).c_str(), 0xf2224c);
					return true;
				}

				int32 index = _basePerks._basePerksBonus[i].index;
				if (index == -1)
					return true;

				auto itr = Engine->m_PerksArray.find(index);
				if (itr == Engine->m_PerksArray.end()) {
					Engine->WriteInfoMessageExt((char*)string_format(xorstr("No such perk was found. please try later.")).c_str(), 0xf2224c);
					return true;
				}

				if (!itr->second->status) {
					Engine->WriteInfoMessageExt((char*)string_format(xorstr("This perk is deprecated.")).c_str(), 0xf2224c);
					return true;
				}

				if (Engine->pPerks.perkType[index] >= itr->second->maxPerkCount) {
					Engine->WriteInfoMessageExt((char*)string_format(xorstr("You have reached the maximum number for this perk. Please see other perks.")).c_str(), 0xf2224c);
					return true;
				}

				if (perkUseTick > getMSTime())
				{
					uint32 time = perkUseTick - getMSTime();
					time /= 1000;
					if (time < 1)
						time = 1;
					Engine->WriteInfoMessageExt((char*)string_format(xorstr("Please wait %d second to used perk."), time).c_str(), D3DCOLOR_ARGB(255, 255, 111, 0));
					return true;
				}

				perkUseTick = getMSTime() + (500);
				Packet newpkt(XSafe, uint8(XSafeOpCodes::PERKS));
				newpkt << uint8(perksSub::perkPlus) << _basePerks._basePerksBonus[i].index;
				Engine->Send(&newpkt);
				return true;
			}
		}
	}

	return false;
}


DWORD Func_State = 0;
void __stdcall UiStateReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiState->ReceiveMessage(pSender, dwMsg);

	__asm
	{
		MOV ECX, uiStateVTable
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_State
		CALL EAX
	}
}

void CUIStatePlug::Tick()
{
	//if (!Engine->IsVisible(m_dVTableAddr))
	//	return;

	//if (amktick > GetTickCount())
	//	return;

	//if (Engine->GetState(m_btnStr10) == UI_STATE_BUTTON_DOWN)
	//{
	//	amktick = GetTickCount() + 100;
	//	UiStateReceiveMessage_Hook((DWORD*)m_btnStr10, UIMSG_BUTTON_CLICK);
	//}else if (Engine->GetState(m_btnHp10) == UI_STATE_BUTTON_DOWN)
	//{
	//	amktick = GetTickCount() + 100;
	//	UiStateReceiveMessage_Hook((DWORD*)m_btnHp10, UIMSG_BUTTON_CLICK);
	//}else if (Engine->GetState(m_btnDex10) == UI_STATE_BUTTON_DOWN)
	//{
	//	amktick = GetTickCount() + 100;
	//	UiStateReceiveMessage_Hook((DWORD*)m_btnDex10, UIMSG_BUTTON_CLICK);
	//}else if (Engine->GetState(m_btnMp10) == UI_STATE_BUTTON_DOWN)
	//{
	//	amktick = GetTickCount() + 100;
	//
	//	UiStateReceiveMessage_Hook((DWORD*)m_btnMp10, UIMSG_BUTTON_CLICK);
	//}else if (Engine->GetState(m_btnInt10) == UI_STATE_BUTTON_DOWN)
	//{
	//	amktick = GetTickCount() + 100;
	//	UiStateReceiveMessage_Hook((DWORD*)m_btnInt10, UIMSG_BUTTON_CLICK);
	//}
	
}
void CUIStatePlug::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_State = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)UiStateReceiveMessage_Hook;
}

void CUIStatePlug::resetPerks()
{
	Engine->SetVisible(m_btnReset, true);
	Engine->SetVisible(_basePerks.base, false);
}

void CUIStatePlug::reOrderPerk(uint16 page)
{
	Engine->SetVisible(_basePerks.base_bonus, false);
	for (int i = 0; i < UI_PERKCOUNT; i++)
	{
		_basePerks._basePerksBonus[i].index = -1;
		Engine->SetVisible(_basePerks._basePerksBonus[i].base, false);
		Engine->SetVisible(_basePerks._basePerksBonus[i].btn_plus, false);
		Engine->SetString(_basePerks._basePerksBonus[i].str_descp, "");
		Engine->SetString(_basePerks._basePerksBonus[i].str_perk, "");
		Engine->SetString(_basePerks._basePerksBonus[i].str_maxPerk, "");
	}

	std::vector< _PERKS> mPerks;
	foreach(itr, Engine->m_PerksArray)
		if (itr->second && itr->second->status)
			mPerks.push_back(*itr->second);

	if (mPerks.empty())
		return;

	std::sort(mPerks.begin(), mPerks.end(),
		[](auto const& a, auto const& b) { return a.pIndex < b.pIndex; });

	Engine->SetVisible(_basePerks.base_bonus, true);

	pageCount = abs(ceil((double)mPerks.size() / (double)UI_PERKCOUNT));
	if (pageCount < 1) pageCount = 1;
	Engine->SetString(_basePerks.str_page, string_format("%d/%d", cPage, pageCount));

	int p = (page - 1);
	if (p < 0) p = 0;
	size_t i = abs(p * UI_PERKCOUNT);

	uint8 c_slot = 0; uint16 count = 0;
	foreach(itr, mPerks)
	{
		count++;
		if (count <= i)
			continue;

		_basePerks._basePerksBonus[c_slot].index = itr->pIndex;
		Engine->SetVisible(_basePerks._basePerksBonus[c_slot].base, true);

		std::string descp = string_format("%s%d %s", itr->percentage ? "%" : "", itr->perkCount, itr->strDescp.c_str());
		Engine->SetString(_basePerks._basePerksBonus[c_slot].str_descp, descp);

		Engine->SetString(_basePerks._basePerksBonus[c_slot].str_perk, std::to_string(Engine->pPerks.perkType[itr->pIndex]));
		Engine->SetString(_basePerks._basePerksBonus[c_slot].str_maxPerk, string_format("max:%d", itr->maxPerkCount));

		Engine->SetVisible(_basePerks._basePerksBonus[c_slot].btn_plus, true);

		if (Engine->pPerks.perkType[itr->pIndex] >= itr->maxPerkCount)
			Engine->SetVisible(_basePerks._basePerksBonus[c_slot].btn_plus, false);
		else
			Engine->SetVisible(_basePerks._basePerksBonus[c_slot].btn_plus, Engine->pPerks.remPerk ? true : false);

		c_slot++;
		if (c_slot >= UI_PERKCOUNT)
			break;
	}
}

void CUIStatePlug::perksOpen()
{
	Engine->SetVisible(m_btnReset, false);
	cPage = pageCount = 1;
	Engine->SetString(str_remPerkCount, std::to_string(Engine->pPerks.remPerk));
	reOrderPerk();
	Engine->SetVisible(_basePerks.base, true);
}

void CUIStatePlug::resetPerkPoint(Packet& pkt)
{
	pkt >> Engine->pPerks.remPerk;
	memset(Engine->pPerks.perkType, 0, sizeof(Engine->pPerks.perkType));
	if (Engine->IsVisible(_basePerks.base)
		&& Engine->IsVisible(_basePerks.base_bonus)) {
		cPage = pageCount = 1;
		reOrderPerk();
	}
	Engine->SetString(str_remPerkCount, std::to_string(Engine->pPerks.remPerk));
	Engine->WriteInfoMessageExt((char*)string_format(xorstr("The perk points has been resed successfully. Congratulations.")).c_str(), 0x42d1eb);
}

void CUIStatePlug::perkUseItem(Packet& pkt)
{
	pkt >> Engine->pPerks.remPerk;
	setOptions();
	Engine->WriteInfoMessageExt((char*)string_format(xorstr("You have successfully activated the perk item. Congratulations.")).c_str(), 0x42d1eb);
}

void CUIStatePlug::setOptions()
{
	if (Engine->IsVisible(_basePerks.base) && Engine->IsVisible(_basePerks.base_bonus)) {
		for (int i = 0; i < UI_PERKCOUNT; i++)
		{
			int32 index = _basePerks._basePerksBonus[i].index;
			if (index == -1) continue;
			auto itr = Engine->m_PerksArray.find(index);
			if (itr == Engine->m_PerksArray.end())
				continue;

			if (Engine->pPerks.perkType[index] >= itr->second->maxPerkCount)
				Engine->SetVisible(_basePerks._basePerksBonus[i].btn_plus, false);
			else
				Engine->SetVisible(_basePerks._basePerksBonus[i].btn_plus, Engine->pPerks.remPerk ? true : false);

			Engine->SetString(_basePerks._basePerksBonus[i].str_perk, std::to_string(Engine->pPerks.perkType[index]));
		}
	}
	Engine->SetString(str_remPerkCount, std::to_string(Engine->pPerks.remPerk));
}

void CUIStatePlug::perkPlus(Packet& pkt)
{
	uint8 bResult = pkt.read<uint8>();
	if ((perksError)bResult == perksError::success)
	{
		uint32 index; uint16 perkCount, remPerkCount;
		pkt >> index >> perkCount >> remPerkCount;

		if (index >= PERK_COUNT)
			return;

		auto itr = Engine->m_PerksArray.find(index);
		if (itr == Engine->m_PerksArray.end())
			return;

		Engine->pPerks.perkType[index]++;
		Engine->pPerks.remPerk = remPerkCount;
		setOptions();
		Engine->WriteInfoMessageExt((char*)string_format(xorstr("The perk has been used successfully. Congratulations.")).c_str(), 0x42d1eb);
	}
	else
	{
		switch ((perksError)bResult)
		{
		case perksError::remPerks:
			Engine->WriteInfoMessageExt((char*)string_format(xorstr("You do not have perk rights to use.")).c_str(), 0x42d1eb);
			break;
		case perksError::notFound:
			Engine->WriteInfoMessageExt((char*)string_format(xorstr("No such perk was found. please try later..")).c_str(), 0x42d1eb);
			break;
		case perksError::maxPerkCount:
			Engine->WriteInfoMessageExt((char*)string_format(xorstr("You have reached the maximum number for this perk. Please see other perks.")).c_str(), 0x42d1eb);
			break;
		}
	}
}