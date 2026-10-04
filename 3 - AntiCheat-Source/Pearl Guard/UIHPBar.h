#pragma once
#include "stdafx.h"
class CUIHPBarPlug;
#include "Pearl Engine.h"

class CUIHPBarPlug
{
public:
	CUIHPBarPlug();
	~CUIHPBarPlug();

	void ParseUIElements();
	CTimer* m_Timer;
	DWORD m_dVTableAddr;
	DWORD text_hp;
	//DWORD text_fps;
	//DWORD text_ping;
	DWORD text_mp;
	DWORD str_exp;
	DWORD text_exp;
	DWORD progress_exp, progress_Sexp;
	DWORD text_nick, text_level,text_cordinate;

	DWORD Text_burning;
	DWORD progress_flame;
	DWORD base_burning_frame;

	int64 MaxExp;

	int nflexptime;

	void HPChange(uint16 hp, uint16 maxhp);
	void ExpChange(int64 exp, int64 maxhp);
	void BurningChange(uint16 ilevel);
	void MPChange(uint16 mp, uint16 maxmp);
	void Tick();
};