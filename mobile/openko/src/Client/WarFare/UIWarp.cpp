// UIWarp.cpp: implementation of the UIWarp class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIWarp.h"
#include "UIManager.h"
#include "GameProcMain.h"
#include "text_resources.h"

#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIString.h>
#include <N3Base/N3UIList.h>
#include <N3Base/LogWriter.h>

CUIWarp::CUIWarp()
{
	m_pBtn_Ok         = nullptr;
	m_pBtn_Cancel     = nullptr;
	m_pList_Infos     = nullptr;
	m_pText_Agreement = nullptr; // 동의 사항..
}

CUIWarp::~CUIWarp()
{
}

bool CUIWarp::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

	N3_VERIFY_UI_COMPONENT(m_pBtn_Ok, GetChildByID<CN3UIButton>("Btn_Ok"));
	N3_VERIFY_UI_COMPONENT(m_pBtn_Cancel, GetChildByID<CN3UIButton>("Btn_Cancel"));

	// 2369 re_warp.istirap: "List_Infos" listesi yok; satırlar str_warp0..8 yazıları, seçim img_select resmidir.
	m_bRows2369 = false;
	if (GetChildByID<CN3UIList>("List_Infos") == nullptr && GetChildByID<CN3UIString>("str_warp0") != nullptr)
	{
		m_bRows2369 = true;
		for (int i = 0; i < MAX_ROWS_2369; i++)
			m_pRowStrs[i] = GetChildByID<CN3UIString>(fmt::format("str_warp{}", i));
		m_pImgSelect = GetChildByID("img_select");
		m_pBtnClose  = GetChildByID<CN3UIButton>("btn_close");
		if (m_pImgSelect)
			m_pImgSelect->SetVisible(false);
		CLogWriter::Write("CUIWarp: 2369 satır düzeni ({} satır, seçim resmi {}, kapat düğmesi {})", MAX_ROWS_2369, m_pImgSelect ? "var" : "yok",
			m_pBtnClose ? "var" : "yok");
	}
	if (!m_bRows2369)
		N3_VERIFY_UI_COMPONENT(m_pList_Infos, GetChildByID<CN3UIList>("List_Infos"));
	N3_VERIFY_UI_COMPONENT(m_pText_Agreement, GetChildByID<CN3UIString>("Text_Agreement"));

	return true;
}

void CUIWarp::SelectRow2369(int iRow)
{
	if (iRow < 0 || iRow >= (int) m_ListInfos.size() || iRow >= MAX_ROWS_2369)
		iRow = m_ListInfos.empty() ? -1 : 0;
	m_iCurSel2369 = iRow;
	for (int i = 0; i < MAX_ROWS_2369; i++)
	{
		if (m_pRowStrs[i] == nullptr)
			continue;
		m_pRowStrs[i]->SetColor(i == iRow ? 0xffffffff : 0xffd0d0d0);
	}
	if (m_pImgSelect != nullptr)
	{
		if (iRow >= 0 && m_pRowStrs[iRow] != nullptr)
		{
			RECT rcRow = m_pRowStrs[iRow]->GetRegion();
			RECT rcImg = m_pImgSelect->GetRegion();
			int iH     = rcImg.bottom - rcImg.top;
			int iRowH  = rcRow.bottom - rcRow.top;
			m_pImgSelect->SetPos(rcImg.left, rcRow.top + (iRowH - iH) / 2);
			m_pImgSelect->SetVisible(true);
		}
		else
			m_pImgSelect->SetVisible(false);
	}
	this->UpdateAgreement();
}

bool CUIWarp::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg & UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_Ok)
		{
			CGameProcedure::s_pProcMain->MsgSend_Warp();
		}
		else if (pSender == m_pBtn_Cancel || pSender == m_pBtnClose)
		{
		}
		this->SetVisible(false);
	}
	else if (m_bRows2369 && (dwMsg & (UIMSG_STRING_LCLICK | UIMSG_STRING_LDCLICK)))
	{
		for (int i = 0; i < MAX_ROWS_2369; i++)
		{
			if (pSender != m_pRowStrs[i] || m_pRowStrs[i] == nullptr)
				continue;
			if (i >= (int) m_ListInfos.size())
				break;
			SelectRow2369(i);
			if (dwMsg & UIMSG_STRING_LDCLICK)
			{
				CGameProcedure::s_pProcMain->MsgSend_Warp();
				this->SetVisible(false);
			}
			break;
		}
	}
	else if (dwMsg & UIMSG_LIST_SELCHANGE)
	{
		if (pSender == m_pList_Infos)
		{
			this->UpdateAgreement(); // 동의문 업데이트..
		}
	}
	else if (dwMsg & UIMSG_LIST_DBLCLK)
	{
		CGameProcedure::s_pProcMain->MsgSend_Warp();
		this->SetVisible(false);
	}

	return true;
}

void CUIWarp::InfoAdd(__WarpInfo&& WI)
{
	m_ListInfos.push_back(std::move(WI));
}

bool CUIWarp::InfoGetCur(__WarpInfo& WI)
{
	if (m_bRows2369)
	{
		if (m_iCurSel2369 < 0 || m_iCurSel2369 >= static_cast<int>(m_ListInfos.size()))
			return false;
		auto it = m_ListInfos.begin();
		std::advance(it, m_iCurSel2369);
		WI = *it;
		return true;
	}

	if (m_pList_Infos == nullptr)
		return false;

	int iSel = m_pList_Infos->GetCurSel();
	if (iSel < 0 || iSel >= static_cast<int>(m_ListInfos.size()))
		return false;

	auto it = m_ListInfos.begin();
	std::advance(it, iSel);
	WI = *it;

	return true;
}

void CUIWarp::UpdateList()
{
	if (m_bRows2369)
	{
		int i = 0;
		for (const __WarpInfo& wi : m_ListInfos)
		{
			if (i >= MAX_ROWS_2369)
				break;
			if (m_pRowStrs[i])
				m_pRowStrs[i]->SetString(wi.szName);
			i++;
		}
		for (; i < MAX_ROWS_2369; i++)
			if (m_pRowStrs[i])
				m_pRowStrs[i]->SetString("");
		SelectRow2369(0);
		return;
	}

	if (m_pList_Infos == nullptr)
		return;

	m_pList_Infos->ResetContent();
	it_WI it = m_ListInfos.begin(), itEnd = m_ListInfos.end();
	for (; it != itEnd; it++)
	{
		m_pList_Infos->AddString(it->szName);
	}

	m_pList_Infos->SetCurSel(0);
	this->UpdateAgreement();
}

void CUIWarp::UpdateAgreement()
{
	if (m_pText_Agreement == nullptr)
		return;
	if (!m_bRows2369 && m_pList_Infos == nullptr)
		return;

	int iSel = m_bRows2369 ? m_iCurSel2369 : m_pList_Infos->GetCurSel();
	if (iSel < 0 || iSel >= static_cast<int>(m_ListInfos.size()))
		return;

	auto it = m_ListInfos.begin();
	std::advance(it, iSel);
	m_pText_Agreement->SetString(it->szAgreement);
}

void CUIWarp::Reset()
{
	m_ListInfos.clear();
	this->UpdateList();
}

void CUIWarp::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if (bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI(); //this_ui
}

bool CUIWarp::OnKeyPress(int iKey)
{
	switch (iKey)
	{
		case DIK_ESCAPE:
			ReceiveMessage(m_pBtn_Cancel, UIMSG_BUTTON_CLICK);
			return true;

		case DIK_RETURN:
			ReceiveMessage(m_pBtn_Ok, UIMSG_BUTTON_CLICK);
			return true;

		default:
			break;
	}

	return CN3UIBase::OnKeyPress(iKey);
}
