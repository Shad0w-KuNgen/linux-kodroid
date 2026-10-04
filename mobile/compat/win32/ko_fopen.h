// ko_fopen.h — OpenKO kaynaklarına derleyici tarafından zorla dahil edilir (-include).
// Doğrudan fopen() çağrılarının Windows yollarını ('\', harf duyarsız) çözmesini sağlar.
#pragma once
#if !defined(_WIN32)
#include <cstdio>
#include <string>
FILE* ko_fopen(const char* path, const char* mode);
namespace std { using ::ko_fopen; } // std::fopen çağrıları için
std::string KoResolvePath(const std::string& path);
#define fopen ko_fopen
#endif
