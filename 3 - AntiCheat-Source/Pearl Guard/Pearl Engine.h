#pragma once
#include "stdafx.h"
#include "CItem.h"
#include "Discord.h"
#include "Pearl Engine.h"
#include "PlayerBase.h"
#include "TableManager.h"
#include "SettingsManager.h"
#include "StringHelper.h"
#include "N3BASE/N3Base.h"
#include "N3BASE/N3UIBase.h"
#include "UIManager.h"
#include "LocalInput.h"
#include "UIState.h"
#include "UITargetBar.h"
#include "DropResult.h"
#include "UIHpBarMenu.h"
#include "hPowerUpStore.h"
#include "UITaskbarMain.h"
#include "UITaskbarSub.h"
#include "hEventShowList.h"
#include "UIClanWindow.h"
#include "UITradeItemDisplay.h"
#include "UIMiniMenu.h"
#include "UIGenieSub.h"
#include "CharacterToolTip.h"
#include "CollectionRace.h"
#include "UISeedHelper.h"
#include "UIPieceChange.h"
#include "hMerchantList.h"
#include "hSearchMonsterDrop.h"
#include "UILogin.h"
#include "Support.h"
#include "UITradePrice.h"
#include "UITradeInventory.h"
#include "UITradeItemDisplaySpecial.h"
#include "MerchantManager.h"
#include "UITooltip.h"
#include "UIInventory.h"
#include "CTagChange.h"
#include "hWhellOfFun.h"
#include "UIPartyBBS.h"
#include "UIHPBar.h"
#include "UIPMHook.h"
#include "TopRightReWarIcon.h"
#include "UIMinimap.h"
#include "HookSkillPage.h"
#include "UIScoreBoard.h"
#include "UIDailyQuests.h"
#include "UIInformationWind.h"
#include "CSpell.h"
#include "FunctionGuard.h"
#include "UISchedular.h"
#include "UIGenieMain.h"
#include "UIChatBar.h"
#include "UINoticeMsgWind.h"
#include "PlayerRank.h"
#include "hLotteryEvent.h"
#include "hAccountRegister.h"
#include "QuestComplated.h"
#include "RightClickExchange.h"
#include "Cindirella.h"
#include <signal.h>

struct LotteryEvent
{
	uint32 remainingTime;
	uint32 participant;
	uint32 limit;
	LotteryEvent()
	{
		remainingTime = 0;
		participant = 0;
		limit = 1;
	}
};
#define foreach(itr, arr) \
	for (auto itr = arr.begin(); itr != arr.end(); itr++)
struct MerchantItem
{
	uint32 itemBase;
	uint8 bSrcPos;
	bool isKC;
	uint32 price;
};

struct MerchantItemView
{

	uint8 bSrcPos[8];
	bool isKC[8];
	uint32 price[8];

	MerchantItemView()
	{
		for (int i = 0; i < 8; i++)
		{
			bSrcPos[i] = 0;
			isKC[i] = 0;
			price[i] = 0;
		}

	}

};
struct MerchantEyeData
{
	uint8 nIndex;
	std::string strMerchantItem[12];
	MerchantEyeData()
	{
		for (int i = 0; i < 12; i++)
			strMerchantItem[i] = "";
		nIndex = 0;
	}
};
struct tagName
{
	std::string tag;
	uint8 r, g, b;
	tagName() : tag("-"), r(255), g(255), b(255) {};
};

class PearlEngine 
{
public:
	FunctionGuard fncGuard;

	DWORD varius;
	DWORD GetTarget(uint16 getid);
	DWORD GetTarget();
	_PERKS_DATA pPerks;
	std::map<uint32, _PERKS*> m_PerksArray;
	bool bDisableAllSkillFX, bDisableAreaSkillFX, bDisableHealFX;
	bool DeathAll, DeathMe, DeathParty, DeathNone;
	bool bDisableViewEmblem, bDisableViewPathos;
	std::recursive_mutex tagLock;
	std::map<uint16, tagName> tagList;
	void OpenjuraidScreen(bool update, uint16 karus, uint16 elmo, uint16 remtime);
	time_t timesPassed;

	Discord* dc;
	DWORD hkCampAdress;
	DWORD hkServerNotice;
	LotteryEvent lotteryEvent;

	CSTLMap <_PM_MAP, std::string> pmusers;
	std::string accountID;
	vector<HookPM*> privatemessages;
	DWORD isAgree;
	std::map<uint32, CSpell> skillmap;
	std::map<uint32, CSpell> skillmapBackup;
	std::map<uint32, SpellCRC> skillcrc;
	bool _InitIcon(DWORD& iconObject, DWORD parent, DWORD itemID);
	DWORD _CreateIconItemSkill(DWORD self, DWORD UIIcon, DWORD itemID, short count, uint32 expiration = 0, uint16 rental_remTime = 0, uint32 rental_period = 0, uint32 rental_price = 0);
	uint32 serverhour, serverminute, serversecond;
	uint8 GetNation();
	PearlEngine(std::string basePath);
	virtual ~PearlEngine();
	bool power, render;
	static DWORD WINAPI EngineMain(PearlEngine* e);
	static DWORD WINAPI SuspendCheck(PearlEngine* e);
	static DWORD WINAPI TitleCheck(PearlEngine* e);
	DWORD KOGetTarget();
	uint16 KOGetSSID;
	HANDLE MainThread, ScanThread, SuspendThread, LisansThread, AliveThread;
	HANDLE TitleThread;
	std::string m_connectedIP;
	bool IsHWBreakpointExists();
	bool IsitaSandBox();
	User Player;
	bool Loading;
	bool TitleWindow(int id, LPCSTR WindowTitle);
	DWORD hpBarAdress;
	bool LoqOut;
	bool LoadingControl;
	void HandleGameStart(Packet &pkt);
	void HandleZoneChange(Packet &pkt);

	void HandleAccountInfoSave(Packet &pkt);
	void HandleChaoticExchange(Packet &pkt);
	void HandleLastSeen(Packet &pkt);
	void ChaoticExchangeStop();
	void ChaoticExchangeSucces();
	void RecvAntiAfkList(Packet &pkt);
	void RecvWheelData(Packet& pkt);
	void HandleTagName(Packet&pkt);
	void addnewtagid(uint16 sockid, tagName tag);
	bool NumberValid(const std::string &str2);
	bool StringValid(const std::string &str2);

	void HandlePusRefund(Packet &pkt);

	std::string GetZoneName(uint16 zoneID);
	DWORD Adress;
	void Disconnect();
	void Send(Packet* pkt);
	void Update();
	POINT lastkoScreen;
	POINT curlastkoScreen;
	bool InjectLogin();
	void StayAlive();
	void SendProcess(uint16 toWHO);
	bool fileExist(const char* fileName);
	void SetTexImage(DWORD uif, const std::string& szFN);
	void __stdcall Shutdown(std::string log);
	bool WordGuardSystem(std::string Word, uint8 WordStr);
	void SetVisible(DWORD vTable, bool type);
	void SetUiRegion(DWORD vTable, RECT rc);
	void SetString(DWORD vTable, std::string str);
	void UIScreenCenter(DWORD vTable);
	void SetUIStyle(DWORD dwTable ,DWORD Style);
	void SetStringColor(DWORD vTable, DWORD color);
	std::string GetString(DWORD vTable);
	std::string GetPartyString(DWORD vTable);
	std::string GetStringElementName(DWORD vTable);
	std::string GetStringFromPWEdit(DWORD vTable);
	void WriteString(DWORD value, char* vl);
	bool IsVisible(DWORD vTable);
	void GetChildByID(DWORD vTable, std::string id, DWORD& child);
	DWORD GetChildByID(DWORD vTable, std::string id);
	void AddListString(DWORD vTable, const std::string& szString, DWORD color);
	void ClearListString(DWORD vTable);
	void SetRegion(DWORD dwTable, DWORD uif);
	void _ShowToolTip(DWORD spItem, LONG x, LONG y, TOOLTIP_TYPE type);

	/*Merchant Eye*/
	std::map<uint16, MerchantEyeData> MerchantEyeData;
	std::map<uint16, uint16> MerchantID;
	std::recursive_mutex MerchantEyeLock;
	std::recursive_mutex merchantItemsPriceLock;
	std::recursive_mutex MerchantEyeLockData;
	std::map<uint8, MerchantItem> merchantItems;
	std::map<uint16, MerchantItemView> merchantItemsPrice;
	/*Merchant Eye*/
	int16 GetListSelect(DWORD vTable);
	void EditKillFocus(DWORD vTable);
	void GetBaseByChild(DWORD vTable, DWORD& base);
	void ShowMsg(MSG_TYPE type, std::string msg, ...);
	DWORD rdwordExt(DWORD ulBase);
	DWORD rdword(DWORD ulBase, std::vector<int> offsets);
	DWORD ReadDWORD(DWORD ulBase, std::vector<int> offsets);
	void WriteInfoMessageExt(char* pMsg, DWORD dwColor);
	void WriteChatAddInfo(DWORD pColor, bool isBold, const char* pText, uint8 nRank = 0);
	void WriteInfoMessage(char* pMsg, DWORD dwColor);
	void GetUiPos(DWORD vTable, POINT& pt);
	RECT GetUiRegion(DWORD vTable);
	RECT GetUiRegion2(DWORD vTable);
	POINT GetUiPos(DWORD vTable);
	POINT GetUiPos2(DWORD vTable);
	void SetUIPos(DWORD vTable, POINT pt);
	void UiPost(DWORD vTable, LONG x, LONG y);
	void SetState(DWORD vTable, DWORD state);
	DWORD GetRecvMessagePtr(DWORD adres);
	uint32 GetState(DWORD vTable);
	void InitCallHook(DWORD hookFuncAddr, DWORD myFuncAddr);
	void SetMemArray(DWORD Adres, BYTE Deger, DWORD len);
	inline void CALLHOOK(DWORD Addr1, DWORD Addr2);
	DWORD CalculateCallAddrWrite(DWORD Addr1, DWORD Addr2);
	void SendChaoticExchange(uint16 nObjectID, uint32 ExchangeRobItemID, int8 ExchangeRobItemSlot);
	std::string exeGetName();
	bool disableCameraZoom;
	void InitCameraZoom();
	std::string KoRandomNameUIF(std::string uif);
	void InitJmpHook(DWORD hookFuncAddr, DWORD myFuncAddr);
	inline void JMPHOOK(DWORD Addr1, DWORD Addr2);
	void InitSendHook();
	void InitRecvHook();
	inline BYTE GetByte(char* sBuf, int& index);
	inline DWORD GetDWORD(char* sBuf, int& index);
	inline int GetShort(char* sBuf, int& index);
	void SendItemAdd(uint32 itemID, uint16 count, uint32 gold, uint8 srcPos, uint8 dstPos, uint8 mode);
	void PacketCheckThreadID(uint8 packet, DWORD ThreadID);
	void InitSetString();
	bool IsIn(DWORD vTable, int x, int y);
	void SetItemFlag(DWORD vTable, uint8 flag);
	POINT lastMousePos;
	LONG GetUiWidth(DWORD vTable);
	LONG GetUiHeight(DWORD vTable);
	size_t GetScrollValue(DWORD vTable);
	void SetScrollValue(DWORD vTable, int val);
	static long SelectRandomMob();
	CPlayerBase* m_PlayerBase;
	int16 GetSocketID();
	DWORD GetPartyLeaderBase();
	int16 GetPartyLeaderTarget();
	DWORD merchantdisplaydata;
	int16 GetPartyLeaderID();
	bool isInParty();
	std::map<DWORD, DWORD> mReturnAdressSend;
	DWORD merchantdisplayItem[4];
	DWORD merchantdisplaySpecialdata;
	DWORD merchantdisplaySpecialItem[8];
	CTableManager* tblMgr;
	CSettingsManager* m_SettingsMgr;
	std::string m_BasePath;
	bool logState;
	bool drawMode;
	bool _lowPower;
	LPDIRECT3DDEVICE9 _pDevice;
	bool IsCRActive;
	COLLECTION_RACE_EVENT pCollectionRaceEvent;
	ZINDAN_WARINFO pzindanwar;

	bool isSiegeWarStart, isSiegeWarActive;
	_CASTLE_SIEGE_WAR_TIMER pCastleSiegeWar;

	//INLINE bool isInited() { return m_bSelectedCharacter; }
	/*INLINE bool isHookStart() { return m_bHookStart; }
	INLINE bool isGameStart() { return m_bGameStart; }*/

	void SetCurValue(DWORD dwTable, float value, float changeSpeed);
	void SetProgressRange(DWORD uif, float min, float max);
	void ChangeRunState();
	void CommandCameraChange();
	std::map<uint8, uint32> WheelItemData;
	bool UPanelStart;
	bool m_bSelectedCharacter;
	bool m_bGameStart;
	bool m_bHookStart;
	bool m_isGenieStatus;
	bool m_bInParty;
	bool m_LoginPlugInjected;
	ULONGLONG m_recentdelete_time;

	time_t m_bCheckSleep;
	std::string GetItemDXT(int nItemID);
	std::string number_format(int dwNum, bool dot = true);
	void str_replace(std::string& str, std::string find, std::string replace);
	void str_replace(std::string& str, char find, char replace);
	bool str_contains(std::string str, std::string find);
	void str_split(std::string const& str, const char delim, std::vector<std::string>& out);
	void str_split(std::string str, std::string delim, std::vector<std::string>& out);
	std::vector<std::string> str_split(std::string str, std::string regex);
	void str_tolower(std::string& str);
	std::string dcpUIF(std::string path);
	std::string GetEditString(DWORD vTable);
	void SetEditString(DWORD vTable, const std::string& szString);
	void ShowToolTipEx(uint32 itemID, LONG x, LONG y);
	
	void maptest();//sait
	std::string strClientName;

	uint32 moneyReq;
	DWORD m_zMob;
	CStringHelper* StringHelper;
	CUIManager* m_UiMgr;
	CDropResult* uiDropResult;
	CUIStatePlug* uiState;
	CUITargetBarPlug* uiTargetBar;
	CUITaskbarMainPlug* uiTaskbarMain;
	CUITaskbarSubPlug* uiTaskbarSub;
	CUIClanWindowPlug* uiClanWindowPlug;
	CUIMiniMenuPlug* uiMiniMenuPlug;
	HookPlayerRank* uiPlayerRank;
	CUIMoticeMsgWnd* uiNoticeWind;
	CUIGenieSubPlug* uiGenieSubPlug;
	CUISeedHelperPlug* uiSeedHelperPlug;
	CUIPieceChangePlug* uiPieceChangePlug;
	CUIAccountRegister* uiRegister;
	CUILogin* uiLogin;
	CUITradePricePlug* uiTradePrice;
	CUITradeInventoryPlug* uiTradeInventory;
	CUITradeItemDiplayPlug* uiTradeItemDisplay;
	CUITradeItemDisplaySpecialPlug* uiTradeItemDisplaySpecial;
	CMerchantManager* m_MerchantMgr;
	CUITooltip* uiToolTip;
	CUIInventoryPlug* uiInventoryPlug;
	CUIPartyBBSPlug* uiPartyBBS;
	CUIHPBarPlug* uiHPBarPlug;
	CEventShowListPlug* uiEventShowList;
	CUIHPBarMenuPlug* uiHpMenuPlug;
	CUIAccountRegister* uiAccountRegisterPlug;
	CUITopRightNewPlug* uiTopRightNewPlug;
	CUISchedular* uiSchedularPlug;
	CUIMinimapPlug* uiMinimapPlug;
	CUIChatBarPlug* uiChatBarPlug;
	CUIScoreBoardPlug* uiScoreBoard;
	CUIGenieMain* uiGenieMain;
	CUIMerchantListPlug* uiMerchantList;
	HookQuestPage* uiQuestPage;
	CSealToolTip* uiSealToolTip;
	CUIPowerUpStore* uiPowerUpStore;
	CUIInformationWindPlug* uiInformationWind;
	CUISupportReportPlug* uiSupport;
	CUICollectionRacePlug* uiCollection;
	CUILotteryEventPlug* uiLottery;
	CUICindirellaPlug* uiCindirella;
	CUIWheelOfFun* uiWheel;
	CUISearchMonsterDropPlug* uiSearchMonster;
	CTagChange* uiTagChange;
	HookSkillPage* uiSkillPage;
	CUIQuestComplated* uiQuestComplated;
	RightClickExchange* hkRightClickExchange;;
	struct CindirellaInfo {
		bool prepare;
		int8 index;
		uint16 mykill_count, mydead_count, e_kill_count, k_kill_count;
		uint64 remainingtime, selected_remtime;
		CindirellaInfo() {
			Initialize();
		}
		void Initialize() {
			prepare = false;
			index = -1;
			mykill_count = mydead_count = e_kill_count = k_kill_count = 0;
			remainingtime = selected_remtime = 0;
		}
	};
	CindirellaInfo m_cindirellaInfo;
private:
	void InitPlayer();
	
};

extern PearlEngine * Engine;