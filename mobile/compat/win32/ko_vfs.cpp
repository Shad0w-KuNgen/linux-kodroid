// ko_vfs.cpp — bkz. ko_vfs.h
#include "ko_vfs.h"

#include <sys/stat.h>
#include <dirent.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
// --- SHA-1 (FIPS 180-1) ---
struct Sha1
{
	uint32_t h[5] = { 0x67452301u, 0xEFCDAB89u, 0x98BADCFEu, 0x10325476u, 0xC3D2E1F0u };
	uint8_t block[64] {};
	size_t blockLen = 0;
	uint64_t total  = 0;
	static uint32_t rol(uint32_t v, int n)
	{
		return (v << n) | (v >> (32 - n));
	}
	void processBlock(const uint8_t* p)
	{
		uint32_t w[80];
		for (int i = 0; i < 16; ++i)
			w[i] = (uint32_t) p[i * 4] << 24 | (uint32_t) p[i * 4 + 1] << 16 | (uint32_t) p[i * 4 + 2] << 8 | (uint32_t) p[i * 4 + 3];
		for (int i = 16; i < 80; ++i)
			w[i] = rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
		uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
		for (int i = 0; i < 80; ++i)
		{
			uint32_t f, k;
			if (i < 20) { f = (b & c) | (~b & d); k = 0x5A827999u; }
			else if (i < 40) { f = b ^ c ^ d; k = 0x6ED9EBA1u; }
			else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDCu; }
			else { f = b ^ c ^ d; k = 0xCA62C1D6u; }
			uint32_t t = rol(a, 5) + f + e + k + w[i];
			e = d; d = c; c = rol(b, 30); b = a; a = t;
		}
		h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e;
	}
	void update(const void* data, size_t len)
	{
		const uint8_t* p = static_cast<const uint8_t*>(data);
		total += len;
		while (len > 0)
		{
			size_t take = std::min(len, sizeof(block) - blockLen);
			std::memcpy(block + blockLen, p, take);
			blockLen += take;
			p += take;
			len -= take;
			if (blockLen == sizeof(block))
			{
				processBlock(block);
				blockLen = 0;
			}
		}
	}
	void finish(uint8_t out[20])
	{
		uint64_t bits = total * 8;
		uint8_t pad   = 0x80;
		update(&pad, 1);
		uint8_t zero = 0;
		while (blockLen != 56)
			update(&zero, 1);
		uint8_t lenBytes[8];
		for (int i = 0; i < 8; ++i)
			lenBytes[i] = (uint8_t) (bits >> (56 - 8 * i));
		update(lenBytes, 8);
		for (int i = 0; i < 5; ++i)
		{
			out[i * 4]     = (uint8_t) (h[i] >> 24);
			out[i * 4 + 1] = (uint8_t) (h[i] >> 16);
			out[i * 4 + 2] = (uint8_t) (h[i] >> 8);
			out[i * 4 + 3] = (uint8_t) (h[i]);
		}
	}
};

// Pearl Guard LoadCrypto: CryptHashData(parola, 29) → CryptDeriveKey(RC4, 128 bit) = SHA1'in ilk 16 baytı
const char ISTIRAP_PASSWORD[] = "(A;dq1DPVFgVs1Aez$VS3R0hge@NvM_TJvblD4.af0h@r4bUzp";
constexpr size_t ISTIRAP_PASSWORD_LEN = 29;

const uint8_t* IstirapKey()
{
	static uint8_t key[16];
	static bool init = false;
	if (!init)
	{
		uint8_t digest[20];
		KoSha1(ISTIRAP_PASSWORD, ISTIRAP_PASSWORD_LEN, digest);
		std::memcpy(key, digest, 16);
		init = true;
	}
	return key;
}

std::string Lower(std::string s)
{
	for (char& c : s)
		c = (char) tolower((unsigned char) c);
	return s;
}

bool Exists(const std::string& p, uint64_t* size = nullptr)
{
	struct stat sb {};
	if (stat(p.c_str(), &sb) != 0)
		return false;
	if (size)
		*size = (uint64_t) sb.st_size;
	return true;
}

void MkDirs(const std::string& dir)
{
	std::string cur;
	for (size_t i = 0; i <= dir.size(); i++)
	{
		if (i == dir.size() || dir[i] == '/')
		{
			if (!cur.empty())
				mkdir(cur.c_str(), 0775);
		}
		if (i < dir.size())
			cur.push_back(dir[i]);
	}
}

// Dizin içinde harf duyarsız ad (tek seviye)
std::string FindEntry(const std::string& dir, const std::string& nameLower)
{
	DIR* d = opendir(dir.empty() ? "." : dir.c_str());
	if (!d)
		return std::string();
	std::string found;
	while (dirent* e = readdir(d))
	{
		if (Lower(e->d_name) == nameLower)
		{
			found = e->d_name;
			break;
		}
	}
	closedir(d);
	return found;
}

bool ReadAll(const std::string& path, std::vector<uint8_t>& out)
{
	FILE* f = ::fopen(path.c_str(), "rb");
	if (!f)
		return false;
	fseek(f, 0, SEEK_END);
	long n = ftell(f);
	fseek(f, 0, SEEK_SET);
	out.resize(n > 0 ? (size_t) n : 0);
	bool ok = n <= 0 || fread(out.data(), 1, out.size(), f) == out.size();
	fclose(f);
	return ok;
}

bool WriteAll(const std::string& path, const uint8_t* data, size_t n)
{
	std::string tmp = path + ".tmp";
	FILE* f         = ::fopen(tmp.c_str(), "wb");
	if (!f)
		return false;
	bool ok = n == 0 || fwrite(data, 1, n, f) == n;
	fclose(f);
	if (!ok)
	{
		::remove(tmp.c_str());
		return false;
	}
	::remove(path.c_str());
	return ::rename(tmp.c_str(), path.c_str()) == 0;
}

// --- Paketler (ui.hdr/ui.src, item.hdr/item.src, ...) ---
// 2369 istemcisi klasörleri "<ad>.hdr" (dizin) + "<ad>.src" (ham ardışık veri) olarak paketleyebilir.
// Dizin: u32 kayıt sayısı; her kayıt u32 adUzunluk | ad | u32 ofset | u32 boyut. Kayıt adı alt yol
// içerebilir ("item\\x.n3cpart"); hem tam ad hem yalın dosya adı ile aranır.
struct PackEntry
{
	uint32_t offset = 0, size = 0;
};
struct UiPack
{
	bool loaded = false, valid = false;
	std::string srcPath, cacheDir;
	std::unordered_map<std::string, PackEntry> entries; // küçük harf ad → kayıt
};
std::mutex g_mutex;
std::unordered_map<std::string, UiPack> g_packs; // "<base>|<klasör>" → paket (geçersiz de olsa bir kez bakılır)

void LogVfs(const char* fmt, const std::string& a, const std::string& b = std::string())
{
	fprintf(stderr, fmt, a.c_str(), b.c_str());
	fputc('\n', stderr);
}

std::string ToSlash(std::string s)
{
	for (char& c : s)
		if (c == '\\')
			c = '/';
	return s;
}

bool ParsePackIndex(const std::string& hdrPath, UiPack& pack)
{
	std::vector<uint8_t> h;
	if (!ReadAll(hdrPath, h) || h.size() < 4)
		return false;
	auto u32 = [&](size_t pos) { uint32_t v; memcpy(&v, h.data() + pos, 4); return v; };
	uint32_t count = u32(0);
	size_t pos     = 4;
	for (uint32_t i = 0; i < count; i++)
	{
		if (pos + 4 > h.size())
			break;
		uint32_t nameLen = u32(pos);
		pos += 4;
		if (nameLen > 1024 || pos + nameLen + 8 > h.size())
			break;
		std::string name((const char*) h.data() + pos, nameLen);
		pos += nameLen;
		PackEntry e;
		e.offset = u32(pos);
		e.size   = u32(pos + 4);
		pos += 8;
		std::string key = Lower(ToSlash(name));
		while (!key.empty() && key[0] == '/')
			key.erase(0, 1);
		pack.entries[key] = e;
		size_t slash = key.find_last_of('/');
		if (slash != std::string::npos)
			pack.entries.emplace(key.substr(slash + 1), e); // yalın ad (ilk kayıt kazanır)
	}
	return !pack.entries.empty();
}

// "<base><klasör>/" için paket: <klasör>/<klasör>.hdr, <base>/<klasör>.hdr ya da klasördeki ilk *.hdr (+ eş .src)
UiPack& LoadPack(const std::string& base, const std::string& dirLower)
{
	UiPack& pack = g_packs[base + "|" + dirLower];
	if (pack.loaded)
		return pack;
	pack.loaded      = true;
	std::string dirE = FindEntry(base, dirLower);
	std::string hdr, src;
	if (!dirE.empty())
	{
		std::string dir = base + dirE + "/";
		std::string h   = FindEntry(dir, dirLower + ".hdr"), s = FindEntry(dir, dirLower + ".src");
		if (!h.empty() && !s.empty())
		{
			hdr = dir + h;
			src = dir + s;
		}
		else if (DIR* d = opendir(dir.c_str()))
		{
			// Klasördeki herhangi bir <x>.hdr + <x>.src çifti
			while (dirent* e = readdir(d))
			{
				std::string n = e->d_name, nl = Lower(n);
				if (nl.size() > 4 && nl.compare(nl.size() - 4, 4, ".hdr") == 0)
				{
					std::string s2 = FindEntry(dir, nl.substr(0, nl.size() - 4) + ".src");
					if (!s2.empty())
					{
						hdr = dir + n;
						src = dir + s2;
						break;
					}
				}
			}
			closedir(d);
		}
	}
	if (hdr.empty())
	{
		std::string h = FindEntry(base, dirLower + ".hdr"), s = FindEntry(base, dirLower + ".src");
		if (!h.empty() && !s.empty())
		{
			hdr = base + h;
			src = base + s;
		}
	}
	if (hdr.empty())
		return pack;
	pack.srcPath  = src;
	pack.cacheDir = base + dirLower + "_cache/";
	pack.valid    = ParsePackIndex(hdr, pack);
	LogVfs("[ko-vfs] paket: %s (%s kayit)", hdr, std::to_string(pack.entries.size()));
	return pack;
}

std::string ExtractFromPack(UiPack& pack, const std::string& nameLower)
{
	auto it = pack.entries.find(nameLower);
	if (it == pack.entries.end())
		return std::string();
	const PackEntry& e = it->second;
	// Önbellek: aynı boyutta varsa yeniden kullan
	std::string flat = nameLower;
	for (char& c : flat)
		if (c == '/')
			c = '_';
	MkDirs(pack.cacheDir);
	std::string out = pack.cacheDir + flat;
	uint64_t have   = 0;
	if (Exists(out, &have) && have == e.size)
		return out;
	FILE* f = ::fopen(pack.srcPath.c_str(), "rb");
	if (!f)
		return std::string();
	std::vector<uint8_t> data(e.size);
	fseeko(f, (off_t) e.offset, SEEK_SET);
	bool ok = e.size == 0 || fread(data.data(), 1, data.size(), f) == data.size();
	fclose(f);
	if (!ok || !WriteAll(out, data.data(), data.size()))
		return std::string();
	return out;
}
} // namespace

void KoSha1(const void* data, size_t len, uint8_t out[20])
{
	Sha1 s;
	s.update(data, len);
	s.finish(out);
}

void KoRc4(const uint8_t* key, size_t keyLen, uint8_t* data, size_t len)
{
	uint8_t s[256];
	for (int k = 0; k < 256; ++k)
		s[k] = (uint8_t) k;
	uint8_t j = 0;
	for (int k = 0; k < 256; ++k)
	{
		j = (uint8_t) (j + s[k] + key[k % keyLen]);
		std::swap(s[k], s[j]);
	}
	uint8_t i = 0;
	j         = 0;
	for (size_t n = 0; n < len; ++n)
	{
		i = (uint8_t) (i + 1);
		j = (uint8_t) (j + s[i]);
		std::swap(s[i], s[j]);
		data[n] ^= s[(uint8_t) (s[i] + s[j])];
	}
}

void KoIstirapDecrypt(std::vector<uint8_t>& data)
{
	// PearlEngine::dcpUIF: ilk 4 bayt düz; blok = boyut çiftse 32, tekse 31; her blok RC4 başından
	if (data.size() <= 4)
		return;
	size_t blockLen = (data.size() % 2 == 0) ? 32 : 31;
	const uint8_t* key = IstirapKey();
	for (size_t pos = 4; pos < data.size(); pos += blockLen)
	{
		size_t n = std::min(blockLen, data.size() - pos);
		KoRc4(key, 16, data.data() + pos, n);
	}
}

std::string KoIstirapDecryptToCache(const std::string& existingPath)
{
	std::string p = existingPath;
	size_t slash  = p.find_last_of('/');
	std::string dir = slash == std::string::npos ? "" : p.substr(0, slash + 1);
	std::string name = slash == std::string::npos ? p : p.substr(slash + 1);
	// base = "…/istirap/" dizininin üstü
	std::string base = dir;
	if (base.size() >= 1)
	{
		size_t s2 = base.find_last_of('/', base.size() - 2);
		base      = s2 == std::string::npos ? "" : base.substr(0, s2 + 1);
	}
	std::string cacheDir = base + "ui_cache/istirap/";
	std::string out      = cacheDir + Lower(name) + ".uif";
	uint64_t srcSize = 0, have = 0;
	if (!Exists(p, &srcSize))
		return std::string();
	std::lock_guard<std::mutex> lock(g_mutex);
	if (Exists(out, &have) && have == srcSize)
		return out;
	std::vector<uint8_t> data;
	if (!ReadAll(p, data))
		return std::string();
	KoIstirapDecrypt(data);
	MkDirs(cacheDir);
	if (!WriteAll(out, data.data(), data.size()))
		return std::string();
	LogVfs("[ko-vfs] istirap cozuldu: %s -> %s", p, out);
	return out;
}

std::string KoVfsResolve(const std::string& normalizedPath)
{
	// "//" → "/" (2369 tablolarında "\\item\\x" gibi başı ayraçlı adlar temel yola eklenince oluşur)
	std::string path;
	path.reserve(normalizedPath.size());
	for (char c : normalizedPath)
		if (c != '/' || path.empty() || path.back() != '/')
			path.push_back(c);
	std::string low = Lower(path);
	if (low.size() > 8 && low.compare(low.size() - 8, 8, ".istirap") == 0)
		return KoIstirapDecryptToCache(path);
	// "<base>/<klasör>/<ad>": klasör paketi (ui, ui_us→ui, item, chr, ...)
	size_t slash = low.find_last_of('/');
	if (slash == std::string::npos || slash == 0)
		return std::string();
	std::string name    = low.substr(slash + 1);
	std::string dirPart = low.substr(0, slash); // "…/ui" ya da "ui"
	if (name.empty())
		return std::string();
	std::lock_guard<std::mutex> lock(g_mutex);
	// Paket klasörü yolun herhangi bir üst klasörü olabilir: "…/fx/billboard/x/y.dxt" → fx paketi, kayıt "billboard/x/y.dxt"
	// (fx.hdr kayıtları alt yollu). En derinden başlayarak en çok 4 seviye yukarı bakılır.
	std::string rel = name;    // paket içi göreli ad
	std::string cur = dirPart; // aday paket klasörü (tam yol)
	for (int level = 0; level < 4 && !cur.empty(); level++)
	{
		size_t slash2       = cur.find_last_of('/');
		std::string dirName = slash2 == std::string::npos ? cur : cur.substr(slash2 + 1);
		std::string base    = slash2 == std::string::npos ? std::string() : path.substr(0, slash2 + 1); // "" ya da "…/"
		if (dirName.empty())
			break;
		std::vector<std::string> dirs { dirName };
		if (dirName == "ui_us")
			dirs.push_back("ui");
		else if (dirName == "ui")
			dirs.push_back("ui_us");
		for (const std::string& d : dirs)
		{
			UiPack& pack = LoadPack(base, d);
			if (!pack.valid)
				continue;
			std::string r = ExtractFromPack(pack, rel);
			if (r.empty())
				r = ExtractFromPack(pack, d + "/" + rel);
			if (r.empty() && rel != name)
				r = ExtractFromPack(pack, name); // yalın ad
			if (!r.empty())
				return r;
		}
		rel = dirName + "/" + rel;
		cur = slash2 == std::string::npos ? std::string() : cur.substr(0, slash2);
	}
	return std::string();
}

void KoVfsReset()
{
	std::lock_guard<std::mutex> lock(g_mutex);
	g_packs.clear();
}
