// Win32 uyumluluk katmanı — fonksiyon gövdeleri. Platform (SDL) katmanı KoWin32GetHooks()
// üzerinden gerçek pencere/imleç davranışını sağlar; kanca yoksa güvenli varsayılanlar döner.
#include <windows.h>

#include <cstdio>
#include <ctime>
#include <filesystem>
#include <unistd.h>

#if defined(__ANDROID__)
#include <android/log.h>
#endif

namespace
{
HWND g_mainWindow = nullptr;
}

KoWin32Hooks& KoWin32GetHooks()
{
	static KoWin32Hooks hooks;
	return hooks;
}

HWND KoWin32MainWindow() { return g_mainWindow; }
void KoWin32SetMainWindow(HWND h) { g_mainWindow = h; }

HWND GetActiveWindow() { return g_mainWindow; }
HWND GetForegroundWindow() { return g_mainWindow; }
HWND GetFocus() { return g_mainWindow; }
HWND SetFocus(HWND) { return g_mainWindow; }
HWND SetActiveWindow(HWND) { return g_mainWindow; }
BOOL ShowWindow(HWND, int) { return TRUE; }
BOOL SetWindowText(HWND, LPCSTR) { return TRUE; }
HCURSOR g_cursor = nullptr;
HCURSOR SetCursor(HCURSOR c) { HCURSOR old = g_cursor; g_cursor = c; return old; }
HCURSOR GetCursor() { return g_cursor; }

BOOL EnumDisplaySettings(LPCSTR, DWORD, DEVMODE* dm)
{
	int w = 1024, h = 768;
	if (KoWin32GetHooks().getClientSize)
		KoWin32GetHooks().getClientSize(&w, &h);
	*dm = {};
	dm->dmSize = sizeof(DEVMODE);
	dm->dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
	dm->dmBitsPerPel = 32;
	dm->dmPelsWidth = (DWORD) w;
	dm->dmPelsHeight = (DWORD) h;
	dm->dmDisplayFrequency = 60;
	return TRUE;
}

LONG ChangeDisplaySettings(DEVMODE*, DWORD) { return DISP_CHANGE_SUCCESSFUL; }

HINSTANCE ShellExecute(HWND, LPCSTR, LPCSTR file, LPCSTR, LPCSTR, int)
{
	if (KoWin32GetHooks().openUrl && file)
		KoWin32GetHooks().openUrl(file);
	else
		std::fprintf(stderr, "[ShellExecute] %s\n", file ? file : "");
	return (HINSTANCE) (ULONG_PTR) 33;
}
HCURSOR LoadCursor(HINSTANCE, LPCSTR id) { return (HCURSOR) id; }
HICON LoadIcon(HINSTANCE, LPCSTR id) { return (HICON) id; }
LRESULT SendMessage(HWND, UINT, WPARAM, LPARAM) { return 0; }
BOOL PostMessage(HWND, UINT, WPARAM, LPARAM) { return TRUE; }

BOOL GetClientRect(HWND, RECT* rc)
{
	int w = 1024, h = 768;
	if (KoWin32GetHooks().getClientSize)
		KoWin32GetHooks().getClientSize(&w, &h);
	SetRect(rc, 0, 0, w, h);
	return TRUE;
}

BOOL GetWindowRect(HWND hwnd, RECT* rc) { return GetClientRect(hwnd, rc); }
BOOL ClientToScreen(HWND, POINT*) { return TRUE; }
BOOL ScreenToClient(HWND, POINT*) { return TRUE; }

BOOL GetCursorPos(POINT* pt)
{
	int x = 0, y = 0;
	if (KoWin32GetHooks().getCursorPos)
		KoWin32GetHooks().getCursorPos(&x, &y);
	pt->x = x;
	pt->y = y;
	return TRUE;
}

BOOL SetCursorPos(int x, int y)
{
	if (KoWin32GetHooks().setCursorPos)
		KoWin32GetHooks().setCursorPos(x, y);
	return TRUE;
}

int ShowCursor(BOOL show)
{
	if (KoWin32GetHooks().showCursor)
		return KoWin32GetHooks().showCursor(show);
	return show ? 0 : -1;
}

void PostQuitMessage(int code)
{
	if (KoWin32GetHooks().postQuit)
		KoWin32GetHooks().postQuit(code);
}

int MessageBox(HWND, LPCSTR text, LPCSTR caption, UINT type)
{
	if (KoWin32GetHooks().messageBox)
		return KoWin32GetHooks().messageBox(text, caption, type);
	std::fprintf(stderr, "[MessageBox] %s: %s\n", caption ? caption : "", text ? text : "");
	return IDOK;
}

int MessageBoxA(HWND h, LPCSTR text, LPCSTR caption, UINT type) { return MessageBox(h, text, caption, type); }

int MessageBoxW(HWND h, LPCWSTR text, LPCWSTR caption, UINT type)
{
	char t[1024] = "", c[256] = "";
	if (text) std::snprintf(t, sizeof(t), "%ls", text);
	if (caption) std::snprintf(c, sizeof(c), "%ls", caption);
	return MessageBox(h, t, c, type);
}

BOOL MoveWindow(HWND, int, int, int, int, BOOL) { return TRUE; }

SHORT GetAsyncKeyState(int vk)
{
	if (KoWin32GetHooks().isKeyDown && KoWin32GetHooks().isKeyDown(vk))
		return (SHORT) 0x8000;
	return 0;
}

SHORT GetKeyState(int vk) { return GetAsyncKeyState(vk); }

DWORD GetCurrentDirectory(DWORD n, LPSTR buf)
{
	if (!getcwd(buf, n))
		return 0;
	return (DWORD) std::strlen(buf);
}

BOOL SetCurrentDirectory(LPCSTR path) { return chdir(path) == 0; }

DWORD GetModuleFileName(HMODULE, LPSTR buf, DWORD n)
{
	std::error_code ec;
	auto p = std::filesystem::read_symlink("/proc/self/exe", ec);
	std::string s = ec ? std::string("./app") : p.string();
	std::snprintf(buf, n, "%s", s.c_str());
	return (DWORD) std::strlen(buf);
}

void _splitpath(const char* path, char* drive, char* dir, char* fname, char* ext)
{
	std::filesystem::path p(path ? path : "");
	if (drive) drive[0] = 0;
	if (dir)
	{
		std::string d = p.parent_path().string();
		if (!d.empty() && d.back() != '/') d += '/';
		std::strcpy(dir, d.c_str());
	}
	if (fname) std::strcpy(fname, p.stem().string().c_str());
	if (ext) std::strcpy(ext, p.extension().string().c_str());
}

void GetLocalTime(SYSTEMTIME* st)
{
	using namespace std::chrono;
	auto now = system_clock::now();
	std::time_t t = system_clock::to_time_t(now);
	std::tm tm {};
	localtime_r(&t, &tm);
	st->wYear = (WORD) (tm.tm_year + 1900); st->wMonth = (WORD) (tm.tm_mon + 1); st->wDayOfWeek = (WORD) tm.tm_wday;
	st->wDay = (WORD) tm.tm_mday; st->wHour = (WORD) tm.tm_hour; st->wMinute = (WORD) tm.tm_min; st->wSecond = (WORD) tm.tm_sec;
	st->wMilliseconds = (WORD) (duration_cast<milliseconds>(now.time_since_epoch()).count() % 1000);
}

void GetSystemTime(SYSTEMTIME* st)
{
	using namespace std::chrono;
	auto now = system_clock::now();
	std::time_t t = system_clock::to_time_t(now);
	std::tm tm {};
	gmtime_r(&t, &tm);
	st->wYear = (WORD) (tm.tm_year + 1900); st->wMonth = (WORD) (tm.tm_mon + 1); st->wDayOfWeek = (WORD) tm.tm_wday;
	st->wDay = (WORD) tm.tm_mday; st->wHour = (WORD) tm.tm_hour; st->wMinute = (WORD) tm.tm_min; st->wSecond = (WORD) tm.tm_sec;
	st->wMilliseconds = (WORD) (duration_cast<milliseconds>(now.time_since_epoch()).count() % 1000);
}

UINT GetTempFileName(LPCSTR path, LPCSTR prefix, UINT unique, LPSTR out)
{
	static unsigned counter = 0;
	unsigned id = unique ? unique : (unsigned) (getpid() * 1000 + (++counter));
	std::snprintf(out, MAX_PATH, "%s/%s%04X.tmp", (path && *path) ? path : "/tmp", prefix ? prefix : "tmp", id & 0xFFFF);
	if (!unique)
	{
		FILE* f = std::fopen(out, "wb");
		if (f) std::fclose(f);
	}
	return id;
}

DWORD GetTempPath(DWORD n, LPSTR buf)
{
	const char* t = std::getenv("TMPDIR");
	std::snprintf(buf, n, "%s/", t ? t : "/tmp");
	return (DWORD) std::strlen(buf);
}

void OutputDebugString(LPCSTR s)
{
#if defined(__ANDROID__)
	__android_log_print(ANDROID_LOG_DEBUG, "KnightOnline", "%s", s ? s : "");
#else
	std::fputs(s ? s : "", stderr);
#endif
}
