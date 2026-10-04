#pragma once
class CUIJuraidMountain;
#include "stdafx.h"
#include "Pearl Engine.h"
#include "Timer.h"
#include "N3BASE/N3UIBase.h"
#include "N3BASE/N3UIString.h"
#include "N3BASE/N3UIButton.h"
#include "N3BASE/N3UIEdit.h"

class CUIJuraidMountain : public CN3UIBase
{
public:
	DWORD m_dVTableAddr;
	CN3UIBase* grp_minimum, * grp_maximum;
	CN3UIButton* btn_maximum, * icon, * btn_cancel, * btn_enter;
	CN3UIString* txt_elmo_0, * txt_karu_0, * txt_time;
	bool bMinimalize;
	ULONGLONG timeLapsTick;
	uint64 remainingtime;
public:
	CUIJuraidMountain();
	~CUIJuraidMountain();
	bool Load(HANDLE hFile);
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	void setOptions(bool update = false, uint16 remtime = 0, uint16 elmo = 0, uint16 karus = 0);
	void JoinOrDisband(bool join);
	void Close();
	void Open();
	void Tick();
};