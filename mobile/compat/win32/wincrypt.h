// wincrypt.h uyumluluk sarmalayıcısı. CryptoAPI yerine N3Base/WinCrypt.cpp'nin
// taşınabilir sürümü (SHA-1 + RC4) kullanılır; burada yalnızca tipler tanımlanır.
#pragma once
#if defined(_WIN32)
#include_next <wincrypt.h>
#else
#include <windows.h>
typedef ULONG_PTR HCRYPTPROV;
typedef ULONG_PTR HCRYPTHASH;
typedef ULONG_PTR HCRYPTKEY;
#define PROV_RSA_FULL 1
#define CRYPT_VERIFYCONTEXT 0xF0000000
#define CRYPT_NEWKEYSET 0x8
#define CALG_SHA 0x8004
#define CALG_SHA1 0x8004
#define CALG_RC4 0x6801
#define MS_ENHANCED_PROV "Microsoft Enhanced Cryptographic Provider v1.0"
#endif
