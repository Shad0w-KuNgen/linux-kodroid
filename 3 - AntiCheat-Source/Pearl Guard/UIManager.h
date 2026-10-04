#pragma once

class CUIManager;
#include "stdafx.h"
#include "Pearl Engine.h"
#include "N3BASE/N3Base.h"
#include "N3BASE/N3BaseFileAccess.h"
#include "N3BASE/N3UIBase.h"
#include "N3BASE/N3UIButton.h"
#include "N3BASE/N3UIString.h"
#include "N3BASE/N3UIList.h"
#include "N3BASE/N3UIEdit.h"
#include "N3BASE/N3UIProgress.h"
#include "N3BASE/N3UIImage.h"
#include "N3BASE/LogWriter.h"
#include "LocalInput.h"
#include "CastleSiegeWarTimer.h"
#include "MessageBoxUIF.h"
#include "MerchantEye.h"
#include "CastleSiegeWar.h"
#include "JuraidMountain.h"
#include "PerkMenu.h"
//#include "RightCorner.h"

typedef std::list<CN3UIBase*>			UIList;
typedef UIList::iterator				UIListItor;
typedef UIList::const_iterator			UIListItorConst;
typedef UIList::reverse_iterator		UIListReverseItor;

class CUIManager : public CN3UIBase
{
public:
	DWORD m_dVTableAddr;
	CN3UIBase* uiBase;
	CLocalInput* localInput;
	CN3UIBase* m_FocusedUI;

	CUIMessageBox* uiMsgBox;
	CUIMerchantEye* uiMerchantEye;
	CUICastleSiegeWar* uiCSW;
	CUIJuraidMountain* uiJuraid;
	CPerkMenu* uiPerkMenu;
	//CUIRightCorner* uiRightCorner;


	vector<PUSItem> item_list;
	vector<PusCategory> cat_list;
	vector<PUSREFUNDITEM> pusrefund_itemlist;
	vector<DailyQuest*> kcbq_quests;

	vector<EventShow> m_eventshowlist;

	UIList		m_Children;
	UIList		m_DestroyQueue;

	bool s_bKeyPressed;
	bool s_bKeyPress;
	uint32_t m_dwMouseFlagsCur;
	bool m_bEnableOperation;

public:
	CUIManager();
	~CUIManager();
	void Init(LPDIRECT3DDEVICE9 lDevice);
	void AddChild(CN3UIBase* pChild, bool focused = false);
	void RemoveChild(CN3UIBase* pChild);
	void SetChildFonts(CN3UIBase* pChild, std::string fontName, int minusFontSize, bool bold, bool italic);
	void SetChildFontsEx(CN3UIBase* pChild, std::string fontName, int fontSize, bool bold, bool italic);
	void Tick();
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	void TickDestroyQueue();
	void TickFocusedUI();
	void Render();
	void IsCursorInOurUIs(POINT currentCursorPos);
	void SetFocusedUI(CN3UIBase* pUI);
	void ReorderChildList();
	CN3UIBase* GetTopUI(bool bVisible);
	void OpenItemInfo();
	void OpenMerchantNotice();
	void HideAllUI();
	void ShowAllUI();
	void PrepareRenderState();
	void RestoreRenderState();
	void InitUifHookVtables();
	void CalcVtable();
	void StaticMemberInit();
	void ProcessUIKeyInput();
	RECT GetScreenRect();
	POINT GetScreenCenter(CN3UIBase* ui);
	void InitProcessLocalInput();
	const POINT MouseGetPos(const POINT& ptCur);
	void SendMouseProc(uint32 dwFlags, const POINT& ptCur, const POINT& ptOld);
	void InitMouseProc();
	void InitUIHideAll();
	void InitUIShowAll();
	void ShowMessageBox(string title, string text, MsgBoxTypes type = MsgBoxTypes::YesNo, ParentTypes parent = ParentTypes::PARENT_NONE);
	void ShowDropList(Packet& pkt);
	void ShowMerchantList(Packet& pkt);
	void ShowDropResult();
	bool IgnoreMouseProc;
	void OpenMonsterSearch();
	void OpenScheduler();
	void OpenDailyQuest();
private:
	DWORD dwZEnable, dwAlphaBlend, dwSrcBlend, dwDestBlend, dwFog;
	DWORD dwMagFilter, dwMinFilter, dwMipFilter;
};