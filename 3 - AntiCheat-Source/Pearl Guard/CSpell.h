#include "stdafx.h"
struct CSpell;
#pragma once
struct CSpell
{
public:
	uint32_t dwID; // 1
	std::string szEngName; // 2 0x4
	std::string szKrName; // 3 0x20
	std::string szDesc; // 4 0x3C
	int iSelfAnimID1; // 5 0x40
	int iSelfAnimID2; // 6 0x44
	int idwTargetAnimID; // 7 0x48
	int iSelfFX1; // 8 0x4C
	int iSelfPart1; // 9 0x50
	int iSelfFX2; // 10 0x54
	int iSelfPart2; // 11 0x58
	int iFlyingFX; // 12 0x6C
	int iTargetFX; // 13 0x70
	int iTargetPart; // 14 0x74
	int iMoral; // 15 0x78
	int iNeedLevel; // 16 0x7C
	int iNeedSkill; // 17 0x80
	int iExhaustMSP; // 18 0x84
	int iExhaustHP; // 19 0x88
	short iExhaustSP; // 20 0x8C
	uint32_t itemGroup; // 21 0x90
	uint32_t dwNeedItem; // 22 0x94
	//int a1; // 23 0x98
	//int a2; // 24 0x9C
	//int a3; // 25 0xA0
	int iCastTime; // 26 0xA4
	int iReCastTime; // 27 0xA8
	float fIDK0; // 28 0xA8
	uint8 a4; // 29
	float fIDK1; // 30 0xAC
	int iPercentSuccess; // 31 0xB0
	uint32_t dwType1; // 32 0xA8
	uint32_t dwType2; // 33 0xAC
	int iRange; // 34 0xB0
	int iIDK2; // 35 0xB4
	uint32_t UNK1; // 36 0xBC
	int UNK2; // 37 0xC0
	uint32_t dwIconID; // 38 0xC4
	uint8_t UNK4; // 39 0xC8
	uint8_t UNK5; // 40 0xCC
};