// N3BaseFileAccess.h: interface for the CN3BaseFileAccess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3BASEFILEACCESS_H__C99953BD_12BE_4B37_823F_4F4B2379FF74__INCLUDED_)
#define AFX_N3BASEFILEACCESS_H__C99953BD_12BE_4B37_823F_4F4B2379FF74__INCLUDED_

#pragma once

#include "N3Base.h"
#include <string>

#include <FileIO/File.h>

enum e_N3FormatVersion : uint16_t
{
	N3FORMAT_VER_UNKN = 0,
	N3FORMAT_VER_1098 = 1098,
	N3FORMAT_VER_1264 = 1264,
	N3FORMAT_VER_1298 = 1298,
};

inline constexpr e_N3FormatVersion N3FORMAT_VER_DEFAULT = N3FORMAT_VER_1298;

// UIF düğüm sürümüne göre kapanış sesinden sonraki ek bayt sayısı: v0/v1 yok, v2 → 2, v3 ve üstü → 3
inline int N3UIExtraBytesForNodeVersion(int16_t sNodeVersion)
{
	if (sNodeVersion <= 1)
		return 0;
	return sNodeVersion == 2 ? 2 : 3;
}

// Sondaki '\0', boşluk ve satır sonlarını at (2xxx UIF araçlarının yazdığı ID/dosya adları)
inline void N3TrimTrailingJunk(std::string& sz)
{
	while (!sz.empty())
	{
		char c = sz.back();
		if (c == '\0' || c == ' ' || c == '\t' || c == '\r' || c == '\n')
			sz.pop_back();
		else
			break;
	}
}

class CN3BaseFileAccess : public CN3Base
{
protected:
	static constexpr int MAX_SUPPORTED_PATH_LENGTH = 260;

	std::string m_szFileName; // Base Path 를 제외한 로컬 경로 + 파일 이름

public:
	uint32_t m_iFileFormatVersion;
	// UIF düğüm sürümü (1264 başlığındaki int16 "idk"): 0 eski (string'de satır aralığı yok), 1 = 1264,
	// 2 = 2xxx (kapanış sesinden sonra 2 ek bayt), 3 = 2369 (3 ek bayt). Her düğüm kendi değerini taşır.
	int16_t m_sNodeVersion = 1;
	int m_iLOD; // 로딩할때 쓸 LOD

public:
	// Full Path
	const std::string& FileName() const
	{
		return m_szFileName;
	}

	void FileNameSet(const std::string& szFileName);

	bool LoadFromFile();                                      // 파일에서 읽어오기.
	virtual bool LoadFromFile(const std::string& szFileName); // 파일에서 읽어오기.
	virtual bool LoadSupportedVersions(File& file);
	virtual bool Load(File& file);                            // 핸들에서 읽어오기..

	virtual bool SaveToFile();                                // 현재 파일 이름대로 저장.
	virtual bool SaveToFile(const std::string& szFileName);   // 새이름으로 저장.
	virtual bool Save(File& file);                            // 핸들을 통해 저장..

public:
	void Release() override;

	CN3BaseFileAccess();
	~CN3BaseFileAccess() override;
};

#endif // !defined(AFX_N3BASEFILEACCESS_H__C99953BD_12BE_4B37_823F_4F4B2379FF74__INCLUDED_)
