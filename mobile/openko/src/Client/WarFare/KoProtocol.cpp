// KoProtocol.cpp — bkz. KoProtocol.h. Düzenler ISTIRAP v2369 sunucu kaynağından türetilmiştir:
//   LogInServer/LoginSession.cpp (HandleServerlist/HandleLogin), LoginServer.cpp (UpdateServerList)
//   GameServer/LoginHandler.cpp (VersionCheck), shared/KOSocket.cpp (SendCompressed)
//   GameServer/DatabaseThread.cpp + DBAgent.cpp (ReqAllCharInfo/LoadCharInfo)
//   GameServer/UserInfoSystem.cpp (SendMyInfo, GetUserInfo), Npc.cpp (GetNpcInfo)
//   GameServer/CharacterMovementHandler.cpp (MoveProcess), CharacterSelectionHandler.cpp (GameStart)
#include "StdAfx.h"
#include "KoProtocol.h"
#include "PacketDef.h"
#include "APISocket.h"

#include <shared/Ini.h>

#include <algorithm>
#include <cstring>
#include <filesystem>

namespace
{
int g_version   = KoProto::V1298;
int g_loginPort = KoProto::LOGIN_PORT_1298;
int g_gamePort  = KoProto::GAME_PORT_1298;

// Sunucunun ByteBuffer'ı: varsayılan uint16 uzunluklu dizge (DByte), SByte() sonrası uint8 uzunluk.
bool ReadStrD(Packet& pkt, std::string& out)
{
	uint16_t len = pkt.read<uint16_t>();
	if (pkt.rpos() + len > pkt.size())
		return false;
	return pkt.readString(out, len);
}

bool ReadStrS(Packet& pkt, std::string& out)
{
	uint8_t len = pkt.read<uint8_t>();
	if (pkt.rpos() + len > pkt.size())
		return false;
	return pkt.readString(out, len);
}

bool Remaining(const Packet& pkt, size_t need)
{
	return pkt.rpos() + need <= pkt.size();
}

void AddByte(std::vector<uint8_t>& out, uint8_t v)
{
	out.push_back(v);
}
void AddU16(std::vector<uint8_t>& out, uint16_t v)
{
	out.push_back((uint8_t) (v & 0xFF));
	out.push_back((uint8_t) (v >> 8));
}
void AddStrD(std::vector<uint8_t>& out, const std::string& s)
{
	AddU16(out, (uint16_t) s.size());
	out.insert(out.end(), s.begin(), s.end());
}
void AddStrS(std::vector<uint8_t>& out, const std::string& s)
{
	AddByte(out, (uint8_t) std::min<size_t>(s.size(), 255));
	out.insert(out.end(), s.begin(), s.begin() + std::min<size_t>(s.size(), 255));
}

// MYINFO ve USER_INOUT'un ortak kısmı: ittifak/kulüp + pelerin bloğu.
// MYINFO : u16 ittifak, u8 bayrak, str(u8) ad, u8 derece, u8 sıra, u16 işaret sürümü, u16 pelerin, rgb, u8 bayrak
// USERIN : u16 ittifak, str(u8) ad, u8 derece, u8 sıra, u16 işaret sürümü, u16 pelerin, rgb, u8 0, u8 bayrak
bool ReadClan(Packet& pkt, KoProto::ClanInfo2369& c, bool myInfoLayout)
{
	if (!Remaining(pkt, 3))
		return false;
	c.allianceID = pkt.read<uint16_t>();
	if (myInfoLayout)
		c.flag = pkt.read<uint8_t>();
	if (!ReadStrS(pkt, c.name))
		return false;
	if (!Remaining(pkt, 1 + 1 + 2 + 2 + 4 + (myInfoLayout ? 0 : 1)))
		return false;
	c.grade       = pkt.read<uint8_t>();
	c.ranking     = pkt.read<uint8_t>();
	c.markVersion = pkt.read<uint16_t>();
	c.capeID      = pkt.read<uint16_t>();
	c.capeR       = pkt.read<uint8_t>();
	c.capeG       = pkt.read<uint8_t>();
	c.capeB       = pkt.read<uint8_t>();
	c.capeFlag    = pkt.read<uint8_t>();
	if (!myInfoLayout)
		c.flag = pkt.read<uint8_t>();
	return true;
}
} // namespace

namespace KoProto
{
int Version()
{
	return g_version;
}
bool Is2369()
{
	return g_version == V2369;
}
int LoginPort()
{
	return g_loginPort;
}
int GamePort()
{
	return g_gamePort;
}

void Configure(int version, int loginPort, int gamePort)
{
	g_version   = (version == V2369) ? V2369 : V1298;
	g_loginPort = loginPort > 0 ? loginPort : (Is2369() ? LOGIN_PORT_2369 : LOGIN_PORT_1298);
	g_gamePort  = gamePort > 0 ? gamePort : (Is2369() ? GAME_PORT_2369 : GAME_PORT_1298);
}

int InferVersionFromFiles(int filesVersion)
{
	return filesVersion >= 2000 ? V2369 : V1298;
}

int LoadFromServerIni(const std::string& iniPath)
{
	std::string path = iniPath;
#if !defined(_WIN32)
	path = KoResolvePath(iniPath); // büyük/küçük harf duyarsız (Server.ini / server.ini)
#endif
	CIni ini;
	int version = V1298, loginPort = 0, gamePort = 0;
	if (ini.Load(std::filesystem::path(path)))
	{
		int files = ini.GetInt("Version", "Files", 0);
		version   = ini.GetInt("Server", "Protocol", InferVersionFromFiles(files));
		loginPort = ini.GetInt("Server", "LoginPort", 0);
		gamePort  = ini.GetInt("Server", "GamePort", 0);
	}
	Configure(version, loginPort, gamePort);
	return g_version;
}

// ---- Login sunucusu ------------------------------------------------------------------------
bool ParseServerList2369(Packet& pkt, std::vector<ServerInfo2369>& out)
{
	out.clear();
	if (!Remaining(pkt, 3))
		return false;
	/*uint16_t echo =*/pkt.read<uint16_t>();
	int count = pkt.read<uint8_t>();
	for (int i = 0; i < count; i++)
	{
		ServerInfo2369 s;
		if (!ReadStrD(pkt, s.lanIP) || !ReadStrD(pkt, s.ip) || !ReadStrD(pkt, s.name))
			return false;
		if (!Remaining(pkt, 2 * 5 + 2))
			return false;
		s.users         = pkt.read<int16_t>();
		s.serverID      = pkt.read<int16_t>();
		s.groupID       = pkt.read<int16_t>();
		s.playerCap     = pkt.read<int16_t>();
		s.freePlayerCap = pkt.read<int16_t>();
		/*uint8_t zero =*/pkt.read<uint8_t>();
		s.screenType    = pkt.read<uint8_t>();
		if (!ReadStrD(pkt, s.karusKing) || !ReadStrD(pkt, s.karusNotice) || !ReadStrD(pkt, s.elmoradKing)
			|| !ReadStrD(pkt, s.elmoradNotice))
			return false;
		out.push_back(std::move(s));
	}
	return true;
}

bool ParseLoginReply2369(Packet& pkt, LoginReply2369& out)
{
	out = LoginReply2369 {};
	if (!Remaining(pkt, 3))
		return false;
	/*uint16_t zero =*/pkt.read<uint16_t>();
	out.result = pkt.read<uint8_t>();
	if (out.result == LOGIN_2369_SUCCESS)
	{
		if (Remaining(pkt, 2))
			out.premium = pkt.read<int16_t>();
		ReadStrD(pkt, out.account);
	}
	else if (out.result == LOGIN_2369_IN_GAME)
	{
		if (ReadStrD(pkt, out.inGameIP) && Remaining(pkt, 2))
		{
			out.inGamePort = pkt.read<uint16_t>();
			ReadStrD(pkt, out.account);
		}
	}
	return true;
}

// ---- Oyun sunucusu: giriş ---------------------------------------------------------------
bool ParseVersionCheck2369(Packet& pkt, int& version)
{
	if (!Remaining(pkt, 3))
		return false;
	/*uint8_t ok =*/pkt.read<uint8_t>();
	version = pkt.read<uint16_t>();
	// uint8, uint64, uint64, uint8 : kullanılmıyor
	return true;
}

bool ParseCompressedHeader(Packet& pkt, uint32_t& compressedLen, uint32_t& originalLen, uint32_t& crc)
{
	if (Is2369())
	{
		if (!Remaining(pkt, 12))
			return false;
		compressedLen = pkt.read<uint32_t>();
		originalLen   = pkt.read<uint32_t>();
	}
	else
	{
		if (!Remaining(pkt, 8))
			return false;
		compressedLen = pkt.read<uint16_t>();
		originalLen   = pkt.read<uint16_t>();
	}
	crc = pkt.read<uint32_t>();
	return Remaining(pkt, compressedLen);
}

bool ParseAllCharInfo2369(Packet& pkt, std::vector<CharInfo2369>& out, uint8_t* subOpcode)
{
	out.clear();
	if (!Remaining(pkt, 2))
		return false;
	uint8_t sub = pkt.read<uint8_t>();
	if (subOpcode)
		*subOpcode = sub;
	if (sub != 1)
		return false;
	uint8_t result = pkt.read<uint8_t>();
	if (result != 1)
		return false;
	for (int i = 0; i < MAX_CHARS_2369; i++)
	{
		CharInfo2369 c;
		if (!ReadStrD(pkt, c.id))
			return false;
		if (!Remaining(pkt, 1 + 2 + 1 + 1 + 4 + 1 + 8 * 6))
			return false;
		c.race  = pkt.read<uint8_t>();
		c.cls   = pkt.read<uint16_t>();
		c.level = pkt.read<uint8_t>();
		c.face  = pkt.read<uint8_t>();
		c.hair  = pkt.read<uint32_t>();
		c.zone  = pkt.read<uint8_t>();
		for (int k = 0; k < 8; k++)
		{
			c.itemID[k]  = pkt.read<uint32_t>();
			c.itemDur[k] = pkt.read<uint16_t>();
		}
		out.push_back(std::move(c));
	}
	return true;
}

// ---- MYINFO ----------------------------------------------------------------------------
bool ParseMyInfo2369(Packet& pkt, MyInfo2369& o)
{
	o = MyInfo2369 {};
	if (!Remaining(pkt, 3))
		return false;
	o.id = pkt.read<uint16_t>();
	if (!ReadStrS(pkt, o.name))
		return false;
	if (!Remaining(pkt, 2 + 2 + 2 + 1 + 1 + 2 + 1 + 4 + 1 + 1 + 2 + 1 + 2 + 8 + 8 + 4 + 4 + 2 + 1))
		return false;
	o.x              = pkt.read<uint16_t>();
	o.z              = pkt.read<uint16_t>();
	o.y              = pkt.read<int16_t>();
	o.nation         = pkt.read<uint8_t>();
	o.race           = pkt.read<uint8_t>();
	o.cls            = pkt.read<uint16_t>();
	o.face           = pkt.read<uint8_t>();
	o.hair           = pkt.read<uint32_t>();
	o.rank           = pkt.read<uint8_t>();
	o.title          = pkt.read<uint8_t>();
	pkt.read<uint8_t>(); // 0 (miğfer gizle)
	pkt.read<uint8_t>(); // 0 (kostüm gizle)
	o.level          = pkt.read<uint8_t>();
	o.points         = pkt.read<int16_t>();
	o.maxExp         = pkt.read<int64_t>();
	o.exp            = pkt.read<int64_t>();
	o.loyalty        = pkt.read<uint32_t>();
	o.monthlyLoyalty = pkt.read<uint32_t>();
	o.clanID         = pkt.read<int16_t>();
	o.fame           = pkt.read<uint8_t>();
	if (!ReadClan(pkt, o.clan, true))
		return false;
	if (!Remaining(pkt, 4 + 2 * 4 + 4 + 4 + 10 + 2 + 2 + 6 + 4 + 1 + 2 + 9))
		return false;
	for (int i = 0; i < 4; i++)
		pkt.read<uint8_t>(); // 2,3,4,5 bilinmiyor
	o.maxHp     = pkt.read<int16_t>();
	o.hp        = pkt.read<int16_t>();
	o.maxMp     = pkt.read<int16_t>();
	o.mp        = pkt.read<int16_t>();
	o.maxWeight = pkt.read<uint32_t>();
	o.weight    = pkt.read<uint32_t>();
	o.str        = pkt.read<uint8_t>();
	o.strBonus   = pkt.read<uint8_t>();
	o.sta        = pkt.read<uint8_t>();
	o.staBonus   = pkt.read<uint8_t>();
	o.dex        = pkt.read<uint8_t>();
	o.dexBonus   = pkt.read<uint8_t>();
	o.intel      = pkt.read<uint8_t>();
	o.intelBonus = pkt.read<uint8_t>();
	o.cha        = pkt.read<uint8_t>();
	o.chaBonus   = pkt.read<uint8_t>();
	o.totalHit   = pkt.read<int16_t>();
	o.totalAc    = pkt.read<int16_t>();
	for (int i = 0; i < 6; i++)
		o.resist[i] = pkt.read<uint8_t>();
	o.gold         = pkt.read<uint32_t>();
	o.authority    = pkt.read<uint8_t>();
	o.knightsRank  = pkt.read<int8_t>();
	o.personalRank = pkt.read<int8_t>();
	for (int i = 0; i < 9; i++)
		o.skill[i] = pkt.read<uint8_t>();
	constexpr size_t ITEM_BYTES = 4 + 2 + 2 + 1 + 2 + 4 + 4;
	if (!Remaining(pkt, ITEM_BYTES * INVENTORY_TOTAL_2369))
		return false;
	for (int i = 0; i < INVENTORY_TOTAL_2369; i++)
	{
		ItemData2369& it = o.items[i];
		it.num           = pkt.read<uint32_t>();
		it.duration      = pkt.read<int16_t>();
		it.count         = pkt.read<uint16_t>();
		it.flag          = pkt.read<uint8_t>();
		it.rentalMinutes = pkt.read<uint16_t>();
		it.uniqueID      = pkt.read<uint32_t>();
		it.expiration    = pkt.read<uint32_t>();
	}
	if (!Remaining(pkt, 2))
		return true; // eski sunucu derlemesi: kuyruk yok
	o.accountStatus = pkt.read<uint8_t>();
	int nPrem       = pkt.read<uint8_t>();
	for (int i = 0; i < nPrem && Remaining(pkt, 3); i++)
	{
		PremiumData2369 p;
		p.type  = pkt.read<uint8_t>();
		p.hours = pkt.read<uint16_t>();
		o.premium.push_back(p);
	}
	if (!Remaining(pkt, 1 + 1 + 4 + 5 + 1 + 2 + 1 + 5 + 8 + 2 + 2 + 4))
		return true;
	o.premiumInUse = pkt.read<uint8_t>();
	o.chicken      = pkt.read<uint8_t>();
	o.manner       = pkt.read<uint32_t>();
	for (int i = 0; i < 5; i++)
		o.military[i] = pkt.read<uint8_t>();
	pkt.read<uint8_t>(); // 0
	o.genieTime    = pkt.read<uint16_t>();
	o.rebirthLevel = pkt.read<uint8_t>();
	for (int i = 0; i < 5; i++)
		o.rebStat[i] = pkt.read<uint8_t>();
	o.sealedExp  = pkt.read<uint64_t>();
	o.coverTitle = pkt.read<uint16_t>();
	o.skillTitle = pkt.read<uint16_t>();
	o.symbol     = pkt.read<uint32_t>();
	if (Remaining(pkt, 5))
	{
		pkt.read<uint32_t>(); // 0
		pkt.read<uint8_t>();  // 0
	}
	return true;
}

// ---- Diğer oyuncu ------------------------------------------------------------------------
bool ParseUserInfo2369(Packet& pkt, UserInfo2369& o)
{
	o = UserInfo2369 {};
	if (!ReadStrS(pkt, o.name))
		return false;
	if (!Remaining(pkt, 1 + 1 + 1 + 2 + 1))
		return false;
	o.nation = pkt.read<uint8_t>();
	pkt.read<uint8_t>(); // 0
	pkt.read<uint8_t>(); // 0
	o.clanID = pkt.read<int16_t>();
	o.fame   = pkt.read<uint8_t>();
	if (!ReadClan(pkt, o.clan, false))
		return false;
	if (!Remaining(pkt, 1 + 1 + 2 + 2 + 2 + 2 + 1 + 4 + 1 + 4 + 1 + 1 + 1 + 1 + 1 + 2 + 2 + 1 + 1 + 2 + 1 + 1))
		return false;
	o.level            = pkt.read<uint8_t>();
	o.race             = pkt.read<uint8_t>();
	o.cls              = pkt.read<uint16_t>();
	o.x                = pkt.read<uint16_t>();
	o.z                = pkt.read<uint16_t>();
	o.y                = pkt.read<int16_t>();
	o.face             = pkt.read<uint8_t>();
	o.hair             = pkt.read<uint32_t>();
	o.resHpType        = pkt.read<uint8_t>();
	o.abnormalType     = pkt.read<uint32_t>();
	o.needParty        = pkt.read<uint8_t>();
	o.authority        = pkt.read<uint8_t>();
	o.partyLeader      = pkt.read<uint8_t>();
	o.invisibilityType = pkt.read<uint8_t>();
	o.teamColour       = pkt.read<uint8_t>();
	o.devil            = pkt.read<uint16_t>();
	o.direction        = pkt.read<int16_t>();
	o.chicken          = pkt.read<uint8_t>();
	o.kingFlag         = pkt.read<uint8_t>();
	pkt.read<uint16_t>(); // 0
	o.knightsRank  = pkt.read<int8_t>();
	o.personalRank = pkt.read<int8_t>();
	if (!Remaining(pkt, USER_ITEM_SLOTS_2369 * 7))
		return false;
	for (int i = 0; i < USER_ITEM_SLOTS_2369; i++)
	{
		o.items[i].num      = pkt.read<uint32_t>();
		o.items[i].duration = pkt.read<int16_t>();
		o.items[i].flag     = pkt.read<uint8_t>();
	}
	if (!Remaining(pkt, 1))
		return true;
	o.zone = pkt.read<uint8_t>();
	if (!Remaining(pkt, 1 + 1 + 4 + 5 + 2 + 4 + 4 + 2))
		return true;
	pkt.read<uint8_t>();  // -1
	pkt.read<uint8_t>();  // -1
	pkt.read<uint32_t>(); // 0
	o.hidingWings  = pkt.read<uint8_t>();
	o.hidingHelmet = pkt.read<uint8_t>();
	o.hidingCospre = pkt.read<uint8_t>();
	o.genieActive  = pkt.read<uint8_t>();
	o.rebirthLevel = pkt.read<uint8_t>();
	o.coverTitle   = pkt.read<uint16_t>();
	o.symbol       = pkt.read<uint32_t>();
	pkt.read<uint32_t>(); // yüz süresi
	pkt.read<uint16_t>(); // 2364 yeni alan
	return true;
}

// ---- NPC ------------------------------------------------------------------------------
bool ParseNpcInfo2369(Packet& pkt, NpcInfo2369& o)
{
	o = NpcInfo2369 {};
	if (!Remaining(pkt, 2 + 1 + 2 + 4 + 1 + 4 + 2 + 4 + 4 + 1 + 1 + 2 + 2 + 2 + 4 + 1 + 2 + 2 + 2))
		return false;
	o.protoID      = pkt.read<int16_t>();
	o.kind         = pkt.read<uint8_t>();
	o.pid          = pkt.read<uint16_t>();
	o.sellingGroup = pkt.read<int32_t>();
	o.type         = pkt.read<uint8_t>();
	pkt.read<uint32_t>(); // 0
	o.size    = pkt.read<int16_t>();
	o.weapon1 = pkt.read<uint32_t>();
	o.weapon2 = pkt.read<uint32_t>();
	o.nation  = pkt.read<uint8_t>();
	o.level   = pkt.read<uint8_t>();
	o.x       = pkt.read<uint16_t>();
	o.z       = pkt.read<uint16_t>();
	o.y       = pkt.read<int16_t>();
	o.gateOpen   = pkt.read<uint32_t>();
	o.objectType = pkt.read<uint8_t>();
	pkt.read<uint16_t>(); // 0
	pkt.read<uint16_t>(); // 0
	o.direction = pkt.read<int16_t>();
	return true;
}

// ---- Sohbet ----------------------------------------------------------------------------
uint8_t MapChatType2369(uint8_t t)
{
	switch (t)
	{
		case 1: return N3_CHAT_NORMAL;           // GENERAL_CHAT
		case 2: return N3_CHAT_PRIVATE;          // PRIVATE_CHAT
		case 3: return N3_CHAT_PARTY;            // PARTY_CHAT
		case 4: return N3_CHAT_FORCE;            // FORCE_CHAT
		case 5: return N3_CHAT_SHOUT;            // SHOUT_CHAT
		case 6: return N3_CHAT_CLAN;             // KNIGHTS_CHAT
		case 7: return N3_CHAT_PUBLIC;           // PUBLIC_CHAT (GM duyurusu)
		case 8: return N3_CHAT_WAR;              // WAR_SYSTEM_CHAT
		case 9: return N3_CHAT_TITLE;            // PERMANENT_CHAT (üst satır sabit duyuru)
		case 10: return N3_CHAT_TITLE_DELETE;    // END_PERMANENT_CHAT
		case 11: return N3_CHAT_WAR;             // MONUMENT_NOTICE
		case 12: return N3_CHAT_NORMAL;          // GM_CHAT
		case 13: return N3_CHAT_WAR;             // COMMAND_CHAT
		case 14: return N3_CHAT_NORMAL;          // MERCHANT_CHAT
		case 15: return N3_CHAT_CLAN;            // ALLIANCE_CHAT
		case 17: return N3_CHAT_WAR;             // ANNOUNCEMENT_CHAT
		case 19: return N3_CHAT_SHOUT;           // SEEKING_PARTY_CHAT
		case 21: return 0;                       // GM_INFO_CHAT (bilgi penceresi)
		case 22: return N3_CHAT_PRIVATE;         // COMMAND_PM_CHAT
		case 24: return N3_CHAT_CLAN;            // CLAN_NOTICE
		case 25: case 26: case 27: case 28: return N3_CHAT_WAR; // KROWAZ/DEATH/CHAOS bildirimleri
		case 29: return N3_CHAT_NORMAL;          // ANNOUNCEMENT_WHITE_CHAT
		case 33: return N3_CHAT_NORMAL;          // CHATROM_CHAT
		case 34: case 35: return N3_CHAT_CLAN;   // NOAH_KNIGHTS_CHAT / ALLIANCE_NOTICE
		default: return 0;
	}
}

// ---- Yapıcılar ---------------------------------------------------------------------------
void BuildServerListReq(std::vector<uint8_t>& out)
{
	out.clear();
	AddByte(out, LS_SERVERLIST);
	if (Is2369())
		AddU16(out, 0); // echo
}

void BuildAllCharInfoReq(std::vector<uint8_t>& out)
{
	out.clear();
	AddByte(out, WIZ_ALLCHAR_INFO_REQ);
	if (Is2369())
		AddByte(out, 1); // AllCharInfoOpcode
}

void BuildSelectCharacter(std::vector<uint8_t>& out, const std::string& account, const std::string& charName,
	uint8_t zoneInit, uint8_t zoneCur)
{
	out.clear();
	AddByte(out, WIZ_SEL_CHAR);
	AddStrD(out, account);
	AddStrD(out, charName);
	AddByte(out, zoneInit);
	if (!Is2369())
		AddByte(out, zoneCur);
}

void BuildGameStart(std::vector<uint8_t>& out, uint8_t step, const std::string& charName)
{
	out.clear();
	AddByte(out, WIZ_GAMESTART);
	AddByte(out, step);
	if (Is2369())
		AddStrS(out, charName);
}

void BuildMove(std::vector<uint8_t>& out, uint16_t willX, uint16_t willZ, int16_t willY, int16_t speed, uint8_t flag,
	uint16_t curX, uint16_t curZ, int16_t curY)
{
	out.clear();
	AddByte(out, WIZ_MOVE);
	AddU16(out, willX);
	AddU16(out, willZ);
	AddU16(out, (uint16_t) willY);
	AddU16(out, (uint16_t) speed);
	AddByte(out, flag);
	if (Is2369())
	{
		AddU16(out, curX);
		AddU16(out, curZ);
		AddU16(out, (uint16_t) curY);
	}
}
} // namespace KoProto
