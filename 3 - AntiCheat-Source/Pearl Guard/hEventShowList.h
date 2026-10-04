#pragma once
class CEventShowListPlug;
#include "stdafx.h"
#include "Pearl Engine.h"
#include "Timer.h"
class CEventShowListPlug
{
public:
	CTimer* m_Timer;

	uint8 page;
	int pageCount;
	DWORD el_base;
	DWORD m_dVTableAddr;

	DWORD btn_details;
	DWORD base_news;
	DWORD btn_news_close;
	DWORD text_page;
	DWORD text_server_time;
	DWORD btn_up;
	DWORD btn_down;
	DWORD btn_close;
	DWORD daily_event;
	DWORD baseGroup[10];
	DWORD txt_event_time[10];
	DWORD txt_event_name[10];


	CEventShowListPlug();
	~CEventShowListPlug();

	std::vector<EventShow> mshowlist;

	void ParseUIElements();
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void InitReceiveMessage();
	bool OnKeyPress(int iKey);
	void Close();
	void Open();
	void Update(uint32 reqs[5], uint32 reqsCount[5], uint32 rewards[4], bool reset = false);
	void Tick();
	void InitList(uint8 page = 1);
	void OpenScheduler();

private:

};