#include "StdAfxBase.h"
#include "N3TableBaseImpl.h"
#include "KoTableCrypt.h"

#include <cstdio>
#include <vector>

#include <FileIO/FileReader.h>
#include <FileIO/FileWriter.h>

#ifdef _N3GAME
#include "LogWriter.h"
#endif

CN3TableBaseImpl::CN3TableBaseImpl()
{
}

CN3TableBaseImpl::~CN3TableBaseImpl()
{
}

void CN3TableBaseImpl::LogTable(const std::string& szMsg)
{
#ifdef _N3GAME
	CLogWriter::Write("{}", szMsg);
#else
	printf("%s\n", szMsg.c_str());
#endif
}

bool CN3TableBaseImpl::SkipData(File& file, DATA_TYPE DataType)
{
	if (DataType == DT_STRING)
	{
		int iStrLen = 0;
		file.Read(&iStrLen, sizeof(iStrLen));
		if (iStrLen > 0)
		{
			std::string sz(iStrLen, ' ');
			file.Read(&sz[0], iStrLen);
		}
		return true;
	}
	int n = SizeOf(DataType);
	if (n <= 0)
		return false;
	uint8_t tmp[8];
	file.Read(tmp, n);
	return true;
}

bool CN3TableBaseImpl::LoadFromFile(const std::string& szFN)
{
	if (szFN.empty())
		return false;
	m_szFileName = szFN;

	FileReader encryptedFile;
	if (!encryptedFile.OpenExisting(szFN))
	{
#ifdef _N3GAME
		CLogWriter::Write("N3TableBase - Can't open file(read) File Handle error ({})", szFN);
#endif
		return false;
	}

	std::error_code ec;

	// 파일 암호화 풀기.. .. 임시 파일에다 쓴다음 ..
	std::string szFNTmp      = szFN + ".tmp";
	size_t encryptedFileSize = static_cast<size_t>(encryptedFile.Size());
	if (encryptedFileSize == 0)
	{
		encryptedFile.Close();
		std::filesystem::remove(szFNTmp, ec); // 임시 파일 지우기..
		return false;
	}

	// 원래 파일을 읽고..
	std::vector<uint8_t> datas(encryptedFileSize);
	encryptedFile.Read(datas.data(), encryptedFileSize); // 암호화된 데이터 읽고..
	encryptedFile.Close();                               // 원래 파일 닫고

	if (KoTableIsLayer2(datas.data(), datas.size()))
	{
		// 2xxx (1886/2195/2369) verisi: DES katmanı + iç XOR; klasik XOR katmanı yok (KoTableCrypt.h)
		uint8_t prefix[5] = {};
		if (!KoTableLayer2Decrypt(datas, prefix))
		{
#ifdef _N3GAME
			CLogWriter::Write("N3TableBase - 2xxx DES katmanı çözüldü ama başlık geçersiz ({})", szFN);
#endif
			return false;
		}
#ifdef _N3GAME
		CLogWriter::Write("N3TableBase - 2xxx DES katmanı çözüldü ({}, {} bayt, önek {:02x}{:02x}{:02x}{:02x}{:02x})", szFN,
			datas.size(), prefix[0], prefix[1], prefix[2], prefix[3], prefix[4]);
#endif
	}
	else
	{
		// 테이블 만드는 툴에서 쓰는 키와 같은 키.. (klasik akış XOR'u: key_r 0x0816, c1 0x6081, c2 0x1608)
		KoTableXorDecrypt(datas.data(), datas.size());
		if (!KoTableHeaderLooksValid(datas.data(), datas.size()))
		{
#ifdef _N3GAME
			char szHex[16 * 2 + 1] = {};
			for (size_t i = 0; i < 16 && i < datas.size(); i++)
				snprintf(szHex + i * 2, 3, "%02x", datas[i]);
			CLogWriter::Write("N3TableBase - XOR sonrası başlık geçersiz ({}): boyut {} (%8={}), ilk16={}", szFN, datas.size(),
				datas.size() % 8, szHex);
#endif
		}
	}

	uint8_t* pDatas = datas.data();
	encryptedFileSize = datas.size();

	// TODO: Rather than write to file to read it back again, we should just read it from a memory stream.

	// 임시 파일에 쓴다음.. 다시 연다..
	{
		FileWriter tmpFileWriter;
		if (!tmpFileWriter.Create(szFNTmp))
		{
			tmpFileWriter.Close();
			return false;
		}

		tmpFileWriter.Write(pDatas, encryptedFileSize); // 임시파일에 암호화 풀린 데이터 쓰기
	}

	pDatas = nullptr;

	// 임시 파일 읽기 모드로 열기.
	FileReader decryptedFile;
	if (!decryptedFile.OpenExisting(szFNTmp))
	{
		std::filesystem::remove(szFNTmp, ec);
		return false;
	}

	bool bResult = Load(decryptedFile);
	decryptedFile.Close();

	if (!bResult)
	{
#ifdef _N3GAME
		CLogWriter::Write("N3TableBase - incorrect table ({})", szFN);
#endif
	}

	// 임시 파일 지우기..
	std::filesystem::remove(szFNTmp, ec);

	return bResult;
}

bool CN3TableBaseImpl::ReadData(File& file, DATA_TYPE DataType, void* pData)
{
	switch (DataType)
	{
		case DT_CHAR:
			file.Read(pData, sizeof(char));
			break;

		case DT_BYTE:
			file.Read(pData, sizeof(uint8_t));
			break;

		case DT_SHORT:
			file.Read(pData, sizeof(int16_t));
			break;

		case DT_WORD:
			file.Read(pData, sizeof(uint16_t));
			break;

		case DT_INT:
			file.Read(pData, sizeof(int));
			break;

		case DT_DWORD:
			file.Read(pData, sizeof(uint32_t));
			break;

		case DT_STRING:
		{
			std::string& szString = *((std::string*) pData);

			int iStrLen           = 0;
			file.Read(&iStrLen, sizeof(iStrLen));

			szString.clear();
			if (iStrLen > 0)
			{
				szString.assign(iStrLen, ' ');
				file.Read(&szString[0], iStrLen);
			}
		}
		break;

		case DT_FLOAT:
			file.Read(pData, sizeof(float));
			break;

		case DT_DOUBLE:
			file.Read(pData, sizeof(double));
			break;

		case DT_NONE:
		default:
			__ASSERT(0, "");
			return false;
	}

	return true;
}

int CN3TableBaseImpl::SizeOf(DATA_TYPE DataType) const
{
	switch (DataType)
	{
		case DT_CHAR:
			return sizeof(char);

		case DT_BYTE:
			return sizeof(uint8_t);

		case DT_SHORT:
			return sizeof(int16_t);

		case DT_WORD:
			return sizeof(uint16_t);

		case DT_INT:
			return sizeof(int);

		case DT_DWORD:
			return sizeof(uint32_t);

		case DT_STRING:
			return sizeof(std::string);

		case DT_FLOAT:
			return sizeof(float);

		case DT_DOUBLE:
			return sizeof(double);

		default:
			break;
	}

	__ASSERT(0, "");
	return 0;
}
