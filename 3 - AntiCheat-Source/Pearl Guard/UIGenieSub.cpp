#include "stdafx.h"
#include "UIGenieSub.h"

CUIGenieSubPlug::CUIGenieSubPlug()
{
	vector<int>offsets;
	offsets.push_back(0x30);
	offsets.push_back(0);
	offsets.push_back(0x53C);
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);
	ParseUIElements();
}

CUIGenieSubPlug::~CUIGenieSubPlug()
{

}
void CUIGenieSubPlug::ParseUIElements()
{
	m_textGenieTime = Engine->GetChildByID(m_dVTableAddr, "str_progress");
	TargetSam = Engine->GetChildByID(m_dVTableAddr, "TargetSam");
	TargetTwo = Engine->GetChildByID(m_dVTableAddr, "TargetTwo");
	
	RECT ASD = Engine->GetUiRegion2(m_dVTableAddr);
	ASD.bottom = ASD.bottom - 25;
	Engine->SetUiRegion(m_dVTableAddr, ASD);
}