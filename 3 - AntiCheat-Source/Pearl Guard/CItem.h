#pragma once
struct CItem;
struct CItemExt;
struct __IconItemSkillEx;

#include "stdafx.h"

struct CItem {
public:
	uint32_t Num;
	uint8_t extNum;
	std::string strName;
	std::string Description;
	uint32_t UNK0;
	uint8_t UNK1;
	uint32_t IconID;
	uint32_t dxtID;
	uint32_t UNK2;
	uint32_t UNK3;
	uint8_t Kind;
	uint8_t UNK4;
	uint8_t Slot;
	uint8_t UNK5;
	uint8_t Class;
	short Attack;
	short Delay;
	short Range;
	short Weight;
	short Duration;
	int repairPrice;
	int sellingPrice;
	short AC;
	uint8_t isCountable;
	uint32_t Effect1;
	uint32_t Effect2;
	uint8 ReqLevelMin;
	uint8 ReqLevelMax;
	uint8_t UNK6;
	uint8_t UNK7;
	uint8_t ReqStr;
	uint8_t ReqHp;
	uint8_t ReqDex;
	uint8_t ReqInt;
	uint8_t ReqMp;
	uint8_t SellingGroup;
	uint8_t ItemGrade;
	int UNK8;
	short iBaseBound;
	uint32_t UNK10;
	uint8_t UNK11;
};

struct CItemExt
{
public:
	uint32_t extensionID;
	std::string szHeader;
	uint32_t baseItemID;
	std::string Description;
	uint32_t EffectID;
	uint32_t dxtID;
	uint32_t iconID;
	uint8_t byMagicOrRare;
	short siDamage;
	short siAttackIntervalPercentage;
	short siHitRate;
	short siEvationRate;
	short siMaxDurability;
	short siPriceMultiply;
	short siDefense;
	short siDefenseRateDagger;
	short siDefenseRateSword;
	short siDefenseRateBlow;
	short siDefenseRateAxe;
	short siDefenseRateSpear;
	short siDefenseRateArrow;
	short siDefenseRateJamadar;
	uint8_t byDamageFire;
	uint8_t byDamageIce;
	uint8_t byDamageThuner;
	uint8_t byDamagePoison;
	uint8_t byStillHP;
	uint8_t byDamageMP;
	uint8_t byStillMP;
	uint8_t byReturnPhysicalDamage;
	uint8_t bySoulBind;
	short siBonusStr;
	short siBonusSta;
	short siBonusDex;
	short siBonusInt;
	short siBonusMagicAttak;
	short siBonusHP;
	short siBonusMSP;
	short siRegistFire;
	short siRegistIce;
	short siRegistElec;
	short siRegistMagic;
	short siRegistPoison;
	short siRegistCurse;
	short dwEffectID1;
	uint32_t dwEffectID2;
	uint32_t siNeedLevel;
	short siNeedRank;
	short siNeedTitle;
	short siNeedStrength;
	short siNeedStamina;
	short siNeedDexterity;
	short siNeedInteli;
	short siNeedMagicAttack;
	short UNK1;
	uint8_t UNK2;
	short iBound;
};
struct _MAP
{
	uint8 x;
	uint8 z;
	uint8 y;
	DWORD table;
};

struct __IconItemSkillEx {

	DWORD pUIIcon;
	std::string szIconFN;

	int16_t f32;
	int16_t iRentTime;
	int32_t iRentPeriod;
	int32_t iRentPrice;

	std::string unk;

	int32_t f72;

	int32_t iExpiration;

	int32_t f80;
	int32_t f84;
	int32_t f88;
	uint32_t f89;
	int32_t f96;
	int32_t f100;

	CItem* pItemBasic;
	CItemExt* pItemExt;
	int iCount;
	int iDurability;
};