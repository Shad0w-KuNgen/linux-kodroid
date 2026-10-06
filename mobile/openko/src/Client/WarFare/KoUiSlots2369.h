#pragma once
// 2369 (ISTIRAP) UIs_us.tbl: 347 sütun. 1.298 struct'ı (__TABLE_UI_RESRC) sütunları konumla alır; 2369 tablosunda bazı
// yuvalar başka pencerelere ayrılmış (ör. [5] co_soccer_state.uif durum çubuğu yuvasında). Bu modül satırın tüm
// sütunlarını yakalar ve her yuvayı ad desenine göre en uygun sütunla yeniden eşler; sonucu günlükler.
#include <string>
#include <vector>
#include <cstdint>

struct __TABLE_UI_RESRC;
class CN3UIBase;

void KoUiRowCapture(const std::string& szFile, uint32_t dwKey, const std::vector<std::string>& cols);
// Yakalanan sütunlara göre yuvaları yeniden eşle (yalnız 2369). Değişen yuvalar Log.txt'ye yazılır.
void KoUiApplySlots2369(uint32_t dwNationKey, __TABLE_UI_RESRC& r);
// Yakalanan sütunları (varsa) döndür — tanılama günlüğü için
const std::vector<std::string>* KoUiCapturedColumns(uint32_t dwNationKey);
/// 2369 Zones.tbl satır yakalama (ses/müzik sütunları struct'ta yok)
void KoZoneRowCapture(const std::string& szFile, uint32_t dwKey, const std::vector<std::string>& cols);
/// Bölgenin müzik dosyası (ör. "Snd\\21_moradon.ogg"); yoksa boş. iZone ve iZone/10 denenir.
std::string KoZoneBgmFile(int iZone);
/// 2369 pencerelerinde varsayılan açık gelen alt grupları (ör. sohbetin kanal filtresi, çantanın "yeniden diz" onayı)
/// ağaçta derinlemesine bulup gizler; bulunanlar Log.txt'ye yazılır.
void KoUiHideDeep(CN3UIBase* pRoot, const std::vector<std::string>& ids, const char* szWhere);
