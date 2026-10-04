#pragma once
class CUIMerchantListPlug;
#include "stdafx.h"
#include "Pearl Engine.h"
struct MerchantData
{
	uint8 type;
	int16 socketID;
	uint32 merchantID;
	string seller;
	uint32 nItemID, count, price;
	bool isKC;
	float x, y, z;
	TABLE_ITEM_BASIC* tbl;
};




struct MerchantUI
{
	DWORD row;
	DWORD txt_seller, txt_item, txt_grade, txt_type, txt_count, txt_price;
	DWORD btn_go, btn_pm;
};


class CUIMerchantListPlug
{
	

public:
	DWORD m_dVTableAddr;
	DWORD m_btngiftpus;//14.11.2020 User Sag Tik Pus
	DWORD m_btnViewInfo;
	int page, pageCount;

	MerchantUI mData[14];
	DWORD btn_close, btn_search, btn_sort_price, btn_previous, btn_next;
	DWORD edit_itemname;
	DWORD txt_itemname, txt_page;


	CUIMerchantListPlug();
	~CUIMerchantListPlug();
	void ParseUIElements();
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void InitReceiveMessage();
	void Close();
	void Open();
	void ClearList();
	void GO(string name);
	void PM(string name);
	void SetItem(uint8 _p, string q = "");
	vector<MerchantData> merchantList;
	void OpenMerchantList();
private:

};