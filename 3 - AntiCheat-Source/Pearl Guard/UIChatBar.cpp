
#include "UIChatBar.h"
#include "Pearl Engine.h"

enum Message
{
	Normal = 1,
	Private = 2,
	Shout = 3,
	Party = 4,
	Clan = 5,
	Ally = 6,
	Chat_Lock = 7
};

CUIChatBarPlug::CUIChatBarPlug()
{
	vector<int>offsets;
	offsets.push_back(0x20C);
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);

	if (m_dVTableAddr == 0)
		return;

	m_noah_knight_btn = m_loop = m_filter_base = m_Chating_Lock =  0;

	for (int i = 1; i <= 6; i++)
	{
		m_btn[i] = 0;
		m_BlockButtons[i] = 0;
	}

	ParseUIElements();
	InitReceiveMessage();
}

CUIChatBarPlug::~CUIChatBarPlug()
{

}
void CUIChatBarPlug::ParseUIElements()
{
	//Genel
	DWORD parent, baseID, base_chat, base_scroll, BaseMain2, base_ID;
	std::string find = xorstr("base_main");
	Engine->GetChildByID(m_dVTableAddr, find, baseID);
	find = xorstr("base_filter");
	Engine->GetChildByID(baseID, find, parent);
	find = xorstr("btn_check_normal");
	Engine->GetChildByID(parent, find, m_BlockButtons[1]);
	find = xorstr("btn_check_private");
	Engine->GetChildByID(parent, find, m_BlockButtons[2]);
	find = xorstr("btn_check_shout");
	Engine->GetChildByID(parent, find, m_BlockButtons[3]);
	find = xorstr("btn_check_party_force");
	Engine->GetChildByID(parent, find, m_BlockButtons[4]);
	find = xorstr("btn_check_clan");
	Engine->GetChildByID(parent, find, m_BlockButtons[5]);
	find = xorstr("btn_check_union");
	Engine->GetChildByID(parent, find, m_BlockButtons[6]);

	find = xorstr("BaseChangeChattingMode");
	base_main = baseID;

	Engine->GetChildByID(baseID, find, BaseMain2);

	for (int i = 1; i <= 6; i++)
	{
		find = string_format(xorstr("btn_%d"), i);
		Engine->GetChildByID(BaseMain2, find, m_btn[i]);
		Engine->SetState(m_btn[i], UI_STATE_BUTTON_DOWN);
	}

	//Chating Lock
	find = xorstr("btn_7");
	Engine->GetChildByID(BaseMain2, find, m_Chating_Lock2);

	//base filter buttons
	find = xorstr("base_scroll");
	Engine->GetChildByID(baseID, find, base_scroll);
	find = xorstr("btn_filter");
	Engine->GetChildByID(base_scroll, find, m_loop);
	find = xorstr("btn_chat_lock");
	Engine->GetChildByID(base_scroll, find, m_Chating_Lock);

	//base_main = Engine->GetChildByID(m_dVTableAddr, "base_main");
	//mykoframe = Engine->GetChildByID(m_dVTableAddr, "mykoframe");
	//base_filter = Engine->GetChildByID(base_main, "base_filter");
	//m_BlockButtons[1] = Engine->GetChildByID(base_filter, "btn_check_normal");
	//m_BlockButtons[2] = Engine->GetChildByID(base_filter, "btn_check_private");
	//m_BlockButtons[3] = Engine->GetChildByID(base_filter, "btn_check_shout");
	//m_BlockButtons[4] = Engine->GetChildByID(base_filter, "btn_check_party_force");
	//m_BlockButtons[5] = Engine->GetChildByID(base_filter, "btn_check_clan");
	//m_BlockButtons[6] = Engine->GetChildByID(base_filter, "btn_check_union");
	//BaseChangeChattingMode = Engine->GetChildByID(base_main, "BaseChangeChattingMode");
	//for (int i = 1; i <= 6; i++)
	//{
	//	m_btn[i] = Engine->GetChildByID(BaseChangeChattingMode, string_format(xorstr("btn_%d"), i + 1));
	//	Engine->SetState(m_btn[i], UI_STATE_BUTTON_DOWN);
	//}

	//m_Chating_Lock2 = Engine->GetChildByID(BaseChangeChattingMode, "btn_7");
	//base_scroll = Engine->GetChildByID(base_main, "base_scroll");
	//m_loop = Engine->GetChildByID(base_scroll, "btn_filter");
	//m_Chating_Lock = Engine->GetChildByID(base_scroll, "btn_chat_lock");
}

DWORD uiChatBarVTable;

bool CUIChatBarPlug::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	uiChatBarVTable = m_dVTableAddr;
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	if (pSender == (DWORD*)m_Chating_Lock2 && Engine->GetState(m_Chating_Lock2) == UI_STATE_BUTTON_DOWN)
	{

		ChangeChattingMode(m_Chating_Lock, 3);
		MessageNotice(7, 0);
	}
	else if (pSender == (DWORD*)m_Chating_Lock2 && Engine->GetState(m_Chating_Lock2) == UI_STATE_BUTTON_ON)
	{
		ChangeChattingMode(m_Chating_Lock, 2);
		MessageNotice(7, 1);
	}

	for (size_t i = 1; i <= 6; i++)
	{
		if (pSender == (DWORD*)m_btn[i] && Engine->GetState(m_btn[i]) == UI_STATE_BUTTON_DOWN)
		{
			ChangeChattingMode(m_BlockButtons[i], 3);
			MessageNotice(i,1);
			break;
		}
		else if (pSender == (DWORD*)m_btn[i] && Engine->GetState(m_btn[i]) == UI_STATE_BUTTON_ON)
		{
			ChangeChattingMode(m_BlockButtons[i], 2);
			MessageNotice(i,0);
			break;
		}
	}

	return true;
}

DWORD Func_Chatbar;

void __stdcall UiChatBarReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiChatBarPlug->ReceiveMessage(pSender, dwMsg);

	__asm
	{
		MOV ECX, uiChatBarVTable
		PUSH dwMsg
		PUSH pSender
		CALL Func_Chatbar
	}
}

void CUIChatBarPlug::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_Chatbar = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)UiChatBarReceiveMessage_Hook;
}

void CUIChatBarPlug::SendButtonClick(DWORD btn)
{
	UiChatBarReceiveMessage_Hook((DWORD*)btn, UIMSG_BUTTON_CLICK);
}

void CUIChatBarPlug::ChangeChattingMode(DWORD btn, uint32_t dwMsg)
{
	switch (dwMsg)
	{
	case 2: // block
		Engine->SetState(btn, UI_STATE_BUTTON_ON);
		SendButtonClick(btn);
		break;
	case 3: // On
		Engine->SetState(btn, UI_STATE_BUTTON_DOWN);
		SendButtonClick(btn);
		break;
	default:
		break;
	}

}

void CUIChatBarPlug::MessageNotice(uint8 type , uint8 status)
{
	if (type == Message::Normal && status == None)
		Engine->WriteInfoMessage(xorstr("Normal Chat Blocked."), 0xFFE0301E);
	else if (type == Message::Private && status == None)
		Engine->WriteInfoMessage(xorstr("Private Chat Blocked."), 0xFFE0301E);
	else if (type == Message::Shout && status == None)
		Engine->WriteInfoMessage(xorstr("Shout Chat Blocked."), 0xFFE0301E);
	else if (type == Message::Party && status == None)
		Engine->WriteInfoMessage(xorstr("Party Chat Blocked."), 0xFFE0301E);
	else if (type == Message::Clan && status == None)
		Engine->WriteInfoMessage(xorstr("Clan Chat Blocked."), 0xFFE0301E);
	else if (type == Message::Ally && status == None)
		Engine->WriteInfoMessage(xorstr("Ally Chat Blocked."), 0xFFE0301E);
	else if (type == Message::Chat_Lock && status == None)
		Engine->WriteInfoMessage(xorstr("Chatting Box Blocked."), 0xFFE0301E);

	if (type == Message::Normal && status == Ok)
		Engine->WriteInfoMessage(xorstr("Normal Chat Enabled."), 0xFF33C1B1);
	else if (type == Message::Private && status == Ok)
		Engine->WriteInfoMessage(xorstr("Private Chat Enabled."), 0xFF33C1B1);
	else if (type == Message::Shout && status == Ok)
		Engine->WriteInfoMessage(xorstr("Shout Chat Enabled."), 0xFF33C1B1);
	else if (type == Message::Party && status == Ok)
		Engine->WriteInfoMessage(xorstr("Party Chat Enabled."), 0xFF33C1B1);
	else if (type == Message::Clan && status == Ok)
		Engine->WriteInfoMessage(xorstr("Clan Chat Enabled."), 0xFF33C1B1);
	else if (type == Message::Ally && status == Ok)
		Engine->WriteInfoMessage(xorstr("Ally Chat Enabled."), 0xFF33C1B1);
	else if (type == Message::Chat_Lock && status == Ok)
		Engine->WriteInfoMessage(xorstr("Chatting Box Enabled."), 0xFF33C1B1);
}


