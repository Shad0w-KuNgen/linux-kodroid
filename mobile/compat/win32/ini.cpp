// ini.cpp — GetPrivateProfileString/Int ve WritePrivateProfileString (basit INI okuyucu).
#include <windows.h>

#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace
{
std::string Trim(const std::string& s)
{
	size_t a = s.find_first_not_of(" \t\r\n"), b = s.find_last_not_of(" \t\r\n");
	return a == std::string::npos ? std::string() : s.substr(a, b - a + 1);
}
std::string Lower(std::string s)
{
	for (char& c : s) c = (char) tolower((unsigned char) c);
	return s;
}
using Ini = std::map<std::string, std::vector<std::pair<std::string, std::string>>>;

Ini Read(const char* file)
{
	Ini ini;
	std::ifstream in(file ? file : "");
	std::string line, section;
	while (std::getline(in, line))
	{
		line = Trim(line);
		if (line.empty() || line[0] == ';' || line[0] == '#')
			continue;
		if (line.front() == '[' && line.back() == ']')
		{
			section = Lower(Trim(line.substr(1, line.size() - 2)));
			ini[section];
			continue;
		}
		auto eq = line.find('=');
		if (eq == std::string::npos)
			continue;
		ini[section].emplace_back(Trim(line.substr(0, eq)), Trim(line.substr(eq + 1)));
	}
	return ini;
}
} // namespace

DWORD GetPrivateProfileString(LPCSTR section, LPCSTR key, LPCSTR def, LPSTR out, DWORD outSize, LPCSTR file)
{
	if (!out || outSize == 0)
		return 0;
	Ini ini = Read(file);
	std::string value = def ? def : "";
	auto it = ini.find(Lower(section ? section : ""));
	if (it != ini.end() && key)
	{
		std::string k = Lower(key);
		for (const auto& kv : it->second)
			if (Lower(kv.first) == k)
			{
				value = kv.second;
				if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"') || (value.front() == '\'' && value.back() == '\'')))
					value = value.substr(1, value.size() - 2);
				break;
			}
	}
	std::snprintf(out, outSize, "%s", value.c_str());
	return (DWORD) std::strlen(out);
}

UINT GetPrivateProfileInt(LPCSTR section, LPCSTR key, INT def, LPCSTR file)
{
	char buf[64];
	GetPrivateProfileString(section, key, "", buf, sizeof(buf), file);
	if (buf[0] == 0)
		return (UINT) def;
	return (UINT) std::strtol(buf, nullptr, 0);
}

BOOL WritePrivateProfileString(LPCSTR section, LPCSTR key, LPCSTR value, LPCSTR file)
{
	Ini ini = Read(file);
	std::string sec = Lower(section ? section : "");
	auto& entries   = ini[sec];
	bool found      = false;
	for (auto& kv : entries)
		if (Lower(kv.first) == Lower(key ? key : ""))
		{
			kv.second = value ? value : "";
			found     = true;
		}
	if (!found && key)
		entries.emplace_back(key, value ? value : "");
	std::ofstream outf(file ? file : "", std::ios::trunc);
	for (const auto& [s, kvs] : ini)
	{
		outf << '[' << s << "]\n";
		for (const auto& kv : kvs)
			outf << kv.first << '=' << kv.second << '\n';
	}
	return TRUE;
}
