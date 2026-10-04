#include "PearlGui.h"
#include "Timer.h"
#include "resource.h"
#include <future>
#include "CSpell.h"
#include "FunctionGuard.h"
#include "../resource.h"
using namespace std;
extern bool ischeatactive;
HRESULT WINAPI HookCreateDevice();

#define HOOK(func,addy)	o##func = (t##func)DetourFunction((PBYTE)addy,(PBYTE)hk##func)
#define D3D_RELEASE(D3D_PTR) if( D3D_PTR ){ D3D_PTR->Release(); D3D_PTR = NULL; }
#define ES	0
#define DIP	1
#define RES 2
#define FOX 3
HRESULT WINAPI hkReset(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
typedef HRESULT(WINAPI* tEndScene)(LPDIRECT3DDEVICE9);
typedef HRESULT(WINAPI* tReset)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
typedef HRESULT(WINAPI* mySendMessage) (HWND, UINT, WPARAM, LPARAM);

typedef HRESULT(WINAPI* CreateDevice_t)(IDirect3D9* Direct3D_Object, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow,
	DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters,
	IDirect3DDevice9** ppReturnedDeviceInterface);
HRESULT WINAPI D3DCreateDevice_hook(IDirect3D9 * Direct3D_Object, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow,
	DWORD BehaviorFlags, D3DPRESENT_PARAMETERS * pPresentationParameters,
	IDirect3DDevice9 * *ppReturnedDeviceInterface);

CreateDevice_t D3DCreateDevice_orig;

typedef HRESULT(WINAPI* tSetScissorRect)(LPDIRECT3DDEVICE9 pDevice, D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9* ppSB);
tSetScissorRect  oSetScissorRect;

HRESULT WINAPI SetScissorRect(LPDIRECT3DDEVICE9 pDevice, D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9* ppSB)
{
	HRESULT ret = oSetScissorRect(pDevice, Type, ppSB);
	//printf("THETHYKE | EXCEPTION 4\n");
	ischeatactive = true;
	return ret;
}
PDWORD IDirect3D9_vtable = NULL;
#define CREATEDEVICE_VTI 16

DWORD VTable[3] = { 0 };
DWORD D3DEndScene;
DWORD D3DReset;
tReset oReset;
tEndScene oPresent;
tEndScene oEndScene;
WNDPROC oWndProc;
static int myfpslimit = 60;
static bool isfpslimit = false;
static bool sendHooked = false;
static bool reset = false;
static bool itsMe = false;

LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL;
LPDIRECT3DINDEXBUFFER9  g_pIB = NULL;

HWND window = 0;
NOTIFYICONDATA nid;
bool hiddenWindow = false;

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	

	switch (uMsg)
	{
	case WM_APP + 1:
		switch (lParam)
		{
		case WM_LBUTTONDBLCLK:
			if (Engine)
				if (!IsWindowVisible(window)) {
					ShowWindow(window, SW_SHOW);
					hiddenWindow = false;
				}
			break;
		}
		break;
	}
	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
	//return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HRESULT WINAPI hkReset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* Present)
{
	if (Engine->m_SettingsMgr == NULL) 
		Engine->m_SettingsMgr = new CSettingsManager();

	Present->Windowed = Engine->m_SettingsMgr->m_iRealFullScreen == 1 ? false : true;
	Present->PresentationInterval = Engine->m_SettingsMgr->m_iVsync == 1 ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
	
	if (Engine->m_SettingsMgr->m_iVsync == 1)
		Present->FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	reset = true;
	return oReset(pDevice, Present);
}

extern CSpell* GetSkillBase(int iSkillID);
DWORD adresdd = 0x33C;

ULONGLONG thtime = GetTickCount64();
HRESULT WINAPI hkEndScene(LPDIRECT3DDEVICE9 pDevice)
{
	/*if (GetAsyncKeyState(VK_RETURN) & 1 && Engine->Adress > 0)
	{
		POINT pt;
		pt.x = 50;
		pt.y = 50;
		Engine->m_UiMgr->SendMouseProc(UI_MOUSE_LBCLICK, pt, pt);
	}*/

	if (GetAsyncKeyState(VK_F1) & 1)
	{
		//vector<int>offsets;
		//offsets.push_back(0x518); //0x518
		//offsets.push_back(0);

		//DWORD m_dVTableAddr = Engine->rdword(KO_DLG, offsets);
		//Engine->SetVisible(m_dVTableAddr, true);
		//Engine->UIScreenCenter(m_dVTableAddr);
		//Engine->AddListString()
	}

	if(!Engine->render)
		return oEndScene(pDevice);

	if (!Engine->power)
		return (BYTE)0x90;
	
	
	if (GetAsyncKeyState(VK_F12) & 1)
	{
		if (GetForegroundWindow() == window) {
			ShowWindow(window, false);
			hiddenWindow = true;
		}
		//Engine->m_UiMgr->uiTagChange->OpenTagChange();
	}

	if (hiddenWindow) Sleep(1000 / 60); //f12 basinca cpu düþürme

	if (!sendHooked) 
	{
		sendHooked = true;
		Engine->m_UiMgr = new CUIManager();
		Engine->m_UiMgr->Init(pDevice);
		Engine->InitSendHook();
		Engine->InitRecvHook();
		Engine->InitSetString();

		for (uint32 i = 101001; i <= 610159; i++)
		{
			CSpell* spell = GetSkillBase(i);
			if (spell) {
				Engine->skillmap.insert({ spell->dwID, *spell });
				SpellCRC crc(crc32((uint8*)(DWORD)spell, 0xA8, -1), crc32((uint8*)((DWORD)spell + 0xB4), 0x2C, -1));
				Engine->skillcrc.insert({ spell->dwID + 2031, crc });
				Engine->skillmapBackup.insert({ spell->dwID, *spell });
			}
		}
	}

	if (reset)
	{
		reset = false;
	
		if (Engine->uiTaskbarMain) Engine->uiTaskbarMain->UpdatePosition();
	}


	if (!Engine->drawMode || Engine->Loading == true)
		return oEndScene(pDevice);

	if (Engine->uiLogin == NULL)
	{
		Engine->uiLogin = new CUILogin();
		if (Engine->uiLogin->m_bGroupLogin == NULL)
			Engine->uiLogin = NULL;
	}

	//if (Engine->uiHpMenuPlug != NULL
	//	&& !Engine->IsVisible(Engine->uiHpMenuPlug->m_dVTableAddr)
	//		&& Engine->IsVisible(Engine->uiHPBarPlug->m_dVTableAddr))
	//		Engine->uiHpMenuPlug->OpenTopLeft();

	//if (Engine->uiHpMenuPlug != NULL
	//	&& Engine->IsVisible(Engine->uiHpMenuPlug->m_dVTableAddr)
	//	&& !Engine->IsVisible(Engine->uiHPBarPlug->m_dVTableAddr))
	//	Engine->SetVisible(Engine->uiHpMenuPlug->m_dVTableAddr, false);

	/*if (Engine->uiAnvil != NULL)
		Engine->uiAnvil->Tick();*/

	if (Engine->uiSeedHelperPlug != NULL)
		Engine->uiSeedHelperPlug->Tick();

	//if (Engine->uiPieceChangePlug != NULL)
	//	Engine->uiPieceChangePlug->Tick();

	if (Engine->uiState != NULL)
		Engine->uiState->Tick();

	if (Engine->uiTradePrice != NULL)
		Engine->uiTradePrice->Tick();

	if (Engine->uiSkillPage != NULL)
		Engine->uiSkillPage->Tick();

	if (Engine->uiScoreBoard) Engine->uiScoreBoard->Tick();

	if (Engine->m_UiMgr != NULL) {
		Engine->m_UiMgr->Tick();
		Engine->m_UiMgr->Render();
	}

	if (GetAsyncKeyState(VK_F1) & 1)
	{

		//Engine->m_UiMgr->uiTopLeft->FPS();
	}
	if (Engine->uiLottery != NULL)
		Engine->uiLottery->Tick();

	if (Engine->uiCindirella != NULL)
		Engine->uiCindirella->Tick();

	if (Engine->uiHPBarPlug != NULL)
		Engine->uiHPBarPlug->Tick();

	if (Engine->uiWheel != NULL)
		Engine->uiWheel->Tick();

	if (Engine->IsCRActive && Engine->uiCollection != NULL)
		Engine->uiCollection->Tick();

	if (GetTickCount64() > thtime) {
		if (Engine->MainThread) ResumeThread(Engine->MainThread);
		if (Engine->AliveThread) ResumeThread(Engine->AliveThread);
		if (Engine->ScanThread) ResumeThread(Engine->ScanThread);
		if (Engine->TitleThread) ResumeThread(Engine->TitleThread);
		thtime = GetTickCount64() + 1000;
	}

	//if (Engine->m_UiMgr->uixDailyQuest != NULL)
	//	Engine->m_UiMgr->uixDailyQuest->Tick();
	
	if (Engine->uiEventShowList != NULL)
		Engine->uiEventShowList->Tick();

	if (Engine->uiQuestPage != NULL)
		Engine->uiQuestPage->Tick();

	if (Engine->uiGenieMain != NULL)
		Engine->uiGenieMain->Tick();

	return oEndScene(pDevice);
}

LRESULT CALLBACK MsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void DX_Init(DWORD * table)
{
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, xorstr("DX"), NULL };
	RegisterClassEx(&wc);
	HWND hWnd = CreateWindow(xorstr("DX"), NULL, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, GetDesktopWindow(), NULL, wc.hInstance, NULL);
	LPDIRECT3D9 pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	LPDIRECT3DDEVICE9 pd3dDevice;
	itsMe = true;
	pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &d3dpp, &pd3dDevice);
	itsMe = false;
	DWORD * pVTable = (DWORD*)pd3dDevice;
	pVTable = (DWORD*)pVTable[0];

	table[FOX] = pVTable[17];
	table[ES]  = pVTable[42];
	table[DIP] = pVTable[82];
	table[RES] = pVTable[16];

	DestroyWindow(hWnd);
}
DWORD FindDevice(DWORD Len)
{
	DWORD dwObjBase = 0;

	dwObjBase = (DWORD)LoadLibraryA("d3d9.dll");
	while (dwObjBase++ < dwObjBase + Len)
		if ((*(WORD*)(dwObjBase + 0x00)) == 0x06C7 && (*(WORD*)(dwObjBase + 0x06)) == 0x8689 && (*(WORD*)(dwObjBase + 0x0C)) == 0x8689) {
			dwObjBase += 2; break;
		}

	return(dwObjBase);
}

DWORD GetDeviceAddress(int VTableIndex)
{
	PDWORD VTable;
	*(DWORD*)&VTable = *(DWORD*)FindDevice(0x128000);
	return VTable[VTableIndex];
}
DWORD WINAPI InitGUI()
{

	while (GetModuleHandle(xorstr("d3d9.dll")) == NULL) 
	{
		Sleep(250);
	}

	//HookCreateDevice(); //Fps sorunu burdan kaynaklanýyor kapattýk sorun çýkarsa açýcaz 24.01.2021

	DX_Init(VTable);

	HOOK(EndScene, VTable[ES]);
	HOOK(Reset, VTable[RES]);

	char buff[50];
	ZeroMemory(buff, 50);
	sprintf_s(buff, "ISTIRAP Client[%d]", GetCurrentProcessId()); //old version client name change
	oSetScissorRect = (tSetScissorRect)DetourFunction((PBYTE)GetDeviceAddress(59), (PBYTE)SetScissorRect);

	window = NULL;
	while (window == NULL) {
		Sleep(100);
		window = FindWindowA(NULL, buff); //old version client name change

	}

	oWndProc = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)WndProc);
	std::memset(&nid, 0, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = window;
	nid.uID = 0;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_APP + 1;
	nid.hIcon = LoadIcon(GetModuleHandle("Nx.ACS"), MAKEINTRESOURCE(IDI_ICON1));
	lstrcpy(nid.szTip, buff);
	Shell_NotifyIcon(NIM_ADD, &nid);
	Shell_NotifyIcon(NIM_SETVERSION, &nid);

	return 0;
}


void UIMain() 
{
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)InitGUI, NULL, NULL, NULL);
}


BYTE ByteExt(DWORD ulBase)
{
	if (!IsBadReadPtr((VOID*)ulBase, sizeof(BYTE)))
	{
		return(*(BYTE*)(ulBase));
	}
	return 0;
}

const DWORD KO_MULTI_CAP = 0xC7C25C;// 0x00bb4c64;
const DWORD KO_MUTEX = 0xA84C91;// 0x00009DFE01;
DWORD WINAPI MultiPatch()
{
	char buff[50];
	sprintf_s(buff, "ISTIRAP Client[%d]", GetCurrentProcessId());

	memcpy((LPVOID)KO_MULTI_CAP, buff, 50);

	return 1;
}

void PatchMulti()
{
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)MultiPatch, NULL, NULL, NULL);
}

HRESULT WINAPI HookCreateDevice()
{
	IDirect3D9 * device = Direct3DCreate9(D3D_SDK_VERSION);
	if (!device)
		return D3DERR_INVALIDCALL;

	IDirect3D9_vtable = (DWORD*)*(DWORD*)device;
	device->Release();
	DWORD protectFlag;
	if (VirtualProtect(&IDirect3D9_vtable[CREATEDEVICE_VTI], sizeof(DWORD), PAGE_READWRITE, &protectFlag))
	{
		*(DWORD*)&D3DCreateDevice_orig = IDirect3D9_vtable[CREATEDEVICE_VTI];
		*(DWORD*)&IDirect3D9_vtable[CREATEDEVICE_VTI] = (DWORD)D3DCreateDevice_hook;
		if (!VirtualProtect(&IDirect3D9_vtable[CREATEDEVICE_VTI], sizeof(DWORD), protectFlag, &protectFlag))
		{
			return D3DERR_INVALIDCALL;
		}
	}
	else
		return D3DERR_INVALIDCALL;

	return D3D_OK;
}

HRESULT WINAPI D3DCreateDevice_hook(IDirect3D9* Direct3D_Object, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow,DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice9** ppReturnedDeviceInterface)
{
	if (!itsMe) 
	{
		if (Engine->m_SettingsMgr == NULL) 
			Engine->m_SettingsMgr = new CSettingsManager();

		pPresentationParameters->Windowed = Engine->m_SettingsMgr->m_iRealFullScreen == 1 ? false : true;
		pPresentationParameters->PresentationInterval = Engine->m_SettingsMgr->m_iVsync == 1 ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
		if (Engine->m_SettingsMgr->m_iVsync == 1)
			pPresentationParameters->FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	}

	return D3DCreateDevice_orig(Direct3D_Object, Adapter, DeviceType, hFocusWindow, BehaviorFlags | D3DCREATE_MULTITHREADED, pPresentationParameters, ppReturnedDeviceInterface);
}
