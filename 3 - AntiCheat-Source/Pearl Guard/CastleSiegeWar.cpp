#include "CastleSiegeWar.h"

CUICastleSiegeWar::CUICastleSiegeWar()
{
	txt_castle_owner = NULL;
	txt_csw_remaining_time = NULL;
	m_Timer = NULL;
	m_iRemainingTime = 0;
	minimalize = false;
	set = false;
	timeLapsProgress = 0;


	base_max = NULL;
	btn_max = NULL;
	txt_time = NULL;
	txt_clan = NULL;
	base_time = NULL;
	time = NULL;
	SRect = RECT();
}

CUICastleSiegeWar::~CUICastleSiegeWar()
{

}

bool CUICastleSiegeWar::Load(HANDLE hFile)
{
	if (CN3UIBase::Load(hFile) == false)
		return false;

	base_max = (CN3UIBase*)GetChildByID(xorstr("base_max"));
	btn_max = (CN3UIButton*)GetChildByID(xorstr("btn_max"));
	txt_time = (CN3UIString*)base_max->GetChildByID(xorstr("txt_time"));
	txt_clan = (CN3UIString*)base_max->GetChildByID(xorstr("txt_clan"));
	base_time = (CN3UIBase*)base_max->GetChildByID(xorstr("base_time"));
	time = (CN3UIProgress*)base_time->GetChildByID(xorstr("time"));
	time->SetRange(0, 60);

	txt_time->SetString("");

	/*RECT CRect = Engine->GetScreenRect();
	SRect = CRect;*/

	POINT koScreen = *(POINT*)0x00DE297C; RECT rct{};
	
	rct.left = 0;
	rct.top = 0;
	rct.right = koScreen.x;
	rct.bottom = koScreen.y;
	SRect = rct;

	POINT pos{};
	pos.x = (rct.right - rct.left) - 400;
	pos.y = (((rct.bottom - rct.top) / 2) / 2 / 2) - 50;
	SetPos(pos.x, pos.y);

	return true;
}

bool CUICastleSiegeWar::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == btn_max)
		{
			minimalize = !minimalize;
			base_max->SetVisible(minimalize ? false : true);
			//SetRegion(minimalize ? btn_max.GetdwTable() : GetdwTable());
			return true;
		}
	}
	return true;
}

bool CheckChangeClientSize(RECT CRect, RECT SRect)
{
	return CRect.right != SRect.right
		|| CRect.left != SRect.left
		|| CRect.bottom != SRect.bottom
		|| CRect.top != SRect.top;
}

void CUICastleSiegeWar::Tick()
{
	if (!IsVisible())
		return;

	POINT koScreen = *(POINT*)0x00DE297C; RECT rct{};
	rct.left = 0;
	rct.top = 0;
	rct.right = koScreen.x;
	rct.bottom = koScreen.y;

	if (CheckChangeClientSize(rct, SRect)) {
		SRect = rct;
		POINT pos{};
		pos.x = (rct.right - rct.left) - 400;
		pos.y = (((rct.bottom - rct.top) / 2) / 2 / 2) - 50;
		SetPos(pos.x, pos.y);
	}

	uint64 remVal = 0;
	if (m_iRemainingTime > getMSTime())
		remVal = m_iRemainingTime - getMSTime();

	if (!remVal)
	{
		SetVisible(false);
		return;
	}

	int seconds = remVal / 1000;
	remVal %= 1000;
	int minutes = seconds / 60;
	seconds %= 60;

	if ((seconds + minutes <= 0))
	{
		SetVisible(false);
		return;
	}

	std::string remtime = "";
	if (minutes < 10 && seconds < 10)
		remtime = string_format(xorstr("0%d : 0%d"), minutes, seconds);
	else if (minutes < 10)
		remtime = string_format(xorstr("0%d : %d"), minutes, seconds);
	else if (seconds < 10)
		remtime = string_format(xorstr("%d : 0%d"), minutes, seconds);
	else
		remtime = string_format(xorstr("%d : %d"), minutes, seconds);

	txt_time->SetString(string_format("%s", remtime.c_str()));

	if (!set
		&& getMSTime() > timeLapsProgress
		&& IsVisible())
	{
		float n = 0.0f;
		if (m_iRemainingTime > getMSTime())
			n = (float)(m_iRemainingTime - getMSTime()) / 1000.0f;

		if (n >= 0.0f)
			time->SetCurValue(n, 0);
		else {
			set = true;
			time->SetCurValue(0, 0);
		}
		timeLapsProgress = getMSTime() + 500;
	}


	/*if (m_iRemainingTime > 0)
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

		txt_csw_remaining_time->SetString(string_format(xorstr("%s"), remainingTime.c_str()));
	}
	else
	{
		SetVisible(false);
	}*/
}

void CUICastleSiegeWar::setOptions(uint32 remtime, std::string knights_name, uint8 OpStatus, uint32 war_time)
{
	if (!remtime) {
		m_iRemainingTime = 0;
		return;
	}

	if (knights_name.empty())
		knights_name = "' '";

	set = false;
	time->SetRange(0, war_time * 60);
	m_iRemainingTime = (1000 * remtime) + getMSTime();

	float n = 0.0f;
	if (m_iRemainingTime > getMSTime())
		n = (float)(m_iRemainingTime - getMSTime()) / 1000.0f;
	if (n >= 0.0f) time->SetCurValue(n, 0);
	else time->SetCurValue(0, 0);

	if (txt_clan)
		txt_clan->SetString(knights_name);
	Open();

	/*if (reset)
	{
		m_iRemainingTime = 0;	
		if (txt_castle_owner)
			txt_castle_owner->SetString("");
		
		Close();
		return;
	}

	m_iRemainingTime = remtime;
	if (txt_castle_owner)
		txt_castle_owner->SetString(owner.c_str());
	Open();*/
}

void CUICastleSiegeWar::Open()
{
	base_max->SetVisible(true);
	SetVisible(true);
}

void CUICastleSiegeWar::Close()
{
	m_iRemainingTime = 0;
	if (txt_clan) txt_clan->SetString("");
	SetVisible(false);
}