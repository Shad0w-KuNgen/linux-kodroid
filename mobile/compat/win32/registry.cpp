// registry.cpp — Windows kayıt defterinin dosya tabanlı öykünmesi.
// Oyun yalnızca HKEY_CURRENT_USER altında ikili (REG_BINARY) ayarlar saklıyor
// (pencere konumları vb.). Değerler "<dizin>/ko_registry.ini" içinde hex olarak tutulur.
#include <windows.h>
#include <ko_fopen.h> // KoResolvePath

#include <cstdio>
#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace
{
struct Key
{
	std::string path;
};
std::string g_dir = ".";
std::map<std::string, std::map<std::string, std::pair<DWORD, std::vector<uint8_t>>>> g_store;
bool g_loaded = false;

std::string FilePath() { return KoResolvePath(g_dir + "/ko_registry.ini"); }

void Load()
{
	if (g_loaded)
		return;
	g_loaded = true;
	std::ifstream in(FilePath());
	std::string line, section;
	while (std::getline(in, line))
	{
		if (line.empty())
			continue;
		if (line.front() == '[' && line.back() == ']')
		{
			section = line.substr(1, line.size() - 2);
			continue;
		}
		auto eq = line.find('=');
		if (eq == std::string::npos)
			continue;
		std::string name = line.substr(0, eq), val = line.substr(eq + 1);
		auto colon = val.find(':');
		if (colon == std::string::npos)
			continue;
		DWORD type = (DWORD) std::strtoul(val.substr(0, colon).c_str(), nullptr, 10);
		std::string hex = val.substr(colon + 1);
		std::vector<uint8_t> bytes;
		for (size_t i = 0; i + 1 < hex.size(); i += 2)
			bytes.push_back((uint8_t) std::strtoul(hex.substr(i, 2).c_str(), nullptr, 16));
		g_store[section][name] = {type, bytes};
	}
}

void Save()
{
	std::ofstream out(FilePath(), std::ios::trunc);
	for (const auto& [section, values] : g_store)
	{
		out << '[' << section << "]\n";
		for (const auto& [name, tv] : values)
		{
			out << name << '=' << tv.first << ':';
			char b[3];
			for (uint8_t c : tv.second)
			{
				std::snprintf(b, sizeof(b), "%02x", c);
				out << b;
			}
			out << '\n';
		}
	}
}
} // namespace

void KoRegistrySetDirectory(const char* dir)
{
	g_dir    = dir ? dir : ".";
	g_loaded = false;
}

LONG RegOpenKey(HKEY, LPCSTR subKey, HKEY* out)
{
	Load();
	std::string path = subKey ? subKey : "";
	if (g_store.find(path) == g_store.end())
		return ERROR_FILE_NOT_FOUND;
	*out = new Key {path};
	return ERROR_SUCCESS;
}

LONG RegOpenKeyEx(HKEY root, LPCSTR subKey, DWORD, DWORD, HKEY* out) { return RegOpenKey(root, subKey, out); }

LONG RegCreateKey(HKEY, LPCSTR subKey, HKEY* out)
{
	Load();
	std::string path = subKey ? subKey : "";
	g_store[path];
	*out = new Key {path};
	Save();
	return ERROR_SUCCESS;
}

LONG RegSetValueEx(HKEY key, LPCSTR name, DWORD, DWORD type, const BYTE* data, DWORD len)
{
	if (!key)
		return ERROR_FILE_NOT_FOUND;
	auto* k = static_cast<Key*>(key);
	g_store[k->path][name ? name : ""] = {type, std::vector<uint8_t>(data, data + len)};
	Save();
	return ERROR_SUCCESS;
}

LONG RegQueryValueEx(HKEY key, LPCSTR name, LPDWORD, LPDWORD type, LPBYTE data, LPDWORD len)
{
	if (!key)
		return ERROR_FILE_NOT_FOUND;
	auto* k = static_cast<Key*>(key);
	auto it = g_store.find(k->path);
	if (it == g_store.end())
		return ERROR_FILE_NOT_FOUND;
	auto vit = it->second.find(name ? name : "");
	if (vit == it->second.end())
		return ERROR_FILE_NOT_FOUND;
	if (type)
		*type = vit->second.first;
	DWORD need = (DWORD) vit->second.second.size();
	if (data)
	{
		if (!len || *len < need)
		{
			if (len) *len = need;
			return ERROR_MORE_DATA;
		}
		std::memcpy(data, vit->second.second.data(), need);
	}
	if (len)
		*len = need;
	return ERROR_SUCCESS;
}

LONG RegDeleteValue(HKEY key, LPCSTR name)
{
	if (!key)
		return ERROR_FILE_NOT_FOUND;
	auto* k = static_cast<Key*>(key);
	g_store[k->path].erase(name ? name : "");
	Save();
	return ERROR_SUCCESS;
}

LONG RegCloseKey(HKEY key)
{
	delete static_cast<Key*>(key);
	return ERROR_SUCCESS;
}
