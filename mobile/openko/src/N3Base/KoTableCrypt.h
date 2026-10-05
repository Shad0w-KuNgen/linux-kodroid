// KoTableCrypt.h — Knight Online .tbl şifre katmanları.
//
// Klasik (tüm sürümler): akış XOR'u (key_r 0x0816, c1 0x6081, c2 0x1608), N3TableBaseImpl::LoadFromFile.
// 2xxx (1886/2195/2369 …): ham dosya = [16 bayt sabit başlık][uint32 BE özgün uzunluk][8 baytlık DES blokları]
// (IP/FP'siz DES: 16 tur Feistel, E/S/P tabloları, sabit 16×48 bitlik tur anahtarları). Çözülen veri
// uzunluğa kırpılır, akış XOR'u (0x0418/0x8041/0x1804) uygulanır; sonuç [5 bayt önek][standart N3 tablosu].
// Algoritma co3moz/ko-tbl-reader (MIT) decryption/double.js'den türetildi; örneklerle doğrulandı (ko_tbl_test).
#ifndef KO_TABLE_CRYPT_H
#define KO_TABLE_CRYPT_H

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

// XOR katmanı çözülmüş N3 tablo başlığı makul mu? (sütun sayısı 1..256, türler DT_CHAR..DT_DOUBLE, ilk sütun DT_DWORD)
bool KoTableHeaderLooksValid(const uint8_t* pData, size_t nSize);

// Ham dosya 2xxx DES katmanı düzeninde mi? (sabit 16 baytlık başlık, boyut % 8 == 4)
bool KoTableIsLayer2(const uint8_t* pData, size_t nSize);

// 2xxx DES katmanını çözer: data yerinde düz N3 tablosuna dönüşür (5 baytlık önek atılır). Başlık geçersizse false
// (data değişmemiş kalır). pPrefix verilirse atılan 5 bayt oraya yazılır.
bool KoTableLayer2Decrypt(std::vector<uint8_t>& data, uint8_t* pPrefix = nullptr);

// Klasik akış XOR katmanı (yerinde).
void KoTableXorDecrypt(uint8_t* pData, size_t nSize, uint16_t key_r = 0x0816, uint16_t c1 = 0x6081, uint16_t c2 = 0x1608);

#endif // KO_TABLE_CRYPT_H
