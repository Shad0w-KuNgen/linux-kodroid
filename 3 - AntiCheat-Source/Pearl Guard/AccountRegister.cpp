#include "AccountRegister.h"
#include "stdafx.h"
#include "Pearl Engine.h"

using namespace std;

CAccountRegister::CAccountRegister()
{
	txt_otp = NULL;
	txt_seal = NULL;
	txt_phone = NULL;
	txt_email = NULL;

	edit_otp_password = NULL;
	edit_seal_password = NULL;
	edit_phone_number = NULL;
	edit_email = NULL;

	btn_confirm = NULL;
	btn_close = NULL;
}

CAccountRegister::~CAccountRegister(){
	txt_otp = NULL;
	txt_seal = NULL;
	txt_phone = NULL;
	txt_email = NULL;

	edit_otp_password = NULL;
	edit_seal_password = NULL;
	edit_phone_number = NULL;
	edit_email = NULL;

	btn_confirm = NULL;
	btn_close = NULL;
}

bool CAccountRegister::Load(HANDLE hFile)
{
	if (CN3UIBase::Load(hFile) == false) return false;

	std::string find = xorstr("edit_otp_password");
	edit_otp_password = (CN3UIEdit*)GetChildByID(find);

	find = xorstr("text_otp_password");
	txt_otp = (CN3UIString*)edit_otp_password->GetChildByID(find);

	find = xorstr("edit_seal_password");
	edit_seal_password = (CN3UIEdit*)GetChildByID(find);

	find = xorstr("text_seal_password");
	txt_seal = (CN3UIString*)edit_seal_password->GetChildByID(find);

	find = xorstr("edit_phone_number");
	edit_phone_number = (CN3UIEdit*)GetChildByID(find);

	find = xorstr("text_phone_number");
	txt_phone = (CN3UIString*)edit_phone_number->GetChildByID(find);

	find = xorstr("edit_email");
	edit_email = (CN3UIEdit*)GetChildByID(find);

	find = xorstr("text_email");
	txt_email = (CN3UIString*)edit_email->GetChildByID(find);

	find = xorstr("btn_confirm");
	btn_confirm = (CN3UIButton*)GetChildByID(find);

	find = xorstr("btn_close");
	btn_close = (CN3UIButton*)GetChildByID(find);

	/*std::string find = xorstr("edit_otp_password");
	CN3UIEdit* tmp = (CN3UIEdit*)GetChildByID(find);
	find = xorstr("text_otp_password");
	txt_otp = (CN3UIString*)tmp->GetChildByID(find);
	edit_otp_password = tmp;

	find = xorstr("edit_seal_password");
	tmp = (CN3UIEdit*)GetChildByID(find);
	find = xorstr("text_seal_password");
	txt_seal = (CN3UIString*)tmp->GetChildByID(find);
	edit_seal_password = tmp;

	find = xorstr("edit_phone_number");
	tmp = (CN3UIEdit*)GetChildByID(find);
	find = xorstr("text_phone_number");
	txt_phone = (CN3UIString*)tmp->GetChildByID(find);
	edit_phone_number = tmp;

	find = xorstr("edit_email");
	tmp = (CN3UIEdit*)GetChildByID(find);
	find = xorstr("text_email");
	txt_email = (CN3UIString*)tmp->GetChildByID(find);
	edit_email = tmp;

	find = xorstr("btn_close");
	btn_close = (CN3UIButton*)GetChildByID(find);

	find = xorstr("btn_confirm");
	btn_confirm = (CN3UIButton*)GetChildByID(find);*/

	SetPos(Engine->m_UiMgr->GetScreenCenter(this).x, Engine->m_UiMgr->GetScreenCenter(this).y);
	//Engine->m_UiMgr->SetFocusedUI(this);
	return true;
}

bool PearlEngine::NumberValid(const std::string &str2)
{
	std::string str = str2;
	STRTOLOWER(str);
	char a[10] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };

	int size = (int)str.length();
	for (int i = 0; i < size; i++)
	{
		if (str.at(i) != a[0]
			&& str.at(i) != a[1]
			&& str.at(i) != a[2]
			&& str.at(i) != a[3]
			&& str.at(i) != a[4]
			&& str.at(i) != a[5]
			&& str.at(i) != a[6]
			&& str.at(i) != a[7]
			&& str.at(i) != a[8]
			&& str.at(i) != a[9])
			return false;
	}
	return true;
}

void CAccountRegister::RefreshText()
{
	if (NULL == edit_email)  return;
	if (NULL == edit_otp_password) return;
	if (NULL == edit_phone_number) return;
	if (NULL == edit_seal_password) return;
	if (NULL == Engine->m_UiMgr->uiAccountRegister) return;

	edit_email->SetString("");
	edit_otp_password->SetString(""); 
	edit_phone_number->SetString("");
	edit_seal_password->SetString("");

	/*if (IsVisible()) {
		if (edit_email->HaveFocus())
			edit_email->KillFocus();
		else if (edit_otp_password->HaveFocus())
			edit_otp_password->KillFocus();
		else if (edit_phone_number->HaveFocus())
			edit_phone_number->KillFocus();
		else if (edit_seal_password->HaveFocus())
			edit_seal_password->KillFocus();
	}*/
}

bool CAccountRegister::ReceiveMessage(CN3UIBase * pSender, uint32_t dwMsg)
{
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (btn_confirm == NULL || btn_close == NULL)
			return false;

		if (pSender == btn_confirm) {
			//süre

			if (edit_email == NULL)  return false;
			if (edit_otp_password == NULL) return false;
			if (edit_phone_number == NULL) return false;
			if (edit_seal_password == NULL) return false;
			if (Engine->m_UiMgr->uiAccountRegister == NULL) return false;

			if (txt_otp == NULL) return false;
			if (txt_seal == NULL) return false; 
			if (txt_phone == NULL) return false;
			if (txt_email == NULL) return false;

			string email = txt_email->GetString();
			string phone = txt_phone->GetString();
			string seal = txt_seal->GetString();
			string otp = txt_otp->GetString();

			if (email.empty()) {
				Engine->WriteInfoMessageExt((char*)string_format(xorstr("Account Register : Email address cannot be left blank.")).c_str(), 0xa30000);
				RefreshText();
				return false;
			}
			
			if (phone.empty()) {
				Engine->WriteInfoMessageExt((char*)string_format(xorstr("Account Register : Phone number cannot be left blank.")).c_str(), 0xa30000);
				RefreshText();
				return false;
			}

			if (phone.size() != 11) {
				Engine->WriteInfoMessageExt((char*)string_format(xorstr("Account Register : You must enter your phone number correctly. Ex: 05320001122")).c_str(), 0xa30000);
				RefreshText();
				return false;
			}

			if (!Engine->NumberValid(phone)) {
				Engine->WriteInfoMessageExt((char*)string_format(xorstr("Account Register : You must enter your phone number correctly. Ex: 05320001122")).c_str(), 0xa30000);
				RefreshText();
				return false;
			}

			std::string numberformat = phone.substr(0, 2);
			if (numberformat != "05") {
				Engine->WriteInfoMessageExt((char*)string_format(xorstr("Power Up Store : Your phone number must start with 05. Ex: 05320001122")).c_str(), 0xa30000);
				RefreshText();
				return false;
			}

			if (seal.empty()) {
				Engine->WriteInfoMessageExt((char*)string_format(xorstr("Account Register : Item seal password cannot be left blank. Ex: 12345678")).c_str(), 0xa30000);
				RefreshText();
				return false;
			}

			if (seal.size() != 8) {
				Engine->WriteInfoMessageExt((char*)string_format(xorstr("Account Register : The item seal password must consist of a maximum of 8 digits. Ex: 12345678")).c_str(), 0xa30000);
				RefreshText();
				return false;
			}

			if (!Engine->NumberValid(seal)) {
				Engine->WriteInfoMessageExt((char*)string_format(xorstr("Account Register : You must enter your item seal password correctly. Ex: 12345678")).c_str(), 0xa30000);
				RefreshText();
				return false;
			}

			if (otp.empty()) {
				Engine->WriteInfoMessageExt((char*)string_format(xorstr("Account Register : OTP number cannot be left blank. Ex: 123456")).c_str(), 0xa30000);
				RefreshText();
				return false;
			}

			if (otp.size() != 6) {
				Engine->WriteInfoMessageExt((char*)string_format(xorstr("Account Register : The OTP number must consist of a maximum of 6 digits. Ex: 123456")).c_str(), 0xa30000);
				RefreshText();
				return false;
			}

			if (!Engine->NumberValid(otp)) {
				Engine->WriteInfoMessageExt((char*)string_format(xorstr("Account Register : You must enter your OTP number correctly. Ex: 123456")).c_str(), 0xa30000);
				RefreshText();
				return false;
			}

			Packet test(XSafe, uint8(XSafeOpCodes::ACCOUNT_INFO_SAVE));
			test.DByte();
			test << uint8(1) << email << phone << seal << otp;
			Engine->Send(&test);
		}
		else if (pSender == btn_close) {
			Close();
			Packet test(XSafe, uint8(XSafeOpCodes::ACCOUNT_INFO_SAVE));
			test << uint8(2);
			Engine->Send(&test);
		}
	}
	return true;
}

uint32_t CAccountRegister::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
	return dwRet;
}

void CAccountRegister::Open()
{
	SetVisible(true);
}

void CAccountRegister::Close()
{
	//SetVisible(false);

	Engine->m_UiMgr->RemoveChild(this);
	Engine->m_UiMgr->uiAccountRegister->Release();
	Engine->m_UiMgr->uiAccountRegister = NULL;
}

bool CAccountRegister::OnKeyPress(int iKey)
{
	if (Engine->m_UiMgr->uiAccountRegister == NULL)
		return CN3UIBase::OnKeyPress(iKey);

	if (!IsVisible() || !(Engine->m_UiMgr->m_FocusedUI == this))
		return CN3UIBase::OnKeyPress(iKey);

	switch (iKey)
	{
	case DIK_ESCAPE:
		//Close();
		return true;
		break;
	}

	return CN3UIBase::OnKeyPress(iKey);
}

void CAccountRegister::OpenAccountRegister()
{
	std::string name = Engine->dcpUIF(xorstr("Msoft\\AccountRegister.Msoft"));
	Engine->m_UiMgr->uiAccountRegister = new CAccountRegister();
	Engine->m_UiMgr->uiAccountRegister->Init(this);
	Engine->m_UiMgr->uiAccountRegister->LoadFromFile(Engine->m_BasePath + name.c_str(), N3FORMAT_VER_1068);
	Engine->m_UiMgr->AddChild(Engine->m_UiMgr->uiAccountRegister);
	remove(name.c_str());
}