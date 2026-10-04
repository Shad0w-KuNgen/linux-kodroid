#include "stdafx.h"
#include "UIScoreBoard.h"

CUIScoreBoardPlug::CUIScoreBoardPlug()
{
	Ename = Kname = EScore = KScore = 0;

	m_Timer = NULL;
	vector<int>offsets;
	offsets.push_back(0x30); 
	offsets.push_back(0);
	offsets.push_back(0x5B8);
	offsets.push_back(0);
	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);
	base_clan = 0;
	base_runa = 0;
	ParseUIElements();
	InitReceiveMessage();
}

CUIScoreBoardPlug::~CUIScoreBoardPlug()
{

}

void CUIScoreBoardPlug::ParseUIElements()
{
	base_clan = Engine->GetChildByID(m_dVTableAddr, "base_clan");
	base_runa = Engine->GetChildByID(m_dVTableAddr, "base_runa");
	Ename = Engine->GetChildByID(base_clan, "str_clan_name_1");
	Kname = Engine->GetChildByID(base_clan, "str_clan_name_2");
	EScore = Engine->GetChildByID(base_clan, "str_clan_score_1");
	KScore = Engine->GetChildByID(base_clan, "str_clan_score_2");
	base_time = Engine->GetChildByID(base_clan, "base_time");
	RemTime = Engine->GetChildByID(base_time, "str_time");
}

DWORD ScoreBoardVTable;
bool CUIScoreBoardPlug::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	ScoreBoardVTable = m_dVTableAddr;
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	return true;
}


DWORD Func_ScoreBoard;
void __stdcall ScoreBoardReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiScoreBoard->ReceiveMessage(pSender, dwMsg);

	__asm
	{
		MOV ECX, ScoreBoardVTable
		PUSH dwMsg
		PUSH pSender
		CALL Func_ScoreBoard
	}
}

void CUIScoreBoardPlug::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_ScoreBoard = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)ScoreBoardReceiveMessage_Hook;
}

void CUIScoreBoardPlug::Tick() {
	if (Engine->pzindanwar.start) {
		Engine->SetVisible(m_dVTableAddr, Engine->pzindanwar.remtime > 0);
		if (Engine->IsVisible(m_dVTableAddr)){
			if (m_Timer->IsElapsedSecond()) Engine->pzindanwar.remtime--;

			uint16_t remainingMinutes = (uint16_t)ceil(Engine->pzindanwar.remtime / 60);
			uint16_t remainingSeconds = Engine->pzindanwar.remtime - (remainingMinutes * 60);
			std::string remainingTime;
			if (remainingMinutes < 10 && remainingSeconds < 10)
				remainingTime = string_format(xorstr("0%d:0%d"), remainingMinutes, remainingSeconds);
			else if (remainingMinutes < 10)
				remainingTime = string_format(xorstr("0%d:%d"), remainingMinutes, remainingSeconds);
			else if (remainingSeconds < 10)
				remainingTime = string_format(xorstr("%d:0%d"), remainingMinutes, remainingSeconds);
			else remainingTime = string_format(xorstr("%d:%d"), remainingMinutes, remainingSeconds);

			if (Engine->pzindanwar.remtime) Engine->SetString(Engine->uiScoreBoard->RemTime,string_format(xorstr("%s"), remainingTime.c_str()));
			if (!Engine->pzindanwar.remtime) {
				Engine->SetVisible(m_dVTableAddr, false);
				Engine->pzindanwar.Init();
			}
		}
	}
}