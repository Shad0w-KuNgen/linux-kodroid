// KoTableSchemas.cpp — 1.298 istemci verisindeki tablo şemaları (sütun türleri), dosya adına göre.
// Üretildi: mobile/scripts/tbl_tool.py (1.298 Data/*.tbl). Harf: C char, B byte, S short, W word, I int,
// D dword, T string, F float, R double. CN3TableBase::Load, 2xxx verisinde sütun düzeni farklıysa dosya
// sütunlarını bu şemaya (oyunun struct'ına) hizalar.
#include "KoTableSchemas.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <string>
#include <vector>

namespace
{
struct Entry
{
	const char* name;
	const char* types;
};
const Entry SCHEMAS[] = {
	{ "cloak.tbl", "DTDDB" },
	{ "disguisering.tbl", "DTIDDBT" },
	{ "disguisering_us.tbl", "DTIDDBT" },
	{ "exchange_quest.tbl", "DDTIIIIIBBBBBBIIIIII" },
	{ "item_ext_0_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_10_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_11_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_12_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_13_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_14_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_15_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_16_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_17_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_18_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_19_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_1_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_20_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_21_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_22_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_23_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_2_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_3_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_4_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_5_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_6_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_7_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_8_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_ext_9_us.tbl", "DTDTDDDBSSSSSSSSSSSSSBBBBBBBBBSSSSSSSSSSSSSDDSSSSSSSS" },
	{ "item_org_us.tbl", "DBTTDBDDDDBBBBBSSSSSIISBDDCCBBBBBBBBB" },
	{ "npc_looks.tbl", "DTTTTTTTTTTTTTTTTIIIIIIIIIIIIIIIIIIBBB" },
	{ "npc_shout_us.tbl", "DT" },
	{ "newchrvalue.tbl", "DTIIIIIIDDDDDDDDDDDD" },
	{ "quest_content_us.tbl", "DIITTT" },
	{ "quest_menu_us.tbl", "DT" },
	{ "quest_talk_us.tbl", "DT" },
	{ "skill_magic_1.tbl", "DIIIIIIIIIII" },
	{ "skill_magic_2.tbl", "DIIIII" },
	{ "skill_magic_6.tbl", "DTTIIIIIIIIIIIIIIIIIBDDDDBDD" },
	{ "skill_magic_7.tbl", "DI" },
	{ "skill_magic_main_us.tbl", "DTTTIIIIIIIIIIIIIIIDDIIFFIDDII" },
	{ "texts_us.tbl", "DT" },
	{ "ui_help_us.tbl", "DTT" },
	{ "uis_us.tbl", "DTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT" },
	{ "upc_defaultlooks.tbl", "DTTTTTTTTTTTTTTTTIIIIIIIIIIIIIIIIIIBBB" },
	{ "web_address_us.tbl", "DTTBTBTBTB" },
	{ "zones.tbl", "DTTTTTTTTIITTITFTDDDDTIT" },
	{ "fx.tbl", "DTTIB" },
	{ "help_us.tbl", "DIIITT" },
	{ "skill_magic_3.tbl", "DIIIIII" },
	{ "skill_magic_4.tbl", "DIIIIIIIIIIIIIIIIIIIIIIIII" },
	{ "skill_magic_9.tbl", "DTTBBIBBBBIBBB" },
	{ "sound.tbl", "DTII" },
};
} // namespace

const char* KoTableSchema1298(const std::string& fileName)
{
	// yol ve büyük/küçük harf duyarsız
	std::string base = fileName;
	size_t p = base.find_last_of("\\/");
	if (p != std::string::npos)
		base = base.substr(p + 1);
	std::transform(base.begin(), base.end(), base.begin(), [](unsigned char c) { return (char) std::tolower(c); });
	for (const Entry& e : SCHEMAS)
		if (base == e.name)
			return e.types;
	return nullptr;
}

uint32_t KoTableTypeFromLetter(char c)
{
	switch (c)
	{
		case 'C': return 1;
		case 'B': return 2;
		case 'S': return 3;
		case 'W': return 4;
		case 'I': return 5;
		case 'D': return 6;
		case 'T': return 7;
		case 'F': return 8;
		case 'R': return 9;
		case 'Q': return 10;
		default: return 0;
	}
}

char KoTableLetterFromType(uint32_t t)
{
	static const char L[] = "?CBSWIDTFRQ";
	return t < 11 ? L[t] : '?';
}

std::string KoTableTypesToString(const std::vector<uint32_t>& types)
{
	std::string s;
	for (uint32_t t : types)
		s.push_back(KoTableLetterFromType(t));
	return s;
}

namespace
{
std::string BaseLower(const std::string& fileName)
{
	std::string base = fileName;
	size_t p = base.find_last_of("\\/");
	if (p != std::string::npos)
		base = base.substr(p + 1);
	std::transform(base.begin(), base.end(), base.begin(), [](unsigned char c) { return (char) std::tolower(c); });
	return base;
}

// Elle bilinen eşlemeler: dosya sütun sayısına göre, dosya sütunu -> struct sütunu (-1 atla)
struct ManualMap
{
	const char* name;
	int fileCols;
	const char* map; // virgülle ayrılmış struct sütun indeksleri, "x" = atla
};
const ManualMap MANUAL_MAPS[] = {
	// 2195/2369 Zones: 9. sütun (HDR gökyüzü) eklendi, sona .mob ve .NaviMesh eklendi
	{ "zones.tbl", 27, "0,1,2,3,4,5,6,7,8,x,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,x,x" },
	// 2369 (ISTIRAP) Zones (DTTTTTTTTTIITTITFTDDDDTITTTI): 9. sütun HDR gökyüzü (moradon_hdr.n3sky) atlanır,
	// sonda .evtsub/.mob/.NaviMesh/int atlanır (yerel tbl_tool dökümü, satır 210)
	{ "zones.tbl", 28, "0,1,2,3,4,5,6,7,8,x,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,x,x,x" },
};

bool ParseManual(const char* map, std::vector<int>& colMap)
{
	colMap.clear();
	std::string cur;
	for (const char* p = map;; ++p)
	{
		if (*p == ',' || *p == '\0')
		{
			if (cur == "x")
				colMap.push_back(-1);
			else if (!cur.empty())
				colMap.push_back(std::atoi(cur.c_str()));
			cur.clear();
			if (*p == '\0')
				break;
		}
		else
			cur.push_back(*p);
	}
	return !colMap.empty();
}
} // namespace

bool KoTableAlignSchema(const std::string& fileName, const std::vector<uint32_t>& fileTypes, std::vector<uint32_t>& outTypes,
	std::vector<int>& colMap, std::string& report)
{
	report.clear();
	const char* schema = KoTableSchema1298(fileName);
	if (schema == nullptr)
	{
		report = "1.298 şeması bilinmiyor: " + BaseLower(fileName) + " (dosya " + KoTableTypesToString(fileTypes) + ")";
		return false;
	}
	outTypes.clear();
	for (const char* c = schema; *c; ++c)
		outTypes.push_back(KoTableTypeFromLetter(*c));
	const int nf = (int) fileTypes.size(), ne = (int) outTypes.size();
	colMap.assign(nf, -1);

	std::string base = BaseLower(fileName);
	bool manual      = false;
	for (const ManualMap& m : MANUAL_MAPS)
	{
		if (base == m.name && m.fileCols == nf)
		{
			std::vector<int> cm;
			if (ParseManual(m.map, cm) && (int) cm.size() == nf)
			{
				bool ok = true;
				for (int i = 0; i < nf && ok; i++)
					ok = cm[i] < 0 || (cm[i] < ne && fileTypes[i] == outTypes[cm[i]]);
				if (ok)
				{
					colMap = cm;
					manual = true;
				}
			}
			break;
		}
	}

	if (!manual)
	{
		// En uzun ortak alt dizi (sıra korunur): eşit türler eşlenir, araya eklenen dosya sütunları atlanır
		std::vector<std::vector<int>> L(nf + 1, std::vector<int>(ne + 1, 0));
		for (int i = nf - 1; i >= 0; i--)
			for (int j = ne - 1; j >= 0; j--)
				L[i][j] = (fileTypes[i] == outTypes[j]) ? L[i + 1][j + 1] + 1 : std::max(L[i + 1][j], L[i][j + 1]);
		int i = 0, j = 0;
		while (i < nf && j < ne)
		{
			if (fileTypes[i] == outTypes[j])
			{
				colMap[i] = j;
				i++;
				j++;
			}
			else if (L[i + 1][j] >= L[i][j + 1])
				i++;
			else
				j++;
		}
	}

	std::vector<bool> used(ne, false);
	std::string skipped, unmatched;
	for (int i = 0; i < nf; i++)
	{
		if (colMap[i] >= 0)
			used[colMap[i]] = true;
		else
			skipped += (skipped.empty() ? "" : ",") + std::to_string(i);
	}
	for (int j = 0; j < ne; j++)
		if (!used[j])
			unmatched += (unmatched.empty() ? "" : ",") + std::to_string(j);
	report = base + ": dosya " + std::to_string(nf) + " sütun [" + KoTableTypesToString(fileTypes) + "], struct "
		+ std::to_string(ne) + " sütun [" + schema + "]" + (manual ? ", elle eşleme" : ", otomatik hizalama")
		+ "; atlanan dosya sütunları [" + skipped + "]; eşlenmeyen struct sütunları [" + unmatched + "]";
	return colMap.size() > 0 && colMap[0] == 0 && outTypes[0] == 6;
}
