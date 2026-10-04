#pragma once
#include "stdafx.h"
class CUIAccountRegister;
#include "Pearl Engine.h"

class CUIAccountRegister
{
	DWORD txt_otp;
	DWORD txt_seal;
	DWORD txt_phone;
	DWORD txt_email;

	DWORD edit_otp_password;
	DWORD edit_seal_password;
	DWORD edit_phone_number;
	DWORD edit_email;

	DWORD btn_confirm;
	DWORD btn_close;
	DWORD m_dVTableAddr;

public:
	CUIAccountRegister();
	~CUIAccountRegister();
	void InitReceiveMessage();
	bool Load(HANDLE hFile);
	bool ReceiveMessage(DWORD* pSender, uint32_t dwMsg);
	void RefreshText();
	void ParseUIElements();

	void Close();

	void Open();
	void OpenAccountRegister();
};