#pragma once
class COldStyleTaskbar;

#include "stdafx.h"
#include "Pearl Engine.h"
#include "N3BASE/N3UIBase.h"
#include "N3BASE/N3UIString.h"
#include "N3BASE/N3UIButton.h"
#include "N3BASE/N3UIProgress.h"
#include "N3BASE/N3UIArea.h"

class COldStyleTaskbar : public CN3UIBase {

	CN3UIButton* m_btninventory;
	CN3UIButton* m_btncharacter;
	CN3UIButton* m_btnskill;
	CN3UIButton* m_btntrade;
	CN3UIButton* m_btncommand;
	CN3UIButton* m_btninvite;
	CN3UIButton* m_btnattack;
	CN3UIButton* m_btnwalk;
	CN3UIButton* m_btnrun;
	CN3UIButton* m_btnsit;
	CN3UIButton* m_btnstand;
	CN3UIButton* m_btncamera;
	CN3UIButton* m_btndisband;
	CN3UIButton* m_btnexit;
	CN3UIButton* m_btnmap;


public:
	COldStyleTaskbar();
	~COldStyleTaskbar();
	bool Load(HANDLE hFile);
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	bool OnKeyPress(int iKey);
	void UpdatePosition();
	bool Ayaktami;
	bool Oturuyormu;
	void SitAndStand();
};
