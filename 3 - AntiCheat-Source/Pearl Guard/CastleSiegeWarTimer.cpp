#include "CastleSiegeWarTimer.h"

CCastleSiegeWarTimer::CCastleSiegeWarTimer()
{
	m_Timer = NULL;

	posSet = false;
	m_iStartTime = 0;
	m_iRemainingTime = 0;

	btn_close = NULL;
	m_txtStartingTime = m_txtRemainingTime = m_txtSiegeOwnerClanName = NULL;
	m_InfoSelect = m_byStand = m_byStarting = NULL;
}

CCastleSiegeWarTimer::~CCastleSiegeWarTimer()
{
}

bool CCastleSiegeWarTimer::Load(HANDLE hFile)
{
	if (CN3UIBase::Load(hFile) == false) 
		return false;

	std::string find = xorstr("base_siegewarfare");
	base_siegewarfare = (CN3UIBase*)GetChildByID(find);
	find = xorstr("str_info_1");
	m_txtStartingTime = (CN3UIString*)base_siegewarfare->GetChildByID(find);
	find = xorstr("str_info_2");
	m_txtRemainingTime = (CN3UIString*)base_siegewarfare->GetChildByID(find);
	find = xorstr("str_info_3");
	m_txtSiegeOwnerClanName = (CN3UIString*)base_siegewarfare->GetChildByID(find);
	find = xorstr("btn_close");
	btn_close = (CN3UIButton*)base_siegewarfare->GetChildByID(find);
	find = xorstr("img_infoselecte");
	m_InfoSelect = (CN3UIImage*)base_siegewarfare->GetChildByID(find);
	find = xorstr("img_starting");
	m_byStarting = (CN3UIImage*)base_siegewarfare->GetChildByID(find);
	find = xorstr("img_bystand");
	m_byStand = (CN3UIImage*)base_siegewarfare->GetChildByID(find);
	
	m_InfoSelect->SetVisible(true);
	m_byStand->SetVisible(true);
	m_byStarting->SetVisible(false);

	POINT eventNoticePos;
	Engine->GetUiPos(Engine->uiTargetBar->m_dVTableAddr, eventNoticePos);
	SetPos(eventNoticePos.x + Engine->GetUiWidth(Engine->uiTargetBar->m_dVTableAddr) + 5, 50);

	RECT rc;
	SetRect(&rc, m_rcRegion.left, m_rcRegion.top, m_rcRegion.left + base_siegewarfare->GetWidth(), m_rcRegion.top + base_siegewarfare->GetHeight());
	SetRegion(rc);
	SetMoveRect(base_siegewarfare->GetRegion());
	return true;
}

void CCastleSiegeWarTimer::Tick()
{
	if (Engine->isSiegeWarStart && m_iStartTime > 0)
	{
		if (m_Timer->IsElapsedSecond()) 
			m_iStartTime--;

		if (m_iStartTime <= 0)
			m_iStartTime = 0;

		uint32_t starttimeMinutes = (uint16_t)ceil(m_iStartTime / 60);
		uint32_t starttimeSeconds = m_iStartTime - (starttimeMinutes * 60);

		std::string starttime;
		if (starttimeMinutes < 10 && starttimeSeconds < 10)
			starttime = string_format(xorstr("0%d : 0%d"), starttimeMinutes, starttimeSeconds);
		else if (starttimeMinutes < 10)
			starttime = string_format(xorstr("0%d : %d"), starttimeMinutes, starttimeSeconds);
		else if (starttimeSeconds < 10)
			starttime = string_format(xorstr("%d : 0%d"), starttimeMinutes, starttimeSeconds);
		else
			starttime = string_format(xorstr("%d : %d"), starttimeMinutes, starttimeSeconds);

		m_txtStartingTime->SetString(string_format(xorstr("Start Time: [%s]"), starttime.c_str()));
	}

	if (Engine->isSiegeWarActive && m_iRemainingTime > 0)
	{
		if (m_Timer->IsElapsedSecond()) 
			m_iRemainingTime--;

		uint16_t remainingMinutes = (uint16_t)ceil(m_iRemainingTime / 60);
		uint16_t remainingSeconds = m_iRemainingTime - (remainingMinutes * 60);

		std::string remainingTime;
		if (remainingMinutes < 10 && remainingSeconds < 10)
			remainingTime = string_format(xorstr("0%d : 0%d"), remainingMinutes, remainingSeconds);
		else if (remainingMinutes < 10)
			remainingTime = string_format(xorstr("0%d : %d"), remainingMinutes, remainingSeconds);
		else if (remainingSeconds < 10)
			remainingTime = string_format(xorstr("%d : 0%d"), remainingMinutes, remainingSeconds);
		else 
			remainingTime = string_format(xorstr("%d : %d"), remainingMinutes, remainingSeconds);

		m_txtRemainingTime->SetString(string_format(xorstr("Remaining Time: [%s]"), remainingTime.c_str()));
	}
}

bool CCastleSiegeWarTimer::ReceiveMessage(CN3UIBase * pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == btn_close)
			Close();
	}

	return true;
}

uint32_t CCastleSiegeWarTimer::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT & ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (Engine->uiTargetBar != NULL && !posSet)
	{
		POINT eventNoticePos;
		Engine->GetUiPos(Engine->uiTargetBar->m_dVTableAddr, eventNoticePos);
		posSet = true;
		SetPos(eventNoticePos.x + Engine->GetUiWidth(Engine->uiTargetBar->m_dVTableAddr) + 5, 50);
	}
	dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
	return dwRet;
}

void CCastleSiegeWarTimer::Open()
{
	SetVisible(true);
}

void CCastleSiegeWarTimer::Close()
{
	SetVisible(false);
}

bool CCastleSiegeWarTimer::OnKeyPress(int iKey)
{
	if (!IsVisible())
		return CN3UIBase::OnKeyPress(iKey);
	
	switch (iKey)
	{
	case DIK_ESCAPE:
		ReceiveMessage(btn_close, UIMSG_BUTTON_CLICK);
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

void CCastleSiegeWarTimer::UpdatePosition()
{
	posSet = false;
	if (Engine->uiTargetBar != NULL && !posSet)
	{
		POINT eventNoticePos;
		Engine->GetUiPos(Engine->uiTargetBar->m_dVTableAddr, eventNoticePos);
		posSet = true;
		SetPos(eventNoticePos.x + Engine->GetUiWidth(Engine->uiTargetBar->m_dVTableAddr) + 5, 50);
	}
}

void CCastleSiegeWarTimer::Start()
{
	m_iStartTime = Engine->pCastleSiegeWar.StartTime;
	m_iRemainingTime = Engine->pCastleSiegeWar.RemainingTime;
	
	if (Engine->pCastleSiegeWar.SiegeType == 0x02) 
	{
		m_byStand->SetVisible(false);
		m_byStarting->SetVisible(true);

		if (!Engine->isSiegeWarActive)
			Engine->isSiegeWarActive = true;
	}

	Engine->m_UiMgr->SetChildFontsEx(Engine->m_UiMgr->uiCastleSiegeWarTimer, xorstr("AERO"), 9, FALSE, FALSE);
	m_txtStartingTime->SetString(string_format(xorstr("Start Time: [%d]"), m_iStartTime));
	m_txtRemainingTime->SetString(string_format(xorstr("Remaining Time: [%d]"), m_iRemainingTime));
	m_txtSiegeOwnerClanName->SetString(string_format(xorstr("Owner Clan: [%s]"), Engine->pCastleSiegeWar.SiegeOwnerClanName.c_str()));
	m_Timer = new CTimer(false);
}

void CCastleSiegeWarTimer::Update()
{
	m_byStand->SetVisible(false);
	m_byStarting->SetVisible(true);

	if (m_iRemainingTime <= 0) 
	{
		if (!Engine->isSiegeWarActive)
			Engine->isSiegeWarActive = true;

		m_iRemainingTime = Engine->pCastleSiegeWar.RemainingTime;
		m_txtRemainingTime->SetString(string_format(xorstr("Remaining Time: [%d]"), m_iRemainingTime));
	}

	m_txtSiegeOwnerClanName->SetString(string_format(xorstr("Owner Clan: [%s]"), Engine->pCastleSiegeWar.SiegeOwnerClanName.c_str()));
}

void CCastleSiegeWarTimer::OpenCastleSiegeWarTimer()
{
	std::string name = Engine->dcpUIF(xorstr("Msoft\\CastleSiegeWar.Msoft"));
	Engine->m_UiMgr->uiCastleSiegeWarTimer = new CCastleSiegeWarTimer();
	Engine->m_UiMgr->uiCastleSiegeWarTimer->Init(this);
	Engine->m_UiMgr->uiCastleSiegeWarTimer->LoadFromFile(Engine->m_BasePath + name.c_str(), N3FORMAT_VER_1068);
	Engine->m_UiMgr->AddChild(Engine->m_UiMgr->uiCastleSiegeWarTimer);
	remove(name.c_str());
	Engine->m_UiMgr->uiCastleSiegeWarTimer->Start();
	Engine->m_UiMgr->uiCastleSiegeWarTimer->SetVisible(false);
}