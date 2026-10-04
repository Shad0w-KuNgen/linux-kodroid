#include "hAccountRegister.h"

CUIAccountRegister::CUIAccountRegister()
{
	m_dVTableAddr = NULL;

	vector<int>offsets;
	offsets.push_back(0x2C0);	// 2369	co_DuelList.uif	Satýr : 60
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);

	 edit_otp_password = NULL;
	 edit_seal_password = NULL;
	 edit_phone_number = NULL;
	 edit_email = NULL;
	 btn_confirm = NULL;
	 btn_close = NULL;

	ParseUIElements();
	InitReceiveMessage();
}

CUIAccountRegister::~CUIAccountRegister()
{
}

void CUIAccountRegister::ParseUIElements()
{
	Engine->GetChildByID(m_dVTableAddr, "edit_otp_password", edit_otp_password);
	Engine->GetChildByID(m_dVTableAddr, "edit_seal_password", edit_seal_password);
	Engine->GetChildByID(m_dVTableAddr, "edit_phone_number", edit_phone_number);
	Engine->GetChildByID(m_dVTableAddr, "edit_email", edit_email);
	Engine->GetChildByID(m_dVTableAddr, "btn_confirm", btn_confirm);
	Engine->GetChildByID(m_dVTableAddr, "btn_close", btn_close);
}

DWORD Func_Register;
DWORD uiRegister;
bool CUIAccountRegister::ReceiveMessage(DWORD* pSender, uint32_t dwMsg)
{
	uiRegister = m_dVTableAddr;
	if (!pSender || pSender == 0 || dwMsg != UIMSG_BUTTON_CLICK)
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (btn_confirm == NULL || btn_close == NULL)
			return false;

		if (pSender == (DWORD*)btn_confirm) {

			if (edit_email == NULL)  return false;
			if (edit_otp_password == NULL) return false;
			if (edit_phone_number == NULL) return false;
			if (edit_seal_password == NULL) return false;
		
			string email = Engine->GetEditString(edit_email).c_str();
			string phone = Engine->GetEditString(edit_phone_number).c_str();
			string seal = Engine->GetEditString(edit_seal_password).c_str();
			string otp = Engine->GetEditString(edit_otp_password).c_str();

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
		else if (pSender == (DWORD*)btn_close) {
			Close();
			Packet test(XSafe, uint8(XSafeOpCodes::ACCOUNT_INFO_SAVE));
			test << uint8(2);
			Engine->Send(&test);
		}
	}


	return true;
}
void CUIAccountRegister::Open()
{
	Engine->SetVisible(m_dVTableAddr,true);
}

void CUIAccountRegister::Close()
{
	Engine->SetVisible(m_dVTableAddr,false);
}
bool PearlEngine::NumberValid(const std::string& str2)
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

void CUIAccountRegister::RefreshText()
{
	if (NULL == edit_email)  return;
	if (NULL == edit_otp_password) return;
	if (NULL == edit_phone_number) return;
	if (NULL == edit_seal_password) return;


	Engine->SetEditString(edit_email, "");
	Engine->SetEditString(edit_otp_password, "");
	Engine->SetEditString(edit_phone_number, "");
	Engine->SetEditString(edit_seal_password, "");
}

void __stdcall UIAccountRegisterReceiveMessage_Hook(DWORD* pSender, uint32_t dwMsg)
{
	Engine->uiAccountRegisterPlug->ReceiveMessage(pSender, dwMsg);
	__asm
	{
		MOV ECX, uiRegister
		PUSH dwMsg
		PUSH pSender
		MOV EAX, Func_Register
		CALL EAX
	}
}

void CUIAccountRegister::InitReceiveMessage()
{
	DWORD ptrMsg = Engine->GetRecvMessagePtr(m_dVTableAddr);
	Func_Register = *(DWORD*)ptrMsg;
	*(DWORD*)ptrMsg = (DWORD)UIAccountRegisterReceiveMessage_Hook;
}