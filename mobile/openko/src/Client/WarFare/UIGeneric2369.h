// 2369 arayüz dosyalarını (ör. re_powerupstore.istirap) özel sınıfı olmadan açıp kapatan genel pencere.
// Kapat/çıkış/iptal düğmeleri pencereyi gizler; içerik düğmeleri şimdilik sunucuya bağlı değildir.
#pragma once
#include "N3Base/N3UIBase.h"
#include <string>

class CUIGeneric2369 : public CN3UIBase
{
public:
	CUIGeneric2369() = default;
	~CUIGeneric2369() override = default;

	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	void Toggle();
	void Open();
	void Close();
	/// Dosyayı yükleyip ekranın ortasına yerleştirir; dosya yoksa false
	bool LoadCentered(const std::string& szFile, int iScreenW, int iScreenH, const char* szLogName);

	std::string m_szLogName;
};
