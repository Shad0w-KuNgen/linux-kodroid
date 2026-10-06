#ifndef CLIENT_N3BASE_N3TABLEBASEIMPL_H
#define CLIENT_N3BASE_N3TABLEBASEIMPL_H

#pragma once

#include <FileIO/File.h>

#include <string>

// NOLINTNEXTLINE(performance-enum-size): used for the file format, size must match
enum TBL_DATA_TYPE : uint32_t
{
	DT_NONE,
	DT_CHAR,
	DT_BYTE,
	DT_SHORT,
	DT_WORD,
	DT_INT,
	DT_DWORD,
	DT_STRING,
	DT_FLOAT,
	DT_DOUBLE,
	DT_INT64 // 2xxx verisi (Player_experience, SpecialAuction): 8 bayt
};

class CN3TableBaseImpl
{
protected:
	using DATA_TYPE = TBL_DATA_TYPE;

	CN3TableBaseImpl();

public:
	virtual ~CN3TableBaseImpl();
	virtual bool Load(File& file) = 0;
	bool LoadFromFile(const std::string& szFN);

protected:
	bool ReadData(File& file, DATA_TYPE DataType, void* pData);
	bool SkipData(File& file, DATA_TYPE DataType); // sütunu okuyup atar (2xxx verisindeki fazladan sütunlar)
	int SizeOf(DATA_TYPE DataType) const;
	static void LogTable(const std::string& szMsg); // _N3GAME: Log.txt, değilse stdout

	std::string m_szFileName; // LoadFromFile'da verilen ad (şema hizalaması için)

public:
	// Ham satır yakalama: dosya adı (küçük harf) s_szRowStringsFile'ı içeriyorsa her satır için TÜM sütunlar
	// metin olarak (sayılar ondalık, dizeler olduğu gibi) kancaya verilir. 2369 UIs_us.tbl'nin 347 sütununu
	// 1.298 yuvalarına ad tabanlı eşlemek için kullanılır.
	static void (*s_pfnRowStrings)(const std::string& szFile, uint32_t dwKey, const std::vector<std::string>& cols);
	static std::string s_szRowStringsFile;
};

#endif // CLIENT_N3BASE_N3TABLEBASEIMPL_H
