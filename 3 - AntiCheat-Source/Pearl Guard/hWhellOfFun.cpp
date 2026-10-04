#include "stdafx.h"
#include "hWhellOfFun.h"

CUIWheelOfFun::CUIWheelOfFun()
{
	vector<int>offsets;
	offsets.push_back(0x338);    // co_rookietip Satýr : 107	
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);
	lastItem = 0;
	KnightCash = NULL;
	base_item = NULL;
	btn_start = NULL;
	btn_close = NULL;
	btn_stop = NULL;
	base_item = NULL;
	TickSay = clock();
	isActiveWhile = false;
	ParseUIElements();
	InitReceiveMessage();
}

CUIWheelOfFun::~CUIWheelOfFun()
{
}

void CUIWheelOfFun::ParseUIElements()
{
	std::string find = xorstr("base_item");


	char szBuf[64];
	for (int i = 0; i < 15; i++)
	{
		nItem[i] = Engine->GetChildByID(m_dVTableAddr, string_format(xorstr("item%.2d"), i));
		/*sprintf(szBuf, "item%.2d", i);
		Engine->GetChildByID(m_dVTableAddr, szBuf, nItem[i]);*/
	}

	btn_stop = Engine->GetChildByID(m_dVTableAddr, "btn_stop");
	btn_start = Engine->GetChildByID(m_dVTableAddr, "btn_start");
	btn_close = Engine->GetChildByID(m_dVTableAddr, "btn_close");
	KnightCash = Engine->GetChildByID(m_dVTableAddr, "txt_cash");
}
void CUIWheelOfFun::ImageChange(int8 sID)
{
	auto it = Engine->WheelItemData.find(sID);
	if (it == Engine->WheelItemData.end())
		return;

	Engine->SetTexImage(nItem[sID],Engine->GetItemDXT(it->second));
	Engine->SetTexImage(nItem[lastItem],"ui\\itemicon_noimage.dxt");
	lastItem = sID;
}
void CUIWheelOfFun::StartChallange()
{
	for (int i = 0; i < 15; i++)
		Engine->SetTexImage(nItem[i],"ui\\itemicon_noimage.dxt");

	Engine->SetState(btn_start,UI_STATE_BUTTON_DISABLE);
	Engine->SetState(btn_stop,UI_STATE_BUTTON_NORMAL);
	isActiveWhile = true;

}
void CUIWheelOfFun::OpenWheel()
{
	Engine->SetString(KnightCash, string_format("Knight Cash : %d", Engine->uiState->m_iCash).c_str());
	//Engine->SetString(KnightCash, Engine->StringHelper->NumberFormat(Engine->uiState->m_iCash).c_str());

	Engine->SetVisible(m_dVTableAddr, true);
	Engine->UIScreenCenter(m_dVTableAddr);
	for (int i = 0; i < 15; i++)
	{
		auto it = Engine->WheelItemData.find(i);
		if (it == Engine->WheelItemData.end())
			return;

		Engine->SetTexImage(nItem[i], Engine->GetItemDXT(it->second));

	}
}
uint32_t CUIWheelOfFun::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{

	
	if (!Engine->IsVisible(m_dVTableAddr)) return false;
	uint32_t dwRet = UI_MOUSEPROC_NONE;

		bool showTooltip = false;

		int i = 0;
		for (auto it : Engine->WheelItemData)
		{
			if (i >= Engine->WheelItemData.size())
				break;


			if (Engine->IsVisible(nItem[i])  && Engine->IsIn(nItem[i],ptCur.x, ptCur.y))
			{
		/*		if (Engine->str_contains(nItem[i]->GetTexFN().c_str(), xorstr("itemicon_noimage.dxt")))
					continue;*/

				//printf("%s \n", nItem[i]->GetTexFN().c_str());
				TABLE_ITEM_BASIC* item = Engine->tblMgr->getItemData(it.second);
				if (!item) {
					i++;
					continue;
				}
				Engine->ShowToolTipEx(it.second, ptCur.x, ptCur.y);
				showTooltip = true;
				break;
			}
			i++;
		}

	return dwRet;

}

void CUIWheelOfFun::GiveItemImage(uint32 nItemID)
{
	auto it = Engine->WheelItemData.begin();
	while (it != Engine->WheelItemData.end())
	{
		if (it->second == nItemID)
		{
			for (int i = 0; i < 15; i++)
				Engine->SetTexImage(nItem[i],"ui\\itemicon_noimage.dxt"); // hepsini gizle

			Engine->SetTexImage(nItem[it->first],Engine->GetItemDXT(it->second)); // çýkan itemi göster

			break;
		}

		it++;
	}

}
void CUIWheelOfFun::StopChallange()
{
	Engine->SetState(btn_start,UI_STATE_BUTTON_NORMAL);
	Engine->SetState(btn_stop,UI_STATE_BUTTON_DISABLE);
	isActiveWhile = false;
}

void CUIWheelOfFun::IsActive(bool enable)
{
	Engine->SetVisible(m_dVTableAddr, enable);

	if (!isActiveWhile)
	{
		for (int i = 0; i < 15; i++)
		{
			auto it = Engine->WheelItemData.find(i);
			if (it == Engine->WheelItemData.end())
				return;

			Engine->SetTexImage(nItem[i],Engine->GetItemDXT(it->second));

		}


	}
	if (enable)
	{
		
		Engine->SetString(KnightCash,string_format("Knight Cash : %d", Engine->uiState->m_iCash).c_str());
		Engine->UIScreenCenter(m_dVTableAddr);

	}
}
int myrands(int a, int b)
{
	int div = b - a;
	return a + (rand() % (b - a));
}
void CUIWheelOfFun::Tick()
{
	if (!Engine->IsVisible(m_dVTableAddr) || !isActiveWhile)
		return;

	if (TickSay < clock() - 120)
	{
		TickSay = clock();

		int8 Rand = myrands(0, 24);

		ImageChange(Rand);
	}

}
DWORD Wheelui;
DWORD Func_Wheel;


bool CUIWheelOfFun::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	Wheelui = m_dVTableAddr;

	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == (DWORD*)btn_start)
			StartChallange();
		else if (pSender ==(DWORD*)btn_stop)
		{
			Packet pkt(XSafe, uint8_t(XSafeOpCodes::WheelData));
			pkt << uint8(1);
			Engine->Send(&pkt);
		}
		else if (pSender == (DWORD*)btn_close)
			IsActive(false);
	}

	return true;
}

void __stdcall UiWheelReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiWheel->ReceiveMessage(pSender, dwMsg);

	__asm
	{
		MOV ECX, Wheelui
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_Wheel
		CALL EAX
	}
}

void CUIWheelOfFun::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_Wheel = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)UiWheelReceiveMessage_Hook;
}