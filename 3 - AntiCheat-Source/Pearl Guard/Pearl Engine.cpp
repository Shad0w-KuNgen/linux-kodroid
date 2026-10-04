#include "Pearl Engine.h"
#include "PearlGui.h"
#include <thread>
#include <future>
#include <TlHelp32.h>
#include "sha1.hpp"
#include "RC5/RC5.h"
#include "CSpell.h"
#include "DateTime.h"
#include "BanSystem.h"
bool isLeaderAttack = false;
bool ischeatactive = false;
bool isHideUser = false;
bool isActivedArrow = false;
bool __genieAttackMove = true;
bool isActivedCure = false;
void SendHWID();
clock_t CheckAliveTime = clock();
clock_t Real_SendTime = clock();
std::string name="";
DWORD KO_UI_SCREEN_SIZE = 0x00DE297C;
extern void Shutdown(string message = "");
extern string WtoString(WCHAR s[]);
extern string strToLower(string str);
DWORD adres = 0;
POINT koScreen;
std::vector<uint16> antiAFKProto = {0};
bool isRFastAttack = false;
bool uiINIT = false;
bool pusINIT = false;
bool tmpItemsINIT = false;
bool PusRefundINIT = false;
bool isRenderObject = true;
vector<ProcessInfo> processTMP;
string tmpGraphics = xorstr("<unknown>");
string tmpProcessor = xorstr("<unknown>");
int sSocketMerchant = 0;
int sSocketMerchantSpecial = 0;

tGetObjectBase Func_GetObjectBase;

bool hasInfArrowItem = false; 
typedef struct
{
private:
	void* pVoid;
public:
	unsigned int Size;
	BYTE* Data;
}RECV_DATA;
uint32 RecvPrice = 0;
uint8 nisKC = 0;
typedef int (WINAPI* MyOldRecv) (SOCKET, uint8*, int, int);
typedef int (WSAAPI* MyRecv) (SOCKET, LPWSABUF, DWORD, LPDWORD, LPDWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE);
typedef int (WINAPI* MySend) (SOCKET, char*, int, int);
typedef int (WINAPI* MyConnect) (SOCKET, const sockaddr*, int);
typedef int (WSAAPI* MyWSAConnect) (SOCKET, const sockaddr*, int, LPWSABUF, LPWSABUF, LPQOS, LPQOS);
typedef int (WSAAPI* MyWSAStartup) (WORD, LPWSADATA);
typedef int (WINAPI* MyTerminateProcess) (HANDLE hProcess, UINT uExitCode);
typedef int (WINAPI* MyExitProcess) (UINT uExitCode);
typedef int (WINAPI* MyLoadTBL) (const std::string& szFN);

MyRecv OrigRecv = NULL;
MySend OrigSend = NULL;
MyConnect OrigConnect = NULL;
MyWSAConnect OrigWSAConnect = NULL;
MyWSAStartup OrigWSAStartup = NULL;
MyTerminateProcess OrigTerminateProcess = NULL;
MyExitProcess OrigExitProcess = NULL;
MyLoadTBL OrigLoadTBL = NULL;

bool allowAlive = true;
bool gameStarted = false;
bool isAlive = false;
DWORD KO_SET_UI_ORG = 0;
HANDLE thisProc = NULL;
DWORD KO_ADR = 0x0;
const DWORD KO_WH = 0x758;
const DWORD KO_PTR_PKT = 0x00F368D8;
const DWORD KO_SND_FNC = 0x004A58F0;	//2369
const DWORD KO_ACC = 0x00D8AC54;
const DWORD KO_ACC_SIZE = 0x00D8AC54 + 0x10;
DWORD KO_WHISPER_GETMESSAGE_ORG = 0;
DWORD KO_WHISPER_VIEWMESAAGE_ORG = 0;
DWORD KO_WHISPER_CREATE_ORG = 0;
DWORD KO_WHISPER_OPEN_ORG = 0;
DWORD KO_WHISPER_RECV_ORG = 0;
MyOldRecv OrigOldRecv = NULL;
void LM_Send(Packet * pkt);
void __stdcall LM_Shutdown(std::string log, std::string graphicCards = tmpGraphics, std::string processor = tmpProcessor);
void LM_SendProcess(uint16 toWHO);

const DWORD KO_FNSBB = KO_FNSB;
void ASMPatch(DWORD dst, char* src, int size)
{
	DWORD oldprotect;
	VirtualProtect((LPVOID)dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
	memcpy((LPVOID)dst, src, size);
	VirtualProtect((LPVOID)dst, size, oldprotect, &oldprotect);
}
CSpell* GetSkillBase(int iSkillID)
{
	DWORD retval = 0;
	__asm {
		mov ecx, KO_SMMB
		mov ecx, DWORD PTR DS : [ecx]
		push iSkillID
		call KO_FNSBB
		mov retval,eax
	}
	return (CSpell*)retval;
}

bool CheckSkill(DWORD skillID)
{
	if (skillID == 106725
		|| skillID == 106735
		|| skillID == 106760
		|| skillID == 206725
		|| skillID == 206735
		|| skillID == 206760
		|| skillID == 106775
		|| skillID == 206775
		|| skillID == 900111)
		return true;

	CSpell* spell = GetSkillBase(skillID);
	if (spell) {
		auto itcrc = Engine->skillcrc.find(spell->dwID + 2031);
		if (itcrc != Engine->skillcrc.end())
		{
			SpellCRC crc(crc32((uint8*)(DWORD)spell, 0xA8, -1), crc32((uint8*)((DWORD)spell + 0xB4), 0x2C, -1));
			return itcrc->second.part1 == crc.part1 && itcrc->second.part2 == crc.part2;
		}
	}
	return false;
}

// Nick renk, tip deðiþtirme
std::string objTMPStringX = "";

void SetNameString(DWORD userBase, std::string text, DWORD color, DWORD fontStyle)
{

	if (!userBase)
		return;

	DWORD fontBase = *(DWORD*)(userBase + 0x400/*0x3AC*/); // font olayý

	if (IsBadReadPtr((void*)fontBase, sizeof(DWORD)))
		return;
	if (!fontBase)
		return;

	objTMPStringX = text;
	__asm {
		push fontStyle
		push offset objTMPStringX
		mov ecx, fontBase
		call KO_UIF_SET_FONT_STRING
	}

	if (color != 0)
	{
		*(DWORD*)(userBase + 0x738 /*0x6A0*/) = color; // user nick color
		*(DWORD*)(fontBase + 0x44) = color; // font color
	}
}		// ----------------------------------------------------------

bool PearlEngine::WordGuardSystem(std::string Word, uint8 WordStr)
{
	char* pword = &Word[0];
	bool bGuard[32] = { false };
	std::string WordGuard = "qwertyuopadfhsgcijklzxvbnmQWERTYUOPSGICADFHJKLZXVBNM1234567890";
	char* pWordGuard = &WordGuard[0];
	for (uint8 i = 0; i < WordStr; i++)
	{
		for (uint8 j = 0; j < strlen(pWordGuard); j++)
			if (pword[i] == pWordGuard[j])
				bGuard[i] = true;

		if (bGuard[i] == false)
			return false;
	}
	return true;
}
uint8 nTime = 0;
DWORD WINAPI PearlEngine::SuspendCheck(PearlEngine* e)
{
	SYSTEMTIME time;
	GetLocalTime(&time);
	Sleep(1000);
	DWORD TimeTest1 = 0, TimeTest2 = 0;
	while (true)
	{
		
		if (time.wMinute % 10 == 0 && Engine->m_bHookStart && time.wSecond ==0)
			Engine->ClearListString(Engine->uiNoticeWind->m_pChatOut);

	Sleep(1000);
		TimeTest1 = TimeTest2;
		TimeTest2 = GetTickCount();
		if (TimeTest1 != 0)
		{
			Sleep(1000);
			if ((TimeTest2 - TimeTest1) > 8000)
				e->Shutdown(xorstr("You cannot suspend the game. Please do not use cheat software."));  // Bu hatayý alan kiþiler "Suspend" etmeye çalýþmýþ demektir.
		}
		if (WaitForSingleObject(e->MainThread, 1) == WAIT_OBJECT_0)
			e->Shutdown(xorstr("All the pieces of the game can't be working together."));
	}
}

std::string forbiddenModules[] = { xorstr("dbk64"), xorstr("dbk32"), xorstr("pchunter"), xorstr("hacker"), xorstr("PROCEXP152"), xorstr("BlackBoneDrv10"), xorstr("since"), xorstr("ntice"), xorstr("winice"), xorstr("syser"), xorstr("77fba431") };

DWORD WINAPI DriverScan(LPVOID lParam)
{
	VIRTUALIZER_START
	while (true) 
	{
		Sleep(3000);
		LPVOID drivers[ARRAY_SIZE];
		DWORD cbNeeded;
		int cDrivers, i;
		WCHAR szDriver[ARRAY_SIZE];
		if (EnumDeviceDrivers(drivers, sizeof(drivers), &cbNeeded) && cbNeeded < sizeof(drivers))
		{
			cDrivers = cbNeeded / sizeof(drivers[0]);
			for (i = 0; i < cDrivers; i++)
			{
				if (GetDeviceDriverBaseNameW(drivers[i], szDriver, sizeof(szDriver) / sizeof(szDriver[0])))
				{
					string driverName = strToLower(WtoString(szDriver));
					for (string fbDriver : forbiddenModules) {
						if (driverName.find(strToLower(fbDriver)) != std::string::npos) {
							string s1 = xorstr("An 3rd party tools has been detected on your system: %s\n");
							string s2 = xorstr("If you don't use any hacking stuff, ");  // Cheat Engine için uyarý ekle
							string s3 = xorstr("please restart computer and try again.");
							Shutdown(string_format(s1 + s2 + s3, driverName.c_str()));
						}
					}
				}
			}
		}
	}
	VIRTUALIZER_END
}

DWORD WINAPI AliveSend(LPVOID lParam)
{
	VIRTUALIZER_START
	while (true) {
	
		CheckAliveTime = clock();
		Engine->StayAlive();
		Sleep(7000);
	}
	VIRTUALIZER_END
}

void SendTarama(DWORD adresss)
{
	if (true)
	{
		Engine->mReturnAdressSend.insert(std::make_pair(0x4ABDA3, 0x4ABDA3));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4AC0A9, 0x4AC0A9));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4AC274, 0x4AC274));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4BB3E9, 0x4BB3E9));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4BB4AC, 0x4BB4AC));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4BB73A, 0x4BB73A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4BBA9A, 0x4BBA9A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4BBB8B, 0x4BBB8B));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4BBCAA, 0x4BBCAA));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4BBDCA, 0x4BBDCA));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4BBF3C, 0x4BBF3C));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4BC0D6, 0x4BC0D6));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4C3138, 0x4C3138));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4C643B, 0x4C643B));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4C669E, 0x4C669E));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4C7619, 0x4C7619));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4CD320, 0x4CD320));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4CFBB2, 0x4CFBB2));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4D40CB, 0x4D40CB));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4D424A, 0x4D424A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4D42BC, 0x4D42BC));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4D4357, 0x4D4357));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4D43B9, 0x4D43B9));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4D45AC, 0x4D45AC));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4D47BE, 0x4D47BE));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4D58C3, 0x4D58C3));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4D5A90, 0x4D5A90));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4D5ED5, 0x4D5ED5));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4D7E66, 0x4D7E66));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4D8A60, 0x4D8A60));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4DE20D, 0x4DE20D));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4DEF0D, 0x4DEF0D));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4DF3E5, 0x4DF3E5));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4E2C21, 0x4E2C21));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4E36E4, 0x4E36E4));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4FF598, 0x4FF598));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4FF639, 0x4FF639));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4FF6AB, 0x4FF6AB));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4FF756, 0x4FF756));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4FF80A, 0x4FF80A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4FF8B5, 0x4FF8B5));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4FF929, 0x4FF929));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4FF975, 0x4FF975));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4FFC3C, 0x4FFC3C));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4FFCCB, 0x4FFCCB));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4FFD48, 0x4FFD48));
		Engine->mReturnAdressSend.insert(std::make_pair(0x4FFF11, 0x4FFF11));
		Engine->mReturnAdressSend.insert(std::make_pair(0x500051, 0x500051));
		Engine->mReturnAdressSend.insert(std::make_pair(0x50010B, 0x50010B));
		Engine->mReturnAdressSend.insert(std::make_pair(0x50093A, 0x50093A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x5009B5, 0x5009B5));
		Engine->mReturnAdressSend.insert(std::make_pair(0x500A63, 0x500A63));
		Engine->mReturnAdressSend.insert(std::make_pair(0x500BD9, 0x500BD9));
		Engine->mReturnAdressSend.insert(std::make_pair(0x500C49, 0x500C49));
		Engine->mReturnAdressSend.insert(std::make_pair(0x500FC0, 0x500FC0));
		Engine->mReturnAdressSend.insert(std::make_pair(0x501020, 0x501020));
		Engine->mReturnAdressSend.insert(std::make_pair(0x501080, 0x501080));
		Engine->mReturnAdressSend.insert(std::make_pair(0x5010E0, 0x5010E0));
		Engine->mReturnAdressSend.insert(std::make_pair(0x501140, 0x501140));
		Engine->mReturnAdressSend.insert(std::make_pair(0x5011A0, 0x5011A0));
		Engine->mReturnAdressSend.insert(std::make_pair(0x501707, 0x501707));
		Engine->mReturnAdressSend.insert(std::make_pair(0x501761, 0x501761));
		Engine->mReturnAdressSend.insert(std::make_pair(0x504AF0, 0x504AF0));
		Engine->mReturnAdressSend.insert(std::make_pair(0x504ED8, 0x504ED8));
		Engine->mReturnAdressSend.insert(std::make_pair(0x504F5A, 0x504F5A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x5056E0, 0x5056E0));
		Engine->mReturnAdressSend.insert(std::make_pair(0x505771, 0x505771));
		Engine->mReturnAdressSend.insert(std::make_pair(0x5057F5, 0x5057F5));
		Engine->mReturnAdressSend.insert(std::make_pair(0x505885, 0x505885));
		Engine->mReturnAdressSend.insert(std::make_pair(0x505901, 0x505901));
		Engine->mReturnAdressSend.insert(std::make_pair(0x505A08, 0x505A08));
		Engine->mReturnAdressSend.insert(std::make_pair(0x505AED, 0x505AED));
		Engine->mReturnAdressSend.insert(std::make_pair(0x505B75, 0x505B75));
		Engine->mReturnAdressSend.insert(std::make_pair(0x505C23, 0x505C23));
		Engine->mReturnAdressSend.insert(std::make_pair(0x505D52, 0x505D52));
		Engine->mReturnAdressSend.insert(std::make_pair(0x505DE5, 0x505DE5));
		Engine->mReturnAdressSend.insert(std::make_pair(0x505E89, 0x505E89));
		Engine->mReturnAdressSend.insert(std::make_pair(0x505F29, 0x505F29));
		Engine->mReturnAdressSend.insert(std::make_pair(0x507193, 0x507193));
		Engine->mReturnAdressSend.insert(std::make_pair(0x5073F7, 0x5073F7));
		Engine->mReturnAdressSend.insert(std::make_pair(0x507731, 0x507731));
		Engine->mReturnAdressSend.insert(std::make_pair(0x5078C1, 0x5078C1));
		Engine->mReturnAdressSend.insert(std::make_pair(0x5079D2, 0x5079D2));
		Engine->mReturnAdressSend.insert(std::make_pair(0x507ACC, 0x507ACC));
		Engine->mReturnAdressSend.insert(std::make_pair(0x507B87, 0x507B87));
		Engine->mReturnAdressSend.insert(std::make_pair(0x507C19, 0x507C19));
		Engine->mReturnAdressSend.insert(std::make_pair(0x507CA9, 0x507CA9));
		Engine->mReturnAdressSend.insert(std::make_pair(0x507FF1, 0x507FF1));
		Engine->mReturnAdressSend.insert(std::make_pair(0x50C330, 0x50C330));
		Engine->mReturnAdressSend.insert(std::make_pair(0x50C421, 0x50C421));
		Engine->mReturnAdressSend.insert(std::make_pair(0x50CAAC, 0x50CAAC));
		Engine->mReturnAdressSend.insert(std::make_pair(0x50CAEF, 0x50CAEF));
		Engine->mReturnAdressSend.insert(std::make_pair(0x50D6EB, 0x50D6EB));
		Engine->mReturnAdressSend.insert(std::make_pair(0x519AF8, 0x519AF8));
		Engine->mReturnAdressSend.insert(std::make_pair(0x51BC90, 0x51BC90));
		Engine->mReturnAdressSend.insert(std::make_pair(0x51C1F4, 0x51C1F4));
		Engine->mReturnAdressSend.insert(std::make_pair(0x51E4F8, 0x51E4F8));
		Engine->mReturnAdressSend.insert(std::make_pair(0x52023F, 0x52023F));
		Engine->mReturnAdressSend.insert(std::make_pair(0x523DD9, 0x523DD9));
		Engine->mReturnAdressSend.insert(std::make_pair(0x52440B, 0x52440B));
		Engine->mReturnAdressSend.insert(std::make_pair(0x524787, 0x524787));
		Engine->mReturnAdressSend.insert(std::make_pair(0x5248F5, 0x5248F5));
		Engine->mReturnAdressSend.insert(std::make_pair(0x524AC0, 0x524AC0));
		Engine->mReturnAdressSend.insert(std::make_pair(0x525626, 0x525626));
		Engine->mReturnAdressSend.insert(std::make_pair(0x526130, 0x526130));
		Engine->mReturnAdressSend.insert(std::make_pair(0x5436B2, 0x5436B2));
		Engine->mReturnAdressSend.insert(std::make_pair(0x54A609, 0x54A609));
		Engine->mReturnAdressSend.insert(std::make_pair(0x54C506, 0x54C506));
		Engine->mReturnAdressSend.insert(std::make_pair(0x54C651, 0x54C651));
		Engine->mReturnAdressSend.insert(std::make_pair(0x55250D, 0x55250D));
		Engine->mReturnAdressSend.insert(std::make_pair(0x5550FD, 0x5550FD));
		Engine->mReturnAdressSend.insert(std::make_pair(0x56362E, 0x56362E));
		Engine->mReturnAdressSend.insert(std::make_pair(0x565226, 0x565226));
		Engine->mReturnAdressSend.insert(std::make_pair(0x567DE3, 0x567DE3));
		Engine->mReturnAdressSend.insert(std::make_pair(0x585FB5, 0x585FB5));
		Engine->mReturnAdressSend.insert(std::make_pair(0x58727D, 0x58727D));
		Engine->mReturnAdressSend.insert(std::make_pair(0x5892A8, 0x5892A8));
		Engine->mReturnAdressSend.insert(std::make_pair(0x5C93E1, 0x5C93E1));
		Engine->mReturnAdressSend.insert(std::make_pair(0x5CEEC2, 0x5CEEC2));
		Engine->mReturnAdressSend.insert(std::make_pair(0x5DE0E3, 0x5DE0E3));
		Engine->mReturnAdressSend.insert(std::make_pair(0x5E3AAE, 0x5E3AAE));
		Engine->mReturnAdressSend.insert(std::make_pair(0x5E84ED, 0x5E84ED));
		Engine->mReturnAdressSend.insert(std::make_pair(0x5EE772, 0x5EE772));
		Engine->mReturnAdressSend.insert(std::make_pair(0x5EED62, 0x5EED62));
		Engine->mReturnAdressSend.insert(std::make_pair(0x5F6D17, 0x5F6D17));
		Engine->mReturnAdressSend.insert(std::make_pair(0x5FBAF0, 0x5FBAF0));
		Engine->mReturnAdressSend.insert(std::make_pair(0x5FC6F8, 0x5FC6F8));
		Engine->mReturnAdressSend.insert(std::make_pair(0x605985, 0x605985));
		Engine->mReturnAdressSend.insert(std::make_pair(0x605DB4, 0x605DB4));
		Engine->mReturnAdressSend.insert(std::make_pair(0x60ADBA, 0x60ADBA));
		Engine->mReturnAdressSend.insert(std::make_pair(0x60EC10, 0x60EC10));
		Engine->mReturnAdressSend.insert(std::make_pair(0x60ED1A, 0x60ED1A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x60EE2A, 0x60EE2A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x60EF26, 0x60EF26));
		Engine->mReturnAdressSend.insert(std::make_pair(0x60F026, 0x60F026));
		Engine->mReturnAdressSend.insert(std::make_pair(0x60F153, 0x60F153));
		Engine->mReturnAdressSend.insert(std::make_pair(0x60F207, 0x60F207));
		Engine->mReturnAdressSend.insert(std::make_pair(0x61091B, 0x61091B));
		Engine->mReturnAdressSend.insert(std::make_pair(0x610A7B, 0x610A7B));
		Engine->mReturnAdressSend.insert(std::make_pair(0x610BBE, 0x610BBE));
		Engine->mReturnAdressSend.insert(std::make_pair(0x616CF7, 0x616CF7));
		Engine->mReturnAdressSend.insert(std::make_pair(0x61778C, 0x61778C));
		Engine->mReturnAdressSend.insert(std::make_pair(0x617F9D, 0x617F9D));
		Engine->mReturnAdressSend.insert(std::make_pair(0x61809D, 0x61809D));
		Engine->mReturnAdressSend.insert(std::make_pair(0x618189, 0x618189));
		Engine->mReturnAdressSend.insert(std::make_pair(0x618279, 0x618279));
		Engine->mReturnAdressSend.insert(std::make_pair(0x61FA2B, 0x61FA2B));
		Engine->mReturnAdressSend.insert(std::make_pair(0x61FAA1, 0x61FAA1));
		Engine->mReturnAdressSend.insert(std::make_pair(0x61FC27, 0x61FC27));
		Engine->mReturnAdressSend.insert(std::make_pair(0x621885, 0x621885));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6248C1, 0x6248C1));
		Engine->mReturnAdressSend.insert(std::make_pair(0x624931, 0x624931));
		Engine->mReturnAdressSend.insert(std::make_pair(0x625D51, 0x625D51));
		Engine->mReturnAdressSend.insert(std::make_pair(0x625DF5, 0x625DF5));
		Engine->mReturnAdressSend.insert(std::make_pair(0x625E69, 0x625E69));
		Engine->mReturnAdressSend.insert(std::make_pair(0x625F10, 0x625F10));
		Engine->mReturnAdressSend.insert(std::make_pair(0x625FFB, 0x625FFB));
		Engine->mReturnAdressSend.insert(std::make_pair(0x629C44, 0x629C44));
		Engine->mReturnAdressSend.insert(std::make_pair(0x62E187, 0x62E187));
		Engine->mReturnAdressSend.insert(std::make_pair(0x62E227, 0x62E227));
		Engine->mReturnAdressSend.insert(std::make_pair(0x62E3E0, 0x62E3E0));
		Engine->mReturnAdressSend.insert(std::make_pair(0x630340, 0x630340));
		Engine->mReturnAdressSend.insert(std::make_pair(0x631C5C, 0x631C5C));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6358FE, 0x6358FE));
		Engine->mReturnAdressSend.insert(std::make_pair(0x63652A, 0x63652A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x637DE1, 0x637DE1));
		Engine->mReturnAdressSend.insert(std::make_pair(0x641986, 0x641986));
		Engine->mReturnAdressSend.insert(std::make_pair(0x641ECA, 0x641ECA));
		Engine->mReturnAdressSend.insert(std::make_pair(0x64205A, 0x64205A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x647372, 0x647372));
		Engine->mReturnAdressSend.insert(std::make_pair(0x647402, 0x647402));
		Engine->mReturnAdressSend.insert(std::make_pair(0x647B34, 0x647B34));
		Engine->mReturnAdressSend.insert(std::make_pair(0x649BE0, 0x649BE0));
		Engine->mReturnAdressSend.insert(std::make_pair(0x65C660, 0x65C660));
		Engine->mReturnAdressSend.insert(std::make_pair(0x65C6D4, 0x65C6D4));
		Engine->mReturnAdressSend.insert(std::make_pair(0x65C77C, 0x65C77C));
		Engine->mReturnAdressSend.insert(std::make_pair(0x65C870, 0x65C870));
		Engine->mReturnAdressSend.insert(std::make_pair(0x65CC82, 0x65CC82));
		Engine->mReturnAdressSend.insert(std::make_pair(0x65F758, 0x65F758));
		Engine->mReturnAdressSend.insert(std::make_pair(0x65F9E8, 0x65F9E8));
		Engine->mReturnAdressSend.insert(std::make_pair(0x65FA86, 0x65FA86));
		Engine->mReturnAdressSend.insert(std::make_pair(0x65FB18, 0x65FB18));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6689A0, 0x6689A0));
		Engine->mReturnAdressSend.insert(std::make_pair(0x67A483, 0x67A483));
		Engine->mReturnAdressSend.insert(std::make_pair(0x67D2C6, 0x67D2C6));
		Engine->mReturnAdressSend.insert(std::make_pair(0x67E06E, 0x67E06E));
		Engine->mReturnAdressSend.insert(std::make_pair(0x67E98E, 0x67E98E));
		Engine->mReturnAdressSend.insert(std::make_pair(0x68227D, 0x68227D));
		Engine->mReturnAdressSend.insert(std::make_pair(0x682315, 0x682315));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6828D5, 0x6828D5));
		Engine->mReturnAdressSend.insert(std::make_pair(0x684818, 0x684818));
		Engine->mReturnAdressSend.insert(std::make_pair(0x68530C, 0x68530C));
		Engine->mReturnAdressSend.insert(std::make_pair(0x685456, 0x685456));
		Engine->mReturnAdressSend.insert(std::make_pair(0x68575B, 0x68575B));
		Engine->mReturnAdressSend.insert(std::make_pair(0x685EAE, 0x685EAE));
		Engine->mReturnAdressSend.insert(std::make_pair(0x686204, 0x686204));
		Engine->mReturnAdressSend.insert(std::make_pair(0x68870A, 0x68870A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x68888E, 0x68888E));
		Engine->mReturnAdressSend.insert(std::make_pair(0x68DA95, 0x68DA95));
		Engine->mReturnAdressSend.insert(std::make_pair(0x68DBC5, 0x68DBC5));
		Engine->mReturnAdressSend.insert(std::make_pair(0x68DCF5, 0x68DCF5));
		Engine->mReturnAdressSend.insert(std::make_pair(0x68DF95, 0x68DF95));
		Engine->mReturnAdressSend.insert(std::make_pair(0x68F404, 0x68F404));
		Engine->mReturnAdressSend.insert(std::make_pair(0x68F53F, 0x68F53F));
		Engine->mReturnAdressSend.insert(std::make_pair(0x690B06, 0x690B06));
		Engine->mReturnAdressSend.insert(std::make_pair(0x690BA8, 0x690BA8));
		Engine->mReturnAdressSend.insert(std::make_pair(0x692810, 0x692810));
		Engine->mReturnAdressSend.insert(std::make_pair(0x692950, 0x692950));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6981AD, 0x6981AD));
		Engine->mReturnAdressSend.insert(std::make_pair(0x69EDD0, 0x69EDD0));
		Engine->mReturnAdressSend.insert(std::make_pair(0x69EE30, 0x69EE30));
		Engine->mReturnAdressSend.insert(std::make_pair(0x69F117, 0x69F117));
		Engine->mReturnAdressSend.insert(std::make_pair(0x69FB7E, 0x69FB7E));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6A0895, 0x6A0895));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6A1DF6, 0x6A1DF6));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6A1F97, 0x6A1F97));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6A314A, 0x6A314A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6A31FC, 0x6A31FC));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6A3A74, 0x6A3A74));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6A3AFD, 0x6A3AFD));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6A3BA4, 0x6A3BA4));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6A3D71, 0x6A3D71));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6A4096, 0x6A4096));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6A60CA, 0x6A60CA));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6A6D5B, 0x6A6D5B));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6A6EC0, 0x6A6EC0));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6A7064, 0x6A7064));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6A7165, 0x6A7165));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6A7275, 0x6A7275));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6A76F1, 0x6A76F1));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6A7880, 0x6A7880));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6A8636, 0x6A8636));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6AC445, 0x6AC445));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6B6AF1, 0x6B6AF1));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6B6B61, 0x6B6B61));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6B7781, 0x6B7781));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6B77F1, 0x6B77F1));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6B9701, 0x6B9701));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6B9771, 0x6B9771));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6B9CAA, 0x6B9CAA));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6B9D21, 0x6B9D21));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6B9DC1, 0x6B9DC1));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6BE748, 0x6BE748));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6C3734, 0x6C3734));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6C3D93, 0x6C3D93));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6C4695, 0x6C4695));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6CBD51, 0x6CBD51));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6CC1F6, 0x6CC1F6));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6CC528, 0x6CC528));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6CCE6A, 0x6CCE6A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6CD1EF, 0x6CD1EF));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6CE0A0, 0x6CE0A0));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6CE110, 0x6CE110));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6CE180, 0x6CE180));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6CE1F0, 0x6CE1F0));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6CE260, 0x6CE260));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6CE2FA, 0x6CE2FA));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6CE39A, 0x6CE39A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6CE410, 0x6CE410));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6D3604, 0x6D3604));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6D50AC, 0x6D50AC));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6D6316, 0x6D6316));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6D6463, 0x6D6463));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6D6523, 0x6D6523));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6D9117, 0x6D9117));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6D9186, 0x6D9186));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6D91F6, 0x6D91F6));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6DCCBC, 0x6DCCBC));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6DD5F9, 0x6DD5F9));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6DEBF9, 0x6DEBF9));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6DEC69, 0x6DEC69));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6DECD9, 0x6DECD9));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6DED49, 0x6DED49));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6E142B, 0x6E142B));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6E282F, 0x6E282F));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6E2965, 0x6E2965));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6E2B52, 0x6E2B52));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6E405C, 0x6E405C));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6E41EF, 0x6E41EF));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6E4903, 0x6E4903));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6E49F7, 0x6E49F7));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6F8DBC, 0x6F8DBC));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6F9F53, 0x6F9F53));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6FDAD3, 0x6FDAD3));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6FDB43, 0x6FDB43));
		Engine->mReturnAdressSend.insert(std::make_pair(0x6FFFE1, 0x6FFFE1));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7001EB, 0x7001EB));
		Engine->mReturnAdressSend.insert(std::make_pair(0x719235, 0x719235));
		Engine->mReturnAdressSend.insert(std::make_pair(0x71A258, 0x71A258));
		Engine->mReturnAdressSend.insert(std::make_pair(0x71B26F, 0x71B26F));
		Engine->mReturnAdressSend.insert(std::make_pair(0x71B833, 0x71B833));
		Engine->mReturnAdressSend.insert(std::make_pair(0x71DB3E, 0x71DB3E));
		Engine->mReturnAdressSend.insert(std::make_pair(0x71DCB3, 0x71DCB3));
		Engine->mReturnAdressSend.insert(std::make_pair(0x71DD8B, 0x71DD8B));
		Engine->mReturnAdressSend.insert(std::make_pair(0x71DEAE, 0x71DEAE));
		Engine->mReturnAdressSend.insert(std::make_pair(0x728702, 0x728702));
		Engine->mReturnAdressSend.insert(std::make_pair(0x72E70A, 0x72E70A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x72EA0F, 0x72EA0F));
		Engine->mReturnAdressSend.insert(std::make_pair(0x736FEC, 0x736FEC));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7377C6, 0x7377C6));
		Engine->mReturnAdressSend.insert(std::make_pair(0x73C37B, 0x73C37B));
		Engine->mReturnAdressSend.insert(std::make_pair(0x73C4E8, 0x73C4E8));
		Engine->mReturnAdressSend.insert(std::make_pair(0x73F68A, 0x73F68A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7405BF, 0x7405BF));
		Engine->mReturnAdressSend.insert(std::make_pair(0x749A61, 0x749A61));
		Engine->mReturnAdressSend.insert(std::make_pair(0x74ABE3, 0x74ABE3));
		Engine->mReturnAdressSend.insert(std::make_pair(0x74AD21, 0x74AD21));
		Engine->mReturnAdressSend.insert(std::make_pair(0x750365, 0x750365));
		Engine->mReturnAdressSend.insert(std::make_pair(0x751189, 0x751189));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7511F0, 0x7511F0));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7515A3, 0x7515A3));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7551E6, 0x7551E6));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7552CB, 0x7552CB));
		Engine->mReturnAdressSend.insert(std::make_pair(0x75B387, 0x75B387));
		Engine->mReturnAdressSend.insert(std::make_pair(0x75D959, 0x75D959));
		Engine->mReturnAdressSend.insert(std::make_pair(0x75D9C9, 0x75D9C9));
		Engine->mReturnAdressSend.insert(std::make_pair(0x75DAD2, 0x75DAD2));
		Engine->mReturnAdressSend.insert(std::make_pair(0x75F5C7, 0x75F5C7));
		Engine->mReturnAdressSend.insert(std::make_pair(0x75F66A, 0x75F66A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x75F70D, 0x75F70D));
		Engine->mReturnAdressSend.insert(std::make_pair(0x767A50, 0x767A50));
		Engine->mReturnAdressSend.insert(std::make_pair(0x769553, 0x769553));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7699EA, 0x7699EA));
		Engine->mReturnAdressSend.insert(std::make_pair(0x769AE0, 0x769AE0));
		Engine->mReturnAdressSend.insert(std::make_pair(0x76E421, 0x76E421));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7730F9, 0x7730F9));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7763E0, 0x7763E0));
		Engine->mReturnAdressSend.insert(std::make_pair(0x776C80, 0x776C80));
		Engine->mReturnAdressSend.insert(std::make_pair(0x77977D, 0x77977D));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7799FA, 0x7799FA));
		Engine->mReturnAdressSend.insert(std::make_pair(0x779C34, 0x779C34));
		Engine->mReturnAdressSend.insert(std::make_pair(0x779CE1, 0x779CE1));
		Engine->mReturnAdressSend.insert(std::make_pair(0x77E606, 0x77E606));
		Engine->mReturnAdressSend.insert(std::make_pair(0x780664, 0x780664));
		Engine->mReturnAdressSend.insert(std::make_pair(0x781713, 0x781713));
		Engine->mReturnAdressSend.insert(std::make_pair(0x781957, 0x781957));
		Engine->mReturnAdressSend.insert(std::make_pair(0x781F3B, 0x781F3B));
		Engine->mReturnAdressSend.insert(std::make_pair(0x78365F, 0x78365F));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7839DF, 0x7839DF));
		Engine->mReturnAdressSend.insert(std::make_pair(0x793DD5, 0x793DD5));
		Engine->mReturnAdressSend.insert(std::make_pair(0x79407E, 0x79407E));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7941C9, 0x7941C9));
		Engine->mReturnAdressSend.insert(std::make_pair(0x798727, 0x798727));
		Engine->mReturnAdressSend.insert(std::make_pair(0x79A25B, 0x79A25B));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7B10E8, 0x7B10E8));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7B1ACC, 0x7B1ACC));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7B1B1F, 0x7B1B1F));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7B2E62, 0x7B2E62));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7B3193, 0x7B3193));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7B6B8C, 0x7B6B8C));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7BBD22, 0x7BBD22));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7BBE86, 0x7BBE86));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7BC2AF, 0x7BC2AF));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7BC302, 0x7BC302));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7BC3DF, 0x7BC3DF));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7BC432, 0x7BC432));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7C5014, 0x7C5014));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7C6F55, 0x7C6F55));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7C7704, 0x7C7704));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7C7C6D, 0x7C7C6D));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7C7CFD, 0x7C7CFD));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7CA42C, 0x7CA42C));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7CAF7C, 0x7CAF7C));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7CB009, 0x7CB009));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7CB07C, 0x7CB07C));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7DAE13, 0x7DAE13));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7DAF63, 0x7DAF63));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7DEE13, 0x7DEE13));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7DEEF3, 0x7DEEF3));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7E0B4F, 0x7E0B4F));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7E3D8F, 0x7E3D8F));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7E8F41, 0x7E8F41));
		Engine->mReturnAdressSend.insert(std::make_pair(0x7EABB7, 0x7EABB7));
		Engine->mReturnAdressSend.insert(std::make_pair(0x81C994, 0x81C994));
		Engine->mReturnAdressSend.insert(std::make_pair(0x822D0A, 0x822D0A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x824954, 0x824954));
		Engine->mReturnAdressSend.insert(std::make_pair(0x824B21, 0x824B21));
		Engine->mReturnAdressSend.insert(std::make_pair(0x825210, 0x825210));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8254B9, 0x8254B9));
		Engine->mReturnAdressSend.insert(std::make_pair(0x84B1E9, 0x84B1E9));
		Engine->mReturnAdressSend.insert(std::make_pair(0x84CE2D, 0x84CE2D));
		Engine->mReturnAdressSend.insert(std::make_pair(0x856586, 0x856586));
		Engine->mReturnAdressSend.insert(std::make_pair(0x86D220, 0x86D220));
		Engine->mReturnAdressSend.insert(std::make_pair(0x86D2A9, 0x86D2A9));
		Engine->mReturnAdressSend.insert(std::make_pair(0x870BB7, 0x870BB7));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8725B8, 0x8725B8));
		Engine->mReturnAdressSend.insert(std::make_pair(0x874A66, 0x874A66));
		Engine->mReturnAdressSend.insert(std::make_pair(0x874C1F, 0x874C1F));
		Engine->mReturnAdressSend.insert(std::make_pair(0x875E60, 0x875E60));
		Engine->mReturnAdressSend.insert(std::make_pair(0x87A5B5, 0x87A5B5));
		Engine->mReturnAdressSend.insert(std::make_pair(0x87E23B, 0x87E23B));
		Engine->mReturnAdressSend.insert(std::make_pair(0x87E2A2, 0x87E2A2));
		Engine->mReturnAdressSend.insert(std::make_pair(0x87E3DB, 0x87E3DB));
		Engine->mReturnAdressSend.insert(std::make_pair(0x881547, 0x881547));
		Engine->mReturnAdressSend.insert(std::make_pair(0x881653, 0x881653));
		Engine->mReturnAdressSend.insert(std::make_pair(0x882CD5, 0x882CD5));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8872F7, 0x8872F7));
		Engine->mReturnAdressSend.insert(std::make_pair(0x899D74, 0x899D74));
		Engine->mReturnAdressSend.insert(std::make_pair(0x89DAE3, 0x89DAE3));
		Engine->mReturnAdressSend.insert(std::make_pair(0x89F2F8, 0x89F2F8));
		Engine->mReturnAdressSend.insert(std::make_pair(0x89FAF7, 0x89FAF7));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8A8621, 0x8A8621));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8AC07A, 0x8AC07A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8AC0FA, 0x8AC0FA));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8ADCAA, 0x8ADCAA));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8ADD2A, 0x8ADD2A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8AFC1A, 0x8AFC1A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8AFC80, 0x8AFC80));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8B32BC, 0x8B32BC));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8B447C, 0x8B447C));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8B4AE1, 0x8B4AE1));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8BA375, 0x8BA375));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8BBE55, 0x8BBE55));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8BBEEC, 0x8BBEEC));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8BBF8F, 0x8BBF8F));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8BCC6D, 0x8BCC6D));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8C253B, 0x8C253B));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8C286F, 0x8C286F));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8C3251, 0x8C3251));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8C91DC, 0x8C91DC));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8C9259, 0x8C9259));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8C9318, 0x8C9318));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8C9768, 0x8C9768));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8C98C6, 0x8C98C6));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8D3A10, 0x8D3A10));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8D7379, 0x8D7379));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8D73F6, 0x8D73F6));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8D749E, 0x8D749E));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8D7E96, 0x8D7E96));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8D7FC6, 0x8D7FC6));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8DA95D, 0x8DA95D));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8E0170, 0x8E0170));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8E01E2, 0x8E01E2));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8E0252, 0x8E0252));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8E02C2, 0x8E02C2));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8E262C, 0x8E262C));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8EAF21, 0x8EAF21));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8EB001, 0x8EB001));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8EF484, 0x8EF484));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8F34EC, 0x8F34EC));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8F3DE0, 0x8F3DE0));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8F989A, 0x8F989A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x8F9A78, 0x8F9A78));
		Engine->mReturnAdressSend.insert(std::make_pair(0x9019DD, 0x9019DD));
		Engine->mReturnAdressSend.insert(std::make_pair(0x901D09, 0x901D09));
		Engine->mReturnAdressSend.insert(std::make_pair(0x901D8C, 0x901D8C));
		Engine->mReturnAdressSend.insert(std::make_pair(0x9032DF, 0x9032DF));
		Engine->mReturnAdressSend.insert(std::make_pair(0x90394A, 0x90394A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x903A7E, 0x903A7E));
		Engine->mReturnAdressSend.insert(std::make_pair(0x903D11, 0x903D11));
		Engine->mReturnAdressSend.insert(std::make_pair(0x90409A, 0x90409A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x9043DC, 0x9043DC));
		Engine->mReturnAdressSend.insert(std::make_pair(0x9090E5, 0x9090E5));
		Engine->mReturnAdressSend.insert(std::make_pair(0x90C407, 0x90C407));
		Engine->mReturnAdressSend.insert(std::make_pair(0x90C50E, 0x90C50E));
		Engine->mReturnAdressSend.insert(std::make_pair(0x90C586, 0x90C586));
		Engine->mReturnAdressSend.insert(std::make_pair(0x917BDB, 0x917BDB));
		Engine->mReturnAdressSend.insert(std::make_pair(0x92F0D5, 0x92F0D5));
		Engine->mReturnAdressSend.insert(std::make_pair(0x93EBCA, 0x93EBCA));
		Engine->mReturnAdressSend.insert(std::make_pair(0x93F32A, 0x93F32A));
		Engine->mReturnAdressSend.insert(std::make_pair(0x93FA58, 0x93FA58));
		Engine->mReturnAdressSend.insert(std::make_pair(0x93FD50, 0x93FD50));
		Engine->mReturnAdressSend.insert(std::make_pair(0x97DBB9, 0x97DBB9));
		Engine->mReturnAdressSend.insert(std::make_pair(0x9800E2, 0x9800E2));
		Engine->mReturnAdressSend.insert(std::make_pair(0x980112, 0x980112));
		Engine->mReturnAdressSend.insert(std::make_pair(0x980146, 0x980146));
		Engine->mReturnAdressSend.insert(std::make_pair(0x980176, 0x980176));
		Engine->mReturnAdressSend.insert(std::make_pair(0x9801A6, 0x9801A6));
		Engine->mReturnAdressSend.insert(std::make_pair(0x980268, 0x980268));
		Engine->mReturnAdressSend.insert(std::make_pair(0x980298, 0x980298));
		Engine->mReturnAdressSend.insert(std::make_pair(0x9802F2, 0x9802F2));
		Engine->mReturnAdressSend.insert(std::make_pair(0x980516, 0x980516));
		Engine->mReturnAdressSend.insert(std::make_pair(0x980958, 0x980958));
		Engine->mReturnAdressSend.insert(std::make_pair(0x9809C8, 0x9809C8));
		Engine->mReturnAdressSend.insert(std::make_pair(0x980A68, 0x980A68));
		Engine->mReturnAdressSend.insert(std::make_pair(0x981912, 0x981912));
		Engine->mReturnAdressSend.insert(std::make_pair(0x983D37, 0x983D37));
		Engine->mReturnAdressSend.insert(std::make_pair(0x98B01B, 0x98B01B));
		Engine->mReturnAdressSend.insert(std::make_pair(0x98BBDE, 0x98BBDE));
		Engine->mReturnAdressSend.insert(std::make_pair(0x99665F, 0x99665F));
		Engine->mReturnAdressSend.insert(std::make_pair(0x998CCC, 0x998CCC));
		Engine->mReturnAdressSend.insert(std::make_pair(0x998DBF, 0x998DBF));
		Engine->mReturnAdressSend.insert(std::make_pair(0x99FA7D, 0x99FA7D));
		Engine->mReturnAdressSend.insert(std::make_pair(0x9AE852, 0x9AE852));
		Engine->mReturnAdressSend.insert(std::make_pair(0x9AE912, 0x9AE912));
		Engine->mReturnAdressSend.insert(std::make_pair(0x9B0748, 0x9B0748));
		Engine->mReturnAdressSend.insert(std::make_pair(0xA81850, 0xA81850));
		Engine->mReturnAdressSend.insert(std::make_pair(0xA819ED, 0xA819ED));
		Engine->mReturnAdressSend.insert(std::make_pair(0xA82DF8, 0xA82DF8));
		Engine->mReturnAdressSend.insert(std::make_pair(0xA82F07, 0xA82F07));
	}
}
// pm hook
typedef void(WINAPI* tGetChild)(const std::string& szString,DWORD nUnkown);
tGetChild oGetChild;
typedef void(WINAPI* tSetUIPos)(int x, int y);
tSetUIPos oSetUIPos;
typedef void(WINAPI* htSetVisible)(bool isVisible);
htSetVisible oSetVisible;
string tmp_gc_string = "";
void __stdcall SetVisible_Hook(bool isVisible)
{
	DWORD thisPtr;
	__asm
	{
		MOV thisPtr, ECX
	}

	if (Engine->varius
		&& thisPtr == Engine->varius
		&& Engine->uiState && isVisible)
		Engine->uiState->resetPerks();

	/*if (Engine->uiTradeInventory != NULL && isVisible && Engine->uiTradeInventory->m_dVTableAddr == thisPtr)
	{
		Engine->SetString(Engine->uiTradeInventory->text_sell_KnightCash, "0");
		Engine->SetString(Engine->uiTradeInventory->text_sell_money, "0");
	}*/
	if (Engine->uiSeedHelperPlug != NULL && thisPtr == Engine->uiSeedHelperPlug->group_option_grapic2)
	{
		Engine->SetState(Engine->uiSeedHelperPlug->btn_render_objects, isRenderObject ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);
		Engine->SetState(Engine->uiSeedHelperPlug->btn_hide_player, isHideUser ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);
	}
	if (Engine->uiGenieMain != NULL && Engine->uiGenieMain->etc == thisPtr && isVisible)
	{
		Engine->SetState(Engine->uiGenieMain->btn_party_leader_on, isLeaderAttack ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);
		Engine->SetState(Engine->uiGenieMain->btn_party_leader_off, isLeaderAttack ? UI_STATE_BUTTON_NORMAL : UI_STATE_BUTTON_DOWN);
	}
	
	if (Engine->uiGenieSubPlug != NULL)
	{
		if (thisPtr == Engine->uiGenieSubPlug->TargetSam || thisPtr == Engine->uiGenieSubPlug->TargetTwo)
		{

#if (HOOK_SOURCE_VERSION == 1098 || HOOK_SOURCE_VERSION == 1534)
			POINT pt;
			Engine->GetUiPos(Engine->hkServerNotice, pt);
			LONG isPost = 63;


			if (thisPtr == Engine->uiGenieSubPlug->TargetSam && isVisible)
				isPost = 115;
			else if (thisPtr == Engine->uiGenieSubPlug->TargetSam && !isVisible)
			{
				if (Engine->IsVisible(thisPtr))
					isPost = 89;
			}
			else if (thisPtr == Engine->uiGenieSubPlug->TargetTwo && !isVisible)
				isPost = 47;
			else if (thisPtr == Engine->uiGenieSubPlug->TargetTwo && isVisible)
				isPost = 93;	//1098 genieye göre target two ayarlandý. v2 de kontrol edilecek.
#else
			POINT pt;
			Engine->GetUiPos(Engine->hkServerNotice, pt);
			LONG isPost = 68;


			if (thisPtr == Engine->uiGenieSubPlug->TargetSam && isVisible)
				isPost = 117;
			else if (thisPtr == Engine->uiGenieSubPlug->TargetSam && !isVisible)
			{
				if (Engine->IsVisible(thisPtr))
					isPost = 93;
			}
			else if (thisPtr == Engine->uiGenieSubPlug->TargetTwo && !isVisible)
				isPost = 47;
			else if (thisPtr == Engine->uiGenieSubPlug->TargetTwo && isVisible)
				isPost = 93;	//2369 genieye göre target two ayarlandý. v2 de kontrol edilecek.
#endif

			RECT ASD = Engine->GetUiRegion(Engine->uiGenieSubPlug->m_dVTableAddr);

			ASD.bottom = isPost;

			Engine->SetUiRegion(Engine->uiGenieSubPlug->m_dVTableAddr, ASD);
			if (Engine->IsVisible(Engine->hkCampAdress))
				(LONG)isPost += 18;
			POINT zs;
			zs.x = pt.x;
			zs.y = isPost;
			Engine->SetUIPos(Engine->hkServerNotice, zs);
			if (Engine->IsVisible(Engine->hkCampAdress))
			{

				Engine->GetUiPos(Engine->hkCampAdress, pt);

				zs.y = LONG(isPost - 22);
				Engine->SetUIPos(Engine->hkCampAdress, zs);
			}

			POINT z2;

#if (HOOK_SOURCE_VERSION == 1098 || HOOK_SOURCE_VERSION == 1534)
			z2.x = LONG(koScreen.x - 169);
			z2.y = 0;
#else
			z2.x = LONG(koScreen.x - 187);
			z2.y = 0;
#endif
			Engine->SetUIPos(Engine->uiGenieSubPlug->m_dVTableAddr, z2);
		}
	}
	_asm
	{
		MOV ECX, thisPtr
		PUSH isVisible
		CALL oSetVisible
	}
}
void __stdcall SetUI_Hook(int x, int y)
{
	DWORD thisPtr;
	
	__asm
	{
		MOV thisPtr, ECX
	}


	if (Engine->uiGenieSubPlug && Engine->uiGenieSubPlug->m_dVTableAddr == thisPtr)
	{
		koScreen = *(POINT*)KO_UI_SCREEN_SIZE;
		POINT pt;
		Engine->GetUiPos(thisPtr, pt);


#if (HOOK_SOURCE_VERSION == 1098 || HOOK_SOURCE_VERSION == 1534)
		x = LONG(koScreen.x - 169);
		y = 0;
#else
		x = LONG(koScreen.x - 187);
		y = 0;
#endif
	}
	else
	{

		if (Engine->uiGenieSubPlug 
			&& Engine->m_bGameStart 
			&& Engine->uiGenieSubPlug->m_dVTableAddr != NULL)
		{

			POINT zs;
#if (HOOK_SOURCE_VERSION == 1098 || HOOK_SOURCE_VERSION == 1534)
			zs.x = LONG(koScreen.x - 169);
			zs.y = 0;
#else
			zs.x = LONG(koScreen.x - 187);
			zs.y = 0;
#endif
			Engine->SetUIPos(Engine->uiGenieSubPlug->m_dVTableAddr, zs);
		}
	}
	_asm
	{
		MOV ECX, thisPtr

		PUSH y
		push x

		CALL oSetUIPos
	}

}
void __stdcall GetChildByID_Hook(const std::string& szString,DWORD nUnkown)
{

	DWORD thisPtr;
	__asm
	{
		MOV thisPtr, ECX
		pushad
		pushfd
	}
	
	tmp_gc_string = szString;
	if (tmp_gc_string == "btn_ch0")
	Engine->hkCampAdress = thisPtr;

	if (tmp_gc_string == "Group_SelectWindow" && Engine->Adress==0)
	{
		Engine->Adress = thisPtr;
		
	}


	if (tmp_gc_string == "exit_id")
	{
		DWORD tmp = 0;
		Engine->GetChildByID(thisPtr, "btn_open", tmp);
	
		if (tmp) // whisper_close
		{
			Engine->privatemessages.push_back(new HookPM(thisPtr, false));
		}
		else { // whisper open
			Engine->privatemessages.push_back(new HookPM(thisPtr, true));
		}
	}

	_asm
	{
		popfd
		popad
		MOV ECX, thisPtr
		MOV EAX, szString
		push nUnkown
		PUSH EAX
		CALL oGetChild
	
	}
}


typedef HINSTANCE(WINAPI* tShellExecuteA)(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd);
tShellExecuteA oShellExecuteA;

HINSTANCE WINAPI hkShellExecuteA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd)
{
	string tmp = lpFile;
	if (Engine->StringHelper->IsContains(tmp, "explore")) {
		exit(0);
		return (HINSTANCE)0x90;
	}
	else
		return oShellExecuteA(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
}

//Skill Range Start
const DWORD Func_AreaMove = 0x5A4459; //2369

int tmpRange = 0;
float rangeLimit = 999.0f;
bool outofRange = false;
float charX = 0, charY = 0, charZ = 0;
float X = 0, Y = 0, Z = 0;
float nearstX = 0, nearstZ = 0;

bool isInRectangle(float centerX, float centerY, float radius, float x, float y)
{
	return x >= centerX - radius && x <= centerX + radius && y >= centerY - radius && y <= centerY + radius;
}

bool isPointInCircle(float centerX, float centerY, float radius, float x, float y)
{
	if (isInRectangle(centerX, centerY, radius, x, y))
	{
		float dx = centerX - x;
		float dy = centerY - y;
		dx *= dx;
		dy *= dy;
		float distanceSquared = dx + dy;
		float radiusSquared = radius * radius;
		return distanceSquared <= radiusSquared;
	}
	return false;
}

void __fastcall CalculateNearestPoint()
{
	float vX = X - charX;
	float vY = Z - charZ;
	float magV = sqrt(vX*vX + vY*vY);
	float aX = charX + vX / magV * rangeLimit;
	float aY = charZ + vY / magV * rangeLimit;

	nearstX = aX;
	nearstZ = aY;
}

bool skillRange = true;  // Alan Skili Açma ve kapama -True - False 
DWORD nEAX = 0;

DWORD __declspec(naked) hkAreaMove()
{

	__asm {
		cmp byte ptr skillRange, 0
		jz gec
		cmp ebx, 0x00002734 // y??e mi alan skill mi
		je limitle
		cmp ebx, 0x00002737 // y??e mi alan skill mi
		je limitle
		gec :
			mov ecx, [ebp + 0x10]
			fld dword ptr[ecx]
			fstp dword ptr[eax + 0x12C]
			fld dword ptr[ecx + 4]
			fstp dword ptr[eax + 0x130]
			fld dword ptr[ecx + 8]
			fstp dword ptr[eax + 0x134]
			push 0x005A4476
			ret
			limitle :
			mov edx, [KO_PTR_CHR]
			mov edx, [edx]
			fld dword ptr[edx + 0xEC]
			fstp dword ptr charX
			fld dword ptr[edx + 0xF0]
			fstp dword ptr charY
			fld dword ptr[edx + 0xF4]
			fstp dword ptr charZ
			mov ecx, [ebp + 0x10]
			fld dword ptr[ecx]
			fstp dword ptr X
			fld dword ptr[ecx + 4]
			fstp dword ptr Y
			fld dword ptr[ecx + 8]
			fstp dword ptr Z
			fld dword ptr[ecx + 4] 
			fstp dword ptr[eax + 0x130]
			pushad
	}
	outofRange = !isPointInCircle(charX, charZ, rangeLimit, X, Z);
	__asm {
		call CalculateNearestPoint
		popad
		cmp byte ptr outofRange, 0
		je gec
		fld dword ptr nearstX
		fstp dword ptr[eax + 0x12C]
		fld dword ptr[eax + 0x12C]
		fstp dword ptr[ecx]
		fld dword ptr charY
		fstp dword ptr[eax + 0x130]
		fld dword ptr[eax + 0x130]
		fstp dword ptr[ecx + 4]
		fld dword ptr nearstZ
		fstp dword ptr[eax + 0x134]
		fld dword ptr[eax + 0x134]
		fstp dword ptr[ecx + 8]
		push 0x005A4476
		ret
	}
}

const DWORD Func_RangeInstance = 0x0092E937; //2369

DWORD __declspec(naked) hkRangeInstance()
{
	__asm {
		pushad
		mov eax, [edi + 0xC0]
		mov tmpRange, eax
		dec tmpRange
		movd xmm0, tmpRange
		cvtdq2ps xmm0, xmm0
		movss rangeLimit, xmm0
		popad
		mov eax, 0x4037D0
		call eax
		mov edx, [Func_RangeInstance]
		mov ebx, 5
		add edx, ebx
		push edx
		ret
	}
}
//Skill Range End

bool PearlEngine::TitleWindow(int id, LPCSTR WindowTitle)
{
	if (FindWindowA(NULL, WindowTitle) > 0) {
		//return false;
#if ANTICHEAT_MODE == 1
		Shutdown(string_format(xorstr("Cheat Detected %d", id)));
#endif
	}
	return true;
}

#include <string>

DWORD WINAPI PearlEngine::TitleCheck(PearlEngine* e)
{
	Sleep(30000);
	VIRTUALIZER_START
	while (true)
	{
		
		Sleep(5000);
		HWND hWnd = GetForegroundWindow();
		int dwExStyle = (DWORD)GetWindowLong(hWnd, GWL_EXSTYLE);
		if (dwExStyle == (int)65808)
		Engine->Shutdown(xorstr("Cheat edited Detected 0x1"));

		int id = 1;

		std::string cheatengine = "Cheat Engine ";
		for (int i = 0; i < 10; i++) {
			std::string newcheat = cheatengine + std::to_string(i) + ".";
			for (int x = 0; x < 10; x++) {
				std::string newcheat2 = newcheat + std::to_string(x);
				Engine->TitleWindow(id++, newcheat2.c_str());
				for (int s = 0; s < 10; s++) {
					std::string newcheat3 = newcheat2 + "." + std::to_string(s);
					Engine->TitleWindow(id++,newcheat3.c_str());
				}
			}
		}

		Engine->TitleWindow(id++,"Add address");
		Engine->TitleWindow(id++,"ArtMoney PRO v7.27");
		Engine->TitleWindow(id++,"ArtMoney SE v7.31");
		Engine->TitleWindow(id++,"ArtMoney SE v7.32");
		Engine->TitleWindow(id++,"Cheat Engine 5.0");
		Engine->TitleWindow(id++,"Cheat Engine 5.1");
		Engine->TitleWindow(id++,"Cheat Engine 5.1.1");
		Engine->TitleWindow(id++,"Cheat Engine 5.2");
		Engine->TitleWindow(id++,"Cheat Engine 5.3");
		Engine->TitleWindow(id++,"Cheat Engine 5.4");
		Engine->TitleWindow(id++,"Cheat Engine 5.5");
		Engine->TitleWindow(id++,"Cheat Engine 5.6");
		Engine->TitleWindow(id++,"Cheat Engine 5.6.1");
		Engine->TitleWindow(id++,"Cheat Engine 5.6.2");
		Engine->TitleWindow(id++,"Cheat Engine 5.6.3");
		Engine->TitleWindow(id++,"Cheat Engine 5.6.4");
		Engine->TitleWindow(id++,"Cheat Engine 5.6.5");
		Engine->TitleWindow(id++,"Cheat Engine 5.6.6");
		Engine->TitleWindow(id++,"Cheat Engine 6.0");
		Engine->TitleWindow(id++,"Cheat Engine 6.1");
		Engine->TitleWindow(id++,"Cheat Engine 6.2");
		Engine->TitleWindow(id++,"Cheat Engine 6.3");
		Engine->TitleWindow(id++,"Cheat Engine 6.4");
		Engine->TitleWindow(id++,"Cheat Engine 6.5");
		Engine->TitleWindow(id++,"Cheat Engine 6.6");
		Engine->TitleWindow(id++,"Cheat Engine 6.7");
		Engine->TitleWindow(id++,"Cheat Engine 7.0");
		Engine->TitleWindow(id++,"Cheat Engine 7.1");
		Engine->TitleWindow(id++,"Cheat Engine 7.2");
		Engine->TitleWindow(id++,"Cheat Engine 7.3");
		Engine->TitleWindow(id++,"Cheat Engine");
		Engine->TitleWindow(id++,"Created processes");
		Engine->TitleWindow(id++,"D-C Bypass");
		Engine->TitleWindow(id++,"DC-BYPASS By DjCheats  Public Vercion");
		Engine->TitleWindow(id++,"HiDeToolz");
		Engine->TitleWindow(id++,"HideToolz");
		Engine->TitleWindow(id++,"Injector");
		Engine->TitleWindow(id++,"Olly Debugger");
		Engine->TitleWindow(id++,"Process Explorer 11.33");
		Engine->TitleWindow(id++,"Process Explorer");
		Engine->TitleWindow(id++,"T Search");
		Engine->TitleWindow(id++,"WPE PRO");
		Engine->TitleWindow(id++,"WPePro 0.9a");
		Engine->TitleWindow(id++,"WPePro 1.3");
		Engine->TitleWindow(id++,"hacker");
		Engine->TitleWindow(id++,"rPE - rEdoX Packet Editor");
		Engine->TitleWindow(id++,"OllyDbg");
		Engine->TitleWindow(id++,"HxD");
		Engine->TitleWindow(id++,"Process Hacker 2");
		Engine->TitleWindow(id++,"Process Hacker");
		Engine->TitleWindow(id++,"Ultimate Cheat");
		Engine->TitleWindow(id++,"The following opcodes accessed the selected address");
		Engine->TitleWindow(id++,"lalaker1");
		Engine->TitleWindow(id++,"HiDeToolz");
		Engine->TitleWindow(id++,"HideToolz");
		Engine->TitleWindow(id++,"IDA!");
		Engine->TitleWindow(id++,"The interactive disassembler");
		Engine->TitleWindow(id++,"1nj3ct0r");
		Engine->TitleWindow(id++,"injector");
		Engine->TitleWindow(id++,"Injector");
		Engine->TitleWindow(id++,"dreamfancy");
		Engine->TitleWindow(id++,"pvp hile indir");
		Engine->TitleWindow(id++,"dreamfancy");
		Engine->TitleWindow(id++,"Knight Online tr hile indir");
		Engine->TitleWindow(id++,"Knight Online oyun hileleri sitesi 2015");
		Engine->TitleWindow(id++,"Knight Online Hilecim");
		Engine->TitleWindow(id++,"Knight Online Hileleri");
		Engine->TitleWindow(id++,"Knight Online Hile Ýndir");
		Engine->TitleWindow(id++,"Online Oyun Hileleri");
		Engine->TitleWindow(id++,"N-Jector by NOOB");
		Engine->TitleWindow(id++,"[BETA] Hack Loader v1.0.2");
		Engine->TitleWindow(id++,"[BETA] Hack Loader v1.0.3");
		Engine->TitleWindow(id++,"Wallhack");
		Engine->TitleWindow(id++,"Pro Damage");
		Engine->TitleWindow(id++,"Wait Damage (7x)");
		Engine->TitleWindow(id++,"Okçu hilesi");
		Engine->TitleWindow(id++,"Daily Stage 2 (Günlük Aþama 2 )");
		Engine->TitleWindow(id++,"Injector [FaithDEV]");
		Engine->TitleWindow(id++,"Injector Gadget");
		Engine->TitleWindow(id++,"OldSchoolInject");
		Engine->TitleWindow(id++,"Extreme Injector");
		Engine->TitleWindow(id++,"| Fortis Fortuna Adiuvat |");
		Engine->TitleWindow(id++,"PHTrenbot");
		//Engine->TitleWindow(id++,"Kaynak Ýzleyicisi");
		//Engine->TitleWindow(id++,"Görev Yöneticisi");
		Engine->TitleWindow(id++,"Process Lasso");
		Engine->TitleWindow(id++,"System Explorer 7.0.0.5356");
		//Engine->TitleWindow(id++,"Minor Programý");
		//Engine->TitleWindow(id++,"Macro Minor");
		//Engine->TitleWindow(id++,"Minor Macro");
		Engine->TitleWindow(id++,"Exe String Editore");
		Engine->TitleWindow(id++,"DaveLombardo");
		Engine->TitleWindow(id++,"ShowString");
		Engine->TitleWindow(id++,"TAMORY");
		Engine->TitleWindow(id++,"snoxd");
		Engine->TitleWindow(id++,"SQL Scripts");
		Engine->TitleWindow(id++,"odbg201");
		Engine->TitleWindow(id++,"odbg202");
		Engine->TitleWindow(id++,"OLLYDBG");
		Engine->TitleWindow(id++,"ollydbG");
		Engine->TitleWindow(id++,"PhantOm");
		Engine->TitleWindow(id++,"Themida");
		Engine->TitleWindow(id++,"Pointer Bulucu");
		Engine->TitleWindow(id++,"OTO POINTER");
		Engine->TitleWindow(id++,"Oto Pointer");
		Engine->TitleWindow(id++,"HDRExporter");
		Engine->TitleWindow(id++,"MoleBox Pro 2.6.4.2534");
		Engine->TitleWindow(id++,"MoleBox Pro 2.6.4");
		Engine->TitleWindow(id++,"otoPointer");
		Engine->TitleWindow(id++,"Tbl Editör");
		Engine->TitleWindow(id++,"Themida");
		Engine->TitleWindow(id++,"Themida v1");
		Engine->TitleWindow(id++,"Hooking");
		Engine->TitleWindow(id++,"SOACS_Yourko_2017");
		Engine->TitleWindow(id++,"Dll-Injectorv2 by Padmak");
		Engine->TitleWindow(id++,"ThunderRT6FormDC");
		Engine->TitleWindow(id++,"KoPluginGlobal_v7.0.8");
		Engine->TitleWindow(id++,"PaidBot");
		Engine->TitleWindow(id++,"Extreme Injector v3.5 by master131");
		Engine->TitleWindow(id++,"Multi Macro Bot 1.0");
		Engine->TitleWindow(id++,"VMware vSphere Client");
		Engine->TitleWindow(id++,"Okcu Hilesi");
		Engine->TitleWindow(id++,"MyPacketTool");
		Sleep(30000);
	}
	VIRTUALIZER_END
}

extern HANDLE myMutex;
DWORD KO_FNC_OBJECT_PLAYER_LOOP_ORG;
extern NOTIFYICONDATA nid;

void __declspec(naked) hkEndGame()
{
	__asm {
		pushad
		pushfd
	}
	Shell_NotifyIcon(NIM_DELETE, &nid);
	Engine->render = false;
	if (myMutex)
	{
		ReleaseMutex(myMutex);
	}
	TerminateProcess(GetCurrentProcess(), 1);
	__asm {
		popfd
		popad
		call KO_CALL_END_GAME
		mov edx, KO_FNC_END_GAME
		add edx, 5
		jmp edx
	}
}
//22.12.2020 Semih tarafindan asm ile exedeki kontrolü devre dýþý býrakýp istediðimiz zonelerde actýrabiliyoruz asagida zoneid else if diyerek
//ekstra baska zonelerdede acýlabilir
DWORD zoneID = 0;
DWORD kontrolAdresi = 0x00551A8E;  // 2369
DWORD rankiAc = 0x00551AA3;  // 2369

void __declspec(naked) hkRank()
{
	__asm
	{
		mov eax, [ECX +0xCE8] // 2369
		mov zoneID, eax
		pushad
		pushfd
	}

	if (isInSpecialEventZone(zoneID) || zoneID==BIFROST || isInWarZone(zoneID))
		__asm jmp rankiActir;

	__asm {
		popfd
		popad
		mov edx, kontrolAdresi
		add edx, 6
		jmp edx
		rankiActir:
		popfd
		popad
		jmp rankiAc
	}
}

// Anti afk ve kafaya yazý
DWORD fncScanZ = 0x57F428,/* 0x00563B8E,*/ keepZ = fncScanZ + 5, retNopZ = fncScanZ + 0x41; //2369
DWORD fncScanB = 0x57EB5D,/* 0x00563489,*/ keepB = fncScanB + 5, retNopB = fncScanB + 0x38; //2369
std::string objTMPString = "";
void SetInfoString(DWORD obj, std::string text, DWORD color, DWORD fontStyle)
{
	objTMPString = text + " ";
	DWORD b = obj;
	__asm {
		push color
		push offset objTMPString
		mov ecx, b
		call KO_FNC_SET_INFO_STRING
	}
}

bool __fastcall IsAntiAFK(DWORD mobBase)
{
	if (!mobBase) return false;

	uint16 protoID = *(uint16*)(mobBase + KO_SSID);
	if (std::find(antiAFKProto.begin(), antiAFKProto.end(), protoID) != antiAFKProto.end())
		return true;

	uint16 protoID2 = *(uint16*)(mobBase + KO_OFF_ID);
	if (Engine->m_isGenieStatus && isLeaderAttack && Engine->isInParty())
	{
		if (Engine->GetPartyLeaderTarget() == protoID2)
			return false;

		return true;
	}
	//if (protoID < NPC_BAND) // oyuncuysa  seçme
	//	return true; 
	
	return std::find(antiAFKProto.begin(), antiAFKProto.end(), protoID) != antiAFKProto.end();
}
bool __fastcall IsAntiAFK2(DWORD mobBase)
{
	if (!mobBase) return false;

	uint16 protoID = *(uint16*)(mobBase + KO_SSID);
		
	return std::find(antiAFKProto.begin(), antiAFKProto.end(), protoID) != antiAFKProto.end();
}

DWORD tBase = 0;
void __declspec(naked) hkZ()
{
	__asm {
		call orgCall

		pushad
		pushfd
		mov ecx, esi
		call IsAntiAFK
		movzx eax, al
		test eax, eax
		jne antiafkZ
		popfd
		popad
		mov byte ptr ds : [0x57F440], 0x75
		jmp keepZ
		antiafkZ :

			popfd
			popad
			/*fstp dword ptr[esp + 0x14]
			fld dword ptr[esp + 0x14]
			fld dword ptr[esp + 0x10]
			
			fnstsw ax
			test ah,41
			fstp st(0)*/
			//jmp retNopZ
			mov byte ptr ds:[0x57F440],0xEB
			jmp keepZ
	}
}

void __declspec(naked) hkB()
{
	__asm {
		call orgCall
		pushad
		pushfd
		mov ecx, esi
		call IsAntiAFK
		movzx eax, al
		test eax, eax
		jne antiafkB
		popfd
		popad
		mov byte ptr ds:[0x57EB75], 0x75
		jmp keepB
		antiafkB :
		popfd
		popad
		//jmp retNopB
		mov byte ptr ds:[0x57EB75],0xEB
		jmp keepB
	}
}

void __declspec(naked) hkGenieSelect()
{
	__asm {
		call KO_FNC_GENIE_SELECT_ORG_CALL
		pushad
		pushfd
		mov ecx, eax
		call IsAntiAFK
		movzx eax, al
		test eax, eax
		jne antiafkB
		popfd
		popad
		jmp KO_FNC_GENIE_SELECT_RET
		antiafkB :
		popfd
			popad
			xor eax, eax
			jmp KO_FNC_GENIE_SELECT_RET
	}
}
DWORD GetLevelColor(int diff)
{
	if (diff > 5)
		return 0xFF1DDB16;
	if (diff < -5)
		return diff < -10 ? 0xFFFF0000 : 0xFFFF9436;
	return 0xFFEAEAEA;
}

extern std::string GetName(DWORD obj);

void Kalinlastir(DWORD obj)
{
	//if(obj) SetNameString(obj, GetName(obj), *(DWORD*)(obj + 0x6A0), 0);   //Nick kalýnlaþtýrma
}


void __fastcall Object_Mob_Callback(DWORD obj)
{
	if (!obj) return;
	uint16 protoID = *(uint16*)(obj + KO_SSID);
	string MobID = "";
	uint8 authority = *(uint8*)(*(DWORD*)KO_PTR_CHR + KO_WH);
	int16 id = *(int16*)(obj + KO_OFF_ID);
	uint8 nation = *(uint8*)(obj + KO_OFF_NATION);

	Kalinlastir(obj);

	if (id < NPC_BAND)
		return;

	uint8 myLvl = *(uint8*)(*(DWORD*)KO_PTR_CHR + KO_OFF_LEVEL);
	uint8 lvl = *(uint8*)(obj + KO_OFF_LEVEL);
	DWORD color = GetLevelColor(myLvl - lvl);
	std::string name = GetName(obj);

	if(IsAntiAFK2(obj))
		SetInfoString(obj, xorstr("Anti AFK"), D3DCOLOR_ARGB(255, 0, 255, 0), 0);
	
	if (authority == 0 && protoID)
	{
		if (nation != 0 && protoID != 19067 && protoID != 19068 && protoID != 19069 && protoID != 19070 && protoID != 19071 && protoID != 19072)
			SetNameString(obj, GetName(obj) + " (" + to_string(*(uint8*)(obj + KO_OFF_LEVEL)) + ")", *(DWORD*)(obj + 0x738), 0);

		if (IsAntiAFK2(obj))
		{
			MobID = string_format("Anti AFK [ID : %d]", protoID);
		}
		else
		{
			MobID = string_format("[ID(%d) : SID(%d) : TYPE(0)]",id, protoID);
		}

		SetInfoString(obj, MobID, D3DCOLOR_ARGB(255, 0, 255, 0), 0);
	}
	// obj = mob base	
	
}

bool isGM = false;

struct PartyUser {
	USHORT Id;
	USHORT Hp;
	USHORT MaxHp;
	USHORT Mp;
	USHORT MaxMp;
	USHORT Class;
	USHORT Level;
	bool Curse;
	bool Disease;
};

vector<PartyUser> partyMember;
bool partyInit = false;
bool amIInParty = false;
bool waitingForParty = false;

void __fastcall Object_Player_Callback(DWORD obj)
{
	if (!obj) return;
	if (Engine->m_UiMgr->uiMerchantEye != NULL && Engine->m_UiMgr->uiMerchantEye->IsVisible())
		return;
	uint8 authority = *(uint8*)(obj + KO_WH);
	uint8 Level = *(uint8*)(obj + KO_OFF_LEVEL);
	int16 id = *(int16*)(obj + KO_OFF_ID);
	uint8 nation = *(uint8*)(obj + KO_OFF_NATION);
	uint8 nationm = *(uint8*)(*(DWORD*)KO_PTR_CHR + KO_OFF_NATION);

	if (id > NPC_BAND)
		return;

	bool changeit = false;
	bool dusmanMi = nation != nationm;
#if (HOOK_SOURCE_VERSION == 1098)

	if (GetName(obj) == GetName(*(DWORD*)KO_PTR_CHR))
	{
		if (Engine->uiPartyBBS != NULL && Engine->uiPartyBBS->PartyFind(id))
			SetNameString(obj, GetName(obj), D3DCOLOR_ARGB(255, 255, 255, 0), 0); //party sari
		else if (Level < 30 && Engine->uiPartyBBS != NULL && !Engine->uiPartyBBS->PartyFind(id))
			SetNameString(obj, GetName(obj), D3DCOLOR_ARGB(255, 255, 255, 255), 0); //civciv beyaz
		else
			SetNameString(obj, GetName(obj), D3DCOLOR_ARGB(255, 100, 210, 255), 0); //kendi nickim açýk mavi
	}
	else
	{
		if (Level < 30)
		{
			if (Engine->uiPartyBBS != NULL && Engine->uiPartyBBS->PartyFind(id))
				SetNameString(obj, GetName(obj), D3DCOLOR_ARGB(255, 255, 255, 0), 0); //party sari
			else
				SetNameString(obj, GetName(obj), D3DCOLOR_ARGB(255, 255, 255, 255), 0); //civciv beyaz
		}
		else 
		{
			if (Engine->uiPartyBBS != NULL && Engine->uiPartyBBS->PartyFind(id))
				SetNameString(obj, GetName(obj), D3DCOLOR_ARGB(255, 255, 255, 0), 0); //party sari
			else
			{
				if (dusmanMi)
					SetNameString(obj, GetName(obj), D3DCOLOR_ARGB(255, 255, 128, 128), 0); //karsi irk kirmizi
				else
					SetNameString(obj, GetName(obj), D3DCOLOR_ARGB(255, 128, 128, 255), 0); //kendi irkindan olanlarýn koyu mavisi
			}
		}
	} 
	
	if (*(uint8*)(*(DWORD*)KO_PTR_CHR + KO_WH) == 0)
	{
		if (Level < 30) {
		
			if (Engine->uiPartyBBS != NULL && Engine->uiPartyBBS->PartyFind(id))
				SetNameString(obj, GetName(obj) + " (" + to_string(*(uint8*)(obj + KO_OFF_LEVEL)) + ")", D3DCOLOR_ARGB(255, 255, 255, 0), 0); //party sari
			else
				SetNameString(obj, GetName(obj) + " (" + to_string(*(uint8*)(obj + KO_OFF_LEVEL)) + ")", D3DCOLOR_ARGB(255, 255, 255, 255), 0); //civciv beyaz
		}
		else 
		{
			if (Engine->uiPartyBBS != NULL && Engine->uiPartyBBS->PartyFind(id))
				SetNameString(obj, GetName(obj) + " (" + to_string(*(uint8*)(obj + KO_OFF_LEVEL)) + ")", D3DCOLOR_ARGB(255, 255, 255, 0), 0); //party sari
			else
			{
				if (dusmanMi) 
					SetNameString(obj, GetName(obj) + " (" + to_string(*(uint8*)(obj + KO_OFF_LEVEL)) + ")", D3DCOLOR_ARGB(255, 255, 128, 128), 0); //karsi irk kirmizi
				else 
					SetNameString(obj, GetName(obj) + " (" + to_string(*(uint8*)(obj + KO_OFF_LEVEL)) + ")", D3DCOLOR_ARGB(255, 128, 128, 255), 0); //kendi irkindan olanlarýn koyu mavisi
			}
		}
	}
#endif


	Engine->tagLock.lock();
	auto itr = Engine->tagList.find(id);
	if (itr != Engine->tagList.end())
	{
		tagName tag = itr->second;
		if (tag.tag.length() > 1) SetInfoString(obj, tag.tag, D3DCOLOR_ARGB(255, tag.r, tag.g, tag.b), 0);
	}
	Engine->tagLock.unlock();
	if (authority == 0 && *(uint8*)(obj + 0x7AC) == 3)
	{
		*(uint8*)(obj + 0x7AC) = 0;
		*(uint8*)(obj + KO_WH) = 1;
		return;
	}
	switch (authority)
	{
	case 0:
		if (*(uint8*)(*(DWORD*)KO_PTR_CHR + KO_WH) == 0)
		{
//#if (HOOK_SOURCE_VERSION == 1098)
//			if (isInMyParty)
//				SetNameString(obj, "[GM] " + GetName(obj) + " (" + to_string(*(uint8*)(obj + KO_OFF_LEVEL)) + ")", D3DCOLOR_ARGB(255, 255, 255, 0), 0); //party sari
//			else
//				SetNameString(obj, "[GM] " + GetName(obj) + " (" + to_string(*(uint8*)(obj + KO_OFF_LEVEL)) + ")", D3DCOLOR_ARGB(255, 225, 0, 228), 0); //gm nick turuncu 
//#endif
		}
		else
		{
			SetNameString(obj, "[GM] " + GetName(obj),0, 0);
		}

		SetInfoString(obj, xorstr("[GAME MASTER]"), D3DCOLOR_ARGB(255, 64, 236, 72), 0);
		break;
	}
	
}



void __declspec(naked) hkObjectMobLoop()
{
	__asm {
		mov eax, [ecx]
		mov edx, [eax + 0xC]
		pushad
		pushfd
		call Object_Mob_Callback
		popfd
		popad
		jmp KO_FNC_OBJECT_MOB_LOOP_RET
	}
}

void __declspec(naked) hkObjectPlayerLoop()
{
	__asm {
		pushad
		pushfd
		mov ecx, ebx
		call Object_Player_Callback
		popfd
		popad
		jmp KO_FNC_OBJECT_PLAYER_LOOP_ORG
	}
}


// KO Tick fonksiyonu (sürekli çalýþýr)
const DWORD KO_GAME_TICK = 0x411740;// 0x00410C10;
DWORD TICK_ORG = 0;

const DWORD OFF_PT = 0x358;
const DWORD OFF_PTCOUNT = 0x344;
const DWORD OFF_PTBASE = 0x238;

__inline DWORD RDWORD(DWORD ulBase)
{
	if (ulBase > 0x00AFFFFF && !IsBadReadPtr((VOID*)ulBase, sizeof(DWORD)))
	{
		return(*(DWORD*)(ulBase));
	}
	return 0;
}

const WORD KO_OFF_INV1 = 0x204; // 2369
const WORD KO_OFF_INV2 = 0x244; // 2369

DWORD InvItemId(int slot) {
	if (Engine->uiInventoryPlug)
	{
		if (DWORD inv = Engine->uiInventoryPlug->m_dVTableAddr)
		{
			if (DWORD itemBase = RDWORD(inv + KO_OFF_INV2 + (4 * slot)))
			{
				if (DWORD extBase = RDWORD(itemBase + 0x6C))
				{
					DWORD ID = RDWORD(RDWORD(itemBase + 0x68));   //infinityarrow
					DWORD EXT = RDWORD(extBase);  //infinityarrow
					return ID + EXT;
				}
			}
		}
	}
	return 0;
}

bool HasItem(DWORD itemid) {
	for (int x = 14; x < 42; x++) {
		if (InvItemId(x) == itemid) return true;
	}
	return false;
}

time_t arrowKontrol = 0;
time_t timeLapse = 0, guardCheck = 0;

time_t inv_animtime = 0;

std::vector<CSpell*> archerSpells;
std::vector<CSpell*> cureSpells;

extern TABLE_MOB* GetMobBase(DWORD MobID);

typedef char(__thiscall* tsub_763350)(DWORD ecx, int a2, int a3, int a4, int a5, int a6);
tsub_763350 sub_763350 = (tsub_763350)0x763350;

bool isChicken = false;

void __fastcall civcivVer()
{

}

void __declspec(naked) hkRankImage()
{
	__asm {
		mov al, isChicken
		movzx eax, al
		test eax, eax


		civcivle:

	}
}
bool InifinityArrow = false;

static bool spellsloaded = false;
void __fastcall myTick()
{
	if (timeLapse > clock() - 30)
		return;

	timeLapse = clock();

	if (guardCheck < clock() - 200)
	{
		guardCheck = clock();
		if (!Engine->fncGuard.Check())
		{
			raise(SIGSEGV);
		}
	}

	*(float*)0xDE2B98 = 1.0f / 999.0f;																	// -------------------- fps limiti kaldýrma

	if (DWORD target = Engine->GetTarget()) {
		if (*(uint16*)(target + KO_OFF_ID) >= NPC_BAND) {
			uint8 myLvl = *(uint8*)(*(DWORD*)KO_PTR_CHR + KO_OFF_LEVEL);
			uint8 lvl = *(uint8*)(target + KO_OFF_LEVEL);
			uint8 nation = *(uint8*)(target + KO_OFF_NATION);
			uint16 protoID = *(uint16*)(target + KO_SSID);
			DWORD color = GetLevelColor(myLvl - lvl);
			std::string name = GetName(target);
			if (Engine->uiTargetBar && nation == 0 
				&& protoID != 19067 && protoID != 19068 && protoID != 19069 && protoID != 19070 && protoID != 19071 && protoID != 19072) 
				Engine->uiTargetBar->SetInfo(color, lvl, name);
		}
	}

#if (HOOK_SOURCE_VERSION == 1098)

	if (!partyInit)
	{
		PartyUser mem{};
		mem.Id = 0;
		mem.Hp = 0;
		mem.MaxHp = 0;
		mem.Mp = 0;
		mem.MaxMp = 0;
		mem.Class = 0;
		mem.Level = 0;
		mem.Curse = false;
		mem.Disease = false;
		for (uint8 i = 0; i < 8; i++) partyMember.push_back(mem);
		partyInit = true;
	}

#endif

//animasyon start
	if (*(DWORD*)KO_PTR_CHR) 
	{
		if (Engine->m_PlayerBase)
			Engine->m_PlayerBase->m_iZoneID = *(uint8*)(*(DWORD*)KO_PTR_CHR + KO_OFF_ZONE);

		if (Engine->uiInventoryPlug
			&& GetTickCount64() > inv_animtime
			&& Engine->IsVisible(Engine->uiInventoryPlug->m_dVTableAddr))
		{
			DWORD chr = *(DWORD*)KO_PTR_CHR + KO_OFF_N3CHR;
			if (chr)
				__ChrAniCurSet(chr, *(int*)(*(DWORD*)KO_PTR_CHR + KO_OFF_ANIM_ID), false, 1.175494351e-38F, 0, true);
			inv_animtime = GetTickCount64() + 200;
		}
		

		for (auto a : Engine->privatemessages)
			a->Tick();

	
	//animasyon end----------------------------------

#if (HOOK_SOURCE_VERSION == 1098)

		//if (!IsDebuggerPresent())
		//{
		//	DWORD base, buffer, partyCount;

		//	base = RDWORD(RDWORD(KO_DLG) + OFF_PTBASE);

		//	if (base > 0)
		//		buffer = RDWORD(base + OFF_PT);

		//	if (base > 0)
		//		partyCount = RDWORD(base + OFF_PTCOUNT);

		//	if (base > 0 && buffer > 0)
		//	{
		//		amIInParty = partyCount > 1;
		//		waitingForParty = partyCount > 0;

		//		for (int i = 0; i < 8; i++)
		//		{
		//			partyMember[i].Id = 0;
		//			partyMember[i].Hp = 0;
		//			partyMember[i].MaxHp = 0;
		//			partyMember[i].Mp = 0;
		//			partyMember[i].MaxMp = 0;
		//			partyMember[i].Class = 0;
		//			partyMember[i].Level = 0;
		//			partyMember[i].Curse = false;
		//			partyMember[i].Disease = false;
		//		}

		//		for (int i = 0; i < partyCount; i++)
		//		{
		//			if (partyCount > 0)
		//			{
		//				buffer = RDWORD(buffer);
		//				if (buffer)
		//				{
		//					partyMember[i].Id = *(USHORT*)(buffer + 0x8);
		//					partyMember[i].Level = *(USHORT*)(buffer + 0xC);
		//					partyMember[i].Class = *(USHORT*)(buffer + 0x10);
		//					partyMember[i].Hp = *(USHORT*)(buffer + 0x18);
		//					partyMember[i].MaxHp = *(USHORT*)(buffer + 0x1C);
		//					partyMember[i].Mp = *(USHORT*)(buffer + 0x20);
		//					partyMember[i].MaxMp = *(USHORT*)(buffer + 0x24);
		//					partyMember[i].Curse = *(bool*)(buffer + 0x28);
		//					partyMember[i].Disease = *(bool*)(buffer + 0x29);
		//				}
		//			}
		//		}
		//	}
		//}
#endif
		
		 //infinity arrow
		if (arrowKontrol < clock() - 2000)
		{
			arrowKontrol = clock();
			if (Engine->uiInventoryPlug)
			{
				if (DWORD inv = Engine->uiInventoryPlug->m_dVTableAddr)
				{
					
					if (!spellsloaded) {
						spellsloaded = true;
						archerSpells.clear();
						cureSpells.clear();
						for (uint32 i = 102003; i <= 288566; i++)
						{
							if (CSpell* spell = GetSkillBase(i)) {
								if (spell->dwNeedItem == 391010000)
									archerSpells.push_back(spell);
							}
						}
						std::map<uint32, CSpell>::iterator itr;
						for (itr = Engine->skillmap.begin(); itr != Engine->skillmap.end(); itr++)
						{
							if (CSpell* spell = GetSkillBase(itr->second.dwID)) {
								if (spell->dwNeedItem == 379062000
									|| spell->dwNeedItem == 379059000
									|| spell->dwNeedItem == 379060000
									|| spell->dwNeedItem == 379061000)
									cureSpells.push_back(spell);
							}
						}
						if (CSpell* spell = GetSkillBase(107736)) cureSpells.push_back(spell);
						if (CSpell* spell = GetSkillBase(108736)) cureSpells.push_back(spell);
						if (CSpell* spell = GetSkillBase(207736)) cureSpells.push_back(spell);
						if (CSpell* spell = GetSkillBase(208736)) cureSpells.push_back(spell);
					}
					//if (HasItem(479062000)) // Stone of Priest 
					//{
					//	for (auto& s : cureSpells)
					//	{
					//		if (s->dwNeedItem == 379062000) {
					//			s->dwNeedItem = 0;
					//			auto itcrc = Engine->skillcrc.find(s->dwID + 2031);
					//			if (itcrc != Engine->skillcrc.end()) {
					//				SpellCRC crc(crc32((uint8*)(DWORD)s, 0xA8, -1), crc32((uint8*)((DWORD)s + 0xB4), 0x2C, -1));
					//				itcrc->second = crc;
					//			}
					// 
					// 
					// 
					// 
					//		}
					//	}
					//	*(DWORD*)(0x00673CF6 +1) = 0x1C8DE7F0;
					//	*(DWORD*)(0x00676CCF +1) = 0x1C8DE7F0;
					//	*(DWORD*)(0x00934B8A +1) = 0x1C8DE7F0;
					//	*(DWORD*)(0x00934BC1 +1) = 0x1C8DE7F0;
					//	*(DWORD*)(0x00934BE0 +1) = 0x1C8DE7F0;
					//	*(DWORD*)(0x00936CED +1) = 0x1C8DE7F0;
					//	*(DWORD*)(0x00936D23 +1) = 0x1C8DE7F0;
					//	*(DWORD*)(0x00936D69 +1) = 0x1C8DE7F0;
					//}			 
					//if (HasItem(479061000)) // Stone of Mage 
					//{
					//	for (auto& s : cureSpells)
					//	{
					//		if (s->dwNeedItem == 379061000) {
					//			s->dwNeedItem = 479061000;
					//			auto itcrc = Engine->skillcrc.find(s->dwID + 2031);
					//			if (itcrc != Engine->skillcrc.end()) {
					//				SpellCRC crc(crc32((uint8*)(DWORD)s, 0xA8, -1), crc32((uint8*)((DWORD)s + 0xB4), 0x2C, -1));
					//				itcrc->second = crc;
					//			}
					//		}
					//		*(uint32*)(0x00673C9E + 1) = 479061000;
					//		*(uint32*)(0x00676C77 + 1) = 479061000;
					//		*(uint32*)(0x00934B10 + 1) = 479061000;
					//		*(uint32*)(0x00934B47 + 1) = 479061000;
					//		*(uint32*)(0x00934B66 + 1) = 479061000;
					//		*(uint32*)(0x00934B71 + 1) = 479061000;
					//		*(uint32*)(0x00936C2F + 1) = 479061000;
					//		*(uint32*)(0x00936C6D + 1) = 479061000;
					//		*(uint32*)(0x00936CD0 + 1) = 479061000;
					//	}
					//}
					//if (HasItem(479060000)) // Stone of Rogue 
					//{
					//	for (auto& s : cureSpells)
					//	{
					//		if (s->dwNeedItem == 379060000) {
					//			s->dwNeedItem = 0;
					//			auto itcrc = Engine->skillcrc.find(s->dwID + 2031);
					//			if (itcrc != Engine->skillcrc.end()) {
					//				SpellCRC crc(crc32((uint8*)(DWORD)s, 0xA8, -1), crc32((uint8*)((DWORD)s + 0xB4), 0x2C, -1));
					//				itcrc->second = crc;
					//			}
					//		}
					//		*(DWORD*)(0x00673C46 + 1) = 479060000;
					//		*(DWORD*)(0x00676C1F + 1) = 479060000;
					//		*(DWORD*)(0x009348F7 + 1) = 479060000;
					//		*(DWORD*)(0x00934A96 + 1) = 479060000;
					//		*(DWORD*)(0x00934ACD + 1) = 479060000;
					//		*(DWORD*)(0x00934AEC + 1) = 479060000;
					//		*(DWORD*)(0x00936B71 + 1) = 479060000;
					//		*(DWORD*)(0x00936BAF + 1) = 479060000;
					//		*(DWORD*)(0x00936C12 + 1) = 479060000;
					//	}							    
					//}
					//if (HasItem(479059000)) // Stone of Warrior
					//{
					//	for (auto& s : cureSpells)
					//	{
					//		if (s->dwNeedItem == 379059000) {
					//			s->dwNeedItem = 479059000;
					//			auto itcrc = Engine->skillcrc.find(s->dwID + 2031);
					//			if (itcrc != Engine->skillcrc.end()) {
					//				SpellCRC crc(crc32((uint8*)(DWORD)s, 0xA8, -1), crc32((uint8*)((DWORD)s + 0xB4), 0x2C, -1));
					//				itcrc->second = crc;
					//			}
					//		}
					//		*(DWORD*)(0x00673BEE + 1) = 479059000;
					//		*(DWORD*)(0x00676BC7 + 1) = 479059000;
					//		*(DWORD*)(0x009348F7 + 1) = 479059000;
					//		*(DWORD*)(0x0093493A + 1) = 479059000;
					//		*(DWORD*)(0x00934A72 + 1) = 479059000;
					//		*(DWORD*)(0x00934AB3 + 1) = 479059000;
					//		*(DWORD*)(0x00936AF1 + 1) = 479059000;
					//		*(DWORD*)(0x00936B54 + 1) = 479059000;

					//	}
					//}
					if (HasItem(346391000) && !isActivedArrow) // infinity cure
					{
						for (auto &s : cureSpells)
						{
							if (s->dwNeedItem == 370005000) {
								s->dwNeedItem = 0;
								auto itcrc = Engine->skillcrc.find(s->dwID + 2031);
								if (itcrc != Engine->skillcrc.end()) {
									SpellCRC crc(crc32((uint8*)(DWORD)s, 0xA8, -1), crc32((uint8*)((DWORD)s + 0xB4), 0x2C, -1));
									itcrc->second = crc;
								}
							}
						}
						isActivedArrow = true;
					}
					else if(!isActivedArrow) {
						for (auto &s : cureSpells)
						{
							if (s->dwNeedItem == 0) {
								s->dwNeedItem = 370005000;
								auto itcrc = Engine->skillcrc.find(s->dwID + 2031);
								if (itcrc != Engine->skillcrc.end()) {
									SpellCRC crc(crc32((uint8*)(DWORD)s, 0xA8, -1), crc32((uint8*)((DWORD)s + 0xB4), 0x2C, -1));
									itcrc->second = crc;
								}
							}
						}
					}

					if (HasItem(800606000) && !isActivedCure)
					{
						for (auto &s : archerSpells)
						{
							if (s->dwNeedItem == 391010000) {
								s->dwNeedItem = 0;
								auto itcrc = Engine->skillcrc.find(s->dwID + 2031);
								if (itcrc != Engine->skillcrc.end()) {
									SpellCRC crc(crc32((uint8*)(DWORD)s, 0xA8, -1), crc32((uint8*)((DWORD)s + 0xB4), 0x2C, -1));
									itcrc->second = crc;
								}
							}
						}
						isActivedCure = true;
					}
					else if(!isActivedCure) {
						for (auto &s : archerSpells)
						{
							if (s->dwNeedItem == 0) {
								s->dwNeedItem = 391010000;
								auto itcrc = Engine->skillcrc.find(s->dwID + 2031);
								if (itcrc != Engine->skillcrc.end()) {
									SpellCRC crc(crc32((uint8*)(DWORD)s, 0xA8, -1), crc32((uint8*)((DWORD)s + 0xB4), 0x2C, -1));
									itcrc->second = crc;
								}
							}
						}
					}




				}
			}
		}

	
	} 
}

DWORD __declspec(naked) hkTick()
{
	__asm {
		pushad
		pushfd
		call myTick
		popfd
		popad
		jmp TICK_ORG
	}
}

DWORD rtOrg = 0;
DWORD rTmp = 0;

DWORD __declspec(naked) hkSetFontString()
{
	__asm {
		pushad
		pushfd
		mov rTmp, ecx
	}
	if (RDWORD(rTmp + 0x1C) == 0)
	{
		__asm {
			popfd
			popad
			xor eax, eax
			ret 8
		}
	}
	__asm {
		popfd
		popad
		jmp rtOrg
	}
}

#if (HOOK_SOURCE_VERSION == 1098)
// minimap harita pozisyon ayarlarý
const int upY = 5;
const int upX = 17;

void __declspec(naked) hkMinimapPos()
{
	__asm {
		add edx, upY
		add eax, upX
		push edx
		push eax
		mov eax, [ESI + 0x2C]
		call eax
		pop edi
		pop esi
		pop ebp
		pop ebx
		add esp, 0x38
		ret
	}
}
#endif

DWORD PearlEngine::KOGetTarget()
{
	DWORD KO_ADR = *(DWORD*)KO_PTR_CHR;
	uint16 targetID = *(DWORD*)(KO_ADR + KO_OFF_MOB);
	if (targetID >= NPC_BAND)
	{
		Func_GetObjectBase = (tGetObjectBase)KO_FMBS;
		return Func_GetObjectBase(*(DWORD*)KO_FLDB, targetID, 1);
	}
	else {
		Func_GetObjectBase = (tGetObjectBase)KO_FPBS;
		return Func_GetObjectBase(*(DWORD*)KO_FLDB, targetID, 1);
	}
	return NULL;
}

size_t findCaseInsensitive(std::string data, std::string toSearch)
{
	if (data.empty() || toSearch.empty())
		return string::npos;

	std::transform(data.begin(), data.end(), data.begin(), ::tolower);
	std::transform(toSearch.begin(), toSearch.end(), toSearch.begin(), ::tolower);
	return data.find(toSearch, 0);
}

const DWORD KO_AUTO_ATTACK = 0x0053E61A;
const DWORD KO_ZONE_LOAD_TERRAIN = 0x009544D3;
const DWORD KO_ZONE_LOAD_TERRAIN_KEEP = 0x009544F0;
DWORD KO_ZONE_LOAD_TERRAIN_ORG = 0;

std::list<std::string> oldMapList_GTD = {

	"battlezone_f.gtd",
	//"freezone_a_event2012.gtd"
	//"freezone_b_2012event.gtd",
	"In_dungeon06.gtd",
	"xmas_battle.gtd",
	"Old_Karus.gtd",
	"Old_Elmo.gtd",
	"2017_flagwar.gtd",
	"Worldwar.gtd",
	"FreeZone_bb.gtd",
	"FreeZone.gtd",
	"moradon_xmas.gtd",
#if (HOOK_SOURCE_VERSION == 1098)
	"war_a.gtd",
#endif
};

bool __fastcall IsOldMap(const std::string& gtd)
{
	const std::string tmpGtd = gtd;
	auto it = std::find_if(oldMapList_GTD.begin(), oldMapList_GTD.end(), [&](const std::string& v) {  return findCaseInsensitive(tmpGtd, v) != string::npos; });
	return it != oldMapList_GTD.end();
}

void __declspec(naked) hkLoadTerrainData()
{
	__asm {
		pushad
		pushfd
		mov ecx, esi
		add ecx, 0x24
		call IsOldMap
		movzx eax, al
		test eax, eax
		jne okuma 
		mov ecx, DWORD PTR DS : [KO_PTR_CHR]
		mov ecx, [ecx + KO_OFF_ZONE]
		cmp ecx, 0x70 // Harita zonesi hex kodu yazýlacak.
		je okuma
		popfd
		popad
		jmp KO_ZONE_LOAD_TERRAIN_ORG
		okuma :
		popfd
			popad
			jmp KO_ZONE_LOAD_TERRAIN_KEEP
	}
}

bool __genieAttackBlock = true;


bool __fastcall RAttack(DWORD caller = NULL)
{
	if (caller && caller >= 0xc78b7c && !IsBadReadPtr((void*)caller, sizeof(DWORD)))
		return __genieAttackBlock && *(DWORD*)caller == 0xc78b7c;  // Genie R atack buton kodu
	return false;
}



void __declspec(naked) hkAttack()
{
	__asm {
		mov esi, 0xFFFE
		mov edx, [ESP]
		pushad
		pushfd
		mov ecx, edx
		call RAttack
		movzx eax, al
		test eax, eax
		jne engeller
		popfd
		popad
		mov edx, KO_AUTO_ATTACK
		add edx, 5
		push edx
		ret
		engeller :
		popfd
			popad
			pop edi
			xor al, al
			pop esi
			ret
	}
}

void __fastcall hkDisableVisible(DWORD ecx, DWORD edx, bool a)
{
	a = false;
	*(DWORD*)(ecx + 0x10D) = a;
}

//void __fastcall hkOpenMiniMapfromGlobal(DWORD ecx, DWORD edx, bool a)  // Büyük MAp (M) olaný Küçük MAp'e (N) 'ye çeviriyoruz.
//{
//	a = false;
//	*(DWORD*)(ecx + 0x10D) = a;
//
//	if (Engine->uiMinimapPlug) {
//		if (DWORD map = Engine->uiMinimapPlug->m_dVTableAddr) {
//			if (*(bool*)(map + 0xF9))
//			{
//				__asm {
//					pushad
//					pushfd
//					mov ecx, map
//					mov eax, 0x846C40
//					call eax
//					popfd
//					popad
//				}
//			}
//			else {
//				__asm {
//					pushad
//					pushfd
//					mov ecx, map
//					mov eax, 0x846E00
//					call eax
//					popfd
//					popad
//				}
//			}
//		}
//	}
//}


const DWORD KO_SKILLBAR_KEY_INPUT = 0x00670AA0;
const DWORD KO_SKILLBAR_KEY_INPUT_BLOCK = 0x00670AAC;
DWORD KO_SKILLBAR_KEY_INPUT_KEEP = NULL;
DWORD KO_SKILLBAR_SHORTCUT_KEEP = 0x005007D5;
DWORD KO_SKILLBAR_MOUSE_INPUT_KEEP = 0x0066E519;

const DWORD KO_SKILLBAR_SHORTCUT_INPUT = 0x005007D0;
const DWORD KO_SKILLBAR_SHORTCUT_BLOCK = 0x005007DE;
const DWORD KO_SKILLBAR_MOUSE_INPUT = 0x0066E514; // EDX
const DWORD KO_SKILLBAR_MOUSE_INPUT_BLOCK = 0x0066E58D;

bool __fastcall MacroEngels()
{
	if (Engine->m_isGenieStatus) // geniede skill attýrmak istiyorsan bunu kaldýr
		return true;

	return false;
}

void __declspec(naked) hkSkillbarShortcutInput()
{
	__asm {
		pushad
		pushfd
		call MacroEngels
		movzx eax, al
		test eax, eax
		jne engellez
		popfd
		popad
		mov eax, [0x0F368E8] //2369
		jmp KO_SKILLBAR_SHORTCUT_KEEP
		engellez :
		popfd
			popad
			jmp KO_SKILLBAR_SHORTCUT_BLOCK
	}
}

void __declspec(naked) hkSkillbarMouseInput()
{
	__asm {
		pushad
		pushfd
		call MacroEngels
		movzx eax, al
		test eax, eax
		jne engellex
		popfd
		popad
		mov eax, 0x004C65A0 // 2369
		call eax
		jmp KO_SKILLBAR_MOUSE_INPUT_KEEP
		engellex :
		popfd
			popad
			jmp KO_SKILLBAR_MOUSE_INPUT_BLOCK
	}
}
bool __fastcall SkillKeyCheck(DWORD key)
{
	if (Engine->m_isGenieStatus) // Genie açýkken skill vurmamasý ayarlandý. // geniede skill attýrmak istiyorsan bunu kaldýr
		return true;

#if (HOOK_SOURCE_VERSION == 1098 || HOOK_SOURCE_VERSION == 1534)
	return key >= 8;
#endif

	return false;
}

void __declspec(naked) hkSkillbarKeyInput()
{
	__asm {
		pushad
		pushfd
		mov ecx, edi
		call SkillKeyCheck
		movzx eax, al
		test eax, eax
		jne engelle
		popfd
		popad
		jmp KO_SKILLBAR_KEY_INPUT_KEEP
		engelle :
		popfd
			popad
			jmp KO_SKILLBAR_KEY_INPUT_BLOCK
	}
}


DWORD KO_SET_EVENTNOTICE_POS = 0x0081BB92;
DWORD KO_SET_EVENTNOTICE_POS_ORG = 0;

void __fastcall Ayarlamk(DWORD efx)
{
	Engine->hkServerNotice = efx;
	koScreen = *(POINT*)KO_UI_SCREEN_SIZE;
	POINT pt;
	Engine->GetUiPos(efx, pt);

#if (HOOK_SOURCE_VERSION == 2369 || HOOK_SOURCE_VERSION == 1534)	// 1098 & 1534 vGenie pozisyon ayarlamasý Militarycamp ve premiumlar
	LONG isPost = 68;
	if (Engine->uiGenieSubPlug != NULL)
	{
		if (Engine->IsVisible(Engine->uiGenieSubPlug->TargetSam))
			isPost = 117;
		else if (Engine->IsVisible(Engine->uiGenieSubPlug->TargetTwo))
			isPost = 93;	
#else		// 2369 Genie pozisyon ayarlamasý Militarycamp ve premiumlar
	LONG isPost = 69;  
	if (Engine->uiGenieSubPlug != NULL)
	{
		if (Engine->IsVisible(Engine->uiGenieSubPlug->TargetSam))
			isPost = 117;   
		else if (Engine->IsVisible(Engine->uiGenieSubPlug->TargetTwo))
			isPost = 93;	
#endif
		if (Engine->IsVisible(Engine->hkCampAdress))
			(LONG)isPost += 18;

		pt.y = isPost;
		Engine->SetUIPos(efx, pt);
		if (Engine->IsVisible(Engine->hkCampAdress))
		{

			Engine->GetUiPos(Engine->hkCampAdress, pt);

			pt.y = LONG(isPost - 22);
			Engine->SetUIPos(Engine->hkCampAdress, pt);
		}
		POINT zs;
		zs.x = koScreen.x - 187;		//genie sað base konumu
		zs.y = 0;
		Engine->SetUIPos(Engine->uiGenieSubPlug->m_dVTableAddr, zs);
	}


	if (Engine->uiTaskbarMain) Engine->uiTaskbarMain->UpdatePosition();
}

void __declspec(naked) hkEventNotice()
{
	__asm {
		pushad
		pushfd
		call Ayarlamk
		popfd
		popad
		jmp KO_SET_EVENTNOTICE_POS_ORG
	}
}

DWORD GENIE_ORG = 0x40F7D0;
POINT ptgenie;
int geniex, geniey;

void __declspec(naked) hkGenieSetPos()
{
	__asm {
		pushad
		pushfd
	}
	koScreen = *(POINT*)KO_UI_SCREEN_SIZE;
#if (HOOK_SOURCE_VERSION == 1098 || HOOK_SOURCE_VERSION == 1534)
	geniex = koScreen.x - 170;
#else
	geniex = koScreen.x - 187;
#endif
	geniey = 0;
	__asm {
		popfd
		popad
		mov edx, geniex
		mov [esp + 4], edx
		mov edx, geniey
		mov [esp + 8], edx
		jmp GENIE_ORG
	}
}

#if (HOOK_SOURCE_VERSION == 1098)
DWORD Real_CharacterSelectLeft = 0x004DDAE0;					// 2369
DWORD Real_CharacterSelectLeft_ORG = 0x004DDAE0;				// 2369
DWORD Real_CharacterSelectRight = 0x004DDD80;					// 2369
DWORD Real_CharacterSelectRight_ORG = 0x004DDD80;				// 2369
void __fastcall Hook_CharacterSelectLeft(LPDWORD, LPVOID)
{
	if ((int)(*(DWORD*)(*(DWORD*)(0xF36908) + 0x48C)) != 1)		// 2369
	{
		__asm
		{
			MOV ECX, DWORD PTR DS : [0x00F36908]				// 2369
			CALL Real_CharacterSelectLeft_ORG
		}
	}
}

void __fastcall Hook_CharacterSelectRight(LPDWORD, LPVOID)
{
	if ((int)(*(DWORD*)(*(DWORD*)(0xF36908) + 0x48C)) != 2)  // 2369
	{
		if ((int)(*(DWORD*)(*(DWORD*)(0xF36908) + 0x48C)) == 0)  // 2369
		{
			(*(DWORD*)(*(DWORD*)(0xF36908) + 0x48C)) = 1;  // 2369
			__asm
			{
				MOV ECX, DWORD PTR DS : [0x00F36908]  // 2369
				CALL Real_CharacterSelectLeft_ORG
			}
		}
		else
		{
			__asm
			{
				MOV ECX, DWORD PTR DS : [0x00F36908]  // 2369
				CALL Real_CharacterSelectRight_ORG
			}
		}
	}
} 
#endif

typedef bool (WINAPI* tUIOnKeyPress)(int iKey);
bool WINAPI hkUIOnKeyPress(int iKey)
{
	static int i = 0;
	if (iKey == DIK_RETURN || iKey == DIK_NUMPADENTER) {
		i++;
		bool r = Engine->uiPowerUpStore  && Engine->IsVisible(Engine->uiPowerUpStore->m_dVTableAddr);
		Engine->uiPowerUpStore->ReceiveMessage((DWORD*)Engine->uiPowerUpStore->btn_search, UIMSG_BUTTON_CLICK);
		return r;
	}
	else if (iKey == DIK_ESCAPE)
	{
		if (Engine->uiTagChange != NULL && Engine->IsVisible(Engine->uiTagChange->m_dVTableAddr))
			Engine->uiTagChange->Close();
	}
	return false;
}

#define D3DX_PI    ((FLOAT)  3.141592654f)
#define D3DXToRadian( degree ) ((degree) * (D3DX_PI / 180.0f))
#define D3DXToDegree( radian ) ((radian) * (180.0f / D3DX_PI))

const DWORD GetMark = 0x0078FE08;						// Civciv Sembolü ekleme
constexpr DWORD secondadres2s = 0x0078FE11;				// Civciv Sembolü ekleme
const DWORD ar = 0x0078FC00;							// Civciv Sembolü ekleme
bool _fastcall civcivmi(DWORD nPlayerBase, DWORD eax)
{


	if (nPlayerBase == eax)
	{
		uint8 m_iLevel;
		DWORD chr;
		ReadProcessMemory(GetCurrentProcess(), (LPVOID)KO_PTR_CHR, &chr, sizeof(chr), 0);
		ReadProcessMemory(GetCurrentProcess(), (LPVOID)(chr + 0x74C), &m_iLevel, sizeof(m_iLevel), 0);

		if (m_iLevel < 30 && m_iLevel > 0)
			return true;
		else
			return false;
	}

	if (IsBadReadPtr((VOID*)nPlayerBase, sizeof(DWORD)))
		return false;
	uint8 val = *(uint8*)(nPlayerBase + 0x74C);

	if (val < 30 && val > 0)
		return true;

	return false;
}
void __declspec(naked) hkMarkPos()
{
	__asm
	{

		pushad
		pushfd
		mov ecx, esi
		mov edx, eax
		call civcivmi
		movzx eax, al
		test eax, eax
		jne devam
		popfd
		popad

		push 0x18
		push 0x00
		call ar
		jmp secondadres2s
		devam :
		popfd
			popad
			push  0x06
			push  0x00
			call ar
			jmp secondadres2s


	}
}
DWORD KO_MAGIC_SKILL_ANIMATION_ORG=0;
bool isAttack = true;
void __fastcall hkMagicAnimation()
{
	if (__genieAttackBlock || !(Engine->m_PlayerBase->isWarrior() || Engine->m_PlayerBase->isPriest()))
		return;

	if (DWORD target = Engine->GetTarget()) {
		isAttack = !isAttack;
		__asm {			// En alt atack buton iþlevi 01.02.2021
			push target
			push isAttack
			mov ecx, [KO_DLG]
			mov ecx, [ecx]
			mov eax, 0x00535550
			call eax
		}
	}
}
void __declspec(naked) GetAnimationSkill()
{
	__asm
	{
		pushad
		pushfd
		call hkMagicAnimation
		popfd
		popad
		jmp KO_MAGIC_SKILL_ANIMATION_ORG
	}

}
void __fastcall msga(DWORD esxx)
{

	DWORD tmp = 0;

	Engine->GetChildByID(esxx, std::string("str_mcount"), tmp);
	if (tmp == 0)
		return;
	std::string getString;
	getString = Engine->GetString(tmp).c_str();
	if (!getString.empty())
	{
		int getCount = std::stoi(getString.c_str());
		getCount++;
		Engine->SetString(tmp, string_format("%d", getCount));
	}
	else
		Engine->SetString(tmp, "1");


}

__declspec(naked) void hkPMMessage()
{

	__asm {

		pushad
		pushfd
		mov edx, esi
		call msga
		popfd
		popad
		jmp KO_WHISPER_GETMESSAGE_ORG

	}

}
void __fastcall msgaa(DWORD esxx)
{
	DWORD tmp = 0;

	Engine->GetChildByID(esxx, std::string("str_mcount"), tmp);

	if (tmp > 0)
	{

		Engine->SetString(tmp, "");
	}
}

void __fastcall setVisible(DWORD esxx)
{


	DWORD tmp = 0;


	DWORD tmp1 = 0;

	Engine->GetChildByID(esxx, std::string("base_default"), tmp1);
	*(DWORD*)(tmp1 + 0x10D) = false;
	Engine->GetChildByID(esxx, std::string("btn_view"), tmp1);


	*(DWORD*)(tmp1 + 0x10D) = false;
	Engine->GetChildByID(esxx, std::string("btn_duel"), tmp1);

	*(DWORD*)(tmp1 + 0x10D) = false;
	Engine->GetChildByID(esxx, std::string("btn_party"), tmp1);
	*(DWORD*)(tmp1 + 0x10D) = false;

	Engine->GetChildByID(esxx, std::string("pus_gift"), tmp1);
	*(DWORD*)(tmp1 + 0x10D) = false;


}
__declspec(naked) void hkPMCreateWindow()
{

	__asm {

		pushad
		pushfd
		cmp ebx, 0
		jne gizle
		mov ecx, ecx
		call setVisible
		popfd
		popad
		jmp KO_WHISPER_CREATE_ORG
		gizle :
		mov ecx, ecx
			call setVisible
			popfd
			popad
			jmp KO_WHISPER_CREATE_ORG

	}

}

__declspec(naked) void hkPMCloseWindow()
{

	__asm {

		pushad
		pushfd
		mov ecx, ecx
		call setVisible
		popfd
		popad
		jmp KO_WHISPER_OPEN_ORG

	}

}

__inline DWORD SaitX(DWORD ulBase)
{
	if (!IsBadReadPtr((VOID*)ulBase, sizeof(DWORD)))
	{
		return(*(DWORD*)(ulBase));
	}
	return 0;
}

DWORD pmBase;
void __fastcall setPMRecv(DWORD esxx, uint32_t dwMessage)
{

	if (dwMessage != 5)
		return;

	DWORD tmp = 0;


	std::string elementname = Engine->GetStringElementName(pmBase).c_str();
	DWORD btn_block, btn_view, btn_party, tmp_base;



	if (elementname == "btn_view")
	{
		DWORD tmp1 = 0;
		std::string sName;
		Engine->GetChildByID(esxx, std::string("exit_id"), tmp1);
		sName = Engine->GetString(tmp1).c_str();
		Engine->str_replace(sName, " ", "");
		Packet result(WIZ_USER_INFO, uint8(2));
		result.SByte();
		result << sName;
		Engine->Send(&result);
		__asm
		{
			mov ecx, esxx
			call setVisible

		}
		return;
	}
	else if (elementname == "pus_gift")
	{
		if (Engine->uiPowerUpStore != NULL) 
		{
			Engine->uiPowerUpStore->UpdateItemList(Engine->m_UiMgr->item_list, Engine->m_UiMgr->cat_list);
			Engine->uiPowerUpStore->UpdateRefundItemList(Engine->m_UiMgr->pusrefund_itemlist);
			Engine->uiPowerUpStore->Open();
		}
		__asm
		{
			mov ecx, esxx
			call setVisible

		}
		return;
	}
	else if (elementname == "btn_party")
	{
		DWORD tmp1 = 0;
		Engine->GetChildByID(esxx, std::string("exit_id"), tmp1);
		std::string name = Engine->GetString(tmp1).c_str();
		DWORD base = SaitX(SaitX(KO_DLG) + 0x238);
		DWORD partyCount = SaitX(base + 0x344);
		Packet result(WIZ_PARTY);
		Engine->StringHelper->Replace(name, xorstr(" "), xorstr(""));
		if (partyCount < 1)
			result << uint8(1) << name.c_str();
		else
			result << uint8(3) << name.c_str() << uint8(0);

		Engine->Send(&result);

		__asm
		{
			mov ecx, esxx
			call setVisible

		}
		return;
	}
	else if (elementname == "btn_info")
	{
		DWORD tmp1 = 0;
		bool isVisible = false;
		Engine->GetChildByID(esxx, std::string("base_default"), tmp1);
		*(DWORD*)(tmp1 + 0x10D) = Engine->IsVisible(tmp1) ? false : true;
		Engine->GetChildByID(esxx, std::string("btn_view"), tmp1);


		*(DWORD*)(tmp1 + 0x10D) = Engine->IsVisible(tmp1) ? false : true;
		Engine->GetChildByID(esxx, std::string("btn_duel"), tmp1);

		*(DWORD*)(tmp1 + 0x10D) = Engine->IsVisible(tmp1) ? false : true;
		Engine->GetChildByID(esxx, std::string("btn_party"), tmp1);
		*(DWORD*)(tmp1 + 0x10D) = Engine->IsVisible(tmp1) ? false : true;
		Engine->GetChildByID(esxx, std::string("pus_gift"), tmp1);
		*(DWORD*)(tmp1 + 0x10D) = Engine->IsVisible(tmp1) ? false : true;
	



	}





}
__declspec(naked) void hkPMCloseWindowRecv()
{

	__asm {

		pushad
		pushfd
		mov ecx, ecx
		mov edx, ebx
		MOV pmBase, esi
		call setPMRecv
		popfd
		popad
		jmp KO_WHISPER_RECV_ORG

	}

}

__declspec(naked) void hkPMView()
{

	__asm {

		pushad
		pushfd
		mov ecx, esi
		call msgaa
		popfd
		popad
		jmp KO_WHISPER_VIEWMESAAGE_ORG

	}

}

/*Merchant Eye / HidePlayer*/
DWORD nPlayerRender = 0x005999AE;
DWORD nPlayerRenderOrg;
DWORD nPlayerRender1 = 0x00599E66;

bool _fastcall kendimmi(DWORD nPlayerBase)
{

	if (!Engine->m_bHookStart)return false;
	if (Engine->Player.ZoneID == 21)
	{
		if (Engine->m_UiMgr->uiMerchantEye != NULL && Engine->m_UiMgr->uiMerchantEye->IsVisible())
		{
			if (Engine->m_UiMgr->uiMerchantEye->search_merchant->GetString().empty())
				return false;

			uint16 sSocket = *(uint16*)(nPlayerBase + KO_OFF_ID);
			if (sSocket == Engine->GetSocketID())
				return false;
			if (sSocket > 8000)
				return false;
			Engine->MerchantEyeLockData.lock();
			if (Engine->MerchantID.size() == 0)
			{
				Engine->MerchantEyeLockData.unlock();
				return true;
			}
			auto itr = Engine->MerchantID.find(sSocket);
			if (itr == Engine->MerchantID.end())
			{
				*(uint8*)(nPlayerBase + KO_WH) = 0;
				*(uint8*)(nPlayerBase + 0x7AC) = 3;
				Engine->MerchantEyeLockData.unlock();
				return true;
			}
			Engine->MerchantEyeLockData.unlock();
			*(uint8*)(nPlayerBase + KO_WH) = 1;
	
			return false;
		}
		else if (Engine->m_UiMgr->uiMerchantEye != NULL && !Engine->m_UiMgr->uiMerchantEye->IsVisible())
		{
			uint16 sSocket = *(uint16*)(nPlayerBase + KO_OFF_ID);
			if (sSocket == Engine->GetSocketID())
				return false;
			if (sSocket > 8000)
				return false;
			uint8 sAuthority = *(uint8*)(nPlayerBase + KO_WH);
			if (sAuthority == 0)
				*(uint8*)(nPlayerBase + KO_WH) = 1;
		}
	}
	if (!isHideUser)return false;

	uint16 mSocket = *(uint16*)(nPlayerBase + KO_OFF_ID);
	if (mSocket > 8000)
		return false;

	if (mSocket == Engine->GetSocketID())
		return false;


	return true;
}
__declspec(naked) void hkPlayerRender()
{
	__asm
	{
		pushad
		pushfd
		mov ecx, esi
		call kendimmi
		movzx eax, al
		test eax, eax
		jne devam
		popfd
		popad

		jmp nPlayerRenderOrg
		devam :
		popfd
			popad
			jmp nPlayerRender1

	}
}


/*Merchant Eye / HidePlayer*/

/**/
const DWORD READ_MERCHANT_ADRESS = 0x0062BE4C;
DWORD READ_MERCHANT_ADRESS_ORG = 0;
__declspec(naked) void ReadNearMerchant()
{
	__asm
	{
		pushad
		pushfd
		mov sSocketMerchant, eax
		popfd
		popad

		jmp READ_MERCHANT_ADRESS_ORG

	}
}
const DWORD READ_MERCHANT_SPECIAL_ADRESS = 0x0062C8C8;
DWORD READ_MERCHANT_ADRESS_SPECIAL_ORG = 0;
__declspec(naked) void ReadNearMerchantSpecial()
{
	__asm
	{
		pushad
		pushfd
		mov sSocketMerchantSpecial, eax
		popfd
		popad

		jmp READ_MERCHANT_ADRESS_SPECIAL_ORG

	}
}
/**/
/*Merchant Price*/
DWORD tooltipasm = 0x0086052B;
DWORD tooltipasm2 = 0x00860532;

bool _fastcall ustundeyimamk(DWORD ebx)
{


	if (Engine->IsVisible(Engine->merchantdisplaydata))
	{
		if (sSocketMerchant > 10000 || sSocketMerchant < 1)
			return 0;

		uint8 getdata = 0;
		bool yoklama = false;
		for (int i = 0; i < 4; i++)
		{
			if (Engine->IsIn(Engine->merchantdisplayItem[i], Engine->lastMousePos.x, Engine->lastMousePos.y))
			{
				getdata = i;
				yoklama = true;
				break;
			}
		}
		if (yoklama)
			return true;

	}
	else if (Engine->IsVisible(Engine->merchantdisplaySpecialdata))
	{
		if (sSocketMerchantSpecial > 10000 || sSocketMerchantSpecial < 1)
			return 0;

		uint8 getdata = 0;
		bool yoklama = false;
		for (int i = 0; i < 8; i++)
		{
			if (Engine->IsIn(Engine->merchantdisplaySpecialItem[i], Engine->lastMousePos.x, Engine->lastMousePos.y))
			{
				getdata = i;
				yoklama = true;
				break;
			}
		}
		if (yoklama)
			return true;

	}
	return false;
}

__declspec(naked) void SetPriceMerchant()
{
	__asm
	{
		pushad
		pushfd
		cmp edx, 1
		je pricegoster
		mov ecx, ebx
		call ustundeyimamk
		movzx eax, al
		test eax, eax
		jne pricegoster
		popfd
		popad
		push 0
		jmp tooltipasm2
		pricegoster :
		popfd
			popad
			push 1
			jmp tooltipasm2
	}
}
const DWORD KO_GENIE_MOVE_JMP = 0x009A9803;						// Genie range of activities ayarý buradan fixlendi.
DWORD KO_GENIE_MOVE_ORG;
__declspec(naked) void hkGenieMove()
{
	__asm
	{
		pushad
		pushfd

	}
	if (!__genieAttackMove)
	{
		__asm
		{
			popfd
			popad
			jmp KO_GENIE_MOVE_JMP

		}
	}
	__asm
	{
		popfd
		popad
		jmp KO_GENIE_MOVE_ORG
	}

}
DWORD KO_GENIE_MOVE_ORG2;								
const DWORD KO_GENIE_MOVE_RETURN = 0x009A7B2C;
__declspec(naked) void hkGenieMove2()
{
	__asm
	{
		pushad
		pushfd

	}
	if (!__genieAttackMove)
	{
		__asm
		{
			popfd
			popad
			MOV DWORD PTR DS : [0x009A9D86] , 0x8638000090903CEB
			jmp KO_GENIE_MOVE_RETURN

		}
	}
	__asm
	{
		popfd
		popad
		MOV DWORD PTR DS : [0x009A9D86] , 0x863800000107850F
		jmp KO_GENIE_MOVE_ORG2
	}

}
const DWORD KO_PARTY_REQUEST = 0x00524299;
DWORD KO_PARTY_REQUEST_KEEP = 0;
DWORD mLevel = 0;
DWORD tLevel = 0;
const DWORD KO_PARTY_REQUEST_JMP = 0x00524324;
void __declspec(naked) hkPartyRequest()
{
	__asm {
		pushad
		pushfd
		cmp ecx, 0xE1
		jb KO_PARTY_GO
		cmp ebx, 0xE1
		jae KO_PARTY_GO
		popfd
		popad
		jmp KO_PARTY_REQUEST_KEEP
		KO_PARTY_GO :
		popfd
			popad
			jmp KO_PARTY_REQUEST_JMP
	}
}

DWORD PearlEngine::GetTarget()
{
	DWORD KO_ADR = *(DWORD*)KO_PTR_CHR;
	uint16 targetID = *(DWORD*)(KO_ADR + KO_OFF_MOB);
	if (targetID >= NPC_BAND) {
		Func_GetObjectBase = (tGetObjectBase)KO_FMBS;
		return Func_GetObjectBase(*(DWORD*)KO_FLDB, targetID, 1);
	}
	else {
		Func_GetObjectBase = (tGetObjectBase)KO_FPBS;
		return Func_GetObjectBase(*(DWORD*)KO_FLDB, targetID, 1);
	}
	return NULL;
}
DWORD PearlEngine::GetTarget(uint16 targetID)
{
	DWORD KO_ADR = *(DWORD*)KO_PTR_CHR;
	uint16 byTargetID = targetID;
	if (byTargetID >= 10000) {
		Func_GetObjectBase = (tGetObjectBase)KO_FMBS;
		return Func_GetObjectBase(*(DWORD*)0x00F3E85C, byTargetID, 1);
	}
	else {
		Func_GetObjectBase = (tGetObjectBase)KO_FPBS;
		return Func_GetObjectBase(*(DWORD*)0x00F3E85C, byTargetID, 1);
	}
	return NULL;
}
const DWORD KO_PREMIUM_STRING = 0x0081FE27;
const DWORD KO_PREMIUM_STRING_CONTINUE = 0x0081FEC7;
const DWORD KO_PREMIUM_STRING_BACK = 0x0081FE2D;
const DWORD KO_PREMIUM_STRING_CALL = 0x00A14DA0;

void __declspec(naked) hkPremiumString()
{
	__asm
	{
		movzx eax, byte ptr[esp + 0x17]
		cmp eax,0x4
		je setPremium
		DEC EAX
		jmp KO_PREMIUM_STRING_BACK
		ret
		setPremium:
		lea edx, [esp + 0x00000F4]
		push edx
		push 0x5CA // 1482
		call KO_PREMIUM_STRING_CALL
		jmp KO_PREMIUM_STRING_CONTINUE
		ret
	}
}

DWORD WINAPI PearlEngine::EngineMain(PearlEngine * e)
{
	Func_GetObjectBase = (tGetObjectBase)KO_FMBS;

	memcpy((void*)0x00524295, (char*)"\x90\x90", 2);
	DetourFunction((PBYTE)KO_PREMIUM_STRING, (PBYTE)hkPremiumString);
	KO_PARTY_REQUEST_KEEP = (DWORD)DetourFunction((PBYTE)KO_PARTY_REQUEST, (PBYTE)hkPartyRequest);
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)SendTarama, (LPVOID)KO_SND_FNC, NULL, NULL);
	DetourFunction((PBYTE)tooltipasm, (PBYTE)SetPriceMerchant);
	//KO_GENIE_MOVE_ORG = (DWORD)DetourFunction((PBYTE)0x009A97F1, (PBYTE)hkGenieMove); // Atak Range Sýfýr olduðunda yürümesi engellendi.
	KO_GENIE_MOVE_ORG2 = (DWORD)DetourFunction((PBYTE)0x009A79AD, (PBYTE)hkGenieMove2); // Basic Attack off Genie
	READ_MERCHANT_ADRESS_ORG = (DWORD)DetourFunction((PBYTE)READ_MERCHANT_ADRESS, (PBYTE)ReadNearMerchant);
	READ_MERCHANT_ADRESS_SPECIAL_ORG = (DWORD)DetourFunction((PBYTE)READ_MERCHANT_SPECIAL_ADRESS, (PBYTE)ReadNearMerchantSpecial);
	KO_ZONE_LOAD_TERRAIN_ORG = (DWORD)DetourFunction((PBYTE)KO_ZONE_LOAD_TERRAIN, (PBYTE)hkLoadTerrainData);
	DetourFunction((PBYTE)KO_AUTO_ATTACK, (PBYTE)hkAttack);
	//rtOrg = (DWORD)DetourFunction((PBYTE)KO_UIF_SET_FONT_STRING, (PBYTE)hkSetFontString);		// string kalýnlaþtýrma.
	TICK_ORG = (DWORD)DetourFunction((PBYTE)KO_GAME_TICK, (PBYTE)hkTick);
	KO_MAGIC_SKILL_ANIMATION_ORG = (DWORD)DetourFunction((PBYTE)0x009A55E0, (PBYTE)GetAnimationSkill);
	(tUIOnKeyPress)DetourFunction((PBYTE)0x40F0F0, (PBYTE)hkUIOnKeyPress);
	*(float*)0xDE2B98 = 1.0f / 999.0f;																	// -------------------- fps limiti kaldýrma
	*(uint16*)0x007CBD55 = 0x4FEB;																		// -------------------- klan parasý client kontrol kapatma
	nPlayerRenderOrg = (DWORD)DetourFunction((PBYTE)nPlayerRender, (PBYTE)hkPlayerRender);				// -------------------- PlayerRender Oyuncu Gizleme / Merchant Eye
	DetourFunction((PBYTE)kontrolAdresi, (PBYTE)hkRank);												// -------------------- Zindanwar Rank 
	DetourFunction((PBYTE)Func_RangeInstance, (PBYTE)hkRangeInstance);									// -------------------- mage alan kýsýtlama  
	DetourFunction((PBYTE)Func_AreaMove, (PBYTE)hkAreaMove);											// -------------------- mage alan kýsýtlama
	oShellExecuteA = (tShellExecuteA)DetourFunction((PBYTE)ShellExecuteA, (PBYTE)hkShellExecuteA);
	DetourFunction((PBYTE)KO_FNC_END_GAME, (PBYTE)hkEndGame);											// -------------------- Oyundan Hýzlý çýkma
	memcpy((void*)0x0065F4DC, (char*)"\xC7\x81\x24\x01\x00\x00\x00\x00\x00\x00", 10);					// Pazar bekleme süresi kapatma
	memcpy((void*)0x43342B, (char*)"\xE9\x34\x1\x0\x0\x90", 6);											// Inventory Animasyon patlamasý fix
	*(uint64*)0x00565DCC = 0xDD9090000000E6E9;															// Oyuncularýn bir anda geç yüklenmesinin düzeltildiði yer. Örneðin warriorun descenti veya town atma
	*(uint16*)0x0060771A = 0x00003481;																	// Bdw'de ,Border'da Vip Key açýlmama sorunu fixlendi.
	KO_WHISPER_GETMESSAGE_ORG = (DWORD)DetourFunction((PBYTE)0x007247DB, (PBYTE)hkPMMessage);
	KO_WHISPER_VIEWMESAAGE_ORG = (DWORD)DetourFunction((PBYTE)0x00720DA3, (PBYTE)hkPMView);
	KO_WHISPER_OPEN_ORG = (DWORD)DetourFunction((PBYTE)0x00720DE3, (PBYTE)hkPMCloseWindow);
	KO_WHISPER_RECV_ORG = (DWORD)DetourFunction((PBYTE)0x007248F0, (PBYTE)hkPMCloseWindowRecv);
	KO_WHISPER_CREATE_ORG = (DWORD)DetourFunction((PBYTE)0x00723E90, (PBYTE)hkPMCreateWindow);
	*(uint32*)0x004E906F = 0x0000A6E9;																	// nick izin verme yeri Örn : ___ :: gibi deðiþik karakterlerde yazýlabilir.
	*(uint8*)0x00524283 = 0x5A;																			// 90 Level için Max Level diff kapatma
	//*(uint8*)0x009A8E67 = 0;																			// Geniede priest ölenin baþýna gitmesi fixlendi.
	*(uint16*)0x009A8E1E = 0x4FEB;																		// Geniede priest ölenin baþýna gitmesi fixlendi.
	//*(uint16*)0x009A8E34 = 0xEB39;																	// Geniede priest ölenin baþýna gitmesi fixlendi.
	*(uint16*)0x0067171D = 0x27EB;																		// Chaos bittikten sonra skillbar uifi çiftlenmesi fixlendi.
	//ASMPatch((DWORD)0x0509C91, (char*)"\xEB\x04\x90\x90\x90\x90\xD9\xEE", 8);							// Objeleri yakýndan seri taramasý fix.



	// -------------------- hp progress animasyon iptali
#if (HOOK_SOURCE_VERSION == 1098)
	DWORD flt = (DWORD)malloc(sizeof(float));
	DWORD flt2 = (DWORD)malloc(sizeof(float));
	*(float*)flt = 0.0f;
	*(float*)flt2 = 1000.0f;
	// -------------------- hpbar
	*(DWORD*)0x00833E38 = flt;
	*(DWORD*)0x00833E6A = flt;
	*(DWORD*)0x00833EA0 = flt;
	*(DWORD*)0x00833ED2 = flt;
	*(DWORD*)0x00833F04 = flt;
	*(DWORD*)0x00833E85 = flt;
	*(DWORD*)0x00833BE0 = flt;
	// -------------------- targetbar
	*(DWORD*)0x0084D444 = flt2;
	*(DWORD*)0x0084D44C = flt2;
	*(DWORD*)0x0084D333 = flt2;
	*(DWORD*)0x0084D33B = flt2;
	*(DWORD*)0x0084D3B3 = flt2;
	*(DWORD*)0x0084D3AB = flt2;
#endif
	//// -------------------- slide görüntü bugu fix
	DWORD slidebug = (DWORD)malloc(sizeof(float));
	*(float*)slidebug = 0.75f;
	*(DWORD*)(0x507621 + 2) = slidebug;

	//// -------------------- direction görüntü bugu fix
	DWORD directionbug = (DWORD)malloc(sizeof(float));
	//*(float*)directionbug = 0.35f;
	*(float*)directionbug = 1.5f;
	*(DWORD*)(0x50773B + 2) = directionbug;


	// -------------------- Discord yükleme
	if (e->dc = new Discord) {
		e->dc->Initialize();
		e->dc->Update();
	}

	// -------------------- Anti afk ve kafaya yazý yazma
	DetourFunction((PBYTE)KO_FNC_OBJECT_MOB_LOOP, (PBYTE)hkObjectMobLoop);
	KO_FNC_OBJECT_PLAYER_LOOP_ORG = (DWORD)DetourFunction((PBYTE)KO_FNC_OBJECT_PLAYER_LOOP, (PBYTE)hkObjectPlayerLoop);

	DetourFunction((PBYTE)fncScanZ, (PBYTE)hkZ);
	DetourFunction((PBYTE)fncScanB, (PBYTE)hkB);
	DetourFunction((PBYTE)KO_FNC_GENIE_SCAN, (PBYTE)hkGenieSelect);

	oGetChild = (tGetChild)DetourFunction((PBYTE)KO_GET_CHILD_BY_ID_FUNC, (PBYTE)GetChildByID_Hook);
	oSetVisible = (htSetVisible)DetourFunction((PBYTE)0x00411980, (PBYTE)SetVisible_Hook);
	oSetUIPos = (tSetUIPos)DetourFunction((PBYTE)0x0040F7D0, (PBYTE)SetUI_Hook);

	// -------------------- death animasyon speed 
	DWORD deathTimeAdress = 0x000534C0E;
	DWORD newSleep = (DWORD)malloc(sizeof(float));
	*(float*)newSleep = 0.0f;
	*(DWORD*)(deathTimeAdress + 2) = newSleep;

	// --------------------	Login Ýntro kapatma
	{
		uint8 wValues[] = { 0xA1, 0xFC, 0x68, 0xF3, 0x00 };
		for (auto i = 0; i < sizeof(wValues) / sizeof(wValues[0]); i++)
			*(BYTE*)(0x00A86D39 + i) = wValues[i];
	}
	// --------------------
	*(DWORD*)0xC7164C = (DWORD)hkDisableVisible;										// -------------------- scheculer Gizleme
	*(DWORD*)0xC704A4 = (DWORD)hkDisableVisible;										// -------------------- day price Gizleme
	*(DWORD*)0xC76574 = (DWORD)hkDisableVisible;										// -------------------- CUIItemtimeBtn Gizleme
	//DetourFunction((PBYTE)0x7D5DD0, (PBYTE)hkDisableVisible);							// -------------------- Quest View Ekraný Gizleme
	memcpy((void*)0x009015FD, (char*)"\x90\x90\x90\x90\x90\x90", 6);					// -------------------- TempItem Item List 1 Gizleme
	*(DWORD*)0xC76484 = (DWORD)hkDisableVisible;										// -------------------- TempItem Item List 2 Gizleme
	DetourFunction((PBYTE)KO_SKILLBAR_MOUSE_INPUT, (PBYTE)hkSkillbarMouseInput);		// geniemacro
	DetourFunction((PBYTE)KO_SKILLBAR_SHORTCUT_INPUT, (PBYTE)hkSkillbarShortcutInput);	// geniemacro
	KO_SKILLBAR_KEY_INPUT_KEEP = (DWORD)DetourFunction((PBYTE)KO_SKILLBAR_KEY_INPUT, (PBYTE)hkSkillbarKeyInput);

#if (HOOK_SOURCE_VERSION == 1098)
	DetourFunction((PBYTE)GetMark, (PBYTE)hkMarkPos);									// --------------------	Civciv iconu Çalýþtýran fonksiyon
	*(DWORD*)0x005687B0 = 0x90C3C031;													// -------------------- Karakter Space animasyonu iptal etme
	*(DWORD*)0x005687B0 = 0x90C3C031;													// -------------------- Karakter Space animasyonu iptal 
	*(DWORD*)0xC68ABC = (DWORD)hkDisableVisible;										// -------------------- timeleft Gizleme
	//*(DWORD*)0xC6B64C = (DWORD)hkOpenMiniMapfromGlobal;								// -------------------- globalmap to minimap
	*(DWORD*)0xC73494 = (DWORD)hkDisableVisible;										// -------------------- H command achieve Gizleme
	*(DWORD*)0xC6CEAC = (DWORD)hkDisableVisible;										// -------------------- QuestSeed  Gizleme
	*(DWORD*)0xC66234 = (DWORD)hkDisableVisible;										// -------------------- Nation Channel List Gizleme
	memcpy((void*)0x00788756, (char*)"\xEB\x14", 2); 									// -------------------- Training  Gizleme
	memcpy((void*)0x008DFE40, (char*)"\x31\xC0\xC2\x4\x0", 5); 							// -------------------- Commander Skill Bar Gizleme
	//memcpy((void*)0x0080BD70, (char*)"\C3\x90\x90", 3);  								// -------------------- Minimap Gizleme  -- Çalýþtýrýnca f10 seed helper game option 2 patlýyor.
	memcpy((void*)0x00594C4A, (char*)"\xE9\xBB\x01\x00\x00\x90", 6);					// -------------------- Noah knight Pelerin Kapatma & Gizleme

	// -------------------- minimap sabitleme
	memcpy((void*)0x008335D2, (char*)"\x90\x90", 2);
	memcpy((void*)0x00847BBF, (char*)"\x90\x90", 2);
	(DWORD)DetourFunction((PBYTE)0x0083361A, (PBYTE)hkMinimapPos);
	// --------------------

	// -------------------- noah knight level deðiþtirme
	uint8 noah_max_level = 30;
	*(BYTE*)0x54B259 = noah_max_level - 1;
	*(BYTE*)0x54B36B = noah_max_level;
	*(BYTE*)0x0054B4C4 = noah_max_level - 1;
	*(BYTE*)0x0054B4EA = noah_max_level;
	*(BYTE*)0x00546D6B = noah_max_level - 1;
	*(BYTE*)0x0053AC74 = noah_max_level - 1;
	// ----------------- 


	// ----------------- v1098 Character Select Uif Animasyonu ve set pos'un iptali 
	{
		uint8 wValues[6] = { 0xE9, 0x0E, 0x01, 0x00, 0x00, 0x90 };
		for (auto i = 0; i < sizeof(wValues) / sizeof(wValues[0]); i++)
			*(DWORD*)(0x004E9BEF + i) = wValues[i];
	}

	{
		uint8 wValues[6] = { 0xE9, 0x0B, 0x01, 0x00, 0x00, 0x90 };
		for (auto i = 0; i < sizeof(wValues) / sizeof(wValues[0]); i++)
			*(DWORD*)(0x004E9E0D + i) = wValues[i];
	}	
	// ----------------- 


	// -------------------- 1098 genie set pos
	//KO_SET_UI_ORG = (DWORD)DetourFunction((PBYTE)0x0040F7D0, (PBYTE)hkUiPos);
	*(DWORD*)0xC6F480 = (DWORD)hkGenieSetPos;
	KO_SET_EVENTNOTICE_POS_ORG = (DWORD)DetourFunction((PBYTE)KO_SET_EVENTNOTICE_POS, (PBYTE)hkEventNotice);
	uint8 channelY = 63;
	*(char*)0x0055B6D5 = channelY;
	*(char*)0x0050335C = channelY;
	*(char*)0x0050335B = 0x6A;
	// --------------------

	/* Tahtlý Karakter Kordinatlarý */
	Real_CharacterSelectLeft_ORG = (DWORD)DetourFunction((PBYTE)Real_CharacterSelectLeft, (PBYTE)Hook_CharacterSelectLeft);			// -------------------- Tahtlý karakter
	Real_CharacterSelectRight_ORG = (DWORD)DetourFunction((PBYTE)Real_CharacterSelectRight, (PBYTE)Hook_CharacterSelectRight);		// -------------------- Tahtlý karakter

	*(DWORD*)(0x004DDB79) = 0;
	{	// 1
		static float fX = -0.01f;
		static float fY = -1.16f;
		static float fZ = 2.6f;
		static float fR = 0.0f;

		*(DWORD*)(0x004E0638 + 2) = (DWORD)(&fX);
		*(DWORD*)(0x004E062E + 2) = (DWORD)(&fY);
		*(DWORD*)(0x004E0619 + 2) = (DWORD)(&fZ);
		*(DWORD*)(0x004E0643 + 2) = (DWORD)(&fR);
	}
	{	// 2
		static float fX = 1.73f;
		static float fY = -1.16f;
		static float fZ = 2.0f;
		static float fR = 0.68539819;

		*(DWORD*)(0x004E05D1 + 2) = (DWORD)(&fX);
		*(DWORD*)(0x004E05C7 + 2) = (DWORD)(&fY);
		*(DWORD*)(0x004E05B2 + 2) = (DWORD)(&fZ);
		*(DWORD*)(0x004E05DC + 2) = (DWORD)(&fR);
	}
	{	// 3
		static float fX = -1.82f;
		static float fY = -1.16f;
		static float fZ = 1.95f;
		static float fR = -0.68539819;

		*(DWORD*)(0x004E069B + 2) = (DWORD)(&fX);
		*(DWORD*)(0x004E0691 + 2) = (DWORD)(&fY);
		*(DWORD*)(0x004E067C + 2) = (DWORD)(&fZ);
		*(DWORD*)(0x004E06A6 + 2) = (DWORD)(&fR);
	}
	{	// C1
		static float fX = 0.0f;
		static float fY = -1.16f;
		static float fZ = 3.8f;
		static float fR = 0.78539819;

		*(DWORD*)(0x004DD0A5 + 2) = (DWORD)(&fX);
		*(DWORD*)(0x004DD0B9 + 2) = (DWORD)(&fY);
		*(DWORD*)(0x004DD0C6 + 2) = (DWORD)(&fZ);
		*(DWORD*)(0x004DCF46 + 2) = (DWORD)(&fX);
		*(DWORD*)(0x004DCF5B + 2) = (DWORD)(&fY);
		*(DWORD*)(0x004DCF69 + 2) = (DWORD)(&fZ);
		*(DWORD*)(0x004DCF96 + 2) = (DWORD)(&fR);
	}
	{	// C2
		static float fX = 2.60f;
		static float fY = -1.16f;
		static float fZ = 2.9f;
		static float fR = 1.28539819;

		*(DWORD*)(0x004DD0DD + 2) = (DWORD)(&fX);
		*(DWORD*)(0x004DD0F1 + 2) = (DWORD)(&fY);
		*(DWORD*)(0x004DD0FE + 2) = (DWORD)(&fZ);
		*(DWORD*)(0x004DCFB0 + 2) = (DWORD)(&fX);
		*(DWORD*)(0x004DCFBE + 2) = (DWORD)(&fY);
		*(DWORD*)(0x004DCFC8 + 2) = (DWORD)(&fZ);
		*(DWORD*)(0x004DCFEC + 2) = (DWORD)(&fR);
	}
	{	// C3
		static float fX = -2.7f;
		static float fY = -1.16f;
		static float fZ = 2.9f;
		static float fR = -0.18539819;

		*(DWORD*)(0x004DD115 + 2) = (DWORD)(&fX);
		*(DWORD*)(0x004DD129 + 2) = (DWORD)(&fY);
		*(DWORD*)(0x004DD136 + 2) = (DWORD)(&fZ);
		*(DWORD*)(0x004DD00F + 2) = (DWORD)(&fX);
		*(DWORD*)(0x004DD023 + 2) = (DWORD)(&fY);
		*(DWORD*)(0x004DD030 + 2) = (DWORD)(&fZ);
		*(DWORD*)(0x004DD057 + 2) = (DWORD)(&fR);
	}
	// ----------------- 
	/* Tahtlý Karakter Kordinatlarý */

#elif (HOOK_SOURCE_VERSION == 1534)
DetourFunction((PBYTE)GetMark, (PBYTE)hkMarkPos);									// --------------------	Civciv iconu Çalýþtýran fonksiyon
*(DWORD*)0x005687B0 = 0x90C3C031;													// -------------------- Karakter Space animasyonu iptal etme
*(DWORD*)0x005687B0 = 0x90C3C031;													// -------------------- Karakter Space animasyonu iptal 
*(DWORD*)0xC73494 = (DWORD)hkDisableVisible;										// -------------------- H command achieve Gizleme
*(DWORD*)0xC6CEAC = (DWORD)hkDisableVisible;										// -------------------- QuestSeed  Gizleme
*(DWORD*)0xC66234 = (DWORD)hkDisableVisible;										// -------------------- Nation Channel List Gizleme
memcpy((void*)0x00788756, (char*)"\xEB\x14", 2); 									// -------------------- Training  Gizleme
memcpy((void*)0x008DFE40, (char*)"\x31\xC0\xC2\x4\x0", 5); 							// -------------------- Commander Skill Bar Gizleme
memcpy((void*)0x00594C4A, (char*)"\xE9\xBB\x01\x00\x00\x90", 6);					// -------------------- Noah knight Pelerin Kapatma & Gizleme

// -------------------- noah knight level deðiþtirme
uint8 noah_max_level = 30;
*(BYTE*)0x54B259 = noah_max_level - 1;
*(BYTE*)0x54B36B = noah_max_level;
*(BYTE*)0x0054B4C4 = noah_max_level - 1;
*(BYTE*)0x0054B4EA = noah_max_level;
*(BYTE*)0x00546D6B = noah_max_level - 1;
*(BYTE*)0x0053AC74 = noah_max_level - 1;
// ----------------- 


// ----------------- v1098 Character Select Uif Animasyonu ve set pos'un iptali 
{
	uint8 wValues[6] = { 0xE9, 0x0E, 0x01, 0x00, 0x00, 0x90 };
	for (auto i = 0; i < sizeof(wValues) / sizeof(wValues[0]); i++)
		*(DWORD*)(0x004E9BEF + i) = wValues[i];
}

{
	uint8 wValues[6] = { 0xE9, 0x0B, 0x01, 0x00, 0x00, 0x90 };
	for (auto i = 0; i < sizeof(wValues) / sizeof(wValues[0]); i++)
		*(DWORD*)(0x004E9E0D + i) = wValues[i];
}
// ----------------- 

// -------------------- 1534 genie set pos
//KO_SET_UI_ORG = (DWORD)DetourFunction((PBYTE)0x0040F7D0, (PBYTE)hkUiPos);
* (DWORD*)0xC6F480 = (DWORD)hkGenieSetPos;
KO_SET_EVENTNOTICE_POS_ORG = (DWORD)DetourFunction((PBYTE)KO_SET_EVENTNOTICE_POS, (PBYTE)hkEventNotice);
uint8 channelY = 63;
*(char*)0x0055B6D5 = channelY;
*(char*)0x0050335C = channelY;
*(char*)0x0050335B = 0x6A;
// --------------------
#else
	//* (DWORD*)0xC6F480 = (DWORD)hkGenieSetPos; // genie set pos
	KO_SET_EVENTNOTICE_POS_ORG = (DWORD)DetourFunction((PBYTE)KO_SET_EVENTNOTICE_POS, (PBYTE)hkEventNotice);
#endif

#if ANTICHEAT_MODE == 1
	e->ScanThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)DriverScan, NULL, NULL, NULL);
	e->SuspendThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)SuspendCheck, e, NULL, NULL);
	e->TitleThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)TitleCheck, e, NULL, NULL);
#endif
	e->AliveThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)AliveSend, NULL, NULL, NULL);
	e->m_bCheckSleep = GetTickCount();
	e->m_LoginPlugInjected = true;
	
	while (true)
	{
		e->Update();
#if ANTICHEAT_MODE == 1
		if (IsDebuggerPresent())
			e->Shutdown(xorstr("Debugger"));  // Debugger Tespit Ediyor.

		if (e->IsHWBreakpointExists())
			e->Shutdown(xorstr("BreakPoint"));   // Breakpoint Tespit Ediyor.

		if (e->IsitaSandBox())
			e->Shutdown(xorstr("Sandbox"));	// Sanal Makineyi Tespit Ediyor.
#endif
		if (WaitForSingleObject(e->ScanThread, 1) == WAIT_OBJECT_0
			|| WaitForSingleObject(e->SuspendThread, 1) == WAIT_OBJECT_0
			|| WaitForSingleObject(e->LisansThread, 1) == WAIT_OBJECT_0)
			Engine->Shutdown(xorstr("All the pieces of the game can't work together."));

		if (e->uiNoticeWind && getMSTime() > e->m_recentdelete_time)
		{
			e->uiNoticeWind->Clear();
			e->m_recentdelete_time = getMSTime() + (30 * 1000) * 60;
		}

		Sleep(100);
	}
	return TRUE;
}

void PearlEngine::Update() 
{
	if (!m_bSelectedCharacter)
	{
		return;
	}

	if (!m_bGameStart)
	{
		return;
	}
	
	if (!isAlive)
		return;

	if (thisProc == NULL) 
		thisProc = GetCurrentProcess();
		
	if (KO_ADR == 0x0)
		KO_ADR = *(DWORD*)KO_PTR_CHR;

	uint8 Authority;
	Authority = *(uint8*)(KO_ADR + KO_WH);
	if (this->Player.Authority != Authority)
	{
		if (Authority == USER || Authority == GAMEMASTER || Authority == BANNED)
		{
			tmpGraphics = "";
			for (string gpu : Player.GPU)
				tmpGraphics += xorstr(" | ") + gpu;

			this->Player.Authority = (UserAuthority)Authority;

			Packet result(XSafe);
			result << uint8_t(XSafeOpCodes::AUTHINFO) << Authority << tmpGraphics << Player.CPU;
			this->Send(&result);
#if ANTICHEAT_MODE == 1
			if (Authority != 0 && Authority != 1 && Authority != 2 && Authority != 11 && Authority != 12 && Authority != 250 && Authority != 255)
			{
				Shutdown(xorstr("Game closed due to memory editing\n"));
			}
#endif
		}
	}

	if (m_PlayerBase == NULL)
		m_PlayerBase = new CPlayerBase();

	if (m_PlayerBase != NULL)
		m_PlayerBase->UpdateFromMemory();
}

DWORD Read4Bytes(DWORD paddy) 
{
	return *(int*)paddy;
}

void __stdcall LM_Shutdown(std::string log, std::string graphicCards, std::string processor) 
{
	if (isAlive && gameStarted) 
	{
		Packet result(XSafe);
		result << uint8_t(XSafeOpCodes::LOG) << log << graphicCards << processor;
		LM_Send(&result);
	}
	Shutdown(log);
}
const DWORD adrexxs = 0x0041FC40;
void __declspec(naked) SetTex(DWORD uif, const std::string& szFN)
{
	__asm {
		push ebp
		mov ebp, esp
		push[ebp + 0xC]
		mov ecx, [ebp + 8]
		call adrexxs
		pop ebp
		ret
	}
}

void PearlEngine::SetTexImage(DWORD uif, const std::string& szFN)
{
	if (szFN.empty()) { printf("%d \n", uif);return; }
	SetTex(uif, szFN);
}
void PearlEngine::Shutdown(std::string log) 
{
	tmpGraphics = "";
	for (string gpu : Player.GPU)
		tmpGraphics += xorstr(" | ") + gpu;
	LM_Shutdown(log, tmpGraphics, Player.CPU);
}

void LM_Send(Packet * pkt) 
{
	if (isAlive) 
	{
		uint8 opcode = pkt->GetOpcode();
		uint8 * out_stream = nullptr;
		uint16 len = (uint16)(pkt->size() + 1);

		out_stream = new uint8[len];
		out_stream[0] = pkt->GetOpcode();

		if (pkt->size() > 0)
			memcpy(&out_stream[1], pkt->contents(), pkt->size());

		BYTE* ptrPacket = out_stream;
		SIZE_T tsize = len;

		__asm
		{
			mov ecx, KO_PTR_PKT
			mov ecx, DWORD ptr ds : [ecx]
			push tsize
			push ptrPacket
			call KO_SND_FNC
		}

		delete[] out_stream;
	}
}

bool dirExists(const std::string& dirName_in)
{
	DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;

	return false;
}

std::string getHWID() 
{
	HW_PROFILE_INFO hwProfileInfo;
	GetCurrentHwProfile(&hwProfileInfo);
	string hwidWString = hwProfileInfo.szHwProfileGuid;
	string hwid(hwidWString.begin(), hwidWString.end());
	return hwid;
}

void GetMacHash(uint16& MACOffset1, uint16& MACOffset2);
uint16 GetCPUHash();

uint16 GetVolumeHash()
{
	DWORD SerialNum = 0;
	GetVolumeInformationA(xorstr("C:\\"), NULL, 0, &SerialNum, NULL, NULL, NULL, 0);
	uint16 nHash = (uint16)((SerialNum + (SerialNum >> 16)) & 0xFFFF);
	return nHash;
}

int64 GetHardwareID()
{
	uint16 MACData1, MACData2 = 0;
	GetMacHash(MACData1, MACData2);
	return _atoi64(string_format(xorstr("%d%d%d"), MACData1, MACData2, GetCPUHash()).c_str());/*, GetVolumeHash()).c_str());*/ //09.10.2020 GetVolumaHash Kapatýldý
	//Deger Uzun gelirse sýkýntý yaratabilirdi oyuncu oyuna giremeyebilirdi o sebeple kapatýldý
}

uint16 HashMacAddress(PIP_ADAPTER_INFO info)
{
	uint16 nHash = 0;
	for (uint32 i = 0; i < info->AddressLength; i++)
		nHash += (info->Address[i] << ((i & 1) * 8));
	return nHash;
}

void GetMacHash(uint16& MACOffset1, uint16& MACOffset2)
{
	IP_ADAPTER_INFO AdapterInfo[32];
	DWORD dwBufLen = sizeof(AdapterInfo);

	DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
	if (dwStatus != ERROR_SUCCESS)
		return;

	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
	MACOffset1 = HashMacAddress(pAdapterInfo);
	if (pAdapterInfo->Next)
		MACOffset2 = HashMacAddress(pAdapterInfo->Next);

	if (MACOffset1 > MACOffset2)
	{
		uint16 TempOffset = MACOffset2;
		MACOffset2 = MACOffset1;
		MACOffset1 = TempOffset;
	}
}

uint16 GetCPUHash()
{
	int CPUInfo[4] = { 0, 0, 0, 0 };
	__cpuid(CPUInfo, 0);
	uint16 nHash = 0;
	uint16* nPointer = (uint16*)(&CPUInfo[0]);
	for (uint16 i = 0; i < 8; i++)
		nHash += nPointer[i];

	return nHash;
}



void StartHook()
{

	if (Engine->m_bHookStart) return;
	Engine->m_bHookStart = true;

	//DWORD genieadresi = 0x009A9FC0; //  2369

	//DWORD genieNewCD = (DWORD)malloc(sizeof(float));
	//ASMPatch(genieadresi, (char*)"\xD8\x05", 2);
	//*(float*)genieNewCD = 0.3f;

	//*(DWORD*)(genieadresi + 2) = genieNewCD;

	Engine->InitCameraZoom();
	Engine->m_SettingsMgr = new CSettingsManager();
	Engine->m_SettingsMgr->Init();
	Engine->m_PlayerBase = new CPlayerBase();
	if (Engine->m_PlayerBase->isWarrior())
		isRFastAttack = true;
	//else if (Engine->m_PlayerBase->isRogue())
	//	genieNewCD = 1.6f;
	//else if (Engine->m_PlayerBase->isMage())
	//	genieNewCD = 1.6f;
	else if (Engine->m_PlayerBase->isPriest())
		isRFastAttack = true;

	Engine->varius = Engine->rdword(KO_DLG, std::vector<int>{ 0x208});

#if (HOOK_SOURCE_VERSION == 1098)
	//Plugins
	Engine->hkRightClickExchange = new RightClickExchange();
	Engine->uiState = new CUIStatePlug();
	Engine->uiTargetBar = new CUITargetBarPlug();
	Engine->uiTaskbarMain = new CUITaskbarMainPlug();
	Engine->uiTaskbarSub = new CUITaskbarSubPlug();
	Engine->uiClanWindowPlug = new CUIClanWindowPlug();
	Engine->uiPlayerRank = new HookPlayerRank();
	Engine->uiGenieSubPlug = new CUIGenieSubPlug();
	Engine->uiSeedHelperPlug = new CUISeedHelperPlug();
	Engine->uiTradePrice = new CUITradePricePlug();
	Engine->uiTradeInventory = new CUITradeInventoryPlug();
	Engine->uiTradeItemDisplay = new CUITradeItemDiplayPlug();
	Engine->uiTradeItemDisplaySpecial = new CUITradeItemDisplaySpecialPlug();
	Engine->m_MerchantMgr = new CMerchantManager();
	Engine->uiInventoryPlug = new CUIInventoryPlug();
	Engine->uiScoreBoard = new CUIScoreBoardPlug();
	Engine->uiGenieMain = new CUIGenieMain;
	Engine->uiHPBarPlug = new CUIHPBarPlug();
	Engine->uiHpMenuPlug = new CUIHPBarMenuPlug();
	Engine->uiTopRightNewPlug = new CUITopRightNewPlug();
	Engine->uiMinimapPlug = new CUIMinimapPlug;
	Engine->uiCindirella = new CUICindirellaPlug();
	Engine->uiCollection = new CUICollectionRacePlug();
	Engine->uiNoticeWind = new CUIMoticeMsgWnd();
	Engine->uiWheel = new CUIWheelOfFun();
	Engine->uiSearchMonster = new CUISearchMonsterDropPlug();
	Engine->uiDropResult = new CDropResult();
	Engine->uiTagChange = new CTagChange();
	Engine->uiSkillPage = new HookSkillPage();
	Engine->uiQuestComplated = new CUIQuestComplated();
	Engine->uiLottery = new CUILotteryEventPlug();
	Engine->uiEventShowList = new CEventShowListPlug();
	Engine->uiMerchantList = new CUIMerchantListPlug();
	Engine->uiInformationWind = new CUIInformationWindPlug();
	Engine->uiChatBarPlug = new CUIChatBarPlug();
	Engine->uiQuestPage = new HookQuestPage();
	Engine->uiSealToolTip = new CSealToolTip();
	Engine->uiPowerUpStore = new CUIPowerUpStore();
	Engine->uiPartyBBS = new CUIPartyBBSPlug();
	Engine->uiAccountRegisterPlug = new CUIAccountRegister();
	Engine->uiPieceChangePlug = new CUIPieceChangePlug();
	Engine->uiSupport = new CUISupportReportPlug();	
	//Engine->uiEventNotice = new CUIEventNoticePlug(); //kullanýlmýyor
	//Engine->uiAnvil = new CUIAnvil();
	//Engine->uiToolTip = new CUITooltip(); //kullanýlmýyor	
	//Engine->uiSchedularPlug = new CUISchedular;
	//Engine->uiMiniMenuPlug = new CUIMiniMenuPlug();
#else
	Engine->uiAccountRegisterPlug = new CUIAccountRegister();
	Engine->uiQuestComplated = new CUIQuestComplated();
	Engine->hkRightClickExchange = new RightClickExchange();
	Engine->uiState = new CUIStatePlug();
	Engine->uiTopRightNewPlug = new CUITopRightNewPlug();
	Engine->uiMinimapPlug = new CUIMinimapPlug;
	Engine->uiDropResult = new CDropResult();
	Engine->uiInformationWind = new CUIInformationWindPlug();
	Engine->uiNoticeWind = new CUIMoticeMsgWnd();
	Engine->uiGenieMain = new CUIGenieMain;
	Engine->uiGenieSubPlug = new CUIGenieSubPlug();
	Engine->uiEventShowList = new CEventShowListPlug();
	Engine->uiTargetBar = new CUITargetBarPlug();
	Engine->uiTaskbarMain = new CUITaskbarMainPlug();
	Engine->uiTaskbarSub = new CUITaskbarSubPlug();
	Engine->uiQuestPage = new HookQuestPage();
	Engine->uiClanWindowPlug = new CUIClanWindowPlug();
	Engine->uiMerchantList = new CUIMerchantListPlug();
	Engine->uiTradeInventory = new CUITradeInventoryPlug();
	Engine->uiTradeItemDisplay = new CUITradeItemDiplayPlug();
	Engine->uiTradeItemDisplaySpecial = new CUITradeItemDisplaySpecialPlug();
	Engine->m_MerchantMgr = new CMerchantManager();
	Engine->uiTradePrice = new CUITradePricePlug();
	Engine->uiPlayerRank = new HookPlayerRank();
	Engine->uiInventoryPlug = new CUIInventoryPlug();
	Engine->uiSeedHelperPlug = new CUISeedHelperPlug();
	Engine->uiHPBarPlug = new CUIHPBarPlug();
	Engine->uiScoreBoard = new CUIScoreBoardPlug();
	Engine->uiTagChange = new CTagChange();
	Engine->uiSkillPage = new HookSkillPage();
	Engine->uiSearchMonster = new CUISearchMonsterDropPlug();
	Engine->uiCindirella = new CUICindirellaPlug();
	Engine->uiPowerUpStore = new CUIPowerUpStore();
	Engine->uiCollection = new CUICollectionRacePlug();
	Engine->uiLottery = new CUILotteryEventPlug();
	Engine->uiHpMenuPlug = new CUIHPBarMenuPlug();
	Engine->uiSealToolTip = new CSealToolTip();
	Engine->uiWheel = new CUIWheelOfFun();
	Engine->uiPieceChangePlug = new CUIPieceChangePlug();
	Engine->uiSupport = new CUISupportReportPlug();
#endif

	Engine->m_bHookStart = true;
	Engine->m_UiMgr->OpenMerchantNotice();
#if (HOOK_SOURCE_VERSION == 1098)
	//uint8 Nation = *(uint8*)(*(DWORD*)KO_PTR_CHR + KO_OFF_NATION);
	//if (Nation == 1)
	//{
	//	POINT koScreen = *(POINT*)0x00DE297C;
	//	RECT region = Engine->GetUiRegion(Engine->uiChatBarPlug->m_dVTableAddr);  // chatting Box ve information Box'ýn pozisyonun konumlandýrýldýðý yer
	//	POINT orta;
	//	Engine->lastkoScreen = koScreen;
	//	Engine->curlastkoScreen = koScreen;
	//	LONG w = (region.right - region.left);
	//	orta.x = (koScreen.x / 2) - 513;
	//	orta.y = region.top + 1;
	//	Engine->SetUIPos(Engine->uiChatBarPlug->m_dVTableAddr, orta);
	//	orta.x = (koScreen.x / 2) + 214;
	//	Engine->SetUIPos(Engine->uiInformationWind->m_dVTableAddr, orta);
	//}
	//if (Nation == 2)
	//{
	//	POINT koScreen = *(POINT*)0x00DE297C;
	//	RECT region = Engine->GetUiRegion(Engine->uiChatBarPlug->m_dVTableAddr);  // chatting Box ve information Box'ýn pozisyonun konumlandýrýldýðý yer
	//	POINT orta;
	//	Engine->lastkoScreen = koScreen;
	//	Engine->curlastkoScreen = koScreen;
	//	LONG w = (region.right - region.left);
	//	orta.x = (koScreen.x / 2) - 513;
	//	orta.y = region.top - 12;
	//	Engine->SetUIPos(Engine->uiChatBarPlug->m_dVTableAddr, orta);
	//	orta.x = (koScreen.x / 2) + 215;
	//	Engine->SetUIPos(Engine->uiInformationWind->m_dVTableAddr, orta);
	//}
#endif
	//Engine->uiCollection->OpenCR();
	//Engine->uiCollection->SetVisible(false);

	if (true)
	{
		vector<int>offsets;
		offsets.push_back(0x2C8);
		offsets.push_back(0);

		Engine->merchantdisplaydata = Engine->rdword(KO_DLG, offsets);
		for (int i = 0; i < 4; i++)
		{
			std::string find = string_format("%d", i);
			Engine->GetChildByID(Engine->merchantdisplaydata, find, Engine->merchantdisplayItem[i]);
		}
	}
	if (true)
	{
		vector<int>offsets;
		offsets.push_back(0x2CC);
		offsets.push_back(0);

		Engine->merchantdisplaySpecialdata = Engine->rdword(KO_DLG, offsets);
		for (int i = 0; i < 8; i++)
		{
			std::string find = string_format("%d", i);
			Engine->GetChildByID(Engine->merchantdisplaySpecialdata, find, Engine->merchantdisplaySpecialItem[i]);
		}
	}
}

enum CollectionRaceType
{
	CRStartRecv,
	CRGetActiveTime,
	CRMonsterKilled,
	CRUpdateCounter,
	CRFinishRecv,
	CRHide
};

void CollectionRaceStart(Packet & pkt);
void CollectionRaceGetActiveTime(Packet & pkt);
void CollectionRaceHide();
void CollectionRaceDead(Packet & pkt);
void CollectionRaceUpdateCounter(Packet & pkt);
void CollectionRaceFinish();

void HandleCollectionRace(Packet & pkt)
{
	uint8 OpCode;
	pkt >> OpCode;

	switch (OpCode)
	{
	case CollectionRaceType::CRStartRecv:
		CollectionRaceStart(pkt);
		break;
	case CollectionRaceType::CRGetActiveTime:
		CollectionRaceGetActiveTime(pkt);
		break;
	case CollectionRaceType::CRMonsterKilled:
		CollectionRaceDead(pkt);
		break;
	case CollectionRaceType::CRUpdateCounter:
		CollectionRaceUpdateCounter(pkt);
		break;
	case CollectionRaceType::CRFinishRecv:
		CollectionRaceFinish();
		break;
	case CollectionRaceType::CRHide:
		CollectionRaceHide();
		break;
	default:
		break;
	}
}


void CollectionRaceStart(Packet & pkt)
{
	//Engine->uiCollection = NULL;
	pkt.DByte();
	Engine->IsCRActive = true;
	for (int i = 0; i < 3; i++) 
	{
		pkt >> Engine->pCollectionRaceEvent.sSid[i] >> Engine->pCollectionRaceEvent.killTarget[i];
		Engine->pCollectionRaceEvent.killCount[i] = 0;
	}

	for (int i = 0; i < 3; i++)
		pkt >> Engine->pCollectionRaceEvent.RewardsItemID[i] >> Engine->pCollectionRaceEvent.RewardsItemCount[i] >> Engine->pCollectionRaceEvent.RewardsItemRate[i];

	pkt >> Engine->pCollectionRaceEvent.EventTimer >> Engine->pCollectionRaceEvent.m_iCompleteUserCount >> Engine->pCollectionRaceEvent.TotalParticipantUser >> Engine->pCollectionRaceEvent.Nation;
	pkt >> Engine->pCollectionRaceEvent.EventName >> Engine->pCollectionRaceEvent.sZone;

	//if (Engine->uiCollection == NULL)
	//{
	//	Engine->uiCollection = new CUICollectionRacePlug();

	//}
	if(Engine->uiCollection!=NULL)
	Engine->uiCollection->Start();
	//if (Engine->m_UiMgr != NULL && Engine->uiCollection != NULL)
	//{
	//	Engine->uiCollection->Open();
	//	Engine->uiCollection->Start();
	//}
}

void CollectionRaceGetActiveTime(Packet & pkt)
{
	if (Engine->uiCollection == NULL)
		return;

	Engine->IsCRActive = true;

	

	uint16 userkillcount = 0;
	for (int i = 0; i < 3; i++) 
	{
		pkt >> Engine->pCollectionRaceEvent.sSid[i] >> Engine->pCollectionRaceEvent.killTarget[i] >> userkillcount;
		Engine->pCollectionRaceEvent.killCount[i] = userkillcount;
	}

	for (int i = 0; i < 3; i++)
		pkt >> Engine->pCollectionRaceEvent.RewardsItemID[i] >> Engine->pCollectionRaceEvent.RewardsItemCount[i] >> Engine->pCollectionRaceEvent.RewardsItemRate[i];

	pkt >> Engine->pCollectionRaceEvent.EventTimer >> Engine->pCollectionRaceEvent.m_iCompleteUserCount >> Engine->pCollectionRaceEvent.TotalParticipantUser >> Engine->pCollectionRaceEvent.Nation;
	pkt >> Engine->pCollectionRaceEvent.EventName;

	if (Engine->m_UiMgr != NULL && Engine->uiCollection != NULL)
	{
		Engine->uiCollection->Open();
		Engine->uiCollection->Start();
		Engine->uiCollection->Update();
	}
}

void CollectionRaceHide()
{
	if (Engine->m_UiMgr != NULL && Engine->uiCollection != NULL)
		Engine->uiCollection->Close();
}

void CollectionRaceDead(Packet & pkt)
{


	if (Engine->m_UiMgr == NULL || Engine->uiCollection == NULL)
		return;



	uint16 MonsterProtoID, UserKillCount[3];
	pkt >> MonsterProtoID >> UserKillCount[0] >> UserKillCount[1] >> UserKillCount[2];
	
	if (Engine->IsCRActive)
	{

		bool updateRequired = false;
		for (int i = 0; i < 3; i++) 
		{
			if (Engine->pCollectionRaceEvent.sSid[i] == MonsterProtoID)
			{
				if (Engine->pCollectionRaceEvent.killCount[i] < Engine->pCollectionRaceEvent.killTarget[i])
				{
					Engine->pCollectionRaceEvent.killCount[i] = UserKillCount[i];
					updateRequired = true;
				}
			}
		}

		if (updateRequired && Engine->m_UiMgr != NULL)
			Engine->uiCollection->Update();
	}
}

void CollectionRaceUpdateCounter(Packet & pkt)
{
	if (!Engine->IsCRActive || Engine->m_UiMgr == NULL || Engine->uiCollection == NULL)
		return;

	pkt >> Engine->pCollectionRaceEvent.m_iCompleteUserCount >> Engine->pCollectionRaceEvent.TotalParticipantUser;

	Engine->uiCollection->UpdateFinishUserCounter();
}

void CollectionRaceFinish()
{
	if (Engine->m_UiMgr != NULL && Engine->uiCollection != NULL)
		Engine->SetVisible(Engine->uiCollection->m_dVTableAddr,false);

	if (Engine->uiTopRightNewPlug != NULL)
		Engine->uiTopRightNewPlug->HideCollectionRaceButton();

	for (int i = 0; i < 3; i++)
	{
		Engine->pCollectionRaceEvent.sSid[i] = 0;
		Engine->pCollectionRaceEvent.killTarget[i] = 0;
		Engine->pCollectionRaceEvent.killCount[i] = 0;
		Engine->pCollectionRaceEvent.RewardsItemID[i] = 0;
		Engine->pCollectionRaceEvent.RewardsItemCount[i] = 0;
		Engine->pCollectionRaceEvent.RewardsItemRate[i] = 0;
	}

	Engine->pCollectionRaceEvent.EventTimer = 0;
	Engine->pCollectionRaceEvent.m_iCompleteUserCount = Engine->pCollectionRaceEvent.TotalParticipantUser = 0;
	Engine->pCollectionRaceEvent.EventName = "Collection Race Event";

	Engine->IsCRActive = false;

	//Engine->uiCollection = NULL;
}

void HandleCastleSiegeWarTimer(Packet & pkt) 
{
	pkt >> Engine->pCastleSiegeWar.SiegeType >> Engine->pCastleSiegeWar.StartTime >> Engine->pCastleSiegeWar.RemainingTime;
	pkt.SByte();
	pkt >> Engine->pCastleSiegeWar.SiegeOwnerClanName;

	//if (Engine->m_UiMgr->uiCastleSiegeWarSituation == NULL)
	if (Engine->uiTopRightNewPlug->BasladimiBaslamadimi == false)
	{
		Engine->uiTopRightNewPlug->BasladimiBaslamadimi = true;

		if (Engine->pCastleSiegeWar.SiegeType == 0x01)
			Engine->isSiegeWarStart = true;
		
		if (Engine->pCastleSiegeWar.SiegeType == 0x02) 
		{
			Engine->isSiegeWarStart = false;
			Engine->isSiegeWarActive = true;
		}
	
		//test

	}

	if (Engine->pCastleSiegeWar.SiegeType == 0x00)
	{
		Engine->isSiegeWarStart = false;
		Engine->isSiegeWarActive = false;

		//test sildirme
		Engine->uiTopRightNewPlug->BasladimiBaslamadimi = false;
		if (Engine->uiTopRightNewPlug != NULL)
		{

		}
		//test

		
	}


}

bool voiceStarted = false;

void ZindanWarFlag(Packet &pkt) {
	std::string ename, kname;
	uint16 ekillcount, kkillcount; uint32 remtime;
	pkt >> ename >> ekillcount >> kname >> kkillcount >> remtime;
	if (ename.empty() || kname.empty() || !Engine->uiScoreBoard) return;

	if (!Engine->uiScoreBoard->m_Timer) Engine->uiScoreBoard->m_Timer = new CTimer(false);

	Engine->SetString(Engine->uiScoreBoard->Ename, ename);
	Engine->SetString(Engine->uiScoreBoard->Kname, kname);
	Engine->SetString(Engine->uiScoreBoard->EScore, std::to_string(ekillcount));
	Engine->SetString(Engine->uiScoreBoard->KScore, std::to_string(kkillcount));
	Engine->SetString(Engine->uiScoreBoard->RemTime, std::to_string(remtime));
	Engine->pzindanwar.start = true;
	Engine->pzindanwar.remtime = remtime;
	if (Engine->m_PlayerBase->m_iZoneID == 91) Engine->SetVisible(Engine->uiScoreBoard->m_dVTableAddr, true);
}

void ZindanWarLogOut() {
	if (Engine->uiScoreBoard) Engine->SetVisible(Engine->uiScoreBoard->m_dVTableAddr, false);
	Engine->pzindanwar.Init();
}

void ZindanWarUpdateScore(Packet &pkt) {
	uint8 nation; uint16 killcount;
	pkt >> nation >> killcount;
	if ((nation != 1 && nation != 2) || !Engine->uiScoreBoard) return;
	if (nation == 1) Engine->SetString(Engine->uiScoreBoard->KScore, std::to_string(killcount));
	else Engine->SetString(Engine->uiScoreBoard->EScore, std::to_string(killcount));
}

enum class ZindanOp { flagsend, updatescore, logout};
void HandleZindanWar(Packet &pkt) {
	uint8 subcode = pkt.read<uint8>();
	switch ((ZindanOp)subcode)
	{
	case ZindanOp::flagsend:
		ZindanWarFlag(pkt);
		break;
	case ZindanOp::updatescore:
		ZindanWarUpdateScore(pkt);
		break;
	case ZindanOp::logout:
		ZindanWarLogOut();
		break;
	}
}

int GetMsSinceMidnightGmt(std::chrono::system_clock::time_point tpNow) {
	time_t tnow = std::chrono::system_clock::to_time_t(tpNow);
	tm * tmDate = std::localtime(&tnow);
	std::chrono::duration<int> durTimezone; // 28800 for HKT
	// because mktime assumes local timezone, we shift the time now to GMT, then fid mid
	time_t tmid = std::chrono::system_clock::to_time_t(tpNow - durTimezone);
	tm * tmMid = std::localtime(&tmid);
	tmMid->tm_hour = 0;
	tmMid->tm_min = 0;
	tmMid->tm_sec = 0;
	auto tpMid = std::chrono::system_clock::from_time_t(std::mktime(tmMid));
	auto durSince = tpNow - durTimezone - tpMid;
	auto durMs = std::chrono::duration_cast<std::chrono::milliseconds>(durSince);
	return durMs.count();
}


void HandleDailyQuests(Packet &pkt)
{
	uint8 subcode = 0;
	pkt >> subcode;

	switch (subcode)
	{
		case (uint8)DailyQuestOp::sendlist:
		{
			uint16 isize = 0;
			pkt >> isize;
			for (uint8 i = 0; i < isize; i++)
			{
				DailyQuest* quest = new DailyQuest();
				pkt >> quest->index >> quest->questtype >> quest->killtype;
				for (uint8 j = 0; j < 4; j++)
				{
					pkt >> quest->Mob[j] >> quest->Reward[j] >> quest->Count[j];
				}
				pkt >> quest->KillTarget >> quest->ZoneID >> quest->replaytime >>  quest->MinLevel >> quest->MaxLevel;


				if (Engine->uiQuestPage)
					Engine->uiQuestPage->kcbq_quests.push_back(quest);

				//kcbq_quests.push_back(quest);
			}
			//if (Engine->uiQuestPage) Engine->m_UiMgr->kcbq_quests = Engine->uiQuestPage->kcbq_quests;
		}
		break;
		case (uint8)DailyQuestOp::userinfo:
		{
			uint16 isize = 0;
			pkt >> isize;
			for (uint8 i = 0; i < isize; i++)
			{
				uint8 id = 0;
				pkt >> id;

				uint8 status; uint16 mycount; uint32 time;
				pkt >> status >> mycount >> time;


				if (Engine->uiQuestPage)
				{
					for (uint32 i = 0; i < Engine->uiQuestPage->kcbq_quests.size(); i++)
					{
						if (Engine->uiQuestPage->kcbq_quests[i]->index == id)
						{
							Engine->uiQuestPage->kcbq_quests[i]->Status = status;
							Engine->uiQuestPage->kcbq_quests[i]->MyCount = mycount;
							Engine->uiQuestPage->kcbq_quests[i]->remtime = time;
						}
					}
				}

			}
			if (Engine->uiQuestPage)
			{
				Engine->uiQuestPage->InitQuests();
			}

		}
		break;

		case (uint8)DailyQuestOp::killupdate:
		{
			uint8 questID;
			pkt >> questID;
			uint16 mobid;
			pkt >> mobid;
			if (Engine->uiQuestPage) Engine->uiQuestPage->KillTrigger(questID,mobid);
		}
		break;

	}
}

void HandleEventShowList(Packet& pkt) {
	if (!Engine || !Engine->m_UiMgr) return;

	Engine->m_UiMgr->m_eventshowlist.clear();

	pkt >> Engine->serverhour >> Engine->serverminute >> Engine->serversecond;

	uint16 sz = 0; pkt >> sz;
	if (!sz) return;

	for (int i = 0; i < sz; i++) {
		std::string name = "", time = ""; uint32 hour, minute;
		pkt >> name >> hour >> minute;
		if (name.empty()) continue;
		
		if (hour < 10) time.append(string_format("0%s:", to_string(hour).c_str()));
		else time.append(string_format("%s:", to_string(hour).c_str()));
		if (minute < 10) time.append(string_format("0%s", to_string(minute).c_str()));
		else time.append(string_format("%s", to_string(minute).c_str()));
		Engine->m_UiMgr->m_eventshowlist.push_back(EventShow(name, time, hour, minute));
	}

	std::sort(Engine->m_UiMgr->m_eventshowlist.begin(),
		Engine->m_UiMgr->m_eventshowlist.end(), [](auto const& a, auto const& b) { return a.hour < b.hour && a.minute < b.minute; });

	int i = 0;
}

void HandleCindirella(Packet& pkt) {

	if (!Engine->uiCindirella)
		return;

	uint8 opcode = pkt.read<uint8>();
	switch ((cindopcode)opcode)
	{
	case cindopcode::selectclass:
	{
		uint8 result = pkt.read<uint8>();
		switch ((cindopcode)result)
		{
		case cindopcode::success:
		{
			uint8 index = pkt.read<uint8>();
			if (index > 4) return;

			Engine->uiCindirella->CharacterSelect(index);
		}
		break;
		case cindopcode::timewait:
		{
			uint32 remtime = pkt.read<uint32>();
			if (remtime)
				Engine->WriteInfoMessage((char*)string_format(xorstr("You have %d seconds to change it again"), remtime).c_str(), D3DCOLOR_ARGB(255, 255, 0, 255));
		}
		break;
		case cindopcode::notchange:
			Engine->WriteInfoMessage((char*)xorstr("You have not made any changes to your selections."), D3DCOLOR_ARGB(255, 255, 0, 255));
			break;
		case cindopcode::alreadyclass:
			Engine->WriteInfoMessage((char*)xorstr("same class please choose a different class."), D3DCOLOR_ARGB(255, 255, 0, 255));
			break;
		}
	}
		break;
	case cindopcode::nationchange:
	{
		uint8 result = pkt.read<uint8>();
		switch ((cindopcode)result)
		{
		case cindopcode::alreadynation:
			Engine->WriteInfoMessage((char*)xorstr("same nation please choose a different nation."), D3DCOLOR_ARGB(255, 255, 0, 255));
			break;
		case cindopcode::success:
			Engine->WriteInfoMessage((char*)xorstr("Nation selections applied successfully."), D3DCOLOR_ARGB(255, 255, 64, 89));
			break;
		case cindopcode::timewait:
		{
			uint32 remtime = pkt.read<uint32>();
			if (remtime)
				Engine->WriteInfoMessage((char*)string_format(xorstr("You have %d seconds to change it again"), remtime).c_str(), D3DCOLOR_ARGB(255, 255, 0, 255));
		}
		break;
		}
	}
	break;
	case cindopcode::joinevent:
	{
		uint32 tm = 0;
		pkt >> Engine->m_cindirellaInfo.prepare >> Engine->m_cindirellaInfo.index;
		pkt >> tm >> Engine->m_cindirellaInfo.mykill_count >> Engine->m_cindirellaInfo.mydead_count;
		pkt >> Engine->m_cindirellaInfo.k_kill_count >> Engine->m_cindirellaInfo.e_kill_count;

		Engine->uiCindirella->ison = true;
		Engine->m_cindirellaInfo.remainingtime = tm;

		Engine->uiCindirella->ShowPanel();
		if (!Engine->m_cindirellaInfo.prepare) Engine->uiCindirella->Start();

	}
	break;
	case cindopcode::starting:
	{
		uint32 tm = 0;
		pkt >> tm;

		if (!Engine->uiCindirella->ison) Engine->uiCindirella->ison = true;
		Engine->m_cindirellaInfo.remainingtime = tm;
		Engine->uiCindirella->Start();
		Engine->uiCindirella->Open();
	}
	break;
	case cindopcode::finish:
	{

		Engine->uiCindirella->Reset();
		Engine->uiCindirella->Close();

		if (Engine->uiTopRightNewPlug && Engine->IsVisible(Engine->uiTopRightNewPlug->m_cindirella))
			Engine->SetVisible(Engine->uiTopRightNewPlug->m_cindirella, false);
	}
	break;
	case cindopcode::updatekda:
	{
		uint8 set = pkt.read<uint8>();
		if (!set)
			pkt >> Engine->m_cindirellaInfo.mykill_count >> Engine->m_cindirellaInfo.mydead_count;
		else
			pkt >> Engine->m_cindirellaInfo.e_kill_count >> Engine->m_cindirellaInfo.k_kill_count;

		if (!Engine->uiCindirella || !Engine->uiCindirella->status || Engine->uiCindirella->prepare)
			return;

		Engine->uiCindirella->UpdateKDA();
	}
	break;
	default:
		break;
	}
}

void HandleCSW(Packet& pkt)
{
	enum class csw_flags
	{
		timer,
		finish
	};

	if (Engine->m_UiMgr->uiCSW == NULL)
	{
		std::string name = "";
		name = Engine->dcpUIF(xorstr("ISTIRAP\\re_csw_timer.istirap"));

		if (name.empty())
			return;

		Engine->m_UiMgr->uiCSW = new CUICastleSiegeWar();
		Engine->m_UiMgr->uiCSW->Init(Engine->m_UiMgr);
		Engine->m_UiMgr->uiCSW->LoadFromFile(Engine->m_BasePath + name.c_str(), N3FORMAT_VER_1068);
		Engine->m_UiMgr->AddChild(Engine->m_UiMgr->uiCSW);
		remove(name.c_str());
	}

	if (!Engine->m_UiMgr->uiCSW)
		return;

	uint8 subCode = pkt.read<uint8>();
	switch ((csw_flags)subCode)
	{
	case csw_flags::timer:
	{
		uint8 OpStatus;
		uint32 remtime, war_time; std::string knights_name;
		pkt.SByte();
		pkt >> remtime >> knights_name >> OpStatus >> war_time;
		if (remtime)
			Engine->m_UiMgr->uiCSW->setOptions(remtime, knights_name, OpStatus, war_time);
	}
	break;
	case csw_flags::finish:
	{
		if (Engine->m_UiMgr->uiCSW)
			Engine->m_UiMgr->uiCSW->Close();
	}
	default:
		break;
	}


	/*uint8 subcode = pkt.read<uint8>();
	if (subcode == 0)
	{
		if (Engine->m_UiMgr->uiCSW)
			Engine->m_UiMgr->uiCSW->setOptions(true);
		return;
	}

	uint32 remtime= 0;
	std::string owner_name = "";
	pkt >> remtime >> owner_name;

	

	Engine->m_UiMgr->uiCSW->setOptions(false, remtime, owner_name);*/
}

void HandleJuraidEvent(Packet& pkt)
{
	uint16 karus, elmos, remtime;
	pkt >> karus >> elmos >> remtime;
	if (remtime) Engine->OpenjuraidScreen(true, karus, elmos, remtime);
}

void HandlePerks(Packet& pkt)
{
	uint8 subcode = pkt.read<uint8>();
	switch ((perksSub)subcode)
	{
	case perksSub::info:
	{
		pkt >> Engine->pPerks.remPerk >> Engine->pPerks.perkCoins;
		uint16 size = 0;
		pkt >> size;

		for (int i = 0; i < size; i++) {
			_PERKS* p = new _PERKS();
			pkt.DByte();
			pkt >> p->pIndex >> p->status >> p->perkCount >> p->maxPerkCount >> p->strDescp >> p->percentage;
			auto itr = Engine->m_PerksArray.find(p->pIndex);
			if (itr == Engine->m_PerksArray.end())
				Engine->m_PerksArray.insert(std::make_pair(p->pIndex, p));
			else
				delete p;
		}

		for (int i = 0; i < PERK_COUNT; i++)
			pkt >> Engine->pPerks.perkType[i];
	}
	break;
	case perksSub::perkPlus:
	{
		if (Engine->uiState)
			Engine->uiState->perkPlus(pkt);
	}
	break;
	case perksSub::perkReset:
	{
		if (Engine->uiState)
			Engine->uiState->resetPerkPoint(pkt);
	}
	break;
	case perksSub::perkUseItem:
	{
		if (Engine->uiState)
			Engine->uiState->perkUseItem(pkt);
	}
	break;
	default:
		break;
	}
}

void HandleChestBlockItem()
{
	if(Engine->uiSearchMonster)Engine->uiSearchMonster->Reset();
}

void __cdecl XSafeHandlePacket(Packet pkt)
{
	uint8_t SubOpCode;
	pkt >> SubOpCode;
	switch (SubOpCode)
	{
	case XSafeOpCodes::CHEST_BLOCKITEM:
		HandleChestBlockItem();
		break;
	case XSafeOpCodes::PERKS:
		HandlePerks(pkt);
		break;
	case XSafeOpCodes::CSW:
		HandleCSW(pkt);
		break;
	case XSafeOpCodes::JURAID:
		HandleJuraidEvent(pkt);
		break;
	case XSafeOpCodes::CINDIRELLA:
		HandleCindirella(pkt);
		break;
	case XSafeOpCodes::RESET:
		Engine->Player.moneyreq = pkt.read<uint32>();
		break;
	case XSafeOpCodes::EventShowList:
		HandleEventShowList(pkt);
		break;
	case XSafeOpCodes::DAIL_QUESTS_1098:
		HandleDailyQuests(pkt);
		break;
	case XSafeOpCodes::ZindanWar:	// Zindanwar score board
		HandleZindanWar(pkt);
		break;
	case XSafeOpCodes::TOPLEFT:
		if (Engine->uiHpMenuPlug != NULL)
			Engine->uiHpMenuPlug->UpdateHpMenuVisible(pkt);
		break;
	case XSafeOpCodes::BANSYSTEM:
		banSystem->SetBan();
		break;
	case XSafeOpCodes::RIGHT_CLICK_EXCHANGE:
		if (Engine->hkRightClickExchange != NULL)Engine->hkRightClickExchange->RightClickExchangePacket(pkt);
		break;
	case XSafeOpCodes::MESSAGE2:
	{
		uint8 type, time;
		std::string title, message;
		uint32 colour;
		pkt.DByte();
		pkt >> type;

		if (type == (uint8)infomessageop::box) {
			pkt >> title >> message >> time;
			if (title.empty() || message.empty())
				return;

			Engine->m_UiMgr->ShowMessageBox(title, message, MsgBoxTypes::Ok);
		}
		else if (type == (uint8)infomessageop::infobar) {
			pkt >> message >> colour;
			if (message.empty())
				return;

			Engine->WriteInfoMessageExt((char*)string_format(message).c_str(), colour);
		}
	}
	break;
	case XSafeOpCodes::LOTTERY:
	{
		uint8 subcode;
		pkt >> subcode;
		if (Engine->uiLottery == NULL)
		{
			return;

		}

		if (Engine->uiLottery != NULL)
		{
			if (subcode == 1) // baþlatma
			{
				if (!Engine->IsVisible(Engine->uiLottery->m_dVTableAddr))
				{
					Engine->uiLottery->OpenLottery();
				}

				Engine->SetState(Engine->uiLottery->btn_join,UI_STATE_BUTTON_NORMAL);

				uint32 reqItems[5] = { 0 }, reqItemsCount[5] = { 0 }, rewardItems[4] = { 0 };
				uint32 remainingTime = 0;
				uint32 partLimit = 0;
				uint32 participant = 0;
				uint32 ticketID = 0;

				for (int i = 0; i < 5; i++)
					pkt >> reqItems[i] >> reqItemsCount[i];
				for (int i = 0; i < 4; i++)
					pkt >> rewardItems[i];

				pkt >> partLimit >> remainingTime >> participant >> ticketID;

				Engine->lotteryEvent.limit = partLimit;
				Engine->lotteryEvent.participant = participant;
				Engine->lotteryEvent.remainingTime = remainingTime;

				float per = 0.0f;
				if (Engine->lotteryEvent.participant > 0)
					per = (1 / Engine->lotteryEvent.participant) * 100;
				else
					per = 100.0f;

				//Engine->SetString(Engine->uiLottery->str_change,string_format(xorstr("%.3f%%"), per));
				Engine->SetString(Engine->uiLottery->str_participant,string_format(xorstr(" %d / %d"), participant, partLimit));
				if (ticketID == 0)
					Engine->SetString(Engine->uiLottery->str_ticket,xorstr("-"));
				else {
					Engine->SetString(Engine->uiLottery->str_ticket,string_format(xorstr("#%d"), ticketID));
					Engine->SetState(Engine->uiLottery->btn_join,UI_STATE_BUTTON_DISABLE);
				}

				/*for (int i = 0; i < 5; i++) */
				Engine->SetString(Engine->uiLottery->ticket_buy_count,string_format(xorstr("%d"), reqItemsCount[0]));
				Engine->uiLottery->Update(reqItems, reqItemsCount, rewardItems);
			}
			else if (subcode == 2) // katýlýmcý güncellemesi
			{
				Engine->lotteryEvent.participant++;
				float per = 0.0F;
				per = (1 / Engine->lotteryEvent.participant) * 100;
				//Engine->SetString(Engine->uiLottery->str_change,string_format(xorstr("%.3f%%"), per));
				Engine->SetString(Engine->uiLottery->str_participant,string_format(xorstr(" %d / %d"), Engine->lotteryEvent.participant, Engine->lotteryEvent.limit));
			}
			else if (subcode == 3) // join
			{
				uint8 ok = 0;
				uint32 ticket = 0;
				pkt >> ok;
				if (ok == 1)
				{
					pkt >> ticket;
					//Engine->m_UiMgr->uiLottery->btn_join->SetState(UI_STATE_BUTTON_DISABLE);
					Engine->SetString(Engine->uiLottery->str_ticket,string_format(xorstr("#%d"), ticket));
					Engine->m_UiMgr->ShowMessageBox(xorstr("Lottey Event"), "You have successfully participated in the event.", MsgBoxTypes::Ok);
				}
				else {
					std::string msg = "";
					pkt >> msg;
					Engine->m_UiMgr->ShowMessageBox(xorstr("Error"), msg, MsgBoxTypes::Ok);
				}
			}
			else if (subcode == 4) // end
			{
				uint32 reqItems[5] = { 0 }, reqItemsCount[5] = { 0 }, rewardItems[4] = { 0 };
				Engine->lotteryEvent.limit = 0;
				Engine->lotteryEvent.participant = 0;
				Engine->lotteryEvent.remainingTime = 0;

				/*for (int i = 0; i < 5;i++) */
				Engine->SetString(Engine->uiLottery->ticket_buy_count,xorstr("-"));
				//Engine->SetString(Engine->uiLottery->str_change,xorstr("0%"));
				Engine->SetString(Engine->uiLottery->str_participant,string_format(xorstr(" %d / %d"), 0, 0));
				Engine->SetString(Engine->uiLottery->str_ticket,xorstr("-"));
				Engine->SetState(Engine->uiLottery->btn_join,UI_STATE_BUTTON_NORMAL);
				Engine->uiLottery->Update(reqItems, reqItemsCount, rewardItems, true);

				if (Engine->IsVisible(Engine->uiLottery->m_dVTableAddr))
				{
					Engine->uiLottery->Close();
				}

				if (Engine->uiTopRightNewPlug != NULL)
				{
					if (!Engine->IsVisible(Engine->uiTopRightNewPlug->m_lottery) && Engine->uiTopRightNewPlug->m_lottery)
					{
						Engine->SetVisible(Engine->uiTopRightNewPlug->m_lottery, false);
						Engine->SetState(Engine->uiTopRightNewPlug->m_lottery, UI_STATE_BUTTON_DISABLE);
					}
				}
			}
		}
	}
	break;
	case XSafeOpCodes::ERRORMSG: //30.08.2020
	{
		uint8 subcode;
		pkt >> subcode;

		if (subcode == 1) // join
		{
			uint8 ok = 0;
			uint32 ticket = 0;
			pkt >> ok;
			if (ok == 1)
			{
				std::string msgtitle = "", msg = "";
				pkt.DByte();
				pkt >> msgtitle >> msg;
				Engine->m_UiMgr->ShowMessageBox(msgtitle, msg, MsgBoxTypes::Ok);
			}
		}
	}
	break;
	case XSafeOpCodes::RankSymbol:
	{
		if (Engine->uiPlayerRank == NULL)
			break;
		Engine->uiPlayerRank->ResetBase();
		uint8 Nation, Rank;
		uint16 sCount = 0;
		pkt >> Nation >> sCount;
		if (Nation == 2) {

			for (int i = 0; i < sCount; i++) {
				uint8 byRank;
				pkt >> byRank;
				if (byRank > 0) {
					printf("symbol_us\\s_0_%d_0.dxt \n", byRank);
					Engine->SetTexImage(Engine->uiPlayerRank->nHumanBase[i], string_format("symbol_us\\s_0_%d_0.dxt", byRank));
					Engine->SetVisible(Engine->uiPlayerRank->nHumanBase[i], true);
					Engine->SetVisible(Engine->uiPlayerRank->nHumanBaseEffeckt[i], true);
				}

			}
		}
		pkt >> Nation >> sCount;
		if (Nation == 1) {

			for (int i = 0; i < sCount; i++) {
				uint8 byRank;
				pkt >> byRank;
				if (byRank > 0) {
					printf("symbol_us\\s_0_%d_0.dxt \n", byRank);
					Engine->SetTexImage(Engine->uiPlayerRank->nKarusBase[i], string_format("symbol_us\\s_0_%d_0.dxt", byRank));

					Engine->SetVisible(Engine->uiPlayerRank->nKarusBase[i], true);
					Engine->SetVisible(Engine->uiPlayerRank->nKarusBaseEffeckt[i], true);
				}

			}
		}

	}
	break;
	case XSafeOpCodes::MESSAGE:
	{
		string title, message;
		pkt >> title >> message;

		if (Engine->m_UiMgr != NULL && !title.empty() && !message.empty() && title.length() <= 30 && message.length() <= 255)
			Engine->m_UiMgr->ShowMessageBox(title, message, MsgBoxTypes::Ok);
	}
	break;
	case XSafeOpCodes::MERCHANTLIST:
		Engine->m_UiMgr->ShowMerchantList(pkt);
		break;
	case XSafeOpCodes::AUTODROP:
	{
		uint8 type;
		uint32 nItemID;
		pkt >> type >> nItemID;

		if (!Engine->m_bGameStart || Engine->m_UiMgr == NULL)
			return;

		if (type == 0) // auto mining
		{
			if (Engine->uiDropResult != NULL) {
				Engine->m_UiMgr->ShowDropResult();
				Engine->SetVisible(Engine->uiDropResult->m_dVTableAddr,false);
			}

			Engine->uiDropResult->SetTitle("Auto Mining");
			Engine->uiDropResult->AddItem(nItemID);
			
			TABLE_ITEM_BASIC * item = Engine->tblMgr->getItemData(nItemID);
			if (item != nullptr)
				Engine->WriteInfoMessage((char*)string_format(xorstr("[Mining] you received %s"), item->strName.c_str()).c_str(), 0xFFF2AB);

			Engine->uiTopRightNewPlug->DropResultStatus(true);
		}
		else if (type == 1) // auto fishing
		{
			/*if (Engine->m_UiMgr->uiDropResult == NULL) {
				Engine->m_UiMgr->ShowDropResult();
				Engine->m_UiMgr->uiDropResult->SetVisible(false);
			}*/

			Engine->uiDropResult->SetTitle("Auto Fishing");
			Engine->uiDropResult->AddItem(nItemID);
			
			TABLE_ITEM_BASIC * item = Engine->tblMgr->getItemData(nItemID);
			if (item != nullptr)
				Engine->WriteInfoMessage((char*)string_format(xorstr("[Fishing] you received %s"), item->strName.c_str()).c_str(), 0xFFF2AB);

			Engine->uiTopRightNewPlug->DropResultStatus(true);
		}
		else if (type == 2)
		{
			if (Engine->uiDropResult != NULL)
			{
				Engine->uiDropResult->Close();
				Engine->uiTopRightNewPlug->DropResultStatus(false);
			}
		}
		else if (type == 3) // wheel of fun
		{
			if (Engine->uiWheel != NULL)
			{
				Engine->uiWheel->StopChallange();

				Engine->uiWheel->GiveItemImage(nItemID);
			
			}
		}
		else
		{
			if (Engine->uiDropResult != NULL)
			{
				Engine->uiDropResult->Close();
				Engine->uiTopRightNewPlug->DropResultStatus(false);
			}
			Engine->WriteInfoMessageExt((char*)xorstr("[Mining/Fishing] stopped"), 0xFFF2AB);
		}
	}
	break;
	case XSafeOpCodes::KCUPDATE:
	{
		uint32 cash,tl_balance;
		pkt >> cash >> tl_balance;

		Engine->Player.KnightCash = cash;
		Engine->Player.TlBalance = tl_balance;
		if (Engine->uiState != NULL)
		{
			Engine->uiState->m_iBalance = tl_balance;
			Engine->uiState->m_iCash = cash;
			Engine->uiState->UpdateUI();
		}

		if (Engine->uiTradeInventory != NULL)
			Engine->uiTradeInventory->UpdateTotal(cash);

		if (Engine->uiPowerUpStore != NULL)
			Engine->uiPowerUpStore ->UpdateCash(cash, tl_balance);
	}
	case XSafeOpCodes::USERDATA:
	{
		if (Engine->m_PlayerBase != NULL)
			Engine->m_PlayerBase->Update(pkt);
	}
	break;
	case XSafeOpCodes::CR:
	{
		HandleCollectionRace(pkt);
	}
	break;
	case XSafeOpCodes::DROP_REQUEST:
	case XSafeOpCodes::DROP_LIST:
		Engine->m_UiMgr->ShowDropList(pkt);
		break;
	case XSafeOpCodes::CASHCHANGE:
	{
		uint32 cash,tlbalance;
		pkt >> cash >> tlbalance;

		if (Engine->uiState != NULL)
		{
			if (cash > Engine->uiState->m_iCash)
				Engine->WriteInfoMessageExt((char*)string_format(xorstr("Received %d knight cash."), cash - Engine->uiState->m_iCash).c_str(), 0xFFF2AB);
			else if (cash < Engine->uiState->m_iCash)
				Engine->WriteInfoMessageExt((char*)string_format(xorstr("Lost %d knight cash."), Engine->uiState->m_iCash - cash).c_str(), 0xFF0000);

			if (tlbalance > Engine->uiState->m_iBalance)
				Engine->WriteInfoMessageExt((char*)string_format(xorstr("Received %d tl balance."), tlbalance - Engine->uiState->m_iBalance).c_str(), 0xFFF2AB);
			else if (tlbalance < Engine->uiState->m_iBalance)
				Engine->WriteInfoMessageExt((char*)string_format(xorstr("Lost %d tl balance."), Engine->uiState->m_iBalance - tlbalance).c_str(), 0xFF0000);
		}

		Engine->Player.KnightCash = cash;
		Engine->Player.TlBalance = tlbalance;
		if (Engine->uiState != NULL)
		{
			Engine->uiState->m_iCash = cash;
			Engine->uiState->m_iBalance = tlbalance;
			Engine->uiState->UpdateUI();
		}
		if (Engine->uiWheel != NULL)
		Engine->SetString(Engine->uiWheel->KnightCash,string_format("Knight Cash : %d", cash).c_str());
		
		if (Engine->uiTradeInventory != NULL)
			Engine->uiTradeInventory->UpdateTotal(cash);

		if (Engine->uiPowerUpStore  != NULL)
			Engine->uiPowerUpStore ->UpdateCash(cash, tlbalance);
	}
	break;
	case XSafeOpCodes::INFOMESSAGE:
	{
		string message;
		pkt >> message;
		Engine->WriteInfoMessageExt((char*)message.c_str(), 0xFFF2AB);
	}
	break;
	case XSafeOpCodes::PUS:
	{
		vector<PUSItem> PusItems;
		
		uint32_t PusItemCount = pkt.read<uint32_t>();
		for (uint32 i = 0; i < PusItemCount; i++)
		{
			uint32 sID, sItemID, sPrice, sQuantitiy;
			uint8 sCategory,isTlBalance;

		
			pkt >> sID >> sItemID >> sPrice >> sCategory >> sQuantitiy >> isTlBalance;
		
			PusItems.push_back(PUSItem(sID, sItemID, sPrice,isTlBalance, (PUS_CATEGORY)sCategory, sQuantitiy));
		}

		pusINIT = true;
		Engine->m_UiMgr->item_list = PusItems;
	}
	break;
	case XSafeOpCodes::PusCat:
	{
		vector<PusCategory> Puscats;

		uint32_t CategortyCount = pkt.read<uint32_t>();
		for (uint32 i = 0; i < CategortyCount; i++)
		{
			uint32 sID;
			uint8 isVisible;
			std::string catename;
			pkt >> sID >> catename >> isVisible;
			Puscats.push_back(PusCategory(sID, catename, isVisible));
		}

		pusINIT = true;
		Engine->m_UiMgr->cat_list = Puscats;
	}
	break;
	case XSafeOpCodes::PROCINFO:
	{
		uint16 toWHO = 0;
		pkt >> toWHO;
		LM_SendProcess(toWHO);
	}
	break;
	case XSafeOpCodes::SkillQuest:
	{
		uint16 sQuestID;

		pkt >> sQuestID;
		std::map<uint32, CSpell>::iterator itr;
		for (itr = Engine->skillmap.begin(); itr != Engine->skillmap.end(); itr++)
		{
			if (CSpell* spell = GetSkillBase(itr->second.dwID))
				if(spell->iIDK2==390)
				spell->iIDK2 = 0;
		}
	}
	break;
	case XSafeOpCodes::OPEN:
	{
		string address;
		pkt >> address;
		ShellExecuteA(NULL, xorstr("open"), address.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}
	break;
	case XSafeOpCodes::UIINFO:
	{
		if (!Engine->m_bHookStart) {
			StartHook();
		}

		uint32 cash, moneyreq,tlbalance;
		int64 exp,maxexp; 
		uint16 dd, axe, sword, mace, spear, bow, jamadar;
		pkt >> cash >> tlbalance >> dd >> axe >> sword >> mace >> spear >> bow >> jamadar >> moneyreq >> exp >> maxexp;

#if (HOOK_SOURCE_VERSION == 1098)
		if (Engine->uiHPBarPlug != NULL) {
			Engine->uiHPBarPlug->MaxExp = maxexp;
			Engine->uiHPBarPlug->ExpChange(exp, maxexp);
		}
#endif

		Engine->Player.KnightCash = cash;
		Engine->Player.TlBalance = tlbalance;
		Engine->moneyReq = moneyreq;

		if (Engine->m_PlayerBase == NULL)
			Engine->m_PlayerBase = new CPlayerBase();

		uint16 m_iSocketID;

		std::string m_strCharacterName;
		short m_sClass;
		uint8 m_iRace;
		uint8 m_iLevel;

		uint8 m_iStr;
		uint8 m_iHp;
		uint8 m_iDex;
		uint8 m_iInt;
		uint8 m_iMp;

		pkt >> m_iSocketID;
		pkt.DByte();
		pkt >> m_strCharacterName >> m_sClass >> m_iRace >> m_iLevel
			>> m_iStr >> m_iHp >> m_iDex >> m_iInt >> m_iMp;

		tagName tag;
		pkt >> tag.tag >> tag.r >> tag.g >> tag.b; // tagrenk

		pkt >> Engine->Player.ZoneID >> Engine->Player.burninglevel;

		if (Engine->Player.burninglevel && Engine->uiHPBarPlug != NULL)
			Engine->uiHPBarPlug->BurningChange(Engine->Player.burninglevel);

		Packet UserInformation;
		UserInformation << m_iSocketID << m_strCharacterName << m_sClass << m_iRace << m_iLevel << m_iStr << m_iHp << m_iInt << m_iMp;

		if (Engine->m_PlayerBase != NULL)
			Engine->m_PlayerBase->Update(UserInformation);

		if (Engine->uiState != NULL)
		{
			// dd >> axe >> sword >> mace >> spear >> bow >> jamadar >>
			Engine->uiState->m_iCash = cash;
			Engine->uiState->m_iBalance = tlbalance;
			Engine->uiState->m_iDagger = dd;
			Engine->uiState->m_iAxe = axe;
			Engine->uiState->m_iSword = sword;
			Engine->uiState->m_iClub = mace;
			Engine->uiState->m_iSpear = spear;
			Engine->uiState->m_iArrow = bow;
			Engine->uiState->m_iJamadar = jamadar;
			Engine->uiState->UpdateUI();
		}

		//if (Engine->uiTradeInventory != NULL) Engine->uiTradeInventory->UpdateTotal(cash);
		if (!tag.tag.empty() && tag.tag != "-" )  
			Engine->addnewtagid(m_iSocketID, tag);
		
		uiINIT = true;
	}
	break;
	case XSafeOpCodes::MERC_WIEWER_INFO:
	{
		uint8 subCode;
		std::string Name;
		uint8 saat, dakika;
		pkt.SByte();
		pkt >> subCode >> Name >> saat >> dakika;
		switch (subCode)
		{
		case 1:
			Engine->WriteInfoMessageExt((char*)string_format(xorstr("%s visited your merchant at %02d:%02d"), Name.c_str(), saat, dakika).c_str(), -1);
			break;
		case 2:
			Engine->WriteInfoMessageExt((char*)string_format(xorstr("%s left your merchant at %02d:%02d"), Name.c_str(), saat, dakika).c_str(), -1);
			break;
		}
	}
	break;
	case XSafeOpCodes::DeathNotice:
	{
		uint8 sType;
		std::string kill;
		std::string Death;
		uint16 X, Z;
		pkt.SByte();
		pkt >> sType >> kill >> Death >> X >> Z;

		if (Engine->DeathNone)
			break;
		if (Engine->DeathAll)
		{
			if (sType == 1)
				Engine->WriteChatAddInfo(D3DCOLOR_ARGB(255, 254, 128, 254), false, string_format("*** %s has defeated %s - ( %d, %d ) ***", kill.c_str(), Death.c_str(), X, Z).c_str());
			else if (sType == 2)
				Engine->WriteChatAddInfo(D3DCOLOR_ARGB(255, 197, 209, 189), false, string_format("*** %s has defeated %s - ( %d, %d ) ***", kill.c_str(), Death.c_str(), X, Z).c_str());
			else
				Engine->WriteChatAddInfo(D3DCOLOR_ARGB(255, 255, 255, 0), false, string_format("*** %s has defeated %s - ( %d, %d ) ***", kill.c_str(), Death.c_str(), X, Z).c_str());

			break;
		}
		else if (Engine->DeathParty)
		{
			if (sType == 1)
				Engine->WriteChatAddInfo(D3DCOLOR_ARGB(255, 254, 128, 254), false, string_format("*** %s has defeated %s - ( %d, %d ) ***", kill.c_str(), Death.c_str(), X, Z).c_str());
			else if (sType == 2)
				Engine->WriteChatAddInfo(D3DCOLOR_ARGB(255, 197, 209, 189), false, string_format("*** %s has defeated %s - ( %d, %d ) ***", kill.c_str(), Death.c_str(), X, Z).c_str());

			break;
		}
		else if (Engine->DeathMe)
		{
			if (sType == 1)
				Engine->WriteChatAddInfo(D3DCOLOR_ARGB(255, 254, 128, 254), false, string_format("*** %s has defeated %s - ( %d, %d ) ***", kill.c_str(), Death.c_str(), X, Z).c_str());

			break;
		}
				
	}
	break;
	//case XSafeOpCodes::UPGRADE_RATE: 
	//	{
	//		uint32 Rate = pkt.read<uint32>();
	//		Engine->uiAnvil->SetResult("%" + std::to_string(Rate)); 
	//	}
	//	break;
	case XSafeOpCodes::CASTLE_SIEGE_TIMER:
		HandleCastleSiegeWarTimer(pkt);
		break;
	case XSafeOpCodes::ShowQuestList:
		Engine->uiSealToolTip->QuestCheck(pkt);
		break;
	case XSafeOpCodes::ACCOUNT_INFO_SAVE:
		Engine->HandleAccountInfoSave(pkt);
		break;
	case XSafeOpCodes::CHAOTIC_EXCHANGE:
		Engine->HandleChaoticExchange(pkt);
		break;
	case XSafeOpCodes::CHAT_LASTSEEN:
		Engine->HandleLastSeen(pkt);
		break;
	case XSafeOpCodes::AntiAfkList:
		Engine->RecvAntiAfkList(pkt);
		break;
	case XSafeOpCodes::WheelData:
		Engine->RecvWheelData(pkt);
		break;
	case XSafeOpCodes::TagInfo:
		Engine->HandleTagName(pkt);
	break;
	case XSafeOpCodes::PusRefund:
		Engine->HandlePusRefund(pkt);
		break;
	case XSafeOpCodes::QuestBoard:
	{
		pkt.SByte();

		std::string questName;
		uint16 Total, Cur;
		uint16 MonsterID;
		pkt >> questName >> Cur >> Total >> MonsterID;

		Engine->uiQuestComplated->SendQuestBoard(questName, Cur, Total, MonsterID, 1);
	}
	break;	
	case XSafeOpCodes::MerchantEye:
	{
			uint16 bSize, sSocketID;
			pkt >> bSize;

			pkt.SByte();

			Engine->MerchantEyeLock.lock();
			Engine->MerchantEyeData.clear();
			for (int i = 0; i < bSize; i++)
			{

				pkt >> sSocketID;
				MerchantEyeData item;
				item.nIndex = 1;
				
				for (int a = 0; a < 12; a++)
				{
					std::string ItemName;
					pkt >> ItemName;

					item.strMerchantItem[a] = ItemName;

				}
				Engine->MerchantEyeData.insert(std::pair<uint16, MerchantEyeData>(sSocketID, item));
			}
			Engine->MerchantEyeLock.unlock();
			if (Engine->m_UiMgr->uiMerchantEye == NULL)
			{
				std::string name = Engine->dcpUIF(xorstr("ISTIRAP\\re_merchant_eye.istirap"));
				Engine->m_UiMgr->uiMerchantEye = new CUIMerchantEye();
				Engine->m_UiMgr->uiMerchantEye->Init(Engine->m_UiMgr);
				Engine->m_UiMgr->uiMerchantEye->LoadFromFile(Engine->m_BasePath + name.c_str(), N3FORMAT_VER_1068);
				Engine->m_UiMgr->AddChild(Engine->m_UiMgr->uiMerchantEye);
				Engine->m_UiMgr->uiMerchantEye->Close();
				remove(name.c_str());
			}
			Engine->m_UiMgr->uiMerchantEye->Open();
	}
	break;
	case XSafeOpCodes::GenieInfo:
	{
		uint8 Status;

		pkt >> Status;

		if (Status)
		{
			Engine->m_isGenieStatus = true;

			std::map<uint32, CSpell>::iterator itr;
			const vector<uint32> healSkills = {
				// Karus
				111001,111005,111500,111509,111518,111527,111536,111545,112560,112554,112557,
				111554,111557,111560,112001,112005,112500,112509,112518,112527,112536,112545,
				
				//El Morad
				211001,211005,211500,211509,211518,211527,211536,211545,211554,211557,212557,
				211560,212001,212005,212500,212509,212518,212527,212536,212545,212554,212560,
				
			};
			const vector<uint32> NovaSkills = {
				110571,210571,110671,210671,110771,210771,110560,210560,110660,210660,110760,210760,110545,210545,110645,210645,110745,210745,
				
			};

			const vector<uint32> ThirtyThreeSkills = {
				109533,
				209533,
				110533,
				210533,
				109633,
				209633,
				110633,
				210633,
				109733,
				209733,
				110733,
				210733,
			};

			const vector<uint32> buffskills = {
				111654,112654,211654,212654,111606,112606,211606,212606,212672,112673,212673,
				111603,112603,211603,212603,111612,112612,211612,212612,112671,212671,112672,
				111621,112621,211621,212621,111630,112630,211630,212630,112670,212670,211660,
				111639,112639,211639,212639,111651,112651,212655,211651,212651,111660,112660,
				212660,112676,212676,112675,212675,112674,212674,111609,112609,211609,211627,
				212609,111615,112615,211615,212615,111624,112624,211624,212624,111627,112627,
				212627,111629,112629,211629,212629,111633,112633,211633,112657,211657,212657,
				212633,111636,112636,211636,212636,111642,112642,211642,211656,212656,111657,
				212642,111645,112645,211645,212645,111655,112655,211655,111656,112656,

			};
			

			bool cont = false;
			for (itr = Engine->skillmap.begin(); itr != Engine->skillmap.end(); itr++)
			{
				if (Engine->m_PlayerBase->GetClass() != itr->second.dwID / 1000)
					continue;
				if (itr->second.iCastTime < 1)
					continue;

				if (std::find(ThirtyThreeSkills.begin(), ThirtyThreeSkills.end(), itr->second.dwID) != ThirtyThreeSkills.end())
					continue;
				
				int8 hesaps = itr->second.iCastTime / 2;
				if (hesaps > 6)
					hesaps = 6;
				if (hesaps > 0)
					itr->second.iCastTime = hesaps;
				cont = false;
				/*if (std::find(NovaSkills.begin(), NovaSkills.end(), itr->second.dwID) != NovaSkills.end())
					continue;*/
				if (std::find(healSkills.begin(), healSkills.end(), itr->second.dwID) != healSkills.end())
					continue;
				if (std::find(buffskills.begin(), buffskills.end(), itr->second.dwID) != buffskills.end())
					continue;
			
				if (CSpell* spell = GetSkillBase(itr->second.dwID))
				{
					if (hesaps > 0)
						spell->iCastTime = hesaps;

				}
			}

		}
		else if (!Status)
		{
			Engine->m_isGenieStatus = false;
			std::map<uint32, CSpell>::iterator itr;
	
			for (itr = Engine->skillmap.begin(); itr != Engine->skillmap.end(); itr++)
			{
				auto bkp = Engine->skillmapBackup.find(itr->second.dwID);
				if (bkp == Engine->skillmapBackup.end())
					continue;

				itr->second.iCastTime = bkp->second.iCastTime;

				if (CSpell* spell = GetSkillBase(itr->second.dwID))
				{
					spell->iCastTime = bkp->second.iCastTime;

				}

			}
			
		}
	}
	break;
	default:
		break;
	}
}

void PearlEngine::HandlePusRefund(Packet &pkt) {
	uint8 opcode = pkt.read<uint8>();
	switch ((pusrefunopcode)opcode)
	{
	case pusrefunopcode::listsend:
	{
		if (PusRefundINIT) return;
		PusRefundINIT = true;

		vector<PUSREFUNDITEM> PusRefundItems;

		uint16 count = 0;
		pkt >> count;

		for (int i = 0; i < count; i++) {
			uint64 serial; uint32 sItemID, sPrice, expiredtime;
			pkt >> serial >> sItemID >> sPrice >> expiredtime;
			PusRefundItems.push_back(PUSREFUNDITEM(serial, sItemID, sPrice, expiredtime));
		}
		PusRefundINIT = true;
		Engine->m_UiMgr->pusrefund_itemlist = PusRefundItems;
	}
	break;
	case pusrefunopcode::itemnotfound:
		Engine->WriteInfoMessageExt((char*)string_format(xorstr("Power Up Store : Item not found.")).c_str(), 0xa30000);
		break;
	case pusrefunopcode::timeexpired:
		Engine->WriteInfoMessageExt((char*)string_format(xorstr("Power Up Store : The item you want to return has expired.")).c_str(), 0xa30000);
		break;
	case pusrefunopcode::procestime:
		Engine->WriteInfoMessageExt((char*)string_format(xorstr("Power Up Store : Please wait at least 5 seconds for the return process.")).c_str(), 0xa30000);
		break;
	case pusrefunopcode::notinventory:
		Engine->WriteInfoMessageExt((char*)string_format(xorstr("Power Up Store : The item was not found in their inventories.")).c_str(), 0xa30000);
		break;
	case pusrefunopcode::itemused:
		Engine->WriteInfoMessageExt((char*)string_format(xorstr("Power Up Store : Used items cannot be returned.")).c_str(), 0xa30000);
		break;
	case pusrefunopcode::itemreurnsucces:
	{
		uint64 serial; uint32 itemid;
		pkt >> serial >> itemid;
		for (auto it = m_UiMgr->pusrefund_itemlist.begin(); it != m_UiMgr->pusrefund_itemlist.end(); ++it) {
			if (it->serial == serial && it->itemid == itemid) {m_UiMgr->pusrefund_itemlist.erase(it);break;}
		}
		Engine->uiPowerUpStore->UpdateRefundItemList(Engine->m_UiMgr->pusrefund_itemlist);
		Engine->uiPowerUpStore->RefundSetItems();
		Engine->WriteInfoMessageExt((char*)string_format(xorstr("Power Up Store : The item has been successfully returned.")).c_str(), 0xa30000);
	}
	break;
	case pusrefunopcode::listadd:
	{
		uint64 serial; uint32 itemid, itemprice, expiredtime; uint16 itemcount,itemduration;
		pkt >> serial >> itemid >> itemcount >> itemprice >> itemduration >> expiredtime;
		if (!serial) return;
		m_UiMgr->pusrefund_itemlist.push_back(PUSREFUNDITEM(serial, itemid, itemprice, expiredtime));
		Engine->uiPowerUpStore->UpdateRefundItemList(Engine->m_UiMgr->pusrefund_itemlist);
		Engine->uiPowerUpStore->RefundSetItems(true);
	}
	break;
	}
}

void PearlEngine::addnewtagid(uint16 id, tagName tag) {
	tagLock.lock();
	auto itr = tagList.find(id);
	if (itr != tagList.end()) itr->second = tag;
	else tagList.insert({ id, tag });
	tagLock.unlock();
}

void PearlEngine::HandleTagName(Packet &pkt) {
	uint8 subcode = pkt.read<uint8>();
	switch ((tagerror)subcode)
	{
	case tagerror::Open:
	{
		if (!m_UiMgr) return;

		if (!Engine->uiTagChange) Engine->uiTagChange->OpenTagChange();
		if (!Engine->uiTagChange) return;

		Engine->SetVisible(Engine->uiTagChange->m_dVTableAddr,true);
	}
		break;
	case tagerror::List:
	{
		uint16 counter = 0;
		pkt >> counter;
		for (uint16 i = 0; i < counter; i++) {
			tagName tag;
			uint16 id = 0;
			pkt.DByte();
			pkt >> id >> tag.tag >> tag.r >> tag.g >> tag.b;
			addnewtagid(id,tag);
		}
	}
	break;
	case tagerror::success:
	{
		tagName tag;
		uint8 regipnu = 0; uint16 sockid;
		pkt.DByte();
		pkt >> regipnu >> sockid >> tag.tag >> tag.r >> tag.g >> tag.b;
		if (!regipnu) Engine->WriteInfoMessageExt((char*)string_format(xorstr("tag change success!")).c_str(), 0xa30000);
		addnewtagid(sockid, tag);
	}
	break;
	case tagerror::error:
		Engine->WriteInfoMessageExt((char*)string_format(xorstr("tag change error!")).c_str(), 0xa30000);
		break;
	case tagerror::already:
		Engine->WriteInfoMessageExt((char*)string_format(xorstr("tag change already!")).c_str(), 0xa30000);
		break;
	case tagerror::noitem:
		Engine->WriteInfoMessageExt((char*)string_format(xorstr("tag change no item!")).c_str(), 0xa30000);
		break;
	}
}

void PearlEngine::RecvAntiAfkList(Packet &pkt) {
	antiAFKProto.clear();
	uint16 size = 0; pkt >> size;
	if (!size) return;
	for (int i = 0; i < size; i++) {
		uint16 NpcID;
		pkt >> NpcID;
		if (!NpcID) continue;
		antiAFKProto.push_back(NpcID);
	}
}
void PearlEngine::RecvWheelData(Packet& pkt) {
	WheelItemData.clear();
	uint16 size = 0; pkt >> size;
	uint8 sCounter = 0;
	if (!size) return;
	for (int i = 0; i < size; i++) {
		if (sCounter >= 15)
			break;
		uint32 nItemID;
		pkt >> nItemID;
		if (!nItemID) continue;

		Engine->WheelItemData.insert(std::pair<uint8, uint32>(sCounter, nItemID));
		sCounter++;
	}
}
void PearlEngine::HandleLastSeen(Packet &pkt)
{
	std::string pTUser = "";
	pkt >> pTUser;
	if (pTUser.empty() || pTUser == "")
		return;

	uint8 UserLastSeen[2];
	pkt >> UserLastSeen[0] >> UserLastSeen[1];

	_PM_MAP * pPmMap = Engine->pmusers.GetData(pTUser);
	if (pPmMap == nullptr)
		return;

	pPmMap->LastSeen[0] = UserLastSeen[0];
	pPmMap->LastSeen[1] = UserLastSeen[1];
	pPmMap->LastSeenCheck = true;

}
void PearlEngine::HandleChaoticExchange(Packet &pkt)
{
	enum class ChaoticExchangeResult
	{
		ChaoticStop = 1,
		ChaoticSucces = 2
	};

	uint8 opcode;
	pkt >> opcode;

	switch ((ChaoticExchangeResult)opcode)
	{
	case ChaoticExchangeResult::ChaoticStop:
		ChaoticExchangeStop();
		break;
	case ChaoticExchangeResult::ChaoticSucces:
		ChaoticExchangeSucces();
		break;
	default:
		break;
	}
}

void PearlEngine::ChaoticExchangeStop()
{
	//Engine->uiPieceChangePlug->gemcount = 0;
}

void PearlEngine::ChaoticExchangeSucces()
{
	POINT pt;
	Engine->GetUiPos(Engine->uiPieceChangePlug->m_btnStop, pt);
	pt.y -= 2;
	Engine->m_UiMgr->SendMouseProc(UI_MOUSE_LBCLICK, pt, pt);
}

void HandleEventProcess(Packet& pkt)
{
	uint8 subcode = pkt.read<uint8>();

	if (subcode == TempleOpCodes::TEMPLE_EVENT)
	{
		uint16 active_event = 0, rem_time = 0;
		pkt >> active_event >> rem_time;
		if ((EventOpCode)active_event == EventOpCode::TEMPLE_EVENT_JURAD_MOUNTAIN
			&& rem_time > 0)
			Engine->OpenjuraidScreen(false, 0, 0, rem_time);
	}
	else if (subcode == TempleOpCodes::TEMPLE_EVENT_JOIN)
	{
		uint8 unk1;
		int16 active_event;
		pkt >> unk1 >> active_event;
		if (unk1 != 1 || (EventOpCode)active_event != EventOpCode::TEMPLE_EVENT_JURAD_MOUNTAIN)
			return;

		if (!Engine->m_UiMgr->uiJuraid)
			return;

		Engine->m_UiMgr->uiJuraid->JoinOrDisband(true);
	}
	else if (subcode == TempleOpCodes::TEMPLE_EVENT_DISBAND)
	{
		uint8 unk1;
		int16 active_event;
		pkt >> unk1 >> active_event;
		if (unk1 != 1 || (EventOpCode)active_event != EventOpCode::TEMPLE_EVENT_JURAD_MOUNTAIN)
			return;

		if (!Engine->m_UiMgr->uiJuraid)
			return;

		Engine->m_UiMgr->uiJuraid->JoinOrDisband(false);
	}
}

bool first = true;
bool __cdecl HandlePacket(Packet pkt)
{
	uint8 OpCode = pkt.GetOpcode();

	switch (OpCode)
	{
		case WIZ_NOTICE_SEND:
		{
			std::string msg;
			pkt >> msg;
			if (first)
			{
				first = false;
				Engine->WriteChatAddInfo(D3DCOLOR_ARGB(255, 254, 128, 254), false, msg.c_str());

			}
		}
		break;
		case XSafe:// E9 D5 true fals
		{
			XSafeHandlePacket(pkt);
			return true;
		}
		break;		
		case WIZ_EVENT:
			HandleEventProcess(pkt);
			break;
		case WIZ_ADD_MSG:
		{
			uint8 opcode;
			pkt >> opcode;
		
			 Engine->uiNoticeWind->MoticeMsgProcess(pkt, opcode);
			 return true;
		}
		break;
		case WIZ_CHAT:
		{
			uint8 type, nation;
			pkt >> type >> nation;
			if (type == 2)
			{
				int16 sID;
				string user, msg;
				uint8 auth = 0; 
				int8 rank;
				pkt >> sID;
				pkt.SByte();
				pkt >> user;
				pkt.DByte();
				pkt >> msg >> rank >> auth;

				if (user.empty() || msg.empty()/* || auth > 0*/) //Renkli Pm için authority kontrolü kapatýldý 27.09.2020
					return false;

				_PM_MAP * pPmMap = Engine->pmusers.GetData(user);
				if(pPmMap == nullptr)
				{
					_PM_MAP * pPmMap2 = new _PM_MAP;
					pPmMap2->sID = sID;
					pPmMap2->uName = user;
					pPmMap2->msgcount = 1;
					pPmMap2->btype = auth;
					pPmMap2->MessageCountCheck = true;
					pPmMap2->MessageClean = true;

					Engine->pmusers.PutData(user,pPmMap2);
				}
			
				
			}
		}
		break;
		case WIZ_CHAT_TARGET:
		{
			uint8 type;
			pkt >> type;
			if (type == 1)
			{
				uint8 auth = 0; int8 rank;
				int16 tmp;
				string user;
				pkt >> tmp >> user >> rank >> auth;
				if (tmp == 1)
				{
					if (user.empty()/* || auth > 0*/) //Renkli Pm için authority kontrolü kapatýldý 27.09.2020
						return false;
					
					_PM_MAP * pPmMap = Engine->pmusers.GetData(user);
					if (pPmMap == nullptr)
					{
						_PM_MAP * pPmMap = new _PM_MAP;
						pPmMap->uName = user;
						pPmMap->msgcount = 0;
						pPmMap->btype = auth;
						pPmMap->MessageCountCheck = true;
						pPmMap->MessageClean = false;
						Engine->pmusers.PutData(user, pPmMap);
					}
				}
			}
		}
		break;
		case WIZ_CLASS_CHANGE:
		{
			uint8 SubOpCode = 0, ReturnValue = 0;
			pkt >> SubOpCode >> ReturnValue;

			if (SubOpCode == 3 && ReturnValue == 1)
			{
				uint32 gold;
				uint16 str, hp, dex, intx, mp;
				int16 maxhp, maxmp;
				uint16 totalhit;
				uint32 maxweight;
				int16 points;
					
				pkt >> gold >> str >> hp >> dex >> intx >> mp >> maxhp >> maxmp >> totalhit >> maxweight >> points;
				if (Engine->uiHPBarPlug != NULL)
				{
					Engine->uiHPBarPlug->HPChange(maxhp, maxhp);
					Engine->uiHPBarPlug->MPChange(maxmp, maxmp);
				}
			}
		}
		break;
		case WIZ_POINT_CHANGE:
		{
			uint8 subcode;
			pkt >> subcode;


			uint32 tmp32;
			uint16 maxhp, hp, maxmp, mp, tmp;
			pkt >> tmp >> maxhp >> maxmp >> tmp >> tmp32 >> hp >> mp;
			if (Engine->uiHPBarPlug != NULL)
			{
				Engine->uiHPBarPlug->HPChange(hp, maxhp);
				Engine->uiHPBarPlug->MPChange(mp, maxmp);
			}
		}
		break;
		case WIZ_STATE_CHANGE:
		{
			uint8 bType; /*= pkt.read<uint8>(), buff;*/
			uint16 GetID;
			uint32 nBuff;/* = pkt.read<uint32>();
			buff = *(uint8 *)&nBuff; // don't ask*/
			pkt >> GetID >> bType >> nBuff;		

		}
		break;
		case WIZ_LEVEL_CHANGE:
		{

			short maxhp, hp, maxmp, mp;
			uint16 sock;
			uint8 level, freeskill;
			int16 points;
			int64 maxexp, xp;
			pkt >> sock >> level >> points >> freeskill >> maxexp >> xp >> maxhp >> hp >> maxmp >> mp;

			if (Engine->GetSocketID() != sock)
				return false;

			if (Engine->uiHPBarPlug != NULL) {
				Engine->uiHPBarPlug->HPChange(hp, maxhp);
				if (Engine->dc) Engine->dc->Update(true);

				Engine->uiHPBarPlug->MaxExp = maxexp;
				Engine->uiHPBarPlug->ExpChange(xp, maxexp);
			}
		}
		break;
		case WIZ_EXP_CHANGE:
		{
			uint8 opcode;
			pkt >> opcode;
#if (HOOK_SOURCE_VERSION == 1098)
			if (opcode == 1 && Engine->uiHPBarPlug != NULL) {
				int64 exp;
				pkt >> exp;
				Engine->uiHPBarPlug->ExpChange(exp, Engine->uiHPBarPlug->MaxExp);
			}
#else
			/*if (opcode == 3 && Engine->uiHPBarPlug != NULL) {
				pkt >> Engine->Player.burninglevel;
				if (Engine->Player.burninglevel > 3)
					Engine->Player.burninglevel = 3;

				Engine->uiHPBarPlug->BurningChange(Engine->Player.burninglevel);
			}*/
#endif
		}
		break;
		case WIZ_HP_CHANGE:
		{
			short maxhp, hp;
			pkt >> maxhp >> hp;
			if (Engine->uiHPBarPlug != NULL)
				Engine->uiHPBarPlug->HPChange(hp, maxhp);
		}
		break;
		case WIZ_MSP_CHANGE:
		{
			short maxmp, mp;
			pkt >> maxmp >> mp;
			if (Engine->uiHPBarPlug != NULL)
				Engine->uiHPBarPlug->MPChange(mp, maxmp);
		}
		break;
		case WIZ_SEL_CHAR:
			Engine->m_bSelectedCharacter = true;
			break;
		case WIZ_MYINFO:
		{
			Engine->m_bSelectedCharacter = true;
			Engine->m_bGameStart = true;
			SendHWID();

			if(Engine->uiSeedHelperPlug)
				Engine->uiSeedHelperPlug->SendOption();
		}
		break;
		case WIZ_PREMIUM:
		{
			uint8 subcode;
			pkt >> subcode;
			if (subcode == 2)
			{
				uint8 PremiumStatus;
				uint32 sPremiumDay;
				pkt >> PremiumStatus >> sPremiumDay;

				if (Engine->uiClanWindowPlug != NULL)
				{
					Engine->SetStringColor(Engine->uiClanWindowPlug->ClanPremStatus, D3DCOLOR_ARGB(255, 0, 254, 0));
					Engine->SetString(Engine->uiClanWindowPlug->ClanPremStatus, "Active");
				}
			}
			else
			{
				if (Engine->uiClanWindowPlug != NULL)
				{
					Engine->SetStringColor(Engine->uiClanWindowPlug->ClanPremStatus, D3DCOLOR_ARGB(255, 255, 0, 0));
					Engine->SetString(Engine->uiClanWindowPlug->ClanPremStatus, "DeActive");
				}
			}
		}
		break;
#if (HOOK_SOURCE_VERSION == 1098)
		case WIZ_PARTY:
		{
			uint8 subcode;
			pkt >> subcode;
			if (subcode == PARTY_INSERT)
			{
				short partyid, maxhp, hp, maxmp, mp;
				uint8 ret, level, nation, UserPartyType;
				uint16 iclass;
				string userName;
				//pkt >> ret >> userName >> maxhp >> hp >> level >> iclass >> maxmp >> mp >> nation;
				pkt >> partyid >> ret >> userName >> maxhp >> hp >> level >> iclass >> maxmp >> mp >> nation >> UserPartyType;
				Engine->m_bInParty = true;
#if (HOOK_SOURCE_VERSION == 1098)
				if (Engine->m_bInParty == true)
				{
					if (Engine->uiTaskbarMain != NULL)
					{
						Engine->SetVisible(Engine->uiTaskbarMain->m_btninvite, false);
						Engine->SetState(Engine->uiTaskbarMain->m_btninvite, UI_STATE_BUTTON_DOWN);
						Engine->SetVisible(Engine->uiTaskbarMain->m_btndisband, true);
						Engine->SetState(Engine->uiTaskbarMain->m_btndisband, UI_STATE_BUTTON_NORMAL);
					}
			}

#else

				if (Engine->uiTaskbarMain != NULL) {
					Engine->SetState(Engine->uiTaskbarMain->m_btnPartySettings, UI_STATE_BUTTON_NORMAL);
				}
#endif
			
			}
			else if (subcode == PARTY_DELETE)
			{
				Engine->m_bInParty = false;
#if (HOOK_SOURCE_VERSION == 1098)	
if (Engine->m_bInParty == false)
				{
					if (Engine->uiTaskbarMain != NULL)
					{
						Engine->SetVisible(Engine->uiTaskbarMain->m_btninvite, true);
						Engine->SetState(Engine->uiTaskbarMain->m_btninvite, UI_STATE_BUTTON_NORMAL);
						Engine->SetVisible(Engine->uiTaskbarMain->m_btndisband, false);
						Engine->SetState(Engine->uiTaskbarMain->m_btndisband, UI_STATE_BUTTON_DOWN);
					}
				}
#else
				if (Engine->uiTaskbarMain != NULL) {
					Engine->SetState(Engine->uiTaskbarMain->m_btnPartySettings, UI_STATE_BUTTON_DISABLE);
				}
#endif
			}
		}
		break;
#endif
		case WIZ_ZONE_CHANGE:
			Engine->HandleZoneChange(pkt);
		break;
		case WIZ_TARGET_HP:
		{
			if (!Engine->m_bHookStart) {
				StartHook();
			}
			if (Engine->uiTargetBar != NULL)
				Engine->uiTargetBar->SetTargetHp(pkt);
		}
		break;
		case WIZ_GAMESTART:
		{
	
			
			Engine->HandleGameStart(pkt);
			ASMPatch((DWORD)0x00549949, (char*)"\xEB\x37", 2);
		}
		break;
		case WIZ_ITEM_MOVE:
		{
			if (!Engine->m_bHookStart)
				StartHook();

			uint8 command, subcommand, x1, x2;
			uint16 m_sTotalHit, m_sTotalAc;
			uint32 m_sMaxWeight;
			short m_MaxHp, m_MaxMp;
			int16 BonusTotal[5];

			uint16 m_sFireR, m_sColdR, m_sLightningR;
			uint16 m_sMagicR, m_sDiseaseR, m_sPoisonR;

			int16 hp, mp;
			uint32 cash, moneyreq;
			uint16 dd, axe, sword, mace, spear, bow, jamadar;

			pkt >> command >> subcommand;

			if (subcommand != 0 && command != 2)
			{
				pkt >> m_sTotalHit >> m_sTotalAc >> m_sMaxWeight >> x1 >> x2 >> m_MaxHp >> m_MaxMp;
				for (int z = 0; z < 5; z++)
					pkt >> BonusTotal[z];

				pkt >> m_sFireR >> m_sColdR >> m_sLightningR >> m_sMagicR >> m_sDiseaseR >> m_sPoisonR;
				pkt >> cash >> dd >> axe >> sword >> mace >> spear >> bow >> jamadar >> moneyreq >> hp >> mp;

				Engine->Player.KnightCash = cash;
				Engine->Player.moneyreq = moneyreq;

				if (Engine->uiState != NULL)
				{
					Engine->uiState->m_iCash = cash;
					Engine->uiState->m_iDagger = dd;
					Engine->uiState->m_iAxe = axe;
					Engine->uiState->m_iSword = sword;
					Engine->uiState->m_iClub = mace;
					Engine->uiState->m_iSpear = spear;
					Engine->uiState->m_iArrow = bow;
					Engine->uiState->m_iJamadar = jamadar;
					Engine->uiState->UpdateUI();
				}

				if (Engine->uiTradeInventory != NULL)
					Engine->uiTradeInventory->UpdateTotal(cash);

				if (Engine->uiHPBarPlug != NULL)
				{
					Engine->uiHPBarPlug->HPChange(hp, m_MaxHp);
					Engine->uiHPBarPlug->MPChange(mp, m_MaxMp);
				}
				uiINIT = true;
			}
		}
		break;
		
		case WIZ_MERCHANT:
		{
			uint8 subCode;
			pkt >> subCode;

			if (Engine->m_MerchantMgr != NULL)
			{
				if (subCode == MERCHANT_ITEM_ADD)
					Engine->m_MerchantMgr->UpdateRecentItemAddReq(pkt);
				else if (subCode == MERCHANT_SLOT_UPDATE)
					Engine->m_MerchantMgr->UpdateItemDisplaySlots(pkt);
				else if (subCode == MERCHANT_ITEM_CANCEL)
					Engine->m_MerchantMgr->RemoveItemFromSlot(pkt);
				else if (subCode == MERCHANT_INSERT)
					Engine->m_MerchantMgr->MerchantCreated(pkt);
				else if (subCode == MERCHANT_ITEM_LIST)
					Engine->m_MerchantMgr->SetTheirMerchantSlots(pkt);
				else if (subCode == MERCHANT_TRADE_CANCEL)
					Engine->m_MerchantMgr->ResetMerchant();
				else if (subCode == MERCHANT_CLOSE)
					Engine->m_MerchantMgr->ResetMerchant();
			}
			switch (subCode)
			{
			case 0x32:
			{
				uint8 a, b, MerchantState, isPremium;
				uint16 mySocket;
				uint32 nPrice;
				uint8 isKC;
				pkt >> uint8(a)

					>> mySocket
					>> uint8(MerchantState)
					>> isPremium; // Type of merchant [normal - gold] // bool*/
				Engine->merchantItemsPriceLock.lock();
				Engine->merchantItemsPrice.erase(mySocket);
				MerchantItemView item;
				for (int i = 0, listCount = (MerchantState == 1 ? 4 : (isPremium ? 8 : 4)); i < listCount; i++)
				{
					pkt >> nPrice >> isKC;

					item.bSrcPos[i] = i;
					item.isKC[i] = isKC;
					item.price[i] = nPrice;


				}
				Engine->merchantItemsPrice.insert(std::pair<uint16, MerchantItemView>(mySocket, item));
				Engine->merchantItemsPriceLock.unlock();
			}
			break;
			break;
			}
		}
		break;
		//case WIZ_ITEM_UPGRADE:
		//	{
		//		uint8 subCode;
		//		pkt >> subCode;
		//		if (subCode == ITEM_BIFROST_EXCHANGE)
		//		{
		//			uint8 resultOpCode;
		//			pkt >> resultOpCode;

		//			if (resultOpCode == Failed) // kýrdýrmayý durdur
		//			{
		//				Engine->uiPieceChangePlug->m_bAutoExchangeStarted = false;
		//				Engine->uiPieceChangePlug->m_Timer = NULL;
		//			}
		//			else if (resultOpCode == Success)
		//			{
		//				POINT pt;
		//				Engine->GetUiPos(Engine->uiPieceChangePlug->m_btnStop, pt);
		//				pt.y -= 2;
		//				Engine->m_UiMgr->SendMouseProc(UI_MOUSE_LBCLICK, pt, pt);
		//			}
		//		}
		//	}
		//	break;
//		case WIZ_RANK:
//			Engine->Player.RankPacketReceived = true;
//			break;
//		case WIZ_PARTY_HP:
//		{
//			uint8 subCode;
//			pkt >> subCode;
//
//			if (subCode == 1 && Engine->uiPartyBBS != NULL)
//				Engine->uiPartyBBS->UpdatePartyUserHPBAR(pkt);
//		}
//			break;
	}

	if (!Engine->UPanelStart && Engine->m_bGameStart)
	{
		Packet result(XSafe);
		result << uint8_t(XSafeOpCodes::UIINFO);
		Engine->Send(&result);
		Engine->UPanelStart = true;
	}
	
	/*Engine->m_PlayerBase->UpdateGold();*/

	return false;
}

void PearlEngine::HandleAccountInfoSave(Packet &pkt)
{
	enum class AccountInfoSave {Open = 1,Close = 2};

	uint8 Opcode;
	pkt >> Opcode;
	switch ((AccountInfoSave)Opcode)
	{
	case AccountInfoSave::Open:
	{
		if (m_UiMgr == NULL)
			return;

		if (Engine->uiAccountRegisterPlug == NULL)
		{
			Engine->uiAccountRegisterPlug = new CUIAccountRegister();
		}
		Engine->uiAccountRegisterPlug->Open();
	}
	break;
	case AccountInfoSave::Close:
	{
		uint8 Opcode;
		pkt >> Opcode;

		if (m_UiMgr == NULL 
			|| Engine->uiAccountRegisterPlug == NULL)
			return;

		if (Opcode != 1) {
			Engine->uiAccountRegisterPlug->RefreshText();
			Engine->WriteInfoMessageExt((char*)string_format(xorstr("Account Info Save Failed")).c_str(), 0xa30000);
			return;
		}

		if (Engine->uiAccountRegisterPlug != NULL) {
			Engine->uiAccountRegisterPlug->Close();
		}
	}
	break;
	default:
		break;
	}
}

void PearlEngine::HandleGameStart(Packet &pkt)
{
	m_bSelectedCharacter = true;
	m_bGameStart = true;
	LoadingControl = false;
	gameStarted = true;
	if (Engine->dc) Engine->dc->Update(true);
}

void PearlEngine::HandleZoneChange(Packet &pkt)
{
	uint8 subCode;
	pkt >> subCode;

	if (Engine->dc) Engine->dc->Update(true);

	switch (subCode)
	{
	case 2:
		LoadingControl = false;
		Player.isTeleporting = false;
		break;
	case 3:
	{
		uint16 oldzone = Player.ZoneID;
		uint16 newZone;
		pkt >> newZone;
		Player.ZoneID = newZone;
		Player.isTeleporting = false;
		LoadingControl = false;

		if(uiQuestPage)
			uiQuestPage->InitQuests();

		if (Player.ZoneID != 30 && oldzone == 30 && Engine->m_UiMgr->uiCSW)
		{
			Engine->m_UiMgr->uiCSW->Close();
		}
	}
	break;
	}
}

std::mutex recv_mutex;

void RecvMake(RECV_DATA * pRecv)
{
	const std::lock_guard<std::mutex> lock(recv_mutex);

	Packet pkt;

	unsigned int length = pRecv->Size;
	if (length > 0)
		length--;

	pkt = Packet(pRecv->Data[0]);
	if (length > 0)
	{
		pkt.resize(length);
		memcpy((void*)pkt.contents(), &pRecv->Data[1], length);
	}

	HandlePacket(pkt);
}

DWORD rdwordExt(DWORD ulBase)
{
	if (!IsBadReadPtr((VOID*)ulBase, sizeof(DWORD)))
	{
		return(*(DWORD*)(ulBase));
	}
	return 0;
}

typedef bool(__stdcall* tRecv)(RECV_DATA* pRecv, void* pParam);
tRecv oRecv;
DWORD pktRet = 0;

bool WINAPI hkRECV(RECV_DATA* pRecv, void* pParam)
{
	if (pRecv->Size < 1)
		return true;

	pktRet = rdwordExt(KO_DLG);

	if (pktRet == 0)
		return true;

	__asm
	{
		MOV ECX, pktRet
		PUSH pParam
		PUSH pRecv
		CALL oRecv
	}

	RecvMake(pRecv);

	//async(RecvMake, pRecv).get();
}

const DWORD KO_OPEN_PUS = 0x008B623A;

void __fastcall pusuAc()
{
	if (Engine->uiPowerUpStore  == NULL)
	{
		Engine->uiPowerUpStore ->OpenPowerUpStore();
	}
	Engine->uiPowerUpStore ->UpdateItemList(Engine->m_UiMgr->item_list, Engine->m_UiMgr->cat_list);
	Engine->uiPowerUpStore ->UpdateRefundItemList(Engine->m_UiMgr->pusrefund_itemlist);
	Engine->uiPowerUpStore ->Open();
}

void __declspec(naked) hkOpenPUS()
{
	__asm {
		pushad
		pushfd
		call pusuAc
		popfd
		popad
		mov edx, KO_OPEN_PUS
		add edx, 9
		push edx
		ret
	}
}


bool RecvMake2(RECV_DATA* pRecv)
{
	RECV_DATA tmp = *pRecv;

	Packet pkt;

	unsigned int length = tmp.Size;
	if (length > 0)
		length--;

	pkt = Packet(tmp.Data[0]);
	if (length > 0)
	{
		pkt.resize(length);
		memcpy((void*)pkt.contents(), &tmp.Data[1], length);
	}

	return HandlePacket(pkt);
}

DWORD recvReturnAddress = 0;
void __declspec(naked) hkRecv_thyke(RECV_DATA* pRecv, void* pParam)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		pushad
		pushfd
		mov eax, [ebp + 4]
		mov recvReturnAddress, eax
		mov eax, dword ptr[pRecv]
		cmp dword ptr[eax + 4], 0
		je retback
		push eax
		call RecvMake2
		add esp, 4
		movzx ecx, al
		test ecx, ecx
		jne retend
		// devam et
		retback :
		popfd
			popad
			pop edi
			pop esi
			pop ebx
			pop ebp
			push - 1
			push 0x00B843C6
			mov eax, 0x00565890
			add eax, 7
			jmp eax
			// bitir
			retend :
		popfd
			popad
			pop edi
			pop esi
			pop ebx
			pop ebp
			mov al, 01
			add esp, 0x0c
			jmp recvReturnAddress
	}
}

void PearlEngine::InitRecvHook()
{
	DetourFunction((PBYTE)KO_OPEN_PUS, (PBYTE)hkOpenPUS);
	DetourFunction((PBYTE)0x00565890, (PBYTE)hkRecv_thyke);
}

int WSAAPI WSAStartupDetour(WORD wVersionRequired, LPWSADATA lpWSAData) 
{
	isAlive = true;
	return OrigWSAStartup(wVersionRequired, lpWSAData);
}

void PearlEngine::InitJmpHook(DWORD hookFuncAddr, DWORD myFuncAddr)
{
	SetMemArray(hookFuncAddr, 0x90, 6);
	JMPHOOK(hookFuncAddr, myFuncAddr);
}

inline void PearlEngine::JMPHOOK(DWORD Addr1, DWORD Addr2)
{
	BYTE jmp[] = { 0xE9,0,0,0,0 };
	DWORD diff = CalculateCallAddrWrite(Addr2, Addr1);

	memcpy(jmp + 1, &diff, 4);
	WriteProcessMemory(HANDLE(-1), (LPVOID)Addr1, jmp, 5, 0);
}

void SendHWID()
{
	isAlive = true;
	char AccName[25];
	ReadProcessMemory(GetCurrentProcess(), (LPVOID)KO_ACC, AccName, sizeof(AccName), NULL);
	string m_strAccountID = string(AccName);
	uint16 MACData1, MACData2 = 0;
	GetMacHash(MACData1, MACData2);
	int64 UserHardwareID = GetHardwareID();
	// yeni eklendi
	SHA1 sha;
	string itemorg = md5(sha.from_file(Engine->m_BasePath + xorstr("Data\\item_org_us.tbl")));
	string skillmagic = md5(sha.from_file(Engine->m_BasePath + xorstr("Data\\Skill_Magic_Main_us.tbl")));
	string zones = md5(sha.from_file(Engine->m_BasePath + xorstr("Data\\Zones.tbl")));
	string skillmagictk = md5(sha.from_file(Engine->m_BasePath + xorstr("Data\\Skill_Magic_Main_tk.tbl")));
	string srcversion = "f5h4y7r8d5v3sd1s696g9y7r5w5q1a2d23gf3e625q4"; // dllversion uyumsuz ise dc eder

	Packet result(XSafe);
	result << uint8(22) << m_strAccountID << itemorg << skillmagic << zones << skillmagictk << srcversion; // dllversion uyumsuz ise dc eder
	// ------
	Engine->Send(&result);
}

DWORD GetModuleSize(DWORD processID, char * module)
{
	HANDLE hSnap;
	MODULEENTRY32 xModule;
	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processID);
	xModule.dwSize = sizeof(MODULEENTRY32);
	if (Module32First(hSnap, &xModule)) 
	{
		while (Module32Next(hSnap, &xModule)) 
		{
			if (!strncmp((char*)xModule.szModule, module, 8)) 
			{
				CloseHandle(hSnap);
				return (DWORD)xModule.modBaseSize;
			}
		}
	}
	CloseHandle(hSnap);
	return 0;
}

// Packet simulation
inline void SetByte(char * tBuf, BYTE sByte, int & index)
{
	*(tBuf + index) = (char)sByte;
	index++;
};

inline void SetString(char * tBuf, char * sBuf, int len, int& index)
{
	memcpy(tBuf + index, sBuf, len);
	index += len;
};

char * sPacket;
uint8 bPacketSize;
uint8_t PacketMain = 0;
int32_t pSiz = 0, Send_Index = 0, ReturnAdress = 0;
DWORD HookAddres = 0, PushAdress = 0, BackToAdress = 0;
uint8 header = 0, subCode = 0;
uint32 nPrice, nItemID;
uint16 sCount;
uint8 bSrcPos, bDstPos, bMode;
uint32 nSkillID = 0;
int16 myID = 0, targetID = 0, targetX = 0, targetY = 0, targetZ = 0;
uint16 nObjectID = 0;
uint32 nExchangeItemID = 0, CheckReturnValue = 0;
int8_t nExchangeRobItemSlot = -1;

Packet OfficialList(XSafe);

uint32_t CheckSendPacket()
{
	uint32_t ReturnValue = 0;
	if (PacketMain != XSafe && PacketMain != WIZ_EDIT_BOX && PacketMain != WIZ_SEL_NATION && PacketMain != WIZ_MERCHANT  && PacketMain != WIZ_PARTY && PacketMain != WIZ_USER_INFO)
	{
		std::string CheckReturnAdress = to_string(ReturnAdress);
		ReturnValue = CheckReturnAdress.size();
		if (ReturnValue > 7)
			Engine->m_UiMgr->ShowMessageBox(xorstr("ThirdPartyTools"), xorstr("You shouldn't try cheating."), Ok);
	}
	return ReturnValue;
}

//uint32_t CheckSendPacket()
//{
//	uint32_t ReturnValue = 0;
//	if (PacketMain != XSafe)
//	{
//		std::string CheckReturnAdress = to_string(ReturnAdress);
//		ReturnValue = CheckReturnAdress.size();
//		if (ReturnValue > 7)
//			Engine->m_UiMgr->ShowMessageBox(xorstr("ThirdPartyTools"), xorstr("You shouldn't try cheating."), Ok);
//	}
//	return ReturnValue;
//}

__declspec(naked) void Real_Send() 
{
	__asm
	{
		MOV EAX, [ESP]
		MOV ReturnAdress, EAX
		MOV EAX, [ESP + 4]
		MOV sPacket, EAX
		MOV AL, BYTE PTR DS : [EAX]
		MOV PacketMain, AL
		MOV EAX, [ESP + 8]
		MOV pSiz, EAX
	}

	_asm pushad
	_asm pushfd


	Real_SendTime = clock();
	Engine->PacketCheckThreadID(PacketMain, ReturnAdress);

	if (PacketMain == WIZ_ZONE_CHANGE)
	{
		header = Engine->GetByte(sPacket, Send_Index);
		subCode = Engine->GetByte(sPacket, Send_Index);
		if (subCode == 1)
			Engine->Player.isTeleporting = true;
		else if (subCode == 2) {
			Engine->LoadingControl = false;
			Engine->Player.isTeleporting = false;
		}
	}

	if (PacketMain == WIZ_MAGIC_PROCESS)
	{
		Send_Index = 0;
		header = Engine->GetByte(sPacket, Send_Index);
		subCode = Engine->GetByte(sPacket, Send_Index);
		nSkillID = Engine->GetDWORD(sPacket, Send_Index);
		myID = Engine->GetShort(sPacket, Send_Index);
		targetID = Engine->GetShort(sPacket, Send_Index);
		targetX = Engine->GetShort(sPacket, Send_Index);
		targetY = Engine->GetShort(sPacket, Send_Index);
		targetZ = Engine->GetShort(sPacket, Send_Index);

		
#if ANTICHEAT_MODE == 1
		if (!CheckSkill(nSkillID) && !Engine->m_isGenieStatus) {
			__asm jmp PaketiAtma
		}
#endif
	}

	
		if (PacketMain == WIZ_MERCHANT)
		{
			Send_Index = 0;

			header = Engine->GetByte(sPacket, Send_Index);
			subCode = Engine->GetByte(sPacket, Send_Index);

			if (subCode == MerchantOpcodes::MERCHANT_MENISIA_LIST)
			{
				OfficialList << uint8(XSafeOpCodes::MERCHANTLIST) << uint8_t(0x00);
				Engine->Send(&OfficialList);
			}

			if (subCode == MERCHANT_ITEM_ADD) // item add to merchant
			{
				nItemID = Engine->GetDWORD(sPacket, Send_Index);
				sCount = Engine->GetShort(sPacket, Send_Index);
				nPrice = Engine->GetDWORD(sPacket, Send_Index);
				bSrcPos = Engine->GetByte(sPacket, Send_Index);
				bDstPos = Engine->GetByte(sPacket, Send_Index);
				bMode = Engine->GetByte(sPacket, Send_Index);
				Engine->SendItemAdd(nItemID, sCount, nPrice, bSrcPos, bDstPos, bMode);
			}
		}
		else if (PacketMain == WIZ_LOGOUT)
		{
			if (Engine->m_UiMgr != NULL)
				Engine->m_UiMgr->Release();
		}
		else if (PacketMain == 0x0D || PacketMain == 0x0E || PacketMain == 0x2E && !gameStarted)
			gameStarted = true;
		else if (PacketMain == WIZ_LOGOUT)
		{
			Engine->Player.logOut = true;
			TerminateProcess(GetCurrentProcess(), 0);
		}
		else if (PacketMain == WIZ_ITEM_UPGRADE)
		{
			Send_Index = 0;

			header = Engine->GetByte(sPacket, Send_Index);
			subCode = Engine->GetByte(sPacket, Send_Index);
			if (subCode == 5) // chaotic kýrdýrma
			{
				nObjectID = Engine->GetShort(sPacket, Send_Index);
				nExchangeItemID = Engine->GetDWORD(sPacket, Send_Index);
				nExchangeRobItemSlot = Engine->GetDWORD(sPacket, Send_Index);

				if (Engine->uiPieceChangePlug != NULL) {
					Engine->uiPieceChangePlug->m_nObjectID = nObjectID;
					Engine->uiPieceChangePlug->m_nExchangeItemID = nExchangeItemID;
					Engine->uiPieceChangePlug->m_nExchangeRobItemSlot = nExchangeRobItemSlot;
					Engine->SendChaoticExchange(nObjectID, nExchangeItemID, nExchangeRobItemSlot);
				}
			}
		}
	



	_asm
	{
		popfd
		popad
		PUSH - 1
		PUSH PushAdress
		JMP BackToAdress
		PaketiAtma :
		popfd
			popad
			add esp, 0x0c
			jmp ReturnAdress
	}
}

void PearlEngine::SendItemAdd(uint32 itemID, uint16 count, uint32 gold, uint8 srcPos, uint8 dstPos, uint8 mode)
{
	uint8 isKC = Engine->uiTradePrice->m_bIsKC ? 1 : 0;

	nisKC = isKC;
	RecvPrice = gold;
	Packet pkt(XSafe);
	pkt << uint8_t(XSafeOpCodes::MERCHANT) << ((uint8)MERCHANT_ITEM_ADD) << itemID << count << gold << srcPos << dstPos << mode << isKC;
	Send(&pkt);
	m_MerchantMgr->SetRecentItemAddReq(pkt);
}

void PearlEngine::PacketCheckThreadID(uint8 Packet, DWORD ThreadID)
{
	
	bool isPacket = true;
	switch (Packet)
	{
	case XSafe:
	case WIZ_POINT_CHANGE:
	case WIZ_MERCHANT:
	case WIZ_PARTY:
	case WIZ_GENIE:
	case WIZ_EVENT:
	case WIZ_USER_INFO:
	case WIZ_EDIT_BOX:							// Eðer paket atýnca oyun kapanýyor ise buraya dýþlamamýz gerekiyor Örneðimiz WIZ EDIT BOXtýr
		isPacket = false;
		break;
	}
	
	if (Engine->mReturnAdressSend.size() != 490)
		isPacket = true;
	if (isPacket)
	{
		auto itr = Engine->mReturnAdressSend.find(ReturnAdress);
		if (itr == Engine->mReturnAdressSend.end())
		{
			//printf("THETHYKE | EXCEPTION 1\n");
			ischeatactive = true;
		}
	}
}

inline int PearlEngine::GetShort(char* sBuf, int& index)
{
	index += 2;
	return *(short*)(sBuf + index - 2);
};

inline DWORD PearlEngine::GetDWORD(char* sBuf, int& index)
{
	index += 4;
	return *(DWORD*)(sBuf + index - 4);
};

inline BYTE PearlEngine::GetByte(char* sBuf, int& index)
{
	int t_index = index;
	index++;
	return (BYTE)(*(sBuf + t_index));
};

const DWORD KO_SND = KO_SND_FNC;

void PearlEngine::InitSendHook()
{
	HookAddres = KO_SND;
	PushAdress = *(DWORD*)(HookAddres + 4);
	BackToAdress = KO_SND + 7;

	InitJmpHook(KO_SND, (DWORD)Real_Send);
}

int WINAPI SendDetour(SOCKET s, char* buf, int len, int flags)
{
	Packet pkt;
	uint16 header;
	uint16 length;
	uint16 footer;
	memcpy(&header, buf, 2);
	memcpy(&length, buf + 2, 2);
	memcpy(&footer, buf + 4 + length, 2);
	uint8* in_stream = new uint8[length];
	memcpy(in_stream, buf + 4, length);

	if (length > 0)
		length--;

	pkt = Packet(in_stream[0], (size_t)length);
	if (length > 0)
	{
		pkt.resize(length);
		memcpy((void*)pkt.contents(), &in_stream[1], length);
	}

	delete[]in_stream;

	uint8 cmd = pkt.GetOpcode();
	

	return OrigSend(s, buf, len, flags);
}

string strToLower(string str) 
{
	for (auto& c : str) c = tolower(c);
	return str;
}

string WtoString(WCHAR s[]) 
{
	wstring ws(s);
	string ret;

	for (char x : ws)
		ret += x;
	
	return ret;
}

int currentID = 0;
vector<string> activeWindows;

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) {
	DWORD dwProcessId;
	GetWindowThreadProcessId(hWnd, &dwProcessId);
	if (currentID != dwProcessId) return TRUE;
	char String[255];
	if (!hWnd)
		return TRUE;
	if (IsWindowVisible(hWnd))
	{
		char wnd_title[256];
		GetWindowTextA(hWnd, wnd_title, sizeof(wnd_title));
		activeWindows.push_back(string((wnd_title)));
	}
	return TRUE;
}

bool inArray(vector<string> arr, string obj) 
{
	for (string i : arr) {
		if (obj == i) return true;
	}
	return false;
}

typedef HMODULE(WINAPI* tLoadLibrary)(LPCSTR lpLibFileName);
tLoadLibrary oLoadLibrary;

HMODULE WINAPI hkLoadLibrary(LPCSTR lpLibFileName) // Cheat Engine ile speed hack %100 Fix , Start
{
	string mdl = lpLibFileName;
	if (mdl.find("speedhack") != std::string::npos)
	{
		__asm {
			inc esp
			mov esp, 0x0
			push esp
			ret
		}
	}
	return oLoadLibrary(lpLibFileName);
} // Cheat Engine ile speed hack %100 Fix , End

void PearlEngine::InitPlayer() 
{
	oLoadLibrary = (tLoadLibrary)DetourFunction((PBYTE)LoadLibraryA, (PBYTE)hkLoadLibrary);

	if (thisProc == NULL) 
		thisProc = GetCurrentProcess();

	if (KO_ADR == 0x0) 
		ReadProcessMemory(thisProc, (LPCVOID)KO_PTR_CHR, &KO_ADR, sizeof(DWORD), 0);

	ofstream logFile;
	if (!dirExists(xorstr("\\")))
		CreateDirectoryA(xorstr("\\"), NULL);

	logFile.open(xorstr("\\init_log.txt"));
	Player.Nick = "";
	Player.Level = 0;
	Player.RebLevel = 0;
	Player.ZoneID = 0;
	Player.NationPoint = 0;
	Player.KnightCash = 0;
	Player.ddAc = 0;
	Player.axeAc = 0;
	Player.swordAc = 0;
	Player.maceAc = 0;
	Player.arrowAc = 0;
	Player.spearAc = 0;
	Player.isTeleporting = false;
	Player.isRankOpen = false;
	Player.logOut = false;
	Player.burninglevel = 0;
	Player.burningtime = 0;
	ReadProcessMemory(thisProc, (LPVOID)(KO_ADR + KO_OFF_ZONE), &Player.ZoneID, sizeof(Player.ZoneID), 0);
	this->Player.Authority = USER;
	uint16 MACData1, MACData2 = 0;
	GetMacHash(MACData1, MACData2);
	this->Player.MAC = uint32(MACData1 + MACData2);
	//Init GPU info
	DISPLAY_DEVICE DevInfo;
	DevInfo.cb = sizeof(DISPLAY_DEVICE);
	DWORD iDevNum = 0;
	logFile << xorstr("-- ISTIRAP Initializing --") << endl;
	while (EnumDisplayDevices(NULL, iDevNum, &DevInfo, 0))
	{
		if (inArray(this->Player.GPU, DevInfo.DeviceString)) {
			iDevNum++;
			continue;
		}
		this->Player.GPU.push_back(DevInfo.DeviceString);
		iDevNum++;
		logFile << xorstr("------ GPU: ") << DevInfo.DeviceString << endl;
	}
	tmpGraphics = "";
	for (string gpu : Player.GPU)
		tmpGraphics += xorstr(" | ")+ gpu;
	//Init processor info
	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo);
	int CPUInfo[4] = { -1 };
	__cpuid(CPUInfo, 0x80000000);
	unsigned int nExIds = CPUInfo[0];
	char CPUBrandString[0x40] = { 0 };
	for (unsigned int i = 0x80000000; i <= nExIds; ++i)
	{
		__cpuid(CPUInfo, i);
		if (i == 0x80000002)
		{
			memcpy(CPUBrandString,
				CPUInfo,
				sizeof(CPUInfo));
		}
		else if (i == 0x80000003)
		{
			memcpy(CPUBrandString + 16,
				CPUInfo,
				sizeof(CPUInfo));
		}
		else if (i == 0x80000004)
		{
			memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
		}
	}
	this->Player.CPU = string(CPUBrandString) + xorstr(" | ") + to_string(siSysInfo.dwNumberOfProcessors) + xorstr(" Core(s)");
	logFile << xorstr("------ CPU: ") << this->Player.CPU.c_str() << endl;
	tmpProcessor = Player.CPU;
	//Init hwid info
	this->Player.HWID = GetHardwareID();
	//Init screen info
	ScreenInfo* screen = new ScreenInfo();
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);
	screen->height = desktop.bottom;
	screen->width = desktop.right;
	this->Player.Screen = screen;
	//Init processes
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;
	if (EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
	{
		cProcesses = cbNeeded / sizeof(DWORD);
		for (i = 0; i < cProcesses; i++)
		{
			if (aProcesses[i] != 0) {
				char szProcessName[MAX_PATH];
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
				if (NULL != hProcess)
				{
					HMODULE hMod;
					DWORD cbNeeded;
					if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
					{
						GetModuleBaseNameA(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
						ProcessInfo procInfo;
						procInfo.id = aProcesses[i];
						procInfo.name = szProcessName;
						currentID = aProcesses[i];
						activeWindows.clear();
						EnumWindows(EnumWindowsProc, NULL);
						for (string windowName : activeWindows) {
							procInfo.windows.push_back(windowName);
						}
						this->Player.Processes.push_back(procInfo);
					}
				}
			}
		}
	}
	logFile.close();

	processTMP = Player.Processes;
	

	MainThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)EngineMain, this, NULL, NULL);
}

void LM_SendProcess(uint16 toWHO) 
{
	processTMP.clear();


	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		return;
	}

	PROCESSENTRY32 process;
	process.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hSnapshot, &process)) {
		CloseHandle(hSnapshot);
		return;
	}

	do {
		ProcessInfo procInfo;
		procInfo.id = process.th32ProcessID;
		procInfo.name = process.szExeFile;

		currentID = process.th32ProcessID;
		activeWindows.clear();
		EnumWindows(EnumWindowsProc, NULL);
		for (string windowName : activeWindows) {
			procInfo.windows.push_back(windowName);
		}
		processTMP.push_back(procInfo);

		std::cout << process.th32ProcessID << "\t" << process.szExeFile << std::endl;
	} while (Process32Next(hSnapshot, &process));

	CloseHandle(hSnapshot);

	Packet pkt(XSafe);
	pkt << uint8(XSafeOpCodes::PROCINFO) << uint16(toWHO) << uint32(processTMP.size());
	for (ProcessInfo proc : processTMP) 
	{
		pkt << int(proc.id) << string(proc.name) << int(proc.windows.size());
		for (string window : proc.windows)
			pkt << string(window);
	}
	LM_Send(&pkt);
}

void PearlEngine::SendProcess(uint16 toWHO) 
{
	LM_SendProcess(toWHO);
}

void PearlEngine::Disconnect() 
{
	allowAlive = false;
}

void PearlEngine::Send(Packet* pkt) 
{
	LM_Send(pkt);
}

void PearlEngine::StayAlive() 
{
	if (!isAlive || !allowAlive)
		return;

	char AccName[25]{};
	uint8 size = 0;
	ReadProcessMemory(GetCurrentProcess(), (LPVOID)KO_ACC_SIZE, &size, 1, NULL);
	if (size < 16)
		ReadProcessMemory(GetCurrentProcess(), (LPVOID)KO_ACC, AccName, sizeof(AccName), NULL);
	else
	{
		DWORD address;
		ReadProcessMemory(GetCurrentProcess(), (LPVOID)KO_ACC, &address, sizeof(address), NULL);
		ReadProcessMemory(GetCurrentProcess(), (LPVOID)address, AccName, sizeof(AccName), NULL);
	}

	accountID = string(AccName);
	if (accountID.size()) STRTOUPPER(accountID);

	clock_t realtime = Real_SendTime;
	bool cheatactive = ischeatactive;

	uint32 my = myrand(500, 3500);
	std::string public_key = md5("1X" + std::to_string(PL_VERSION) + "10001" + std::to_string(realtime) + std::to_string(cheatactive) + accountID.c_str());
	Packet pkt(XSafe, uint8(XSafeOpCodes::ALIVE));
	pkt.DByte();
	pkt << uint32(realtime) << my << public_key << uint8(cheatactive) << uint32(myrand(500, 3500));
	LM_Send(&pkt);
}

int WINAPI hTerminateProcess(HANDLE hProcess, UINT uExitCode) 
{
	if (hProcess == GetCurrentProcess())
		if (Engine->m_UiMgr != NULL)
			Engine->m_UiMgr->Release();
	return OrigTerminateProcess(hProcess, uExitCode);
}

int WINAPI hExitProcess(UINT uExitCode) 
{
	if (Engine->m_UiMgr != NULL)
		Engine->m_UiMgr->Release();

	return OrigExitProcess(uExitCode);
}

PearlEngine::~PearlEngine() 
{
}

//Uif Hook

DWORD PearlEngine::rdwordExt(DWORD ulBase)
{
	if (!IsBadReadPtr((VOID*)ulBase, sizeof(DWORD)))
	{
		return(*(DWORD*)(ulBase));
	}
	return 0;
}


DWORD PearlEngine::rdword(DWORD ulBase, std::vector<int> offsets)
{
	DWORD ibase = rdwordExt(ulBase);
	for (size_t i = 0; i < offsets.size() - 1; i++)
	{
		int offset = offsets[i];
		ibase += offset;
		int ibase1 = ibase;
		ibase = rdwordExt(ibase);
	}

	return ibase;
}

DWORD PearlEngine::ReadDWORD(DWORD ulBase, std::vector<int> offsets)
{
	DWORD ibase;
	ReadProcessMemory(GetCurrentProcess(), (LPVOID)ulBase, &ibase, sizeof(ibase), NULL);
	for (size_t i = 0; i < offsets.size() - 1; i++)
	{
		int offset = offsets[i];
		ibase += offset;
		int ibase1 = ibase;
		ReadProcessMemory(GetCurrentProcess(), (LPVOID)ibase, &ibase, sizeof(ibase), NULL);
	}
	return ibase;
}

std::string m_strReplaceString = "";
DWORD m_dvTable = 0x0;

void __declspec(naked) SetStringAsm()
{
	_asm
	{
		MOV ECX, m_dvTable
		MOV EAX, OFFSET m_strReplaceString
		PUSH EAX
		CALL KO_SET_STRING_FUNC
		RET
	}
}

typedef void(__thiscall* tSetString)(DWORD uiObject, const std::string& szString);
tSetString Func_SetString = (tSetString)KO_SET_STRING_FUNC;
void PearlEngine::UIScreenCenter(DWORD vTable)
{
	if (vTable == 0)
		return;
	if (IsBadReadPtr((VOID*)vTable, sizeof(DWORD)))
		return;

	
	RECT screen = Engine->m_UiMgr->GetScreenRect();
	POINT ret;
	ret.x = (screen.right / 2) - (Engine->GetUiWidth(vTable) / 2);
	ret.y = (screen.bottom / 2) - (Engine->GetUiHeight(vTable) / 2);
	Engine->SetUIPos(vTable, ret);

}
void PearlEngine::SetUIStyle(DWORD dwTable ,DWORD Style)
{
	*(DWORD*)(dwTable + 0xE4) = Style;
}
void PearlEngine::SetString(DWORD vTable, std::string str)
{
	if (vTable == 0)
		return;
	if (IsBadReadPtr((VOID*)vTable, sizeof(DWORD)))
		return;
	/*if(RDWORD(vTable + 0x1C) > 0)*/
	Func_SetString(vTable, str);
}


const   DWORD   KO_SET_SCROLL_VALUE_FUNC = 0x0042B3A0; // 2369
DWORD m_dvBase = 0x0;
int m_iVal = 0;

void __declspec(naked) SetScrollValueAsm()
{
	_asm
	{
		MOV ECX, m_dvBase
		MOV ESI, m_dvTable
		MOV EAX, m_iVal
		PUSH EAX
		CALL KO_SET_SCROLL_VALUE_FUNC
		RET
	}
}

void PearlEngine::SetScrollValue(DWORD vTable, int val)
{
	m_dvBase = *(DWORD*)(vTable + 0x134);
	m_dvTable = vTable;
	m_iVal = val;
	SetScrollValueAsm();
}

void PearlEngine::WriteString(DWORD value, char* vl)
{
	WriteProcessMemory(GetCurrentProcess(), (void*)value, (LPVOID)vl, sizeof(vl), 0);
}

std::string PearlEngine::GetString(DWORD vTable)
{
	if (!vTable)
		return "";

	DWORD nameKen = *(DWORD*)((DWORD)vTable + 0x154);
	char* name = new char[nameKen + 1]{ 0 };
	if (nameKen < 16)
		memcpy(name, (char*)((DWORD)vTable + 324), nameKen);
	else
		memcpy(name, (char*)*(DWORD*)((DWORD)vTable + 324), nameKen);
	std::string n = name;
	std::free(name);
	return n;

}

std::string PearlEngine::GetPartyString(DWORD vTable)
{
	if (vTable == 0x00)
		return "";

	char* buff;
	DWORD value = vTable + 324; //2383 for 
	buff = (char*)malloc(100);
	memcpy(buff, (char*)*(DWORD*)((DWORD)value), 100);
	return std::string(buff);
}

std::string PearlEngine::GetStringElementName(DWORD vTable)
{
	char* buff;
	DWORD value = vTable + 0x64;

	buff = (char*)malloc(100);
	ReadProcessMemory(GetCurrentProcess(), (void*)value, (LPVOID)buff, 100, 0);

	return std::string(buff);
}
std::string PearlEngine::GetStringFromPWEdit(DWORD vTable)
{
	char* buff;
	DWORD value = vTable + 140;

	buff = (char*)malloc(100);
	ReadProcessMemory(GetCurrentProcess(), (void*)value, (LPVOID)buff, 100, 0);

	return std::string(buff);
}

/*DWORD m_dChild = 0x0;
std::string m_strGetChildID;

typedef DWORD(__thiscall* tGetChildByIDFnc)(DWORD ecx, const std::string& a);
tGetChildByIDFnc GetChildByIDFnc = (tGetChildByIDFnc)0x410DF0;

void PearlEngine::GetChildByID(DWORD vTable, std::string id, DWORD& child)
{
	if (!vTable) return;
	child = GetChildByIDFnc(vTable, id);

	//m_dvTable = vTable;
	//m_dChild = child;
	//m_strGetChildID = id;
	//m_dChild = GetChildByIDFnc(m_dvTable, m_strGetChildID);
	//if (m_dChild == 0)
	//{
	//	std::string msg = string_format(xorstr("Element couldn't found: %s"), id.c_str());
	//	Engine->ShowMsg(MSG_INFO, msg);
	//	child = NULL;
	//	return;
	//}
	//child = m_dChild;
}*/


DWORD m_dChild = 0x0;
std::string m_strGetChildID;

void __declspec(naked) GetChildByIDAsm()
{
	_asm
	{
		MOV ECX, [m_dvTable]
		MOV EAX, OFFSET m_strGetChildID
		push m_dChild
		PUSH EAX
		CALL oGetChild
		MOV m_dChild, EAX
		RET
	}
}

__inline DWORD rRDWORD(DWORD ulBase)
{
	if (!IsBadReadPtr((VOID*)ulBase, sizeof(DWORD)))
	{
		return(*(DWORD*)(ulBase));
	}
	return 0;
}

int16 GetPartyLeaderSocket()
{
	DWORD base = rRDWORD(rRDWORD(KO_DLG) + KO_OFF_PTBASE);
	DWORD buffer = rRDWORD(base + KO_OFF_PT);
	buffer = rRDWORD(buffer);
	USHORT LeaderSocket = *(USHORT*)(buffer + 0x8);
	return LeaderSocket;
}

DWORD GetLeaderBase()
{
	return Engine->GetTarget(GetPartyLeaderSocket());
}
bool PearlEngine::isInParty()
{
	return GetPartyLeaderSocket();
}
int16 PearlEngine::GetPartyLeaderID()
{
	return GetPartyLeaderSocket();
}
DWORD PearlEngine::GetPartyLeaderBase()
{
	return GetLeaderBase();
}
int16 PearlEngine::GetPartyLeaderTarget()
{
	if (DWORD byAdress = GetLeaderBase())
	{
		return *(uint16*)(byAdress + KO_OFF_MOB);
	}

	return 0;
}

typedef DWORD(__thiscall* tGetChildByID)(DWORD uiObject, const std::string& szChildID, DWORD nUnkown);
tGetChildByID Func_GetChildByID = (tGetChildByID)KO_GET_CHILD_BY_ID_FUNC;
DWORD PearlEngine::GetChildByID(DWORD dwTable, std::string strID)
{
	if (!dwTable)
		return false;

	if (IsBadReadPtr((VOID*)dwTable, sizeof(DWORD)))
		return false;

	DWORD vTable = *(DWORD*)dwTable;
	if (!vTable || vTable < 0xC00000 || vTable > 0xFFFFFF)
		return false;

	return DWORD(Func_GetChildByID(dwTable, strID, 0));
}

void PearlEngine::GetChildByID(DWORD vTable, std::string id, DWORD& child)
{
	m_dvTable = vTable;
	m_dChild = child;
	m_strGetChildID = id;
	GetChildByIDAsm();
	if (m_dChild == 0)
	{
		std::string msg = string_format(xorstr("Element couldn't found: %s"), id.c_str());
		Engine->ShowMsg(MSG_INFO, msg);
		child = NULL;
		return;
	}
	child = m_dChild;
}

int16 PearlEngine::GetSocketID()
{
	return *(int16*)(*(DWORD*)(KO_PTR_CHR)+KO_OFF_ID);
}
const DWORD KO_UIF_LIST_ADD_TEXT = 0x00422850;  // Search Monster Drop List'e eleman ekleme
const DWORD KO_UIF_LIST_CLEAR_TEXT = 0x004213C0;  // Search Monster Drop List'e eleman ekleme
int16 PearlEngine::GetListSelect(DWORD vTable)
{
	if (vTable == NULL)
		return -1;


	return *(DWORD*)(vTable + 0x144);
}
void PearlEngine::ClearListString(DWORD vTable)
{
	if (vTable == NULL)
		return;
	__asm
	{
		MOV ECX , vTable
		call  KO_UIF_LIST_CLEAR_TEXT
	}
}

std::string tmpstr = "";

void PearlEngine::AddListString(DWORD vTable, const std::string& szString, DWORD color)
{
	if (vTable == NULL || szString.empty())
		return;

	tmpstr = szString;

	__asm {
		push 0xFF80FF80
		push 0xF
		push 0
		push 0
		push 0
		push 0
		push 0
		push offset tmpstr
		push color
		push offset tmpstr
		mov ecx, vTable
		call KO_UIF_LIST_ADD_TEXT
	}
}

const DWORD KO_UIF_KILL_FOCUS = 0x00416590;  // Killfocus Fonksiyonu -> Power Up store ve diðer uiflerde kullanýlacak.

void PearlEngine::EditKillFocus(DWORD vTable)
{
	if (vTable == NULL)
		return;

	__asm {
		mov ecx, vTable
		call KO_UIF_KILL_FOCUS
	}
}

void PearlEngine::GetBaseByChild(DWORD vTable, DWORD& base)
{
	base = *(DWORD*)(vTable + 0xBC);
}

DWORD m_dBool = 0;

typedef void (__thiscall* tSetVisibleFunc)(DWORD ecx, bool a2);
tSetVisibleFunc SetVisibleFunc = (tSetVisibleFunc)KO_SET_VISIBLE_FUNC;
void PearlEngine::SetUiRegion(DWORD vTable, RECT rc)
{
	uintptr_t** ptrVtable = (uintptr_t**)vTable;

	LONG left = (LONG)ptrVtable[59];
	LONG top = (LONG)ptrVtable[60];
	LONG right = (LONG)ptrVtable[61];
	LONG bottom = (LONG)ptrVtable[62];

	ptrVtable[59] = (uintptr_t*)rc.left;
	ptrVtable[60] = (uintptr_t*)rc.top;
	ptrVtable[61] = (uintptr_t*)rc.right;
	ptrVtable[62] = (uintptr_t*)rc.bottom;
}
void PearlEngine::SetVisible(DWORD vTable, bool type)
{
	if (vTable == 0x0)
		return;

	//*(bool*)(vTable + 0x10D) = type;

	
	SetVisibleFunc(vTable, type);
}

bool PearlEngine::IsVisible(DWORD vTable)
{
	return vTable ? *(bool*)(vTable + 0x10D) : false;
}

PVOID GetLibraryProcAddress(LPCSTR LibraryName, LPCSTR ProcName)
{
	return GetProcAddress(GetModuleHandleA(LibraryName), ProcName);
}

PearlEngine::PearlEngine(std::string basePath) 
{
	varius = 0;
	pPerks = _PERKS_DATA();
	m_PerksArray.clear();
	merchantdisplaydata = merchantdisplaySpecialdata = 0;
	m_recentdelete_time = (30 * 1000) * 60;
	OrigWSAStartup = (MyWSAStartup)DetourFunction((PBYTE)WSAStartup, (PBYTE)WSAStartupDetour);
	fncGuard.KeepFunction((DWORD)clock, 0x4BA, xorstr("clock"));
	fncGuard.KeepFunction((DWORD)memcpy, 0x33D, xorstr("memcpy"));
	fncGuard.KeepFunction((DWORD)memcpy_s, 0x6D, xorstr("memcpy_s"));
	fncGuard.KeepFunction((DWORD)malloc, 0xEA, xorstr("malloc"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("Kernel32.dll"), xorstr("IsDebuggerPresent")), 6, xorstr("IsDebuggerPresent"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("Kernel32.dll"), xorstr("CreateThread")), 0x2B, xorstr("CreateThread"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("Kernel32.dll"), xorstr("CreateRemoteThread")), 0x2C, xorstr("CreateRemoteThread"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("Kernel32.dll"), xorstr("SetThreadContext")), 0xC, xorstr("SetThreadContext"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("Kernel32.dll"), xorstr("GetThreadContext")), 0xC, xorstr("GetThreadContext"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("Kernel32.dll"), xorstr("ReadProcessMemory")), 0x3D, xorstr("ReadProcessMemory"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("Kernel32.dll"), xorstr("WriteProcessMemory")), 0xC, xorstr("WriteProcessMemory"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("Kernel32.dll"), xorstr("VirtualAlloc")), 0x58, xorstr("VirtualAlloc"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("Kernel32.dll"), xorstr("VirtualAllocEx")), 0xC, xorstr("VirtualAllocEx"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("Kernel32.dll"), xorstr("VirtualFreeEx")), 0xC, xorstr("VirtualFreeEx"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("Kernel32.dll"), xorstr("VirtualProtectEx")), 0xC, xorstr("VirtualProtectEx"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("Kernel32.dll"), xorstr("VirtualQueryEx")), 0xC, xorstr("VirtualQueryEx"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("Kernel32.dll"), xorstr("OpenThread")), 0xC, xorstr("OpenThread"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("Kernel32.dll"), xorstr("OpenProcess")), 0xC, xorstr("OpenProcess"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("Kernel32.dll"), xorstr("GetTickCount")), 9, xorstr("GetTickCount"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("Kernel32.dll"), xorstr("GetTickCount64")), 0x54, xorstr("GetTickCount64"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("User32.dll"), xorstr("GetAsyncKeyState")), 0xC1, xorstr("GetAsyncKeyState"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("User32.dll"), xorstr("EnumWindows")), 0x46, xorstr("EnumWindows"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("User32.dll"), xorstr("GetForegroundWindow")), 6, xorstr("GetForegroundWindow"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("User32.dll"), xorstr("GetWindowTextA")), 0x77, xorstr("GetWindowTextA"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("User32.dll"), xorstr("GetWindowTextW")), 0x29D, xorstr("GetWindowTextW"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("Ws2_32.dll"), xorstr("connect")), 0x181, xorstr("connect"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("Ws2_32.dll"), xorstr("recv")), 0x175, xorstr("recv"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("Ws2_32.dll"), xorstr("send")), 0xB2, xorstr("send"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("Ws2_32.dll"), xorstr("WSARecv")), 0x2F5, xorstr("WSARecv"));
	fncGuard.KeepFunction((DWORD)GetLibraryProcAddress(xorstr("Ws2_32.dll"), xorstr("WSASend")), 0x2F5, xorstr("WSASend"));
	bDisableAllSkillFX = false;
	bDisableAreaSkillFX = false;
	bDisableViewEmblem = false;
	bDisableViewPathos = false;
	bDisableHealFX = false;
	DeathAll = true;
	DeathMe = false;
	DeathNone = false;
	DeathParty = false;
	timesPassed = 0;
	dc = NULL;
	render = true;
	m_connectedIP = "";
	power = true;
	m_BasePath = basePath;
	hpBarAdress = 0;
	ScanThread = NULL;
	IsCRActive = false;
	isSiegeWarStart = isSiegeWarActive = false;
	Loading = false;
	LoqOut = false;
	LoadingControl = false;
	disableCameraZoom = false;
	tblMgr = NULL;
	moneyReq = 0;
	m_zMob = 0;
	StringHelper = NULL;
	m_PlayerBase = NULL;
	m_UiMgr = NULL;
	uiState = NULL;
	uiTargetBar = NULL;
	uiTaskbarMain = NULL;
	uiTaskbarSub = NULL;
	uiClanWindowPlug = NULL;
	uiMiniMenuPlug = NULL;
	uiPlayerRank = NULL;
	uiNoticeWind = NULL;
	uiGenieSubPlug = NULL;
	uiSchedularPlug = NULL;
	uiScoreBoard = NULL;
	uiGenieMain = NULL;
	m_SettingsMgr = NULL;
	uiSeedHelperPlug = NULL;
	uiPieceChangePlug = NULL;
	uiLogin = NULL;
	uiQuestPage = NULL;
	uiSealToolTip = NULL;
	uiPowerUpStore = NULL;
	uiTradePrice = NULL;
	uiTradeInventory = NULL;
	uiTradeItemDisplay = NULL;
	uiTradeItemDisplaySpecial = NULL;
	uiChatBarPlug = NULL;
	m_SettingsMgr = NULL;
	m_MerchantMgr = NULL;
	uiToolTip = NULL;
	uiInventoryPlug = NULL;
	uiPartyBBS = NULL;
	uiHPBarPlug = NULL;
	uiHpMenuPlug = NULL;
	uiAccountRegisterPlug = NULL;
	uiTopRightNewPlug = NULL;
	uiMinimapPlug = NULL;
	uiSupport = NULL;
	uiCollection = NULL;
	uiWheel = NULL;
	uiSearchMonster = NULL;
	uiDropResult = NULL;
	uiTagChange = NULL;
	uiSkillPage = NULL;
	uiQuestComplated = NULL;
	hkRightClickExchange = NULL;
	uiLottery = NULL;
	uiCindirella = NULL;
	uiEventShowList = NULL;
	uiMerchantList = NULL;
	_pDevice = NULL;
	logState = true;
	drawMode = true;
	_lowPower = false;
	strClientName.clear();
	m_bSelectedCharacter = false;
	m_bGameStart = false;
	m_bInParty = false;
	m_bHookStart = false;
	m_isGenieStatus = false;
	Adress = 0;
	UPanelStart = false;
	InitPlayer();
	tblMgr->Init();
}

void PearlEngine::ShowMsg(MSG_TYPE type, std::string msg, ...)
{
	std::string result;
	va_list ap;

	va_start(ap, msg);
	tstring_format(msg, &result, ap);
	va_end(ap);

	if (type == MSG_ERROR)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FORE_COLOR_LIGHTRED);
#ifndef _DEBUG
		MessageBoxA(NULL, xorstr("Error-> "), "Error\n", MB_OK);
#endif
	}
	else if (type == MSG_SUCCESS)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FORE_COLOR_LIGHTGREEN);
#ifndef _DEBUG
		MessageBoxA(NULL, xorstr("OK-> "), "Error\n", MB_OK);
#endif
	}
	else if (type == MSG_WARNING)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FORE_COLOR_YELLOW);
#ifndef _DEBUG
		MessageBoxA(NULL, xorstr("Warn-> "), "Error\n", MB_OK);
#endif
	}
	else if (type == MSG_INFO)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FORE_COLOR_LIGHTBLUE);
#ifndef _DEBUG
		MessageBoxA(NULL, xorstr("Message-> "), "Error\n", MB_OK);
#endif
	}
#ifndef _DEBUG
	MessageBoxA(NULL, result.c_str(), "result\n", MB_OK);
#endif
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FORE_COLOR_LIGHTGRAY);
}

void PearlEngine::WriteInfoMessageExt(char* pMsg, DWORD dwColor)
{
	int iMsgLen = strlen(pMsg);
	char* pParam = new char[iMsgLen + 33];
	DWORD	dwParamAddr = (DWORD)pParam;

	memset(pParam, 0, iMsgLen + 33);
	memcpy(pParam + 32, pMsg, iMsgLen);

	*(int*)(pParam + 20) = iMsgLen;
	*(DWORD*)(pParam + 4) = (DWORD)pParam + 32;
	*(DWORD*)(pParam + 24) = 0x1F;

	__asm
	{
		MOV ECX, DWORD PTR DS : [0x00F3690C]

		push 0
		push dwColor
		push dwParamAddr
		mov  eax, KO_ADD_INFO_MSG_FUNC
		call eax
	}

	delete[] pParam;
}
void PearlEngine::WriteChatAddInfo(DWORD pColor, bool isBold, const char* pText, uint8 nRank)
{
	reinterpret_cast<void(__thiscall*)(DWORD, int, const std::string&, DWORD, bool, int)>(KO_CHAT_ADD_INFO)(*(DWORD*)((*(DWORD*)KO_DLG) + 0x20C), 1, std::string(pText), pColor, !isBold, nRank);
}
void PearlEngine::WriteInfoMessage(char* pMsg, DWORD dwColor)
{
	WriteInfoMessageExt(pMsg, dwColor);
}

POINT PearlEngine::GetUiPos(DWORD vTable)
{
	POINT tmp;
	tmp.x = 0;
	tmp.y = 0;

	if (!vTable || IsBadReadPtr((VOID*)vTable, sizeof(DWORD)))
		return tmp;

	tmp = *(POINT*)(vTable + 0xEC);
	return tmp;
}
POINT PearlEngine::GetUiPos2(DWORD vTable)
{
	POINT pt;
	pt.x = 0;
	pt.y = 0;

	if (vTable == 0)
		return pt;

	uintptr_t** ptrVtable = (uintptr_t**)vTable;


	pt.x = (LONG)ptrVtable[59];
	pt.y = (LONG)ptrVtable[60];

	return pt;
}
RECT PearlEngine::GetUiRegion(DWORD vTable)
{
	return *(RECT*)(vTable + 0xEC);
}
RECT PearlEngine::GetUiRegion2(DWORD vTable)
{
	uintptr_t** ptrVtable = (uintptr_t**)vTable;

	RECT rc;
	rc.left = (LONG)ptrVtable[59];
	rc.top = (LONG)ptrVtable[60];
	rc.right = (LONG)ptrVtable[61];
	rc.bottom = (LONG)ptrVtable[62];
	return rc;
}

typedef int(__thiscall* tMoveOffset)(DWORD ecx, signed int a2, int a3);
tMoveOffset __MoveOffset = (tMoveOffset)0x4108F0; // move offset

void PearlEngine::UiPost(DWORD vTable, LONG x, LONG y)
{

	__MoveOffset(vTable, x, y);
}
void PearlEngine::SetUIPos(DWORD vTable, POINT pt)
{
	RECT rc = GetUiRegion(vTable);
	int dx, dy;
	dx = pt.x - rc.left;
	dy = pt.y - rc.top;

	__MoveOffset(vTable, dx, dy);
}

void PearlEngine::GetUiPos(DWORD vTable, POINT& pt)
{
	uintptr_t** ptrVtable = (uintptr_t**)vTable;
	pt.x = (LONG)ptrVtable[59];
	pt.y = (LONG)ptrVtable[60];
}

DWORD m_iParam1;
DWORD m_iParam2;
DWORD m_iParam3;

void __declspec(naked) SetStateAsm()
{
	_asm
	{
		MOV ECX, m_dvTable
		MOV EAX, m_iParam1
		PUSH EAX
		CALL KO_UI_SET_STATE_FUNC
		RET
	}
}

void PearlEngine::SetState(DWORD vTable, DWORD state)
{
	if (!vTable || IsBadReadPtr((VOID*)(vTable), sizeof(DWORD))) return;
	m_dvTable = vTable;
	m_iParam1 = state;
	SetStateAsm();
}

DWORD PearlEngine::GetRecvMessagePtr(DWORD adres)
{
	if (adres == 0)
		return 0;
	return (*(DWORD*)adres) + 0x70;
}

uint32 PearlEngine::GetState(DWORD vTable)
{
	return Read4Bytes(vTable + 0xE0);
}

void SetMemBYTE(DWORD Adres, BYTE Deger)
{
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)Adres, &Deger, 1, NULL);
}

void PearlEngine::SetMemArray(DWORD Adres, BYTE Deger, DWORD len)
{
	for (DWORD i = 0; i < len; i++)
		SetMemBYTE(Adres + i, Deger);
}

void PearlEngine::InitCallHook(DWORD hookFuncAddr, DWORD myFuncAddr)
{
	SetMemArray(hookFuncAddr, 0x90, 5);
	CALLHOOK(hookFuncAddr, myFuncAddr);
}

DWORD PearlEngine::CalculateCallAddrWrite(DWORD Addr1, DWORD Addr2)
{
	return Addr1 - Addr2 - 5;
}

inline void PearlEngine::CALLHOOK(DWORD Addr1, DWORD Addr2)
{
	BYTE call[] = { 0xE8,0,0,0,0 };
	DWORD diff = CalculateCallAddrWrite(Addr2, Addr1);

	memcpy(call + 1, &diff, 4);
	WriteProcessMemory(HANDLE(-1), (LPVOID)Addr1, call, 5, 0);
}

void __stdcall CameraZoom_Hook(float fDelta)
{
	DWORD thisPtr;
	__asm
	{
		MOV thisPtr, ECX
	}

	if (Engine->disableCameraZoom)
		return;

	__asm
	{
		MOV ECX, thisPtr
		PUSH fDelta
		MOV EAX, KO_CAMERA_ZOOM_FUNC
		CALL EAX
	}
}

void PearlEngine::InitCameraZoom()
{
	InitCallHook(KO_CAMERA_ZOOM_CALL_ADDR, (DWORD)CameraZoom_Hook);
}

void PearlEngine::SendChaoticExchange(uint16 nObjectID, uint32 ExchangeRobItemID, int8 ExchangeRobItemSlot)
{
	bool bank = uiPieceChangePlug->m_bank, sell = uiPieceChangePlug->m_sell;
	
	int8 curCount = atoi(Engine->GetString(uiPieceChangePlug->m_textExCount).c_str());
	if (curCount < 1) curCount = 1;
	else if (curCount > 100) curCount = 100;
	
	Packet pkt(XSafe, uint8(XSafeOpCodes::CHAOTIC_EXCHANGE));
	pkt << nObjectID << ExchangeRobItemID << ExchangeRobItemSlot << bank << sell << curCount;
	Send(&pkt);
}
std::string PearlEngine::exeGetName()
{
	DWORD ADDR_CHR = *(DWORD*)(KO_PTR_CHR);
	char AccName[25];
	ReadProcessMemory(GetCurrentProcess(), (LPVOID)(ADDR_CHR + KO_OFF_NAME), AccName, sizeof(AccName), NULL);
	return string(AccName);
}
const	DWORD	KO_SET_STRING_COLOR_FUNC = 0x0040F710;
const	DWORD	KO_SET_STRING_COLOR_RET_ADDR = 0x0040F716;

void PearlEngine::SetStringColor(DWORD vTable, DWORD color) // Pm renk deðiþtirme patlama sorunu çözüldü
{
	if (vTable == 0)
		return;
	if (!IsBadReadPtr((VOID*)vTable, sizeof(DWORD)) && !IsBadReadPtr((VOID*)(vTable + 0x15C), sizeof(DWORD)))
		*(DWORD*)(vTable + 0x15C) = color;
}

std::string purchasingPriceSearch = xorstr("purchasing price");

string loadingArray[] = 
{
	xorstr("Allocating Terrain..."),
	xorstr("Loading "),
	xorstr("Loading Effect Data..."),
	xorstr("Loading River Data..."),
	xorstr("Loading Terrain Patch Data..."),
	xorstr("Loading Terrain Tile Data..."),
	xorstr("Loading Terrain Grass Data..."),
	xorstr("Loading Lightmap Data..."),
	xorstr("Loading colormap"),
	xorstr("Loading Objects..."),
	xorstr("Loading Character Data..."),
	xorstr("Loading Information") ,
	xorstr("Loading Information[1]..."),
	xorstr("Loading Information[2]..."),
	xorstr("Loading Information[3]..."),
	xorstr("Loading Information[4]..."),
	xorstr("Loading Information[5]..."),
	xorstr("Loading Information[6]..."),
	xorstr("Loading Information[7]..."),
	xorstr("Loading Information[8]..."),
	xorstr("Loading Information[9]..."),
	xorstr("Loading Information[10]...")
};
bool isFindAdress = false;
bool checkkk = false;
bool isFalsed = false;
DWORD KO_CHR = *(DWORD*)KO_PTR_CHR;

bool MerchantPriceView(uint8& PriceType, uint32& Price)
{


	if (Engine->IsVisible(Engine->merchantdisplaydata))
	{
		if (sSocketMerchant > 10000 || sSocketMerchant < 1)
			return 0;
		PriceType = 0;
		Price = 0;
		uint8 getdata = 0;
		bool yoklama = false;
		for (int i = 0; i < 4; i++)
		{
			if (Engine->IsIn(Engine->merchantdisplayItem[i], Engine->lastMousePos.x, Engine->lastMousePos.y))
			{
				getdata = i;
				yoklama = true;
				break;
			}
		}
		if (yoklama)
		{
			Engine->merchantItemsPriceLock.lock();
			foreach(it, Engine->merchantItemsPrice)
			{
				if (it->first != sSocketMerchant)
					continue;

				for (int i = 0; i < 4; i++)
				{
					if (it->second.bSrcPos[i] != getdata)
						continue;

					PriceType = it->second.isKC[i];
					Price = it->second.price[i];
					break;
				}
				if (Price > 0)
					break;
			}
			Engine->merchantItemsPriceLock.unlock();
			return true;
		}
	}
	else if (Engine->IsVisible(Engine->merchantdisplaySpecialdata))
	{
		if (sSocketMerchantSpecial > 10000 || sSocketMerchantSpecial < 1)
			return 0;
		PriceType = 0;
		Price = 0;
		uint8 getdata = 0;
		bool yoklama = false;
		for (int i = 0; i < 8; i++)
		{
			if (Engine->IsIn(Engine->merchantdisplaySpecialItem[i], Engine->lastMousePos.x, Engine->lastMousePos.y))
			{
				getdata = i;
				yoklama = true;
				break;
			}
		}

		if (yoklama)
		{
			Engine->merchantItemsPriceLock.lock();
			foreach(it, Engine->merchantItemsPrice)
			{
				if (it->first != sSocketMerchantSpecial)
					continue;

				for (int i = 0; i < 8; i++)
				{

					if (it->second.bSrcPos[i] != getdata)
						continue;

					PriceType = it->second.isKC[i];
					Price = it->second.price[i];
					break;
				}
				if (Price > 0)
					break;
			}
			Engine->merchantItemsPriceLock.unlock();
			return true;
		}
	}

	return false;
}
clock_t recvTick2 = 0;
void __fastcall ThreadControlAlive()
{
	if (recvTick2 > clock() - 18000)
		return;

	recvTick2 = clock();

	if (CheckAliveTime < clock() - 18000)
	{
		//printf("THETHYKE | EXCEPTION 2\n");
		ischeatactive = true;
		Engine->StayAlive();

	}else if (Real_SendTime < clock() - 18000)
	{
		//printf("THETHYKE | EXCEPTION 3\n");
		ischeatactive = true;
		Engine->StayAlive();

	}
}

void __stdcall SetString_Hook(const std::string& szString)
{

	DWORD thisPtr;
	DWORD UIBase;
	__asm
	{
		MOV thisPtr, ECX
		MOV UIBase, ESI
	}

	bool test = false;
#if (HOOK_SOURCE_VERSION == 1098)	
	if (!Engine->m_bHookStart && Engine->Adress > 0 && !Engine->Loading && !isFalsed)		// 1098 KArakter Seçme ekraný ui pozisyonu ayarlanan yer
	{
		DWORD parenta;
		if (true)
		{

			Engine->GetChildByID(Engine->Adress, "Group_SelectWindow", parenta);
			DWORD KO_UI_SCREEN_SIZE = 0x00DE297C;
			POINT koScreen = *(POINT*)KO_UI_SCREEN_SIZE;
			RECT region = Engine->GetUiRegion(parenta);
			POINT orta;
			LONG w = (region.right - region.left);
			orta.x = (koScreen.x / 2) - 512;
			orta.y = region.top;
			Engine->SetUIPos(parenta, orta);

		}
		
		if (true)
		{
			DWORD parent;
			DWORD button;
			DWORD KO_UI_SCREEN_SIZE = 0x00DE297C;
			POINT koScreen = *(POINT*)KO_UI_SCREEN_SIZE;
			RECT region = Engine->GetUiRegion(parenta);
			Engine->GetChildByID(Engine->Adress, "Group_OtherCharacter", parent);
			Engine->GetChildByID(parenta, "btn_exit", button);
			POINT regiony = Engine->GetUiPos(button);
			regiony.x = (koScreen.x / 2) - 512;
			regiony.y -= 6;
			Engine->SetUIPos(parent, regiony);

		}
	}
#elif (HOOK_SOURCE_VERSION == 1534)	
	if (!Engine->m_bHookStart && Engine->Adress > 0 && !Engine->Loading && !isFalsed)			// 1534 KArakter Seçme ekraný ui pozisyonu ayarlanan yer
	{
		DWORD parenta;
		if (true)
		{

			Engine->GetChildByID(Engine->Adress, "Group_SelectWindow", parenta);
			DWORD KO_UI_SCREEN_SIZE = 0x00DE297C;
			POINT koScreen = *(POINT*)KO_UI_SCREEN_SIZE;
			RECT region = Engine->GetUiRegion(parenta);
			POINT orta;
			LONG w = (region.right - region.left);
			orta.x = (koScreen.x / 2) - 512;
			orta.y = koScreen.y - Engine->GetUiHeight(parenta);
			Engine->SetUIPos(parenta, orta);

		}

		if (true)
		{
			DWORD parent;
			Engine->GetChildByID(Engine->Adress, "Group_OtherCharacter", parent);
			DWORD KO_UI_SCREEN_SIZE = 0x00DE297C;
			POINT koScreen = *(POINT*)KO_UI_SCREEN_SIZE;
			RECT region = Engine->GetUiRegion(parent);
			POINT orta;
			LONG w = (region.right - region.left);
			orta.x = (koScreen.x / 2) - 512;
			orta.y = koScreen.y - Engine->GetUiHeight(parent) - 65;
			Engine->SetUIPos(parent, orta);

		}
	}
#endif

	m_strReplaceString = szString;
	if (Engine->m_bHookStart && Engine->uiState->m_strFreeStatPoint == thisPtr)
	{
		int FreePoint = std::stoi(m_strReplaceString.c_str());
		if (FreePoint < 1)
			Engine->uiState->UpdatePointButton(false);
		else 	if (FreePoint > 0)
			Engine->uiState->UpdatePointButton(true);
	}

	if (Engine->m_bHookStart)
	{
		if (Engine->Player.ZoneID == 30)
			Engine->str_replace(m_strReplaceString, "Mission termination time....", "Castle Siege War Time : ");

		//if (Engine->IsVisible(Engine->uiTradeInventory->m_dVTableAddr))
		//{
		//	if (thisPtr == Engine->uiTradeInventory->text_sell_money)
		//	{
		//		if (stoi(m_strReplaceString) > 0)
		//		{
		//			if (nisKC == 1)

		//			{
		//				std::string currentTotalKC = Engine->GetString(Engine->uiTradeInventory->text_sell_KnightCash).c_str();
		//				if (currentTotalKC.empty())currentTotalKC = "0";
		//				std::string currentTotalCoins = Engine->GetString(Engine->uiTradeInventory->text_sell_money).c_str();
		//				uint32 nCoinsPriceCoins = stoi(currentTotalCoins);
		//				uint32 nCoinsPriceKC = stoi(currentTotalKC);
		//				nCoinsPriceKC += RecvPrice;
		//				Engine->SetString(Engine->uiTradeInventory->text_sell_KnightCash, string_format("%s", Engine->number_format(nCoinsPriceKC).c_str()));
		//				m_strReplaceString = string_format("%s", Engine->number_format(nCoinsPriceCoins).c_str());

		//			}
		//			else if (nisKC == 0)
		//			{
		//				std::string currentTotalCoins = Engine->GetString(Engine->uiTradeInventory->text_sell_money).c_str();
		//				if (currentTotalCoins.empty())currentTotalCoins = "0";
		//				//	if(currentTotalCoins=="0")Engine->SetString(Engine->uiTradeInventory->text_sell_KnightCash,"0");
		//				uint32 nCoinsPriceCoins = stoi(currentTotalCoins);

		//				nCoinsPriceCoins += RecvPrice;

		//				m_strReplaceString = string_format("%s", Engine->number_format(nCoinsPriceCoins).c_str());

		//			}
		//		}
		//	}
		//}
	}

	//PREMIUM CLAN yazýsýný clan premiuma çevirdik
	if (Engine->StringHelper->IsContains(m_strReplaceString, xorstr("PREMIUM")) && Engine->m_bHookStart) {
		
		if (Engine->GetStringElementName(thisPtr) == "text" && !Engine->StringHelper->IsContains(m_strReplaceString, xorstr("PC CAFE")))
		{
			DWORD nbase;
			std::string find = "btn_premium";
			Engine->GetChildByID(UIBase, find, nbase);
			find = "btn_clanpremium";
			Engine->GetChildByID(nbase, find, nbase);
			Engine->SetVisible(nbase, false);
		}
	}
	//*************************************

	for (auto a : Engine->privatemessages)
		if (thisPtr == a->exit_id)
		{
			a->user_id = m_strReplaceString;
			a->m_bcolored = true;
		}
	
	if (Engine->StringHelper == NULL)
		Engine->StringHelper = new CStringHelper();

	if (Engine->uiGenieSubPlug != NULL && Engine->uiGenieSubPlug->m_textGenieTime == thisPtr)
	{
		m_strReplaceString = string_format("Time Left : %s Hour(s)", m_strReplaceString.c_str());
	}
	if (!Engine->m_bHookStart) {
		if (Engine->m_bGameStart || Engine->StringHelper->IsContains(m_strReplaceString, xorstr("Loading Information[")))
			StartHook();
	}

	for (string larr : loadingArray) 
	{
		if (Engine->StringHelper->IsContains(m_strReplaceString, larr))
		{
			Engine->Loading = true;
			test = true;
		}
	}

	if (Engine->m_SettingsMgr != NULL)
	{
		string ServerIniVer = Engine->m_SettingsMgr->ClientVersion;
		if (ServerIniVer == "0")
		{
			Engine->Loading = false;
		}
		
		string Ver = "Ver. ";
		string ValidVersion = Ver + "" + ServerIniVer.substr(0, 1) + "." + ServerIniVer.substr(1);
		if (m_strReplaceString == ValidVersion)
		{
			Engine->Loading = true;
			test = true;
			Engine->LoadingControl = true;
		}
	}
	else
	{
		Engine->Loading = true;
		test = true;
		Engine->LoadingControl = true;
	}

	if (Engine->LoqOut || Engine->Player.isTeleporting)
	{
		Engine->Loading = true;
		test = true;
	}

	if (!test && !Engine->LoadingControl)
		Engine->Loading = false;

	if (Engine->uiHpMenuPlug != NULL)
		Engine->uiHpMenuPlug->UpdatePosition();
	if (Engine->m_bGameStart)
	{
		if (Engine->uiHpMenuPlug != NULL && Engine->IsVisible(Engine->uiHPBarPlug->m_dVTableAddr) && !Engine->IsVisible(Engine->uiHpMenuPlug->m_dVTableAddr))
		{
			Engine->uiHpMenuPlug->OpenHpBarMenu();
			Engine->m_UiMgr->ShowAllUI();
		}

		if (Engine->m_MerchantMgr != NULL)
		{
			if (Engine->m_MerchantMgr->NeedToCheckTooltip() || Engine->m_MerchantMgr->NeedToCheckDisplayTooltip())
			{
				if (Engine->StringHelper->IsContains(m_strReplaceString, purchasingPriceSearch))
				{
					if (Engine->m_MerchantMgr->UpdateTooltipString(m_strReplaceString)) // kc ise renk deðiþtir
						Engine->SetStringColor(thisPtr, 0xff7a70);
				}
			}
		}

		if ((Engine->merchantdisplaydata > 0 && Engine->IsVisible(Engine->merchantdisplaydata)) 
			|| (Engine->merchantdisplaySpecialdata > 0 && Engine->IsVisible(Engine->merchantdisplaySpecialdata)))
		{
			if (Engine->str_contains(m_strReplaceString, purchasingPriceSearch) && !Engine->m_MerchantMgr->IsMerchanting())
			{
				uint32 nPrice;
				uint8 isKC;
				if (MerchantPriceView(isKC, nPrice))
				{
					if (isKC)
					{
						m_strReplaceString = string_format("Knight Cash Price : %s", Engine->number_format(nPrice, false).c_str());
					
					}
					else
						m_strReplaceString = string_format("Purchasing Price : %s", Engine->number_format(nPrice, false).c_str());
				}
			}
		}
	}

	_asm
	{
		MOV ECX, thisPtr
		MOV EAX, OFFSET m_strReplaceString

		PUSH EAX
		CALL KO_SET_STRING_FUNC
		call ThreadControlAlive
	}
}

void PearlEngine::InitSetString()
{
	*(DWORD*)KO_SET_STRING_PTR = (DWORD)SetString_Hook;
}

void __declspec(naked) IsInAsm()
{
	_asm
	{
		MOV ECX, [m_dvTable]
		MOV EAX, m_iParam2
		PUSH EAX
		MOV EAX, m_iParam1
		PUSH EAX
		CALL KO_UIBASE_IS_IN_FUNC
		MOV m_dBool, EAX
		RET
	}
}


void PearlEngine::SetItemFlag(DWORD vTable,uint8 flag)
{
	DWORD tmp = vTable;
	if (!tmp) return;
	__asm {
		mov ecx, tmp
		movsx eax, flag
		push eax
		call KO_UIF_SET_ITEM_FLAG
	}
}

bool PearlEngine::IsIn(DWORD vTable, int x, int y)
{
	if (vTable == 0)
		return false;


	POINT pt = GetUiPos(vTable);
	LONG w = GetUiWidth(vTable);
	LONG h = GetUiHeight(vTable);

	RECT rp;
	rp.left = pt.x;
	rp.right = pt.x + w;
	rp.top = pt.y;
	rp.bottom = pt.y + h;

	return lastMousePos.x >= rp.left && lastMousePos.x <= rp.right && lastMousePos.y >= rp.top && lastMousePos.y <= rp.bottom;

}

bool PearlEngine::fileExist(const char* fileName)
{
	std::ifstream infile(string(m_BasePath + fileName).c_str());
	return infile.good();
}

LONG PearlEngine::GetUiWidth(DWORD vTable)
{
	uintptr_t** ptrVtable = (uintptr_t**)vTable;
	LONG right = (LONG)ptrVtable[61];
	LONG left = (LONG)ptrVtable[59];

	return right - left;
}

LONG PearlEngine::GetUiHeight(DWORD vTable)
{
	uintptr_t** ptrVtable = (uintptr_t**)vTable;
	LONG bottom = (LONG)ptrVtable[62];
	LONG top = (LONG)ptrVtable[60];

	return bottom - top;
}

size_t PearlEngine::GetScrollValue(DWORD vTable)
{
	DWORD track = *(DWORD*)(vTable + 0x134);
	return track ? *(uint32*)(track + 0x144) : 0;
}

bool PearlEngine::IsitaSandBox()
{
	unsigned char bBuffering;
	unsigned long aCreateProcesses = (unsigned long)GetProcAddress(GetModuleHandleA(xorstr("KERNEL32.dll")), xorstr("CreateProcessA"));

	ReadProcessMemory(GetCurrentProcess(), (void*)aCreateProcesses, &bBuffering, 1, 0);

	if (bBuffering == 0xE9)
		return  1;
	else
		return 0;
}

bool PearlEngine::IsHWBreakpointExists()
{
	CONTEXT ctx;
	ZeroMemory(&ctx, sizeof(CONTEXT));
	ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	HANDLE hThread = GetCurrentThread();
	if (GetThreadContext(hThread, &ctx) == 0)
		return false;

	if ((ctx.Dr0) || (ctx.Dr1) || (ctx.Dr2) || (ctx.Dr3))
		return true;
	else
		return false;
}

DWORD KO_ADR_CHR = 0x0;
DWORD KO_ADR_DLG = 0x0;

BYTE ReadByte(DWORD vTable)
{
	return *(byte*)vTable;
}

long ReadLong(DWORD vTable)
{
	return *(long*)vTable;
}

typedef void(__thiscall* tSetCurValue)(DWORD uiObject, float a1, float a2, float a3);
tSetCurValue Func_SetCurValue = (tSetCurValue)0x424890;

void PearlEngine::SetCurValue(DWORD dwTable, float value, float changeSpeed)
{
	Func_SetCurValue(dwTable, value, 0, changeSpeed);
}

void PearlEngine::SetProgressRange(DWORD uif, float min, float max)
{
	DWORD tmp = uif;
	*(DWORD*)(uif + 0x150) = min;
	*(DWORD*)(uif + 0x14C) = max;
	__asm {
		mov eax, [tmp]
		mov eax, [eax]
		mov eax, [eax + 0xB0]
		mov ecx, tmp
		call eax
	}
}

void PearlEngine::ChangeRunState()
{
	__asm {
		mov ecx, [KO_PTR_CHR]
		mov ecx, [ecx]
		mov eax, 0x00567E50
		call eax
	}
}

void PearlEngine::CommandCameraChange()
{
	__asm {
		push -1
		mov ecx, [0xF368E0]
		mov ecx, [ecx]
		mov eax, 0x00A7C300
		call eax
	}
}


long PearlEngine::SelectRandomMob()
{
	__asm {
		MOV ECX, KO_DLG
		MOV ECX, DWORD PTR DS : [ECX]
		MOV EAX, KO_FNCZ
		CALL EAX
	}
}

string PearlEngine::number_format(int dwNum, bool dot)
{
	std::string value = std::to_string(dwNum);
	int len = value.length();
	int dlen = 3;

	while (len > dlen)
	{
		value.insert(len - dlen, 1, dot ? '.' : ',');
		dlen += 4;
		len += 1;
	}

	return value;
}

void PearlEngine::str_tolower(std::string& str)
{
	for (size_t i = 0; i < str.length(); ++i)
		str[i] = (char)tolower(str[i]);
}

bool PearlEngine::str_contains(std::string str, std::string find)
{
	std::string s = str;
	str_tolower(s);

	std::string f = find;
	str_tolower(f);

	if (s.find(f) != std::string::npos)
		return true;
	return false;
}
std::string PearlEngine::GetItemDXT(int nItemID)
{

	TABLE_ITEM_BASIC* tbl = Engine->tblMgr->getItemData(nItemID);
	if (tbl != nullptr)
	{


		std::vector<char> buffer(256, NULL);

		sprintf(&buffer[0], "ui\\itemicon_%.1d_%.4d_%.2d_%.1d.dxt",
			(tbl->dxtID / 10000000),
			(tbl->dxtID / 1000) % 10000,
			(tbl->dxtID / 10) % 100,
			tbl->dxtID % 10);

		std::string szIconFN = &buffer[0];

		return szIconFN;
	}
	else
		return "UI\\itemicon_noimage.dxt";
}
void PearlEngine::str_replace(std::string& str, std::string find, std::string replace)
{
	if (find.empty())
		return;

	size_t start_pos = 0;
	while ((start_pos = str.find(find, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, find.length(), replace);
		start_pos += replace.length();
	}
}

void PearlEngine::str_replace(std::string& str, char find, char replace)
{
	std::replace(str.begin(), str.end(), (char)25, (char)39); // replace all 'x' to 'y'
}

void PearlEngine::str_split(std::string const& str, const char delim, std::vector<std::string>& out)
{
	size_t start;
	size_t end = 0;

	while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
	{
		end = str.find(delim, start);
		out.push_back(str.substr(start, end - start));
	}
}

void PearlEngine::str_split(std::string str, std::string delim, std::vector<std::string>& out)
{
	size_t pos_start = 0, pos_end, delim_len = delim.length();
	std::string token;

	while ((pos_end = str.find(delim, pos_start)) != std::string::npos)
	{
		token = str.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		out.push_back(token);
	}

	out.push_back(str.substr(pos_start));
}

std::vector<std::string> PearlEngine::str_split(std::string str, std::string regex)
{
	std::smatch m;
	std::regex e(regex);
	std::vector<std::string> ret;

	while (std::regex_search(str, m, e))
	{
		for (string x : m)
			ret.push_back(x);
		str = m.suffix().str();
	}

	return ret;
}

std::string PearlEngine::KoRandomNameUIF(std::string uif)
{
	srand((unsigned)time(NULL) * getpid());
	int random1 = rand() % 10000;
	int random2 = rand() % 100;
	std::string text = string_format("%s_%d%d.uif", uif.c_str(), random1, random2);
	return text;
}

extern HCRYPTKEY hKey;

std::string PearlEngine::dcpUIF(std::string path)
{
	std::string rtn = "";
	if (!path.empty())
	{
		rtn = KoRandomNameUIF(path);
		PBYTE pbBuffer = NULL;
		HANDLE hSourceFile = CreateFileA(path.c_str(), FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		HANDLE hDestinationFile = CreateFileA(rtn.c_str(), FILE_WRITE_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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
	return rtn;
}

typedef void(__thiscall* tSetEditString)(DWORD ecx, const std::string& szString);
tSetEditString __SetEditString = (tSetEditString)0x4189A0;
typedef const std::string& (__thiscall* tGetEditString)(DWORD ecx);
tGetEditString __GetEditString = (tGetEditString)0x416960;

std::string PearlEngine::GetEditString(DWORD vTable)
{
	return __GetEditString(vTable);
}

void PearlEngine::SetEditString(DWORD vTable, const std::string& szString)
{
	__SetEditString(vTable, szString);
}

void ShowToolTip(__IconItemSkillEx* spItem, LONG x, LONG y)
{
	DWORD tooltipUI = 0;
	__asm {
		mov ecx, [KO_UI_DLG]
		mov ecx, [ecx]
		mov tooltipUI, ecx
	}

	int _type = 0;
	LONG _x = x, _y = y;

	__asm {
		push 0
		push 0
		push 1
		push 0 // istedigi np gosterilsin mi?
		push 1 
		push 1 // 1 selling price // 2 buying price
		push 0 // show price
		push _type // inventory / merchant / drop hangisiysey onu yazar
		push spItem
		push _y
		push _x
		mov ecx, tooltipUI
		call KO_UI_TOOLTIP_FNC
	}
}

void PearlEngine::_ShowToolTip(DWORD spItem, LONG x, LONG y, TOOLTIP_TYPE type)
{
	DWORD tooltipUI = 0;
	__asm {
		mov ecx, [KO_UI_DLG]
		mov ecx, [ecx]
		mov tooltipUI, ecx
	}

	int _type = (int)type;
	LONG _x = x, _y = y;

	__asm {
		push 0
		push 0
		push 1
		push 0 // istedigi np gosterilsin mi
		push 0
		push 1 // 1 = selling price, 2 = buy price
		push 0 // show price
		push _type // tip ? yani inventory, merchant, drop
		push spItem
		push _y
		push _x
		mov ecx, tooltipUI
		call KO_UI_TOOLTIP_FNC
	}
}

void __declspec(naked) SetN3UIIcon(DWORD* uif)
{
	__asm {
		push ebp
		mov ebp, esp
		push esi
		mov esi, [ebp + 8]
		mov ecx, [esi]
		call KO_UIF_SET_N3ICON
		mov[esi], eax
		pop esi
		pop ebp
		ret
	}
}

void __declspec(naked) SetUVRect(DWORD uif, float left, float top, float right, float bottom)
{
	__asm {
		push ebp
		mov ebp, esp
		movss xmm0, [ebp + 0x18]
		sub esp, 0x20
		mov ecx, [ebp + 8]
		mov[esp + 0x1C], 0
		mov[esp + 0x18], 0
		mov[esp + 0x14], 0
		mov[esp + 0x10], 0
		movss[esp + 0xC], xmm0
		movss xmm0, [ebp + 0x14]
		movss[esp + 8], xmm0
		movss xmm0, [ebp + 0x10]
		movss[esp + 4], xmm0
		movss xmm0, [ebp + 0xC]
		movss[esp], xmm0
		call dword ptr[KO_UIF_IMG_SETUVRECT]
		pop ebp
		ret
	}
}

DWORD __fastcall __new(size_t allocSize)
{
	DWORD result = 0;
	try {
		if (result = (DWORD)malloc(allocSize)) memset((void*)result, 0, allocSize);
	}
	catch (const std::bad_alloc&) {
		return NULL;
	}
	return result;
}

void SetParent(DWORD uif, DWORD parent)
{
	__asm {
		mov ecx, uif
		mov eax, [ecx]
		mov eax, [eax + 0x80]
		push parent
		call eax
	}
}

extern TABLE_ITEM_BASIC* GetItemBase(DWORD ItemID);
extern TABLE_ITEM_EXT* GetItemExtBase(DWORD dwItemNum, DWORD bExtIndex);

DWORD InitIcon(DWORD itemID)
{
	DWORD parent = *(DWORD*)KO_UI_MGR;
	CItem* item = (CItem*)GetItemBase(itemID);
	if (!item) return false;
	CItemExt* itemExt = (CItemExt*)GetItemExtBase(itemID, item->extNum);
	DWORD dwIconID = item->dxtID;
	if (itemExt && itemExt->iconID > 1000000)
		dwIconID = itemExt->iconID;

	std::string dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);

	DWORD icon = __new(0x2A0);
	SetN3UIIcon(&icon);
	SetParent(icon, parent);
	Engine->SetTexImage(icon, dxt);
	SetUVRect(icon, 0.0f, 0.0f, 0.703125f, 0.703125f);
	*(DWORD*)(icon + 0xDC) = 11;
	__asm {
		mov ecx, icon
		push 0x110
		mov eax, [ecx]
		mov eax, [eax + 0x4C]
		call eax
		mov ecx, icon
		push 0
		mov eax, [ecx]
		mov eax, [eax + 0x50]
		call eax
	}
	return icon;
}

bool PearlEngine::_InitIcon(DWORD& iconObject, DWORD parent, DWORD itemID)
{
	CItem* item = (CItem*)GetItemBase(itemID);
	if (!item)
		return false;

	CItemExt* itemExt = (CItemExt*)GetItemExtBase(itemID, item->extNum);
	if (!itemExt)
		return false;

	DWORD dwIconID = item->dxtID;
	if (itemExt->dxtID > 0)
		dwIconID = itemExt->dxtID;


	//std::string dxt = string_format(xorstr("UI\\Itemicon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);

	std::string dxt = "";
	if (itemID / 1000000000 - 1 > 2)
		dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);
	else
		dxt = string_format(xorstr("UI\\ItemIcon_%.2d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);

	if (iconObject)
	{
		DWORD icon = iconObject;
		SetParent(icon, parent);
		SetTex(icon, dxt);
		SetUVRect(icon, 0.0f, 0.0f, 0.703125f, 0.703125f);
		__asm {
			mov ecx, icon
			push 1
			mov eax, [ecx]
			mov eax, [eax + 0x50]
			call eax
		}

		iconObject = icon;
		return true;
	}

	DWORD icon = __new(0x2A0);
	SetN3UIIcon(&icon);
	SetParent(icon, parent);
	Engine->SetTexImage(icon, dxt);
	SetUVRect(icon, 0.0f, 0.0f, 0.703125f, 0.703125f);
	*(DWORD*)(icon + 0xDC) = 11; // SetUIType(UI_TYPE_ICON);
	__asm { // SetStyle(UISTYLE_ICON_ITEM|UISTYLE_ICON_CERTIFICATION_NEED);
		mov ecx, icon
		push 0x110
		mov eax, [ecx]
		mov eax, [eax + 0x4C]
		call eax
		// -------------- setvisible true
		mov ecx, icon
		push 1
		mov eax, [ecx]
		mov eax, [eax + 0x50]
		call eax
	}
	iconObject = icon;
	return true;
}


void __declspec(naked) SetIconItemSkill(__IconItemSkillEx* uif)
{
	__asm {
		push ebp
		mov ebp, esp
		mov ecx, [ebp + 8]
		call KO_UIF_ICONITEMSKILL_SET
		pop ebp
		ret
	}
}

void PearlEngine::SetRegion(DWORD dwTable, DWORD uif)
{
	if (!dwTable)return;
	DWORD tmpTp = dwTable;
	DWORD otherRect = uif + UI_POS;
	__asm {
		mov ecx, tmpTp
		mov eax, [ecx]
		mov eax, [eax + 0x38]
		push otherRect
		call eax
	}
	*(RECT*)(dwTable + 0xFC) = *(RECT*)(dwTable + UI_POS);
}

void __declspec(naked) _SetIconItemSkill(DWORD* uif)
{
	__asm {
		push ebp
		mov ebp, esp
		push esi
		mov esi, [ebp + 8]
		mov ecx, [esi]
		call KO_UIF_ICONITEMSKILL_SET
		pop esi
		pop ebp
		ret
	}
}

void __declspec(naked) _IntoString(DWORD dwTable, std::string& szString)
{
	__asm {
		push ebp
		mov ebp, esp
		mov ecx, [ebp + 8]
		push[ebp + 0xC]
		call KO_UIF_LOADSTRING
		pop ebp
		ret
	}
}

DWORD PearlEngine::_CreateIconItemSkill(DWORD self, DWORD UIIcon, DWORD itemID, short count, uint32 expiration, uint16 rental_remTime, uint32 rental_period, uint32 rental_price)
{
	CItem* item = (CItem*)GetItemBase(itemID);
	if (!item)
		return NULL;

	CItemExt* itemExt = (CItemExt*)GetItemExtBase(itemID, item->extNum);
	if (!itemExt)
		return NULL;

	DWORD dwIconID = item->dxtID;
	if (itemExt && itemExt->iconID > 1000000)
		dwIconID = itemExt->dxtID;

	std::string dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);

	DWORD spItem = self;
	if (!spItem) {
		DWORD tmp = __new(0x78);
		if (tmp)
		{
			__asm {
				mov ecx, tmp
				call KO_UIF_ICONITEMSKILL_CREATE
				mov spItem, eax
			}
		}
	}

	if (!spItem) return NULL;

	__asm {
		mov ecx, spItem
		call KO_UIF_ICONITEMSKILL_SET
	}

	if (itemID == itemID)
		count = 0;

	_SetIconItemSkill(&spItem);
	_IntoString(spItem + KO_UIF_ICONITEMSKILL_OFF_DXT, dxt);
	*(uint16*)(spItem + KO_UIF_ICONITEMSKILL_OFF_RENT_TIME) = rental_remTime; // rental remaining time uint16
	*(uint32*)(spItem + KO_UIF_ICONITEMSKILL_OFF_RENT_PREIOD) = rental_period; // rental period minute uint32
	*(uint32*)(spItem + KO_UIF_ICONITEMSKILL_OFF_RENT_PRICE) = rental_price; // rental price uint32
	*(uint32*)(spItem + KO_UIF_ICONITEMSKILL_OFF_EXPIRATION) = expiration; // expiration time
	*(DWORD*)(spItem + KO_UIF_ICONITEMSKILL_OFF_ITEM) = (DWORD)item;
	*(DWORD*)(spItem + KO_UIF_ICONITEMSKILL_OFF_ITEM_EXT) = (DWORD)itemExt;
	*(short*)(spItem + KO_UIF_ICONITEMSKILL_OFF_ITEM_COUNT) = count;
	*(short*)(spItem + KO_UIF_ICONITEMSKILL_OFF_ITEM_DURATION) = item->Duration + itemExt->siMaxDurability;
	*(DWORD*)spItem = UIIcon;
	return spItem;
}

__IconItemSkillEx* CreateIconItemSkill(DWORD UIIcon, DWORD itemID, short count, uint32 expiration, uint16 rental_remTime, uint32 rental_period, uint32 rental_price)
{
	CItem* item = (CItem * )GetItemBase(itemID);
	if (!item)
		return NULL;

	CItemExt* itemExt = (CItemExt *)GetItemExtBase(itemID, item->extNum);

	DWORD dwIconID = item->dxtID;
	if (itemExt && itemExt->iconID > 1000000)
		dwIconID = itemExt->iconID;

	std::string dxt = string_format(xorstr("UI\\ItemIcon_%.1d_%.4d_%.2d_%.1d.dxt"), dwIconID / 10000000u, dwIconID / 1000u % 10000, dwIconID / 10u % 100, dwIconID % 10);

	__IconItemSkillEx* spItem = NULL;

	DWORD tmp = __new(0x78);
	if (tmp)
	{
		__asm {
			mov ecx, tmp
			call KO_UIF_ICONITEMSKILL_CREATE
			mov spItem, eax
		}
	}

	if (!spItem) return NULL;

	__asm {
		mov ecx, spItem
		call KO_UIF_ICONITEMSKILL_SET
	}

	SetIconItemSkill(spItem);

	spItem->pUIIcon = UIIcon;
	spItem->szIconFN = dxt;
	spItem->iRentTime = rental_remTime;
	spItem->iRentPeriod = rental_period;
	spItem->iRentPrice = rental_price;
	spItem->iExpiration = expiration;
	spItem->pItemBasic = item;
	spItem->pItemExt = itemExt;
	spItem->iCount = count;
	spItem->iDurability = item->Duration + (itemExt ? itemExt->siMaxDurability : 0);
	return spItem;
}

std::map <uint32, __IconItemSkillEx*> iconItemMap;

void PearlEngine::ShowToolTipEx(uint32 itemID, LONG x, LONG y)
{
	auto itr = iconItemMap.find(itemID);
	if (itr != iconItemMap.end())
	{
		ShowToolTip(itr->second, x, y);
		return;
	}

	if (DWORD icon = InitIcon(itemID))
	{
		if (__IconItemSkillEx* spItem = CreateIconItemSkill(icon, itemID, 1, 0, 0, 0, 0))
		{
			iconItemMap.insert(std::make_pair(itemID, spItem));
			ShowToolTip(spItem, x, y);
		}
	}
}