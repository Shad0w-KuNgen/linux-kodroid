#include "UISchedular.h"

CUISchedular::CUISchedular()
{
	m_dVTableAddr = NULL;
	m_btnsituation = NULL;
	m_btndropresult = NULL;
	m_btnbug = NULL;
	btn_lottery = NULL;

	BasladimiBaslamadimi = false;
	vector<int>offsets;
	offsets.push_back(0x50);
	offsets.push_back(0);
	offsets.push_back(0x510);
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);

	ParseUIElements();
	InitReceiveMessage();
}

CUISchedular::~CUISchedular()
{
}

void CUISchedular::ParseUIElements()
{

	std::string find = xorstr("btn_dropresult");
	Engine->GetChildByID(m_dVTableAddr, find, m_btndropresult);
	find = xorstr("btn_bug");
	Engine->GetChildByID(m_dVTableAddr, find, m_btnbug);
	find = xorstr("btn_lottery");
	Engine->GetChildByID(m_dVTableAddr, find, btn_lottery);

	Engine->SetVisible(m_btndropresult, false);
	Engine->SetState(m_btndropresult, UI_STATE_BUTTON_DISABLE);

	/*Engine->SetVisible(btn_lottery, false);
	Engine->SetState(btn_lottery, UI_STATE_BUTTON_DISABLE);*/
}

void CUISchedular::DropResultStatus(bool status)
{
	if (this)
	{
		Engine->SetVisible(m_btndropresult, status);
		Engine->SetState(m_btndropresult, status ? UI_STATE_BUTTON_NORMAL : UI_STATE_BUTTON_DISABLE);
	}
}

DWORD uiSchedularT;
bool CUISchedular::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	uiSchedularT = m_dVTableAddr;
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	if (pSender == (DWORD*)m_btnbug)
	{
		if (Engine->uiState == NULL)
		{
			Engine->m_UiMgr->ShowMessageBox(xorstr("Error"), xorstr("Support is available only after the game has started."), Ok);
			return true;
		}

		
		if (Engine->uiSupport)
			Engine->uiSupport->OpenSupport();
	}
	else if (pSender == (DWORD*)m_btndropresult)
	{
	
			if (Engine->IsVisible(Engine->uiDropResult->m_dVTableAddr))
				Engine->uiDropResult->Close();
			else
				Engine->uiDropResult->Open();
		
	}
	else if (pSender == (DWORD*)btn_lottery)
	{
		if (Engine->uiLottery != NULL)
		{
			Engine->SetVisible(Engine->uiLottery->m_dVTableAddr,true);
			Engine->SetVisible(btn_lottery, false);
			Engine->SetState(btn_lottery, UI_STATE_BUTTON_DISABLE);
		}
	}
	
	return true;
}


DWORD Func_Schedular = 0;
void __stdcall uiSchedular_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiSchedularPlug->ReceiveMessage(pSender, dwMsg);
	__asm
	{
		MOV ECX, uiSchedularT
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_Schedular
		CALL EAX
	}
}

void CUISchedular::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_Schedular = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)uiSchedular_Hook;
}
