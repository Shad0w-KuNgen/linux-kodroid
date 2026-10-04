#include "PowerUpStore.h"
#include "HDRReader.h"

bool pus_compare(const PUSItem & a, const PUSItem & b);

CPowerUpStore::CPowerUpStore()
{
	shopping_confirm = NULL;
	btn_close = NULL;
	btn_search = NULL;
	btn_reseller = NULL;
	btn_next = NULL;
	btn_previous = NULL;
	btn_useesn = NULL;
	txt_search = NULL;
	txt_page = NULL;

	txt_cash = NULL;
	txt_tlbalance = NULL;
	page = 1;
	pageCount = 1;
	cat = 0;
	itemscount = 0;
	m_LastTick = GetTickCount();
}

CPowerUpStore::~CPowerUpStore()
{

}

bool CPowerUpStore::Load(HANDLE hFile)
{
	if (CN3UIBase::Load(hFile) == false)
		return false;

	// Base
	std::string find = xorstr("shopping_confirm");
	shopping_confirm = (CN3UIBase*)GetChildByID(find);
	find = xorstr("btn_confirm");
	btn_confirm = (CN3UIButton*)shopping_confirm->GetChildByID(find);
	find = xorstr("btn_cancel");
	btn_cancel = (CN3UIButton*)shopping_confirm->GetChildByID(find);
	find = xorstr("shopping_icon");
	shopping_icon = (CN3UIImage*)shopping_confirm->GetChildByID(find);
	find = xorstr("shopping_name");
	shopping_name = (CN3UIString*)shopping_confirm->GetChildByID(find);
	find = xorstr("shopping_price");
	shopping_price = (CN3UIString*)shopping_confirm->GetChildByID(find);
	find = xorstr("shopping_quantity");
	shopping_quantity = (CN3UIString*)shopping_confirm->GetChildByID(find);
	find = xorstr("shopping_after");
	shopping_after = (CN3UIString*)shopping_confirm->GetChildByID(find);
	find = xorstr("btn_up");
	btn_up = (CN3UIButton*)shopping_confirm->GetChildByID(find);
	find = xorstr("btn_down");
	btn_down = (CN3UIButton*)shopping_confirm->GetChildByID(find);
	find = xorstr("str_count");
	item_count = (CN3UIString*)shopping_confirm->GetChildByID(find);
	find = xorstr("txt_gift_name");
	gift_edit_name = (CN3UIEdit*)shopping_confirm->GetChildByID(find);
	shopping_confirm->SetVisible(false);

	//basket menu
	find = xorstr("Basket_confirm");
	basket_confirm = (CN3UIBase*)GetChildByID(find);
	find = xorstr("text_basket_total_price");
	basket_cash = (CN3UIString*)basket_confirm->GetChildByID(find);
	find = xorstr("btn_all_confirm");
	basket_ok = (CN3UIButton*)basket_confirm->GetChildByID(find);
	find = xorstr("btn_cancel");
	basket_no = (CN3UIButton*)basket_confirm->GetChildByID(find);
	basket_confirm->SetVisible(false);

	// edit
	
	find = xorstr("esncode");
	txt_code = (CN3UIEdit*)GetChildByID(find);
	// Buttons
	find = xorstr("btn_close");
	btn_close = (CN3UIButton*)GetChildByID(find);
	find = xorstr("btn_search");
	btn_search = (CN3UIButton*)GetChildByID(find);
	find = xorstr("btn_reseller");
	btn_reseller = (CN3UIButton*)GetChildByID(find);
	find = xorstr("btn_next");
	btn_next = (CN3UIButton*)GetChildByID(find);
	find = xorstr("btn_previous");
	btn_previous = (CN3UIButton*)GetChildByID(find);
	find = xorstr("btn_useesn");
	btn_useesn = (CN3UIButton*)GetChildByID(find);
	find = xorstr("btn_refund");
	btn_pusrefund = (CN3UIButton*)GetChildByID(find);

	for (int i = 1; i <= 5; i++)
	{
		find = string_format(xorstr("btn_tab%d"), i);
		CN3UIButton* tmp = (CN3UIButton*)GetChildByID(find);
		btn_tabs.push_back(tmp);
	}

	btn_tabs[0]->SetState(UI_STATE_BUTTON_DISABLE);

	// Texts
	find = xorstr("edit_search");
	edit_search = (CN3UIEdit*)GetChildByID(find);
	find = xorstr("txt_search");
	txt_search = (CN3UIString*)edit_search->GetChildByID(find);
	find = xorstr("txt_page");
	txt_page = (CN3UIString*)GetChildByID(find);
	find = xorstr("txt_cash");
	txt_cash = (CN3UIString*)GetChildByID(find);
	find = xorstr("txt_tl_balance");
	txt_tlbalance = (CN3UIString*)GetChildByID(find);
	find = xorstr("text_basket_total_price");
	total_cash = (CN3UIString*)GetChildByID(find);
	find = xorstr("btn_buyall");
	btn_buyall = (CN3UIButton*)GetChildByID(find);
	find = xorstr("btn_clearall");
	btn_clearall = (CN3UIButton*)GetChildByID(find);

	////esn code
	//find = xorstr("Use_Cash_Base");
	//Use_Cash_Base = (CN3UIBase*)GetChildByID(find);
	//for (int i = 0; i < 5; i++) {
	//	find = string_format(xorstr("edit_0%d"), i);
	//	txt_esnedit[i] = (CN3UIEdit*)Use_Cash_Base->GetChildByID(find);
	//	txt_esnedit[i]->SetMaxString(4);
	//}
	//find = xorstr("btn_add_kc");
	//btn_esnconfirm = (CN3UIButton*)Use_Cash_Base->GetChildByID(find);
	//find = xorstr("btn_cancel_kc");
	//btn_esncancel = (CN3UIButton*)Use_Cash_Base->GetChildByID(find);
	//Use_Cash_Base->SetVisible(false);

	// Pus Refund
	find = xorstr("base_refund");
	basepusrefund = (CN3UIBase*)GetChildByID(find);
	for (int i = 0; i < 18; i++) {
		find = string_format(xorstr("refund_items_%d"), i);
		baserefunditem[i] = (CN3UIBase*)basepusrefund->GetChildByID(find);

		CN3UIBase* itemGroup = baserefunditem[i];
		UI_REFUND_LIST refund_list;
		find = string_format(xorstr("refund_item%d"), i);
		refund_list.name = (CN3UIString*)itemGroup->GetChildByID(find);
		find = string_format(xorstr("refund_price%d"), i);
		refund_list.price = (CN3UIString*)itemGroup->GetChildByID(find);
		find = xorstr("item_icon");
		refund_list.icon = (CN3UIImage*)itemGroup->GetChildByID(find);
		find = xorstr("btn_ireturn");
		refund_list.btn_ireturnadd = (CN3UIButton*)itemGroup->GetChildByID(find);

		refund_list.name->SetString(xorstr("-"));
		refund_list.price->SetString(xorstr("-"));
		refund_list.icon->GetParent()->SetVisible(false);
		refund_items.push_back(refund_list);
	}
	find = xorstr("btn_close");
	btn_refundclose = (CN3UIButton*)basepusrefund->GetChildByID(find);
	basepusrefund->SetVisible(false);
	
	txt_code->SetMaxString(26);
	txt_code->SetString("");
	// Pus Items
	for (int i = 1; i <= PUS_ITEMSCOUNT; i++)
	{
		find = string_format(xorstr("items%d"), i);
		//CN3UIBase* itemGroup = (CN3UIBase*)GetChildByID(find);
		itemGroupBase[i - 1] = (CN3UIBase*)GetChildByID(find);
		CN3UIBase* itemGroup = itemGroupBase[i - 1];

		for (int j = 1; j <= PUS_ITEMSCOUNT; j++)
		{
			find = string_format(xorstr("item%d"), j);
			CN3UIImage* item = (CN3UIImage*)itemGroup->GetChildByID(find);

			UI_PUSITEM pus_item;
			find = xorstr("item_icon");
			pus_item.icon = (CN3UIImage*)item->GetChildByID(find);
			find = xorstr("item_name");
			pus_item.name = (CN3UIString*)item->GetChildByID(find);
			find = xorstr("item_price");
			pus_item.price = (CN3UIString*)item->GetChildByID(find);
			find = xorstr("item_quantitiy");
			pus_item.quantitiy = (CN3UIString*)item->GetChildByID(find);
			find = xorstr("btn_purchase");
			pus_item.purchase = (CN3UIButton*)item->GetChildByID(find);
			find = xorstr("btn_add_basket");
			pus_item.add_to = (CN3UIButton*)item->GetChildByID(find);

			pus_item.name->SetString(xorstr("-"));
			pus_item.price->SetString(xorstr("-"));
			pus_item.quantitiy->SetString(xorstr("-"));
			pus_item.icon->GetParent()->SetVisible(false);

			pus_items.push_back(pus_item);
		}
	}

	for (int i = 1; i <= 7; i++)
	{
		find = string_format(xorstr("purchase_list%d"), i);
		CN3UIImage* item = (CN3UIImage*)GetChildByID(find);
		UI_PURCHASE_LIST pus_item;
		find = xorstr("item_icon");
		pus_item.icon = (CN3UIImage*)item->GetChildByID(find);
		find = xorstr("shopping_name");
		pus_item.name = (CN3UIString*)item->GetChildByID(find);
		find = xorstr("shopping_price");
		pus_item.price = (CN3UIString*)item->GetChildByID(find);
		find = xorstr("btn_cancel");
		pus_item.cancel = (CN3UIButton*)item->GetChildByID(find);

		pus_item.name->SetString(xorstr("-"));
		pus_item.price->SetString(xorstr("-"));
		pus_item.icon->GetParent()->SetVisible(false);
		purchase_items.push_back(pus_item);
	}

	txt_page->SetString(to_string(page));

	SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, Engine->m_UiMgr->GetScreenCenter(this).y);
	Engine->m_UiMgr->SetFocusedUI(this);

	return true;
}

bool CPowerUpStore::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == btn_useesn) {
			int firstbox;
			std::string kesncode = "";
			kesncode = txt_code->GetString().c_str();

			std::vector<std::string> out;
			Engine->str_split(kesncode.c_str(), '-', out);
			if (out.size() != 5)
				return false;
			else if (out.size() == 5) {
				firstbox = atoi(out[0].c_str());
				std::string secondtext = kesncode;
				Engine->str_replace(secondtext, "-", "");
				Engine->str_replace(secondtext, out[0].c_str(), "");
				kesncode = secondtext;


			}
			Engine->WriteChatAddInfo(D3DCOLOR_ARGB(255, 254, 128, 254), false, string_format("%d %s",firstbox,kesncode.c_str()).c_str());

			Packet result(WIZ_EDIT_BOX);
			result.SByte();
			result << uint8(4) << firstbox << kesncode;
			Engine->Send(&result);

			

		}
		//---------- Ekledim
		
		if (pSender == btn_buyall) { 
			basket_confirm->SetVisible(true);
			setBaseItems(false);
		}
		if (pSender == basket_no) {
			basket_confirm->SetVisible(false);
			setBaseItems(true);
		}
		if (pSender == basket_ok) {
			basket_confirm->SetVisible(false);
			setBaseItems(true);
		}
		if (pSender == btn_cancel) {
			shopping_confirm->SetVisible(false);
			if (gift_edit_name) {
				gift_edit_name->SetString("");
				gift_edit_name->KillFocus();
			}
			setBaseItems(true);
		}
		if (pSender == btn_confirm) {
			shopping_confirm->SetVisible(false);
			setBaseItems(true);
		}

		if (pSender == btn_pusrefund) {
			basepusrefund->SetVisible(true);
			setBaseItems(false);
			UpdateRefundItemList(Engine->m_UiMgr->pusrefund_itemlist);
			RefundSetItems();
		}
		if (pSender == btn_refundclose) {
			basepusrefund->SetVisible(false);
			setBaseItems(true);
			RefundSetItems();
			//UpdateRefundItemList(Engine->m_UiMgr->pusrefund_itemlist);
		}

		for (int i = 0; i < 18; i++) {
			if (pSender == refund_items[i].btn_ireturnadd) {

				if (GetTickCount() - m_LastTick <= 5) {
					Engine->WriteInfoMessageExt((char*)string_format(xorstr("time error!")).c_str(), 0xa30000);
					return false;
				} m_LastTick = GetTickCount();

				Packet newpkt(XSafe, uint8(XSafeOpCodes::PusRefund));
				newpkt << uint8(pusrefunopcode::ireturn) << refund_items[i].serial;
				Engine->Send(&newpkt);
			}
		}

		////---------- Ekledim
		//if (pSender == btn_esncancel) {
		//	Use_Cash_Base->SetVisible(false);
		//	std::string str[5];
		//	for (int i = 0; i < 5; i++) {
		//		txt_esnedit[i]->SetString("");
		//		txt_esnedit[i]->KillFocus();
		//	}
		//	setBaseItems(true);

		//}
		/*if (pSender == btn_esnconfirm) {
			std::string str[5];
			Packet result(WIZ_EDIT_BOX);
			result.SByte();
			std::string Number2 = "";
			for (int i = 0; i < 5; i++) {
				str[i] = txt_esnedit[i]->GetString();
				if( i > 0)
				Number2 += str[i];
				txt_esnedit[i]->KillFocus();
			}

			if (Number2.length() == 16) {
				if (!str[0].empty()) {
					int number1 = atoi(str[0].c_str());
					result << uint8(4) << number1 << Number2;
					Engine->Send(&result);
					Use_Cash_Base->SetVisible(false);
					setBaseItems(true);
					for (int i = 0; i < 5; i++)
						txt_esnedit[i]->SetString("");

				}
			}

		}*/

		if (pSender == btn_close)
			Close();
		else if (pSender == btn_next)
		{
			if (page < pageCount)
			{
				page++;
				SetItems(page, cat);
			}
		}
		else if (pSender == btn_previous)
		{
			if (page > 1)
			{
				page--;
				SetItems(page, cat);
			}
		}
		else if (pSender == btn_search)
		{
			SetItems(page, cat, txt_search->GetString());
		}
		else if (pSender == btn_clearall)
			PurchaseDeleteAll();
		else if (pSender == basket_ok)
			PurchaseBuyAll();
		else if (pSender == basket_no)
			basket_confirm->SetVisible(false);
		else if (pSender == btn_buyall)
			BasketMenuOpen();
		else if (pSender == btn_confirm)
		{
			if (gift_edit_name == NULL) return false;

			if (itemscount == 0) itemscount = 1;

			Packet pkt(XSafe);
			pkt << uint8_t(XSafeOpCodes::PUS);
			
			pkt.DByte();
			if (gift_edit_name->GetString().empty()) pkt << uint8(0x01) << uint32(PusID) << uint8(itemscount);
			else pkt << uint8(0x03) << uint32(PusID) << std::string(gift_edit_name->GetString());
			Engine->Send(&pkt);

			shopping_confirm->SetVisible(false);
			btn_confirm->SetState(UI_STATE_BUTTON_NORMAL);
			btn_cancel->SetState(UI_STATE_BUTTON_NORMAL);
			itemscount = 1;
			item_count->SetString(string_format("%d", itemscount));

			gift_edit_name->SetString("");
			gift_edit_name->KillFocus();
		}
		else if (pSender == btn_up)
		{
			itemscount++;

			if (itemscount > 28)
				itemscount = 28;

			item_count->SetString(string_format("%d", itemscount));
			shopping_after->SetString(string_format(xorstr("%d Knight Cash"), itemscount * shopping_p));
		}
		else if (pSender == btn_down)
		{
			itemscount--;

			if (itemscount <= 0)
				itemscount = 1;

			item_count->SetString(string_format("%d", itemscount));
			shopping_after->SetString(string_format(xorstr("%d Knight Cash"), itemscount * shopping_p));

		}
		else if (pSender == btn_cancel)
		{
			shopping_confirm->SetVisible(false);
			btn_confirm->SetState(UI_STATE_BUTTON_NORMAL);
			btn_cancel->SetState(UI_STATE_BUTTON_NORMAL);
			itemscount = 1;
			item_count->SetString(string_format("%d", itemscount));
		}
		else if (pSender == btn_pusrefund) {
			RefundSetItems();
		}
		/*else if (pSender == btn_useesn)
		{
			if (Engine->uiTaskbarMain != NULL)
			{
				Engine->uiTaskbarMain->OpenPPCard();
				Close();
			}
		}**/
		else if (pSender == btn_reseller)
		{
			ShellExecute(NULL, xorstr("open"), xorstr("https://www.klasgame.com/mmorpg-oyunlar/ko-filozof"), NULL, NULL, SW_SHOWNORMAL); //Klasgame baðlantý linki bnuraya eklenicek.
		}
		else
		{
			for (int i = 0; i < btn_tabs.size(); i++)
			{

				if (pSender == btn_tabs[i])
				{
					for (int j = 0; j < btn_tabs.size(); j++)
						btn_tabs[j]->SetState(UI_STATE_BUTTON_NORMAL);
					btn_tabs[i]->SetState(UI_STATE_BUTTON_DISABLE);
					page = 1;
					cat = i;
					SetItems(page, cat);
				}
			}
			for (int i = 0; i < PUS_ITEMPERCOUNT; i++)
			{
				if (pus_items[i].purchase != NULL)
				{
					if (pSender == pus_items[i].purchase)
					{
						shopping_id = pus_items[i].sItemID;
						for (int i = 0; i < item_list.size(); i++)
						{
							if (item_list[i].ItemID == shopping_id)
							{
								PusID = item_list[i].ID;
								shopping_p = item_list[i].Price;
								shopping_q = item_list[i].Quantitiy;
								break;
							}
						}
						OpenShopping();
					}
				}
			}

			for (int i = 0; i < PUS_ITEMPERCOUNT; i++)
			{
				if (pus_items[i].add_to != NULL)
				{
					if (pSender == pus_items[i].add_to)
					{
						shopping_id = pus_items[i].sItemID;
						if (shopping_id != NULL)
						{
							for (int i = 0; i < item_list.size(); i++)
							{
								if (item_list[i].ItemID == shopping_id)
								{
									PurchaseItemAdd(shopping_id, item_list[i].Price, item_list[i].isTlBalance);
									break;
								}
							}
						}
					}
				}
			}
		}

		PurchaseButtonClick(pSender);
	}

	return true;
}

void CPowerUpStore::OpenShopping()
{
	shopping_confirm->SetState(UI_STATE_BUTTON_NORMAL);
	if (Engine->Player.KnightCash < shopping_p)
		shopping_confirm->SetState(UI_STATE_BUTTON_DISABLE);

	gift_edit_name->SetString("");

	TABLE_ITEM_BASIC * tbl = Engine->tblMgr->getItemData(shopping_id);
	if (tbl != nullptr)
	{

		if (TABLE_ITEM_EXT* ext = Engine->tblMgr->getExtData(tbl->extNum, shopping_id))
		{
			DWORD dwIconID = tbl->dxtID;
			if (ext->dxtID > 0)
				dwIconID = ext->dxtID;

			std::string dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);

			shopping_icon->SetTex(dxt);
		}
		else {
			std::vector<char> buffer(256, NULL);

			sprintf(&buffer[0], "ui\\itemicon_%.1d_%.4d_%.2d_%.1d.dxt",
				(tbl->dxtID / 10000000),
				(tbl->dxtID / 1000) % 10000,
				(tbl->dxtID / 10) % 100,
				tbl->dxtID % 10);

			std::string szIconFN = &buffer[0];

			shopping_icon->SetTex(szIconFN);
		}

		shopping_name->SetString(tbl->strName);
		shopping_price->SetString(string_format(xorstr("Price: %s"), Engine->StringHelper->NumberFormat(shopping_p).c_str()));
		shopping_quantity->SetString(string_format(xorstr("Quantity: %s"), Engine->StringHelper->NumberFormat(shopping_q).c_str()));
		shopping_after->SetString(string_format(xorstr("%d Knight Cash"),shopping_p));
	}
	shopping_confirm->SetVisible(true);
}

uint32_t CPowerUpStore::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (Engine->m_UiMgr->m_FocusedUI == this)
	{
		bool showTooltip = false;
		bool pass = false;

		if (shopping_icon != NULL)
		{
			if (shopping_icon->IsIn(ptCur.x, ptCur.y) && shopping_id > 0)
			{
				TABLE_ITEM_BASIC * tbl = Engine->tblMgr->getItemData(shopping_id);
				if (tbl != nullptr)
				{
					Engine->m_UiMgr->m_pUITooltipDlg->DisplayTooltipsEnable(ptCur.x, ptCur.y, GetItemStruct(shopping_icon, shopping_icon->FileName(), tbl, shopping_id), this, false, false, false);
					showTooltip = true;
					pass = true;
				}
			}
		}

		if (shopping_confirm->IsIn(ptCur.x, ptCur.y) && shopping_confirm->IsVisible())
			pass = true;

		if (!pass)
		{
			for (auto it : pus_items)
			{
#if(PUS_ITEMSCOUNT == 4)
				if (it.icon != NULL && it.sItemID > 0 && itemGroupBase[0]->IsVisible() && itemGroupBase[1]->IsVisible() && itemGroupBase[2]->IsVisible() && itemGroupBase[3]->IsVisible())
#else
				if (it.icon != NULL && it.sItemID > 0 && itemGroupBase[0]->IsVisible() && itemGroupBase[1]->IsVisible() && itemGroupBase[2]->IsVisible())
#endif
				{
					if (it.icon->IsIn(ptCur.x, ptCur.y) && it.icon->GetParent()->IsVisible())
					{
						TABLE_ITEM_BASIC * tbl = Engine->tblMgr->getItemData(it.sItemID);
						if (tbl != nullptr)
						{
							Engine->m_UiMgr->m_pUITooltipDlg->DisplayTooltipsEnable(ptCur.x, ptCur.y, GetItemStruct(it.icon, it.icon->FileName(), tbl, it.sItemID), this, false, false, false);
							showTooltip = true;
							break;
						}
					}
				}
			}
		}
		Engine->m_UiMgr->m_pUITooltipDlg->SetVisible(showTooltip);
	}
	dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
	return dwRet;
}

bool CPowerUpStore::OnKeyPress(int iKey)
{
	if (!IsVisible())
		return CN3UIBase::OnKeyPress(iKey);

	switch (iKey)
	{
	case DIK_ESCAPE:
		Close();
		return true;
		break;
	case DIK_RETURN:
		ReceiveMessage(btn_search, UIMSG_BUTTON_CLICK);
		if (edit_search->HaveFocus()) edit_search->KillFocus();
		return true;
		break;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

void CPowerUpStore::Open()
{
	txt_code->SetString("");
	std::string OPTIONS_INI = xorstr("Option.ini");
	std::string path = Engine->m_BasePath + OPTIONS_INI;
	std::string section = xorstr("ViewPort");
	std::string key = xorstr("Width");
	Engine->m_SettingsMgr->m_clientwidth = GetPrivateProfileIntA(section.c_str(), key.c_str(), 0, path.c_str());
	key = xorstr("Height");
	Engine->m_SettingsMgr->m_clientheigth = GetPrivateProfileIntA(section.c_str(), key.c_str(), 0, path.c_str());


	txt_cash->SetString(Engine->StringHelper->NumberFormat(Engine->Player.KnightCash).c_str());
	txt_tlbalance->SetString(Engine->StringHelper->NumberFormat(Engine->Player.TlBalance).c_str());
	SetItems();

	RefundSetItems();


	SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, Engine->m_UiMgr->GetScreenCenter(this).y);

	for (int i = 0; i < cat_list.size(); i++)
	{

		std::string find = string_format(xorstr("txt"));
		CN3UIString* tmp = (CN3UIString*)btn_tabs[i]->GetChildByID(find);
		if (cat_list[i].Status)
		{
			tmp->SetString(cat_list[i].categoryName.c_str());
			btn_tabs[i]->SetVisible(true);
		}
		else
			btn_tabs[i]->SetVisible(false);
	}


	SetVisible(true);
}

void CPowerUpStore::setBaseItems(int value) {
	for (int i = 1; i <= PUS_ITEMSCOUNT; i++)
	{
		itemGroupBase[i - 1]->SetVisible(value);
	}
}
void CPowerUpStore::Close()
{
	edit_search->KillFocus();
	edit_search->SetString("");
	SetVisible(false);
	m_bNeedToRestore = false;
}

void CPowerUpStore::UpdateItemList(vector<PUSItem> items, vector<PusCategory> cats)
{
	item_list = items;
	cat_list = cats;
	page = 1;
	pageCount = abs(ceil((double)item_list.size() / (double)PUS_ITEMPERCOUNT));
}

void CPowerUpStore::UpdateRefundItemList(vector<PUSREFUNDITEM> refund_items) { refund_list = refund_items; }

bool pus_refund(const PUSREFUNDITEM & a, const PUSREFUNDITEM & b)
{
	return a.expiredtime < b.expiredtime;
}

void CPowerUpStore::RefundSetItems() {
	vector<PUSREFUNDITEM> tmpList;
	for (int i = 0; i < (int)refund_list.size(); i++)  tmpList.push_back(refund_list[i]);
	
	if (tmpList.empty()) {
		for (int i = 0; i < 18; i++) refund_items[i].icon->GetParent()->SetVisible(false);
		return;
	}

	std::sort(tmpList.begin(), tmpList.end(), pus_refund);

	int count = 0, size = (int)tmpList.size();
	for (int i = 0; i < 18; i++) {

		if (++count > 18 || size == 0) break;
		if (i >= size) {
			refund_items[i].icon->GetParent()->SetVisible(false);
			continue;
		}

		TABLE_ITEM_BASIC * tbl = Engine->tblMgr->getItemData(tmpList[i].itemid);
		if (tbl == nullptr) {
			refund_items[i].icon->GetParent()->SetVisible(false);
			continue;
		}

		if (TABLE_ITEM_EXT* ext = Engine->tblMgr->getExtData(tbl->extNum, tmpList[i].itemid)) {
			DWORD dwIconID = tbl->dxtID;
			if (ext->dxtID > 0) dwIconID = ext->dxtID;
			std::string dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);
			refund_items[i].icon->SetTex(dxt);
		}
		else {
			std::vector<char> buffer(256, NULL);

			sprintf(&buffer[0], "ui\\itemicon_%.1d_%.4d_%.2d_%.1d.dxt",(tbl->dxtID / 10000000),(tbl->dxtID / 1000) % 10000,
				(tbl->dxtID / 10) % 100,tbl->dxtID % 10);

			std::string szIconFN = &buffer[0];
			refund_items[i].icon->SetTex(szIconFN);
		}

		refund_items[i].sItemID = tmpList[i].itemid;
		refund_items[i].serial = tmpList[i].serial;
		refund_items[i].name->SetString(tbl->strName);
		refund_items[i].price->SetString(string_format(xorstr("Price: %s"), Engine->StringHelper->NumberFormat(tmpList[i].kcprice).c_str()));
		refund_items[i].icon->GetParent()->SetVisible(true);
	}
}

void CPowerUpStore::SetItems(int p, int cat, string q)
{
	vector<PUSItem> tmpList;
	for (int i = 0; i < item_list.size(); i++)
	{
		if (cat > 0 && q == "")
		{
			if (item_list[i].Category == cat)
				tmpList.push_back(item_list[i]);
		}
		else if (cat == 0 && q != "")
		{
			TABLE_ITEM_BASIC * tbl = Engine->tblMgr->getItemData(item_list[i].ItemID);
			if (tbl != nullptr)
			{
				if (Engine->StringHelper->IsContains(tbl->strName, q))
					tmpList.push_back(item_list[i]);
			}
		}
		else if (cat > 0 && q != "")
		{
			if (item_list[i].Category == cat)
			{
				TABLE_ITEM_BASIC * tbl = Engine->tblMgr->getItemData(item_list[i].ItemID);
				if (tbl != nullptr)
				{
					if (Engine->StringHelper->IsContains(tbl->strName, q))
						tmpList.push_back(item_list[i]);
				}
			}
		}
		else
			tmpList.push_back(item_list[i]);
	}

	if (tmpList.size() == 0)
	{
		for(int i = 0; i < PUS_ITEMPERCOUNT; i++)
			pus_items[i].icon->GetParent()->SetVisible(false);

		txt_page->SetString(to_string(page));
		pageCount = 1;
		return;
	}

	std::sort(tmpList.begin(), tmpList.end(), pus_compare);

	pageCount = abs(ceil((double)tmpList.size() / (double)PUS_ITEMPERCOUNT));

	int begin = (p - 1) * PUS_ITEMPERCOUNT;
	int j = -1;
	for (int i = begin; i < begin+ PUS_ITEMPERCOUNT; i++)
	{
		j++;
#if (PUS_ITEMPERCOUNT == 16)
		if (j > 15)
			break;
#else
		if (j > 11)
			break;
#endif
		if (i > tmpList.size() - 1)
		{
			pus_items[j].icon->GetParent()->SetVisible(false);
			continue;
		}

		TABLE_ITEM_BASIC * tbl = Engine->tblMgr->getItemData(tmpList[i].ItemID);
		if (tbl == nullptr)
		{
			pus_items[j].icon->GetParent()->SetVisible(false);
			continue;
		}

		if (TABLE_ITEM_EXT* ext = Engine->tblMgr->getExtData(tbl->extNum, tmpList[i].ItemID))
		{
			DWORD dwIconID = tbl->dxtID;
			if (ext->dxtID > 0)
				dwIconID = ext->dxtID;

			std::string dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);

			pus_items[j].icon->SetTex(dxt);
		}
		else {
			std::vector<char> buffer(256, NULL);

			sprintf(&buffer[0], "ui\\itemicon_%.1d_%.4d_%.2d_%.1d.dxt",
				(tbl->dxtID / 10000000),
				(tbl->dxtID / 1000) % 10000,
				(tbl->dxtID / 10) % 100,
				tbl->dxtID % 10);

			std::string szIconFN = &buffer[0];

			pus_items[j].icon->SetTex(szIconFN);
		}


		pus_items[j].sItemID = tmpList[i].ItemID;

		pus_items[j].name->SetString(tbl->strName);
		pus_items[j].price->SetString(string_format(xorstr("%s %s"), Engine->StringHelper->NumberFormat(tmpList[i].Price).c_str(),(tmpList[i].isTlBalance==1 ? "TL" : "KC")));
		pus_items[j].quantitiy->SetString(string_format(xorstr("%s"), Engine->StringHelper->NumberFormat(tmpList[i].Quantitiy).c_str()));

		bool enoughtMoney = true;
		bool enoughtTLBalance = true;
		if ( Engine->Player.KnightCash < tmpList[i].Price)
			enoughtMoney = false;
		if (Engine->Player.TlBalance < tmpList[i].Price)
			enoughtTLBalance = false;
		if (!tmpList[i].isTlBalance)
		{
			if (enoughtMoney)
			{
				//pus_items[j].price->SetColor(D3DCOLOR_RGBA(146, 252, 175, 255));
				pus_items[j].price->SetColor(D3DCOLOR_RGBA(255, 0, 0, 255));
				pus_items[j].purchase->SetState(UI_STATE_BUTTON_NORMAL);
			}
			else
			{
				//pus_items[j].price->SetColor(D3DCOLOR_RGBA(247, 141, 173, 255));
				pus_items[j].price->SetColor(D3DCOLOR_RGBA(255, 0, 0, 255));
				pus_items[j].purchase->SetState(UI_STATE_BUTTON_DISABLE);
			}
		}
		else {
			if (enoughtTLBalance)
			{
				//pus_items[j].price->SetColor(D3DCOLOR_RGBA(146, 252, 175, 255));
				pus_items[j].price->SetColor(D3DCOLOR_RGBA(255, 0, 0, 255));
				pus_items[j].purchase->SetState(UI_STATE_BUTTON_NORMAL);
			}
			else
			{
				//pus_items[j].price->SetColor(D3DCOLOR_RGBA(247, 141, 173, 255));
				pus_items[j].price->SetColor(D3DCOLOR_RGBA(255, 0, 0, 255));
				pus_items[j].purchase->SetState(UI_STATE_BUTTON_DISABLE);
			}

		}
		pus_items[j].icon->GetParent()->SetVisible(true);

	}
	txt_page->SetString(to_string(page));
}

void CPowerUpStore::UpdateCash(uint32 kc,uint32 tl)
{
	txt_cash->SetString(Engine->StringHelper->NumberFormat(kc).c_str());
	txt_tlbalance->SetString(Engine->StringHelper->NumberFormat(tl).c_str());
}

__IconItemSkill * CPowerUpStore::GetItemStruct(CN3UIImage * img, std::string fileSz, __TABLE_ITEM_BASIC * tbl, uint32 realID)
{
	__IconItemSkill* newStruct = new __IconItemSkill;
	newStruct->pUIIcon = img;
	newStruct->szIconFN = fileSz;
	newStruct->index = 0;

	__TABLE_ITEM_BASIC* newItemBasic = new __TABLE_ITEM_BASIC;
	__TABLE_ITEM_BASIC* itemOrg = tbl;
	newItemBasic->Num = itemOrg->Num;
	newItemBasic->extNum = itemOrg->extNum;
	newItemBasic->strName = itemOrg->strName;
	newItemBasic->Description = itemOrg->Description;

	newItemBasic->Class = itemOrg->Class;

	newItemBasic->Attack = itemOrg->Attack;
	newItemBasic->Delay = itemOrg->Delay;
	newItemBasic->Range = itemOrg->Range;
	newItemBasic->Weight = itemOrg->Weight;
	newItemBasic->Duration = itemOrg->Duration;
	newItemBasic->repairPrice = itemOrg->repairPrice;
	newItemBasic->sellingPrice = itemOrg->sellingPrice;
	newItemBasic->AC = itemOrg->AC;
	newItemBasic->isCountable = itemOrg->isCountable;
	newItemBasic->ReqLevelMin = itemOrg->ReqLevelMin;

	newItemBasic->ReqStr = itemOrg->ReqStr;
	newItemBasic->ReqHp = itemOrg->ReqHp;
	newItemBasic->ReqDex = itemOrg->ReqDex;
	newItemBasic->ReqInt = itemOrg->ReqInt;
	newItemBasic->ReqMp = itemOrg->ReqMp;

	newItemBasic->SellingGroup = itemOrg->SellingGroup;

	newStruct->pItemBasic = newItemBasic;

	__TABLE_ITEM_EXT* newItemExt = Engine->tblMgr->getExtData(itemOrg->extNum, realID);

	if (newItemExt == nullptr)
	{
		newItemExt = new __TABLE_ITEM_EXT;
		newItemExt->extensionID = itemOrg->extNum;
		newItemExt->szHeader = itemOrg->strName;
		newItemExt->byMagicOrRare = itemOrg->ItemGrade;
		newItemExt->siDamage = 0;
		newItemExt->siAttackIntervalPercentage = 100;
		newItemExt->siHitRate = 0;
		newItemExt->siEvationRate = 0;
		newItemExt->siMaxDurability = 0;
		newItemExt->siDefense = 0;
		newItemExt->siDefenseRateDagger = 0;
		newItemExt->siDefenseRateSword = 0;
		newItemExt->siDefenseRateBlow = 0;
		newItemExt->siDefenseRateAxe = 0;
		newItemExt->siDefenseRateSpear = 0;
		newItemExt->siDefenseRateArrow = 0;
		newItemExt->byDamageFire = 0;
		newItemExt->byDamageIce = 0;
		newItemExt->byDamageThuner = 0;
		newItemExt->byDamagePoison = 0;
		newItemExt->byStillHP = 0;
		newItemExt->byDamageMP = 0;
		newItemExt->byStillMP = 0;
		newItemExt->siBonusStr = 0;
		newItemExt->siBonusSta = 0;
		newItemExt->siBonusHP = 0;
		newItemExt->siBonusDex = 0;
		newItemExt->siBonusMSP = 0;
		newItemExt->siBonusInt = 0;
		newItemExt->siBonusMagicAttak = 0;
		newItemExt->siRegistFire = 0;
		newItemExt->siRegistIce = 0;
		newItemExt->siRegistElec = 0;
		newItemExt->siRegistMagic = 0;
		newItemExt->siRegistPoison = 0;
		newItemExt->siRegistCurse = 0;
		newItemExt->siNeedLevel = 0;
	}

	newStruct->pItemExt = newItemExt;

	return newStruct;
}

bool pus_compare(const PUSItem & a, const PUSItem & b)
{
	return a.ID < b.ID;
}

void CPowerUpStore::OpenPowerUpStore()
{
	std::string name = Engine->dcpUIF(xorstr("Msoft\\PowerUpStore.Msoft"));
	Engine->m_UiMgr->uiPowerUpStore = new CPowerUpStore();
	Engine->m_UiMgr->uiPowerUpStore->Init(this);
	Engine->m_UiMgr->uiPowerUpStore->LoadFromFile(Engine->m_BasePath + name.c_str(), N3FORMAT_VER_1068);
	Engine->m_UiMgr->AddChild(Engine->m_UiMgr->uiPowerUpStore);
	remove(name.c_str());
}

void CPowerUpStore::PurchaseItemAdd(uint32 ItemID, uint32 Price, uint8 PriceType)
{
	for (int i = 0; i < 7; i++)
	{
		if (purchase_items[i].icon->GetParent()->IsVisible())
			continue;

		TABLE_ITEM_BASIC * tbl = Engine->tblMgr->getItemData(ItemID);
		if (tbl != nullptr)
		{

			if (TABLE_ITEM_EXT* ext = Engine->tblMgr->getExtData(tbl->extNum, shopping_id))
			{
				DWORD dwIconID = tbl->dxtID;
				if (ext->dxtID > 0)
					dwIconID = ext->dxtID;

				std::string dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);

				purchase_items[i].icon->SetTex(dxt);
				purchase_items[i].name->SetString(tbl->strName);
				purchase_items[i].sItemID = ItemID;
				purchase_items[i].sCash = Price;
			
				std::string ncash = string_format("%s %s", Engine->StringHelper->NumberFormat(Price).c_str(),(PriceType==1 ? "TL" :" KnightCash"));
				purchase_items[i].price->SetString(ncash);
				purchase_items[i].icon->GetParent()->SetVisible(true);
			}
			else
			{
				std::vector<char> buffer(256, NULL);

				sprintf(&buffer[0], "ui\\itemicon_%.1d_%.4d_%.2d_%.1d.dxt",
					(tbl->dxtID / 10000000),
					(tbl->dxtID / 1000) % 10000,
					(tbl->dxtID / 10) % 100,
					tbl->dxtID % 10);

				std::string szIconFN = &buffer[0];

				purchase_items[i].icon->SetTex(szIconFN);
				purchase_items[i].name->SetString(tbl->strName);
				purchase_items[i].sItemID = ItemID;
				purchase_items[i].sCash = Price;

				std::string ncash = string_format("%s %s", Engine->StringHelper->NumberFormat(Price).c_str(), (PriceType == 1 ? "TL" : " KnightCash"));
				purchase_items[i].price->SetString(ncash);
				purchase_items[i].icon->GetParent()->SetVisible(true);
			}

			break;
		}
	}

	PurchaseTotal();
}

void CPowerUpStore::PurchaseButtonClick(CN3UIBase* pSender)
{
	for (int i = 0; i < 7; i++)
	{
		if (purchase_items[i].cancel != NULL)
		{
			if (pSender == purchase_items[i].cancel)
			{
				purchase_items[i].sItemID = 0;
				purchase_items[i].sCash = 0;
				purchase_items[i].icon->GetParent()->SetVisible(false);
				PurchaseTotal();
				break;
			}
		}
	}
}

void CPowerUpStore::PurchaseTotal()
{
	uint32 totalprice = 0;

	for (int i = 0; i < 7; i++)
	{
		if (purchase_items[i].icon->GetParent()->IsVisible())
			totalprice += purchase_items[i].sCash;
	}

	total_cash->SetString(string_format(xorstr("%s"), Engine->StringHelper->NumberFormat(totalprice).c_str()));
}

void CPowerUpStore::PurchaseDeleteAll()
{
	for (int i = 0; i < 7; i++)
	{
		purchase_items[i].sItemID = 0;
		purchase_items[i].sCash = 0;
		purchase_items[i].type = 0;
		purchase_items[i].icon->GetParent()->SetVisible(false);
	}

	PurchaseTotal();
}

void CPowerUpStore::PurchaseBuyAll()
{
	basket_confirm->SetVisible(false);

	for (int i = 0; i < 7; i++)
	{
		if (purchase_items[i].icon->GetParent()->IsVisible() && purchase_items[i].sItemID > 0)
		{
			for (int j = 0; j < item_list.size(); j++)
			{
				if (item_list[j].ItemID == purchase_items[i].sItemID)
				{
					Packet pkt(XSafe);
					pkt << uint8_t(XSafeOpCodes::PUS) << uint8(0x01) << uint32(item_list[j].ID) << uint8(1);
					Engine->Send(&pkt);
					break;
				}
			}
		}
	}

	PurchaseDeleteAll();
	PurchaseTotal();
}

void CPowerUpStore::BasketMenuOpen()
{
	uint32 totalprice = 0;

	for (int i = 0; i < 7; i++)
	{
		if (purchase_items[i].icon->GetParent()->IsVisible())
			totalprice += purchase_items[i].sCash;
	}

	basket_cash->SetString(string_format(xorstr("%s Knight Cash"), Engine->StringHelper->NumberFormat(totalprice).c_str()));
	basket_confirm->SetVisible(true);
}