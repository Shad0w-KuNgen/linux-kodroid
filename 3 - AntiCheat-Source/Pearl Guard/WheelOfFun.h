#pragma once
class WheelOfFun;
#include "stdafx.h"
#include "Pearl Engine.h"
#include "N3BASE/N3UIBase.h"
#include "N3BASE/N3UIString.h"
#include "N3BASE/N3UIButton.h"
#include "N3BASE\N3UIArea.h"

class WheelOfFun : public CN3UIBase
{
	uint8 lastItem;
	CN3UIBase* base_item;
	CN3UIImage* nItem[25];

	CN3UIButton* btn_stop;
	CN3UIButton* btn_start;
	CN3UIButton* btn_close;


public:
	WheelOfFun();
	~WheelOfFun();
	bool Load(HANDLE hFile);
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	void StartChallange();
	void ImageChange(int8 sID);
	void StopChallange();
	void GiveItemImage(uint32 nItemID);
	bool isActiveWhile;
	CN3UIString* KnightCash;
	void Tick();
	long TickSay;
	void IsActive(bool enable);
	void OpenWheel();
	__IconItemSkill* GetItemStruct(CN3UIImage* img, std::string fileSz, __TABLE_ITEM_BASIC* tbl, uint32 realID);
};