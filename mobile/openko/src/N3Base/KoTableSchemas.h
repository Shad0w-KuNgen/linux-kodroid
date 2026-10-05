// KoTableSchemas.h — 1.298 tablo şemaları (bkz. KoTableSchemas.cpp)
#ifndef KO_TABLE_SCHEMAS_H
#define KO_TABLE_SCHEMAS_H

#pragma once

#include <cstdint>
#include <string>

// Dosya adına (yol/harf duyarsız) göre 1.298 sütun tür dizgisi ("DTTT..."); bilinmiyorsa nullptr.
const char* KoTableSchema1298(const std::string& fileName);
uint32_t KoTableTypeFromLetter(char c);
char KoTableLetterFromType(uint32_t t);

#include <vector>

// 2xxx verisi: dosya sütun türlerini (fileTypes) oyunun 1.298 struct düzenine hizalar.
// outTypes = struct'ın beklediği tür listesi; colMap[dosya sütunu] = struct sütunu ya da -1 (atla).
// report: insan okunur özet (atlanan dosya sütunları, eşlenmeyen struct sütunları). Şema bilinmiyorsa false.
bool KoTableAlignSchema(const std::string& fileName, const std::vector<uint32_t>& fileTypes, std::vector<uint32_t>& outTypes,
	std::vector<int>& colMap, std::string& report);

// Türleri harf dizgisine çevirir ("DTTTI...")
std::string KoTableTypesToString(const std::vector<uint32_t>& types);

#endif // KO_TABLE_SCHEMAS_H
