// vfs_test.cpp — UI paketi (ui.hdr/ui.src) ve .istirap çözümü başsız testi (sentetik veriler).
#include "ko_vfs.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

std::string KoResolvePath(const std::string& path); // paths.cpp

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

namespace fs = std::filesystem;

static void Put32(std::vector<uint8_t>& v, uint32_t x)
{
	for (int i = 0; i < 4; i++)
		v.push_back((uint8_t) (x >> (8 * i)));
}
static void PutStr(std::vector<uint8_t>& v, const std::string& s)
{
	Put32(v, (uint32_t) s.size());
	v.insert(v.end(), s.begin(), s.end());
}
static std::vector<uint8_t> Read(const std::string& p)
{
	std::ifstream f(p, std::ios::binary);
	return std::vector<uint8_t>((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}
static void Write(const fs::path& p, const std::vector<uint8_t>& d)
{
	std::ofstream f(p, std::ios::binary);
	f.write((const char*) d.data(), (std::streamsize) d.size());
}

static void TestPack(bool sizeIncludesHeader)
{
	std::printf("[test] UI paketi (boyut %s)\n", sizeIncludesHeader ? "kayit" : "veri");
	fs::path tmp = fs::temp_directory_path() / (sizeIncludesHeader ? "ko_vfs_test_a" : "ko_vfs_test_b");
	fs::remove_all(tmp);
	fs::create_directories(tmp / "UI");
	std::string base = tmp.string() + "/";

	// ui.src: iki kayıt
	std::vector<uint8_t> src;
	std::vector<uint8_t> dataA = { 'N', 'T', 'F', 3, 1, 2, 3, 4, 5 }, dataB(300, 0xAB);
	std::string pathA = "E:\\ui\\icon.dxt", pathB = "E:\\ui\\co_login.uif";
	uint32_t offA = (uint32_t) src.size();
	PutStr(src, pathA);
	src.insert(src.end(), dataA.begin(), dataA.end());
	uint32_t offB = (uint32_t) src.size();
	PutStr(src, pathB);
	src.insert(src.end(), dataB.begin(), dataB.end());
	uint32_t recA = (uint32_t) (4 + pathA.size() + dataA.size()), recB = (uint32_t) (4 + pathB.size() + dataB.size());

	std::vector<uint8_t> hdr;
	Put32(hdr, 2);
	PutStr(hdr, "co_login.uif");
	Put32(hdr, offB);
	Put32(hdr, sizeIncludesHeader ? recB : (uint32_t) dataB.size());
	PutStr(hdr, "ICON.dxt");
	Put32(hdr, offA);
	Put32(hdr, sizeIncludesHeader ? recA : (uint32_t) dataA.size());
	Write(tmp / "UI" / "ui.hdr", hdr);
	Write(tmp / "UI" / "ui.src", src);
	KoVfsReset();

	std::string r1 = KoResolvePath(base + "ui\\co_login.uif");
	CHECK(fs::exists(r1) && Read(r1) == dataB, "co_login.uif paketten cikmadi: %s", r1.c_str());
	std::string r2 = KoResolvePath(base + "ui_us\\icon.DXT"); // 1.298 yolu + harf duyarsız
	CHECK(fs::exists(r2) && Read(r2) == dataA, "icon.dxt (ui_us, harf) paketten cikmadi: %s", r2.c_str());
	std::string r3 = KoResolvePath(base + "ui\\yok.uif");
	CHECK(!fs::exists(r3), "olmayan dosya var sanildi");
	// Önbellek yeniden kullanımı: aynı yol
	CHECK(KoResolvePath(base + "ui\\co_login.uif") == r1, "onbellek yolu sabit");
	fs::remove_all(tmp);
}

static void TestIstirap()
{
	std::printf("[test] .istirap cozumu\n");
	// Anahtar: SHA1(parola[:29]) ilk 16 bayt — bilinen SHA1 test vektörüyle KoSha1 doğrulaması
	uint8_t d[20];
	KoSha1("abc", 3, d);
	const uint8_t abc[20] = { 0xa9, 0x99, 0x3e, 0x36, 0x47, 0x06, 0x81, 0x6a, 0xba, 0x3e, 0x25, 0x71, 0x78, 0x50, 0xc2, 0x6c,
		0x9c, 0xd0, 0xd8, 0x9d };
	CHECK(memcmp(d, abc, 20) == 0, "SHA1(abc)");
	// RC4 bilinen vektör: anahtar "Key", "Plaintext" → BBF316E8D940AF0AD3
	uint8_t pt[9] = { 'P', 'l', 'a', 'i', 'n', 't', 'e', 'x', 't' };
	KoRc4((const uint8_t*) "Key", 3, pt, 9);
	const uint8_t rc4[9] = { 0xBB, 0xF3, 0x16, 0xE8, 0xD9, 0x40, 0xAF, 0x0A, 0xD3 };
	CHECK(memcmp(pt, rc4, 9) == 0, "RC4(Key, Plaintext)");

	for (int parity = 0; parity < 2; parity++)
	{
		// Sentetik UIF: 4 bayt sürüm + 100/101 bayt içerik; şifreleme = çözme (RC4 simetrik, aynı blok kuralı)
		std::vector<uint8_t> plain;
		for (int i = 0; i < 104 + parity; i++)
			plain.push_back((uint8_t) (i * 7 + 3));
		std::vector<uint8_t> enc = plain;
		KoIstirapDecrypt(enc);
		CHECK(enc != plain && memcmp(enc.data(), plain.data(), 4) == 0, "ilk 4 bayt duz, kalan sifreli");
		fs::path tmp = fs::temp_directory_path() / "ko_vfs_test_c";
		fs::remove_all(tmp);
		fs::create_directories(tmp / "ISTIRAP");
		std::string name = parity ? "re_login_intro.istirap" : "login.istirap";
		Write(tmp / "ISTIRAP" / name, enc);
		std::string r = KoResolvePath(tmp.string() + "/istirap\\" + name);
		CHECK(r.find("ui_cache/istirap/") != std::string::npos && Read(r) == plain, "istirap cozumu (%s): %s", name.c_str(), r.c_str());
		fs::remove_all(tmp);
	}
}

int main()
{
	TestPack(true);
	TestPack(false);
	TestIstirap();
	if (g_fail)
	{
		std::printf("ko_vfs_test: %d hata\n", g_fail);
		return 1;
	}
	std::printf("ko_vfs_test: tum testler gecti\n");
	return 0;
}
