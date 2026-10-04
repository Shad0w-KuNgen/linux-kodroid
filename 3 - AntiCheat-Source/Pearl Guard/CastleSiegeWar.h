#pragma once
class CUICastleSiegeWar;
#include "stdafx.h"
#include "Pearl Engine.h"
#include "Timer.h"
#include "N3BASE/N3UIBase.h"
#include "N3BASE/N3UIString.h"
#include "N3BASE/N3UIButton.h"
#include "N3BASE/N3UIEdit.h"

class CUICastleSiegeWar : public CN3UIBase
{
private:
	CN3UIString* txt_castle_owner, * txt_csw_remaining_time;
	CTimer* m_Timer;

	CN3UIBase* base_max, * base_time;
	CN3UIButton* btn_max;
	CN3UIString* txt_clan, * txt_time;
	CN3UIProgress* time;
	ULONGLONG timeLapsProgress;
	bool set;
	RECT SRect;
public:
	bool minimalize;
	uint32 m_iRemainingTime;

	CUICastleSiegeWar();
	~CUICastleSiegeWar();
	bool Load(HANDLE hFile);
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	void Close();
	void Open();
	void setOptions(uint32 remtime, std::string knights_name, uint8 OpStatus, uint32 war_time);
	void Tick();
};