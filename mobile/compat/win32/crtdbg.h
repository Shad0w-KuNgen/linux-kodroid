// crtdbg.h uyumluluk sarmalayıcısı (MSVC hata ayıklama CRT'si yok).
#pragma once
#if defined(_WIN32)
#include_next <crtdbg.h>
#else
#include <cassert>
#define _CrtDbgBreak() ((void) 0)
#define _ASSERT(e) assert(e)
#define _ASSERTE(e) assert(e)
#endif
