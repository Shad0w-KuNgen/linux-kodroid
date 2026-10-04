#pragma once
class CUIChatBarPlug;
#include "stdafx.h"
class CUIChatBarPlug
{
public:
	DWORD m_dVTableAddr;
	DWORD m_BlockButtons[7];
	DWORD m_btn[7];
	DWORD m_noah_knight_btn;
	DWORD m_loop;
	DWORD m_filter_base;
	DWORD m_Chating_Lock;
	DWORD m_Chating_Lock2;
	DWORD base_chat_on;
	DWORD base_main;
	DWORD mykoframe;
	DWORD base_on;
	DWORD base_filter;
	DWORD base_scroll;
	DWORD BaseChangeChattingMode;
	DWORD base_chat_top;
	DWORD base_chat;

public:
	CUIChatBarPlug();
	~CUIChatBarPlug();
	void ParseUIElements();
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void InitReceiveMessage();
	void SendButtonClick(DWORD btn);
	void ChangeChattingMode(DWORD btn, uint32_t dwMsg);
	void MessageNotice(uint8 type, uint8 status);
};