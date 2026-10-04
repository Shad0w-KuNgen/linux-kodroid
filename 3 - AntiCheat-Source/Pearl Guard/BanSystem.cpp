#include "BanSystem.h"
#include <fstream>

std::string GetDirectory(const std::string& path)
{
	size_t found = path.find_last_of("/\\");
	return(path.substr(0, found));
}

bool is_file_exist(const char* fileName)
{
	std::ifstream infile(fileName);
	return infile.good();
}

void BanSystem::SetBan()
{
	for (std::string key : regKeys) {
		HKEY LocalMachine = CreateKey(HKEY_LOCAL_MACHINE, key.c_str());
		RegCloseKey(LocalMachine);
	}
	for (std::string file : files)
	{
		if (!is_file_exist(file.c_str()))
		{
			
				CreateFileA(file.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		}
	}
}

void BanSystem::RemoveBan()
{
	for (std::string key : regKeys) {
		RegDeleteKeyExA(HKEY_LOCAL_MACHINE, key.c_str(), REG_OPTION_NON_VOLATILE, NULL);
	}
	for (std::string file : files)
	{
		if (is_file_exist(file.c_str()))
		{
			DeleteFileA(file.c_str());
		}
	}
}

BanSystem::BanSystem()
{
	systemDrive = getenv(xorstr("SystemDrive"));

	regKeys = {
		xorstr("SOFTWARE\\OEM\\Sorceressr"),
		xorstr("SOFTWARE\\Google\\Chrome\\WebHistory"),
		xorstr("SOFTWARE\\Windows\\CurrentVersion\\ManuelProxyr"),
		xorstr("SOFTWARE\\Windows\\CurrentVersion\\Startup\\Processesr"),
		xorstr("SOFTWARE\\Microsoft\\Analog\\Providers\\Coupler"),
		xorstr("SOFTWARE\\Microsoft\\Analog\\Providers\\Singler"),
	};

	//files.push_back(systemDrive + xorstr("\\Windows\\System32\\drivers\\complating.sys"));
	files.push_back(systemDrive + xorstr("\\ProgramData\\Microsoft\\NetFramework\\chnet.sys"));
	files.push_back(systemDrive + xorstr("\\ProgramData\\Microsoft\\windesktop.ini"));
	files.push_back(systemDrive + xorstr("\\ProgramData\\Microsoft\\Windows\\SystemData\\systeminfo.data"));
	files.push_back(systemDrive + xorstr("\\ProgramData\\Microsoft\\Windows\\SystemData\\systeminfo.info"));
}

bool BanSystem::RegCheck()
{
	return false;
	for (std::string key : regKeys) if (KeyExists(HKEY_LOCAL_MACHINE, key.c_str())) return true;
	return false;
}

bool BanSystem::FileCheck()
{
	for (std::string file : files)
		if (is_file_exist(file.c_str()))
		{
			//MessageBox(::GetActiveWindow(), file.c_str(), "DirectInput", MB_OK);
			return true;
		}
		
	return false;
}

HKEY BanSystem::CreateKey(HKEY hRootKey, LPCSTR strKey)
{
	HKEY hKey;
	LONG nError = RegCreateKeyEx(hRootKey, strKey, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	return hKey;
}

bool BanSystem::KeyExists(HKEY hRootKey, LPCSTR strKey)
{
	HKEY hKey;
	LONG nError = RegOpenKeyEx(hRootKey, strKey, NULL, KEY_ALL_ACCESS, &hKey);
	return nError != ERROR_FILE_NOT_FOUND;
}

HKEY BanSystem::OpenKey(HKEY hRootKey, LPCSTR strKey)
{
	HKEY hKey;
	LONG nError = RegOpenKeyEx(hRootKey, strKey, NULL, KEY_ALL_ACCESS, &hKey);
	if (nError == ERROR_FILE_NOT_FOUND)
		nError = RegCreateKeyEx(hRootKey, strKey, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	return hKey;
}

void BanSystem::SetVal(HKEY hKey, LPCTSTR lpValue, DWORD data)
{
	LONG nError = RegSetValueEx(hKey, lpValue, NULL, REG_DWORD, (LPBYTE)&data, sizeof(DWORD));
}

DWORD BanSystem::GetVal(HKEY hKey, LPCTSTR lpValue)
{
	DWORD data;
	DWORD size = sizeof(data);
	DWORD type = REG_DWORD;

	LONG nError = RegQueryValueExA(hKey, lpValue, NULL, &type, (LPBYTE)&data, &size);
	if (nError == ERROR_FILE_NOT_FOUND)
		data = 0;
	return data;
}

BOOL BanSystem::WriteDirectory(string dd)
{
	HANDLE fFile;
	WIN32_FIND_DATA fileinfo;
	vector<string> m_arr;
	BOOL tt;
	int x1 = 0;
	string tem = "";

	fFile = FindFirstFile(dd.c_str(), &fileinfo);
	if (fileinfo.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
	{
		FindClose(fFile);
		return TRUE;
	}

	m_arr.clear();
	for (x1 = 0; x1 < dd.length(); x1++)
	{
		if (dd.at(x1) != '\\')
			tem += dd.at(x1);
		else
		{
			m_arr.push_back(tem);
			tem += "\\";
		}
		if (x1 == dd.length() - 1)
			m_arr.push_back(tem);
	}

	FindClose(fFile);
	for (x1 = 1; x1 < m_arr.size(); x1++)
	{
		tem = m_arr.at(x1);
		tt = CreateDirectory(tem.c_str(), NULL);
		if (tt)
			SetFileAttributes(tem.c_str(), FILE_ATTRIBUTE_NORMAL);
	}
	m_arr.clear();
	fFile = FindFirstFile(dd.c_str(), &fileinfo);
	if (fileinfo.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
	{
		FindClose(fFile);
		return TRUE;
	}
	else
	{
		FindClose(fFile);
		return FALSE;
	}
}