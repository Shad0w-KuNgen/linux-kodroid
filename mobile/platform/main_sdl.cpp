// main_sdl.cpp — Knight Online istemcisinin SDL2 + OpenGL ES 3 uygulama kabuğu.
//
// WarFareMain.cpp'nin (Win32 pencere + mesaj döngüsü) karşılığı. Linux masaüstünde ve
// Android'de (SDL'in Android projesi SDL_main'i çağırır) aynı kod çalışır.
#include "StdAfx.h"
#include "GameProcedure.h"
#include "GameProcMain.h"
#include "GameProcLogIn.h"
#include "GameEng.h"
#include "APISocket.h"
#include "UIManager.h"
#include "UIMessageBoxManager.h"
#include "UIMessageBox.h"

#include <N3Base/N3Base.h>
#include <N3Base/N3UIBase.h>
#include <N3Base/N3UIEdit.h>
#include <N3Base/LogWriter.h>
#include <shared/Ini.h>

#include <d3d9.h>
#include <winsock2.h>

#include <SDL.h>

#include "KoPlatformInput.h"

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>

namespace
{
SDL_Window* g_window = nullptr;
SDL_GLContext g_glContext = nullptr;
bool g_quit = false;

void LoadOptions(const std::string& iniPath)
{
	CIni ini(iniPath);
	auto& o          = CN3Base::s_Options;
	o.iTexLOD_Chr     = std::clamp(ini.GetInt("Texture", "LOD_Chr", 0), 0, 1);
	o.iTexLOD_Shape   = std::clamp(ini.GetInt("Texture", "LOD_Shape", 0), 0, 1);
	o.iTexLOD_Terrain = std::clamp(ini.GetInt("Texture", "LOD_Terrain", 0), 0, 1);
	o.iUseShadow      = ini.GetInt("Shadow", "Use", 1);
	o.iViewWidth      = ini.GetInt("ViewPort", "Width", 1024);
	o.iViewHeight     = ini.GetInt("ViewPort", "Height", 768);
	o.iViewColorDepth = 32;
	o.iViewDist       = std::clamp(ini.GetInt("ViewPort", "Distance", 512), 256, 512);
	o.iEffectSndDist  = std::clamp(ini.GetInt("Sound", "Distance", 48), 20, 48);
	o.bSndBgmEnable   = ini.GetBool("Sound", "Bgm", true);
	o.bSndEffectEnable = ini.GetBool("Sound", "Effect", true);
	o.bSndEnable      = o.bSndBgmEnable || o.bSndEffectEnable;
	o.bWindowCursor   = ini.GetBool("Cursor", "WindowCursor", true);
	o.bWindowMode     = ini.GetBool("Screen", "WindowMode", true);
	o.bVSyncEnabled   = ini.GetBool("Screen", "VSyncEnabled", true);
}

std::string ResolveClientDir(int argc, char** argv)
{
	if (const char* env = std::getenv("KO_CLIENT_DIR"))
		return env;
	for (int i = 1; i + 1 < argc; ++i)
		if (std::string(argv[i]) == "--client-dir")
			return argv[i + 1];
#if defined(__ANDROID__)
	if (const char* ext = SDL_AndroidGetExternalStoragePath())
		return ext;
#endif
	char cwd[_MAX_PATH] = "";
	GetCurrentDirectory(_MAX_PATH, cwd);
	return cwd;
}

// ---- Kancalar ---------------------------------------------------------------
void HookPresent(void*)
{
	SDL_GL_SwapWindow(g_window);
}
void HookDrawableSize(void*, int* w, int* h)
{
	SDL_GL_GetDrawableSize(g_window, w, h);
}
void HookClientSize(int* w, int* h)
{
	SDL_GL_GetDrawableSize(g_window, w, h);
}
void HookGetCursorPos(int* x, int* y)
{
	*x = KoInput().mouseX;
	*y = KoInput().mouseY;
}
void HookSetCursorPos(int x, int y)
{
	KoInput().mouseX = x;
	KoInput().mouseY = y;
	SDL_WarpMouseInWindow(g_window, x, y);
}
int HookShowCursor(BOOL show)
{
	return SDL_ShowCursor(show ? SDL_ENABLE : SDL_DISABLE);
}
void HookPostQuit(int)
{
	g_quit = true;
}
int HookMessageBox(const char* text, const char* caption, UINT type)
{
	CLogWriter::Write("[MessageBox] {}: {}", caption ? caption : "", text ? text : "");
	if (g_window)
	{
		Uint32 flags = (type & MB_ICONERROR) ? SDL_MESSAGEBOX_ERROR : (type & MB_ICONWARNING) ? SDL_MESSAGEBOX_WARNING : SDL_MESSAGEBOX_INFORMATION;
		SDL_ShowSimpleMessageBox(flags, caption ? caption : "Knight OnLine", text ? text : "", g_window);
	}
	return IDOK;
}
void HookOpenUrl(const char* url)
{
	if (url && (std::strncmp(url, "http", 4) == 0))
		SDL_OpenURL(url);
	else
		CLogWriter::Write("[ShellExecute] desteklenmiyor: {}", url ? url : "");
}

void OnSocketEvent(SOCKET, HWND, unsigned, long event)
{
	switch (event)
	{
		case FD_READ:
			if (CGameProcedure::s_pSocket)
				CGameProcedure::s_pSocket->Receive();
			break;
		case FD_CLOSE:
			CGameProcedure::ReportServerConnectionClosed(true);
			break;
		default:
			break;
	}
}

void OnMouseWheel(short delta)
{
	CN3UIBase* pUI = nullptr;
	if (CGameProcedure::s_pMsgBoxMgr != nullptr)
		pUI = CGameProcedure::s_pMsgBoxMgr->GetFocusMsgBox();
	if (pUI != nullptr && pUI->IsVisible() && pUI->OnMouseWheelEvent(delta))
		return;
	if (CGameProcedure::s_pUIMgr != nullptr)
		pUI = CGameProcedure::s_pUIMgr->GetFocusedUI();
	if (pUI != nullptr && pUI->IsVisible() && pUI->OnMouseWheelEvent(delta))
		return;
	if (CGameProcedure::s_pProcActive == CGameProcedure::s_pProcMain && CGameProcedure::s_pEng)
		CGameProcedure::s_pEng->CameraZoom(delta * 0.05f);
}

void OnCloseRequest()
{
	if (CGameProcedure::s_pProcActive != nullptr && CGameProcedure::s_pProcActive == CGameProcedure::s_pProcMain)
	{
		CGameProcedure::s_pProcMain->RequestExit(); // oyun içi: güvenli çıkış sayacı
		return;
	}
	if (CGameProcedure::s_pSocket) CGameProcedure::s_pSocket->Disconnect();
	if (CGameProcedure::s_pSocketSub) CGameProcedure::s_pSocketSub->Disconnect();
	g_quit = true;
}

void OnFocusChanged(bool focused)
{
	KoInput().windowFocused            = focused;
	CGameProcedure::s_bIsWindowInFocus = focused;
	if (focused)
	{
		CN3UIEdit* pEdit = CN3UIBase::GetFocusedEdit();
		if (pEdit != nullptr)
		{
			pEdit->KillFocus();
			pEdit->SetFocus();
		}
	}
}

void HandleTextInputKey(const SDL_KeyboardEvent& key)
{
	if (!CN3UIEdit::WantsTextInput())
		return;
	switch (key.keysym.scancode)
	{
		case SDL_SCANCODE_BACKSPACE: CN3UIEdit::InputKey(CN3UIEdit::EDITKEY_BACKSPACE); break;
		case SDL_SCANCODE_DELETE: CN3UIEdit::InputKey(CN3UIEdit::EDITKEY_DELETE); break;
		case SDL_SCANCODE_LEFT: CN3UIEdit::InputKey(CN3UIEdit::EDITKEY_LEFT); break;
		case SDL_SCANCODE_RIGHT: CN3UIEdit::InputKey(CN3UIEdit::EDITKEY_RIGHT); break;
		case SDL_SCANCODE_HOME: CN3UIEdit::InputKey(CN3UIEdit::EDITKEY_HOME); break;
		case SDL_SCANCODE_END: CN3UIEdit::InputKey(CN3UIEdit::EDITKEY_END); break;
		case SDL_SCANCODE_RETURN:
		case SDL_SCANCODE_KP_ENTER: CN3UIEdit::InputKey(CN3UIEdit::EDITKEY_RETURN); break;
		default: break;
	}
}

void PumpEvents()
{
	KoInputState& in = KoInput();
	int dw = 1, dh = 1;
	SDL_GL_GetDrawableSize(g_window, &dw, &dh);
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
			case SDL_QUIT: OnCloseRequest(); break;
			case SDL_APP_TERMINATING: g_quit = true; break;
			case SDL_WINDOWEVENT:
				if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) OnFocusChanged(true);
				else if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST) OnFocusChanged(false);
				break;
			case SDL_MOUSEMOTION:
				if (e.motion.which != SDL_TOUCH_MOUSEID)
				{
					in.mouseX = e.motion.x;
					in.mouseY = e.motion.y;
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				if (e.button.which != SDL_TOUCH_MOUSEID)
				{
					bool down = e.type == SDL_MOUSEBUTTONDOWN;
					in.mouseX = e.button.x;
					in.mouseY = e.button.y;
					if (e.button.button == SDL_BUTTON_LEFT) in.lbDown = down;
					else if (e.button.button == SDL_BUTTON_MIDDLE) in.mbDown = down;
					else if (e.button.button == SDL_BUTTON_RIGHT) in.rbDown = down;
				}
				break;
			case SDL_MOUSEWHEEL:
				if (e.wheel.y != 0)
					OnMouseWheel((short) (e.wheel.y * WHEEL_DELTA));
				break;
			// Dokunmatik: ilk parmak sol tık, ikinci parmak sağ tık (ilk sürüm)
			case SDL_FINGERDOWN:
			case SDL_FINGERMOTION:
			case SDL_FINGERUP:
			{
				int fingers = SDL_GetNumTouchFingers(e.tfinger.touchId);
				int x = (int) (e.tfinger.x * dw), y = (int) (e.tfinger.y * dh);
				if (e.type == SDL_FINGERDOWN)
				{
					if (fingers <= 1)
					{
						in.mouseX = x; in.mouseY = y; in.lbDown = true;
					}
					else
					{
						in.rbDown = true;
					}
				}
				else if (e.type == SDL_FINGERMOTION)
				{
					if (fingers <= 1 || e.tfinger.fingerId == 0)
					{
						in.mouseX = x; in.mouseY = y;
					}
				}
				else
				{
					if (fingers <= 1) { in.lbDown = false; in.rbDown = false; }
					else in.rbDown = false;
				}
				break;
			}
			case SDL_TEXTINPUT:
				CN3UIEdit::InputText(e.text.text);
				break;
			case SDL_KEYDOWN:
				HandleTextInputKey(e.key);
				break;
			default:
				break;
		}
	}

	// Sanal klavye / metin girişi yönetimi
	bool wants = CN3UIEdit::WantsTextInput();
	if (wants && !SDL_IsTextInputActive())
		SDL_StartTextInput();
	else if (!wants && SDL_IsTextInputActive())
		SDL_StopTextInput();
}
} // namespace

int main(int argc, char** argv)
{
	std::string clientDir = ResolveClientDir(argc, argv);
	if (!clientDir.empty() && clientDir.back() != '/' && clientDir.back() != '\\')
		clientDir += '/';
	CN3Base::PathSet(clientDir);
	KoRegistrySetDirectory(clientDir.c_str());
	SetCurrentDirectory(clientDir.c_str());
	LoadOptions(clientDir + "Option.ini");

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
	{
		std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		return -1;
	}
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;
#if defined(__ANDROID__) || defined(__IPHONEOS__)
	flags |= SDL_WINDOW_FULLSCREEN;
#else
	if (!CN3Base::s_Options.bWindowMode)
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
#endif
	g_window = SDL_CreateWindow("Knight OnLine Client", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		CN3Base::s_Options.iViewWidth, CN3Base::s_Options.iViewHeight, flags);
	if (!g_window)
	{
		std::fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
		return -1;
	}
	g_glContext = SDL_GL_CreateContext(g_window);
	if (!g_glContext)
	{
		std::fprintf(stderr, "SDL_GL_CreateContext: %s\n", SDL_GetError());
		return -1;
	}
	SDL_GL_SetSwapInterval(CN3Base::s_Options.bVSyncEnabled ? 1 : 0);

	// Gerçek çizim alanı (mobilde tam ekran) oyun seçeneklerine yazılır
	int dw = 0, dh = 0;
	SDL_GL_GetDrawableSize(g_window, &dw, &dh);
	CN3Base::s_Options.iViewWidth  = dw;
	CN3Base::s_Options.iViewHeight = dh;

	d3d9gles::PlatformHooks gl {};
	gl.present         = HookPresent;
	gl.getDrawableSize = HookDrawableSize;
	d3d9gles::SetPlatformHooks(gl);

	KoWin32Hooks& w32 = KoWin32GetHooks();
	w32.getClientSize = HookClientSize;
	w32.getCursorPos  = HookGetCursorPos;
	w32.setCursorPos  = HookSetCursorPos;
	w32.showCursor    = HookShowCursor;
	w32.postQuit      = HookPostQuit;
	w32.messageBox    = HookMessageBox;
	w32.isKeyDown     = KoInputIsVkDown;
	w32.openUrl       = HookOpenUrl;

	HWND hWndMain = (HWND) g_window;
	KoWin32SetMainWindow(hWndMain);

	srand((uint32_t) time(nullptr));

	CGameProcedure::s_bWindowed = true;
	CGameProcedure::StaticMemberInit(nullptr, hWndMain);
	CGameProcedure::ProcActiveSet((CGameProcedure*) CGameProcedure::s_pProcLogIn);

	while (!g_quit)
	{
		PumpEvents();
		KoWinsockPoll(OnSocketEvent);
		CGameProcedure::TickActive();
		CGameProcedure::RenderActive();
	}

	CGameProcedure::StaticMemberRelease();
	SDL_GL_DeleteContext(g_glContext);
	SDL_DestroyWindow(g_window);
	SDL_Quit();
	return 0;
}
