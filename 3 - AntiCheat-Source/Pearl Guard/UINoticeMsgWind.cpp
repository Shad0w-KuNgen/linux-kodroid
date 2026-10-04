#include "UINoticeMsgWind.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CUIMoticeMsgWnd::CUIMoticeMsgWnd()
{
	vector<int>offsets;
	offsets.push_back(0x464);  // co_movie_save Satýr : 222
	offsets.push_back(0);

	el_base = Engine->rdword(KO_DLG, offsets);
	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);

	ParseUIElements();
	InitReceiveMessage();
}

CUIMoticeMsgWnd::~CUIMoticeMsgWnd()
{
	
}
DWORD func_notice = 0;
void CUIMoticeMsgWnd::ParseUIElements()
{
	m_Btn_Close = Engine->GetChildByID(m_dVTableAddr, "btn_close");
	m_pChatOut = Engine->GetChildByID(m_dVTableAddr, "txt_merchant");
	txt_title = Engine->GetChildByID(m_dVTableAddr, "txt_title");
}
DWORD uiRecent = 0;
void __stdcall UiNoticeWindowReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiNoticeWind->ReceiveMessage(pSender, dwMsg);

	__asm
	{
		MOV ECX, uiRecent
		PUSH dwMsg
		PUSH pSender
		MOV EAX, func_notice
		CALL EAX
	}
}


void CUIMoticeMsgWnd::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(el_base);
	func_notice = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)UiNoticeWindowReceiveMessage_Hook;
}
bool CUIMoticeMsgWnd::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	uiRecent = el_base;
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	 if (pSender ==(DWORD*) m_Btn_Close)
	{
		Close();
		return true;
	}

	return false;
}




void CUIMoticeMsgWnd::Open(bool isMerchant)
{
	Engine->SetVisible(el_base,true);
	Engine->ClearListString(m_pChatOut);

	std::vector<NoticeRecent> tmpRecentNotice;
	tmpRecentNotice.clear();

	if (isMerchant)
	{
		for (auto it : MerchantChat)
			tmpRecentNotice.push_back(it.second);
	}
	else {

		for (auto it : RecentNotice)
			tmpRecentNotice.push_back(it.second);

	}
	Engine->SetString(txt_title, (isMerchant ? "Merchant Notice" : "Recent Notice"));
	std::sort(tmpRecentNotice.begin(), tmpRecentNotice.end(),
		[](NoticeRecent const& a, NoticeRecent const& b) { return a.sIndex > b.sIndex; });

	for (auto it : tmpRecentNotice)
		Engine->AddListString(m_pChatOut, it.Notice.c_str(), (isMerchant ? D3DCOLOR_ARGB(255, 198, 198, 251) : D3DCOLOR_ARGB(255, 255, 255, 0)));
}

void CUIMoticeMsgWnd::Close()
{
	Engine->SetVisible(el_base, false);
}

void CUIMoticeMsgWnd::Clear()
{
	Close();
	RecentNotice.clear();
	MerchantChat.clear();
}

void CUIMoticeMsgWnd::MoticeMsgProcess(Packet& pkt,uint8 sType)
{
	std::string txt;
	uint32 color;

	pkt.DByte();
	pkt >> txt >> color;

	if (sType==2)
	{
		uint16 nIndex = uint16(RecentNotice.size() + 1);
		NoticeRecent item;
		item.sIndex = nIndex;
		item.Notice = txt.c_str();
		RecentNotice.insert(std::pair<uint16, NoticeRecent>(nIndex, item));
	}
	else {
		uint16 nIndex = uint16(MerchantChat.size() + 1);
		NoticeRecent item;
		item.sIndex = nIndex;
		item.Notice = txt.c_str();
		MerchantChat.insert(std::pair<uint16, NoticeRecent>(nIndex, item));
	}

	if (!Engine->IsVisible(el_base)) {
		if (sType == 2)
			Engine->uiInformationWind->notice_state = true;
		else
			Engine->uiInformationWind->merchant_state = true;
	}
	else if (Engine->IsVisible(el_base))
		Open((sType == 2 ? false : true));
}
