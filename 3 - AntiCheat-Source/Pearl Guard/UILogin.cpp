#include "UILogin.h"

uint32 idLastState, pwLastState;

CUILogin::CUILogin()
{
	if (Engine->m_SettingsMgr == NULL) {
		Engine->m_SettingsMgr = new CSettingsManager();
		Engine->m_SettingsMgr->Init();
	}

	vector<int>offsets;
	offsets.push_back(0x30);
	offsets.push_back(0); //offsettt

	m_dVTableAddr = Engine->rdword(0x00F368F4, offsets);

	m_bGroupLogin = 0;
	m_btnRememberID = 0;
	m_EditPW = 0;
	m_EditUID = 0;
	m_btnLogin = 0;

	ParseUIElements();
}

CUILogin::~CUILogin()
{
	m_bGroupLogin = 0;
	m_btnRememberID = 0;
	m_EditPW = 0;
	m_EditUID = 0;
	m_btnLogin = 0;
}

DWORD uiLoginVTable;
DWORD Func_UILogin = 0;
void __stdcall UiLoginReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiLogin->ReceiveMessage(pSender, dwMsg);
	__asm
	{
		MOV ECX, uiLoginVTable
		PUSH dwMsg
		PUSH pSender
		CALL Func_UILogin
	}
}
extern DWORD KO_UI_SCREEN_SIZE;
void CUILogin::ParseUIElements()
{
	m_bGroupLogin = Engine->GetChildByID(m_dVTableAddr, "Group_Login");

	if (m_bGroupLogin == NULL) {
		return;
	}

	m_btnRememberID = Engine->GetChildByID(m_bGroupLogin, "btn_remember");
	idLastState = Engine->GetState(m_btnRememberID);

	m_btnLogin = Engine->GetChildByID(m_bGroupLogin, "btn_ok");
	m_EditUID = Engine->GetChildByID(m_bGroupLogin, "Edit_ID");
	m_EditPW = Engine->GetChildByID(m_bGroupLogin, "Edit_PW");
	Group_ServerList_01 = Engine->GetChildByID(m_dVTableAddr, "Group_ServerList_01");
	Group_introduction_el = Engine->GetChildByID(m_dVTableAddr, "Group_introduction_el");
	Group_introduction_ka = Engine->GetChildByID(m_dVTableAddr, "Group_introduction_ka");
	premium = Engine->GetChildByID(m_dVTableAddr, "premium");

	if (Engine->m_SettingsMgr->m_uID.length() > 0 && m_EditUID)
	{
		Engine->SetEditString(m_EditUID, Engine->m_SettingsMgr->m_uID);
	}

	if (Engine->m_SettingsMgr->m_uPW.length() > 0 && m_EditPW)
		Engine->SetEditString(m_EditPW, Engine->m_SettingsMgr->m_uPW);

	if (Engine->m_SettingsMgr->m_uID.length() > 0 || Engine->m_SettingsMgr->m_uPW.length() > 0)
		Engine->SetState(m_btnRememberID, UI_STATE_BUTTON_DOWN);

	InitReceiveMessage();

	POINT koScreen = *(POINT*)KO_UI_SCREEN_SIZE;

#if (HOOK_SOURCE_VERSION == 1098)
	if (true)
	{
		POINT ret;
		POINT org = Engine->GetUiPos(m_bGroupLogin);
		ret.x = org.x;
		ret.y = koScreen.y - Engine->GetUiHeight(m_bGroupLogin);
		Engine->SetUIPos(m_bGroupLogin, ret);
	}
	if (true)
	{
		POINT ret;
		POINT org = Engine->GetUiPos(Group_ServerList_01);
		ret.x = 0;
		ret.y = 0;
		Engine->SetUIPos(Group_ServerList_01, ret);
		ret.y = 0;
		ret.x = koScreen.x - Engine->GetUiWidth(Group_introduction_el);
		Engine->SetUIPos(Group_introduction_el, ret);
		
		ret.y = Engine->GetUiHeight(Group_introduction_el) + 25 ;
		Engine->SetUIPos(Group_introduction_ka, ret);
		
		RECT region = Engine->GetUiRegion(premium);
	
		ret.x = (koScreen.x / 2) - 513;
		ret.y = 0;
		Engine->SetUIPos(premium, ret);
	}
#elif (HOOK_SOURCE_VERSION == 1534)
	if (true)
	{
		POINT ret;
		POINT org = Engine->GetUiPos(m_bGroupLogin);
		ret.x = 710;
		ret.y = 150;
		Engine->SetUIPos(m_bGroupLogin, ret);
	}
	if (true)
	{
			POINT ret;
			POINT org = Engine->GetUiPos(Group_ServerList_01);
			ret.x = 710;
			ret.y = 200;
			Engine->SetUIPos(Group_ServerList_01, ret);
		
	}
#endif
}

bool CUILogin::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	uiLoginVTable = m_dVTableAddr;
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	if (pSender == (DWORD*)m_btnLogin)
	{
		if (Engine->GetState(m_btnRememberID) == UI_STATE_BUTTON_DOWN)
		{
			Engine->m_SettingsMgr->m_uID = Engine->GetEditString(m_EditUID);
			Engine->m_SettingsMgr->m_uPW = Engine->GetEditString(m_EditPW);
			Engine->m_SettingsMgr->Save();
		}
		else {
			Engine->m_SettingsMgr->m_uID = "";
			Engine->m_SettingsMgr->m_uPW = "";
			Engine->m_SettingsMgr->Save();
		}

		/*std::string accountID = Engine->GetEditString(m_EditUID);
		if (accountID.size())
		{
			Engine->accountID = accountID;
			STRTOUPPER(Engine->accountID);
		}*/
	}

	return true;
}

void CUILogin::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_UILogin = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)UiLoginReceiveMessage_Hook;
}