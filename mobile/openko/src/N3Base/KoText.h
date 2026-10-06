// KoText.h — oyun metni kod sayfası.
// 1.298 verisi/sunucusu: ASCII + CP949 (Korece, yüksek bitli bayt = çift baytlı karakterin ilk baytı).
// ISTIRAP 2369 (Türkçe): Windows-1254, tek baytlı; yüksek bitli bayt tek başına bir karakterdir (ç ğ ı ö ş ü …).
// Eski kod her yerde "0x80 & c → 2 bayt" varsayıyordu; "Geçici" → "Ge??ci" gibi bozulmaların sebebi buydu.
#pragma once

#include <cstdint>
#include <string>

int KoTextCodePage();          // 949 (varsayılan) ya da 1254
void KoSetTextCodePage(int cp);

// Bu bayttan başlayan karakterin bayt uzunluğu (1 ya da 2)
inline int KoTextCharLen(char c)
{
	return ((uint8_t) c & 0x80) && KoTextCodePage() != 1254 ? 2 : 1;
}

// Windows-1254 bayt → Unicode kod noktası
uint32_t KoText1254ToUnicode(uint8_t by);
// Unicode kod noktası → Windows-1254 baytı (0 = temsil edilemez)
uint8_t KoTextUnicodeTo1254(uint32_t cp);
// UTF-8 → oyun kodlaması (1254: tabloyla; temsil edilemeyen karakterler atlanır)
std::string KoTextUtf8To1254(const std::string& utf8);
// Sunucudan gelen metin: kod sayfası 1254 iken dize geçerli UTF-8 çok baytlı diziler içeriyorsa 1254'e çevrilir
// (ISTIRAP sohbet/duyuruları UTF-8 gelir, U+FFFD → '?'); değilse olduğu gibi kalır.
std::string KoTextNormalizeIncoming(const std::string& s);
