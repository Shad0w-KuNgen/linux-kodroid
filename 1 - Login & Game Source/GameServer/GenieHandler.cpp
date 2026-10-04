#include "stdafx.h"
#include "DBAgent.h"

using std::string;
using std::unique_ptr;
extern CDBAgent g_DBAgent;

#pragma region CUser::HandleGenie(Packet & pkt)
void CUser::HandleGenie(Packet & pkt)
{
	uint8 command = pkt.read<uint8>();

	switch (command)
	{
	case GenieInfoRequest:
		GenieNonAttackProgress(pkt);
		break;
	case GenieUpdateRequest:
		GenieAttackProgress(pkt);
		break;
	case 25:
		GenieNotice(pkt);
		break;
	default:
		//printf("Genie OpCode %u \n", command);				// Hata ! Burasý sürekli uyarý veriyor. Opcode 3 veriyor sürekli.
		TRACE("Genie OpCode %u \n", command);
		break;
	}
}
#pragma endregion

void CUser::GenieNotice(Packet& pkt)
{
	bool status = pkt.read<bool>();
	std::string notice = "Karakteriniz Genie ile hiç bir þekilde yürüme fonksiyonu gerçekleþtirmez.";
	if (status) notice = "Yürüme fonksiyonu aktif edilmiþtir.";
	g_pMain->SendGameNotice(ChatType::GENERAL_CHAT, notice, "", false, this, false);
}

#pragma region CUser::GenieNonAttackProgress(Packet & pkt)
void CUser::GenieNonAttackProgress(Packet & pkt)
{
	uint8 command = pkt.read<uint8>();

	switch (command)
	{
	case GenieUseSpiringPotion:
		GenieUseGenieSpirint(pkt);
		break;
	case GenieLoadOptions:
		HandleGenieLoadOptions();
		break;
	case GenielSaveOptions:
		HandleGenieSaveOptions(pkt);
		break;
	case GenieStartHandle:
		GenieStart();
		break;
	case GenieStopHandle:
		GenieStop();
		break;
	default:
		TRACE("GenieNonAttackProgress Unknow Attack Handle %d\n", command);
		//printf("GenieNonAttackProgress Unknow Attack Handle %d\n", command);		// Hata ! Burasý sürekli uyarý veriyor.  - Handle 155 & 22 & 130 & 90 & 87 Uyarýsý
		break;
	}

}
#pragma endregion

#pragma region CUser::HandleGenieLoadOptions()
void CUser::HandleGenieLoadOptions()
{
	// DeaFSoft Genie seçeneklerinin yüklenmeye baþlandýðýný bildirelim
	//g_pMain->SendHelpDescription(this, "Genie Seçenekleri Yükleniyor...");

	Packet result(WIZ_GENIE, uint8(GenieInfoRequest));
	result << uint8(GenieLoadOptions);

	for (int i = 0; i < sizeof(m_GenieOptions); i++) {
		result << uint8(*(uint8*)(m_GenieOptions + i));
	}

	// DeaFSoft Seçenekler baþarýyla gönderildi
	Send(&result);

	// DeaFSoft Kullanýcýya baþarý mesajý gönder
	//g_pMain->SendHelpDescription(this, "Genie Seçenekleri Baþarýyla Yüklendi."); 
}
#pragma endregion

#pragma region CUser::HandleGenieSaveOptions(Packet & pkt)
void CUser::HandleGenieSaveOptions(Packet& pkt)
{
	// DeaFSoft Gelen paket uzunluðunu kontrol edelim
	if (pkt.size() < sizeof(m_GenieOptions)) 
	{
		//g_pMain->SendHelpDescription(this, "Genie Seçenekleri Kaydedilemedi : Eksik Veri.");
		return;
	}

	// DeaFSoft Genie seçeneklerini paket içeriðinden oku
	for (int i = 0; i < sizeof(m_GenieOptions); i++) 
	{
		m_GenieOptions[i] = pkt.read<uint8>();
	}

	// DeaFSoft Kaydedildiðini kullanýcýya bildir
	//g_pMain->SendHelpDescription(this, "Genie Seçenekleri Baþarýyla Kaydedildi.");
}
#pragma endregion

#pragma region CUser::GenieAttackProgress(Packet & pkt)
void CUser::GenieAttackProgress(Packet& pkt)
{
	uint8 command = pkt.read<uint8>();

	// DeaFSoft Süre dolmuþsa Genie'yi durdur ve kullanýcýya haber ver
	if (UNIXTIME > m_1098GenieTime) 
	{
		g_pMain->SendHelpDescription(this, "Genie Süresi Doldu, Saldýrý Durduruluyor.");
		return SendGenieStop(true);
	}

	switch (command)
	{
	case GenieMove:
		MoveProcess(pkt);
		break;

	case GenieRotate:
		Rotate(pkt);
		break;

	case GenieMainAttack:
		Attack(pkt);
		break;

	case GenieMagic:
		CMagicProcess::MagicPacket(pkt, this);
		break;

	default:
		// DeaFSoft Bilinmeyen komut tespit edildiðinde oyuncuya Notice gönder
		TRACE("Genie Unknown Attack Handle %d\n", command);
		break;
	}
}
#pragma endregion

#pragma region CUser::GenieStart()
void CUser::GenieStart()
{
	// DeaFSoft Premium kontrolü veya Genie süresi dolmuþsa izin verme. 27.04.2025
	if ((g_pMain->pServerSetting.LootandGeniePremium && GetPremium() == 0) || UNIXTIME > m_1098GenieTime)
	{
		// DeaFSoft Hile korumasý : Premium yoksa ya da süresi bitmiþse ve Genie açmaya çalýþýyorsa.
		TRACE("[GENÝE HACK] = Oyuncu : [ %s ] Genie Baþlatmaya Çalýsýyor Dc Edildi! Siktir Git.\n", GetName().c_str());
		g_pMain->SendHelpDescription(this, string_format("[GENÝE HACK] = Oyuncu : [ %s ] Ýzinsiz Genie Açmaya Çalýsýyor Dc Edildi! Siktir Git.", GetName().c_str()));
		Disconnect();
		return;
	}

	// DeaFSoft Normal doðru Genie açýlýþý
	Packet result(WIZ_GENIE, uint8(GenieStatusActive));
	result << uint8(4) << uint16(1) << GetGenieTime();
	m_bGenieStatus = true;
	Send(&result);
	SendGenieStart(true);

	bool NoticeGenie = true;

	// DeaFSoft Genie Baþlýklý ve Renkli Notice Ekliyoruz. 27.04.2025
	if (NoticeGenie)
	{
		uint32 genieTime = GetGenieTime(); // DeaFSoft Genie süresini alýyoruz.
		g_pMain->SendHelpDescription(this, string_format("[GENÝE AKTÝF] = Genie Kalan Süresi : [ -%d ] Kaldý. Baþarýyla Aktif Edildi.", genieTime));
	}
}
#pragma endregion

#pragma region CUser::SendGenieStart(bool isToRegion /* = false */)
void CUser::SendGenieStart(bool isToRegion /* = false */)
{
	Packet result(WIZ_GENIE, uint8(GenieInfoRequest));
	result << uint8(GenieStartHandle) << uint16(1) << GetGenieTime();
	Send(&result);
	Packet newpkt(XSafe, uint8(0xDB));
	newpkt << uint8(1);
	Send(&newpkt);
	Packet result2(WIZ_GENIE, uint8(GenieInfoRequest));
	result2 << uint8(GenieActivated) << uint16(GetID()) << uint8(1);

	if (isToRegion) SendToRegion(&result2, nullptr, GetEventRoom());
	else Send(&result2);
	
	if (m_bGenieStatus == GenieStatusInactive) GenieStop();
	//SetOffCha(_choffstatus::ACTIVE, offcharactertype::genie);
}
#pragma endregion

#pragma region CUser::GenieStop()
void CUser::GenieStop()
{
	if (m_bGenieStatus == GenieStatusInactive) return;

	Packet newpkt(XSafe, uint8(0xDB));
	newpkt << uint8(0);
	Send(&newpkt);

	Packet result(WIZ_GENIE, uint8(1));
	result << uint8(5) << uint16(1) << GetGenieTime();
	m_bGenieStatus = false;
	SendGenieStop(true);
	Send(&result);

	{
		Packet newpkt(XSafe, uint8(0xDB));
		newpkt << uint8(0);
		Send(&newpkt);
	}

	bool NoticeGenie = true;

	// DeaFSoft Genie Baþlýklý Notice Ekliyoruz. 27.04.2025
	if (NoticeGenie)
	{
		// DeaFSoft Genie iþlemi Durdurulurken, iþlemin süresiyle ilgili bilgiyi içeren Notice ekliyoruz.
		uint32 genieTime = GetGenieTime();  // DeaFSoft Genie süresini alýyoruz.
		g_pMain->SendHelpDescription(this, string_format("[GENÝE DURDUR] = Genie Durduruluyor. Kalan Süresi : [ -%d ] Kaldý.", genieTime));  // DeaFSoft Genie süresi, saniye cinsinden gösteriliyor.
	}
}
#pragma endregion

#pragma region CUser::SendGenieStop()
void CUser::SendGenieStop(bool isToRegion /* = false */)
{
	Packet result(WIZ_GENIE, uint8(GenieInfoRequest));
	if (GetGenieTime() > 0) 
		result << uint8(GenieStopHandle) << uint16(1) << GetGenieTime();
	Send(&result);
	Packet newpkt(XSafe, uint8(0xDB));
	newpkt << uint8(0);
	Send(&newpkt);
	Packet result2(WIZ_GENIE, uint8(GenieInfoRequest));
	result2 << uint8(GenieActivated) << uint16(GetID()) << uint8(0);
	if (isToRegion) SendToRegion(&result2, nullptr, GetEventRoom());
	else Send(&result2);
	//SetOffCha(_choffstatus::DEACTIVE, offcharactertype::genie);
}
#pragma endregion

#pragma region CUser::UpdateGenieTime(uint16 m_sTime)
void CUser::UpdateGenieTime(uint16 m_sTime)
{
	Packet result(WIZ_GENIE, uint8(GenieInfoRequest));
	result << uint8(GenieRemainingTime) << m_sTime;
	Send(&result);

	bool NoticeGenie = true;

	// DeaFSoft Genie Süre Bittiðinde Notice Ekliyoruz. 27.04.2025
	if (m_sTime == GenieStatusInactive)
	{
		g_pMain->SendHelpDescription(this, string_format("[GENÝE SÜRE BÝTTÝ] = Genie Süreniz Doldu : [ -%d ], Sona Erdi.", m_sTime)); // Genie bittiðinde mesaj Eklendi.
		GenieStop();
	}
    else 
	{
		g_pMain->SendHelpDescription(this, string_format("[GENÝE KALAN] = Kullanýcý : %s Genie Kalan Süresi : [ -%d ] Kaç Kaldý.", GetName().c_str(), m_sTime));
    }
}
#pragma endregion

#pragma region CUser::GenieUseGenieSpirint()
void CUser::GenieUseGenieSpirint(Packet & pkt)
{

#if 0
	if (isTrading() || isMerchanting() || isMining() || isFishing()) return;

	uint32 nItemID; uint16 GenieItem;
	pkt >> nItemID;

	_ITEM_TABLE* ItemTable = g_pMain->GetItemPtr(nItemID);
	if (ItemTable == nullptr)
		return;

	GenieItem = GetItemCount(nItemID);

	if (!CheckExistItem(nItemID))
		return;

	if (nItemID != 810305000 && nItemID != 810378000  && nItemID != 900772000)
		return;

	if (RobItem(nItemID))
		m_GenieTime = 120;

	if (m_sFirstUsingGenie <= 0)
		m_sFirstUsingGenie = 1;

	Packet result(WIZ_GENIE, uint8(GenieUseSpiringPotion));
	result << uint8(GenieUseSpiringPotion) << GetGenieTime();
	Send(&result);
#endif // 0
}
#pragma endregion

#pragma region CDBAgent::UpdateGenieData(string& strCharID, CUser* pUser)
bool CDBAgent::UpdateGenieData(string& strCharID, CUser* pUser)
{
	// DeaFSoft Karakter ID uyuþmazlýðý
	if (strCharID != pUser->GetName()) 
	{
		if (pUser != nullptr) g_pMain->SendHelpDescription(pUser, ("Genie Verisi Güncellemesi Baþarýsýz : Yetkisiz Karakter (%s).", strCharID.c_str()));
		else 
		printf("Genie Verisi Güncellemesi Baþarýsýz: Yetkisiz Karakter (%s).\n", strCharID.c_str());
	}

	unique_ptr<OdbcCommand> dbCommand(m_GameDB->CreateCommand());
	if (dbCommand.get() == nullptr) 
	{
		if (pUser != nullptr) g_pMain->SendHelpDescription(pUser, "Genie Verisi Güncellemesi Baþarýsýz : Veritabaný Komutu Oluþturulamadý.");
		else
		printf("Genie Verisi Güncellemesi Baþarýsýz : Veritabaný Komutu Oluþturulamadý.\n");
		return false;
	}

	// DeaFSoft Parametreleri ekle
	dbCommand->AddParameter(SQL_PARAM_INPUT, strCharID.c_str(), strCharID.length());
	dbCommand->AddParameter(SQL_PARAM_INPUT, pUser->GetAccountName().c_str(), pUser->GetAccountName().length());
	dbCommand->AddParameter(SQL_PARAM_INPUT, (char*)pUser->m_GenieOptions, sizeof(pUser->m_GenieOptions), SQL_BINARY);

	// DeaFSoft Stored procedure çaðrýsý
	string proc = string_format(_T("{CALL UPDATE_GENIE_DATA(?,?, ?, %d,%d)}"), (uint32)pUser->m_1098GenieTime, pUser->m_sFirstUsingGenie);

	if (!dbCommand->Execute(proc)) 
	{
		// DeaFSoft Hata raporu
		ReportSQLError(m_GameDB->GetError());
		// DeaFSoft Oyuncuya notice
		if (pUser != nullptr) g_pMain->SendHelpDescription(pUser, "Genie Verisi Güncellemesi Baþarýsýz : Sunucu Hatasý.");
		else
		printf("Genie Verisi Güncellemesi Baþarýsýz : Sunucu Hatasý.\n");
		return false;
	}

	return true;
}
#pragma endregion

#pragma region CDBAgent::LoadGenieData(string& strCharID, CUser* pUser)
bool CDBAgent::LoadGenieData(string& strCharID, CUser* pUser)
{
	if (pUser == nullptr || strCharID != pUser->GetName()) 
	{
		// DeaFSoft Kullanýcý geçerli deðil veya karakter adý uyuþmuyor
		if (pUser != nullptr) g_pMain->SendHelpDescription(pUser, "Genie Verisi Yüklenemedi : Geçersiz Kullanýcý.");
		else
		printf("Genie Verisi Yüklenemedi : Geçersiz Kullanýcý.\n");
		return false;
	}

	unique_ptr<OdbcCommand> dbCommand(m_GameDB->CreateCommand());
	if (dbCommand.get() == nullptr) 
	{
		// DeaFSoft Veritabaný komutu oluþturulamadý
		if (pUser != nullptr) g_pMain->SendHelpDescription(pUser, "Genie Verisi Yüklenemedi : Veritabaný Baðlantý Hatasý.");
		else
		printf("Genie Verisi Yüklenemedi : Geçersiz Kullanýcý.\n");
		return false;
	}

	dbCommand->AddParameter(SQL_PARAM_INPUT, strCharID.c_str(), strCharID.length());
	dbCommand->AddParameter(SQL_PARAM_INPUT, pUser->GetAccountName().c_str(), pUser->GetAccountName().length());

	// DeaFSoft Veritabanýnda veriyi yüklemeye çalýþ
	if (!dbCommand->Execute(_T("{CALL LOAD_GENIE_DATA(?,?)}"))) 
	{
		ReportSQLError(m_GameDB->GetError());
		if (pUser != nullptr) g_pMain->SendHelpDescription(pUser, "Genie Verisi Yüklenemedi : Sunucu Hatasý.");
		else
		printf("Genie Verisi Yüklenemedi : Sunucu Hatasý.\n");
		return false;
	}

	// DeaFSoft Veritabanýndan veri yoksa
	if (!dbCommand->hasData()) 
	{
		if (pUser != nullptr) g_pMain->SendHelpDescription(pUser, "Genie Verisi Bulunamadý.");
		else
		printf("Genie Verisi Bulunamadý.\n");
		return false;
	}

	// DeaFSoft Veri baþarýlý þekilde yüklendi
	int field = 1;
	uint32 genietime = 0;
	dbCommand->FetchUInt32(field++, genietime);
	dbCommand->FetchBinary(field++, (char*)pUser->m_GenieOptions, sizeof(pUser->m_GenieOptions));
	dbCommand->FetchByte(field++, pUser->m_sFirstUsingGenie);

	pUser->m_1098GenieTime = genietime;

	return true;
}
#pragma endregion

#pragma region CUser::GenieExchange(uint32 itemid, uint32 time, bool newChar) 
bool CUser::GenieExchange(uint32 itemid, uint32 time, bool newChar) 
{
	// DeaFSoft Geçersiz süre veya eþya kontrolü
	if (!time || (!newChar && !itemid)) 
	{
		g_pMain->SendHelpDescription(this, "Genie Deðiþimi Baþarýsýz : Geçersiz Süre Veya Eþya.");
		return false;
	}

	// DeaFSoft Eþya kullanýmý gerekiyor, varsa kontrol et ve sil
	if (!newChar) 
	{
		if (!CheckExistItem(itemid)) 
		{
			g_pMain->SendHelpDescription(this, string_format("Genie Deðiþimi Baþarýsýz: Item Bulunamadý (ID: %u).", itemid));
			return false;
		}
		if (!RobItem(itemid)) 
		{
			g_pMain->SendHelpDescription(this, string_format("Genie Deðiþimi Baþarýsýz: Item Silinemedi (ID: %u).", itemid));
			return false;
		}
		g_pMain->SendHelpDescription(this, "Eþya Baþarýyla Kullanýldý. Güle Güle");
	}

	// DeaFSoft Ýlk kullaným flag'ini ayarla
	if (!m_sFirstUsingGenie) 
	{
		m_sFirstUsingGenie = 1;
		g_pMain->SendHelpDescription(this, "Genie Ýlk Kez Kullanýldý, Bonus Süreler Aktif.");
	}

	// DeaFSoft Kalan sürenin hesaplanmasý
	int remtime = int(m_1098GenieTime > UNIXTIME ? m_1098GenieTime - UNIXTIME : 0);
	m_1098GenieTime = UNIXTIME + (time * HOUR) + (remtime > 0 ? remtime : 0);

	// DeaFSoft Kullanýcýya yeni süre bilgisini gönder
	uint32 newRemaining = GetGenieTime();
	g_pMain->SendHelpDescription(this, string_format("Genie Etkinleþtirildi! Kalan Süre : [ %d ] Oldunuz Tebrikler!", newRemaining));

	// DeaFSoft Ýstemciye WIZ_GENIE paketi
	if (!newChar) 
	{
		Packet result(WIZ_GENIE, uint8(GenieUseSpiringPotion));
		result << uint8(GenieUseSpiringPotion) << GetGenieTime();
		Send(&result);
	}

	return true;
}
#pragma endregion

#pragma region CUser::CheckGenieTime()
void CUser::CheckGenieTime() 
{
	// DeaFSoft Genie'nin süresi dolmuþsa iþlemi sonlandýr
	if (UNIXTIME > m_1098GenieTime) 
	{
		g_pMain->SendHelpDescription(this, "Genie Süresi Dolmuþ, Sistem Sonlandýrýlýyor.");
		GenieStop();
	}

	// DeaFSoft Genie zaman bilgisini istemciye gönder
	Packet result(WIZ_GENIE, uint8(GenieInfoRequest));
	result << uint8(GenieRemainingTime) << GetGenieTime();
	Send(&result);
}
#pragma endregion

bool CDBAgent::LoadPriestBotGenieData(string& strCharID, CUser* pUser)
{
	if (pUser == nullptr
		|| strCharID != pUser->GetName())
		return false;

	unique_ptr<OdbcCommand> dbCommand(m_GameDB->CreateCommand());
	if (dbCommand.get() == nullptr)
		return false;

	dbCommand->AddParameter(SQL_PARAM_INPUT, strCharID.c_str(), strCharID.length());

	if (!dbCommand->Execute(_T("{CALL LOAD_GENIE_DATA(?)}")))
		ReportSQLError(m_GameDB->GetError());

	if (!dbCommand->hasData())
		return false;

	CBot* pPriest = nullptr;
	pPriest = g_pMain->m_MapBotList.GetData(pUser->m_bUserPriestBotID);
	if (!pPriest)
		return false;

	int field = 1;
	dbCommand->FetchUInt16(field++, pPriest->m_GenieTime);
	dbCommand->FetchBinary(field++, (char*)pPriest->m_GenieOptions, sizeof(pPriest->m_GenieOptions));
	dbCommand->FetchByte(field++, pPriest->m_sFirstUsingGenie);

	return true;
}

