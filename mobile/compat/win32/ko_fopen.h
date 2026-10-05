// ko_fopen.h — OpenKO kaynaklarına derleyici tarafından zorla dahil edilir (-include).
// Doğrudan fopen() çağrılarının Windows yollarını ('\', harf duyarsız) çözmesini sağlar.
#pragma once
#if !defined(_WIN32)
#include <cstdio>
#include <string>
FILE* ko_fopen(const char* path, const char* mode);
namespace std { using ::ko_fopen; } // std::fopen çağrıları için
std::string KoResolvePath(const std::string& path);
/// Okuma için açılamayan dosyalar (yol başına bir kez) buraya düşer; platform katmanı Log.txt'ye yazar.
void KoSetMissingFileCallback(void (*fn)(const char* path, void* user), void* user);
void KoLogMissingFile(const char* path);
#define fopen ko_fopen
#endif
