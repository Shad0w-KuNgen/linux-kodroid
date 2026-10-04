#pragma once
class CUIMerchantEye;
#include "stdafx.h"
#include "Pearl Engine.h"
#include "Timer.h"
#include "N3BASE/N3UIBase.h"
#include "N3BASE/N3UIString.h"
#include "N3BASE/N3UIButton.h"
#include "N3BASE/N3UIEdit.h"

class CUIMerchantEye : public CN3UIBase
{
public:

	CN3UIButton* btn_buy, * btn_sell, * btn_cancel;
	// Max Group
	CN3UIEdit* search_merchant;
public:
	string eventName;
	CUIMerchantEye();
	~CUIMerchantEye();
	bool Load(HANDLE hFile);
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);

	void Close();
	void Open();
	void Tick();
	std::string lastChar;


};