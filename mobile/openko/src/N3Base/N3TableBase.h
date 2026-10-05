// N3TableBase.h: interface for the CN3TableBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_N3TABLEBASE_H__DD4F005E_05B0_49E3_883E_94BE6C8AC7EF__INCLUDED_)
#define AFX_N3TABLEBASE_H__DD4F005E_05B0_49E3_883E_94BE6C8AC7EF__INCLUDED_

#pragma once

#include <vector>
#include <map>
#include <string>

#include "My_3DStruct.h" // _ASSERT
#include "N3TableBaseImpl.h"
#include "KoTableSchemas.h"

template <typename Type>
class CN3TableBase : public CN3TableBaseImpl
{
public:
	using MAP_TYPE = std::map<uint32_t, Type>;

	CN3TableBase();
	~CN3TableBase() override;

	// Attributes
protected:
	std::vector<DATA_TYPE> m_DataTypes; // 실제 사용되는 정보의 데이타 타입
	MAP_TYPE m_Datas;                   // 실제 사용되는 정보

										// Operations
public:
	const MAP_TYPE& GetMap() const
	{
		return m_Datas;
	}

	Type* Find(uint32_t dwID) // ID로 data 찾기
	{
		auto it = m_Datas.find(dwID);
		if (it == m_Datas.end())
			return nullptr; // 찾기에 실패 했다!~!!

		return &it->second;
	}

	int GetSize() const
	{
		return static_cast<int>(m_Datas.size());
	}

	// index로 찾기..
	Type* GetIndexedData(int index)
	{
		if (index < 0 || index >= static_cast<int>(m_Datas.size()))
			return nullptr;

		auto it = m_Datas.begin();
		std::advance(it, index);
		return &it->second;
	}

	// 해당 ID의 Index 리턴..	Skill에서 쓴다..
	bool IDToIndex(uint32_t dwID, int* index)
	{
		auto it = m_Datas.find(dwID);
		if (it == m_Datas.end())
			return false; // 찾기에 실패 했다!~!!

		auto itSkill = m_Datas.begin();
		int iSize    = static_cast<int>(m_Datas.size());
		for (int i = 0; i < iSize; i++, itSkill++)
		{
			if (itSkill == it)
			{
				*index = i;
				return true;
			}
		}

		return false;
	}

	void Release();

protected:
	bool Load(File& file) override;
	bool MakeOffsetTable(std::vector<int>& offsets);
};

// cpp파일에 있으니까 link에러가 난다. 왜 그럴까?

template <class Type>
CN3TableBase<Type>::CN3TableBase()
{
}

template <class Type>
CN3TableBase<Type>::~CN3TableBase()
{
	Release();
}

template <class Type>
void CN3TableBase<Type>::Release()
{
	m_DataTypes.clear(); // data type 저장한것 지우기
	m_Datas.clear();     // row 데이타 지우기
}

template <class Type>
bool CN3TableBase<Type>::Load(File& file)
{
	Release();

	// data(column) 의 구조가 어떻게 되어 있는지 읽기
	int iDataTypeCount = 0;
	file.Read(&iDataTypeCount, 4); // (엑셀에서 column 수)
	__ASSERT(iDataTypeCount > 0, "Data Type 이 0 이하입니다.");
	if (iDataTypeCount <= 0 || iDataTypeCount > 1024)
		return false;

	std::vector<DATA_TYPE> fileTypes(iDataTypeCount, DT_NONE);
	file.Read(&fileTypes[0], sizeof(DATA_TYPE) * iDataTypeCount); // 각각의 column에 해당하는 data type

	// Dosya sütunu -> struct sütunu eşlemesi. 1.298 verisinde birebir; 2xxx verisinde (sütun eklenmiş)
	// dosya sütunları 1.298 şemasına hizalanır (KoTableSchemas), fazlalıklar atlanır.
	std::vector<int> colMap(iDataTypeCount);
	for (int i = 0; i < iDataTypeCount; i++)
		colMap[i] = i;
	m_DataTypes = fileTypes;

	std::vector<int> offsets;
	bool bDirect = MakeOffsetTable(offsets) && offsets[iDataTypeCount] == (int) sizeof(Type) && DT_DWORD == m_DataTypes[0];
	if (!bDirect)
	{
		std::vector<uint32_t> ft(fileTypes.begin(), fileTypes.end()), et;
		std::string report;
		if (!KoTableAlignSchema(m_szFileName, ft, et, colMap, report))
		{
			LogTable("N3TableBase - sütun düzeni struct'a uymuyor ve hizalanamadı: " + report);
			m_DataTypes.clear();
			return false;
		}
		m_DataTypes.clear();
		for (uint32_t t : et)
			m_DataTypes.push_back((DATA_TYPE) t);
		offsets.clear();
		if (!MakeOffsetTable(offsets) || offsets[(int) m_DataTypes.size()] != (int) sizeof(Type) || DT_DWORD != m_DataTypes[0])
		{
			LogTable("N3TableBase - 1.298 şeması struct boyutuyla uyuşmuyor: " + report);
			m_DataTypes.clear();
			return false;
		}
		LogTable("N3TableBase - 2xxx şeması hizalandı: " + report);
	}

	// row 가 몇줄인지 읽기
	int iRC = 0;
	file.Read(&iRC, sizeof(iRC));
	if (iRC < 0 || iRC > 5000000)
		return false;

	Type Data {};
	for (int i = 0; i < iRC; i++)
	{
		for (int j = 0; j < iDataTypeCount; j++)
		{
			int k = colMap[j];
			if (k >= 0)
				// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
				ReadData(file, fileTypes[j], reinterpret_cast<char*>(&Data) + offsets[k]);
			else
				SkipData(file, fileTypes[j]);
		}

		uint32_t dwKey           = *((uint32_t*) (&Data));
		[[maybe_unused]] auto pt = m_Datas.insert(std::make_pair(dwKey, Data));

		__ASSERT(pt.second, "CN3TableBase<Type> : Key 중복 경고.");
	}

	if (!bDirect || m_Datas.empty())
		LogTable("N3TableBase - " + m_szFileName + ": " + std::to_string(iDataTypeCount) + " sütun, "
			+ std::to_string(m_Datas.size()) + " satır" + (m_Datas.empty() ? "" : ", ilk anahtar " + std::to_string(m_Datas.begin()->first)));

	return true;
}

// structure는 4바이트 정렬하여서 메모리를 잡는다. 따라서 아래 함수가 필요하다.
// 아래 함수로 OffsetTable을 만들어 쓴 후에는 만드시 리턴값을 delete [] 를 해주어야 한다.
template <class Type>
bool CN3TableBase<Type>::MakeOffsetTable(std::vector<int>& offsets)
{
	if (m_DataTypes.empty())
		return false;

	static constexpr int StructAlignment = alignof(Type);

	int iDataTypeCount                   = (int) m_DataTypes.size();

	offsets.clear();
	offsets.resize(iDataTypeCount + 1);
	offsets[0]        = 0;

	int iPrevDataSize = SizeOf(m_DataTypes[0]);
	for (int i = 1; i < iDataTypeCount; i++)
	{
		int iCurDataSize    = SizeOf(m_DataTypes[i]);
		int iPreviousOffset = offsets[i - 1];

		int modulo          = (iCurDataSize % StructAlignment);
		if (0 == modulo)
		{
			modulo = (iPreviousOffset + iPrevDataSize) % StructAlignment;
			if (0 == modulo)
				offsets[i] = iPreviousOffset + iPrevDataSize;
			else
				offsets[i] = ((int) (iPreviousOffset + iPrevDataSize + (StructAlignment - 1))
								 / StructAlignment)
							 * StructAlignment;
		}
		else if (1 == modulo)
		{
			offsets[i] = iPreviousOffset + iPrevDataSize;
		}
		else if (2 == modulo)
		{
			modulo = ((iPreviousOffset + iPrevDataSize) % 2);
			if (0 == modulo)
				offsets[i] = iPreviousOffset + iPrevDataSize;
			else
				offsets[i] =
					iPreviousOffset + iPrevDataSize
					+ 1; // NOTE: Effectively this is (2 - modulo), but modulo can only be 1 here.
		}
		else if (4 == modulo)
		{
			modulo = ((iPreviousOffset + iPrevDataSize) % 4);
			if (0 == modulo)
				offsets[i] = iPreviousOffset + iPrevDataSize;
			else
				offsets[i] = iPreviousOffset + iPrevDataSize + (4 - modulo);
		}
		else
		{
			__ASSERT(0, "");
		}

		iPrevDataSize = iCurDataSize;
	}

	offsets[iDataTypeCount] = ((int) (offsets[iDataTypeCount - 1] + iPrevDataSize
									  + (StructAlignment - 1))
								  / StructAlignment)
							  * StructAlignment;
	return true;
}

#endif // !defined(AFX_N3TABLEBASE_H__DD4F005E_05B0_49E3_883E_94BE6C8AC7EF__INCLUDED_)
