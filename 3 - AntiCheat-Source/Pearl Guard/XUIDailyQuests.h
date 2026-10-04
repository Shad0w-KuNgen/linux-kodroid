#pragma once
class XdailyQuest;
#include "stdafx.h"
#include "Pearl Engine.h"
#include "N3BASE/N3UIBase.h"
#include "N3BASE/N3UIString.h"
#include "N3BASE/N3UIButton.h"
class XdailyQuest : public CN3UIBase
{
public:
	uint8 page;
	uint8 pagecountpageCount;
	int pageCount;
	CN3UIButton* m_btnOk;
	CN3UIButton *m_btnReview;
	CN3UIButton *m_btnCancel;
	CN3UIString* text_page;
	CN3UIButton* btn_up;
	CN3UIButton* btn_down;
	CN3UIButton* btn_close;
	uint16 selectQuestID;
	CN3UIString* m_txtUprate;
	CTimer* m_Timer;
	DWORD slots[28];
	CN3UIButton* baseQuestButton[12];
	CN3UIString* txt_quest_time[12];
	CN3UIString* txt_quest_status[12];
	CN3UIString* txt_quest_title[12];
	CN3UIString* txt_quest_zone[12];
	CN3UIString* txt_quest_killtype[12];
	CN3UIBase* baseGroup[12];
public:
	XdailyQuest();
	~XdailyQuest();
	vector<DailyQuest*> kcbq_quests;
	vector<DailyQuest*> filteredList;
	void InitQuests(uint8 p = 1);
	void KillTrigger(uint16 mob);
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	void Tick();
	bool IsInSlots();
	bool Load(HANDLE file);
	bool OnKeyPress(int iKey);
	void Close();
	void Open();
	void OpenDailyQuest();
private:

};