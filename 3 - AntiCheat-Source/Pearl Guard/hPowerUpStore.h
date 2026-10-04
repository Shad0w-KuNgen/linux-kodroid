#pragma once
class CUIPowerUpStore;
#include "stdafx.h"
#include "Pearl Engine.h"


class CUIPowerUpStore 
{
	int page;
	int pageCount;
	int cat;
	int itemscount;

	uint32 shopping_id, PusID;
	uint32 shopping_p;
	uint32 shopping_q;

	DWORD shopping_confirm;
	DWORD btn_confirm;
	DWORD btn_cancel;
	DWORD shopping_icon;
	DWORD shopping_name;
	DWORD shopping_price;
	DWORD shopping_quantity;
	DWORD shopping_after;
	DWORD btn_up;
	DWORD btn_down;
	DWORD item_count;
	DWORD gift_edit_name;

	DWORD basket_confirm;
	DWORD basket_cash;
	DWORD basket_ok;
	DWORD basket_no;

	DWORD btn_close;

	DWORD btn_reseller;
	DWORD btn_next;
	DWORD btn_previous;
	DWORD btn_useesn;
	DWORD btn_buyall;
	DWORD btn_clearall;
	DWORD btn_pusrefund;

	time_t m_LastTick;

	ULONGLONG esntime;

	DWORD txt_code;
	DWORD btn_esncancel;
	DWORD btn_esnconfirm;
	DWORD Use_Cash_Base;

	DWORD basepusrefund;
	DWORD btn_refundclose;
	DWORD baserefunditem[18];

	DWORD edit_search;
	DWORD txt_search;
	DWORD txt_tlbalance;
	DWORD txt_page;
	DWORD txt_cash;
	DWORD total_cash;
	vector<DWORD> btn_tabs;

	DWORD itemGroupBase[PUS_ITEMSCOUNT];

	struct UI_PUSITEM
	{
		uint32 sItemID;
		DWORD icon;
		DWORD name;
		DWORD price;
		DWORD quantitiy;
		DWORD purchase;
		DWORD add_to;
		DWORD nBase;
	};

	struct UI_PURCHASE_LIST
	{
		uint32 sItemID;
		uint32 sCash;
		uint8 type;
		DWORD icon;
		DWORD name;
		DWORD price;
		DWORD cancel;
		DWORD nBase;
	};

	struct UI_REFUND_LIST {
		uint32 sItemID;
		uint64 serial;
		DWORD icon;
		DWORD name;
		DWORD price;
		DWORD btn_ireturnadd;
		DWORD nBase;
	};

	vector<UI_PUSITEM> pus_items;
	vector<UI_PURCHASE_LIST> purchase_items;
	vector<PUSItem> item_list;
	vector<PusCategory> cat_list;
	vector<UI_REFUND_LIST> refund_items;
	vector<PUSREFUNDITEM> refund_list;


public:
	DWORD m_dVTableAddr;
	DWORD btn_search;
	CUIPowerUpStore();
	~CUIPowerUpStore();
	bool Load(HANDLE hFile);
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void InitReceiveMessage();
	void Close();
	void Open();
	void setBaseItems(int value);
	void OpenShopping();
	void ParseUIElements();
	void UpdateItemList(vector<PUSItem> items, vector<PusCategory> cats);
	void UpdateRefundItemList(vector<PUSREFUNDITEM> refund_items);
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	void SetItems(int p = 1, int cat = 1, string q = "");
	void RefundSetItems(bool dontopen = false);
	void UpdateCash(uint32 kc, uint32 tl);
	void OpenPowerUpStore();
	void PurchaseItemAdd(uint32 ItemID, uint32 Price, uint8 PriceType);
	void PurchaseTotal();
	void PurchaseDeleteAll();
	void PurchaseBuyAll();
	void BasketMenuOpen();
	void PurchaseButtonClick(DWORD pSender);
};