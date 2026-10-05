// tbl_test.cpp — .tbl şifre katmanları (klasik XOR, 2xxx DES) başsız testi.
// Örnek tablolar: platform/tests/data (co3moz/ko-tbl-reader test dizininden, MIT).
#include "KoTableCrypt.h"
#include "KoTableSchemas.h"
#include "N3TableBase.h"

#include <filesystem>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
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

static std::vector<uint8_t> ReadFile(const std::string& path)
{
	std::ifstream f(path, std::ios::binary);
	return std::vector<uint8_t>((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

static uint32_t U32(const std::vector<uint8_t>& d, size_t pos)
{
	uint32_t v = 0;
	memcpy(&v, d.data() + pos, 4);
	return v;
}

// Tabloyu baştan sona yürüt; tüketilen bayt sayısını döndür (hata: 0)
static size_t Walk(const std::vector<uint8_t>& d, uint32_t& cols, uint32_t& rows)
{
	static const size_t SZ[10] = { 0, 1, 1, 2, 2, 4, 4, 0, 4, 8 };
	cols = U32(d, 0);
	std::vector<uint32_t> types(cols);
	for (uint32_t i = 0; i < cols; i++)
		types[i] = U32(d, 4 + 4 * i);
	size_t pos = 4 + 4 * cols;
	rows       = U32(d, pos);
	pos += 4;
	for (uint32_t r = 0; r < rows; r++)
		for (uint32_t c = 0; c < cols; c++)
		{
			if (types[c] == 7)
			{
				if (pos + 4 > d.size())
					return 0;
				uint32_t len = U32(d, pos);
				pos += 4 + len;
			}
			else
				pos += SZ[types[c]];
			if (pos > d.size())
				return 0;
		}
	return pos;
}

static void TestLayer2(const std::string& file, uint32_t expCols, uint32_t expRows, const char* expPrefixHex)
{
	std::printf("[test] 2xxx DES katmani: %s\n", file.c_str());
	std::vector<uint8_t> raw = ReadFile(std::string(KO_TEST_DATA_DIR) + "/" + file);
	CHECK(!raw.empty(), "dosya okunamadi");
	if (raw.empty())
		return;
	CHECK(KoTableIsLayer2(raw.data(), raw.size()), "katman-2 tespiti");
	CHECK(!KoTableHeaderLooksValid(raw.data(), raw.size()), "ham veri baslik gibi gorunmemeli");
	uint8_t prefix[5] = {};
	std::vector<uint8_t> data = raw;
	CHECK(KoTableLayer2Decrypt(data, prefix), "cozum");
	char hex[11] = {};
	for (int i = 0; i < 5; i++)
		snprintf(hex + 2 * i, 3, "%02x", prefix[i]);
	CHECK(strcmp(hex, expPrefixHex) == 0, "onek %s (beklenen %s)", hex, expPrefixHex);
	CHECK(KoTableHeaderLooksValid(data.data(), data.size()), "cozulen baslik");
	uint32_t cols = 0, rows = 0;
	size_t used = Walk(data, cols, rows);
	CHECK(cols == expCols && rows == expRows, "sutun %u satir %u (beklenen %u/%u)", cols, rows, expCols, expRows);
	CHECK(used == data.size(), "tum veri tuketildi (%zu/%zu)", used, data.size());
	CHECK(prefix[4] == (uint8_t) cols, "onek[4] == sutun sayisi");

	// Bozuk uzunluk alani reddedilmeli, veri değişmemeli
	std::vector<uint8_t> bad = raw;
	bad[16] = 0x7f;
	const std::vector<uint8_t> badCopy = bad;
	bool badOk = KoTableLayer2Decrypt(bad);
	CHECK(!badOk, "gecersiz uzunluk reddedilir (sonuc %d, boyut %zu/%zu)", badOk, bad.size(), raw.size());
	CHECK(bad == badCopy, "reddedilen veri degismemeli");
}

static void TestXorRoundTrip()
{
	std::printf("[test] klasik XOR katmani\n");
	// Küçük sentetik tablo: 2 sütun (dword, string), 1 satır
	std::vector<uint8_t> plain = { 2, 0, 0, 0, 6, 0, 0, 0, 7, 0, 0, 0, 1, 0, 0, 0, 0x39, 0x30, 0, 0, 2, 0, 0, 0, 'h', 'i' };
	CHECK(KoTableHeaderLooksValid(plain.data(), plain.size()), "sentetik baslik");
	// Şifrele (tbl aracının Encrypt'i): cipher = plain ^ (key_r>>8); key_r = (cipher + key_r)*c1 + c2
	std::vector<uint8_t> enc = plain;
	uint16_t key_r = 0x0816;
	for (auto& b : enc)
	{
		uint8_t c = (uint8_t) (b ^ (key_r >> 8));
		key_r     = (uint16_t) ((c + key_r) * 0x6081 + 0x1608);
		b         = c;
	}
	CHECK(!KoTableIsLayer2(enc.data(), enc.size()), "XOR verisi katman-2 sanilmamali");
	KoTableXorDecrypt(enc.data(), enc.size());
	CHECK(enc == plain, "XOR gidis-donus");
	std::vector<uint8_t> junk(40, 0xAB);
	CHECK(!KoTableHeaderLooksValid(junk.data(), junk.size()), "cop veri baslik degil");
}

// 1.298 __TABLE_ZONE ile aynı düzen (24 sütun: D T×8 I I T T I T F T D×4 T I T)
struct Zone24
{
	uint32_t dwID = 0;
	std::string s1, s2, s3, s4, s5, s6, s7, s8;
	int i1 = 0, i2 = 0;
	std::string s9, s10;
	int i3 = 0;
	std::string s11;
	float f1 = 0;
	std::string s12;
	uint32_t d1 = 0, d2 = 0, d3 = 0, d4 = 0;
	std::string s13;
	int i4 = 0;
	std::string s14;
};

static void TestSchemaAlign()
{
	std::printf("[test] 2xxx sema hizalama\n");
	// Zones 27 -> 24 (elle eşleme)
	std::vector<uint32_t> file27, out;
	for (char c : std::string("DTTTTTTTTTIITTITFTDDDDTITTT"))
		file27.push_back(KoTableTypeFromLetter(c));
	std::vector<int> map;
	std::string rep;
	CHECK(KoTableAlignSchema("Data\\Zones.tbl", file27, out, map, rep), "zones hizalama: %s", rep.c_str());
	CHECK(out.size() == 24 && map.size() == 27 && map[8] == 8 && map[9] == -1 && map[10] == 9 && map[24] == 23 && map[25] == -1,
		"zones eşleme hatalı: %s", rep.c_str());
	// Otomatik hizalama: Texts'e bir string ve bir int eklenmiş olsun (D T T I) -> (D T)
	std::vector<uint32_t> ft = { 6, 7, 7, 5 };
	CHECK(KoTableAlignSchema("texts_us.tbl", ft, out, map, rep) && out.size() == 2 && map[0] == 0 && map[1] == 1 && map[2] == -1
			  && map[3] == -1,
		"texts otomatik hizalama: %s", rep.c_str());
	// Bilinmeyen tablo adı
	CHECK(!KoTableAlignSchema("yok.tbl", ft, out, map, rep), "bilinmeyen sema reddedilir");

	// Gerçek dosya: zones2195.tbl -> Zones.tbl adıyla CN3TableBase<Zone24> ile yükle
	std::filesystem::path tmp = std::filesystem::temp_directory_path() / "ko_tbl_test";
	std::filesystem::create_directories(tmp);
	std::filesystem::copy_file(std::string(KO_TEST_DATA_DIR) + "/zones2195.tbl", tmp / "Zones.tbl",
		std::filesystem::copy_options::overwrite_existing);
	CN3TableBase<Zone24> tbl;
	CHECK(tbl.LoadFromFile((tmp / "Zones.tbl").string()), "Zones.tbl (2195) yuklenemedi");
	CHECK(tbl.GetSize() == 87, "satir sayisi %d", tbl.GetSize());
	Zone24* z = tbl.Find(10);
	CHECK(z != nullptr, "bolge 10 yok");
	if (z)
	{
		CHECK(z->s1 == "Zones\\karus2004.gtd" && z->s2 == "karus2004", "bolge 10 gtd/ad: %s / %s", z->s1.c_str(), z->s2.c_str());
		CHECK(z->s8 == "Misc\\Sky\\Karus.N3Sky", "gokyuzu (HDR sutunu atlanmali): %s", z->s8.c_str());
		CHECK(z->i1 == 1 && z->i2 == 0 && z->s9 == "Zones\\karus_start.glo " && z->f1 == 1.0f, "int/glo/float");
		CHECK(z->d1 == 20003 && z->d2 == 95 && z->s13 == "Zones\\karus2004.opdsub" && z->i4 == 1
				  && z->s14 == "Zones\\karus2004.evtsub",
			"kuyruk sutunlari: %s %s", z->s13.c_str(), z->s14.c_str());
	}
}

int main()
{
	TestXorRoundTrip();
	TestSchemaAlign();
	TestLayer2("zones2195.tbl", 27, 87, "1b0000001b");
	TestLayer2("itemcrash_us1886.tbl", 4, 220, "a54e000804");
	if (g_fail)
	{
		std::printf("ko_tbl_test: %d hata\n", g_fail);
		return 1;
	}
	std::printf("ko_tbl_test: tum testler gecti\n");
	return 0;
}
