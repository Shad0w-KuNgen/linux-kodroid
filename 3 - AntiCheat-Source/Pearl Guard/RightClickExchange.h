#pragma once
class RightClickExchange;
#include "stdafx.h"
#include "Pearl Engine.h"
#include "Timer.h"
class UIObject;
class RightClickExchange
{
	bool posSet;
public:
	struct ItemInfo
	{
		uint32 nItemID;
		uint16 nRentalTime;
		DWORD icon;

		ItemInfo()
		{
			nItemID = 0;
			nRentalTime = 0;
			icon = NULL;

		}
	};
	struct ExchangeInformation
	{
		uint32 nItemID;
		uint16 sPercent;

	};
	vector<ExchangeInformation*>GetExchangeList;
	DWORD el_Base,  click_area, base_item,  base_item_area,  txt_title,  btn_exchange,  btn_close,  grp_selected,  base_itemicon;
	DWORD ItemBase[25];
	DWORD area_slot[25];
	bool SectionMode;
	bool ExchangeItemStatus;
	uint32 nBaseItemID;
	uint32 SelectItem;
	uint8 bCurrentPage;
	uint8 bPageCount;
	DWORD percent[25],  percent_base,  btn_down,  page,  btn_next;
	std::vector<ItemInfo*> m_Icon;
	RightClickExchange();
	~RightClickExchange();
	CTimer* m_Timer;
	void GetExchangePage(uint8 bPage);
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	void Close();
	void Open();
	void InitReceiveMessage();
	void ParseUIElements();
	void RightClickExchangePacket(Packet pkt);
	std::vector <uint32> RightClickItemList;
	private:
	#define PAGE_COUNT 25
};