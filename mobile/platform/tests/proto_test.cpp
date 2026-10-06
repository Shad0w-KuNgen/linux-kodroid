// proto_test.cpp — KoProtocol (1298 / 2369) başsız testi.
// Sunucu kaynağındaki (ISTIRAP v2369) Packet << sırasını birebir taklit eden bir yazıcıyla paketler
// üretilir ve istemci ayrıştırıcılarının aynı değerleri geri okuduğu doğrulanır.
#include "StdAfx.h"
#include "KoProtocol.h"
#include "PacketDef.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

static int g_fail = 0;
#define CHECK(cond, ...)                                           \
	do                                                             \
	{                                                              \
		if (!(cond))                                               \
		{                                                          \
			std::printf("  BASARISIZ %s:%d: ", __FILE__, __LINE__); \
			std::printf(__VA_ARGS__);                              \
			std::printf("\n");                                     \
			++g_fail;                                              \
		}                                                          \
	} while (0)

// Sunucu ByteBuffer'ı: varsayılan DByte (uint16 uzunluk), SByte() sonrası uint8 uzunluk.
struct W
{
	std::vector<uint8_t> b;
	bool sbyte = false;
	W& u8(uint8_t v)
	{
		b.push_back(v);
		return *this;
	}
	W& i8(int8_t v)
	{
		return u8((uint8_t) v);
	}
	W& u16(uint16_t v)
	{
		b.push_back(v & 0xFF);
		b.push_back(v >> 8);
		return *this;
	}
	W& i16(int16_t v)
	{
		return u16((uint16_t) v);
	}
	W& u32(uint32_t v)
	{
		for (int i = 0; i < 4; i++)
			b.push_back((v >> (8 * i)) & 0xFF);
		return *this;
	}
	W& i32(int32_t v)
	{
		return u32((uint32_t) v);
	}
	W& u64(uint64_t v)
	{
		for (int i = 0; i < 8; i++)
			b.push_back((v >> (8 * i)) & 0xFF);
		return *this;
	}
	W& i64(int64_t v)
	{
		return u64((uint64_t) v);
	}
	W& str(const std::string& s)
	{
		if (sbyte)
			u8((uint8_t) s.size());
		else
			u16((uint16_t) s.size());
		b.insert(b.end(), s.begin(), s.end());
		return *this;
	}
	W& SByte()
	{
		sbyte = true;
		return *this;
	}
	W& DByte()
	{
		sbyte = false;
		return *this;
	}
	Packet pkt() const
	{
		Packet p;
		p.append(b.data(), b.size());
		return p;
	}
};

static void TestConfig()
{
	std::printf("[test] protokol/port yapilandirmasi\n");
	KoProto::Configure(KoProto::V2369);
	CHECK(KoProto::Is2369() && KoProto::LoginPort() == 15200 && KoProto::GamePort() == 15301, "2369 varsayilan portlar %d/%d",
		KoProto::LoginPort(), KoProto::GamePort());
	KoProto::Configure(KoProto::V1298);
	CHECK(!KoProto::Is2369() && KoProto::LoginPort() == 15100 && KoProto::GamePort() == 15001, "1298 varsayilan portlar");
	KoProto::Configure(KoProto::V2369, 16000, 16001);
	CHECK(KoProto::LoginPort() == 16000 && KoProto::GamePort() == 16001, "ozel portlar");
	KoProto::Configure(1234);
	CHECK(KoProto::Version() == KoProto::V1298, "bilinmeyen surum 1298'e duser");
	CHECK(KoProto::InferVersionFromFiles(2434) == KoProto::V2369, "Files=2434 -> 2369");
	CHECK(KoProto::InferVersionFromFiles(1299) == KoProto::V1298, "Files=1299 -> 1298");

	const char* dir = std::getenv("TMPDIR");
	std::string base = dir ? dir : "/tmp";
	{
		std::string p = base + "/ko_proto_test_a.ini";
		std::ofstream(p) << "[Server]\r\nCount=1\r\nIP0=86.105.4.195\r\n\r\n[Version]\r\nFiles=2434\r\n";
		CHECK(KoProto::LoadFromServerIni(p) == KoProto::V2369 && KoProto::LoginPort() == 15200, "ini: Files>=2000 -> 2369");
	}
	{
		std::string p = base + "/ko_proto_test_b.ini";
		std::ofstream(p) << "[Server]\r\nProtocol=1298\r\nLoginPort=15555\r\n[Version]\r\nFiles=2434\r\n";
		CHECK(KoProto::LoadFromServerIni(p) == KoProto::V1298 && KoProto::LoginPort() == 15555 && KoProto::GamePort() == 15001,
			"ini: Protocol/LoginPort anahtarlari");
	}
	{
		std::string p = base + "/ko_proto_test_c.ini";
		std::ofstream(p) << "[Server]\r\nProtocol=2369\r\nGamePort=15302\r\n";
		CHECK(KoProto::LoadFromServerIni(p) == KoProto::V2369 && KoProto::GamePort() == 15302 && KoProto::LoginPort() == 15200,
			"ini: GamePort anahtari");
	}
	CHECK(KoProto::LoadFromServerIni(base + "/ko_proto_test_yok.ini") == KoProto::V1298, "ini yok -> 1298");
}

static void TestServerList()
{
	std::printf("[test] LS_SERVERLIST 2369\n");
	KoProto::Configure(KoProto::V2369);
	W w;
	w.u16(0).u8(2);
	w.str("192.168.1.5").str("86.105.4.195").str("ISTIRAP").i16(37).i16(1).i16(1).i16(3000).i16(3000).u8(0).u8(1);
	w.str("KarusKing").str("Karus notice").str("ElKing").str("El notice");
	w.str("10.0.0.2").str("86.105.4.196").str("Test2").i16(-1).i16(2).i16(1).i16(100).i16(100).u8(0).u8(2);
	w.str("").str("").str("").str("");
	Packet p = w.pkt();
	std::vector<KoProto::ServerInfo2369> list;
	CHECK(KoProto::ParseServerList2369(p, list), "ayristirma");
	CHECK(list.size() == 2, "adet %zu", list.size());
	if (list.size() == 2)
	{
		CHECK(list[0].ip == "86.105.4.195" && list[0].lanIP == "192.168.1.5" && list[0].name == "ISTIRAP", "sunucu 0 alanlari");
		CHECK(list[0].users == 37 && list[0].serverID == 1 && list[0].playerCap == 3000 && list[0].screenType == 1, "sunucu 0 sayilar");
		CHECK(list[0].karusKing == "KarusKing" && list[0].elmoradNotice == "El notice", "sunucu 0 kral/duyuru");
		CHECK(list[1].users == -1 && list[1].name == "Test2", "sunucu 1 dolu");
	}
	CHECK(p.rpos() == p.size(), "tum paket tuketildi (%zu/%zu)", p.rpos(), p.size());

	W bad;
	bad.u16(0).u8(1).str("1.2.3.4");
	Packet pb = bad.pkt();
	CHECK(!KoProto::ParseServerList2369(pb, list), "kesik paket reddedilir");
}

static void TestLoginReply()
{
	std::printf("[test] LS_LOGIN_REQ 2369 yaniti\n");
	{
		W w;
		w.u16(0).u8(1).i16(2).str("mobil");
		Packet p = w.pkt();
		KoProto::LoginReply2369 r;
		CHECK(KoProto::ParseLoginReply2369(p, r) && r.result == 1 && r.premium == 2 && r.account == "mobil", "basari");
	}
	{
		W w;
		w.u16(0).u8(5).str("86.105.4.195").u16(15572).str("mobil");
		Packet p = w.pkt();
		KoProto::LoginReply2369 r;
		CHECK(KoProto::ParseLoginReply2369(p, r) && r.result == 5 && r.inGameIP == "86.105.4.195" && r.inGamePort == 15572,
			"oyunda: ip/port");
		// 1298 istemci kodu ayni baytlari int16 uzunluk + ip + int16 port olarak okur
		Packet q = w.pkt();
		q.read<uint16_t>();
		q.read<uint8_t>();
		int len = q.read<int16_t>();
		std::string ip;
		q.readString(ip, len);
		CHECK(ip == "86.105.4.195" && q.read<int16_t>() == 15572, "1298 yolu ile uyumlu");
	}
	{
		W w;
		w.u16(0).u8(3);
		Packet p = w.pkt();
		KoProto::LoginReply2369 r;
		CHECK(KoProto::ParseLoginReply2369(p, r) && r.result == 3, "sifre hatali");
	}
}

static void TestVersionAndCompressed()
{
	std::printf("[test] WIZ_VERSION_CHECK ve sikistirma basligi\n");
	W w;
	w.u8(0).u16(2369).u8(0).u64(0).u64(0).u8(0);
	Packet p = w.pkt();
	int v = 0;
	CHECK(KoProto::ParseVersionCheck2369(p, v) && v == 2369, "surum %d", v);

	KoProto::Configure(KoProto::V2369);
	W c;
	c.u32(5).u32(9).u32(0xDEADBEEF).u8(1).u8(2).u8(3).u8(4).u8(5);
	Packet pc = c.pkt();
	uint32_t a = 0, b = 0, crc = 0;
	CHECK(KoProto::ParseCompressedHeader(pc, a, b, crc) && a == 5 && b == 9 && crc == 0xDEADBEEF && pc.rpos() == 12, "2369 baslik");
	KoProto::Configure(KoProto::V1298);
	W c2;
	c2.u16(5).u16(9).u32(7).u8(1).u8(2).u8(3).u8(4).u8(5);
	Packet pc2 = c2.pkt();
	CHECK(KoProto::ParseCompressedHeader(pc2, a, b, crc) && a == 5 && b == 9 && crc == 7 && pc2.rpos() == 8, "1298 baslik");
	W c3;
	c3.u32(50).u32(9).u32(7).u8(1);
	Packet pc3 = c3.pkt();
	KoProto::Configure(KoProto::V2369);
	CHECK(!KoProto::ParseCompressedHeader(pc3, a, b, crc), "veri eksikse reddedilir");
}

static void AppendChar(W& w, const std::string& id, uint8_t race, uint16_t cls, uint8_t level, uint8_t face, uint32_t hair,
	uint8_t zone, uint32_t base)
{
	// DBAgent::LoadCharInfo: str id, u8 irk, u16 sinif, u8 seviye, u8 yuz, u32 sac, u8 bolge, 8 x (u32, u16)
	w.str(id).u8(race).u16(cls).u8(level).u8(face).u32(hair).u8(zone);
	for (int i = 0; i < 8; i++)
		w.u32(id.empty() ? 0 : base + i).u16(id.empty() ? 0 : 100 + i);
}

static void TestAllCharInfo()
{
	std::printf("[test] WIZ_ALLCHAR_INFO_REQ 2369\n");
	W w;
	w.u8(1).u8(1);
	AppendChar(w, "Savasci", 1, 101, 83, 2, 0x01020304, 21, 1000);
	AppendChar(w, "", 0, 0, 0, 0, 0, 0, 0);
	AppendChar(w, "Rahip", 12, 305, 60, 1, 7, 1, 2000);
	AppendChar(w, "Dorduncu", 2, 201, 10, 0, 0, 21, 3000);
	Packet p = w.pkt();
	std::vector<KoProto::CharInfo2369> chars;
	uint8_t sub = 0;
	CHECK(KoProto::ParseAllCharInfo2369(p, chars, &sub) && sub == 1 && chars.size() == 4, "4 karakter (%zu)", chars.size());
	if (chars.size() == 4)
	{
		CHECK(chars[0].id == "Savasci" && chars[0].cls == 101 && chars[0].level == 83 && chars[0].hair == 0x01020304
				  && chars[0].zone == 21,
			"karakter 0");
		CHECK(chars[0].itemID[0] == 1000 && chars[0].itemDur[7] == 107, "karakter 0 esyalar");
		CHECK(chars[1].id.empty(), "bos yuva");
		CHECK(chars[2].id == "Rahip" && chars[2].race == 12, "karakter 2");
		CHECK(chars[3].id == "Dorduncu", "karakter 3");
	}
	CHECK(p.rpos() == p.size(), "tum paket tuketildi");

	W w2;
	w2.u8(2).u8(0);
	Packet p2 = w2.pkt();
	CHECK(!KoProto::ParseAllCharInfo2369(p2, chars, &sub) && sub == 2, "alt opcode 2 yok sayilir");
}

// SendMyInfo klan blogu: klan yoksa sunucu u64 0, u16 pelerin, u32 0 yazar
static void AppendMyInfoNoClan(W& w, uint16_t cape)
{
	w.u64(0).u16(cape).u32(0);
}
static void AppendMyInfoClan(W& w)
{
	// u16 ittifak, u8 bayrak, str(u8) ad, u8 derece, u8 sira, u16 isaret; u16 pelerin, u8 r,g,b, u8 bayrak
	w.u16(77).u8(2).str("KlanAdi").u8(3).u8(9).u16(5).u16(1234).u8(10).u8(20).u8(30).u8(1);
}

static void BuildMyInfo(W& w, bool clan)
{
	w.SByte();
	w.u16(4242).str("Mobil").u16(8140).u16(4375).u16(469).u8(2).u8(12).u16(305).u8(1).u32(0x00AABBCC).u8(0).u8(0).u8(0).u8(0);
	w.u8(83).i16(5).i64(123456789012LL).i64(99999999LL).u32(5000).u32(100).i16(clan ? 77 : -1).u8(clan ? 1 : 0);
	if (clan)
		AppendMyInfoClan(w);
	else
		AppendMyInfoNoClan(w, 0xFFFF);
	w.u8(2).u8(3).u8(4).u8(5);
	w.i16(3200).i16(3100).i16(2200).i16(2100).u32(1000).u32(250);
	w.u8(60).u8(1).u8(70).u8(2).u8(80).u8(3).u8(90).u8(4).u8(100).u8(5);
	w.i16(999).i16(888);
	w.u8(11).u8(12).u8(13).u8(14).u8(15).u8(16);
	w.u32(123456789).u8(0).i8(-1).i8(3);
	for (int i = 0; i < 9; i++)
		w.u8((uint8_t) (10 + i));
	for (int i = 0; i < KoProto::INVENTORY_TOTAL_2369; i++)
	{
		bool has = (i == 1 || i == 4 || i == 14 || i == 41 || i == 74);
		w.u32(has ? 100000 + i : 0).i16(has ? 20000 : 0).u16(has ? 1 : 0).u8(0).u16(0).u32(0).u32(0);
	}
	w.SByte();
	w.u8(1).u8(1).u8(2).u16(48); // hesap durumu, 1 premium (tip 2, 48 saat)
	w.u8(2).u8(0).u32(30000).u8(1).u8(2).u8(3).u8(4).u8(5).u8(0).u16(0).u8(1).u8(5).u8(5).u8(5).u8(5).u8(5);
	w.u64(0).u16(0).u16(0).u32(1).u32(0).u8(0);
}

static void TestMyInfo()
{
	std::printf("[test] WIZ_MYINFO 2369\n");
	for (int clan = 0; clan < 2; clan++)
	{
		W w;
		BuildMyInfo(w, clan == 1);
		Packet p = w.pkt();
		KoProto::MyInfo2369 m;
		CHECK(KoProto::ParseMyInfo2369(p, m), "ayristirma (klan=%d)", clan);
		CHECK(m.id == 4242 && m.name == "Mobil" && m.x == 8140 && m.z == 4375 && m.y == 469, "kimlik/konum");
		CHECK(m.nation == 2 && m.race == 12 && m.cls == 305 && m.face == 1 && m.hair == 0x00AABBCC, "irk/sinif/sac");
		CHECK(m.level == 83 && m.points == 5 && m.maxExp == 123456789012LL && m.exp == 99999999LL, "seviye/exp");
		CHECK(m.loyalty == 5000 && m.monthlyLoyalty == 100, "np");
		if (clan)
			CHECK(m.clanID == 77 && m.clan.name == "KlanAdi" && m.clan.grade == 3 && m.clan.ranking == 9 && m.clan.capeID == 1234
					  && m.clan.capeR == 10 && m.clan.capeFlag == 1 && m.clan.allianceID == 77 && m.clan.flag == 2,
				"klan bilgisi");
		else
			CHECK(m.clanID == -1 && m.clan.name.empty() && m.clan.capeID == 0xFFFF, "klan yok");
		CHECK(m.maxHp == 3200 && m.hp == 3100 && m.maxMp == 2200 && m.mp == 2100 && m.maxWeight == 1000 && m.weight == 250,
			"hp/mp/agirlik");
		CHECK(m.str == 60 && m.strBonus == 1 && m.cha == 100 && m.chaBonus == 5 && m.totalHit == 999 && m.totalAc == 888,
			"statlar");
		CHECK(m.resist[0] == 11 && m.resist[5] == 16 && m.gold == 123456789 && m.authority == 0, "direnc/altin");
		CHECK(m.knightsRank == -1 && m.personalRank == 3 && m.skill[0] == 10 && m.skill[8] == 18, "rutbe/beceri");
		CHECK(m.items[1].num == 100001 && m.items[1].duration == 20000 && m.items[1].count == 1 && m.items[0].num == 0,
			"yuva esyalari");
		CHECK(m.items[14].num == 100014 && m.items[41].num == 100041 && m.items[74].num == 100074, "envanter/cantalar");
		CHECK(m.accountStatus == 1 && m.premium.size() == 1 && m.premium[0].type == 2 && m.premium[0].hours == 48, "premium");
		CHECK(m.premiumInUse == 2 && m.manner == 30000 && m.military[4] == 5 && m.rebirthLevel == 1 && m.symbol == 1, "kuyruk");
		CHECK(p.rpos() == p.size(), "tum paket tuketildi (%zu/%zu)", p.rpos(), p.size());
	}
	W bad;
	bad.SByte().u16(1).str("x").u16(1);
	Packet pb = bad.pkt();
	KoProto::MyInfo2369 m;
	CHECK(!KoProto::ParseMyInfo2369(pb, m), "kesik paket reddedilir");
}

// GetUserInfo klan blogu
static void AppendUserNoClan(W& w, uint16_t cape)
{
	w.u32(0).u16(0).u8(0).u16(cape).u32(0).u8(0);
}
static void AppendUserClan(W& w)
{
	// u16 ittifak, str(u8) ad, u8 derece, u8 sira, u16 isaret; u16 pelerin, u8 r,g,b, u8 0; u8 bayrak
	w.u16(77).str("KlanAdi").u8(3).u8(9).u16(5).u16(1234).u8(10).u8(20).u8(30).u8(0).u8(2);
}

static void BuildUserInfo(W& w, bool clan)
{
	w.SByte();
	w.str("Diger").u8(1).u8(0).u8(0).i16(clan ? 77 : -1).u8(clan ? 5 : 0);
	if (clan)
		AppendUserClan(w);
	else
		AppendUserNoClan(w, 0xFFFF);
	w.u8(70).u8(1).u16(101).u16(100).u16(200).u16(30).u8(2).u32(0x11223344);
	w.u8(2).u32(1).u8(2).u8(0).u8(1).u8(0).u8(0).u16(0).i16(-900).u8(0).u8(9).u16(0).i8(-1).i8(-1);
	for (int i = 0; i < KoProto::USER_ITEM_SLOTS_2369; i++)
		w.u32(i < 8 ? 200000 + i : 0).i16(i < 8 ? 5000 : 0).u8(0);
	w.u8(21).u8(0xFF).u8(0xFF).u32(0).u8(0).u8(0).u8(0).u8(0).u8(0).u16(0).u32(1).u32(0).u16(0);
}

// v167 telefon günlüğünden ham MYINFO (opcode sonrası, osman): MaxHP 2002, HP 100, MaxMP 1982, MP 1982, altın 1000010331
static void TestMyInfoRaw2369()
{
	std::printf("TestMyInfoRaw2369\n");
	const char* hex =
		"01 00 05 6f 73 6d 61 6e 93 22 46 29 c4 00 02 0b ce 00 00 00 00 00 00 00 00 00 00 53 24 01 90 d1 ea 06 02 00 00 00 "
		"92 fa c9 02 00 00 00 00 bf 58 00 00 52 55 00 00 99 3a 05 00 00 03 05 48 55 4d 41 4e 01 00 01 00 03 00 00 00 00 01 "
		"02 03 04 05 d2 07 64 00 be 07 be 07 f6 22 00 00 cd 01 00 00 48 18 44 00 3c 00 32 00 32 00 19 00 93 00 00 00 00 00 "
		"00 00 5b f2 9a 3b 00 ff ff 37 00 00 00 00 50 00 0d 00";
	std::vector<uint8_t> b;
	for (const char* p = hex; *p;)
	{
		while (*p == ' ') p++;
		if (!*p) break;
		b.push_back((uint8_t) std::strtoul(std::string(p, 2).c_str(), nullptr, 16));
		p += 2;
	}
	// eşya listesi + kuyruk: sıfırlarla doldur (okuyucu yalnız yeterli bayt ister)
	b.resize(b.size() + 19 * KoProto::INVENTORY_TOTAL_2369 + 8, 0);
	Packet p;
	p.append(b.data(), b.size());
	KoProto::MyInfo2369 m;
	bool ok = KoProto::ParseMyInfo2369(p, m);
	CHECK(ok, "ham MYINFO cozulemedi");
	CHECK(m.name == "osman", "ad %s", m.name.c_str());
	CHECK(m.level == 83, "seviye %d", (int) m.level);
	CHECK(m.maxHp == 2002 && m.hp == 100, "HP %d/%d (2002/100 bekleniyor)", (int) m.hp, (int) m.maxHp);
	CHECK(m.maxMp == 1982 && m.mp == 1982, "MP %d/%d", (int) m.mp, (int) m.maxMp);
	CHECK(m.maxWeight == 8950 && m.weight == 461, "agirlik %u/%u", (unsigned) m.weight, (unsigned) m.maxWeight);
	CHECK(m.str == 72 && m.sta == 68, "STR %d STA %d", (int) m.str, (int) m.sta);
	CHECK(m.gold == 1000010331u, "altin %u", (unsigned) m.gold);
	CHECK(m.clanID == 15001 && m.clan.name == "HUMAN", "klan %d %s", (int) m.clanID, m.clan.name.c_str());
}

static void TestUserInfo()
{
	std::printf("[test] WIZ_USER_INOUT / REQ_USERIN 2369\n");
	for (int clan = 0; clan < 2; clan++)
	{
		W w;
		BuildUserInfo(w, clan == 1);
		Packet p = w.pkt();
		KoProto::UserInfo2369 u;
		CHECK(KoProto::ParseUserInfo2369(p, u), "ayristirma (klan=%d)", clan);
		CHECK(u.name == "Diger" && u.nation == 1 && u.level == 70 && u.race == 1 && u.cls == 101, "temel alanlar");
		CHECK(u.x == 100 && u.z == 200 && u.y == 30 && u.face == 2 && u.hair == 0x11223344, "konum/gorunum");
		CHECK(u.resHpType == 2 && u.abnormalType == 1 && u.needParty == 2 && u.partyLeader == 1 && u.direction == -900
				  && u.kingFlag == 9,
			"durum");
		if (clan)
			CHECK(u.clanID == 77 && u.fame == 5 && u.clan.name == "KlanAdi" && u.clan.capeID == 1234 && u.clan.flag == 2,
				"klan");
		else
			CHECK(u.clanID == -1 && u.clan.name.empty() && u.clan.capeID == 0xFFFF && u.clan.flag == 0, "klan yok");
		CHECK(u.items[0].num == 200000 && u.items[7].num == 200007 && u.items[7].duration == 5000 && u.items[8].num == 0,
			"esyalar");
		CHECK(u.zone == 21 && u.symbol == 1, "kuyruk");
		CHECK(p.rpos() == p.size(), "tum paket tuketildi (%zu/%zu)", p.rpos(), p.size());
	}
	// WIZ_USER_INOUT akisi: u16 tur, u16 kimlik, bilgi
	W w;
	w.u16(3).u16(55);
	BuildUserInfo(w, false);
	Packet p = w.pkt();
	CHECK(p.read<uint16_t>() == 3 && p.read<uint16_t>() == 55, "inout basligi");
	KoProto::UserInfo2369 u;
	CHECK(KoProto::ParseUserInfo2369(p, u) && u.name == "Diger", "inout govdesi");
}

static void TestNpcInfo()
{
	std::printf("[test] WIZ_NPC_INOUT / REQ_NPCIN 2369 (varsayilan NPC)\n");
	W w;
	// GetNpcInfo default: i16 proto, u8 tur, u16 pid, i32 satis grubu, u8 tip, u32 0, i16 boyut, u32 silah1, u32 silah2,
	// u8 ulus, u8 seviye, u16 x, u16 z, u16 y, u32 kapi, u8 nesne tipi, u16 0, u16 0, i16 yon
	w.i16(1001).u8(2).u16(1001).i32(31).u8(5).u32(0).i16(120).u32(110000001).u32(0).u8(2).u8(55).u16(8100).u16(4300).u16(400)
		.u32(1).u8(0).u16(0).u16(0).i16(-1200);
	Packet p = w.pkt();
	KoProto::NpcInfo2369 n;
	CHECK(KoProto::ParseNpcInfo2369(p, n), "ayristirma");
	CHECK(n.protoID == 1001 && n.kind == 2 && n.pid == 1001 && n.sellingGroup == 31 && n.type == 5 && n.size == 120, "kimlikler");
	CHECK(n.weapon1 == 110000001 && n.nation == 2 && n.level == 55 && n.x == 8100 && n.z == 4300 && n.y == 400, "konum");
	CHECK(n.gateOpen == 1 && n.objectType == 0 && n.direction == -1200, "kapi/yon");
	CHECK(p.rpos() == p.size(), "tum paket tuketildi");
	// WIZ_REQ_NPCIN: u16 adet, adet x (u16 kimlik + bilgi)
	W r;
	r.u16(2);
	for (int i = 0; i < 2; i++)
	{
		r.u16(10000 + i);
		r.i16(1001).u8(1).u16(1001).i32(0).u8(0).u32(0).i16(100).u32(0).u32(0).u8(0).u8(10).u16(1).u16(2).u16(3).u32(0).u8(0)
			.u16(0).u16(0).i16(0);
	}
	Packet pr = r.pkt();
	CHECK(pr.read<uint16_t>() == 2, "adet");
	for (int i = 0; i < 2; i++)
	{
		CHECK(pr.read<uint16_t>() == 10000 + i, "kimlik %d", i);
		CHECK(KoProto::ParseNpcInfo2369(pr, n) && n.kind == 1 && n.level == 10, "npc %d", i);
	}
	CHECK(pr.rpos() == pr.size(), "liste tuketildi");
}

static void TestChatMap()
{
	std::printf("[test] sohbet tipi eslemesi\n");
	CHECK(KoProto::MapChatType2369(1) == N3_CHAT_NORMAL && KoProto::MapChatType2369(2) == N3_CHAT_PRIVATE, "1,2");
	CHECK(KoProto::MapChatType2369(6) == N3_CHAT_CLAN && KoProto::MapChatType2369(8) == N3_CHAT_WAR, "6,8");
	CHECK(KoProto::MapChatType2369(9) == N3_CHAT_TITLE && KoProto::MapChatType2369(10) == N3_CHAT_TITLE_DELETE, "9,10");
	CHECK(KoProto::MapChatType2369(15) == N3_CHAT_CLAN && KoProto::MapChatType2369(17) == N3_CHAT_WAR, "15,17");
	CHECK(KoProto::MapChatType2369(21) == 0 && KoProto::MapChatType2369(200) == 0, "yok sayilanlar");
	// ChatPacket::Construct: u8 tur, u8 ulus, i16 gonderen, str(u8) ad, str(u16) mesaj, i8 rutbe, u8 sistem
	W w;
	w.u8(1).u8(2).i16(77).SByte().str("Mobil").DByte().str("selam").i8(-1).u8(0);
	Packet p = w.pkt();
	uint8_t t = KoProto::MapChatType2369(p.read<uint8_t>());
	int nation = p.read<uint8_t>();
	int id     = p.read<int16_t>();
	std::string name, msg;
	int nl = p.read<uint8_t>();
	p.readString(name, nl);
	int ml = p.read<int16_t>();
	p.readString(msg, ml);
	CHECK(t == N3_CHAT_NORMAL && nation == 2 && id == 77 && name == "Mobil" && msg == "selam", "1298 okuyucu ile uyumlu");
}

static void TestBuilders()
{
	std::printf("[test] gonderilen paketler\n");
	std::vector<uint8_t> b;
	KoProto::Configure(KoProto::V2369);
	KoProto::BuildServerListReq(b);
	CHECK(b.size() == 3 && b[0] == LS_SERVERLIST && b[1] == 0 && b[2] == 0, "2369 LS_SERVERLIST + echo");
	KoProto::BuildAllCharInfoReq(b);
	CHECK(b.size() == 2 && b[0] == WIZ_ALLCHAR_INFO_REQ && b[1] == 1, "2369 ALLCHAR + alt opcode");
	KoProto::BuildSelectCharacter(b, "acc", "Chr", 1, 21);
	CHECK(b.size() == 1 + 2 + 3 + 2 + 3 + 1 && b.back() == 1, "2369 SEL_CHAR (zoneCur yok) %zu", b.size());
	KoProto::BuildGameStart(b, 1, "Chr");
	CHECK(b.size() == 6 && b[0] == WIZ_GAMESTART && b[1] == 1 && b[2] == 3 && b[3] == 'C', "2369 GAMESTART + ad");
	KoProto::BuildMove(b, 100, 200, -30, 45, 1, 90, 190, -31);
	CHECK(b.size() == 1 + 2 + 2 + 2 + 2 + 1 + 6 && b[0] == WIZ_MOVE && b[9] == 1 && b[10] == 90, "2369 MOVE + mevcut konum");
	// Sunucu MoveProcess okuyusu: u16 x, u16 z, u16 y, i16 hiz, u8 echo, u16 cx, u16 cz, u16 cy
	Packet p;
	p.append(b.data() + 1, b.size() - 1);
	CHECK(p.read<uint16_t>() == 100 && p.read<uint16_t>() == 200 && p.read<int16_t>() == -30 && p.read<int16_t>() == 45
			  && p.read<uint8_t>() == 1 && p.read<uint16_t>() == 90 && p.read<uint16_t>() == 190 && p.read<int16_t>() == -31,
		"sunucu okuyusu");

	KoProto::Configure(KoProto::V1298);
	KoProto::BuildServerListReq(b);
	CHECK(b.size() == 1, "1298 LS_SERVERLIST");
	KoProto::BuildAllCharInfoReq(b);
	CHECK(b.size() == 1, "1298 ALLCHAR");
	KoProto::BuildSelectCharacter(b, "acc", "Chr", 1, 21);
	CHECK(b.size() == 13 && b.back() == 21, "1298 SEL_CHAR zoneCur dahil");
	KoProto::BuildGameStart(b, 2, "Chr");
	CHECK(b.size() == 2 && b[1] == 2, "1298 GAMESTART");
	KoProto::BuildMove(b, 100, 200, -30, 45, 3, 90, 190, -31);
	CHECK(b.size() == 10, "1298 MOVE");
}

int main()
{
	TestConfig();
	TestServerList();
	TestLoginReply();
	TestVersionAndCompressed();
	TestAllCharInfo();
	TestMyInfo();
	TestMyInfoRaw2369();
	TestUserInfo();
	TestNpcInfo();
	TestChatMap();
	TestBuilders();
	if (g_fail)
	{
		std::printf("ko_proto_test: %d hata\n", g_fail);
		return 1;
	}
	std::printf("ko_proto_test: tum testler gecti\n");
	return 0;
}
