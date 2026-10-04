#pragma once
class CUIInformationWindPlug;

class CUIInformationWindPlug
{
public:
	DWORD m_dVTableAddr;
	DWORD btn_merchant;
	DWORD btn_Notice;

	bool merchant_state;
	bool notice_state;

public:
	CUIInformationWindPlug();
	~CUIInformationWindPlug();
	void ParseUIElements();
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void InitReceiveMessage();
	void MerchantBtnSetState();
	void NoticeBtnSetState();
};