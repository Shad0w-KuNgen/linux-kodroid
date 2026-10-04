#pragma once
class CPowerUpStore;
#include "stdafx.h"
#include "Pearl Engine.h"
#include "N3BASE/N3UIBase.h"
#include "N3BASE/N3UIString.h"
#include "N3BASE/N3UIButton.h"
#include "N3BASE/N3UIImage.h"
#include "N3BASE/N3UIArea.h"

class CPowerUpStore : public CN3UIBase
{
	int page;
	int pageCount;
	int cat;
	int itemscount;

	uint32 shopping_id, PusID;
	uint32 shopping_p;
	uint32 shopping_q;

	CN3UIBase* shopping_confirm;
	CN3UIButton* btn_confirm;
	CN3UIButton* btn_cancel;
	CN3UIImage* shopping_icon;
	CN3UIString* shopping_name;
	CN3UIString* shopping_price;
	CN3UIString* shopping_quantity;
	CN3UIString* shopping_after;
	CN3UIButton* btn_up;
	CN3UIButton* btn_down;
	CN3UIString* item_count;
	CN3UIEdit* gift_edit_name;

	CN3UIBase* basket_confirm;
	CN3UIString* basket_cash;
	CN3UIButton* basket_ok;
	CN3UIButton* basket_no;

	CN3UIButton* btn_close;
	CN3UIButton* btn_search;
	CN3UIButton* btn_reseller;
	CN3UIButton* btn_next;
	CN3UIButton* btn_previous;
	CN3UIButton* btn_useesn;
	CN3UIButton* btn_buyall;
	CN3UIButton* btn_clearall;
	CN3UIButton* btn_pusrefund;

	time_t m_LastTick;

	CN3UIEdit* txt_code;
	CN3UIButton* btn_esncancel;
	CN3UIButton* btn_esnconfirm;
	CN3UIBase * Use_Cash_Base;

	CN3UIBase * basepusrefund;
	CN3UIButton* btn_refundclose;
	CN3UIBase* baserefunditem[18];

	CN3UIEdit* edit_search;
	CN3UIString* txt_search;
	CN3UIString* txt_tlbalance;
	CN3UIString* txt_page;
	CN3UIString* txt_cash;
	CN3UIString* total_cash;
	vector<CN3UIButton*> btn_tabs;

	CN3UIBase* itemGroupBase[PUS_ITEMSCOUNT];

	struct UI_PUSITEM
	{
		uint32 sItemID;
		CN3UIImage* icon;
		CN3UIString* name;
		CN3UIString* price;
		CN3UIString* quantitiy;
		CN3UIButton* purchase;
		CN3UIButton* add_to;
	};

	struct UI_PURCHASE_LIST
	{
		uint32 sItemID;
		uint32 sCash;
		uint8 type;
		CN3UIImage* icon;
		CN3UIString* name;
		CN3UIString* price;
		CN3UIButton* cancel;
	};

	struct UI_REFUND_LIST {
		uint32 sItemID;
		uint64 serial;
		CN3UIImage* icon;
		CN3UIString* name;
		CN3UIString* price;
		CN3UIButton* btn_ireturnadd;
	};

	vector<UI_PUSITEM> pus_items;
	vector<UI_PURCHASE_LIST> purchase_items;
	vector<PUSItem> item_list;
	vector<PusCategory> cat_list;
	vector<UI_REFUND_LIST> refund_items;
	vector<PUSREFUNDITEM> refund_list;

	
public:
	CPowerUpStore();
	~CPowerUpStore();
	bool Load(HANDLE hFile);
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	bool OnKeyPress(int iKey);
	void Close();
	void Open();
	void setBaseItems(int value);
	void OpenShopping();
	void UpdateItemList(vector<PUSItem> items, vector<PusCategory> cats);
	void UpdateRefundItemList(vector<PUSREFUNDITEM> refund_items);
	void SetItems(int p = 1, int cat = 0, string q = "");
	void RefundSetItems();
	void UpdateCash(uint32 kc,uint32 tl);
	void OpenPowerUpStore();
	void PurchaseItemAdd(uint32 ItemID, uint32 Price,uint8 PriceType);
	void PurchaseTotal();
	void PurchaseDeleteAll();
	void PurchaseBuyAll();
	void BasketMenuOpen();
	void PurchaseButtonClick(CN3UIBase* pSender);
	__IconItemSkill* GetItemStruct(CN3UIImage* img, std::string fileSz, __TABLE_ITEM_BASIC* tbl, uint32 realID);
};