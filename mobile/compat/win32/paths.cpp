// paths.cpp — Windows yol çözümlemesi: '\' → '/' ve büyük/küçük harfe duyarsız dosya bulma.
// Oyun, dosya adlarını küçük harfe çevirip "Data\Item.tbl" gibi yollarla açar; Linux/Android'de
// dosya sistemi duyarlı olduğundan her bileşen dizin içinde harf duyarsız aranır (önbellekli).
#include <windows.h>
#include <cstdio>
#undef fopen
#include "ko_vfs.h"

#include <dirent.h>
#include <sys/stat.h>

#include <map>
#include <cstring>
#include <mutex>
#include <unordered_set>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
std::mutex g_mutex;
// dizin → (küçük harf ad → gerçek ad)
std::unordered_map<std::string, std::unordered_map<std::string, std::string>> g_dirCache;

std::string Lower(std::string s)
{
	for (char& c : s)
		c = (char) tolower((unsigned char) c);
	return s;
}

const std::unordered_map<std::string, std::string>& DirEntries(const std::string& dir)
{
	auto it = g_dirCache.find(dir);
	if (it != g_dirCache.end())
		return it->second;
	auto& m = g_dirCache[dir];
	if (DIR* d = opendir(dir.empty() ? "." : dir.c_str()))
	{
		while (dirent* e = readdir(d))
			m.emplace(Lower(e->d_name), e->d_name);
		closedir(d);
	}
	return m;
}

bool Exists(const std::string& p)
{
	struct stat sb {};
	return stat(p.c_str(), &sb) == 0;
}
} // namespace

namespace
{
void (*g_missingFn)(const char*, void*) = nullptr;
void* g_missingUser                      = nullptr;
std::unordered_set<std::string> g_missingSeen;
std::mutex g_missingMutex;
} // namespace

void KoSetMissingFileCallback(void (*fn)(const char* path, void* user), void* user)
{
	g_missingFn   = fn;
	g_missingUser = user;
}

void KoLogMissingFile(const char* path)
{
	if (!path || !*path)
		return;
	{
		std::lock_guard<std::mutex> lock(g_missingMutex);
		if (g_missingSeen.size() > 2000 || !g_missingSeen.insert(path).second)
			return;
	}
	if (g_missingFn)
		g_missingFn(path, g_missingUser);
	else
		std::fprintf(stderr, "[ko] dosya bulunamadı: %s\n", path);
}

void KoPathCacheReset()
{
	std::lock_guard<std::mutex> lock(g_mutex);
	g_dirCache.clear();
}

static std::string ResolveOnce(const std::string& input)
{
	if (input.empty())
		return input;
	std::string p = input;
	for (char& c : p)
		if (c == '\\')
			c = '/';
	// "C:/..." gibi sürücü harfleri: kaldır
	if (p.size() >= 2 && isalpha((unsigned char) p[0]) && p[1] == ':')
		p = p.substr(2);
	if (Exists(p))
		return p;

	std::lock_guard<std::mutex> lock(g_mutex);
	bool absolute = !p.empty() && p[0] == '/';
	std::vector<std::string> parts;
	{
		std::string cur;
		for (char c : p)
		{
			if (c == '/')
			{
				if (!cur.empty())
					parts.push_back(cur);
				cur.clear();
			}
			else
				cur.push_back(c);
		}
		if (!cur.empty())
			parts.push_back(cur);
	}
	std::string resolved = absolute ? "/" : "";
	for (size_t i = 0; i < parts.size(); ++i)
	{
		const std::string& part = parts[i];
		std::string candidate   = resolved + part;
		if (part == "." || part == ".." || Exists(candidate))
		{
			resolved = candidate + (i + 1 < parts.size() ? "/" : "");
			continue;
		}
		const auto& entries = DirEntries(resolved.empty() ? "." : resolved);
		auto it             = entries.find(Lower(part));
		if (it == entries.end())
		{
			// Bulunamadı: dizin yeni oluşturulmuş olabilir, önbelleği tazeleyip bir kez daha dene
			g_dirCache.erase(resolved.empty() ? "." : resolved);
			const auto& fresh = DirEntries(resolved.empty() ? "." : resolved);
			auto it2          = fresh.find(Lower(part));
			if (it2 == fresh.end())
				return p; // olduğu gibi döndür (yazma için yeni dosya adı olabilir)
			resolved += it2->second;
		}
		else
			resolved += it->second;
		if (i + 1 < parts.size())
			resolved += "/";
	}
	return resolved;
}

// 2xxx istemci verisinde arayüz klasörü "UI", 1.298'de "UI_US": biri yoksa diğerini dene
std::string KoResolvePath(const std::string& input)
{
	std::string r = ResolveOnce(input);
	if (Exists(r))
	{
		// .istirap (şifreli UIF): çözülmüş önbellek kopyasını döndür
		if (r.size() > 8 && Lower(r.substr(r.size() - 8)) == ".istirap")
		{
			std::string d = KoVfsResolve(r);
			if (!d.empty())
				return d;
		}
		return r;
	}
	std::string low = Lower(input);
	for (char& c : low)
		if (c == '\\')
			c = '/';
	auto swapDir = [&](const char* from, const char* to) -> std::string {
		size_t pos = low.find(from);
		if (pos == std::string::npos || (pos > 0 && low[pos - 1] != '/'))
			return std::string();
		std::string alt = input;
		for (char& c : alt)
			if (c == '\\')
				c = '/';
		alt.replace(pos, strlen(from), to);
		std::string ra = ResolveOnce(alt);
		return Exists(ra) ? ra : std::string();
	};
	std::string alt = swapDir("ui_us/", "ui/");
	if (alt.empty())
		alt = swapDir("ui/", "ui_us/");
	if (!alt.empty())
		return alt;
	// 2369: UI paketi (ui.hdr/ui.src) içinden çıkar
	{
		std::string norm = input;
		for (char& c : norm)
			if (c == '\\')
				c = '/';
		if (norm.size() >= 2 && isalpha((unsigned char) norm[0]) && norm[1] == ':')
			norm = norm.substr(2);
		std::string v = KoVfsResolve(norm);
		if (!v.empty())
			return v;
	}
	return r;
}

FILE* ko_fopen(const char* path, const char* mode)
{
	if (!path)
		return nullptr;
	std::string resolved = KoResolvePath(path);
	FILE* f              = ::fopen(resolved.c_str(), mode ? mode : "rb");
	if (!f && mode && (mode[0] == 'w' || mode[0] == 'a'))
	{
		// Yazma: dizin kısmını çöz, dosya adını olduğu gibi kullan
		std::string p = path;
		for (char& c : p)
			if (c == '\\')
				c = '/';
		auto slash = p.find_last_of('/');
		if (slash != std::string::npos)
		{
			std::string dir = KoResolvePath(p.substr(0, slash));
			f               = ::fopen((dir + p.substr(slash)).c_str(), mode);
		}
	}
	if (!f && (!mode || mode[0] == 'r'))
		KoLogMissingFile(path);
	return f;
}
