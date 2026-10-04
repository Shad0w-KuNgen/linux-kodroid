#include "JuraidMountain.h"

CUIJuraidMountain::CUIJuraidMountain()
{
	grp_minimum = NULL;
	grp_maximum = NULL;
	btn_maximum = NULL;
	icon = NULL;
	btn_cancel = NULL;
	btn_enter = NULL;
	txt_elmo_0 = NULL;
	txt_karu_0 = NULL;
	txt_time = NULL;
	bMinimalize = false;
	timeLapsTick = 0;
	remainingtime = 0;
}

CUIJuraidMountain::~CUIJuraidMountain()
{

}

bool CUIJuraidMountain::Load(HANDLE hFile)
{
	if (CN3UIBase::Load(hFile) == false) return false;

	grp_minimum = (CN3UIBase*)GetChildByID(xorstr("grp_minimum"));
	grp_maximum = (CN3UIBase*)GetChildByID(xorstr("grp_maximum"));
	btn_maximum = (CN3UIButton*)grp_minimum->GetChildByID(xorstr("btn_maximum"));
	icon = (CN3UIButton*)grp_minimum->GetChildByID(xorstr("icon"));
	txt_time = (CN3UIString*)grp_minimum->GetChildByID(xorstr("txt_time"));
	txt_elmo_0 = (CN3UIString*)grp_maximum->GetChildByID(xorstr("txt_elmo_0"));
	txt_karu_0 = (CN3UIString*)grp_maximum->GetChildByID(xorstr("txt_karu_0"));
	btn_enter = (CN3UIButton*)grp_maximum->GetChildByID(xorstr("btn_enter"));
	btn_cancel = (CN3UIButton*)grp_maximum->GetChildByID(xorstr("btn_cancel"));
	SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, Engine->m_UiMgr->GetScreenCenter(this).y);
	return true;
}

void CUIJuraidMountain::JoinOrDisband(bool join)
{
	btn_cancel->SetVisible(join ? true : false);
	btn_enter->SetVisible(join ? false : true);
}

void CUIJuraidMountain::setOptions(bool update, uint16 remtime, uint16 karus, uint16 elmo)
{
	if (!remtime)
		return;

	txt_elmo_0->SetString(std::to_string(elmo));
	txt_karu_0->SetString(std::to_string(karus));

	if (update)
		return;

	JoinOrDisband(false);
	SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, Engine->m_UiMgr->GetScreenCenter(this).y);
	remainingtime = uint64(remtime * 1000 + getMSTime());
	Open();
}

bool CUIJuraidMountain::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (!IsVisible() || !pSender || dwMsg != 0x00000001)
		return false;

	if (pSender == btn_maximum )
	{
		bMinimalize = !bMinimalize;
		grp_maximum->SetVisible(bMinimalize ? false : true);
		return true;
	}

	if (pSender == btn_enter)
	{
		Packet newpkt(WIZ_EVENT, uint8(TEMPLE_EVENT_JOIN));
		Engine->Send(&newpkt);
		return true;
	}

	if (pSender == btn_cancel)
	{
		Packet newpkt(WIZ_EVENT, uint8(TEMPLE_EVENT_DISBAND));
		Engine->Send(&newpkt);
		return true;
	}
	return false;
}

void CUIJuraidMountain::Tick()
{
	if (timeLapsTick > getMSTime())
		return;

	timeLapsTick = getMSTime() + 30;

	if (!IsVisible())
		return;

	uint64 remVal = 0;
	if (remainingtime > getMSTime())
		remVal = remainingtime - getMSTime();

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
}

void CUIJuraidMountain::Open()
{
	grp_minimum->SetVisible(true);
	grp_maximum->SetVisible(true);
	SetVisible(true);
}

void CUIJuraidMountain::Close()
{
	SetVisible(false);
}

