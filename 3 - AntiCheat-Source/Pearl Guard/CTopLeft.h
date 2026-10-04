#pragma once
class CTopLeft;
#include "stdafx.h"
#include "Pearl Engine.h"
#include "N3BASE/N3UIBase.h"
#include "N3BASE/N3UIString.h"
#include "N3BASE/N3UIButton.h"

class CTopLeft : public CN3UIBase
{
public:

	time_t timeLapse;
	bool posSet;
	CN3UIButton* btn_facebook;
	CN3UIButton* btn_discord;
	CN3UIButton* btn_wof;
	CN3UIButton* btn_searchdrop;
	CN3UIButton* btn_ticket;
	CN3UIButton* btn_scheduler;
	CN3UIButton* btn_pus;
	CN3UIString* text_online_time;

	std::string FacebookURL, DiscordURL;
public:
	CTopLeft();
	~CTopLeft();
	bool Load(HANDLE hFile);
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	bool OnKeyPress(int iKey);
	void Close();
	void Open();
	void UpdatePosition();
	void UpdateTopLeftVisible(Packet& pkt);
	void OpenTopLeft();
	void Tick();
	void FPS();
};