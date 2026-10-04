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

typedef struct tagRGBQUAD
{
	BYTE rgbBlue;
	BYTE rgbGreen;
	BYTE rgbRed;
	BYTE rgbReserved;
} RGBQUAD;

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
