#pragma once
#include "stdafx.h"
class CUISupportReportPlug;
#include "Pearl Engine.h"

class CUISupportReportPlug
{
public:
	CUISupportReportPlug();
	~CUISupportReportPlug();

	void ParseUIElements();
	void InitReceiveMessage();
	bool bug;
	DWORD txt_subject;
	DWORD txt_message;
	DWORD btn_bug;
	DWORD btn_koxp;
	DWORD btn_report;
	DWORD btn_close;

	DWORD m_dVTableAddr;

	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	void Close();
	void Open();
	void OpenSupport();
};