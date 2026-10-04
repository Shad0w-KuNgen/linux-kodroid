#pragma once
class CDropResult;
#include "stdafx.h"
#include "Pearl Engine.h"

class CDropResult
{
	struct DropItems
	{
		uint32 nItemID;
		DWORD icon;
		TABLE_ITEM_BASIC* tbl;
		std::string fileName;
		DropItems()
		{
			fileName = "";
		}
	};

	DWORD txt_title;
	DWORD btn_close;
	DropItems dropitems[5];


public:
	DWORD m_dVTableAddr;
	CDropResult();
	~CDropResult();
	void ParseUIElements();
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void InitReceiveMessage();
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);

	void Close();
	void Open();
	void SetTitle(std::string text);
	void AddItem(uint32 itemID);
	void OpenDropResult();
};