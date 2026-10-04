#pragma once
class CAccountRegister;
#include "stdafx.h"
#include "Pearl Engine.h"
#include "N3BASE/N3UIBase.h"
#include "N3BASE/N3UIString.h"
#include "N3BASE/N3UIButton.h"

class CAccountRegister : public CN3UIBase
{
	CN3UIString* txt_otp;
	CN3UIString* txt_seal;
	CN3UIString* txt_phone;
	CN3UIString* txt_email;

	CN3UIEdit* edit_otp_password;
	CN3UIEdit* edit_seal_password;
	CN3UIEdit* edit_phone_number;
	CN3UIEdit* edit_email;

	CN3UIButton* btn_confirm;
	CN3UIButton* btn_close;
public:
	CAccountRegister();
	~CAccountRegister();
	bool Load(HANDLE hFile);
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	void RefreshText();
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	bool OnKeyPress(int iKey);
	void Close();
	void Open();
	void OpenAccountRegister();
};