#include "DropResult.h"


CDropResult::CDropResult()
{
	vector<int>offsets;
	offsets.push_back(0x518);  //re_caht_main Satýr : 217
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);

	txt_title = NULL;
	btn_close = NULL;
	for (int i = 0; i < 5; i++) {
		dropitems[i].icon = NULL;
		dropitems[i].nItemID = 0;
		dropitems[i].tbl = nullptr;
	}
	ParseUIElements();
	InitReceiveMessage();
}

CDropResult::~CDropResult()
{

}

void CDropResult::ParseUIElements()
{
	txt_title = Engine->GetChildByID(m_dVTableAddr, "txt_title");
	btn_close = Engine->GetChildByID(m_dVTableAddr, "btn_close");

	for (int i = 1; i <= 5; i++)
	{
		dropitems[i-1].icon = Engine->GetChildByID(m_dVTableAddr, string_format(xorstr("itemicon%d"), i));
	}
	Engine->UIScreenCenter(m_dVTableAddr);
}

DWORD uiBassse;
DWORD Func_Drop = 0;
bool CDropResult::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	uiBassse = m_dVTableAddr;
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	std::string msg = "";
	if (pSender == (DWORD*)btn_close)
		Close();

	return true;
}


void __stdcall UiDropMainReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiDropResult->ReceiveMessage(pSender, dwMsg);
	__asm
	{
		MOV ECX, uiBassse
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_Drop
		CALL EAX
	}
}

void CDropResult::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_Drop = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)UiDropMainReceiveMessage_Hook;
}

uint32_t CDropResult::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	bool showTooltip = false;

	for (int i = 0; i < 5; i++)
	{
		if (dropitems[i].icon != NULL) {
			if (Engine->IsIn(dropitems[i].icon, ptCur.x, ptCur.y) && dropitems[i].tbl != nullptr && dropitems[i].nItemID != 0)
			{
				Engine->ShowToolTipEx(dropitems[i].nItemID, ptCur.x, ptCur.y);
				break;
			}
		}
	}

	return dwRet;
}

void CDropResult::Open()
{
	Engine->SetVisible(m_dVTableAddr, true);
}

void CDropResult::Close()
{
	Engine->SetVisible(m_dVTableAddr,false);
}

void CDropResult::SetTitle(std::string text)
{
	Engine->SetString(txt_title,text);
}

void CDropResult::AddItem(uint32 itemID)
{
	TABLE_ITEM_BASIC* tbl = Engine->tblMgr->getItemData(itemID);
	if (tbl == nullptr)
		return;

	auto pItemExt = Engine->tblMgr->getExtData(tbl->extNum, itemID);

	std::string szIconFN = "";

	if (TABLE_ITEM_EXT* ext = Engine->tblMgr->getExtData(tbl->extNum, itemID))
	{
		DWORD dwIconID = tbl->dxtID;
		if (ext->dxtID > 0)
			dwIconID = ext->dxtID;

		szIconFN = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);
	}
	else {
		std::vector<char> buffer(256, NULL);

		sprintf(&buffer[0], "ui\\itemicon_%.1d_%.4d_%.2d_%.1d.dxt",
			(tbl->dxtID / 10000000),
			(tbl->dxtID / 1000) % 10000,
			(tbl->dxtID / 10) % 100,
			tbl->dxtID % 10);

		szIconFN = &buffer[0];
	}

	for (int i = 0; i < 5; i++)
	{
		if (dropitems[i].nItemID == 0)
		{
			dropitems[i].nItemID = itemID;
			dropitems[i].tbl = tbl;
			Engine->SetTexImage(dropitems[i].icon,szIconFN);
			dropitems[i].fileName = szIconFN.c_str();
			return;
		}
	}

	for (int i = 4; i >= 1; i--)
	{
		dropitems[i].nItemID = dropitems[i - 1].nItemID;
		Engine->SetTexImage(dropitems[i].icon,dropitems[i - 1].fileName);
		dropitems[i].fileName = dropitems[i - 1].fileName;
		dropitems[i].tbl = dropitems[i - 1].tbl;
	}

	dropitems[0].nItemID = itemID;
	dropitems[0].tbl = tbl;
	Engine->SetTexImage(dropitems[0].icon,szIconFN);
	dropitems[0].fileName=szIconFN;
}

void CDropResult::OpenDropResult()
{

}