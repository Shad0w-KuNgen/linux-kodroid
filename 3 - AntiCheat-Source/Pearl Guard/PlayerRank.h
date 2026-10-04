#pragma once
class HookPlayerRank;
#include "stdafx.h"
#include "Pearl Engine.h"

class HookPlayerRank
{
public:
	DWORD el_Base;
	DWORD m_btnOk;
	DWORD m_btnReview;
	DWORD m_btnCancel;
	DWORD nHumanBaseTop;
	DWORD nKarusBaseTop;

	DWORD nKarusBase[10];
	DWORD nHumanBase[10];
	DWORD nHumanBaseEffeckt[10];
	DWORD nKarusBaseEffeckt[10];


public:
	HookPlayerRank();
	~HookPlayerRank();
	void ParseUIElements();
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void InitReceiveMessage();
	bool ResetBase();
private:

};