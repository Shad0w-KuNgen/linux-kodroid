#pragma once
class CUIPartyBBSPlug;
#include "stdafx.h"
#include "Pearl Engine.h"

class CUIPartyBBSPlug
{
public:
	DWORD m_dVTableAddr;
	DWORD btn_disband;//btn_party
	DWORD btn_voice;
	struct PartySlot
	{
		DWORD baseAddr;
		DWORD strName;
		DWORD strHP;
	};

	PartySlot slot[8];

public:
	CUIPartyBBSPlug();
	~CUIPartyBBSPlug();
	void ParseUIElements();
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void InitReceiveMessage();
	void OpenPartyDisband();
	bool PartyFind(uint16 GetID);
	void UpdatePartyUserHPBAR(Packet & pkt);
private:

};