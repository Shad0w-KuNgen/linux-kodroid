#pragma once

class CCastleSiegeWarTimer;

#include "stdafx.h"
#include "Pearl Engine.h"
#include "Timer.h"
#include "N3BASE/N3UIBase.h"
#include "N3BASE/N3UIButton.h"

class CCastleSiegeWarTimer : public CN3UIBase
{
	bool posSet;
	int32 m_iStartTime;
	int16 m_iRemainingTime;

	CN3UIBase* base_siegewarfare;
	CN3UIButton* btn_close;
	CN3UIString* m_txtStartingTime;
	CN3UIString* m_txtRemainingTime;
	CN3UIString* m_txtSiegeOwnerClanName;
	CN3UIImage* m_InfoSelect;
	CN3UIImage* m_byStand;
	CN3UIImage* m_byStarting;
	CTimer * m_Timer;

public:
	CCastleSiegeWarTimer();
	~CCastleSiegeWarTimer();
	bool Load(HANDLE hFile);
	void Tick();
	bool ReceiveMessage(CN3UIBase * pSender, uint32_t dwMsg);
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	bool OnKeyPress(int iKey);
	void Open();
	void Close();
	void UpdatePosition();
	void Start();
	void Update();
	void OpenCastleSiegeWarTimer();
};