// windows.h — Win32 uyumluluk katmanı (Knight Online mobil portu).
//
// OpenKO istemcisi (N3Base + WarFare) Win32 tiplerini ve bir avuç Win32 fonksiyonunu
// kullanıyor. Bu başlık, Windows dışı platformlarda (Android / Linux / iOS) o tipleri ve
// fonksiyonları sağlar. Gerçek Windows'ta sistemin kendi <windows.h>'si kullanılır.
#ifndef KO_MOBILE_COMPAT_WINDOWS_H
#define KO_MOBILE_COMPAT_WINDOWS_H

#if defined(_WIN32)
#include_next <windows.h>
#else

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cerrno>
#include <sys/stat.h>
#include <cmath>
#include <string>
#include <chrono>
#include <thread>
#include <algorithm>

// ---------------------------------------------------------------------------
// Çağrı kuralları / nitelikler
// ---------------------------------------------------------------------------
#define WINAPI
#define APIENTRY
#define CALLBACK
#define PASCAL
#define __stdcall
#define __cdecl
#define _In_
#define _In_opt_
#define _Out_
#define _Out_opt_
#define _Inout_
#define __declspec(x)

// ---------------------------------------------------------------------------
// Temel tipler
// ---------------------------------------------------------------------------
typedef int           BOOL;
typedef unsigned char BYTE;
typedef unsigned char UCHAR;
typedef char          CHAR;
typedef unsigned short WORD;
typedef unsigned short USHORT;
typedef short         SHORT;
typedef uint32_t      DWORD;
typedef uint32_t      ULONG;
typedef int32_t       LONG;
typedef int           INT;
typedef unsigned int  UINT;
typedef float         FLOAT;
typedef double        DOUBLE;
typedef int64_t       LONGLONG;
typedef uint64_t      ULONGLONG;
typedef int64_t       INT64;
typedef uint64_t      UINT64;
typedef intptr_t      INT_PTR;
typedef uintptr_t     UINT_PTR;
typedef intptr_t      LONG_PTR;
typedef uintptr_t     ULONG_PTR;
typedef uintptr_t     DWORD_PTR;
typedef size_t        SIZE_T;
typedef void          VOID;
typedef void*         PVOID;
typedef void*         LPVOID;
typedef const void*   LPCVOID;
typedef char*         LPSTR;
typedef const char*   LPCSTR;
typedef char*         LPTSTR;
typedef const char*   LPCTSTR;
typedef wchar_t       WCHAR;
typedef wchar_t*      LPWSTR;
typedef const wchar_t* LPCWSTR;
typedef char          TCHAR;
typedef BYTE*         LPBYTE;
typedef DWORD*        LPDWORD;
typedef LONG*         LPLONG;
typedef BOOL*         LPBOOL;
typedef WORD*         LPWORD;
typedef INT*          LPINT;
typedef UINT*         LPUINT;
typedef FLOAT*        LPFLOAT;
typedef LONG          HRESULT;
typedef UINT_PTR      WPARAM;
typedef LONG_PTR      LPARAM;
typedef LONG_PTR      LRESULT;
typedef DWORD         COLORREF;
typedef DWORD*        LPCOLORREF;
typedef void*         HANDLE;
typedef HANDLE        HWND;
typedef HANDLE        HINSTANCE;
typedef HANDLE        HMODULE;
typedef HANDLE        HDC;
typedef HANDLE        HFONT;
typedef HANDLE        HBITMAP;
typedef HANDLE        HBRUSH;
typedef HANDLE        HICON;
typedef HANDLE        HCURSOR;
typedef HANDLE        HMENU;
typedef HANDLE        HGDIOBJ;
typedef HANDLE        HACCEL;
typedef HANDLE        HKL;
typedef HANDLE        HIMC;
typedef HANDLE        HGLOBAL;
typedef HANDLE        HLOCAL;
typedef HANDLE        HRGN;
typedef HANDLE        HPEN;
typedef HANDLE        HKEY;
typedef HANDLE        HPALETTE;
typedef unsigned char byte; // rpcndr.h
typedef LRESULT (*WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef LRESULT (*FARPROC)();

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

#define MAX_PATH   260
#define _MAX_PATH  260
#define _MAX_DRIVE 3
#define _MAX_DIR   256
#define _MAX_FNAME 256
#define _MAX_EXT   256

// HRESULT yardımcıları
#define S_OK                 ((HRESULT) 0L)
#define S_FALSE              ((HRESULT) 1L)
#define E_FAIL               ((HRESULT) 0x80004005L)
#define E_OUTOFMEMORY        ((HRESULT) 0x8007000EL)
#define E_INVALIDARG         ((HRESULT) 0x80070057L)
#define E_NOTIMPL            ((HRESULT) 0x80004001L)
#define E_POINTER            ((HRESULT) 0x80004003L)
#define SUCCEEDED(hr)        (((HRESULT) (hr)) >= 0)
#define FAILED(hr)           (((HRESULT) (hr)) < 0)

#define MAKEWORD(a, b)       ((WORD) (((BYTE) ((DWORD_PTR) (a) & 0xff)) | ((WORD) ((BYTE) ((DWORD_PTR) (b) & 0xff))) << 8))
#define MAKELONG(a, b)       ((LONG) (((WORD) ((DWORD_PTR) (a) & 0xffff)) | ((DWORD) ((WORD) ((DWORD_PTR) (b) & 0xffff))) << 16))
#define LOWORD(l)            ((WORD) ((DWORD_PTR) (l) & 0xffff))
#define HIWORD(l)            ((WORD) ((DWORD_PTR) (l) >> 16))
#define LOBYTE(w)            ((BYTE) ((DWORD_PTR) (w) & 0xff))
#define HIBYTE(w)            ((BYTE) ((DWORD_PTR) (w) >> 8))

#define MAKEINTRESOURCE(i)   ((LPSTR) ((ULONG_PTR) ((WORD) (i))))
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
	((DWORD) (BYTE) (ch0) | ((DWORD) (BYTE) (ch1) << 8) | ((DWORD) (BYTE) (ch2) << 16) | ((DWORD) (BYTE) (ch3) << 24))

// ---------------------------------------------------------------------------
// Yapılar
// ---------------------------------------------------------------------------
typedef struct tagRECT
{
	LONG left;
	LONG top;
	LONG right;
	LONG bottom;
} RECT, *PRECT, *LPRECT;
typedef const RECT* LPCRECT;

typedef struct tagPOINT
{
	LONG x;
	LONG y;
} POINT, *PPOINT, *LPPOINT;

typedef struct tagSIZE
{
	LONG cx;
	LONG cy;
} SIZE, *PSIZE, *LPSIZE;

typedef union _LARGE_INTEGER
{
	struct
	{
		DWORD LowPart;
		LONG HighPart;
	} u;
	LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef struct _GUID
{
	uint32_t Data1;
	uint16_t Data2;
	uint16_t Data3;
	uint8_t Data4[8];
} GUID;
typedef const GUID& REFIID;
typedef GUID IID;

typedef struct tagMSG
{
	HWND hwnd;
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
	DWORD time;
	POINT pt;
} MSG, *LPMSG;

// Bitmap dosya yapıları (BitMapFile.cpp bunları diskten okuyor, paketleme önemli)
#pragma pack(push, 2)
typedef struct tagBITMAPFILEHEADER
{
	WORD bfType;
	DWORD bfSize;
	WORD bfReserved1;
	WORD bfReserved2;
	DWORD bfOffBits;
} BITMAPFILEHEADER, *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;
#pragma pack(pop)

typedef struct tagBITMAPINFOHEADER
{
	DWORD biSize;
	LONG biWidth;
	LONG biHeight;
	WORD biPlanes;
	WORD biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	LONG biXPelsPerMeter;
	LONG biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagBITMAPCOREHEADER
{
	DWORD bcSize;
	WORD bcWidth;
	WORD bcHeight;
	WORD bcPlanes;
	WORD bcBitCount;
} BITMAPCOREHEADER, *LPBITMAPCOREHEADER, *PBITMAPCOREHEADER;

typedef struct tagRGBQUAD
{
	BYTE rgbBlue;
	BYTE rgbGreen;
	BYTE rgbRed;
	BYTE rgbReserved;
} RGBQUAD, *LPRGBQUAD;
typedef struct tagRGBTRIPLE { BYTE rgbtBlue, rgbtGreen, rgbtRed; } RGBTRIPLE;

typedef struct tagBITMAPINFO
{
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD bmiColors[1];
} BITMAPINFO, *LPBITMAPINFO, *PBITMAPINFO;

#define BI_RGB 0L

// ---------------------------------------------------------------------------
// Renk makroları
// ---------------------------------------------------------------------------
#define RGB(r, g, b)  ((COLORREF) (((BYTE) (r) | ((WORD) ((BYTE) (g)) << 8)) | (((DWORD) (BYTE) (b)) << 16)))
#define GetRValue(rgb) (LOBYTE(rgb))
#define GetGValue(rgb) (LOBYTE(((WORD) (rgb)) >> 8))
#define GetBValue(rgb) (LOBYTE((rgb) >> 16))

// ---------------------------------------------------------------------------
// Sanal tuş kodları (WarFare'in kullandığı alt küme)
// ---------------------------------------------------------------------------
#define VK_LBUTTON  0x01
#define VK_RBUTTON  0x02
#define VK_MBUTTON  0x04
#define VK_BACK     0x08
#define VK_TAB      0x09
#define VK_RETURN   0x0D
#define VK_SHIFT    0x10
#define VK_CONTROL  0x11
#define VK_MENU     0x12
#define VK_PAUSE    0x13
#define VK_CAPITAL  0x14
#define VK_ESCAPE   0x1B
#define VK_SPACE    0x20
#define VK_PRIOR    0x21
#define VK_NEXT     0x22
#define VK_END      0x23
#define VK_HOME     0x24
#define VK_LEFT     0x25
#define VK_UP       0x26
#define VK_RIGHT    0x27
#define VK_DOWN     0x28
#define VK_SNAPSHOT 0x2C
#define VK_INSERT   0x2D
#define VK_DELETE   0x2E
#define VK_F1       0x70
#define VK_F2       0x71
#define VK_F3       0x72
#define VK_F4       0x73
#define VK_F5       0x74
#define VK_F6       0x75
#define VK_F7       0x76
#define VK_F8       0x77
#define VK_F9       0x78
#define VK_F10      0x79
#define VK_F11      0x7A
#define VK_F12      0x7B
#define VK_LSHIFT   0xA0
#define VK_RSHIFT   0xA1
#define VK_LCONTROL 0xA2
#define VK_RCONTROL 0xA3
#define VK_LMENU    0xA4
#define VK_RMENU    0xA5

// Mesaj kutusu bayrakları / dönüşleri
#define MB_OK          0x0000
#define MB_OKCANCEL    0x0001
#define MB_YESNO       0x0004
#define MB_ICONERROR   0x0010
#define MB_ICONWARNING 0x0030
#define IDOK     1
#define IDCANCEL 2
#define IDYES    6
#define IDNO     7

// Pencere mesajları (WarFare'in bakacağı alt küme; platform katmanı üretir)
#define WM_NULL     0x0000
#define WM_CREATE   0x0001
#define WM_DESTROY  0x0002
#define WM_SIZE     0x0005
#define WM_ACTIVATE 0x0006
#define WM_SETFOCUS 0x0007
#define WM_KILLFOCUS 0x0008
#define WM_CLOSE    0x0010
#define WM_QUIT     0x0012
#define WM_KEYDOWN  0x0100
#define WM_KEYUP    0x0101
#define WM_CHAR     0x0102
#define WM_SYSKEYDOWN 0x0104
#define WM_COMMAND  0x0111
#define WM_MOUSEMOVE 0x0200
#define WM_LBUTTONDOWN 0x0201
#define WM_LBUTTONUP 0x0202
#define WM_RBUTTONDOWN 0x0204
#define WM_RBUTTONUP 0x0205
#define WM_MOUSEWHEEL 0x020A
#define WM_USER     0x0400
#define WA_INACTIVE 0
#define WA_ACTIVE 1
#define WA_CLICKACTIVE 2
#define WHEEL_DELTA 120
#define GET_WHEEL_DELTA_WPARAM(wParam) ((short) HIWORD(wParam))

// ---------------------------------------------------------------------------
// Platform kancaları: pencere/imleç/zaman gibi işler SDL tarafından sağlanır.
// mobile/platform/ bunları doldurur; test ortamında varsayılanlar yeter.
// ---------------------------------------------------------------------------
struct KoWin32Hooks
{
	void (*getClientSize)(int* w, int* h)        = nullptr;
	void (*getCursorPos)(int* x, int* y)         = nullptr;
	void (*setCursorPos)(int x, int y)           = nullptr;
	int (*showCursor)(BOOL show)                 = nullptr;
	void (*postQuit)(int code)                   = nullptr;
	int (*messageBox)(const char* text, const char* caption, UINT type) = nullptr;
	int (*isKeyDown)(int vk)                     = nullptr;
	void (*openUrl)(const char* url)             = nullptr;
};
KoWin32Hooks& KoWin32GetHooks();

// ---------------------------------------------------------------------------
// Fonksiyonlar (inline / küçük)
// ---------------------------------------------------------------------------
inline void ZeroMemory(void* p, size_t n) { std::memset(p, 0, n); }
inline void CopyMemory(void* d, const void* s, size_t n) { std::memcpy(d, s, n); }
inline void FillMemory(void* p, size_t n, BYTE v) { std::memset(p, v, n); }
inline void MoveMemory(void* d, const void* s, size_t n) { std::memmove(d, s, n); }

inline int lstrlen(const char* s) { return s ? (int) std::strlen(s) : 0; }
inline int lstrlenA(const char* s) { return lstrlen(s); }
inline char* lstrcpy(char* d, const char* s) { return std::strcpy(d, s); }
inline char* lstrcpyA(char* d, const char* s) { return std::strcpy(d, s); }
inline char* lstrcpyn(char* d, const char* s, int n)
{
	if (n <= 0) return d;
	std::strncpy(d, s, (size_t) n - 1);
	d[n - 1] = 0;
	return d;
}
inline char* lstrcat(char* d, const char* s) { return std::strcat(d, s); }
inline int lstrcmp(const char* a, const char* b) { return std::strcmp(a, b); }
inline int lstrcmpi(const char* a, const char* b) { return strcasecmp(a, b); }
inline int lstrcmpiA(const char* a, const char* b) { return strcasecmp(a, b); }
#ifndef _stricmp
#define _stricmp strcasecmp
#endif
#ifndef _strnicmp
#define _strnicmp strncasecmp
#endif
#ifndef stricmp
#define stricmp strcasecmp
#endif
#ifndef strnicmp
#define strnicmp strncasecmp
#endif
#define wsprintf sprintf
#define _snprintf snprintf
#define _itoa(v, s, r) sprintf((s), "%d", (v))

inline char* CharLower(char* s) { for (char* p = s; p && *p; ++p) *p = (char) tolower((unsigned char) *p); return s; }
inline char* CharUpper(char* s) { for (char* p = s; p && *p; ++p) *p = (char) toupper((unsigned char) *p); return s; }
inline char* CharLowerA(char* s) { return CharLower(s); }
inline char* CharUpperA(char* s) { return CharUpper(s); }
inline char* _strlwr(char* s) { return CharLower(s); }
inline char* _strupr(char* s) { return CharUpper(s); }
inline char* strlwr(char* s) { return CharLower(s); }
inline char* strupr(char* s) { return CharUpper(s); }

inline int MulDiv(int a, int b, int c)
{
	if (c == 0) return -1;
	int64_t r = (int64_t) a * b;
	r = (r >= 0) ? (r + c / 2) / c : (r - c / 2) / c;
	return (int) r;
}

// RECT / POINT yardımcıları
inline BOOL SetRect(RECT* r, int l, int t, int rt, int b)
{
	if (!r) return FALSE;
	r->left = l; r->top = t; r->right = rt; r->bottom = b;
	return TRUE;
}
inline BOOL SetRectEmpty(RECT* r) { return SetRect(r, 0, 0, 0, 0); }
inline BOOL IsRectEmpty(const RECT* r) { return (r->right <= r->left) || (r->bottom <= r->top); }
inline BOOL EqualRect(const RECT* a, const RECT* b) { return std::memcmp(a, b, sizeof(RECT)) == 0; }
inline BOOL CopyRect(RECT* d, const RECT* s) { *d = *s; return TRUE; }
inline BOOL PtInRect(const RECT* r, POINT p)
{
	return p.x >= r->left && p.x < r->right && p.y >= r->top && p.y < r->bottom;
}
inline BOOL OffsetRect(RECT* r, int dx, int dy)
{
	r->left += dx; r->right += dx; r->top += dy; r->bottom += dy;
	return TRUE;
}
inline BOOL InflateRect(RECT* r, int dx, int dy)
{
	r->left -= dx; r->right += dx; r->top -= dy; r->bottom += dy;
	return TRUE;
}
inline BOOL IntersectRect(RECT* d, const RECT* a, const RECT* b)
{
	d->left   = std::max(a->left, b->left);
	d->top    = std::max(a->top, b->top);
	d->right  = std::min(a->right, b->right);
	d->bottom = std::min(a->bottom, b->bottom);
	if (IsRectEmpty(d)) { SetRectEmpty(d); return FALSE; }
	return TRUE;
}
inline BOOL UnionRect(RECT* d, const RECT* a, const RECT* b)
{
	if (IsRectEmpty(a)) { *d = *b; return !IsRectEmpty(d); }
	if (IsRectEmpty(b)) { *d = *a; return TRUE; }
	d->left   = std::min(a->left, b->left);
	d->top    = std::min(a->top, b->top);
	d->right  = std::max(a->right, b->right);
	d->bottom = std::max(a->bottom, b->bottom);
	return TRUE;
}

// Zaman
inline DWORD timeGetTime()
{
	using namespace std::chrono;
	return (DWORD) duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
inline DWORD GetTickCount() { return timeGetTime(); }
inline ULONGLONG GetTickCount64()
{
	using namespace std::chrono;
	return (ULONGLONG) duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
inline BOOL QueryPerformanceFrequency(LARGE_INTEGER* f)
{
	f->QuadPart = 1000000000LL;
	return TRUE;
}
inline BOOL QueryPerformanceCounter(LARGE_INTEGER* c)
{
	using namespace std::chrono;
	c->QuadPart = (LONGLONG) duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
	return TRUE;
}
inline void Sleep(DWORD ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

// Sistem saati
typedef struct _SYSTEMTIME
{
	WORD wYear, wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, wMilliseconds;
} SYSTEMTIME, *LPSYSTEMTIME;
void GetLocalTime(SYSTEMTIME* st);
void GetSystemTime(SYSTEMTIME* st);
inline BOOL Beep(DWORD, DWORD) { return TRUE; }
inline UINT GetDoubleClickTime() { return 500; }

// Mesaj döngüsü (SDL platform katmanı kendi döngüsünü kullanır; bunlar yalnızca derleme uyumu için)
#define PM_REMOVE 0x0001
#define PM_NOREMOVE 0x0000
inline BOOL PeekMessage(MSG* msg, HWND, UINT, UINT, UINT) { if (msg) std::memset(msg, 0, sizeof(MSG)); return FALSE; }
inline BOOL GetMessage(MSG* msg, HWND, UINT, UINT) { if (msg) std::memset(msg, 0, sizeof(MSG)); return FALSE; }
inline BOOL TranslateMessage(const MSG*) { return FALSE; }
inline LRESULT DispatchMessage(const MSG*) { return 0; }
inline LRESULT DefWindowProc(HWND, UINT, WPARAM, LPARAM) { return 0; }
inline int lstrcmpA(const char* a, const char* b) { return std::strcmp(a, b); }

// Bellek (GlobalAlloc/GlobalFree — N3VMesh ve BitMapFile kullanıyor)
#define GMEM_FIXED    0x0000
#define GMEM_MOVEABLE 0x0002
#define GMEM_ZEROINIT 0x0040
#define GPTR          0x0040
inline HGLOBAL GlobalAlloc(UINT flags, size_t bytes) { return (flags & GMEM_ZEROINIT) ? std::calloc(1, bytes ? bytes : 1) : std::malloc(bytes ? bytes : 1); }
inline HGLOBAL GlobalFree(HGLOBAL h) { std::free(h); return nullptr; }
inline LPVOID GlobalLock(HGLOBAL h) { return h; }
inline BOOL GlobalUnlock(HGLOBAL) { return TRUE; }

// Yol birleştirme
inline void _makepath(char* path, const char* drive, const char* dir, const char* fname, const char* ext)
{
	std::string s;
	if (drive && *drive) s += drive;
	if (dir && *dir)
	{
		s += dir;
		if (s.back() != '/' && s.back() != '\\') s += '/';
	}
	if (fname && *fname) s += fname;
	if (ext && *ext)
	{
		if (*ext != '.') s += '.';
		s += ext;
	}
	std::strcpy(path, s.c_str());
}

// Ekran modu (mobilde ekran modu değiştirilemez; çağrılar başarı döner)
typedef struct _devicemode
{
	WORD dmSize;
	DWORD dmFields;
	DWORD dmBitsPerPel;
	DWORD dmPelsWidth;
	DWORD dmPelsHeight;
	DWORD dmDisplayFrequency;
} DEVMODE, *LPDEVMODE, DEVMODEA;
#define DM_BITSPERPEL 0x00040000
#define DM_PELSWIDTH  0x00080000
#define DM_PELSHEIGHT 0x00100000
#define DM_DISPLAYFREQUENCY 0x00400000
#define ENUM_CURRENT_SETTINGS  ((DWORD) -1)
#define ENUM_REGISTRY_SETTINGS ((DWORD) -2)
#define CDS_FULLSCREEN 0x00000004
#define DISP_CHANGE_SUCCESSFUL 0
BOOL EnumDisplaySettings(LPCSTR device, DWORD mode, DEVMODE* dm);
LONG ChangeDisplaySettings(DEVMODE* dm, DWORD flags);

// Kayıt defteri — dosya tabanlı öykünme (compat/win32/registry.cpp)
#define HKEY_CLASSES_ROOT  ((HKEY) (ULONG_PTR) 0x80000000)
#define HKEY_CURRENT_USER  ((HKEY) (ULONG_PTR) 0x80000001)
#define HKEY_LOCAL_MACHINE ((HKEY) (ULONG_PTR) 0x80000002)
#define ERROR_SUCCESS   0L
#define ERROR_MORE_DATA 234L
#define ERROR_FILE_NOT_FOUND 2L
#define REG_NONE   0
#define REG_SZ     1
#define REG_BINARY 3
#define REG_DWORD  4
#define KEY_READ 0x20019
#define KEY_WRITE 0x20006
#define KEY_ALL_ACCESS 0xF003F
LONG RegOpenKey(HKEY root, LPCSTR subKey, HKEY* out);
LONG RegOpenKeyEx(HKEY root, LPCSTR subKey, DWORD options, DWORD sam, HKEY* out);
LONG RegCreateKey(HKEY root, LPCSTR subKey, HKEY* out);
LONG RegSetValueEx(HKEY key, LPCSTR name, DWORD reserved, DWORD type, const BYTE* data, DWORD len);
LONG RegQueryValueEx(HKEY key, LPCSTR name, LPDWORD reserved, LPDWORD type, LPBYTE data, LPDWORD len);
LONG RegDeleteValue(HKEY key, LPCSTR name);
LONG RegCloseKey(HKEY key);
/// Kayıt defteri dosyasının yazılacağı dizin (varsayılan: çalışma dizini)
void KoRegistrySetDirectory(const char* dir);

// INI dosyaları (GetPrivateProfile*) ve hata kodu
DWORD GetPrivateProfileString(LPCSTR section, LPCSTR key, LPCSTR def, LPSTR out, DWORD outSize, LPCSTR file);
UINT GetPrivateProfileInt(LPCSTR section, LPCSTR key, INT def, LPCSTR file);
BOOL WritePrivateProfileString(LPCSTR section, LPCSTR key, LPCSTR value, LPCSTR file);
inline DWORD GetLastError() { return (DWORD) errno; }
inline void SetLastError(DWORD e) { errno = (int) e; }

// Geçici dosya
UINT GetTempFileName(LPCSTR path, LPCSTR prefix, UINT unique, LPSTR out);
inline UINT GetTempFileNameA(LPCSTR p, LPCSTR pre, UINT u, LPSTR o) { return GetTempFileName(p, pre, u, o); }
DWORD GetTempPath(DWORD n, LPSTR buf);
inline BOOL DeleteFile(LPCSTR path) { return std::remove(path) == 0; }
inline BOOL DeleteFileA(LPCSTR path) { return DeleteFile(path); }
inline BOOL CopyFile(LPCSTR src, LPCSTR dst, BOOL failIfExists)
{
	if (failIfExists) { FILE* t = std::fopen(dst, "rb"); if (t) { std::fclose(t); return FALSE; } }
	FILE* in = std::fopen(src, "rb"); if (!in) return FALSE;
	FILE* out = std::fopen(dst, "wb"); if (!out) { std::fclose(in); return FALSE; }
	char buf[65536]; size_t n;
	while ((n = std::fread(buf, 1, sizeof(buf), in)) > 0) std::fwrite(buf, 1, n, out);
	std::fclose(in); std::fclose(out);
	return TRUE;
}
inline BOOL MoveFile(LPCSTR src, LPCSTR dst) { return std::rename(src, dst) == 0; }
inline BOOL CreateDirectory(LPCSTR path, void*) { return mkdir(path, 0755) == 0; }

// Kabuk / yerel ayar
#define SW_SHOWNORMAL 1
HINSTANCE ShellExecute(HWND, LPCSTR op, LPCSTR file, LPCSTR params, LPCSTR dir, int show);
inline HINSTANCE ShellExecuteA(HWND h, LPCSTR op, LPCSTR f, LPCSTR p, LPCSTR d, int s) { return ShellExecute(h, op, f, p, d, s); }
inline WORD GetUserDefaultLangID() { return 0x0409; }
inline WORD GetSystemDefaultLangID() { return 0x0409; }
HCURSOR GetCursor();

// Pencere / imleç / çeşitli — platform kancalarına yönlendirilir.
HWND GetActiveWindow();
HWND GetForegroundWindow();
HWND GetFocus();
HWND SetFocus(HWND);
HWND KoWin32MainWindow();
void KoWin32SetMainWindow(HWND);
BOOL GetClientRect(HWND, RECT*);
BOOL GetWindowRect(HWND, RECT*);
BOOL GetCursorPos(POINT*);
BOOL SetCursorPos(int x, int y);
int ShowCursor(BOOL);
HCURSOR SetCursor(HCURSOR);
HCURSOR LoadCursor(HINSTANCE, LPCSTR);
HICON LoadIcon(HINSTANCE, LPCSTR);
void PostQuitMessage(int);
BOOL ShowWindow(HWND, int);
HWND SetActiveWindow(HWND);
BOOL SetWindowText(HWND, LPCSTR);
BOOL ClientToScreen(HWND, POINT*);
BOOL ScreenToClient(HWND, POINT*);
int MessageBox(HWND, LPCSTR text, LPCSTR caption, UINT type);
int MessageBoxA(HWND, LPCSTR text, LPCSTR caption, UINT type);
int MessageBoxW(HWND, LPCWSTR text, LPCWSTR caption, UINT type);
BOOL MoveWindow(HWND, int x, int y, int w, int h, BOOL repaint);
SHORT GetAsyncKeyState(int vk);
SHORT GetKeyState(int vk);
DWORD GetCurrentDirectory(DWORD n, LPSTR buf);
BOOL SetCurrentDirectory(LPCSTR path);
DWORD GetModuleFileName(HMODULE, LPSTR buf, DWORD n);
void _splitpath(const char* path, char* drive, char* dir, char* fname, char* ext);
void OutputDebugString(LPCSTR);
inline void OutputDebugStringA(LPCSTR s) { OutputDebugString(s); }
LRESULT SendMessage(HWND, UINT, WPARAM, LPARAM);
BOOL PostMessage(HWND, UINT, WPARAM, LPARAM);
#define SW_SHOW 5
#define SW_HIDE 0

#endif // !_WIN32
#endif // KO_MOBILE_COMPAT_WINDOWS_H
