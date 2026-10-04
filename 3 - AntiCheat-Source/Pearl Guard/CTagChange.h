#pragma once
class CTagChange;
#include "stdafx.h"
#include "Pearl Engine.h"

class CTagChange
{
public:
	bool bug;
	DWORD  edit_id;
	DWORD btn_cancel;
	DWORD btn_ok;
	DWORD m_dVTableAddr;
	DWORD txt_id;
	DWORD text_explanation;
	DWORD plate;
	DWORD text_title;

public:
	uint8 r, g, b;
	CTagChange();
	~CTagChange();
	void InitReceiveMessage();
	void ParseUIElements();
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	bool OnKeyPress(int iKey);
	void Close();
	void Open();
	void OpenTagChange();
};