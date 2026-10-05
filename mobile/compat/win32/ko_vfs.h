// ko_vfs.h — 2xxx istemci verisi için sanal dosya katmanı.
//
// 1) Klasör paketleri: 2369 istemcisinde "UI\" klasöründe tek tek .uif/.dxt yok; ui.hdr (şifresiz dizin:
//    u32 kayıt sayısı, her kayıt u32 adUzunluk | ad | u32 ofset | u32 boyut) + ui.src (dosyalar ham,
//    ardışık; UIF'in ilk u32'si N3 ad uzunluğu = 0, DXT'nin ilk alanı kendi adı). Aynı düzen başka
//    klasörler için de denenir: "<klasör>\<ad>" diskte yoksa <klasör>/<klasör>.hdr+.src, <klasör>.hdr+.src
//    ya da klasördeki ilk *.hdr/*.src çifti (ör. item/item.hdr) okunur; kayıt <klasör>_cache/<ad> altına
//    çıkarılır ve o yol döndürülür (bir kez). "ui_us\<ad>" için "ui" paketi de denenir.
// 2) .istirap: ISTIRAP sunucusunun şifreli UIF'leri (Pearl Guard dcpUIF): ilk 4 bayt düz; sonra
//    dosya boyutu çiftse 32, tekse 31 baytlık bloklar, her blok anahtar akışının başından RC4
//    (CryptDecrypt Final=TRUE). Anahtar = SHA1(parola[:29]) ilk 16 bayt. Çözülen içerik UIF'in kendisidir
//    (düz bırakılan 4 bayt = N3 ad uzunluğu 0). ui_cache/istirap/<ad>.uif.
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// Normalleştirilmiş ('/' ayraçlı) yol için paket/istirap karşılığı; yoksa boş dizge.
// Diskte bulunamayan "…/<klasör>/<ad>" yolları ile var olan "….istirap" yolları için çağrılır.
std::string KoVfsResolve(const std::string& normalizedPath);

// .istirap içeriğini çözer (bellek içi; test ve araçlar için)
void KoIstirapDecrypt(std::vector<uint8_t>& data);
// Çözülmüş .istirap yolu (ui_cache altında); hata: boş
std::string KoIstirapDecryptToCache(const std::string& existingPath);

// Paket indeksini sıfırla (test)
void KoVfsReset();

// Ham yardımcılar
void KoSha1(const void* data, size_t len, uint8_t out[20]);
void KoRc4(const uint8_t* key, size_t keyLen, uint8_t* data, size_t len);
