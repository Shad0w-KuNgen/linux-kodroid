// UICharacterSelect.cpp: implementation of the UICharacterSelect class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UICharacterSelect.h"
#include <algorithm>
#include "APISocket.h"
#include "GameProcCharacterSelect.h"
#include "UIManager.h"
#include "text_resources.h"

#include <N3Base/N3UIString.h>
#include <N3Base/LogWriter.h>
#include <N3Base/N3UITooltip.h>

CUICharacterSelect::CUICharacterSelect()
{
	m_eType        = UI_TYPE_BASE;

	m_pBtnLeft     = nullptr;
	m_pBtnRight    = nullptr;
	m_pBtnExit     = nullptr;
	m_pBtnDelete   = nullptr;
	m_pBtnBack     = nullptr;
	m_pUserInfoStr = nullptr;
}

CUICharacterSelect::~CUICharacterSelect()
{
}

void CUICharacterSelect::Release()
{
	m_pBtnLeft     = nullptr;
	m_pBtnRight    = nullptr;
	m_pBtnExit     = nullptr;
	m_pBtnDelete   = nullptr;
	m_pBtnBack     = nullptr;
	m_pUserInfoStr = nullptr;
	m_pBtnStart    = nullptr;
	m_pBtnCreate   = nullptr;
	m_pStrId       = nullptr;
	m_pStrLevel    = nullptr;
	m_pStrJob      = nullptr;

	CN3UIBase::Release();
}

bool CUICharacterSelect::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

	// 2369 re_characterselect.uif kimlikleri: btn_left/btn_right (Group_OtherCharacter), btn_exit/btn_back/btn_start/
	// btn_create ve str_id/str_lev/str_job (Group_SelectWindow); 1298: bt_left/bt_right/bt_exit/bt_delete/bt_back/text00.
	auto findAny = [this](const char* szA, const char* szB) -> CN3UIBase* {
		CN3UIBase* p = GetChildByID(szA);
		return p ? p : GetChildByID(szB);
	};
	const bool b2369Layout = GetChildByID("bt_left") == nullptr && GetChildByID("btn_left") != nullptr;
	N3_VERIFY_UI_COMPONENT(m_pBtnLeft, findAny("bt_left", "btn_left"));
	N3_VERIFY_UI_COMPONENT(m_pBtnRight, findAny("bt_right", "btn_right"));
	N3_VERIFY_UI_COMPONENT(m_pBtnExit, findAny("bt_exit", "btn_exit"));
	N3_VERIFY_UI_COMPONENT(m_pBtnDelete, findAny("bt_delete", "btn_delete"));
	N3_VERIFY_UI_COMPONENT(m_pBtnBack, findAny("bt_back", "btn_back"));
	m_pBtnStart  = GetChildByID("btn_start");
	m_pBtnCreate = GetChildByID("btn_create");
	m_pStrId     = GetChildByID<CN3UIString>("str_id");
	m_pStrLevel  = GetChildByID<CN3UIString>("str_lev");
	m_pStrJob    = GetChildByID<CN3UIString>("str_job");
	if (m_pStrId != nullptr && GetChildByID<CN3UIString>("text00") == nullptr)
		m_pUserInfoStr = nullptr; // 2369: bilgi üç ayrı yazıda (DisplayChrInfo)
	else
		N3_VERIFY_UI_COMPONENT(m_pUserInfoStr, GetChildByID<CN3UIString>("text00"));
	if (b2369Layout)
	{
		CLogWriter::Write("CUICharacterSelect: 2369 düzeni (başlat {}, oluştur {}, ad/seviye/sınıf yazıları {}/{}/{})",
			m_pBtnStart ? "var" : "yok", m_pBtnCreate ? "var" : "yok", m_pStrId ? "var" : "yok", m_pStrLevel ? "var" : "yok",
			m_pStrJob ? "var" : "yok");
		// 2369 arayüzü ekran boyutuna göre tasarlanmış: 1298'in düğme taşıma düzeltmeleri gereksiz
		RECT rc2369;
		SetRect(&rc2369, 0, 0, s_CameraData.vp.Width, s_CameraData.vp.Height);
		SetRegion(rc2369);
		// Orijinal 24xx ekranı: seçim paneli sağ kenarda dikey ortada, karakter okları alt ortada, logo sol üstte
		const int iW = s_CameraData.vp.Width, iH = s_CameraData.vp.Height;
		if (CN3UIBase* pWin = GetChildByID("Group_SelectWindow"))
		{
			RECT r = pWin->GetRegion();
			pWin->SetPos(iW - (r.right - r.left) - 8, std::max(0, (iH - (r.bottom - r.top)) / 2));
		}
		if (CN3UIBase* pArrows = GetChildByID("Group_OtherCharacter"))
		{
			RECT r = pArrows->GetRegion();
			pArrows->SetPos((iW - (r.right - r.left)) / 2, std::max(0, (int) (iH * 0.80f) - (r.bottom - r.top)));
		}
		if (CN3UIBase* pBottom = GetChildByID("Group_Bottom_Img"))
		{
			RECT r = pBottom->GetRegion();
			pBottom->SetPos(r.left, iH - (r.bottom - r.top)); // alt kenar görselleri
		}
		return true;
	}

	// 위치를 화면 해상도에 맞게 바꾸기...
	POINT pt {};
	RECT rc      = GetRegion();
	float fRatio = (float) s_CameraData.vp.Width / (rc.right - rc.left);

	if (m_pBtnLeft != nullptr)
	{
		pt   = m_pBtnLeft->GetPos();
		pt.x = (int) (pt.x * fRatio);
		pt.y = s_CameraData.vp.Height - 10 - m_pBtnLeft->GetHeight();
		m_pBtnLeft->SetPos(pt.x, pt.y);
	}

	if (m_pBtnRight != nullptr)
	{
		pt   = m_pBtnRight->GetPos();
		pt.x = (int) (pt.x * fRatio);
		pt.y = s_CameraData.vp.Height - 10 - m_pBtnRight->GetHeight();
		m_pBtnRight->SetPos(pt.x, pt.y);
	}

	if (m_pBtnExit != nullptr)
	{
		pt   = m_pBtnExit->GetPos();
		pt.x = (int) (pt.x * fRatio);
		pt.y = s_CameraData.vp.Height - 10 - m_pBtnExit->GetHeight();
		m_pBtnExit->SetPos(pt.x, pt.y);
	}

	if (m_pBtnBack != nullptr)
	{
		// Previous point in sane cases should be be the exit button.
		// There's a 15 pixel gap between them in the UIF's layout.
		POINT ptPrev = pt;
		pt           = m_pBtnBack->GetPos();
		pt.x         = (int) (pt.x * fRatio);
		pt.y         = ptPrev.y - 15 - m_pBtnBack->GetHeight();
		m_pBtnBack->SetPos(pt.x, pt.y);
	}

	if (m_pBtnDelete != nullptr)
	{
		pt   = m_pBtnDelete->GetPos();
		pt.x = (int) (pt.x * fRatio);
		pt.y = 20;
		m_pBtnDelete->SetPos(pt.x, pt.y);
	}

	SetRect(&rc, 0, 0, s_CameraData.vp.Width, s_CameraData.vp.Height);
	SetRegion(rc);

	return true;
}

void CUICharacterSelect::Tick()
{
	CN3UIBase::Tick();
}

bool CUICharacterSelect::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr)
		return false;

	if (!CGameProcedure::s_pUIMgr->EnableOperation())
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		// Rotate Left..
		if (pSender == m_pBtnLeft)
		{
			CGameProcedure::s_pProcCharacterSelect->DoJobLeft();
		}
		// Rotate Right..
		else if (pSender == m_pBtnRight)
		{
			CGameProcedure::s_pProcCharacterSelect->DojobRight();
		}
		else if (pSender == m_pBtnExit)
		{
			//			CGameProcedure::ProcActiveSet((CGameProcedure*)CGameProcedure::s_pProcLogIn); // 로그인으로 돌아간다..
			std::string szMsg = fmt::format_text_resource(IDS_CONFIRM_EXIT_GAME);
			CGameProcedure::MessageBoxPost(szMsg, "", MB_YESNO, BEHAVIOR_EXIT);
		}
		else if (pSender == m_pBtnBack)
		{
			CGameProcedure::s_bNeedReportConnectionClosed = false;
			CGameProcedure::s_pSocket->Disconnect();
			CGameProcedure::s_bNeedReportConnectionClosed = true;

			CGameProcedure::ProcActiveSet((CGameProcedure*) CGameProcedure::s_pProcLogIn); // 로그인으로 돌아간다..
		}
		else if (pSender != nullptr && (pSender == m_pBtnStart || pSender == m_pBtnCreate))
		{
			// 2369: BAŞLAT = seçili (yoksa ilk dolu) yuvadaki karakterle gir; OLUŞTUR = ilk boş yuvada karakter yarat.
			// ProcessOnReturn seçim değil (Main'e geçiş/ışık), bu yüzden doğrudan yuva seçilip CharacterSelectOrCreate.
			CGameProcCharacterSelect* pSel = CGameProcedure::s_pProcCharacterSelect;
			if (pSel != nullptr && pSel->m_eCurProcess == PROCESS_PRESELECT)
			{
				const bool bStart = pSender == m_pBtnStart;
				int iCur          = pSel->m_eCurPos == POS_LEFT ? 1 : pSel->m_eCurPos == POS_RIGHT ? 2 : 0;
				int iSlot         = -1;
				if (bStart && pSel->m_pChrs[iCur] != nullptr)
					iSlot = iCur;
				for (int i = 0; iSlot < 0 && i < MAX_AVAILABLE_CHARACTER; i++)
					if ((pSel->m_pChrs[i] != nullptr) == bStart)
						iSlot = i;
				if (iSlot >= 0)
				{
					CGameProcedure::s_iChrSelectIndex = iSlot;
					CLogWriter::Write("Karakter seçimi düğmesi {}: yuva {}", bStart ? "BASLAT" : "OLUSTUR", iSlot);
					pSel->CharacterSelectOrCreate();
				}
				else
					CLogWriter::Write("Karakter seçimi düğmesi {}: uygun yuva yok", bStart ? "BASLAT" : "OLUSTUR");
			}
		}
		else if (pSender == m_pBtnDelete)
		{
			std::string szMsg = fmt::format_text_resource(IDS_CONFIRM_DELETE_CHR);

			// NOTE: Character deletion is disabled and this resource is changed appropriately.
			// As such, rather than prompt to delete, we should simply show the new message.
#if 0
			CGameProcedure::MessageBoxPost(szMsg, "", MB_YESNO, BEHAVIOR_DELETE_CHR);
#else
			CGameProcedure::MessageBoxPost(szMsg, "", MB_OK);
#endif
		}
	}

	return true;
}

void CUICharacterSelect::DisplayChrInfo(__CharacterSelectInfo* pCSInfo)
{
	std::string szTotal;

	if (!pCSInfo->szID.empty())
	{
		std::string szClass;
		CGameBase::GetTextByClass(pCSInfo->eClass, szClass);

		// Level: %d\nSpecialty: %s\nID: %s
		szTotal = fmt::format_text_resource(IDS_CHR_SELECT_FMT_INFO, pCSInfo->iLevel, szClass, pCSInfo->szID);
	}
	else
	{
		szTotal = fmt::format_text_resource(IDS_CHR_SELECT_HINT);
	}

	if (m_pUserInfoStr != nullptr)
	{
		m_pUserInfoStr->SetVisible(true);
		m_pUserInfoStr->SetString(szTotal);
	}
	// 2369: ad / seviye / sınıf ayrı yazılar
	const bool bHasChr = !pCSInfo->szID.empty();
	if (m_pStrId != nullptr)
	{
		m_pStrId->SetVisible(true);
		m_pStrId->SetString(bHasChr ? pCSInfo->szID : std::string());
	}
	if (m_pStrLevel != nullptr)
	{
		m_pStrLevel->SetVisible(true);
		m_pStrLevel->SetString(bHasChr ? fmt::format("{}", pCSInfo->iLevel) : std::string());
	}
	if (m_pStrJob != nullptr)
	{
		std::string szClass;
		if (bHasChr)
			CGameBase::GetTextByClass(pCSInfo->eClass, szClass);
		m_pStrJob->SetVisible(true);
		m_pStrJob->SetString(szClass);
	}
	if (m_pBtnStart != nullptr)
		m_pBtnStart->SetVisible(bHasChr);
	if (m_pBtnCreate != nullptr)
		m_pBtnCreate->SetVisible(!bHasChr);
}

void CUICharacterSelect::DontDisplayInfo()
{
	if (m_pUserInfoStr != nullptr)
		m_pUserInfoStr->SetVisible(false);
	for (CN3UIString* p : { m_pStrId, m_pStrLevel, m_pStrJob })
		if (p != nullptr)
			p->SetVisible(false);
}

bool CUICharacterSelect::OnKeyPress(int iKey)
{
	if (CGameProcedure::s_pUIMgr->EnableOperation())
	{
		switch (iKey)
		{
			case DIK_ESCAPE:
				ReceiveMessage(m_pBtnExit, UIMSG_BUTTON_CLICK);
				return true;
			case DIK_LEFT:
				ReceiveMessage(m_pBtnLeft, UIMSG_BUTTON_CLICK);
				return true;
			case DIK_RIGHT:
				ReceiveMessage(m_pBtnRight, UIMSG_BUTTON_CLICK);
				return true;
			case DIK_NUMPADENTER:
			case DIK_RETURN:
				CGameProcedure::s_pProcCharacterSelect->CharacterSelectOrCreate();
				return true;

			default:
				break;
		}
	}

	return CN3UIBase::OnKeyPress(iKey);
}

uint32_t CUICharacterSelect::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!m_bVisible)
		return dwRet;

	// UI 움직이는 코드
	if (UI_STATE_COMMON_MOVE == m_eState)
	{
		if (dwFlags & UI_MOUSE_LBCLICKED)
		{
			SetState(UI_STATE_COMMON_NONE);
		}
		else
		{
			MoveOffset(ptCur.x - ptOld.x, ptCur.y - ptOld.y);
		}
		dwRet |= UI_MOUSEPROC_DONESOMETHING;
		return dwRet;
	}

	if (false == IsIn(ptCur.x, ptCur.y)) // 영역 밖이면
	{
		if (false == IsIn(ptOld.x, ptOld.y))
		{
			return dwRet; // 이전 좌표도 영역 밖이면
		}
	}
	else
	{
		// tool tip 관련
		if (s_pTooltipCtrl != nullptr)
			s_pTooltipCtrl->SetText(m_szToolTip, m_crToolTip);
	}

	if (m_pChildUI && m_pChildUI->IsVisible())
		return dwRet;

	// child에게 메세지 전달
	for (UIListItor itor = m_Children.begin(); m_Children.end() != itor; ++itor)
	{
		CN3UIBase* pChild   = (*itor);
		uint32_t dwChildRet = pChild->MouseProc(dwFlags, ptCur, ptOld);
		if (UI_MOUSEPROC_DONESOMETHING & dwChildRet)
		{ // 이경우에는 먼가 포커스를 받은 경우이다.

			dwRet |= (UI_MOUSEPROC_CHILDDONESOMETHING | UI_MOUSEPROC_DONESOMETHING);
			return dwRet;
		}
	}

	// UI 움직이는 코드
	if (UI_STATE_COMMON_MOVE != m_eState && PtInRect(&m_rcMovable, ptCur) && (dwFlags & UI_MOUSE_LBCLICK))
	{
		SetState(UI_STATE_COMMON_MOVE);
		dwRet |= UI_MOUSEPROC_DONESOMETHING;
		return dwRet;
	}

	return dwRet;
}
