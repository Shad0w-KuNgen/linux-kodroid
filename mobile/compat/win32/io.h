// io.h uyumluluk sarmalayıcısı: _access ve _findfirst/_findnext/_findclose (POSIX ile).
#pragma once
#if defined(_WIN32)
#include_next <io.h>
#else
#include <windows.h>
#include <unistd.h>
#include <cstdint>
#include <ctime>
#define _access access

struct _finddata_t
{
	unsigned attrib;
	time_t time_create;
	time_t time_access;
	time_t time_write;
	unsigned long size;
	char name[260];
};
#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SUBDIR 0x10
#define _A_ARCH   0x20

/// Windows gibi büyük/küçük harf duyarsız joker eşleşmesi yapar ("*.N3Anim").
intptr_t _findfirst(const char* pattern, _finddata_t* fi);
int _findnext(intptr_t handle, _finddata_t* fi);
int _findclose(intptr_t handle);
#endif
