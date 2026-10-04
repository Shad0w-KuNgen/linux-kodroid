#pragma once
class CUIMoticeMsgWnd;
#include "stdafx.h"
#include "Pearl Engine.h"

struct NoticeRecent
{
	uint16 sIndex;
	std::string Notice;
	std::string sSender;

	NoticeRecent()
	{
		sSender = "";
	}
};
class CUIMoticeMsgWnd
{

	DWORD txt_title;
	DWORD m_Btn_Close;
	std::map<uint16, NoticeRecent> RecentNotice;
	std::map<uint16, NoticeRecent> MerchantChat;

public:
	DWORD m_ptitle;

	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void AddMsg(const std::string& szString, D3DCOLOR color = 0xffffffff);
	DWORD m_pChatOut;
	void Open(bool isMerchant=false);
	void Close();
	void MoticeMsgProcess(Packet& pkt,uint8 sType);
	void Clear();
	DWORD el_base;
	DWORD m_dVTableAddr;
	CUIMoticeMsgWnd();
	~CUIMoticeMsgWnd();
	void ParseUIElements();
	void InitReceiveMessage();
};