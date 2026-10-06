#include "StdAfx.h"
#include "KoUiSlots2369.h"
#include "GameDef.h"
#include <N3Base/LogWriter.h>
#include <map>
#include <regex>

namespace
{
std::map<uint32_t, std::vector<std::string>> g_rows;

std::string Lower(std::string s)
{
	for (char& c : s)
		c = (char) tolower((unsigned char) c);
	return s;
}

struct SlotRule
{
	const char* szSlot;
	std::string __TABLE_UI_RESRC::*pMember;
	const char* szPattern; // sütun değerine (küçük harf) uygulanan düzenli ifade; ilk eşleşen sütun alınır
	const char* szExclude; // eşleşmemesi gereken (nullptr = yok)
};

// Desenler ISTIRAP dosya adlarına göre; tam döküm geldikçe daraltılır. Yalnız 1.298 yuvası yanlış/eksik
// pencere yüklediği bilinen yuvalar listelenir.
const SlotRule RULES[] = {
	{ "szStateBar", &__TABLE_UI_RESRC::szStateBar, R"(\\(re|co|ka|el)_?(state_?bar|statebar|hud|mainstate|state2|state_new|hpbar)(_us)?\.(uif|istirap)$)", "soccer|page_state|otherstate" },
	{ "szMiniMap", &__TABLE_UI_RESRC::szMiniMap, R"(\\(re|co)_?mini_?map(_us)?\.(uif|istirap)$)", nullptr },
	{ "szState", &__TABLE_UI_RESRC::szState, R"(\\(re|co|ka|el)_page_state(_us)?\.(uif|istirap)$)", "other" },
	{ "szKnights", &__TABLE_UI_RESRC::szKnights, R"(\\(re|co|ka|el)_?(page_clan|knights)(_us)?\.(uif|istirap)$)", "operation|logo|cape" },
	{ "szKnightsOperation", &__TABLE_UI_RESRC::szKnightsOperation, R"(\\(re|co|ka|el)_?knights_?operation(_us)?\.(uif|istirap)$)", nullptr },
	{ "szNpcEvent", &__TABLE_UI_RESRC::szNpcEvent, R"(\\(re|co|ka|el)_?(npc_?event|smith)(_us)?\.(uif|istirap)$)", nullptr },
	{ "szExchangeRepair", &__TABLE_UI_RESRC::szExchangeRepair, R"(\\(re|co|ka|el)_?(exchange_?repair|repair)(_us)?\.(uif|istirap)$)", "tooltip|tooltop" },
	{ "szNpcExchangeList", &__TABLE_UI_RESRC::szNpcExchangeList, R"(\\(re|co|ka|el)_?(list2|npc_?exchange_?list|exchangelist)(_us)?\.(uif|istirap)$)", nullptr },
	{ "szFriends", &__TABLE_UI_RESRC::szFriends, R"(\\(re|co|ka|el)_page_friends(_us)?\.(uif|istirap)$)", nullptr },
	{ "szExitMenu", &__TABLE_UI_RESRC::szExitMenu, R"(\\(re|co|ka|el)_?exit_?menu(_us)?\.(uif|istirap)$)", nullptr },
	{ "szChat", &__TABLE_UI_RESRC::szChat, R"(\\(re|co|ka|el)_?chat(ting)?(_box)?(_us)?\.(uif|istirap)$)", "open|close|target" },
	{ "szMsgOutput", &__TABLE_UI_RESRC::szMsgOutput, R"(\\(re|co|ka|el)_?(information_box|msgoutput|message_output)(_us)?\.(uif|istirap)$)", nullptr },
	{ "szZoneChangeOrWarp", &__TABLE_UI_RESRC::szZoneChangeOrWarp, R"(\\(re|co|ka|el)_?warp(_us)?\.(uif|istirap)$)", "list" },
	{ "szItemUpgrade", &__TABLE_UI_RESRC::szItemUpgrade, R"(\\(re|co|ka|el)_?item_?upgrade(_us)?\.(uif|istirap)$)", nullptr },
	{ "szPartyOrForce", &__TABLE_UI_RESRC::szPartyOrForce, R"(\\(re|co|ka|el)_party(_us)?\.(uif|istirap)$)", "board|bbs|msg" },
};
} // namespace

void KoUiRowCapture(const std::string& /*szFile*/, uint32_t dwKey, const std::vector<std::string>& cols)
{
	g_rows[dwKey] = cols;
}

const std::vector<std::string>* KoUiCapturedColumns(uint32_t dwNationKey)
{
	auto it = g_rows.find(dwNationKey);
	return it == g_rows.end() ? nullptr : &it->second;
}

void KoUiApplySlots2369(uint32_t dwNationKey, __TABLE_UI_RESRC& r)
{
	const std::vector<std::string>* pCols = KoUiCapturedColumns(dwNationKey);
	if (pCols == nullptr)
		return;
	const std::vector<std::string>& cols = *pCols;
	std::vector<std::string> lower;
	lower.reserve(cols.size());
	for (const std::string& c : cols)
		lower.push_back(Lower(c));

	int iChanged = 0;
	for (const SlotRule& rule : RULES)
	{
		std::regex re(rule.szPattern, std::regex::icase);
		std::regex* pEx = nullptr;
		std::regex ex;
		if (rule.szExclude)
		{
			ex  = std::regex(rule.szExclude, std::regex::icase);
			pEx = &ex;
		}
		std::string& szSlot = r.*(rule.pMember);
		const std::string szCur = Lower(szSlot);
		// Mevcut değer zaten desene uyuyorsa dokunma
		if (std::regex_search(szCur, re) && (pEx == nullptr || !std::regex_search(szCur, *pEx)))
			continue;
		int iFound = -1;
		for (size_t i = 1; i < lower.size(); i++)
		{
			if (lower[i].empty() || !std::regex_search(lower[i], re))
				continue;
			if (pEx != nullptr && std::regex_search(lower[i], *pEx))
				continue;
			// ISTIRAP (.istirap) ve "re_" sürümlerini tercih et: ilk eşleşme yeterli, ama .istirap varsa onu seç
			if (iFound < 0 || (lower[i].find(".istirap") != std::string::npos && lower[(size_t) iFound].find(".istirap") == std::string::npos))
				iFound = (int) i;
		}
		if (iFound < 0)
		{
			CLogWriter::Write("UI yuva {} (ulus {}): desen için sütun bulunamadı, mevcut \"{}\" kalıyor", rule.szSlot, dwNationKey, szSlot);
			continue;
		}
		CLogWriter::Write("UI yuva {} (ulus {}): \"{}\" -> [{}] \"{}\"", rule.szSlot, dwNationKey, szSlot, iFound, cols[(size_t) iFound]);
		szSlot = cols[(size_t) iFound];
		iChanged++;
	}
	// Tanılama: tüm sütunlar (bir kez, ulus başına) — tam eşleme için
	std::string szDump;
	for (size_t i = 0; i < cols.size(); i++)
		if (!cols[i].empty() && cols[i] != "0")
			szDump += fmt::format("[{}]{} ", i, cols[i]);
	CLogWriter::Write("UIs_us.tbl ulus {} sütunları ({} değişti): {}", dwNationKey, iChanged, szDump);
}


// ---------------------------------------------------------------------------
// 2369 Zones.tbl: 28 sütun; istemci struct'ı 24. Atlanan metin sütunları (9, 25, 26) arasında bölge müziği var.
// ---------------------------------------------------------------------------
namespace
{
std::map<uint32_t, std::vector<std::string>> g_ZoneRows;
}

void KoZoneRowCapture(const std::string& /*szFile*/, uint32_t dwKey, const std::vector<std::string>& cols)
{
	g_ZoneRows[dwKey] = cols;
}

std::string KoZoneBgmFile(int iZone)
{
	for (int key : {iZone, iZone / 10, iZone * 10})
	{
		auto it = g_ZoneRows.find((uint32_t) key);
		if (it == g_ZoneRows.end())
			continue;
		for (const std::string& c : it->second)
		{
			std::string low = c;
			for (char& ch : low)
				ch = (char) tolower((unsigned char) ch);
			if (low.find(".ogg") != std::string::npos || low.find(".mp3") != std::string::npos || low.find(".wav") != std::string::npos
				|| low.rfind("snd\\", 0) == 0 || low.rfind("snd/", 0) == 0)
				return c;
		}
	}
	return std::string();
}
