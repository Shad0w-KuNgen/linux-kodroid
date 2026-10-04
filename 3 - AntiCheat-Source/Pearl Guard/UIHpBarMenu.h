#pragma once
#include "stdafx.h"
class CUIHPBarMenuPlug;
#include "Pearl Engine.h"

class CUIHPBarMenuPlug
{
public:
	CUIHPBarMenuPlug();
	~CUIHPBarMenuPlug();

	void ParseUIElements();
	void InitReceiveMessage();
	DWORD btn_daily_event;
	DWORD btn_daily_quest;
	DWORD btn_facebook;
	DWORD btn_discord;
	DWORD btn_wof;
	DWORD btn_searchdrop;
	DWORD btn_ticket;
	DWORD btn_rank;
	DWORD btn_scheduler;
	DWORD btn_pus;
	DWORD text_online_time;
	DWORD m_dVTableAddr;
	int64 MaxExp;

	std::string FacebookURL, DiscordURL;
	std::string LiveURL, ResellerURL;
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	bool OnKeyPress(int iKey);
	void Close();
	void Open();
	void UpdatePosition();
	void UpdateHpMenuVisible(Packet& pkt);
	void OpenHpBarMenu();
	void Tick();
	void FPS();
};