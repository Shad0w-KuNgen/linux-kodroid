#include "hPowerUpStore.h"
bool pus_compare(const PUSItem& a, const PUSItem& b);

CUIPowerUpStore::CUIPowerUpStore()
{
	m_dVTableAddr = NULL;

	vector<int>offsets;
	offsets.push_back(0x348);   // re_rental_message Satýr : 115
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);

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
	cat = 1;
	itemscount = 0;
	esntime = GetTickCount64();
	m_LastTick = GetTickCount();
	ParseUIElements();
	InitReceiveMessage();
}

CUIPowerUpStore::~CUIPowerUpStore()
{

}

void CUIPowerUpStore::ParseUIElements()
{	
	// Base
	shopping_confirm = Engine->GetChildByID(m_dVTableAddr, "shopping_confirm");
	btn_confirm = Engine->GetChildByID(shopping_confirm, "btn_confirm");
	btn_cancel = Engine->GetChildByID(shopping_confirm, "btn_cancel");
	shopping_icon = Engine->GetChildByID(shopping_confirm, "shopping_icon");
	shopping_name = Engine->GetChildByID(shopping_confirm, "shopping_name");
	shopping_price = Engine->GetChildByID(shopping_confirm, "shopping_price");
	shopping_quantity = Engine->GetChildByID(shopping_confirm, "shopping_quantity");
	shopping_after = Engine->GetChildByID(shopping_confirm, "shopping_after");
	btn_up = Engine->GetChildByID(shopping_confirm, "btn_up");
	item_count = Engine->GetChildByID(shopping_confirm, "str_count");
	gift_edit_name = Engine->GetChildByID(shopping_confirm, "txt_gift_name");
	Engine->SetVisible(shopping_confirm, false);

	//basket menu	
	basket_confirm = Engine->GetChildByID(m_dVTableAddr, "Basket_confirm");
	basket_cash = Engine->GetChildByID(basket_confirm, "text_basket_total_price");
	basket_ok = Engine->GetChildByID(basket_confirm, "btn_all_confirm");
	basket_no = Engine->GetChildByID(basket_confirm, "btn_cancel");
	Engine->SetVisible(basket_confirm,false);

	// edit
	txt_code = Engine->GetChildByID(m_dVTableAddr, "esncode");

	// Buttons
	btn_close = Engine->GetChildByID(m_dVTableAddr, "btn_close");
	btn_search = Engine->GetChildByID(m_dVTableAddr, "btn_search");
	btn_reseller = Engine->GetChildByID(m_dVTableAddr, "btn_reseller");
	btn_next = Engine->GetChildByID(m_dVTableAddr, "btn_next");
	btn_previous = Engine->GetChildByID(m_dVTableAddr, "btn_previous");
	btn_useesn = Engine->GetChildByID(m_dVTableAddr, "btn_useesn");
	btn_pusrefund = Engine->GetChildByID(m_dVTableAddr, "btn_refund");

	for (int i = 1; i <= 5; i++)
		btn_tabs.push_back(Engine->GetChildByID(m_dVTableAddr, string_format(xorstr("btn_tab%d"), i)));
	
	Engine->SetState(btn_tabs[0], UI_STATE_BUTTON_DISABLE);

	// Texts
	edit_search = Engine->GetChildByID(m_dVTableAddr, "edit_search");
	txt_search = Engine->GetChildByID(m_dVTableAddr, "edit_search");
	txt_page = Engine->GetChildByID(m_dVTableAddr, "txt_page");
	txt_cash = Engine->GetChildByID(m_dVTableAddr, "txt_cash");
	txt_tlbalance = Engine->GetChildByID(m_dVTableAddr, "txt_tl_balance");
	total_cash = Engine->GetChildByID(m_dVTableAddr, "text_basket_total_price");
	btn_buyall = Engine->GetChildByID(m_dVTableAddr, "btn_buyall");
	btn_clearall = Engine->GetChildByID(m_dVTableAddr, "btn_clearall");
	basepusrefund = Engine->GetChildByID(m_dVTableAddr, "base_refund");
	
	for (int i = 0; i < 18; i++) {
		baserefunditem[i] = Engine->GetChildByID(basepusrefund, string_format(xorstr("refund_items_%d"), i));
		DWORD itemGroup = baserefunditem[i];
	
		UI_REFUND_LIST refund_list{};
		refund_list.sItemID = 0;
		refund_list.nBase = itemGroup;
		refund_list.name = Engine->GetChildByID(itemGroup, string_format(xorstr("refund_item%d"), i));
		refund_list.price = Engine->GetChildByID(itemGroup, string_format(xorstr("refund_price%d"), i));
		refund_list.icon = Engine->GetChildByID(itemGroup, xorstr("item_icon"));
		refund_list.btn_ireturnadd = Engine->GetChildByID(itemGroup, "btn_ireturn");
		Engine->SetString(refund_list.name, "-");
		Engine->SetString(refund_list.price, "-");
		Engine->SetVisible(refund_list.nBase, false);
		Engine->SetVisible(refund_list.icon, true);
		refund_items.push_back(refund_list);
	}

	btn_refundclose = Engine->GetChildByID(basepusrefund, "btn_close");
	Engine->SetVisible(basepusrefund, false);
	Engine->SetEditString(txt_code, "");

	// Pus Items
	for (int i = 1; i <= PUS_ITEMSCOUNT; i++)
	{
		itemGroupBase[i - 1] = Engine->GetChildByID(m_dVTableAddr, string_format(xorstr("items%d"), i));
		DWORD itemGroup = itemGroupBase[i - 1];
		for (int j = 1; j <= PUS_ITEMSCOUNT; j++)
		{
			DWORD item = 0;
			item = Engine->GetChildByID(itemGroup, string_format(xorstr("item%d"), j));

			UI_PUSITEM pus_item{};
			pus_item.nBase = item;
			pus_item.icon = Engine->GetChildByID(item, "item_icon");
			pus_item.name = Engine->GetChildByID(item, "item_name");
			pus_item.price = Engine->GetChildByID(item, "item_price");
			pus_item.quantitiy = Engine->GetChildByID(item, "item_quantitiy");
			pus_item.purchase = Engine->GetChildByID(item, "btn_purchase");
			pus_item.add_to = Engine->GetChildByID(item, "btn_add_basket");
			Engine->SetString(pus_item.name, "-");
			Engine->SetString(pus_item.price, "-");
			Engine->SetString(pus_item.quantitiy, "-");
			pus_items.push_back(pus_item);
		}
	}

	for (int i = 1; i <= 7; i++)
	{
		DWORD item = 0;
		item = Engine->GetChildByID(m_dVTableAddr, string_format(xorstr("purchase_list%d"), i));

		UI_PURCHASE_LIST pus_item{};
		pus_item.nBase = item;
		pus_item.icon = Engine->GetChildByID(item, "item_icon");
		pus_item.name = Engine->GetChildByID(item, "shopping_name");
		pus_item.price = Engine->GetChildByID(item, "shopping_price");
		pus_item.cancel = Engine->GetChildByID(item, "btn_cancel");
		Engine->SetString(pus_item.name, "-");
		Engine->SetString(pus_item.price, "-");
		Engine->SetVisible(item, false);
		purchase_items.push_back(pus_item);
	}
	Engine->SetString(txt_page, to_string(page));
	Engine->UIScreenCenter(m_dVTableAddr);
}

DWORD uiPus;
DWORD Func_puS;

bool CUIPowerUpStore::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	uiPus = m_dVTableAddr;
	if (!pSender || pSender == (DWORD*) 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == (DWORD*) (DWORD*) btn_useesn) {

			if (esntime > GetTickCount64()) {
				Engine->WriteChatAddInfo(D3DCOLOR_ARGB(255, 197, 209, 189), false, string_format("please wait this time %d", (esntime - GetTickCount64()) / 1000).c_str());
				return false;
			}
			esntime = GetTickCount64() + 60 * 1000;

			std::string kesncode = Engine->GetEditString(txt_code).c_str(), checklog = "Please check the characters you entered";

			if (kesncode.empty() || kesncode.length() != 20 || !Engine->WordGuardSystem(kesncode, kesncode.length())) {
				Engine->WriteChatAddInfo(D3DCOLOR_ARGB(255, 197, 209, 189), false, checklog.c_str());
				return false;
			}

			string firstbox = kesncode.substr(0, 4), secondbox = kesncode.substr(4);

			/*std::vector<std::string> out;
			Engine->str_split(kesncode.c_str(), '-', out);
			if (out.size() != 5) {
				Engine->WriteChatAddInfo(D3DCOLOR_ARGB(255, 197, 209, 189), false, checklog.c_str());
				return false;
			}

			int firstbox = atoi(out[0].c_str());
			for (int i = 1; i < out.size(); i++) secondbox.append(out[i]);

			std::string thyke = std::to_string(firstbox);
			if (thyke.empty() || thyke.length() != 4 || secondbox.empty() || secondbox.length() != 16) {
				Engine->WriteChatAddInfo(D3DCOLOR_ARGB(255, 197, 209, 189), false, checklog.c_str());
				return false;
			}*/

			Packet result(WIZ_EDIT_BOX);
			result.SByte();
			result << uint8(4) << std::stoi(firstbox) << secondbox;
			Engine->Send(&result);
		}
		
		if (pSender == (DWORD*)btn_refundclose) {
		
			setBaseItems(1);
			RefundSetItems();
			Engine->SetVisible(basepusrefund, false);
			//UpdateRefundItemList(Engine->m_UiMgr->pusrefund_itemlist);
		}

		for (int i = 0; i < 18; i++) {
			if (pSender == (DWORD*) (DWORD*) refund_items[i].btn_ireturnadd) {

				if (GetTickCount() - m_LastTick <= 5) {
					Engine->WriteInfoMessageExt((char*)string_format(xorstr("time error!")).c_str(), 0xa30000);
					return false;
				} m_LastTick = GetTickCount();

				Packet newpkt(XSafe, uint8(XSafeOpCodes::PusRefund));
				newpkt << uint8(pusrefunopcode::ireturn) << refund_items[i].serial;
				Engine->Send(&newpkt);
			}
		}


		if (pSender == (DWORD*) (DWORD*) btn_close)
			Close();
		else if (pSender == (DWORD*) (DWORD*) btn_next)
		{
			if (page < pageCount)
			{
				page++;
				SetItems(page, cat);
			}
		}
		else if (pSender == (DWORD*) (DWORD*) btn_previous)
		{
			if (page > 1)
			{
				page--;
				SetItems(page, cat);
			}
		}
		else if (pSender == (DWORD*) (DWORD*) btn_search)
		{
			SetItems(page, cat, Engine->GetEditString(txt_search).c_str());
		}
		else if (pSender == (DWORD*) (DWORD*) btn_clearall)
			PurchaseDeleteAll();
		else if (pSender == (DWORD*) (DWORD*) basket_ok)
			PurchaseBuyAll();
		else if (pSender == (DWORD*) (DWORD*) basket_no)
			Engine->SetVisible(basket_confirm,false);
		else if (pSender == (DWORD*) (DWORD*) btn_buyall)
			BasketMenuOpen();
		else if (pSender == (DWORD*) (DWORD*) btn_confirm)
		{
			if (gift_edit_name == NULL) return false;

			if (itemscount == 0) itemscount = 1;

			Packet pkt(XSafe);
			pkt << uint8_t(XSafeOpCodes::PUS);

			pkt.DByte();
			std::string getgiftname = Engine->GetEditString(gift_edit_name).c_str();
			if (getgiftname.empty()) pkt << uint8(0x01) << uint32(PusID) << uint8(itemscount);
			else pkt << uint8(0x03) << uint32(PusID) << std::string(getgiftname);
			Engine->Send(&pkt);

			Engine->SetVisible(shopping_confirm,false);
			Engine->SetState(btn_confirm,UI_STATE_BUTTON_NORMAL);
			Engine->SetState(btn_cancel,UI_STATE_BUTTON_NORMAL);
			itemscount = 1;
			Engine->SetString(item_count,string_format("%d", itemscount));

			Engine->SetEditString(gift_edit_name, "");
		}
		else if (pSender == (DWORD*) (DWORD*) btn_up)
		{
			itemscount++;

			if (itemscount > 28)
				itemscount = 28;

			Engine->SetString(item_count,string_format("%d", itemscount));
			Engine->SetString(shopping_quantity, string_format(xorstr("Quantity: %s"), Engine->StringHelper->NumberFormat(itemscount).c_str()));
			Engine->SetString(shopping_after,string_format(xorstr("%d Knight Cash"), itemscount * shopping_p));
		}
		else if (pSender == (DWORD*) (DWORD*) btn_down)
		{
			if(itemscount > 0) itemscount--;

			Engine->SetString(item_count,string_format("%d", itemscount));
			Engine->SetString(shopping_quantity, string_format(xorstr("Quantity: %s"), Engine->StringHelper->NumberFormat(itemscount).c_str()));
			Engine->SetString(shopping_after,string_format(xorstr("%d Knight Cash"), itemscount * shopping_p));

		}
		else if (pSender == (DWORD*) (DWORD*) btn_cancel)
		{
			Engine->SetVisible(shopping_confirm,false);
			Engine->SetState(btn_confirm,UI_STATE_BUTTON_NORMAL);
			Engine->SetState(btn_cancel,UI_STATE_BUTTON_NORMAL);
			itemscount = 1;
			Engine->SetString(item_count,string_format("%d", itemscount));
		}
		else if (pSender == (DWORD*) (DWORD*) btn_pusrefund) {
			RefundSetItems();
			setBaseItems(0);
		}
		/*else if (pSender == (DWORD*) (DWORD*) btn_useesn)
		{
			if (Engine->uiTaskbarMain != NULL)
			{
				Engine->uiTaskbarMain->OpenPPCard();
				Close();
			}
		}**/
		else if (pSender == (DWORD*) (DWORD*) btn_reseller)
		{
			if (Engine->uiHpMenuPlug != NULL && Engine->uiHpMenuPlug->ResellerURL.size())
				ShellExecute(NULL, xorstr("open"), Engine->uiHpMenuPlug->ResellerURL.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}
		else
		{
			for (int i = 0; i < btn_tabs.size(); i++)
			{

				if (pSender == (DWORD*) (DWORD*) btn_tabs[i])
				{
					for (int j = 0; j < btn_tabs.size(); j++)
						Engine->SetState(btn_tabs[j],UI_STATE_BUTTON_NORMAL);
					Engine->SetState(btn_tabs[i],UI_STATE_BUTTON_DISABLE);
					page = 1;
					cat = i+1;
					SetItems(page, cat);
				}
			}
			for (int i = 0; i < PUS_ITEMPERCOUNT; i++)
			{
				if (pus_items[i].purchase != NULL)
				{
					if (pSender == (DWORD*) (DWORD*) pus_items[i].purchase)
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
					if (pSender == (DWORD*) (DWORD*) pus_items[i].add_to)
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

		PurchaseButtonClick((DWORD)pSender);
	}

	return true;
}

void __stdcall UIPusReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiPowerUpStore->ReceiveMessage(pSender, dwMsg);
	__asm
	{
		MOV ECX, uiPus
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_puS
		CALL EAX
	}
}

void CUIPowerUpStore::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_puS = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)UIPusReceiveMessage_Hook;
}

void CUIPowerUpStore::OpenShopping()
{
	Engine->SetState(shopping_confirm,UI_STATE_BUTTON_NORMAL);
	if (Engine->Player.KnightCash < shopping_p)
		Engine->SetState(shopping_confirm,UI_STATE_BUTTON_DISABLE);

	Engine->SetEditString(gift_edit_name, "");

	TABLE_ITEM_BASIC* tbl = Engine->tblMgr->getItemData(shopping_id);
	if (tbl != nullptr)
	{

		if (TABLE_ITEM_EXT* ext = Engine->tblMgr->getExtData(tbl->extNum, shopping_id))
		{
			DWORD dwIconID = tbl->dxtID;
			if (ext->dxtID > 0)
				dwIconID = ext->dxtID;

			std::string dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);

			Engine->SetTexImage(shopping_icon, dxt);
		}
		else {
			std::vector<char> buffer(256, NULL);

			sprintf(&buffer[0], "ui\\itemicon_%.1d_%.4d_%.2d_%.1d.dxt",
				(tbl->dxtID / 10000000),
				(tbl->dxtID / 1000) % 10000,
				(tbl->dxtID / 10) % 100,
				tbl->dxtID % 10);

			std::string szIconFN = &buffer[0];
			Engine->SetTexImage(shopping_icon, szIconFN);
		
		}

		Engine->SetString(shopping_name,tbl->strName);
		Engine->SetString(shopping_price,string_format(xorstr("Price: %s"), Engine->StringHelper->NumberFormat(shopping_p).c_str()));
		Engine->SetString(shopping_quantity,string_format(xorstr("Quantity: %s"), Engine->StringHelper->NumberFormat(itemscount).c_str()));
		Engine->SetString(shopping_after,string_format(xorstr("%d Knight Cash"), shopping_p));
	}
	Engine->SetVisible(shopping_confirm,true);
}

uint32_t CUIPowerUpStore::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	{
		bool showTooltip = false;
		bool pass = false;

	
		if (Engine->IsIn(shopping_confirm,ptCur.x, ptCur.y) && Engine->IsVisible(shopping_confirm))
			pass = true;

		if (!pass)
		{
			for (auto it : pus_items)
			{
#if(PUS_ITEMSCOUNT == 4)
				if (it.icon != NULL && it.sItemID > 0 && Engine->IsVisible(itemGroupBase[0]) && Engine->IsVisible(itemGroupBase[0]) && Engine->IsVisible(itemGroupBase[2]) && Engine->IsVisible(itemGroupBase[3]))
#else
				if (it.icon != NULL && it.sItemID > 0 && Engine->IsVisible(itemGroupBase[0]) && Engine->IsVisible(itemGroupBase[1]) && Engine->IsVisible(itemGroupBase[2]))
#endif
				{
					if (Engine->IsIn(it.icon,ptCur.x, ptCur.y) && Engine->IsVisible(it.nBase))
					{
						Engine->ShowToolTipEx(it.sItemID, ptCur.x, ptCur.y);
						break;
					}
				}
			}

			for (auto& it : refund_items)
			{
				if (it.icon != NULL && it.sItemID > 0 && Engine->IsVisible(it.nBase) && Engine->IsIn(it.icon, ptCur.x, ptCur.y))
				{
					Engine->ShowToolTipEx(it.sItemID, ptCur.x, ptCur.y);
					break;
				}
			}
		}
	}
	
	return dwRet;
}

void CUIPowerUpStore::Open()
{
	Engine->SetEditString(txt_code, "");
	Engine->SetEditString(txt_search, "");
	Engine->SetVisible(txt_search, true);

	Engine->UIScreenCenter(m_dVTableAddr);
	Engine->SetString(txt_cash,Engine->StringHelper->NumberFormat(Engine->Player.KnightCash).c_str());
	Engine->SetString(txt_tlbalance,Engine->StringHelper->NumberFormat(Engine->Player.TlBalance).c_str());
	SetItems();

	UpdateRefundItemList(Engine->m_UiMgr->pusrefund_itemlist);

	for (int i = 0; i < cat_list.size(); i++)
	{
		DWORD tmp = Engine->GetChildByID(btn_tabs[i], string_format(xorstr("txt")));
		if (cat_list[i].Status)
		{
			Engine->SetString(tmp, cat_list[i].categoryName.c_str());
			Engine->SetVisible(btn_tabs[i],true);
		}
		else
			Engine->SetVisible(btn_tabs[i],false);
	}

	Engine->SetVisible(m_dVTableAddr,true);
}

void CUIPowerUpStore::setBaseItems(int value) {
	return;
	for (int i = 1; i <= PUS_ITEMSCOUNT; i++)
	{
		Engine->SetVisible(itemGroupBase[i - 1], (value==1 ? true : false));
	}
}
void CUIPowerUpStore::Close()
{
	Engine->SetVisible(m_dVTableAddr,false);
	Engine->SetEditString(edit_search, "");
	Engine->SetVisible(txt_search, false);
	Engine->EditKillFocus(edit_search);
	Engine->EditKillFocus(txt_code);
	Engine->EditKillFocus(gift_edit_name);
	/*edit_search->KillFocus();
	edit_search->SetString("");
	SetVisible(false);
	m_bNeedToRestore = false;*/
}

void CUIPowerUpStore::UpdateItemList(vector<PUSItem> items, vector<PusCategory> cats)
{
	item_list = items;
	cat_list = cats;
	page = 1;
	pageCount = abs(ceil((double)item_list.size() / (double)PUS_ITEMPERCOUNT));
}

void CUIPowerUpStore::UpdateRefundItemList(vector<PUSREFUNDITEM> refund_items) { refund_list = refund_items; }

bool pus_refund(const PUSREFUNDITEM& a, const PUSREFUNDITEM& b)
{
	return a.expiredtime < b.expiredtime;
}

void CUIPowerUpStore::RefundSetItems(bool dontopen) {
	vector<PUSREFUNDITEM> tmpList;
	for (int i = 0; i < (int)refund_list.size(); i++)  tmpList.push_back(refund_list[i]);

	if (tmpList.empty()) {
		for (int i = 0; i < 18; i++) Engine->SetVisible(refund_items[i].nBase,false);
		return;
	}

	std::sort(tmpList.begin(), tmpList.end(), pus_refund);

	int count = 0, size = (int)tmpList.size();
	for (int i = 0; i < 18; i++) {

		if (++count > 18 || size == 0) break;
		if (i >= size) {
			Engine->SetVisible(refund_items[i].nBase,false);
			continue;
		}

		TABLE_ITEM_BASIC* tbl = Engine->tblMgr->getItemData(tmpList[i].itemid);
		if (tbl == nullptr) {
			Engine->SetVisible(refund_items[i].nBase,false);
			continue;
		}
		Engine->SetVisible(basepusrefund, !dontopen);
		if (TABLE_ITEM_EXT* ext = Engine->tblMgr->getExtData(tbl->extNum, tmpList[i].itemid)) {
			DWORD dwIconID = tbl->dxtID;
			if (ext->dxtID > 0) dwIconID = ext->dxtID;
			std::string dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);


			Engine->SetTexImage(refund_items[i].icon, dxt);
		}
		else {
			std::vector<char> buffer(256, NULL);

			sprintf(&buffer[0], "ui\\itemicon_%.1d_%.4d_%.2d_%.1d.dxt", (tbl->dxtID / 10000000), (tbl->dxtID / 1000) % 10000,
				(tbl->dxtID / 10) % 100, tbl->dxtID % 10);

			std::string szIconFN = &buffer[0];

			Engine->SetTexImage(refund_items[i].icon, szIconFN);
		}
		

		refund_items[i].sItemID = tmpList[i].itemid;
		refund_items[i].serial = tmpList[i].serial;
		Engine->SetString(refund_items[i].name,tbl->strName);
		Engine->SetString(refund_items[i].price,string_format(xorstr("Price: %s"), Engine->StringHelper->NumberFormat(tmpList[i].kcprice).c_str()));
		Engine->SetVisible(refund_items[i].nBase,true);
	}
}

void CUIPowerUpStore::SetItems(int p, int cat, string q)
{
	vector<PUSItem> tmpList;
	for (int i = 0; i < item_list.size(); i++)
	{
		if (q == "")
		{
			if (item_list[i].Category == cat)
				tmpList.push_back(item_list[i]);
		}
		
		else if (q != "")
		{
				TABLE_ITEM_BASIC* tbl = Engine->tblMgr->getItemData(item_list[i].ItemID);
				if (tbl != nullptr)
				{
					if (Engine->StringHelper->IsContains(tbl->strName, q))
						tmpList.push_back(item_list[i]);
				}
			
		}
		else
			tmpList.push_back(item_list[i]);
	}

	if (tmpList.size() == 0)
	{
		for (int i = 0; i < PUS_ITEMPERCOUNT; i++)
			Engine->SetVisible(pus_items[i].nBase,false);

		Engine->SetString(txt_page,to_string(page));
		pageCount = 1;
		return;
	}

	std::sort(tmpList.begin(), tmpList.end(), pus_compare);

	pageCount = abs(ceil((double)tmpList.size() / (double)PUS_ITEMPERCOUNT));

	int begin = (p - 1) * PUS_ITEMPERCOUNT;
	int j = -1;
	for (int i = begin; i < begin + PUS_ITEMPERCOUNT; i++)
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
			Engine->SetVisible(pus_items[j].nBase,false);
			continue;
		}

		TABLE_ITEM_BASIC* tbl = Engine->tblMgr->getItemData(tmpList[i].ItemID);
		if (tbl == nullptr)
		{
			Engine->SetVisible(pus_items[j].nBase,false);
			continue;
		}

		if (TABLE_ITEM_EXT* ext = Engine->tblMgr->getExtData(tbl->extNum, tmpList[i].ItemID))
		{
			DWORD dwIconID = tbl->dxtID;
			if (ext->dxtID > 0)
				dwIconID = ext->dxtID;

			std::string dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);
			Engine->SetTexImage(pus_items[j].icon, dxt);
		
		}
		else {
			std::vector<char> buffer(256, NULL);

			sprintf(&buffer[0], "ui\\itemicon_%.1d_%.4d_%.2d_%.1d.dxt",
				(tbl->dxtID / 10000000),
				(tbl->dxtID / 1000) % 10000,
				(tbl->dxtID / 10) % 100,
				tbl->dxtID % 10);

			std::string szIconFN = &buffer[0];
			Engine->SetTexImage(pus_items[j].icon, szIconFN);
		}

		pus_items[j].sItemID = tmpList[i].ItemID;
		Engine->SetString(pus_items[j].name,tbl->strName);
		Engine->SetString(pus_items[j].price,string_format(xorstr("%s %s"), Engine->StringHelper->NumberFormat(tmpList[i].Price).c_str(), (tmpList[i].isTlBalance == 1 ? "TL" : "KC")));
		Engine->SetString(pus_items[j].quantitiy,string_format(xorstr("%s"), Engine->StringHelper->NumberFormat(tmpList[i].Quantitiy).c_str()));

		bool enoughtMoney = true;
		bool enoughtTLBalance = true;
		if (Engine->Player.KnightCash < tmpList[i].Price)
			enoughtMoney = false;
		if (Engine->Player.TlBalance < tmpList[i].Price)
			enoughtTLBalance = false;
		if (!tmpList[i].isTlBalance)
		{
			if (enoughtMoney)
			{
				//pus_items[j].price->SetColor(D3DCOLOR_RGBA(146, 252, 175, 255));
				Engine->SetStringColor(pus_items[j].price,D3DCOLOR_RGBA(255, 0, 0, 255));
				Engine->SetState(pus_items[j].purchase,UI_STATE_BUTTON_NORMAL);
			}
			else
			{
				//pus_items[j].price->SetColor(D3DCOLOR_RGBA(247, 141, 173, 255));
				Engine->SetStringColor(pus_items[j].price, D3DCOLOR_RGBA(255, 0, 0, 255));
			
				Engine->SetState(pus_items[j].purchase, UI_STATE_BUTTON_DISABLE);
			}
		}
		else {
			if (enoughtTLBalance)
			{
				//pus_items[j].price->SetColor(D3DCOLOR_RGBA(146, 252, 175, 255));
				Engine->SetStringColor(pus_items[j].price, D3DCOLOR_RGBA(255, 0, 0, 255));
				Engine->SetState(pus_items[j].purchase, UI_STATE_BUTTON_NORMAL);
			
			}
			else
			{
				//pus_items[j].price->SetColor(D3DCOLOR_RGBA(247, 141, 173, 255));
				Engine->SetStringColor(pus_items[j].price, D3DCOLOR_RGBA(255, 0, 0, 255));

				Engine->SetState(pus_items[j].purchase, UI_STATE_BUTTON_DISABLE);
			}

		}
		Engine->SetVisible(pus_items[j].nBase,true);

	}
	Engine->SetString(txt_page,to_string(page));
}

void CUIPowerUpStore::UpdateCash(uint32 kc, uint32 tl)
{
	Engine->SetString(txt_cash, Engine->StringHelper->NumberFormat(kc).c_str());
	Engine->SetString(txt_tlbalance, Engine->StringHelper->NumberFormat(tl).c_str());

}

bool pus_compare(const PUSItem& a, const PUSItem& b)
{
	return a.ID < b.ID;
}

void CUIPowerUpStore::OpenPowerUpStore()
{

}

void CUIPowerUpStore::PurchaseItemAdd(uint32 ItemID, uint32 Price, uint8 PriceType)
{
	for (int i = 0; i < 7; i++)
	{
		if (Engine->IsVisible(purchase_items[i].nBase))
			continue;

		TABLE_ITEM_BASIC* tbl = Engine->tblMgr->getItemData(ItemID);
		if (tbl != nullptr)
		{

			if (TABLE_ITEM_EXT* ext = Engine->tblMgr->getExtData(tbl->extNum, shopping_id))
			{
				DWORD dwIconID = tbl->dxtID;
				if (ext->dxtID > 0)
					dwIconID = ext->dxtID;

				std::string dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);

				Engine->SetTexImage(purchase_items[i].icon, dxt);
				Engine->SetString(purchase_items[i].name,tbl->strName);
				purchase_items[i].sItemID = ItemID;
				purchase_items[i].sCash = Price;

				std::string ncash = string_format("%s %s", Engine->StringHelper->NumberFormat(Price).c_str(), (PriceType == 1 ? "TL" : " KnightCash"));
				Engine->SetString(purchase_items[i].price,ncash);
				Engine->SetVisible(purchase_items[i].nBase,true);
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

				Engine->SetTexImage(purchase_items[i].icon, szIconFN);
				Engine->SetString(purchase_items[i].name, tbl->strName);
				purchase_items[i].sItemID = ItemID;
				purchase_items[i].sCash = Price;

				std::string ncash = string_format("%s %s", Engine->StringHelper->NumberFormat(Price).c_str(), (PriceType == 1 ? "TL" : " KnightCash"));
				Engine->SetString(purchase_items[i].price,ncash);
				Engine->SetVisible(purchase_items[i].nBase,true);
			}

			break;
		}
	}

	PurchaseTotal();
}

void CUIPowerUpStore::PurchaseButtonClick(DWORD pSender)
{
	for (int i = 0; i < 7; i++)
	{
		if (purchase_items[i].cancel != NULL)
		{
			if (pSender ==purchase_items[i].cancel)
			{
				purchase_items[i].sItemID = 0;
				purchase_items[i].sCash = 0;
				Engine->SetVisible(purchase_items[i].nBase,false);
				PurchaseTotal();
				break;
			}
		}
	}
}

void CUIPowerUpStore::PurchaseTotal()
{
	uint32 totalprice = 0;

	for (int i = 0; i < 7; i++)
	{
		if (Engine->IsVisible(purchase_items[i].nBase))
			totalprice += purchase_items[i].sCash;
	}

	Engine->SetString(total_cash,string_format(xorstr("%s"), Engine->StringHelper->NumberFormat(totalprice).c_str()));
}

void CUIPowerUpStore::PurchaseDeleteAll()
{
	for (int i = 0; i < 7; i++)
	{
		purchase_items[i].sItemID = 0;
		purchase_items[i].sCash = 0;
		purchase_items[i].type = 0;
		Engine->SetVisible(purchase_items[i].nBase,false);
	}

	PurchaseTotal();
}

void CUIPowerUpStore::PurchaseBuyAll()
{
	Engine->SetVisible(basket_confirm,false);

	for (int i = 0; i < 7; i++)
	{
		if (Engine->IsVisible(purchase_items[i].nBase) && purchase_items[i].sItemID > 0)
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

void CUIPowerUpStore::BasketMenuOpen()
{
	uint32 totalprice = 0;

	for (int i = 0; i < 7; i++)
	{
		if (Engine->IsVisible(purchase_items[i].nBase))
			totalprice += purchase_items[i].sCash;
	}

	Engine->SetString(basket_cash,string_format(xorstr("%s Knight Cash"), Engine->StringHelper->NumberFormat(totalprice).c_str()));
	Engine->SetVisible(basket_confirm,true);
}