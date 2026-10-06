// N3AnimControl.cpp: implementation of the CN3AnimControl class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfxBase.h"
#include "N3AnimControl.h"
#include "KoTableCrypt.h"

#include <cstring>
#include <stdexcept>
#include <vector>

CN3AnimControl::CN3AnimControl()
{
	m_dwType |= OBJ_ANIM_CONTROL;
}

CN3AnimControl::~CN3AnimControl()
{
}

void CN3AnimControl::Release()
{
	m_Datas.clear(); // animation Data List

	CN3BaseFileAccess::Release();
}

bool CN3AnimControl::Load(File& file)
{
	if (!m_Datas.empty())
		Release();

	int nCount = 0;
	file.Read(&nCount, 4);

	m_Datas.clear(); // animation Data List

	// 2369 (ISTIRAP) .n3anim: her kayıt [u32 parçaUzunluğu][DES katmanı parçası (16 bayt sabit başlık + u32 BE uzunluk +
	// 8'lik bloklar, iç XOR yok)]; çözülen parça = [u16 9][1.298 kaydı: int (yer tutucu, 15.0f) + 11 alan + ad].
	// Tanıma: ilk kaydın uzunluğundan sonra sabit DES başlığı gelir.
	const int64_t iStart = static_cast<int64_t>(file.Offset());
	bool b2369           = false;
	if (nCount > 0 && nCount < 100000)
	{
		uint8_t byPeek[20] {};
		size_t rd = 0;
		if (file.Read(byPeek, sizeof(byPeek), &rd) && rd == sizeof(byPeek))
			b2369 = KoTableIsLayer2(byPeek + 4, 20); // boyut denetimi için 20 yeter (başlık karşılaştırması)
		file.Seek(iStart, SEEK_SET);
	}
	if (b2369)
	{
		for (int i = 0; i < nCount; i++)
		{
			uint32_t uLen = 0;
			if (!file.Read(&uLen, 4) || uLen < 20 || uLen > 4096)
				throw std::runtime_error("CN3AnimControl: 2369 kayıt uzunluğu geçersiz");
			std::vector<uint8_t> chunk(uLen);
			if (!file.Read(chunk.data(), uLen))
				throw std::runtime_error("CN3AnimControl: 2369 kayıt okunamadı");
			if (!KoTableLayer2DecryptDesOnly(chunk))
				throw std::runtime_error("CN3AnimControl: 2369 kayıt DES çözülemedi");
			// [u16][int yer tutucu][11 x 4 bayt][u32 adUzunluk][ad]
			if (chunk.size() < 2 + 4 + 44 + 4)
				throw std::runtime_error("CN3AnimControl: 2369 kayıt kısa");
			const uint8_t* p = chunk.data() + 2 + 4;
			__AnimData Data;
			auto f = [&](int k) { float v; memcpy(&v, p + k * 4, 4); return v; };
			Data.fFrmStart          = f(0);
			Data.fFrmEnd            = f(1);
			Data.fFrmPerSec         = f(2);
			Data.fFrmPlugTraceStart = f(3);
			Data.fFrmPlugTraceEnd   = f(4);
			Data.fFrmSound0         = f(5);
			Data.fFrmSound1         = f(6);
			Data.fTimeBlend         = f(7);
			memcpy(&Data.iBlendFlags, p + 8 * 4, 4);
			Data.fFrmStrike0 = f(9);
			Data.fFrmStrike1 = f(10);
			uint32_t uNL = 0;
			memcpy(&uNL, p + 11 * 4, 4);
			if (uNL > 256 || 2 + 4 + 48 + uNL > chunk.size())
				throw std::runtime_error("CN3AnimControl: 2369 kayıt adı geçersiz");
			Data.szName.assign((const char*) p + 12 * 4, uNL);
			m_Datas.push_back(Data);
		}
		return true;
	}

	for (int i = 0; i < nCount; i++)
	{
		__AnimData Data;
		Data.Load(file);
		m_Datas.push_back(Data);
	}

	return true;
}

#ifdef _N3TOOL
bool CN3AnimControl::Save(File& file)
{
	int nL    = 0;
	int iSize = static_cast<int>(m_Datas.size());
	file.Write(&iSize, 4);

	for (int i = 0; i < iSize; i++)
		m_Datas[i].Save(file);

	return true;
}
#endif // endof #ifdef _N3TOOL

#ifdef _N3TOOL
__AnimData* CN3AnimControl::Add()
{
	__AnimData Data;
	Data.szName = "No Name";
	m_Datas.push_back(Data);

	return &(m_Datas[m_Datas.size() - 1]);
}
#endif // endof #ifdef _N3TOOL

#ifdef _N3TOOL
__AnimData* CN3AnimControl::Insert(int nIndex)
{
	if (nIndex < 0 || nIndex >= static_cast<int>(m_Datas.size()))
		return nullptr;

	it_Ani it = m_Datas.begin();
	for (int i = 0; i < nIndex; i++, it++)
		;

	__AnimData Data;
	Data.szName = "No Name";
	it          = m_Datas.insert(it, Data);

	return &(*it);
}
#endif // endof #ifdef _N3TOOL

#ifdef _N3TOOL
void CN3AnimControl::Swap(int nAni1, int nAni2)
{
	if (nAni1 == nAni2)
		return;

	if (nAni1 < 0 || nAni1 >= static_cast<int>(m_Datas.size()))
		return;

	if (nAni2 < 0 || nAni2 >= static_cast<int>(m_Datas.size()))
		return;

	__AnimData Tmp = m_Datas[nAni2];
	m_Datas[nAni2] = m_Datas[nAni1];
	m_Datas[nAni1] = Tmp;
}
#endif // endof #ifdef _N3TOOL

#ifdef _N3TOOL
void CN3AnimControl::Delete(int nIndex)
{
	if (nIndex < 0 || nIndex >= static_cast<int>(m_Datas.size()))
		return;

	it_Ani it = m_Datas.begin();
	for (int i = 0; i < nIndex; i++, it++)
		;

	m_Datas.erase(it);
}
#endif // endof #ifdef _N3TOOL
