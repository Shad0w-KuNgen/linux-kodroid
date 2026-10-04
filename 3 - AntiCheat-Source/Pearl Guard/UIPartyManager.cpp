#include "stdafx.h"
#include "UIPartyManager.h"
#include "Pearl Engine.h"

CUIPartyManager::CUIPartyManager()
{
	vector<int>offsets;
	offsets.push_back(0x1E8);
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);
	m_btn_settings = 0;
	ParseUIElements();
}

CUIPartyManager::~CUIPartyManager()
{
}

void CUIPartyManager::ParseUIElements()
{
	DWORD parent = 0;
	std::string find = xorstr("x");
	for (int i = 0; i < 8; i++)
	{
		find = string_format(xorstr("Base_Party%d"), i);
		Engine->GetChildByID(m_dVTableAddr, find, slot[i].baseAddr);
		find = string_format(xorstr("str_name%d"), i);
		Engine->GetChildByID(slot[i].baseAddr, find, slot[i].strName);
		find = string_format(xorstr("HP"), i);
		Engine->GetChildByID(slot[i].baseAddr, find, slot[i].strHP);
	}
}

void CUIPartyManager::UpdatePartyUserHPBAR(Packet& pkt)
{
	if (Engine->uiPartyPlug == NULL)
		return;

	int max_hp, hp;
	std::string username;
	pkt.DByte();
	pkt >> username >> max_hp >> hp;
	std::string hptext = xorstr("x");
	for (size_t i = 0; i < 8; i++)
	{
		std::string name = Engine->GetString(slot[i].strName);
		if (username == name)
		{
			hptext = string_format(xorstr("%d / %d"), hp, max_hp);
			Engine->SetString(Engine->uiPartyPlug->slot[i].strHP, hptext.c_str());
			return;
		}
	}
}