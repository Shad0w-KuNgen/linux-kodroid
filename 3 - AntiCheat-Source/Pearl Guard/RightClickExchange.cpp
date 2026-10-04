#include "RightClickExchange.h"

DWORD RightExchangevTable = 0;
DWORD Func_RightExchange;

extern __inline DWORD RDWORD(DWORD ulBase);
void __fastcall ItemMove(DWORD ItemSkillIcon)
{
	if (ItemSkillIcon && !IsBadReadPtr((VOID*)(ItemSkillIcon + 0x68), sizeof(DWORD)))
	{
		DWORD extBase = RDWORD(ItemSkillIcon + 0x6C);

		DWORD ID = RDWORD(RDWORD(ItemSkillIcon + 0x68));
		DWORD EXT = RDWORD(extBase);

		uint32 nItemID = ID + EXT;
		if (Engine->hkRightClickExchange)
		{
			auto itr = Engine->hkRightClickExchange->RightClickItemList;
			if (std::find(itr.begin(), itr.end(), nItemID) != itr.end())
			{
				Packet result(XSafe, uint8(XSafeOpCodes::RIGHT_CLICK_EXCHANGE));
				result << uint8(RightClickExchangeSubcode::GetCurrentItemList) << uint32(nItemID);
				Engine->Send(&result);
			}
			else if (std::find(std::begin(ItemOfGemArray), std::end(ItemOfGemArray), nItemID) != std::end(ItemOfGemArray))
			{
				Packet result(XSafe, uint8(XSafeOpCodes::RIGHT_CLICK_EXCHANGE));
				result << uint8(RightClickExchangeSubcode::GetExchangeInformation) << uint32(nItemID);
				Engine->Send(&result);
				}
			else if (std::find(std::begin(ChestOfArray), std::end(ChestOfArray), nItemID) != std::end(ChestOfArray))
			{
				Packet result(XSafe, uint8(XSafeOpCodes::RIGHT_CLICK_EXCHANGE));
				result << uint8(RightClickExchangeSubcode::GetExchangeChestInformation) << uint32(nItemID);
				Engine->Send(&result);
			}
		}
	}
}
const DWORD KO_INVENTORY_ITEM_MOVE = 0x006012B5;
void __declspec(naked) hkInventMove()
{
	__asm {
		pushad
		pushfd
		cmp eax, -256
		jne f_continue

		mov ecx, ebx
		call ItemMove
		f_continue :
		popfd
			popad
			pop ecx
			pop edi
			pop esi
			pop ebp
			pop ebx
			mov edx, KO_INVENTORY_ITEM_MOVE
			add edx, 5
			push edx
			ret
	}
}
RightClickExchange::RightClickExchange()
{
	vector<int>offsets;
	offsets.push_back(0x2A0);
	offsets.push_back(0x0);
	el_Base = Engine->rdword(KO_DLG, offsets);
	RightExchangevTable = el_Base;
	nBaseItemID = 0;
	ParseUIElements();
	InitReceiveMessage();
	posSet = false;
	m_Timer = new CTimer();
	DetourFunction((PBYTE)KO_INVENTORY_ITEM_MOVE, (PBYTE)hkInventMove);
	bCurrentPage = 1;
	ExchangeItemStatus = false;
}

RightClickExchange::~RightClickExchange()
{

}

void RightClickExchange::RightClickExchangePacket(Packet pkt)
{
	uint8 bOpCode = pkt.read<uint8>();

	switch (bOpCode)
	{
	case RightClickExchangeSubcode::GetItemList:
	{
		RightClickItemList.clear();
		uint8 bCount = pkt.read<uint8>();

		for (int i = 0; i < bCount; i++)
		{
			uint32 itemID = pkt.read<uint32>();
			RightClickItemList.push_back(itemID);
		}
		break;
	}

	case RightClickExchangeSubcode::GetCurrentItemList:
	{
		for (int i = 0; i < 25; i++)
		{
			uint32 nItemID;
			uint32 nRentalTime;
			pkt >> nItemID >> nRentalTime;
			if (nItemID == 0)
			{
				Engine->SetVisible(m_Icon[i]->icon, false);
				Engine->SetUIStyle(m_Icon[i]->icon, 0x20);
				continue;
			}

			Engine->SetTexImage(m_Icon[i]->icon, Engine->GetItemDXT(nItemID));
			m_Icon[i]->nItemID = nItemID;
			m_Icon[i]->nRentalTime = nRentalTime;
			Engine->SetVisible(m_Icon[i]->icon, true);
			Engine->SetUIStyle(m_Icon[i]->icon, 0x20);
		}
		nBaseItemID = pkt.read<uint32>();
		CItem* pData = (CItem*)Engine->tblMgr->getItemData(nBaseItemID);
		if (!pData)
			break;
		Engine->SetString(txt_title, "Right Click Exchange");
		Engine->SetTexImage(base_itemicon, Engine->GetItemDXT(nBaseItemID));
		Engine->SetVisible(base_itemicon, true);
		SectionMode = pkt.read<bool>();
		SelectItem = 0;
		Open();
		Engine->SetVisible(btn_exchange, true);
	}
	break;
	case RightClickExchangeSubcode::GetExchangeMessage:
	{
		uint8 bSucces = pkt.read<uint8>();

		if (bSucces)
		{
			Engine->WriteInfoMessage("RightClick Exchange successfully.", 0x009700);
			this->Close();
		}
		else if (!bSucces)
			Engine->WriteInfoMessage("RightClick Exchange failed.", 0xBA0F30);

		SelectItem = 0;

	}
	case RightClickExchangeSubcode::SendItemExchangeList:
	{
		uint16 nCount = pkt.read<uint16>();
		for (int i = 0; i < nCount;i++)
			Engine->hkRightClickExchange->RightClickItemList.push_back(pkt.read<uint32>());

	}
	break;
	case RightClickExchangeSubcode::SendExchangeInformation:
	{
		uint32 nExchangeGemID;
		uint16 nListSize;
		ExchangeItemStatus = true;
		GetExchangeList.clear();
		pkt >> nExchangeGemID >> nListSize;
		for (int i = 0; i < nListSize;i++)
		{
			ExchangeInformation* apInfo = new ExchangeInformation();
			pkt >> apInfo->nItemID >> apInfo->sPercent;
			if (apInfo->nItemID == 0)
			{
				delete apInfo;
				continue;
			}
			GetExchangeList.push_back(apInfo);
		}

		bPageCount = abs(ceil((double)GetExchangeList.size() / (double)PAGE_COUNT));
		Open();
		Engine->SetTexImage(base_itemicon,Engine->GetItemDXT(nExchangeGemID));
		GetExchangePage(1);
		bCurrentPage = 1;
		Engine->SetVisible(btn_exchange,false);
	}
	break;
	case RightClickExchangeSubcode::SendExchangeChestInformation:
	{
		uint32 nExchangeGemID;
		uint16 nListSize;
		ExchangeItemStatus = true;
		GetExchangeList.clear();
		pkt >> nExchangeGemID >> nListSize;
		for (int i = 0; i < nListSize;i++)
		{
			ExchangeInformation* apInfo = new ExchangeInformation();
			pkt >> apInfo->nItemID >> apInfo->sPercent;
			if (apInfo->nItemID == 0)
			{
				delete apInfo;
				continue;
			}
			//printf("Read : %d Percent : %d \n", apInfo->nItemID, apInfo->sPercent);
			GetExchangeList.push_back(apInfo);
		}

		bPageCount = abs(ceil((double)GetExchangeList.size() / (double)PAGE_COUNT));

		Open();
		Engine->SetTexImage(base_itemicon,Engine->GetItemDXT(nExchangeGemID));
		GetExchangePage(1);
		bCurrentPage = 1;
		Engine->SetVisible(btn_exchange,false);

	}
	break;
	default:
		break;
	}


}
void RightClickExchange::GetExchangePage(uint8 bPage)
{
	if (bPage > bPageCount) return;
	Engine->SetString(page,std::to_string(bPage));
	Engine->SetVisible(btn_down,true);
	Engine->SetVisible(btn_next,true);
	Engine->SetVisible(page,true);
	int begin = (bPage - 1) * PAGE_COUNT;
	for (int i = 0; i < 25; i++)
	{
		Engine->SetVisible(m_Icon[i]->icon,false);
		Engine->SetUIStyle(m_Icon[i]->icon,0x20);
		Engine->SetVisible(percent[i],false);
	}
	for (int i = begin; i < begin + PAGE_COUNT; i++)
	{
		if (i >= GetExchangeList.size()) // maxsize control
			continue;
		ExchangeInformation* pInfo = GetExchangeList[i];
		Engine->SetTexImage(m_Icon[i - begin]->icon, Engine->GetItemDXT(pInfo->nItemID));
		float per = (float)pInfo->sPercent / 100;
		Engine->SetString(percent[i - begin],string_format(xorstr("%.2f%%"), per));
		Engine->SetVisible(percent[i - begin],true);
		Engine->SetVisible(m_Icon[i - begin]->icon,true);
		m_Icon[i - begin]->nItemID = GetExchangeList[i]->nItemID;
		m_Icon[i - begin]->nRentalTime = 0;
		Engine->SetUIStyle(m_Icon[i - begin]->icon,0x20);
	}
}
void RightClickExchange::ParseUIElements()
{

	std::string find = xorstr("txt_title");
	txt_title = Engine->GetChildByID(el_Base, find);
	find = xorstr("btn_exchange");
	btn_exchange = Engine->GetChildByID(el_Base, find);
	find = xorstr("grp_percents");
	percent_base = Engine->GetChildByID(el_Base, find);
	find = xorstr("click_area");
	click_area = Engine->GetChildByID(el_Base, find);
	find = xorstr("btn_close");
	btn_close = Engine->GetChildByID(el_Base, find);
	find = xorstr("grp_selected");
	grp_selected = Engine->GetChildByID(el_Base, find);
	find = xorstr("base_item");
	base_item = Engine->GetChildByID(el_Base, find);
	find = xorstr("page");
	page = Engine->GetChildByID(el_Base, find);
	find = xorstr("btn_next");
	btn_next = Engine->GetChildByID(el_Base, find);
	find = xorstr("btn_down");
	btn_down = Engine->GetChildByID(el_Base, find);
	Engine->SetString(page, "1");
	Engine->SetVisible(btn_down, false);
	Engine->SetVisible(page, false);
	find = xorstr("area_slot");
	base_item_area = Engine->GetChildByID(base_item, find);
	for (int i = 1; i <= 25;i++)
	{
		percent[i - 1] = Engine->GetChildByID(percent_base,string_format(xorstr("percent_%d"), i));
		Engine->SetVisible(percent[i - 1],false);
		find = string_format(xorstr("item_%d"), i);
		ItemBase[i - 1] = Engine->GetChildByID(el_Base, find);
		find = xorstr("area_slot");
		area_slot[i - 1] = Engine->GetChildByID(ItemBase[i - 1], find);;
		DWORD bCreateIcon = 0x00;
		if (!Engine->_InitIcon(bCreateIcon, area_slot[i - 1], 910014000))
		continue;
		ItemInfo* inf = new ItemInfo();
		inf->icon = bCreateIcon;
		inf->nItemID = 0;
		Engine->SetRegion(inf->icon, area_slot[i - 1]);
		Engine->SetVisible(inf->icon, true);
		Engine->SetVisible(bCreateIcon, false);
		m_Icon.push_back(inf);
	}

	DWORD baseCreateIcon=0x00;
	if (!Engine->_InitIcon(baseCreateIcon, base_item_area, 910014000))
		printf("Error Code : 1 \n");
	base_itemicon = baseCreateIcon;
	Engine->SetRegion(baseCreateIcon, base_item_area);
	Engine->SetVisible(baseCreateIcon, true);
}

bool RightClickExchange::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	RightExchangevTable = el_Base;
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == (DWORD*)btn_close)
			Close();
		else if (pSender == (DWORD*)btn_exchange)
		{
			Packet result(XSafe, uint8(XSafeOpCodes::RIGHT_CLICK_EXCHANGE));
			result << uint8(RightClickExchangeSubcode::SendItemExchange) << uint32(nBaseItemID);
			SectionMode ? result << uint32(SelectItem) : result << uint32(0);
			Engine->Send(&result);
			return true;
		}
		else if (pSender == (DWORD*)btn_down)
		{
			if (bCurrentPage > 1)
			{
				bCurrentPage--;
				GetExchangePage(bCurrentPage);
			}
		}
		else if (pSender == (DWORD*)btn_next)
		{
			if (bPageCount > 1 && bCurrentPage < bPageCount)
			{
				bCurrentPage++;
				GetExchangePage(bCurrentPage);
			}
		}
	}

	return true;
}

uint32_t RightClickExchange::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (Engine->IsIn(click_area,ptCur.x,ptCur.y))
	{
		foreach(itr, m_Icon)
		{
			ItemInfo* pData = (*itr);
			if (!Engine->IsVisible(pData->icon) || !Engine->IsIn(pData->icon, ptCur.x, ptCur.y))
				continue;

			Engine->ShowToolTipEx(pData->nItemID, ptCur.x, ptCur.y);

		}
		if ((dwFlags == UI_MOUSE_RBCLICKED || dwFlags == UI_MOUSE_LBCLICKED) && SectionMode)
		{

			for (int i = 0; i < 25;i++)
				Engine->SetUIStyle(m_Icon[i]->icon, 0x20);
			foreach(itr, m_Icon)
			{
				ItemInfo* pData = (*itr);
				if (!Engine->IsVisible(pData->icon) || !Engine->IsIn(pData->icon, ptCur.x, ptCur.y))
					continue;
				SelectItem = pData->nItemID;
				Engine->SetUIStyle(pData->icon, 0x00400000);
		
				break;
			}
		}
	}
	return dwRet;
}
void RightClickExchange::Open()
{
	
	Engine->SetVisible(el_Base, true);
	Engine->UIScreenCenter(el_Base);

}

void RightClickExchange::Close()
{
	Engine->SetVisible(el_Base, false);
}

DWORD oRecvMsgRightClickExchange;
void __stdcall UiRightExchangeReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->hkRightClickExchange->ReceiveMessage(pSender, dwMsg);

	__asm
	{
		MOV ECX, RightExchangevTable
		PUSH dwMsg
		PUSH pSender
		call oRecvMsgRightClickExchange
	}
}
void RightClickExchange::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(el_Base);
	oRecvMsgRightClickExchange = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)UiRightExchangeReceiveMessage_Hook;
}