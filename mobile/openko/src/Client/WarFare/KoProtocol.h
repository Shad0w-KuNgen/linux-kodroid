// KoProtocol.h — çalışma zamanı protokol sürümü seçimi (1298 / 2369) ve 2369 paket düzenleri.
//
// Mobil istemci iki sunucu protokolüyle konuşabilir:
//   1298 : OpenKO sunucusu (LoginServer 15100, GameServer 15001)
//   2369 : ISTIRAP v2369 sunucusu ("1 - Login & Game Source", __VERSION 2369; LoginServer 15200,
//          GameServer 15301). Şifreleme yok (istemci LS_CRYPTION göndermediği sürece sunucu açmaz),
//          WIZ_COMPRESS_PACKET başlığı uint32×3, karakter listesi 4 karakter, uint32 saç, MYINFO ve
//          USER/NPC bilgisi genişletilmiş düzende.
//
// Seçim Server.ini [Server] bölümünden okunur:
//   Protocol=2369        (yoksa [Version] Files >= 2000 ise 2369, değilse 1298)
//   LoginPort=15200      (yoksa protokole göre 15200 / 15100)
//   GamePort=15301       (yoksa protokole göre 15301 / 15001)
//
// Buradaki ayrıştırıcılar motor/arayüz bağımsızdır (yalnız Packet) ve başsız ko_proto_test ile
// sunucu kaynağından türetilen örnek paketlerle doğrulanır.
#ifndef CLIENT_WARFARE_KOPROTOCOL_H
#define CLIENT_WARFARE_KOPROTOCOL_H

#pragma once

#include <shared/Packet.h>

#include <cstdint>
#include <string>
#include <vector>

namespace KoProto
{
inline constexpr int V1298 = 1298;
inline constexpr int V2369 = 2369;

inline constexpr int LOGIN_PORT_1298 = 15100;
inline constexpr int GAME_PORT_1298  = 15001;
inline constexpr int LOGIN_PORT_2369 = 15200;
inline constexpr int GAME_PORT_2369  = 15301;

// 2369 sunucusu envanter boyutları (shared/globals.h: SLOT_MAX 14, HAVE_MAX 28, COSP_MAX 9, MBAG_MAX 12×2)
inline constexpr int SLOT_MAX_2369        = 14;
inline constexpr int HAVE_MAX_2369        = 28;
inline constexpr int COSP_MAX_2369        = 9;
inline constexpr int MBAG_MAX_2369        = 12;
inline constexpr int INVENTORY_TOTAL_2369 = SLOT_MAX_2369 + HAVE_MAX_2369 + COSP_MAX_2369 + 2 * MBAG_MAX_2369; // 75
inline constexpr int MAX_CHARS_2369       = 4;
inline constexpr int USER_ITEM_SLOTS_2369 = 15; // BREAST, LEG, HEAD, GLOVE, FOOT, SHOULDER, RIGHTHAND, LEFTHAND, CWING, CHELMET, CLEFT, CRIGHT, CTOP, CFAIRY, CTATTOO

int Version();
bool Is2369();
/// 2369 saç değeri u32: en üst bayt = saç stili (dosya adı indeksi, upc_xx_hairNN), alt 3 bayt = renk.
/// 1298 (ve 255'ten küçük değerler) olduğu gibi.
inline int HairIndex2369(uint32_t hair)
{
	return hair > 0xFF ? (int) ((hair >> 24) & 0xFF) : (int) hair;
}
int LoginPort();
int GamePort();
void Configure(int version, int loginPort = 0, int gamePort = 0);
// Server.ini'den okur (dosya yoksa 1298 varsayılanı). Dönen değer: etkin protokol sürümü.
int LoadFromServerIni(const std::string& iniPath);
// Protocol anahtarı yoksa [Version] Files değerinden çıkarım.
int InferVersionFromFiles(int filesVersion);

// ---- Login sunucusu --------------------------------------------------------------------
struct ServerInfo2369
{
	std::string lanIP, ip, name;
	int16_t users = 0, serverID = 0, groupID = 0, playerCap = 0, freePlayerCap = 0;
	uint8_t screenType = 0;
	std::string karusKing, karusNotice, elmoradKing, elmoradNotice;
};
// LS_SERVERLIST yanıtı (opcode okunmuş): uint16 echo, uint8 adet, adet × sunucu
bool ParseServerList2369(Packet& pkt, std::vector<ServerInfo2369>& out);

// LS_LOGIN_REQ yanıtı (opcode okunmuş): uint16 0, uint8 sonuç, [başarı: int16 premium, str hesap]
// [5 = oyunda: str ip, uint16 port, str hesap]
enum LoginResult2369 : uint8_t
{
	LOGIN_2369_SUCCESS   = 1,
	LOGIN_2369_NOT_FOUND = 2,
	LOGIN_2369_INVALID   = 3,
	LOGIN_2369_BANNED    = 4,
	LOGIN_2369_IN_GAME   = 5,
	LOGIN_2369_ERROR     = 6,
	LOGIN_2369_AGREEMENT = 0x0F,
	LOGIN_2369_OTP       = 0x10,
	LOGIN_2369_FAILED    = 0xFF
};
struct LoginReply2369
{
	uint8_t result = 0;
	int16_t premium = 0;
	std::string account;
	std::string inGameIP;
	uint16_t inGamePort = 0;
};
bool ParseLoginReply2369(Packet& pkt, LoginReply2369& out);

// ---- Oyun sunucusu: giriş ------------------------------------------------------------
// WIZ_VERSION_CHECK yanıtı (opcode okunmuş): uint8 0, uint16 sürüm, uint8, uint64, uint64, uint8
bool ParseVersionCheck2369(Packet& pkt, int& version);

// WIZ_COMPRESS_PACKET başlığı: 2369'da uint32 sıkıştırılmış, uint32 özgün, uint32 crc; 1298'de uint16, uint16, uint32
bool ParseCompressedHeader(Packet& pkt, uint32_t& compressedLen, uint32_t& originalLen, uint32_t& crc);

// WIZ_ALLCHAR_INFO_REQ yanıtı (opcode okunmuş): uint8 alt=1, uint8 sonuç, 4 × karakter
struct CharInfo2369
{
	std::string id;
	uint8_t race = 0;
	uint16_t cls = 0;
	uint8_t level = 0, face = 0;
	uint32_t hair = 0;
	uint8_t zone = 0;
	// Sıra: HEAD, BREAST, SHOULDER, RIGHTHAND, LEFTHAND, LEG, GLOVE, FOOT (sunucu yuva indeksine göre)
	uint32_t itemID[8] = {};
	uint16_t itemDur[8] = {};
};
// Dönüş: false = sonuç 0 ya da bilinmeyen alt opcode (out boş kalır)
bool ParseAllCharInfo2369(Packet& pkt, std::vector<CharInfo2369>& out, uint8_t* subOpcode = nullptr);

// ---- MYINFO -------------------------------------------------------------------------
struct ClanInfo2369
{
	uint16_t allianceID = 0;
	uint8_t flag = 0; // MYINFO'da var, USER_INOUT'ta pelerinden sonra gelir (byFlag)
	std::string name;
	uint8_t grade = 0, ranking = 0;
	uint16_t markVersion = 0;
	uint16_t capeID = 0xFFFF;
	uint8_t capeR = 0, capeG = 0, capeB = 0, capeFlag = 0;
};
struct ItemData2369
{
	uint32_t num = 0;
	int16_t duration = 0;
	uint16_t count = 0;
	uint8_t flag = 0;
	uint16_t rentalMinutes = 0;
	uint32_t uniqueID = 0;
	uint32_t expiration = 0;
};
struct PremiumData2369
{
	uint8_t type = 0;
	uint16_t hours = 0;
};
struct MyInfo2369
{
	uint16_t id = 0;
	std::string name;
	uint16_t x = 0, z = 0;
	int16_t y = 0;
	uint8_t nation = 0, race = 0;
	uint16_t cls = 0;
	uint8_t face = 0;
	uint32_t hair = 0;
	uint8_t rank = 0, title = 0;
	uint8_t level = 0;
	int16_t points = 0;
	int64_t maxExp = 0, exp = 0;
	uint32_t loyalty = 0, monthlyLoyalty = 0;
	int16_t clanID = 0;
	uint8_t fame = 0;
	ClanInfo2369 clan;
	int16_t maxHp = 0, hp = 0, maxMp = 0, mp = 0;
	uint32_t maxWeight = 0, weight = 0;
	uint8_t str = 0, strBonus = 0, sta = 0, staBonus = 0, dex = 0, dexBonus = 0, intel = 0, intelBonus = 0, cha = 0,
			chaBonus = 0;
	int16_t totalHit = 0, totalAc = 0;
	uint8_t resist[6] = {}; // fire, cold, lightning, magic, disease(curse), poison
	uint32_t gold = 0;
	uint8_t authority = 0;
	int8_t knightsRank = -1, personalRank = -1;
	uint8_t skill[9] = {};
	ItemData2369 items[INVENTORY_TOTAL_2369];
	uint8_t accountStatus = 0;
	std::vector<PremiumData2369> premium;
	uint8_t premiumInUse = 0, chicken = 0;
	uint32_t manner = 0;
	uint8_t military[5] = {};
	uint16_t genieTime = 0;
	uint8_t rebirthLevel = 0;
	uint8_t rebStat[5] = {};
	uint64_t sealedExp = 0;
	uint16_t coverTitle = 0, skillTitle = 0;
	uint32_t symbol = 0;
};
bool ParseMyInfo2369(Packet& pkt, MyInfo2369& out);

// ---- Diğer oyuncu (WIZ_USER_INOUT / WIZ_REQ_USERIN) ---------------------------------------
struct UserItem2369
{
	uint32_t num = 0;
	int16_t duration = 0;
	uint8_t flag = 0;
};
struct UserInfo2369
{
	std::string name;
	uint8_t nation = 0;
	int16_t clanID = 0;
	uint8_t fame = 0;
	ClanInfo2369 clan;
	uint8_t level = 0, race = 0;
	uint16_t cls = 0;
	uint16_t x = 0, z = 0;
	int16_t y = 0;
	uint8_t face = 0;
	uint32_t hair = 0;
	uint8_t resHpType = 0;  // 1 ayakta, 2 oturuyor, 3 ölü
	uint32_t abnormalType = 0;
	uint8_t needParty = 0, authority = 0, partyLeader = 0, invisibilityType = 0, teamColour = 0;
	uint16_t devil = 0;
	int16_t direction = 0;
	uint8_t chicken = 0, kingFlag = 0;
	int8_t knightsRank = -1, personalRank = -1;
	UserItem2369 items[USER_ITEM_SLOTS_2369]; // ilk 8'i 1298 MAX_ITEM_SLOT_OPC sırasıyla aynı
	uint8_t zone = 0;
	uint8_t hidingWings = 0, hidingHelmet = 0, hidingCospre = 0, genieActive = 0, rebirthLevel = 0;
	uint16_t coverTitle = 0;
	uint32_t symbol = 0;
};
// Kimlik (uint16) okunduktan sonraki kısım.
bool ParseUserInfo2369(Packet& pkt, UserInfo2369& out);

// ---- NPC (WIZ_NPC_INOUT / WIZ_REQ_NPCIN) ------------------------------------------------
struct NpcInfo2369
{
	int16_t protoID = 0;   // kaynak/görünüm ID (s_pTbl_NPC_Looks)
	uint8_t kind = 0;      // 1 canavar, 2 NPC
	uint16_t pid = 0;      // resim ID
	int32_t sellingGroup = 0;
	uint8_t type = 0;      // NPC tipi (satıcı vb.)
	int16_t size = 100;    // ölçek, 100 = 1.0
	uint32_t weapon1 = 0, weapon2 = 0;
	uint8_t nation = 0, level = 0;
	uint16_t x = 0, z = 0;
	int16_t y = 0;
	uint32_t gateOpen = 0;
	uint8_t objectType = 0; // 0 karakter NPC, 1 nesne NPC
	int16_t direction = 0;
};
// Kimlik (uint16) okunduktan sonraki kısım (varsayılan NPC düzeni; kale tipleri 15/191 farklı olabilir).
bool ParseNpcInfo2369(Packet& pkt, NpcInfo2369& out);

// ---- Sohbet tipi eşlemesi ----------------------------------------------------------------
// 2369 ChatType → 1298 e_ChatMode (N3_CHAT_*) değeri; 0 = yok say.
uint8_t MapChatType2369(uint8_t serverType);

// ---- Gönderilen paket yapıcıları (opcode dahil ham bayt dizisi) --------------------------------
void BuildServerListReq(std::vector<uint8_t>& out);                                  // 2369: uint16 echo ekler
void BuildAllCharInfoReq(std::vector<uint8_t>& out);                                 // 2369: uint8 1 ekler
void BuildSelectCharacter(std::vector<uint8_t>& out, const std::string& account, const std::string& charName,
	uint8_t zoneInit, uint8_t zoneCur);                                              // 2369: zoneCur gönderilmez
void BuildGameStart(std::vector<uint8_t>& out, uint8_t step, const std::string& charName); // 2369: isim (uint8 uzunluk)
// WIZ_NEW_CHAR: 2369 (GameServer NewCharToAgent) saç uint32, 1.298 uint8
void BuildNewChar(std::vector<uint8_t>& out, uint8_t index, const std::string& name, uint8_t race, uint16_t cls, uint8_t face,
	uint32_t hair, uint8_t str, uint8_t sta, uint8_t dex, uint8_t intel, uint8_t cha);
void BuildMove(std::vector<uint8_t>& out, uint16_t willX, uint16_t willZ, int16_t willY, int16_t speed, uint8_t flag,
	uint16_t curX, uint16_t curZ, int16_t curY);                                     // 2369: mevcut konum eklenir
} // namespace KoProto

#endif // CLIENT_WARFARE_KOPROTOCOL_H
