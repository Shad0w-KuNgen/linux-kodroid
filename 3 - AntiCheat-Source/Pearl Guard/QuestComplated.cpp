#include "QuestComplated.h"
bool isShow = true;
CUIQuestComplated::CUIQuestComplated()
{
	m_dVTableAddr = NULL;

	vector<int>offsets;
	offsets.push_back(0x3A4);
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);
	
	ParseUIElements();
	InitReceiveMessage();
}

CUIQuestComplated::~CUIQuestComplated()
{
}
bool firstLoad = true;
DWORD uiComplate;
DWORD Func_uiComplate;

void CUIQuestComplated::ParseUIElements()
{
	txt_title = Engine->GetChildByID(m_dVTableAddr, "txt_title");
	for (int i = 0; i < 10;i++)
	{
		group[i] = Engine->GetChildByID(m_dVTableAddr, string_format(xorstr("group_%d"), i));
		txt_msg[i] = Engine->GetChildByID(group[i], string_format(xorstr("txt_msg_%d"), i));
	}
	group_base = Engine->GetChildByID(m_dVTableAddr, "group_base");
	btn_show = Engine->GetChildByID(group_base, "btn_show");
	group_top = Engine->GetChildByID(m_dVTableAddr, "group_top");
}

bool CUIQuestComplated::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	uiComplate = m_dVTableAddr;

	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	if (pSender == (DWORD*)btn_show)
	{
		isShow = !isShow;
	}

	return true;
}
void __stdcall uiQuestComplatedReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiQuestComplated->ReceiveMessage(pSender, dwMsg);

	__asm
	{
		MOV ECX, uiComplate
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_uiComplate
		CALL EAX
	}
}
void CUIQuestComplated::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_uiComplate = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)uiQuestComplatedReceiveMessage_Hook;
}

void CUIQuestComplated::SendQuestBoard(std::string QuestName, uint16 CurCount, uint16 TotalCount, uint16 MonsterID, uint8 nSize)
{
	if (CurCount > TotalCount || !isShow)
		return;

	for (int i = 0; i < 10; i++)
		Engine->SetVisible(group[i], false);
	Engine->SetVisible(group[0], true);
	Engine->SetVisible(m_dVTableAddr, true);
	if (CurCount == TotalCount)
	{
		POINT setui, getui, getui2;
		getui = Engine->GetUiPos(group[0]);
		getui2 = Engine->GetUiPos(group_base);
		setui.x = getui2.x;
		setui.y = getui.y + Engine->GetUiHeight(group[0]);
		Engine->SetUIPos(group_base, setui);
		Engine->SetString(txt_title, string_format("%s", QuestName.c_str()));
		Engine->SetString(txt_msg[0], string_format("Monster hunt complete!"));
		return;
	}
	TABLE_MOB* mobData = Engine->tblMgr->getMobData(MonsterID);
	std::string monstername = "<Unkown>";
	if (mobData != nullptr)
		monstername = mobData->strName;
	else if (MonsterID == 1 || MonsterID == 2)
		monstername = "Kill Enemy";

	
	Engine->SetString(txt_title, string_format("%s", QuestName.c_str()));
	Engine->SetString(txt_msg[0], string_format("%s %d/%d", monstername.c_str(), CurCount, TotalCount));

	POINT setui, getui, getui2;
	getui = Engine->GetUiPos(group[0]);
	getui2 = Engine->GetUiPos(group_base);
	setui.x = getui2.x;
	setui.y = getui.y + Engine->GetUiHeight(group[0]);
	Engine->SetVisible(m_dVTableAddr, true);
	Engine->SetUIPos(group_base, setui);
	if (firstLoad)
	{
		RECT screen = Engine->m_UiMgr->GetScreenRect();
		POINT ret;
		ret.x = (screen.right / 2) - (Engine->GetUiWidth(m_dVTableAddr) / 2);
		ret.y = 95;
		Engine->SetUIPos(m_dVTableAddr, ret);
		firstLoad = !firstLoad;
	}

	
}