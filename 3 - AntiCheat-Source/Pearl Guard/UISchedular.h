#pragma once
#include "stdafx.h"
class CUISchedular;
#include "Pearl Engine.h"

class CUISchedular
{
public:
	CUISchedular();
	~CUISchedular();

	bool BasladimiBaslamadimi;

	DWORD m_dVTableAddr;

	void ParseUIElements();
	void InitReceiveMessage();
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void DropResultStatus(bool status);

	DWORD m_btnsituation;
	DWORD m_btndropresult;
	DWORD m_btnbug;
	DWORD btn_lottery;
};