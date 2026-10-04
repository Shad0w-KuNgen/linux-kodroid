#pragma once
#include "stdafx.h"
class CUIQuestComplated;
#include "Pearl Engine.h"

class CUIQuestComplated
{
public:
	CUIQuestComplated();
	~CUIQuestComplated();

	void ParseUIElements();

	DWORD m_dVTableAddr;
	DWORD group[10];
	DWORD txt_msg[10];
	DWORD txt_title;
	DWORD group_top;
	DWORD btn_show;
	DWORD group_base;
	

	int64 MaxExp;
	void InitReceiveMessage();
	void SendQuestBoard(std::string QuestName, uint16 CurCount, uint16 TotalCount, uint16 MonsterID, uint8 nSize);
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
};