#include "StdAfx.h"
#include "UIGeneric2369.h"
#include "GameProcedure.h"
#include "UIManager.h"
#include <N3Base/LogWriter.h>
#include <algorithm>
#include <cctype>

bool CUIGeneric2369::LoadCentered(const std::string& szFile, int iScreenW, int iScreenH, const char* szLogName)
{
	m_szLogName = szLogName ? szLogName : "";
	if (szFile.empty() || !LoadFromFile(szFile))
	{
		CLogWriter::Write("{} penceresi yüklenemedi: \"{}\"", m_szLogName, szFile);
		return false;
	}
	RECT rc = GetRegion();
	int iW  = rc.right - rc.left, iH = rc.bottom - rc.top;
	SetPos(std::max(0, (iScreenW - iW) / 2), std::max(0, (iScreenH - iH) / 2));
	SetStyle(UISTYLE_USER_MOVE_HIDE);
	SetVisibleWithNoSound(false);
	CLogWriter::Write("{} penceresi hazır ({}): {}x{}; ağaç: {}", m_szLogName, szFile, iW, iH, DumpTreeForLog());
	return true;
}

void CUIGeneric2369::Open()
{
	SetVisible(true);
	if (CGameProcedure::s_pUIMgr)
		CGameProcedure::s_pUIMgr->SetFocusedUI(this);
}

void CUIGeneric2369::Close()
{
	SetVisible(false);
	if (CGameProcedure::s_pUIMgr)
		CGameProcedure::s_pUIMgr->ReFocusUI();
}

void CUIGeneric2369::Toggle()
{
	if (IsVisible())
		Close();
	else
		Open();
}

bool CUIGeneric2369::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr)
		return false;
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		std::string szID = pSender->m_szID;
		for (char& c : szID)
			c = (char) std::tolower((unsigned char) c);
		if (szID.find("close") != std::string::npos || szID.find("exit") != std::string::npos || szID.find("cancel") != std::string::npos
			|| szID.find("btn_x") != std::string::npos)
		{
			Close();
			return true;
		}
		CLogWriter::Write("{} penceresi: düğme \"{}\" (henüz sunucuya bağlı değil)", m_szLogName, pSender->m_szID);
	}
	return CN3UIBase::ReceiveMessage(pSender, dwMsg);
}
