// KoText.cpp — bkz. KoText.h
#include "KoText.h"

namespace
{
int g_codePage = 949;

// Windows-1254: 0x80-0x9F özel; 0xA0-0xFF Latin-1 ile aynı, 6 istisna: D0 Ğ, DD İ, DE Ş, F0 ğ, FD ı, FE ş
const uint16_t HIGH_80_9F[32] = { 0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160,
	0x2039, 0x0152, 0x008D, 0x008E, 0x008F, 0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122,
	0x0161, 0x203A, 0x0153, 0x009D, 0x009E, 0x0178 };
} // namespace

int KoTextCodePage()
{
	return g_codePage;
}

void KoSetTextCodePage(int cp)
{
	g_codePage = (cp == 1254) ? 1254 : 949;
}

uint32_t KoText1254ToUnicode(uint8_t by)
{
	if (by < 0x80)
		return by;
	if (by < 0xA0)
		return HIGH_80_9F[by - 0x80];
	switch (by)
	{
		case 0xD0: return 0x011E; // Ğ
		case 0xDD: return 0x0130; // İ
		case 0xDE: return 0x015E; // Ş
		case 0xF0: return 0x011F; // ğ
		case 0xFD: return 0x0131; // ı
		case 0xFE: return 0x015F; // ş
		default: return by;       // Latin-1
	}
}

uint8_t KoTextUnicodeTo1254(uint32_t cp)
{
	if (cp < 0x80)
		return (uint8_t) cp;
	switch (cp)
	{
		case 0x011E: return 0xD0;
		case 0x0130: return 0xDD;
		case 0x015E: return 0xDE;
		case 0x011F: return 0xF0;
		case 0x0131: return 0xFD;
		case 0x015F: return 0xFE;
		case 0x20AC: return 0x80;
		case 0x2018: return 0x91;
		case 0x2019: return 0x92;
		case 0x201C: return 0x93;
		case 0x201D: return 0x94;
		case 0x2013: return 0x96;
		case 0x2014: return 0x97;
		case 0x2026: return 0x85;
		default: break;
	}
	if (cp >= 0xA0 && cp <= 0xFF && cp != 0xD0 && cp != 0xDD && cp != 0xDE && cp != 0xF0 && cp != 0xFD && cp != 0xFE)
		return (uint8_t) cp;
	return 0;
}

std::string KoTextNormalizeIncoming(const std::string& s)
{
	if (g_codePage != 1254)
		return s;
	bool bMulti = false;
	for (size_t i = 0; i < s.size();)
	{
		uint8_t c = (uint8_t) s[i];
		if (c < 0x80)
		{
			++i;
			continue;
		}
		int n = (c >= 0xF0) ? 4 : (c >= 0xE0) ? 3 : (c >= 0xC2) ? 2 : 0;
		if (n == 0 || i + n > s.size())
			return s; // geçerli UTF-8 değil → 1254 varsay
		for (int k = 1; k < n; k++)
			if (((uint8_t) s[i + k] & 0xC0) != 0x80)
				return s;
		bMulti = true;
		i += n;
	}
	if (!bMulti)
		return s;
	std::string out;
	out.reserve(s.size());
	for (size_t i = 0; i < s.size();)
	{
		uint8_t c   = (uint8_t) s[i];
		uint32_t cp = c;
		int n       = 1;
		if (c >= 0xF0)
		{
			cp = ((c & 0x07u) << 18) | ((s[i + 1] & 0x3Fu) << 12) | ((s[i + 2] & 0x3Fu) << 6) | (s[i + 3] & 0x3Fu);
			n  = 4;
		}
		else if (c >= 0xE0)
		{
			cp = ((c & 0x0Fu) << 12) | ((s[i + 1] & 0x3Fu) << 6) | (s[i + 2] & 0x3Fu);
			n  = 3;
		}
		else if (c >= 0xC0)
		{
			cp = ((c & 0x1Fu) << 6) | (s[i + 1] & 0x3Fu);
			n  = 2;
		}
		i += n;
		uint8_t by = KoTextUnicodeTo1254(cp);
		out.push_back(by != 0 ? (char) by : '?');
	}
	return out;
}

std::string KoTextUtf8To1254(const std::string& in)
{
	std::string out;
	out.reserve(in.size());
	for (size_t i = 0; i < in.size();)
	{
		uint8_t c   = (uint8_t) in[i];
		uint32_t cp = c;
		int n       = 1;
		if (c >= 0xF0 && i + 3 < in.size())
		{
			cp = ((c & 0x07u) << 18) | ((in[i + 1] & 0x3Fu) << 12) | ((in[i + 2] & 0x3Fu) << 6) | (in[i + 3] & 0x3Fu);
			n  = 4;
		}
		else if (c >= 0xE0 && i + 2 < in.size())
		{
			cp = ((c & 0x0Fu) << 12) | ((in[i + 1] & 0x3Fu) << 6) | (in[i + 2] & 0x3Fu);
			n  = 3;
		}
		else if (c >= 0xC0 && i + 1 < in.size())
		{
			cp = ((c & 0x1Fu) << 6) | (in[i + 1] & 0x3Fu);
			n  = 2;
		}
		i += n;
		uint8_t by = KoTextUnicodeTo1254(cp);
		if (by != 0)
			out.push_back((char) by);
	}
	return out;
}
