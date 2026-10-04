#include "stdafx.h"
#include "UIMinimap.h"

CUIMinimapPlug::CUIMinimapPlug()
{
	vector<int>offsets;
	offsets.push_back(0x3E4);
	offsets.push_back(0);
	str_zoneid = 0;
	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);

	str_zoneid = Engine->GetChildByID(m_dVTableAddr, "str_zoneid");
	text_xy = Engine->GetChildByID(m_dVTableAddr, "Text_Position");
	btn_StateBar = Engine->GetChildByID(m_dVTableAddr, "btn_StateBar");
}

CUIMinimapPlug::~CUIMinimapPlug()
{
}

void CUIMinimapPlug::Tick()
{
	if (m_dVTableAddr != NULL)
	{ 
		Engine->SetVisible(m_dVTableAddr, false);
		Engine->SetVisible(text_xy, false);
		Engine->SetVisible(str_zoneid, false);
		Engine->SetVisible(btn_StateBar, false);
	}
}
