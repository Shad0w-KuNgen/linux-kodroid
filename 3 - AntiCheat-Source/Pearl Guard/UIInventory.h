#pragma once
class CUIInventoryPlug;
#include "stdafx.h"
#include "Pearl Engine.h"

class CUIInventoryPlug
{
public:
	DWORD m_dVTableAddr;
	DWORD slot[24];
	DWORD btn_trash;
	DWORD base_cos;
	DWORD btn_EmblemView, btn_PathosView;
public:
	CUIInventoryPlug();
	~CUIInventoryPlug();
	void ParseUIElements();
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void InitReceiveMessage();
private:

};