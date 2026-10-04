#include "TopRightReWarIcon.h"

CUITopRightNewPlug::CUITopRightNewPlug()
{
	m_dVTableAddr = NULL;
	m_btnsituation = NULL;
	m_lottery = NULL;
	m_collectionrace = NULL;
	m_cindirella = NULL;
	m_btndropresult = NULL;
	m_btnlive = NULL;
	m_btnbaselive = NULL;
	BasladimiBaslamadimi = false;
	vector<int>offsets;
	offsets.push_back(0x3B4);
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);

	ParseUIElements();
	InitReceiveMessage();
}

CUITopRightNewPlug::~CUITopRightNewPlug()
{
}

void CUITopRightNewPlug::ParseUIElements()
{
	m_btndropresult = Engine->GetChildByID(m_dVTableAddr, "btn_dropresult");
	m_lottery = Engine->GetChildByID(m_dVTableAddr, "btn_lottery");
	m_collectionrace = Engine->GetChildByID(m_dVTableAddr, "btn_collectionrace");
	m_cindirella = Engine->GetChildByID(m_dVTableAddr, "btn_funclass");
	m_btnbaselive = Engine->GetChildByID(m_dVTableAddr, "Base_Live");
	m_btnlive = Engine->GetChildByID(m_dVTableAddr, "btn_live");
	
	Engine->SetVisible(m_btndropresult, false);
	Engine->SetState(m_btndropresult, UI_STATE_BUTTON_DISABLE);
	Engine->SetVisible(m_lottery, false);
	Engine->SetState(m_lottery, UI_STATE_BUTTON_DISABLE);
	Engine->SetVisible(m_collectionrace, false);
	Engine->SetState(m_collectionrace, UI_STATE_BUTTON_DISABLE);
	Engine->SetVisible(m_cindirella, false);
	Engine->SetState(m_cindirella, UI_STATE_BUTTON_DISABLE);
	Engine->SetVisible(m_btnbaselive, false);
	Engine->SetState(m_btnbaselive, UI_STATE_BUTTON_DISABLE);
	Engine->SetVisible(m_btnlive, false);
	Engine->SetState(m_btnlive, UI_STATE_BUTTON_DISABLE);
}

void CUITopRightNewPlug::DropResultStatus(bool status)
{
	if (this)
	{
		Engine->SetVisible(m_btndropresult, status);
		Engine->SetState(m_btndropresult, status ? UI_STATE_BUTTON_NORMAL : UI_STATE_BUTTON_DISABLE);
	}
}

DWORD uiTopRightNewVTable;
bool CUITopRightNewPlug::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	uiTopRightNewVTable = m_dVTableAddr;
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	else if (pSender == (DWORD*)m_btndropresult)
	{
		if (Engine->uiDropResult == NULL)
			Engine->m_UiMgr->ShowDropResult();
		else {
			if (Engine->IsVisible(Engine->uiDropResult->m_dVTableAddr))
				Engine->uiDropResult->Close();
			else
				Engine->uiDropResult->Open();
		}
	}
	else if (pSender == (DWORD*)m_btnlive)
	{
		ShellExecute(NULL, "open", LiveURL.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}
	else if (pSender == (DWORD*)m_lottery)
	{
		if (Engine->uiLottery != NULL)
		{
			Engine->uiLottery->Open();
			Engine->SetVisible(m_lottery, false);
			Engine->SetState(m_lottery, UI_STATE_BUTTON_DISABLE);
		}
	}
	
	else if (pSender == (DWORD*)m_collectionrace)
	{
		if (Engine->uiCollection != NULL)
			Engine->uiCollection->Open();
	}
	else if (pSender == (DWORD*)m_cindirella)
	{
		HideCindirellaButton();

		if (Engine->uiCindirella != NULL)
			Engine->uiCindirella->Open();
	}
	return true;
}

void CUITopRightNewPlug::UpdateTopLeftVisible(uint8_t Live, std::string LiveURLv)
{
	LiveURL = LiveURLv;
	Engine->SetVisible(m_btnlive, Live);
	Engine->SetState(m_btnlive, Live ? eUI_STATE::UI_STATE_BUTTON_NORMAL : eUI_STATE::UI_STATE_BUTTON_DISABLE);
	Engine->SetVisible(m_btnbaselive, Live);
	Engine->SetState(m_btnbaselive, Live ? eUI_STATE::UI_STATE_BUTTON_NORMAL : eUI_STATE::UI_STATE_BUTTON_DISABLE);
}

void CUITopRightNewPlug::OpenCollectionRaceButton()
{
	Engine->SetVisible(m_collectionrace, true);
	Engine->SetState(m_collectionrace, UI_STATE_BUTTON_NORMAL);
}

void CUITopRightNewPlug::OpenCindirellaButton()
{
	Engine->SetVisible(m_cindirella, true);
	Engine->SetState(m_cindirella, UI_STATE_BUTTON_NORMAL);
}

void CUITopRightNewPlug::HideCollectionRaceButton()
{
	Engine->SetVisible(m_collectionrace, false);
	Engine->SetState(m_collectionrace, UI_STATE_BUTTON_DISABLE);
}

void CUITopRightNewPlug::HideCindirellaButton()
{
	Engine->SetVisible(m_cindirella, false);
	Engine->SetState(m_cindirella, UI_STATE_BUTTON_DISABLE);
}

DWORD Func_RIcon = 0;
void __stdcall uiTopRightNewMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiTopRightNewPlug->ReceiveMessage(pSender, dwMsg);
	__asm
	{
		MOV ECX, uiTopRightNewVTable
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_RIcon
		CALL EAX
	}
}

void CUITopRightNewPlug::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_RIcon = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)uiTopRightNewMessage_Hook;
}
