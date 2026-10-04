#include "MerchantList.h"

bool MerchantListCompare(const MerchantData & a, const MerchantData & b);

CMerchantList::CMerchantList()
{

}

CMerchantList::~CMerchantList()
{

}

bool CMerchantList::Load(HANDLE hFile)
{
	if (CN3UIBase::Load(hFile) == false) 
		return false;

	string find = xorstr("btn_close");
	btn_close = (CN3UIButton*)GetChildByID(find);
	find = xorstr("btn_search");
	btn_search = (CN3UIButton*)GetChildByID(find);
	find = xorstr("btn_sort_price");
	btn_sort_price = (CN3UIButton*)GetChildByID(find);
	find = xorstr("edit_itemname");
	edit_itemname = (CN3UIEdit*)GetChildByID(find);
	
	find = xorstr("txt_itemname");
	txt_itemname = (CN3UIString*) edit_itemname->GetChildByID(find);
	find = xorstr("btn_previous");
	btn_previous = (CN3UIButton*)GetChildByID(find);
	find = xorstr("btn_next");
	btn_next = (CN3UIButton*)GetChildByID(find);
	find = xorstr("txt_page");
	txt_page = (CN3UIString*)GetChildByID(find);

	for (int i = 0; i < 14; i++)
	{
		find = string_format(xorstr("row_%d"), i+1);
		mData[i].row = (CN3UIBase*)GetChildByID(find);

		// Row Parts
		find = xorstr("txt_seller");
		mData[i].txt_seller = (CN3UIString*)mData[i].row->GetChildByID(find);
		find = xorstr("txt_item");
		mData[i].txt_item = (CN3UIString*)mData[i].row->GetChildByID(find);
		find = xorstr("txt_grade");
		mData[i].txt_grade = (CN3UIString*)mData[i].row->GetChildByID(find);
		find = xorstr("txt_type");
		mData[i].txt_type = (CN3UIString*)mData[i].row->GetChildByID(find);
		find = xorstr("txt_count");
		mData[i].txt_count = (CN3UIString*)mData[i].row->GetChildByID(find);
		find = xorstr("txt_price");
		mData[i].txt_price = (CN3UIString*)mData[i].row->GetChildByID(find);
		find = xorstr("btn_go");
		mData[i].btn_go = (CN3UIButton*)mData[i].row->GetChildByID(find);
		find = xorstr("btn_pm");
		mData[i].btn_pm = (CN3UIButton*)mData[i].row->GetChildByID(find);
	}

	page = 1;
	
	txt_page->SetString(to_string(page));

	ClearList();

	SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, Engine->m_UiMgr->GetScreenCenter(this).y);

	return true;
}

void CMerchantList::ClearList()
{
	for (int i = 0; i < 14; i++)
	{
		mData[i].txt_seller->SetString("");
		mData[i].txt_item->SetString("");
		mData[i].txt_grade->SetString("");
		mData[i].txt_type->SetString("");
		mData[i].txt_count->SetString("");
		mData[i].txt_price->SetString("");
		mData[i].btn_go->SetVisible(false);
		mData[i].btn_pm->SetVisible(false);
	}
}

void CMerchantList::SetItem(uint8 _p, string q)
{
	page = _p;
	pageCount = abs(ceil((double)merchantList.size() / (double)14));

	ClearList();

	for (uint32 i = 0; i < 14; i++)
	{
		if (14 * (page - 1) + i > merchantList.size() - 1 || merchantList.size() < i + 1)
			break;

		int j = i + 14 * (page - 1);
		TABLE_ITEM_BASIC * tbl = Engine->tblMgr->getItemData(merchantList[j].nItemID);

		if (tbl != nullptr)
		{
			if (q != "" && Engine->StringHelper->IsContains(tbl->strName, q))
			{
				merchantList[j].tbl = tbl;
				mData[i].txt_seller->SetString(merchantList[j].seller);
				mData[i].txt_item->SetString(tbl->strName);

				// Grade algorithm
				TABLE_ITEM_EXT * ext = Engine->tblMgr->getExtData(tbl->extNum, merchantList[j].nItemID);
				if (ext != nullptr)
				{
					if(ext->baseItemID > 0)
						mData[i].txt_item->SetString(ext->szHeader);

					if (ext->extensionID % 10 != 0)
					{
						char szExtID[20];
						sprintf(szExtID, "+%d", ext->extensionID % 10);
						mData[i].txt_grade->SetString(szExtID);
					}
					else
						mData[i].txt_grade->SetString("-");
				}

				mData[i].txt_type->SetString(merchantList[j].type == 0 ? "Sell" : "Buy");
				mData[i].txt_count->SetString(to_string(merchantList[j].count));

				if (merchantList[j].isKC)
					mData[i].txt_price->SetString(string_format(xorstr("%s knight cash"), Engine->StringHelper->NumberFormat(merchantList[j].price).c_str()));
				else
					mData[i].txt_price->SetString(string_format(xorstr("%s coins"), Engine->StringHelper->NumberFormat(merchantList[j].price).c_str()));

				mData[i].btn_go->SetVisible(true);
				mData[i].btn_pm->SetVisible(true);
			}
			else if (q == "")
			{
				mData[i].txt_seller->SetString(merchantList[j].seller);
				mData[i].txt_item->SetString(tbl->strName);

				// Grade algorithm
				TABLE_ITEM_EXT * ext = Engine->tblMgr->getExtData(tbl->extNum, merchantList[j].nItemID);
				if (ext != nullptr)
				{
					if (ext->baseItemID > 0)
						mData[i].txt_item->SetString(ext->szHeader);

					if (ext->extensionID % 10 != 0)
					{
						char szExtID[20];
						sprintf(szExtID, "+%d", ext->extensionID % 10);
						mData[i].txt_grade->SetString(szExtID);
					}
					else
						mData[i].txt_grade->SetString("-");
				}

				mData[i].txt_type->SetString(merchantList[j].type == 0 ? "Sell" : "Buy");
				mData[i].txt_count->SetString(to_string(merchantList[j].count));

				if (merchantList[j].isKC)
					mData[i].txt_price->SetString(string_format(xorstr("%s knight cash"), Engine->StringHelper->NumberFormat(merchantList[j].price).c_str()));
				else
					mData[i].txt_price->SetString(string_format(xorstr("%s coins"), Engine->StringHelper->NumberFormat(merchantList[j].price).c_str()));

				mData[i].btn_go->SetVisible(true);
				mData[i].btn_pm->SetVisible(true);
			}
		}
	}

	txt_page->SetString(to_string(page));
}

bool CMerchantList::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == btn_close)
			Close();
		else if (pSender == btn_search)
			SetItem(1, txt_itemname->GetString());
		else if (pSender == btn_sort_price)
		{
			std::sort(merchantList.begin(), merchantList.end(), MerchantListCompare);
			SetItem(1);
		}

		for (int i = 0; i < 14; i++)
		{
			if (pSender == mData[i].btn_go)
				GO(merchantList[i*page].seller);
			else if (pSender == mData[i].btn_pm)
				PM(merchantList[i*page].seller);
		}
	}

	return true;
}

void CMerchantList::GO(string name) 
{
	Packet result(XSafe);
	result << uint8_t(XSafeOpCodes::MERCHANTLIST) << uint8(1) << name;
	Engine->Send(&result);
}

void CMerchantList::PM(string name) 
{
	Packet result(XSafe);
	result << uint8_t(XSafeOpCodes::MERCHANTLIST) << uint8(2) << name;
	Engine->Send(&result);
}

uint32_t CMerchantList::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
	return dwRet;
}

bool CMerchantList::OnKeyPress(int iKey)
{
	if (!IsVisible())
		return CN3UIBase::OnKeyPress(iKey);

	return true;
}

void CMerchantList::Open()
{
	SetVisible(true);
}

void CMerchantList::Close()
{
	SetVisible(false);
}

bool MerchantListCompare(const MerchantData & a, const MerchantData & b)
{
	return a.price < b.price;
}

void CMerchantList::OpenMerchantList()
{
	std::string name = Engine->dcpUIF(xorstr("Msoft\\Merchant.Msoft"));
	Engine->m_UiMgr->uiMerchantList = new CMerchantList();
	Engine->m_UiMgr->uiMerchantList->Init(this);
	Engine->m_UiMgr->uiMerchantList->LoadFromFile(Engine->m_BasePath + name.c_str(), N3FORMAT_VER_1068);
	Engine->m_UiMgr->AddChild(Engine->m_UiMgr->uiMerchantList);
	remove(name.c_str());
}