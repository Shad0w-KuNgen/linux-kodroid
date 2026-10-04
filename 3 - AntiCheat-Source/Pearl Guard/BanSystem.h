#pragma once
#include "stdafx.h"

class BanSystem
{
private:
	HKEY CreateKey(HKEY hRootKey, LPCSTR strKey);
	bool KeyExists(HKEY hRootKey, LPCSTR strKey);
	BOOL WriteDirectory(string dd);

	std::vector<std::string> regKeys;
	std::vector<std::string> files;
	std::string systemDrive;
public:
	BanSystem();
	bool RegCheck();
	bool FileCheck();
	HKEY OpenKey(HKEY hRootKey, LPCSTR strKey);
	void SetVal(HKEY hKey, LPCTSTR lpValue, DWORD data);
	DWORD GetVal(HKEY hKey, LPCTSTR lpValue);
	void SetBan();
	void RemoveBan();
};

extern BanSystem* banSystem;