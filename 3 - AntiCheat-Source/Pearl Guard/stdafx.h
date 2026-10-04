#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define PERK_COUNT 13
#define UI_PERKCOUNT 13

#include <Windows.h>
#include ".//Virtualizer/include/VirtualizerSDK.h"
#include <stdio.h>
#include <string>
#include <algorithm>
#include <detours.h>
#include <vector>
#include <tchar.h>
#include <psapi.h>
#include <atlstr.h>
#include <fstream>
#include "Packet.h"
#include <shellapi.h>
#include <winnetwk.h>
#include <Iphlpapi.h>
#include <list>
#include "md5.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <utility>
#include "xorstr.h"
#include "crc32.h"
//#include <mmsystem.h>
#include <discord_register.h>
#include <discord_rpc.h>
#include <wincrypt.h> 

#pragma comment(lib, "detours.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "detours.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mpr.lib")
#pragma comment(lib, "VirtualizerSDK32.lib")

#define __ASSERT(expr, expMessage) \
if(!(expr)) {\
	printf("ERROR-> %s\n%s: %d\n\n", expMessage, __FILE__, __LINE__);\
}
#define USER_BAND				0			// Map ID User
#define NPC_BAND				8000		// Map ID Npc 
#define INVALID_BAND			MAXSHORT	// Invalid ID BAND 

#define MAX_USER				NPC_BAND

#define ARRAY_SIZE 1024
#define burntime 60

typedef int(__thiscall* tGetObjectBase)(DWORD _KO_FLDB, int targetID, char unk);

using namespace std;

enum UserAuthority {
	BANNED = 255,
	USER = 1,
	GAMEMASTER = 0
};

struct ScreenInfo {
	int height;
	int width;
};

struct ProcessInfo {
	int id;
	string name;
	vector<string> windows;
};

enum class perksSub
{
	info,
	perkPlus,
	perkReset,
	perkUseItem,
	perkTargetInfo
};

enum class perks
{
	weight,
	healt,
	mana,
	loyalty,
	percentDrop,
	percentExp,
	percentCoinsMon,
	percentCoinsSell,
	percentUpgradeChance,
	percentDamageMonster,
	percentDamagePlayer,
	defence,
	attack
};

struct _PERKS
{
	uint32 pIndex;
	uint16 perkCount, maxPerkCount;
	bool status, percentage;
	std::string strDescp;
};

struct _PERKS_DATA
{
	uint16 perkType[PERK_COUNT], remPerk;
	uint32 perkCoins;
	_PERKS_DATA() {
		Initialize();
	}

	void Initialize() {
		memset(perkType, 0, sizeof(perkType));
		remPerk = 0;
		perkCoins = 0;
	}
};

enum class perksError
{
	remPerks,
	notFound,
	maxPerkCount,
	success
};

struct User
{
	string Nick;
	UserAuthority Authority;
	uint32 KnightCash;
	uint32 moneyreq;
	uint32 TlBalance;
	uint32 NationPoint;
	uint8 Level;
	uint8 RebLevel;
	uint8 ddAc;
	uint8 axeAc;
	uint8 swordAc;
	uint8 maceAc;
	uint8 arrowAc;
	uint8 spearAc;
	bool isTeleporting, isRankOpen, RankPacketReceived;
	bool logOut;
	uint8 ZoneID;
	int64 HWID;
	vector<string> GPU;
	string CPU;
	ScreenInfo* Screen;
	vector<ProcessInfo> Processes;
	uint32 MAC;
	uint16 burninglevel;
	ULONGLONG burningtime;
};

enum class EventOpCode
{
	TEMPLE_EVENT_BORDER_DEFENCE_WAR = 4,
	TEMPLE_EVENT_MONSTER_STONE = 14,
	TEMPLE_EVENT_CHAOS = 24,
	TEMPLE_EVENT_JURAD_MOUNTAIN = 100,
	TEMPLE_EVENT_KNIGHT_BATTLE_ROYALE = 104
};

enum TempleOpCodes
{
	BIFROST_EVENT = 2,
	TEMPLE_SCREEN = 3,
	MONSTER_SQUARD = 5,
	MONSTER_STONE = 6,
	TEMPLE_EVENT = 7,
	TEMPLE_EVENT_JOIN = 8,
	TEMPLE_EVENT_DISBAND = 9,
	TEMPLE_EVENT_FINISH = 10,
	MONSTER_STONE_EXP_REWARD = 11,
	TEMPLE_EVENT_COUNTER = 16,
	TEMPLE_EVENT_COUNTER2 = 17,
	TEMPLE_DRAKI_TOWER_ENTER = 33,
	TEMPLE_DRAKI_TOWER_LIST = 34,
	TEMPLE_DRAKI_TOWER_TIMER = 35,
	TEMPLE_DRAKI_TOWER_OUT1 = 36,
	TEMPLE_DRAKI_TOWER_OUT2 = 37,
	TEMPLE_DRAKI_TOWER_TOWN = 38,
	TEMPLE_EVENT_ALTAR_FLAG = 49,
	TEMPLE_EVENT_ALTAR_TIMER = 50,
	TEMPLE_EVENT_DUNGEON_SIGN = 58,
	TEMPLE_EVENT_REMAINING_TOWER = 59,
	TEMPLE_EVENT_STAGE_COUNTER = 60
};

struct _PM_MAP
{
	uint16 sID;
	std::string uName;
	uint8 btype;
	uint8 msgcount;
	DWORD countstr;
	uint8 LastSeen[2];
	time_t tLSWantTime;
	bool MessageCountCheck;
	bool MessageClean;
	bool LastSeenCheck;

	_PM_MAP()
	{
		sID = 0;
		uName = "";
		btype = msgcount = 0;
		LastSeen[0] = 0;
		LastSeen[1] = 0;
		MessageClean = false;
		countstr = 0x00;
		tLSWantTime = GetTickCount();
		LastSeenCheck = false;
		MessageCountCheck = false;
	}
};


enum PUS_CATEGORY
{
	SPECIAL = 1,
	POWER_UP = 2,
	COSTUME = 3,
	BUNDLE = 4
};

struct PUSItem
{
	uint32 ID, ItemID, Price, Quantitiy;
	uint8 isTlBalance;
	PUS_CATEGORY Category;

	PUSItem(uint32 PusID, uint32 PusItemID, uint32 PusPrice, uint8 TlBalance, PUS_CATEGORY PusCategory, uint32 PusQuantitiy)
	{
		ID = PusID;
		ItemID = PusItemID;
		Price = PusPrice;
		Category = PusCategory;
		Quantitiy = PusQuantitiy;
		isTlBalance = TlBalance;
	}
};
struct PusCategory
{
	uint32 sID;
	std::string categoryName;
	uint8 Status;
	PusCategory(uint32 sid, string categoryname, uint8 status) {
		sID = sid;
		Status = status;
		categoryName = categoryname;

	}
};
struct PUSREFUNDITEM {
	uint64 serial; uint32 itemid, kcprice, expiredtime;

	PUSREFUNDITEM(uint64 a, uint32 b, uint32 c, uint32 d) {
		serial = a;
		itemid = b;
		kcprice = c;
		expiredtime = d;
	}
};

struct ITEM_DATA
{
	string itemName;
	uint32 itemID;
	uint16 sCount, sDuration;
	ITEM_DATA(string name, uint32 id, uint16 count, uint16 duration)
	{
		itemName = name;
		itemID = id;
		sCount = count;
		sDuration = duration;
	}
};

struct COLLECTION_RACE_EVENT
{
	uint32 RewardsItemID[3], RewardsItemCount[3], EventTimer;
	uint16 sSid[3], killTarget[3], killCount[3], m_iCompleteUserCount, TotalParticipantUser;
	uint8 Nation;
	uint8 RewardsItemRate[3];
	std::string EventName;
	uint8 sZone;

	COLLECTION_RACE_EVENT()
	{
		for (int i = 0; i < 3; i++)
		{
			sSid[i] = killTarget[i] = killCount[i] = 0;
			RewardsItemID[i] = RewardsItemCount[i] = 0;
		}
		memset(RewardsItemRate, 0, sizeof(RewardsItemRate));
		sZone = 0;
		EventTimer = 0;
		m_iCompleteUserCount = TotalParticipantUser = 0;
		Nation = 0;
		EventName = "";
	}
};

struct ZINDAN_WARINFO
{
	bool start;
	uint32 remtime;
	ZINDAN_WARINFO() {
		Init();
	}
	void Init() {
		start = false;
		remtime = 0;
	}
};

struct _CASTLE_SIEGE_WAR_TIMER
{
	int32 StartTime;
	int16_t RemainingTime;
	uint8_t SiegeType;
	std::string SiegeOwnerClanName;

	_CASTLE_SIEGE_WAR_TIMER()
	{
		StartTime = 0;
		RemainingTime = 0;
		SiegeType = 0;
		SiegeOwnerClanName.clear();
	}
};

struct DROP
{
	string itemName;
	uint16 dropRate;
	DROP(string n, uint16 d) {
		itemName = n;
		dropRate = d;
	}
};

struct DROP_RESULT
{
	string mName;
	vector<DROP> drops;
	string mZone;
	DROP_RESULT(string n, vector<DROP> d, string z) {
		mName = n;
		drops = d;
		mZone = z;
	}
};

struct DropItem
{
	uint32 nItemID;
	uint16 sPercent;

	DropItem(uint32 ItemID, uint16 sPer)
	{
		nItemID = ItemID;
		sPercent = sPer;
	}
};

typedef struct __TABLE_MOB
{
	uint32 ID;
	std::string strName;
	uint32 Model;
	uint8 isBoss;
	uint8 UNK;
} TABLE_MOB;

typedef struct __TABLE_NPC
{
	uint32 ID;
	std::string strName;
	uint32 Model;
	uint8 isBoss;
	uint8 UNK;

} TABLE_NPC;

#define PL_VERSION 5					// Anticheat derlendiðinde burasý deðiþtirilmeli. 

#define WIZ_LOGOUT				0x0F

#define WIZ_LOGIN				0x01	// Account Login
#define WIZ_NEW_CHAR			0x02	// Create Character DB
#define WIZ_DEL_CHAR			0x03	// Delete Character DB
#define WIZ_SEL_CHAR			0x04	// Select Character
#define WIZ_SEL_NATION			0x05	// Select Nation
#define WIZ_MOVE				0x06	// Move ( 1 Second )
#define WIZ_USER_INOUT			0x07	// User Info Insert, delete
#define WIZ_ATTACK				0x08	// General Attack 
#define WIZ_ROTATE				0x09	// Rotate
#define WIZ_NPC_INOUT			0x0A	// Npc Info Insert, delete
#define WIZ_NPC_MOVE			0x0B	// Npc Move ( 1 Second )
#define WIZ_ALLCHAR_INFO_REQ	0x0C	// Account All Character Info Request
#define WIZ_GAMESTART			0x0D	// Request Other User, Npc Info
#define WIZ_MYINFO				0x0E	// User Detail Data Download
#define WIZ_LOGOUT				0x0F	// Request Logout
#define WIZ_CHAT				0x10	// User Chatting..
#define WIZ_DEAD				0x11	// User Dead
#define WIZ_REGENE				0x12	// User	Regeneration
#define WIZ_TIME				0x13	// Game Timer
#define WIZ_WEATHER				0x14	// Game Weather
#define WIZ_REGIONCHANGE		0x15	// Region UserInfo Receive
#define WIZ_REQ_USERIN			0x16	// Client Request UnRegistered User List
#define WIZ_HP_CHANGE			0x17	// Current HP Download
#define WIZ_MSP_CHANGE			0x18	// Current MP Download
#define WIZ_ITEM_LOG			0x19	// Send To Agent for Writing Log
#define WIZ_EXP_CHANGE			0x1A	// Current EXP Download
#define WIZ_LEVEL_CHANGE		0x1B	// Max HP, MP, SP, Weight, Exp Download
#define WIZ_NPC_REGION			0x1C	// Npc Region Change Receive
#define WIZ_REQ_NPCIN			0x1D	// Client Request UnRegistered NPC List
#define WIZ_WARP				0x1E	// User Remote Warp
#define WIZ_ITEM_MOVE			0x1F	// User Item Move
#define WIZ_NPC_EVENT			0x20	// User Click Npc Event
#define WIZ_ITEM_TRADE			0x21	// Item Trade 
#define WIZ_TARGET_HP			0x22	// Attack Result Target HP 
#define WIZ_ITEM_DROP			0x23	// Zone Item Insert
#define WIZ_BUNDLE_OPEN_REQ		0x24	// Zone Item list Request
#define WIZ_TRADE_NPC			0x25	// ITEM Trade start
#define WIZ_ITEM_GET			0x26	// Zone Item Get
#define WIZ_ZONE_CHANGE			0x27	// Zone Change
#define WIZ_POINT_CHANGE		0x28	// Str, Sta, dex, intel, cha, point up down
#define WIZ_STATE_CHANGE		0x29	// User Sitdown or Stand
#define WIZ_LOYALTY_CHANGE		0x2A	// Nation Contribution
#define WIZ_VERSION_CHECK		0x2B	// Client version check 
#define WIZ_CRYPTION			0x2C	// Cryption
#define WIZ_USERLOOK_CHANGE		0x2D	// User Slot Item Resource Change
#define WIZ_NOTICE				0x2E	// Update Notice Alarm 
#define WIZ_PARTY				0x2F	// Party Related Packet
#define WIZ_EXCHANGE			0x30	// Exchange Related Packet
#define WIZ_MAGIC_PROCESS		0x31	// Magic Related Packet
#define WIZ_SKILLPT_CHANGE		0x32	// User changed particular skill point
#define WIZ_OBJECT_EVENT		0x33	// Map Object Event Occur ( ex : Bind Point Setting )
#define WIZ_CLASS_CHANGE		0x34	// 10 level over can change class 
#define WIZ_CHAT_TARGET			0x35	// Select Private Chanting User
#define WIZ_CONCURRENTUSER		0x36	// Current Game User Count
#define WIZ_DATASAVE			0x37	// User GameData DB Save Request
#define WIZ_DURATION			0x38	// Item Durability Change
#define WIZ_TIMENOTIFY			0x39	// Time Adaption Magic Time Notify Packet ( 2 Seconds )
#define WIZ_REPAIR_NPC			0x3A	// Item Trade, Upgrade and Repair
#define WIZ_ITEM_REPAIR			0x3B	// Item Repair Processing
#define WIZ_KNIGHTS_PROCESS		0x3C	// Knights Related Packet..
#define WIZ_ITEM_COUNT_CHANGE   0x3D    // Item cout change.  
#define WIZ_KNIGHTS_LIST		0x3E	// All Knights List Info download
#define WIZ_ITEM_REMOVE			0x3F	// Item Remove from inventory
#define WIZ_OPERATOR			0x40	// Operator Authority Packet
#define WIZ_SPEEDHACK_CHECK		0x41	// Speed Hack Using Check
#define WIZ_COMPRESS_PACKET		0x42	// Data Compressing Packet
#define WIZ_SERVER_CHECK		0x43	// Server Status Check Packet
#define WIZ_CONTINOUS_PACKET	0x44	// Region Data Packet 
#define WIZ_WAREHOUSE			0x45	// Warehouse Open, In, Out
#define WIZ_SERVER_CHANGE		0x46	// When you change the server
#define WIZ_REPORT_BUG			0x47	// Report Bug to the manager
#define WIZ_HOME				0x48    // 'Come back home' by Seo Taeji & Boys
#define WIZ_FRIEND_PROCESS		0x49	// Get the status of your friend
#define WIZ_GOLD_CHANGE			0x4A	// When you get the gold of your enemy.
#define WIZ_WARP_LIST			0x4B	// Warp List by NPC or Object
#define WIZ_VIRTUAL_SERVER		0x4C	// Battle zone Server Info packet	(IP, Port)
#define WIZ_ZONE_CONCURRENT		0x4D	// Battle zone concurrent users request packet
#define WIZ_CORPSE				0x4e	// To have your corpse have an ID on top of it.
#define WIZ_PARTY_BBS			0x4f	// For the party wanted bulletin board service..
#define WIZ_MARKET_BBS			0x50	// For the market bulletin board service...
#define WIZ_KICKOUT				0x51	// Account ID forbid duplicate connection
#define WIZ_CLIENT_EVENT		0x52	// Client Event (for quest)
#define WIZ_MAP_EVENT			0x53
#define WIZ_WEIGHT_CHANGE		0x54	// Notify change of weight
#define WIZ_SELECT_MSG			0x55	// Select Event Message...
#define WIZ_NPC_SAY				0x56
#define WIZ_BATTLE_EVENT		0x57
#define WIZ_AUTHORITY_CHANGE	0x58
#define WIZ_EDIT_BOX			0x59
#define WIZ_SANTA				0x5A

#define WIZ_ITEM_UPGRADE		0x5B
#define WIZ_PACKET1				0x5C
#define WIZ_PACKET2				0x5D
#define WIZ_ZONEABILITY			0x5E	
#define WIZ_EVENT				0x5F
#define WIZ_STEALTH				0x60
#define WIZ_ROOM_PACKETPROCESS	0x61
#define WIZ_ROOM				0x62
#define WIZ_PACKET3				0x63
#define WIZ_QUEST				0x64
#define WIZ_PACKET4				0x65
#define WIZ_KISS				0x66
#define WIZ_RECOMMEND_USER		0x67
#define WIZ_MERCHANT			0x68
#define WIZ_MERCHANT_INOUT		0x69
#define WIZ_SHOPPING_MALL		0x6A
#define WIZ_SERVER_INDEX		0x6B
#define WIZ_EFFECT				0x6C
#define WIZ_SIEGE				0x6D
#define WIZ_NAME_CHANGE			0x6E
#define WIZ_WEBPAGE				0x6F
#define WIZ_CAPE				0x70
#define WIZ_PREMIUM				0x71
#define WIZ_HACKTOOL			0x72
#define WIZ_RENTAL				0x73
#define WIZ_PACKET5				0x74
#define WIZ_CHALLENGE			0x75
#define WIZ_PET					0x76
#define WIZ_CHINA				0x77
#define WIZ_KING				0x78
#define WIZ_SKILLDATA			0x79
#define WIZ_PROGRAMCHECK		0x7A
#define WIZ_BIFROST				0x7B
#define WIZ_REPORT				0x7C
#define WIZ_LOGOSSHOUT			0x7D
#define WIZ_PACKET6				0x7E
#define WIZ_PACKET7				0x7F
#define WIZ_RANK				0x80
#define WIZ_STORY				0x81
#define WIZ_NATION_CHANGE		0x82
#define WIZ_ZONE_TERRAIN		0x83
#define WIZ_PACKET10			0x84
#define WIZ_CAPTURE				0x85
#define WIZ_MINING				0x86
#define WIZ_HELMET				0x87
#define WIZ_PVP					0x88
#define WIZ_CHANGE_HAIR			0x89
#define WIZ_PACKET12			0x8A
#define WIZ_VIPWAREHOUSE		0x8B
#define WIZ_PACKET14			0x8C
#define WIZ_GENDER_CHANGE		0x8D
#define WIZ_PACKET16			0x8E
#define WIZ_PACKET17			0x8F
#define WIZ_DEATH_LIST			0x90
#define WIZ_CLANPOINTS_BATTLE	0x91
#define WIZ_NATION_CHAT			0x19
#define WIZ_GENIE				0x97
#define WIZ_USER_INFO  			0x98
#define WIZ_ACHIEVE				0x99
#define WIZ_PARTY_HP			0xE8
#define WIZ_EXP_SEAL			0x9A
#define WIZ_SP_CHANGE			0x9B
#define WIZ_ORDER				0x9F
#define WIZ_ADD_MSG				0xDB
#define WIZ_NOTICE_SEND			0xC4
#define XSafe					0xE9
#define WIZ_FPS					0xC9

enum XSafeOpCodes
{
	AUTHINFO = 0xA2,
	PROCINFO = 0xA3,
	OPEN = 0xA4,
	LOG = 0xA5,
	ALIVE = 0xA6,
	UIINFO = 0xA7,
	PUS = 0xA8,
	CASHCHANGE = 0xA9,
	KESN = 0xAA,
	DROP_LIST = 0xAB,
	ITEM_PROCESS = 0xAC,
	RESET = 0xAD,
	DROP_REQUEST = 0xAE,
	CR = 0xAF,
	CLANBANK = 0xB0,
	USERINFO = 0xB1,
	KCPAZAR = 0xB2,
	LOOT_SETTINS = 0xB3,
	CHAOTIC_EXCHANGE = 0xB4,
	MERCHANT = 0xB5,
	USERDATA = 0xB7,
	TEMPITEMS = 0xB8,
	KCUPDATE = 0xB9,
	AUTODROP = 0xBA,
	SUPPORT = 0xBB,
	INFOMESSAGE = 0xBC,
	MERCHANTLIST = 0xBD,
	MESSAGE = 0xBE,
	BANSYSTEM = 0xBF, 
	MERC_WIEWER_INFO = 0xC3,
	UPGRADE_RATE = 0xC4,
	CASTLE_SIEGE_TIMER = 0xC5,
	VOICE = 0xC6,
	LOTTERY = 0xC7,
	TOPLEFT = 0xC8,
	ERRORMSG = 0xC9, //30.08.2020
	RESETREBSTAT = 0xCA,
	UNKNOWN1 = 0xCB,
	ACCOUNT_INFO_SAVE = 0xCC,
	CHAT_LASTSEEN = 0xCD,
	SendRepurchaseMsg = 0xCE,
	Send1299SkillAndStatReset = 0xCF,
	AntiAfkList = 0xD0,
	TagInfo = 0xD1,
	ZindanWar = 0xD2,
	DAIL_QUESTS_1098 = 0xD3,
	PusRefund = 0xD4,
	RankSymbol = 0xD5,
	PusCat = 0xD6,
	DeathNotice = 0xD7,
	EventShowList = 0xD8,
	ShowQuestList = 0xD9,
	WheelData = 0xDA,
	GenieInfo = 0xDB,
	QuestBoard = 0xDC,
	MerchantEye = 0xDD,
	SkillQuest = 0xDF,
	CINDIRELLA = 0xE0,
	CSW,
	JURAID,
	PERKS,
	MESSAGE2,
	CHEST_BLOCKITEM,
	RIGHT_CLICK_EXCHANGE
};

enum class cindopcode {
	selectclass,
	nationchange,
	joinevent,
	starting,
	updatekda,
	finish,
	success,
	timewait,
	notchange,
	alreadyclass,
	alreadynation
};

const DWORD KO_DLG = 0x00F3690C;							// 2369
const DWORD KO_SMMF = 0x00520A70;							// 2369
const DWORD KO_SMMB = 0x00F3E74C;							// 2369
const DWORD KO_UIF_FILE_LOAD = 0x00468E60;					// 2369
const DWORD KO_UIF_FILE_LOAD_ORG_CALL = 0x00468A40;			// 2369
const DWORD KO_UIF_FILE_LOAD_RET = 0x00468EBB;				// 2369

const DWORD KO_UI_TOOLTIP_FNC = 0x00506480;					// 2369
const DWORD KO_UIF_ICONITEMSKILL_SET = 0x00524C80;			// 2369
const DWORD KO_UIF_ICONITEMSKILL_CREATE = 0x0050D7E0;		// 2369
const DWORD KO_UIF_SET_N3ICON = 0x006677A0;					// 2369
const DWORD KO_UIF_IMG_SETUVRECT = 0x0041BD00;				// 2369
const DWORD KO_UI_DLG = 0xF368F4;							// 2369
const DWORD KO_UI_MGR = 0xF368E8;							// 2369
const DWORD KO_OFF_PT = 0x340;								// 2369
const DWORD KO_OFF_PTCOUNT = 0x344;							// 2369
const DWORD KO_OFF_PTBASE = 0x238;							// 2369
const DWORD KO_UIF_LOADSTRING = 0x004037B0;

// gereksizler
#define KO_FNCZ			0x00523590
// ---
#define KO_SSID         0xC18
#define KO_PTR_CHR		0x00F3E860							// 2369
#define KO_FLDB			0x00F3E85C							// 2369
#define KO_FMBS			0x0050E390							// 2369
#define KO_FPBS			0x005101D0							// 2369
#define KO_FNSB			0x00520A70							// 2369
#define KO_RECV_PTR		0x00F368D8							// 2369
#define KO_RECV_FUNC	0x00565890							// 2369
// offset
#define KO_OFF_ZONE		0xCE8								// 2369
#define KO_OFF_NAME		0x720								// 2369
#define KO_OFF_NAMELEN	0x730								// 2369
#define KO_OFF_CLASS	0x7B0								// 2369
#define KO_OFF_ID		0x718								// 2369
#define KO_OFF_MP		0xC48								// 2369
#define KO_OFF_X		0xEC								// 2369
#define KO_OFF_Y		0xF0								// 2369
#define KO_OFF_Z		0xF4								// 2369
#define KO_OFF_EXP		0xC60								// 2369
#define KO_OFF_MAXEXP	0xC58								// 2369
#define KO_OFF_NATION	0x740								// 2369
#define KO_OFF_MOB		0x6D4								// 2369
#define KO_OFF_HP		0x754								// 2369
#define KO_OFF_MAXHP	0x750								// 2369
#define KO_OFF_LEVEL KO_OFF_MAXHP - 4

#define KO_OFF_N3CHR	0xD58								//0xCA0  //Animasyon 2369
#define KO_OFF_ANIM_ID	0x2EC								//0x2A0  //Animasyon 2369


#define ZONE_RONARK_LAND		71
#define ZONE_ARDREAM			72
#define ZONE_RONARK_LAND_BASE	73
#define ZONE_ZINDAN_WAR			91

// offset deðil
#define PARTY_CREATE			0x01	// Party Group Create
#define PARTY_PERMIT			0x02	// Party Insert Permit
#define PARTY_INSERT			0x03	// Party Member Insert
#define PARTY_REMOVE			0x04	// Party Member Remove
#define PARTY_DELETE			0x05	// Party Group Delete
#define PARTY_HPCHANGE			0x06	// Party Member HP change
#define PARTY_LEVELCHANGE		0x07	// Party Member Level change
#define PARTY_CLASSCHANGE		0x08	// Party Member Class Change
#define PARTY_STATUSCHANGE		0x09	// Party Member Status ( disaster, poison ) Change
#define PARTY_REGISTER			0x0A	// Party Message Board Register
#define PARTY_REPORT			0x0B	// Party Request Message Board Messages
#define PARTY_PROMOTE			0x1C	// Promotes user to party leader
#define PARTY_ALL_STATUSCHANGE	0x1D	// Sets the specified status of the selected party members to 1.

#define D3DFONT_BOLD        0x0001
#define D3DFONT_ITALIC      0x0002
const   DWORD   KO_IU_MGR_MOUSE_PROC_PTR = 0x00C5CBD0;  // 2369
const	DWORD	KO_IU_MGR_MOUSE_PROC_FUNC = 0x005C3880; // 2369


#define ANTICHEAT_MODE 0		// Anti Cheat korumasýnýn aktif olmasý için "1" , Pasif olmasý için "0" yapýlmasý gerekiyor.
#define HOOK_SOURCE_VERSION 2369	// 1098 Myk0, 1534 , 2369 Son version'dur.

#if (HOOK_SOURCE_VERSION == 1098 || HOOK_SOURCE_VERSION == 1534)
#define PUS_ITEMSCOUNT 3
#define PUS_ITEMPERCOUNT 9
#else
#define PUS_ITEMSCOUNT 4
#define PUS_ITEMPERCOUNT 16
#endif

const DWORD KO_MOVE_TIME = 0x00C55EC4;

const DWORD KO_SET_CHR_ANIM = 0x436750;					// 0x00432760; //Animasyon 2369
typedef int(__thiscall* tChrAniCurSet)(DWORD ecx, int iAni, bool bOnceAndFreeze, float fBlendTime, float fFreezeTime, bool bStopUpperAnimation); //Animasyon 2369
static tChrAniCurSet __ChrAniCurSet = (tChrAniCurSet)KO_SET_CHR_ANIM;  //Animasyon 2369

const DWORD KO_FNC_END_GAME = 0x00A86E41;				// 2369 Oyundan hýzlý çýkma
const DWORD KO_CALL_END_GAME = 0x004CEC90;				// 2369  Oyundan hýzlý çýkma

const	DWORD	KO_UI_HIDE_ALL_CALL_ADDR = 0x005511F6;	// 2369
const	DWORD	KO_UI_HIDE_ALL_FUNC = 0x005682F0;		// 2369

const	DWORD	KO_UI_SHOW_ALL_CALL_ADDR = 0x00551236;	// 2369
const	DWORD	KO_UI_SHOW_ALL_FUNC = 0x0052C390;		// 2369

const   DWORD   KO_GET_CHILD_BY_ID_FUNC = 0x00410DF0;	// 2369

const   DWORD   KO_SET_STRING_FUNC = 0x00429DB0;		// 2369
const	DWORD	KO_SET_STRING_PTR = 0xC5446C;			// 2369

const   DWORD   KO_SET_VISIBLE_FUNC = 0x411980;			// 2369

const	DWORD	KO_UI_TASKBAR_SUB_RECEIVE_MESSAGE_PTR = 0x00C70184;		// (m_dVTableAddr + 0x70)
const	DWORD	KO_UI_TASKBAR_SUB_RECEIVE_MESSAGE_FUNC = 0x00855320;	// 0x008198B0; // (m_dVTableAddr + 0x70)

const	DWORD	KO_UI_MINIMENU_RECEIVE_MESSAGE_PTR = 0x00BA9978;		// (m_dVTableAddr  + 0x70)
const	DWORD	KO_UI_MINIMENU_RECEIVE_MESSAGE_FUNC = 0x00811AC0;		// *(DWORD*)(m_dVTableAddr  + 0x70)

const	DWORD	KO_SET_VISIBLE_MINIMENU_FUNC = 0x0080F7F0;

const	DWORD	KO_SET_VISIBLE_GENIE_SUB_FUNC = 0x00830810;				// 2369
const	DWORD	KO_SET_VISIBLE_GENIE_SUB_PTR = 0x00C6F4A4;				// 2369

const	DWORD	KO_ADD_INFO_MSG_FUNC = 0x4FFFB0;						// 2369

const	DWORD	KO_UI_SET_STATE_FUNC = 0x0040F0D0;						// 2369
const	DWORD	KO_CHAT_ADD_INFO = 0x0006F3B00;							// 2369

const	DWORD	KO_UI_MERCHANT_PRICE_RECEIVE_MESSAGE_PTR = 0xC5F734;	// 2369
const	DWORD	KO_UI_MERCHANT_PRICE_RECEIVE_MESSAGE_FUNC = 0x62D9B0;	// 2369

const	DWORD	KO_CAMERA_ZOOM_FUNC = 0x00A7C580;						// 2369
const	DWORD	KO_CAMERA_ZOOM_CALL_ADDR = 0x00A842E6;					// 2369

const	DWORD	KO_UIBASE_IS_IN_FUNC = 0x0040F800;

// yeni bulunanlar
const	DWORD	KO_UI_TARGETBAR_RECEIVE_MESSAGE_PTR = 0x00BA9B20;

const	DWORD	KO_UI_CHATTING_BOX_RECEIVE_MESSAGE_PTR = 0x00B9FD38;
const	DWORD	KO_UI_CHATTING_BOX_RECEIVE_MESSAGE_FUNC = 0x006DBBC0;

const	DWORD	KO_UI_INFORMATION_BOX_RECEIVE_MESSAGE_PTR = 0x00B9EDA8;
const	DWORD	KO_UI_INFORMATION_BOX_RECEIVE_MESSAGE_FUNC = 0x006B21B0;

const DWORD KO_UI_CHAT_BAR_RECEIVE_MESSAGE_PTR = 0xB9FD38;
const DWORD KO_UI_CHAT_BAR_RECEIVE_MESSAGE_FUNC = 0x6DBBC0;

//00B94580
const	DWORD	KO_UI_LOGIN_RECEIVE_MESSAGE_PTR = 0x00B94580;
const	DWORD	KO_UI_LOGIN_RECEIVE_MESSAGE_FUNC = 0x004E1980;

const	DWORD	KO_UI_ANVIL_RECEIVE_MESSAGE_PTR = 0x00B98470;
const	DWORD	KO_UI_ANVIL_RECEIVE_MESSAGE_FUNC = 0x005D7930;

const DWORD KO_UIF_SET_ITEM_FLAG = 0x00666AD0;

const DWORD KO_UIF_ICONITEMSKILL_OFF_DXT = 0x00000004;//found
const DWORD KO_UIF_ICONITEMSKILL_OFF_RENT_TIME = 0x00000022;//found
const DWORD KO_UIF_ICONITEMSKILL_OFF_RENT_PREIOD = 0x00000024;//found
const DWORD KO_UIF_ICONITEMSKILL_OFF_RENT_PRICE = 0x00000028;//found
const DWORD KO_UIF_ICONITEMSKILL_OFF_EXPIRATION = 0x0000004C;//found
const DWORD KO_UIF_ICONITEMSKILL_OFF_ITEM = 0x00000068;//found
const DWORD KO_UIF_ICONITEMSKILL_OFF_ITEM_EXT = 0x0000006C;//found
const DWORD KO_UIF_ICONITEMSKILL_OFF_ITEM_COUNT = 0x00000070;//found
const DWORD KO_UIF_ICONITEMSKILL_OFF_ITEM_DURATION = 0x00000074;//found

#define UI_STATE 0xE0
#define UI_POS UI_STATE + 0xC
// Anti afk ve kafaya yazý yazma

const DWORD KO_FNC_OBJECT_MOB_LOOP = 0x57D3D1;			// 0x00562C7E; 2369
const DWORD KO_FNC_OBJECT_MOB_LOOP_RET = 0x57D3D6;		// 0x00562C83; 2369
const DWORD KO_FNC_OBJECT_PLAYER_LOOP = 0x00598A80;		// 0x005737C7; 2369
const DWORD KO_FNC_OBJECT_PLAYER_LOOP_RET = 0x591F3C;	// 0x005737CC; 2369
const DWORD KO_FNC_SET_INFO_STRING = 0x5929B0;			// 0x005740A0; 2369
const DWORD KO_UIF_SET_FONT_STRING = 0x458980;			// 0x00452630; 2369
const DWORD orgCall = 0x4BF450;							// 0x004AF120; ANTIAFK 2369
const DWORD KO_FNC_GENIE_SELECT_ORG_CALL = 0x9A8540;	// 0x00927BC0; 2369
const DWORD KO_FNC_GENIE_SCAN = 0x9A9DE9;				// 0x00929379; 2369
const DWORD KO_FNC_GENIE_SELECT_RET = 0x9A9DEE;			// 0x0092937E; 2369
//	---------

#define CONSOLE_FORE_COLOR_BLACK			0
#define CONSOLE_FORE_COLOR_BLUE				1
#define CONSOLE_FORE_COLOR_GREEN			2
#define CONSOLE_FORE_COLOR_CYAN				3
#define CONSOLE_FORE_COLOR_RED				4
#define CONSOLE_FORE_COLOR_MAGENTA			5
#define CONSOLE_FORE_COLOR_BROWN			6
#define CONSOLE_FORE_COLOR_LIGHTGRAY		7
#define CONSOLE_FORE_COLOR_DARKGRAY			8
#define CONSOLE_FORE_COLOR_LIGHTBLUE		9
#define CONSOLE_FORE_COLOR_LIGHTGREEN		10
#define CONSOLE_FORE_COLOR_LIGHTCYAN		11
#define CONSOLE_FORE_COLOR_LIGHTRED			12
#define CONSOLE_FORE_COLOR_LIGHTMAGENTA		13
#define CONSOLE_FORE_COLOR_YELLOW			14
#define CONSOLE_FORE_COLOR_WHITE			15

enum class TOOLTIP_TYPE
{
	EQUIPMENT = 0,
	INVENTORY = 1,
	MERCHANT = 2,
	MINERVA = 21,
	MAGIC_BAG = 22,
	DROP_BOX = 26,
	ITEM_PURCHASING = 27,
	SELLING_ITEM = 28,
	BUY_MERCHANT = 30,
	HANDOUT = 33,
	OBJECT_TO_RECEIVE = 34,
	QUEST_REQUEST = 35,
	QUEST_REWARD = 36,
	QUEST_ADDITIONAL_REWARD = 37,
	SELL_MERCHANT = 161,
	NONE = 400
};

enum class ItemFlag
{
	ITEM_FLAG_NONE = 0,
	ITEM_FLAG_RENTED = 1,
	ITEM_FLAG_CHAR_SEAL = 2,
	ITEM_FLAG_DUPLICATE = 3,
	ITEM_FLAG_SEALED = 4,
	ITEM_FLAG_NOT_BOUND = 7,
	ITEM_FLAG_BOUND = 8
};



enum MSG_TYPE
{
	MSG_ERROR = 0,
	MSG_SUCCESS = 1,
	MSG_WARNING = 2,
	MSG_INFO = 3
};

enum MerchantOpcodes
{
	MERCHANT_OPEN = 1,
	MERCHANT_CLOSE = 2,
	MERCHANT_ITEM_ADD = 3,
	MERCHANT_ITEM_CANCEL = 4,
	MERCHANT_ITEM_LIST = 5,
	MERCHANT_ITEM_BUY = 6,
	MERCHANT_INSERT = 7,
	MERCHANT_TRADE_CANCEL = 8,
	MERCHANT_ITEM_PURCHASED = 9,

	MERCHANT_SLOT_UPDATE = 10,
	MERCHANT_CREATED = 11,
	MERCHANT_SPECIAL_CREATED = 12,

	MERCHANT_BUY_OPEN = 0x21,
	MERCHANT_BUY_INSERT = 0x22,
	MERCHANT_BUY_LIST = 0x23,
	MERCHANT_BUY_BUY = 0x24,
	MERCHANT_BUY_SOLD = 0x25,
	MERCHANT_BUY_BOUGHT = 0x26,
	MERCHANT_BUY_CLOSE = 0x27,
	MERCHANT_BUY_REGION_INSERT = 0x28,
	MERCHANT_MENISIA_LIST = 0x30
};
const uint8 MAX_MERCH_ITEMS = 12;

enum ItemGrade
{
	ITEM_UPGRADE_REQ = 1,
	ITEM_UPGRADE = 2,
	ITEM_ACCESSORIES = 3,
	ITEM_BIFROST_REQ = 4,
	ITEM_BIFROST_EXCHANGE = 5,
	ITEM_UPGRADE_REBIRTH = 7,
	ITEM_SEAL = 8,
	ITEM_CHARACTER_SEAL = 9,
	ITEM_SPECIAL_EXCHANGE = 11,
};

enum ResultOpCodes
{
	Failed = 0,
	Success = 1
};

enum MsgBoxTypes
{
	None,
	Ok,
	YesNo,
	Cancel,
	OkWithEdit,
	YesNoWithEdit
};

enum ParentTypes
{
	PARENT_NONE,
	PARENT_PAGE_STATE,
	PARENT_SKILL_TREE,
	PARENT_LOGIN,
	PARENT_PERK_RESET
};

enum class infomessageop { infobar, box };

#define foreach(itr, arr) \
	for (auto itr = arr.begin(); itr != arr.end(); itr++)

INLINE void STRTOLOWER(std::string& str)
{
	for (size_t i = 0; i < str.length(); ++i)
		str[i] = (char)tolower(str[i]);
};

INLINE void STRTOUPPER(std::string& str)
{
	for (size_t i = 0; i < str.length(); ++i)
		str[i] = (char)toupper(str[i]);
};

#include <mutex>

class SRWLock
{
	unsigned tid;

public:
	SRWLock()
	{
		tid = 0xffffffff;
		InitializeSRWLock(&b_lock);
	}
	void w_lock()
	{
		unsigned me = GetCurrentThreadId();
		if (tid != 0xffffffff)
		{
			if (tid == me)
			{
				printf("#Error: Recursive calls is not suitable for SRWLock, owner %d\n", me);
				ASSERT(0);
			}
		}


		AcquireSRWLockExclusive(&b_lock);
		tid = me;
	}
	void w_unlock()
	{
		tid = 0xffffffff;
		ReleaseSRWLockExclusive(&b_lock);
	}

	void r_lock()
	{
		unsigned me = GetCurrentThreadId();
		if (tid != 0xffffffff)
		{
			if (tid == me)
			{

				printf("#Error: Recursive calls is not suitable for SRWLock, (RLOCK) %d\n", me);
				ASSERT(0);
			}
		}
		AcquireSRWLockShared(&b_lock);

		tid = me;
	}
	void r_unlock()
	{
		tid = 0xffffffff;
		ReleaseSRWLockShared(&b_lock);
	}
public:
	SRWLOCK b_lock;
};
class Guard
{
public:
	Guard(std::recursive_mutex& mutex) : target(mutex) { target.lock(); }
	Guard(std::recursive_mutex* mutex) : target(*mutex) { target.lock(); }
	~Guard() { target.unlock(); }

protected:
	std::recursive_mutex& target;
};

enum class tagerror {
	Open,
	List,
	newtag,
	success,
	noitem,
	already,
	error
};

enum class pusrefunopcode { ireturn, listsend, itemnotfound, timeexpired, procestime, notinventory, itemused, itemreurnsucces, listadd };

#include "STLMap.h"

struct EventShow {
	std::string name, time;
	uint32 hour, minute;
	EventShow(std::string name, std::string time, uint32 hour, uint32 minute) {
		this->name = name;
		this->time = time;
		this->hour = hour;
		this->minute = minute;
	}
};

struct DailyQuest {
	uint8 index;
	uint16 Mob[4];
	uint16 KillTarget;
	uint32 MyCount, replaytime;
	uint32 Reward[4];
	uint32 Count[4];
	uint8 ZoneID;
	uint8 MinLevel;
	uint8 MaxLevel;
	uint8 Status, questtype, killtype;
	int32 remtime;
	DailyQuest()
	{
		index = 0;
		for (uint8 i = 0; i < 4; i++)
		{
			Mob[i] = 0;
			Reward[i] = 0;
			Count[i] = 0;
		}
		KillTarget = 0;
		MyCount = 0;
		ZoneID = 0;
		MinLevel = 0;
		MaxLevel = 83;
		Status = 0;
		remtime = 0;
		questtype = 0;
		replaytime = 0;
		killtype = 0;
	}
};

enum class DailyQuestOp {
	sendlist,
	userinfo,
	killupdate
};
enum class DailyQuesttimetype { repeat, time, single };
enum class DailyQuestStatus { timewait, comp, ongoing };

struct SpellCRC
{
	DWORD part1;
	DWORD part2;
	SpellCRC(DWORD a, DWORD b)
	{
		part1 = a;
		part2 = b;
	}
};

#define ZONE_MORADON			21
#define ZONE_MORADON2			22
#define ZONE_MORADON3			23
#define ZONE_MORADON4			24
#define ZONE_MORADON5			25

#define ZONE_KARUS				1
#define ZONE_ELMORAD			2
#define ZONE_KARUS2				5
#define ZONE_KARUS3				6
#define ZONE_ELMORAD2			7
#define ZONE_ELMORAD3			8

#define ZONE_KARUS_ESLANT		11
#define ZONE_ELMORAD_ESLANT		12
#define ZONE_KARUS_ESLANT2		13
#define ZONE_KARUS_ESLANT3		14
#define ZONE_ELMORAD_ESLANT2	15
#define ZONE_ELMORAD_ESLANT3	16

#define BIFROST 31

#define ZONE_SPBATTLE_BASE		104
#define ZONE_SPBATTLE1			ZONE_SPBATTLE_BASE +1
#define ZONE_SPBATTLE2			ZONE_SPBATTLE_BASE +2
#define ZONE_SPBATTLE3			ZONE_SPBATTLE_BASE +3
#define ZONE_SPBATTLE4			ZONE_SPBATTLE_BASE +4
#define ZONE_SPBATTLE5			ZONE_SPBATTLE_BASE +5
#define ZONE_SPBATTLE6			ZONE_SPBATTLE_BASE +6
#define ZONE_SPBATTLE7			ZONE_SPBATTLE_BASE +7
#define ZONE_SPBATTLE8			ZONE_SPBATTLE_BASE +8
#define ZONE_SPBATTLE9			ZONE_SPBATTLE_BASE +9
#define ZONE_SPBATTLE10			ZONE_SPBATTLE_BASE +10
#define ZONE_SPBATTLE11			ZONE_SPBATTLE_BASE +11

#define ZONE_BATTLE_BASE		60

#define ZONE_BATTLE			ZONE_BATTLE_BASE + 1 // Napies Gorge
#define ZONE_BATTLE2		ZONE_BATTLE_BASE + 2 // Alseids Prairie
#define ZONE_BATTLE3		ZONE_BATTLE_BASE + 3 // Nieds Triangle
#define ZONE_BATTLE4		ZONE_BATTLE_BASE + 4 // Nereid's Island
#define ZONE_BATTLE5		ZONE_BATTLE_BASE + 5 // Zipang
#define ZONE_BATTLE6		ZONE_BATTLE_BASE + 6 // Oreads

INLINE bool isInSpecialEventZone(DWORD nZoneID) {
	return (nZoneID >= ZONE_SPBATTLE1 && nZoneID <= ZONE_SPBATTLE11);
}

INLINE bool isInWarZone(DWORD nZoneID) {
	return (nZoneID >= ZONE_BATTLE && nZoneID <= ZONE_BATTLE6);
}

INLINE bool isInMoradon(uint8 qzoneid) {
	return qzoneid == ZONE_MORADON || qzoneid == ZONE_MORADON2 || qzoneid == ZONE_MORADON3 || qzoneid == ZONE_MORADON4 || qzoneid == ZONE_MORADON5;
}

INLINE bool isInLufersonCastle(uint8 zoneid) {
	return zoneid == ZONE_KARUS || zoneid == ZONE_KARUS2 || zoneid == ZONE_KARUS3;
}

INLINE bool isInKarusEslant(uint8 zoneid) {
	return zoneid == ZONE_KARUS_ESLANT || zoneid == ZONE_KARUS_ESLANT2 || zoneid == ZONE_KARUS_ESLANT3;
}

INLINE bool isInElmoradCastle(uint8 zoneid) {
	return zoneid == ZONE_ELMORAD || zoneid == ZONE_ELMORAD2 || zoneid == ZONE_ELMORAD3;
}
INLINE bool isInElmoradEslant(uint8 zoneid) {
	return zoneid == ZONE_ELMORAD_ESLANT || zoneid == ZONE_ELMORAD_ESLANT2 || zoneid == ZONE_ELMORAD_ESLANT3;
}


INLINE ULONGLONG getMSTime()
{
	return GetTickCount64();

#ifdef _WIN32
#if WINVER >= 0x0600
	typedef ULONGLONG(WINAPI* GetTickCount64_t)(void);
	static GetTickCount64_t pGetTickCount64 = nullptr;

	if (!pGetTickCount64)
	{
		HMODULE hModule = LoadLibraryA("KERNEL32.DLL");
		pGetTickCount64 = (GetTickCount64_t)GetProcAddress(hModule, "GetTickCount64");
		if (!pGetTickCount64)
			pGetTickCount64 = (GetTickCount64_t)GetTickCount;
		FreeLibrary(hModule);
	}


	return pGetTickCount64();
#else
	return GetTickCount();
#endif
#else
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return (tv.tv_sec * SECOND) + (tv.tv_usec / SECOND);
#endif
}