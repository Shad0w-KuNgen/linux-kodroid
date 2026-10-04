#pragma once
class CUIMinimapPlug;
#include "stdafx.h"
#include "Pearl Engine.h"

class CUIMinimapPlug
{
public:
	DWORD m_dVTableAddr;
	DWORD str_zoneid;
	DWORD text_xy;
	DWORD btn_StateBar;
public:
	CUIMinimapPlug();
	~CUIMinimapPlug();
	void Tick();

private:

};