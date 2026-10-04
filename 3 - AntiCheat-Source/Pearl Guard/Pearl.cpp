#include "BanSystem.h"
#include "stdafx.h"
#include "PearlGui.h"
#include "HDRReader.h"
#include "splash.h"
#include <TlHelp32.h>


BanSystem* banSystem = NULL;
void OpenConsoleWindow();

PearlEngine* Engine = NULL;
HDRReader* hdrReader = NULL;
std::string basePath = "";
int SkillBaseTime = GetTickCount() + 25000;

void LoadCrypto();



void Shutdown(string message = "");

bool _fexists(std::string& filename)
{
	std::ifstream ifile(filename.c_str());
	return (bool)ifile;
}

std::string getexepath()
{

	char result[MAX_PATH];
	string ret = std::string(result, GetModuleFileName(NULL, result, MAX_PATH));
	return ret.substr(0, ret.find_last_of("\\"));
}

DWORD GetFileSize(const char* FileName)
{
	std::ifstream in(FileName, std::ifstream::ate | std::ifstream::binary);
	return in.tellg();
}

void Shutdown(string message)
{

	Engine->power = false;
	string s1 = xorstr("The file is corrupted. Please contact with administrator.\n");
	string s2 = xorstr("If you constantly see this message, ");
	string s3 = xorstr("disable the anti-virus program.");

	if (message == "")
		MessageBoxA(NULL, string(s1 + s2 + s3).c_str(), xorstr("KnightOnline.exe"), MB_OK | MB_ICONEXCLAMATION);
	else
		MessageBoxA(NULL, message.c_str(), xorstr("KnightOnLine.exe"), MB_OK | MB_ICONEXCLAMATION);

	exit(0);
	FreeLibrary(GetModuleHandle(NULL));
	TerminateProcess(GetCurrentProcess(), 0);
}

// dosya yol değiştirme

void eraseSubStr(std::string& mainStr, std::string& toErase)
{
	std::transform(mainStr.begin(), mainStr.end(), mainStr.begin(), ::tolower);
	std::transform(toErase.begin(), toErase.end(), toErase.begin(), ::tolower);

	size_t pos = mainStr.find(toErase);
	if (pos != std::string::npos)
	{
		// If found then erase it from string
		mainStr.erase(pos, toErase.length());
	}
}

typedef HANDLE(WINAPI* tCreateFileA)(
	LPCSTR                lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile
	);

tCreateFileA oCreateFileA;
tCreateFileA oCreateFileCameraA;

HANDLE WINAPI hkCreateFileCameraA(LPCSTR                lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile)
{
	return oCreateFileCameraA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

std::string __tmpfl = "";
DWORD _PID = 0;
HCRYPTKEY hKey = NULL;
std::string xCodeTMP = xorstr("Misc\\river\\caust36.dxt");

HANDLE WINAPI hkCreateFileA(LPCSTR                lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile)
{

	string fl = lpFileName;

	/**/
	/*if (fl.find(xorstr("Chr")) != string::npos)
	{

		Engine->str_replace(fl, "Chr", "Game\\Chr");
		return oCreateFileA(fl.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}*/
	/**/

	if ((int)dwFlagsAndAttributes != 137)

		return oCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	else
	{

		xCodeTMP = string_format(xorstr("Misc\\river\\caust36_%d.dxt"), _PID);
		PBYTE pbBuffer = NULL;
		HANDLE hSourceFile = oCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, hTemplateFile);
		HANDLE hDestinationFile = oCreateFileA(xCodeTMP.c_str(), FILE_WRITE_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE != hSourceFile && INVALID_HANDLE_VALUE != hDestinationFile)
		{

			bool fEOF = false;
			DWORD dwCount;
			LARGE_INTEGER size;
			GetFileSizeEx(hSourceFile, &size);
			LONG len = (size.u.HighPart << (LONG)32) | size.u.LowPart;
			DWORD dwBlockLen = len % 2 == 0 ? 32 : 31;
			DWORD dwBufferLen = dwBlockLen;
			if ((pbBuffer = (PBYTE)malloc(dwBufferLen)))
			{
				if (ReadFile(hSourceFile, pbBuffer, 4, &dwCount, NULL))
				{
					WriteFile(hDestinationFile, pbBuffer, dwCount, &dwCount, NULL);
				}
				ZeroMemory(pbBuffer, dwBufferLen);
				dwCount = 0;
				do
				{
					if (ReadFile(hSourceFile, pbBuffer, dwBlockLen, &dwCount, NULL))
					{
						if (dwCount < dwBlockLen)
						{
							fEOF = TRUE;
						}
						if (CryptDecrypt(hKey, 0, 1, 0, pbBuffer, &dwCount))
						{
							WriteFile(hDestinationFile, pbBuffer, dwCount, &dwCount, NULL);
						}
					}
				} while (!fEOF);
			}

			if (pbBuffer)
				free(pbBuffer);

			if (hSourceFile)
				CloseHandle(hSourceFile);

			if (hDestinationFile)
				CloseHandle(hDestinationFile);

			return oCreateFileA(xCodeTMP.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, hTemplateFile);
		}
	}
	return oCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

HANDLE myMutex = NULL;
#define CONSOLE_MODE 0  // 0 OLUNCA CONSOL KAPALI - 1 OLUNCA KONSOL AÇIK OLUYOR.
extern  "C"  __declspec(dllexport) void __cdecl Init()
{

	oCreateFileA = (tCreateFileA)DetourFunction((PBYTE)CreateFileA, (PBYTE)hkCreateFileA); // dosya yol değiştirme ---------------------
#if HOOK_SOURCE_VERSION == 1098
	//oCreateFileCameraA = (tCreateFileA)DetourFunction((PBYTE)0x004E200C, (PBYTE)hkCreateFileCameraA); // kamera dosya yol değiştirme ---------------------
	//*(DWORD*)(0x004E200C) = 0xE8; // for camera oCreateFileCameraA
	*(DWORD*)(0x004E1FFE) = 0x89;	// n3camera için şifreleme
	*(DWORD*)(0x004DBD51) = 0x89;	// n3camera için şifreleme
	*(DWORD*)(0x004DDCB6) = 0x89;	// n3camera için şifreleme
	*(DWORD*)(0x004DE89E) = 0x89;	// n3camera için şifreleme
	*(DWORD*)(0x004DDBD2) = 0x89;	// n3camera için şifreleme
	*(DWORD*)(0x004DDE6D) = 0x89;	// n3camera için şifreleme
	*(DWORD*)(0x004DEB9E) = 0x89;	// n3camera için şifreleme
	*(DWORD*)(0x004DDF46) = 0x89;	// n3camera için şifreleme //açılmicak
#endif

#if CONSOLE_MODE 1
	AllocConsole();
	freopen(xorstr("CONOUT$"), xorstr("w"), stdout);
#endif	

	PatchMulti();

	basePath = getexepath();
	string myname(basePath + xorstr("\\KnightOnLine.exe"));
	if (!_fexists(myname))
	{
		MessageBoxA(NULL, xorstr("KnightOnLine.exe name invalid"), xorstr("Error"), MB_OK | MB_ICONEXCLAMATION);
		Shutdown();
		return;
	}

	uint8_t	multicount = 0;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			std::string a = entry.szExeFile;
			std::size_t pos = a.find(xorstr("KnightOnLine.exe"));

			if (pos != -1)
				multicount++;
		}
	}

	if (multicount > 3)		// Açılacak olan multi client sayısı
	{
		ExitProcess(0);
		DWORD skillbase2 = 0;
		DWORD skillid2 = 0;
		DWORD value3 = 0;
		int counter = 0;
		unsigned int a2 = 0;
		DWORD skillbase4 = 0;
		DWORD skillid5 = 0;
		DWORD value6 = 0;
		int counter6 = 0;

		__asm
		{
			push   0
			push  0
			mov eax, 0
			mov ecx, 0
			call eax
			mov a2, eax
		}
	}

	/*************************************/

	myname = basePath + xorstr("\\ISTIRAP.dll");
	if (!_fexists(myname))
	{
		MessageBoxA(NULL, xorstr("\\ISTIRAP.dll"), xorstr("Need This File In Directory"), MB_OK | MB_ICONEXCLAMATION);
		Shutdown();
		return;
	}

	char nm[100];
	GetModuleBaseNameA(GetCurrentProcess(), GetModuleHandle(NULL), nm, sizeof(nm));
	string _nm(nm);

	Engine = new PearlEngine(basePath + xorstr("\\"));
	hdrReader = new HDRReader(basePath + xorstr("\\"));

	LoadCrypto();
	banSystem = new BanSystem;
	
	bool a = banSystem->RegCheck();
	bool b = banSystem->FileCheck();

#if(ANTICHEAT_MODE)
	if (a || b)
	{
		Shutdown("Pc Blocked.");
		return;
	}

#endif
	UIMain();
}

void OpenConsoleWindow()
{
	AllocConsole();

	freopen("CONOUT$", "a", stdout);
	freopen("CONIN$", "r", stdin);
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpvReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		Init();
	}
	return TRUE;
}

// Uif Şifreleme başlangıç
HCRYPTPROV hCryptProv = NULL;
HCRYPTHASH hHash = NULL;
DWORD Func_Load = 0;
const std::string& fileStr = "";


bool has_suffix(const std::string& str, const std::string& suffix)
{
	return str.size() >= suffix.size() &&
		str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

void __stdcall uifEncrpytOperation()
{
	xCodeTMP = fileStr;

	if (!fileStr.empty() && has_suffix(fileStr, xorstr(".istirap")))
	{
		xCodeTMP = string_format(xorstr("Misc\\river\\caust36_%d.dxt"), _PID);
		PBYTE pbBuffer = NULL;
		HANDLE hSourceFile = CreateFileA(fileStr.c_str(), FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		HANDLE hDestinationFile = CreateFileA(xCodeTMP.c_str(), FILE_WRITE_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE != hSourceFile && INVALID_HANDLE_VALUE != hDestinationFile)
		{
			bool fEOF = false;
			DWORD dwCount;
			LARGE_INTEGER size;
			GetFileSizeEx(hSourceFile, &size);
			LONG len = (size.u.HighPart << (LONG)32) | size.u.LowPart;
			DWORD dwBlockLen = len % 2 == 0 ? 32 : 31;
			DWORD dwBufferLen = dwBlockLen;
			if ((pbBuffer = (PBYTE)malloc(dwBufferLen)))
			{
				if (ReadFile(hSourceFile, pbBuffer, 4, &dwCount, NULL))
				{
					WriteFile(hDestinationFile, pbBuffer, dwCount, &dwCount, NULL);
				}
				ZeroMemory(pbBuffer, dwBufferLen);
				dwCount = 0;
				do
				{
					if (ReadFile(hSourceFile, pbBuffer, dwBlockLen, &dwCount, NULL))
					{
						if (dwCount < dwBlockLen)
						{
							fEOF = TRUE;
						}
						if (CryptDecrypt(hKey, 0, 1, 0, pbBuffer, &dwCount))
						{
							WriteFile(hDestinationFile, pbBuffer, dwCount, &dwCount, NULL);
						}
					}
				} while (!fEOF);
			}
		}
		if (pbBuffer)
		{
			free(pbBuffer);
		}
		if (hSourceFile)
		{
			CloseHandle(hSourceFile);
		}
		if (hDestinationFile)
		{
			CloseHandle(hDestinationFile);
		}
	}
}

void __declspec(naked) hkLoad()
{
	__asm {
		mov edx, [esp + 4]
		mov fileStr, edx
		pushad
		pushfd
		call uifEncrpytOperation
		popfd
		popad
		mov edx, offset xCodeTMP
		mov[esp + 4], edx
		jmp Func_Load
	}
}

void __fastcall RemoveTMP()
{
	std::string file = string_format(xorstr("Misc\\river\\caust36_%d.dxt"), _PID);
	std::remove(file.c_str());
}

void __declspec(naked) hkEndOfLoad()
{
	__asm {
		pushad
		pushfd
	}

	__asm {
		popfd
		popad
		call KO_UIF_FILE_LOAD_ORG_CALL
		pushad
		pushfd
		call RemoveTMP
		popfd
		popad
		jmp KO_UIF_FILE_LOAD_RET
	}
}

void LoadCrypto()
{
	_PID = GetCurrentProcessId();
	if (CryptAcquireContext(&hCryptProv, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL, 0))
	{
		if (CryptCreateHash(hCryptProv, 0x8004, 0, 0, &hHash))
		{
			if (CryptHashData(hHash, (BYTE*)xorstr("(A;dq1DPVFgVs1Aez$VS3R0hge@NvM_TJvblD4.af0h@r4bUzp"), 29, CRYPT_USERDATA))
			{
				if (!CryptDeriveKey(hCryptProv, 0x6801, hHash, 0x00800000, &hKey))
					TerminateProcess(GetCurrentProcess(), 0);
			}
		}
	}
	Func_Load = (DWORD)DetourFunction((PBYTE)KO_UIF_FILE_LOAD, (PBYTE)hkLoad);
	DetourFunction((PBYTE)(KO_UIF_FILE_LOAD + 0x56), (PBYTE)hkEndOfLoad);
}
