#include "stdafx.h"
#include "PlayerRank.h"
HookPlayerRank::HookPlayerRank()
{


	vector<int>offsets;
	offsets.push_back(0x3F0);
	offsets.push_back(0);

	el_Base = Engine->rdword(KO_DLG, offsets);

	m_btnReview = 0;
	m_btnCancel = 0;
	nHumanBaseTop = 0;
	nKarusBaseTop = 0;
	for (int i = 0; i < 10; i++) {
		nHumanBase[i] = 0;
		nKarusBase[i] = 0;
		nHumanBaseEffeckt[i] = 0;
		nKarusBaseEffeckt[i] = 0;
	}




	ParseUIElements();

}
HookPlayerRank::~HookPlayerRank()
{
}

void HookPlayerRank::ParseUIElements()
{


	std::string find = xorstr("el_symbol");
	Engine->GetChildByID(el_Base, find, nHumanBaseTop);

	for (int i = 0; i < 10; i++) {
		find = string_format(xorstr("rank%d"), i);
		Engine->GetChildByID(nHumanBaseTop, find, nHumanBase[i]);
		Engine->SetVisible(nHumanBase[i], false);

	}
	find = xorstr("ka_symbol");
	Engine->GetChildByID(el_Base, find, nKarusBaseTop);
	for (int i = 0; i < 10; i++) {
		find = string_format(xorstr("rank%d"), i);
		Engine->GetChildByID(nKarusBaseTop, find, nKarusBase[i]);
		Engine->SetVisible(nKarusBase[i], false);

	}


}

bool HookPlayerRank::ResetBase()
{


	for (int i = 0; i < 10; i++) {
		Engine->SetVisible(nKarusBase[i], false);
		Engine->SetVisible(nHumanBase[i], false);
		Engine->SetVisible(nHumanBaseEffeckt[i], false);
		Engine->SetVisible(nKarusBaseEffeckt[i], false);

	}

	return false;
}

