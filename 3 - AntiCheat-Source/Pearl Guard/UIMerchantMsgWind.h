#pragma once
class CUIMerchantMsgWnd;
#include "stdafx.h"
#include "../N3BASE/N3UIBase.h"
#include "../N3BASE/N3UIString.h"
#include "../N3BASE/N3UIButton.h"
#include "../N3BASE/N3UIScrollBar.h"
#include "Pearl Engine.h"
#include <deque>

struct __ChatInfo
{
	std::string szChat; // 문자열
	D3DCOLOR	color; // 문자열 색깔

	__ChatInfo() {};
	__ChatInfo(const std::string& szChat_Arg, D3DCOLOR color_Arg) { szChat = szChat_Arg; color = color_Arg; }
};

typedef std::deque<__ChatInfo*>		ChatList;
typedef ChatList::iterator			ChatListItor;
typedef ChatList::reverse_iterator	ChatListReverseItor;
const int MAX_CHAT_LINES = 100;

class CUIMerchantMsgWnd : public CN3UIBase
{
	CN3UIString* m_pChatOut;
	CN3UIScrollBar* m_pScrollbar;
	CN3UIButton* m_Btn_Close;

	ChatList		m_ChatBuffer;
	ChatList		m_LineBuffer;

	int				m_iChatLineCount;
	RECT			m_rcChatOutRegion;
	CN3UIString** m_ppUILines;

public:
	CN3UIString* m_ptitle;
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	void Release();
	bool Load(HANDLE hFile);
	void AddMsg(const std::string& szString, D3DCOLOR color = 0xffffffff);
	void SetTopLine(int iTopLine);
	void AddLineBuffer(const std::string& szString, D3DCOLOR color);
	void RecalcLineBuffer();
	void CreateLines();
	void Open();
	void Close();
	void MerchantMsgProcess(Packet& pkt,uint8 sType);

	CUIMerchantMsgWnd();
	~CUIMerchantMsgWnd();
};