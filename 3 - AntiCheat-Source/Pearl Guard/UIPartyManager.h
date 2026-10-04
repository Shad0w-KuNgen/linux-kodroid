#pragma once
class CUIPartyManager;

class CUIPartyManager
{
public:
	DWORD m_dVTableAddr;
	DWORD m_btn_settings;
	struct PartySlot
	{
		DWORD baseAddr;
		DWORD strName;
		DWORD strHP;
	};

	PartySlot slot[8];

public:
	CUIPartyManager();
	~CUIPartyManager();
	void ParseUIElements();
	void UpdatePartyUserHPBAR(Packet & pkt);
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void InitReceiveMessage();
};