#pragma once
#include "stdafx.h"
class CUIQuestsPlug;
#include "Pearl Engine.h"

class CUIQuestsPlug
{
public:
	CUIQuestsPlug();
	~CUIQuestsPlug();


	void ParseUIElements();

	DWORD m_dVTableAddr;

	DWORD btn_quest[12], str_status[12], str_time[12];

	void UpdateList(uint8 page = 0);

	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void InitReceiveMessage();

private:

};