// io.h uyumluluk sarmalayıcısı: _access vb. POSIX unistd.h ile karşılanır.
#pragma once
#if defined(_WIN32)
#include_next <io.h>
#else
#include <unistd.h>
#define _access access
#endif
