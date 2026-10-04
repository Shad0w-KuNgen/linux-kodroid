#pragma once
class CEventShowList;
#include "stdafx.h"
#include "Pearl Engine.h"
#include "N3BASE/N3UIBase.h"
#include "N3BASE/N3UIString.h"
#include "N3BASE/N3UIButton.h"
#include "N3BASE\N3UIArea.h"

class CEventShowList : public CN3UIBase
{
public:
	CTimer* m_Timer;

	uint8 page;
	int pageCount;

	CN3UIString* text_page;
	CN3UIString* text_server_time;
	CN3UIButton* btn_up;
	CN3UIButton* btn_down;
	CN3UIButton* btn_close;

	CN3UIBase* baseGroup[10];
	CN3UIString* txt_event_time[10];
	CN3UIString* txt_event_name[10];

	CEventShowList();
	~CEventShowList();

	std::vector<EventShow> mshowlist;

	bool Load(HANDLE hFile);
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	bool OnKeyPress(int iKey);
	void Close();
	void Open();
	void Update(uint32 reqs[5], uint32 reqsCount[5], uint32 rewards[4], bool reset = false);
	void Tick();
	void InitList(uint8 page = 1);
	void OpenScheduler();
};