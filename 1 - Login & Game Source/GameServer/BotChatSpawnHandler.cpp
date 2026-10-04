#include "StdAfx.h"

COMMAND_HANDLER(CUser::HandleBotDisconnected)
{
	if (!isGM())
		return false;

	if (vargs.size() > 0)
	{
		// send description
		g_pMain->SendHelpDescription(this, "Using Sample : +botkill");
		return true;
	}

	CBot* pBot = g_pMain->GetBotPtr(GetTargetID());
	if (pBot == nullptr)
	{
		g_pMain->SendHelpDescription(this, "Such a bots does not exist in the game.");
		return true;
	}

	pBot->UserInOut(INOUT_OUT);
	g_pMain->RemoveMapBotList(pBot->GetID(), pBot->GetName());
	return true;
}

COMMAND_HANDLER(CUser::HandleBotAfkSystem)
{
	if (!isGM())
		return false;

	if (vargs.size() < 2)
	{
		// send description
		g_pMain->SendHelpDescription(this, "Using Sample : +afkbotspawn count minute level");
		return true;
	}

	int sTime = 0, sLevel = 0, sCount = 0;
	sCount = atoi(vargs.front().c_str());
	vargs.pop_front();
	sTime = atoi(vargs.front().c_str());
	vargs.pop_front();
	sLevel = atoi(vargs.front().c_str());

	if (sCount > 100)
		sCount = 100;

	for (int i = 0; i < sCount; i++)
	{
		float BonX = myrand(1, 5) * 1.0f;
		float BonZ = myrand(1, 5) * 1.0f;
		g_pMain->SpawnEventAfkBotHandler(sTime, GetZoneID(), GetX() + BonX, GetY(), GetZ() + BonZ, sLevel);
	}
	return true;
}

COMMAND_HANDLER(CUser::HandleBotAllDisconnected)
{
	if (!isGM())
		return false;

	std::vector<uint32> HandleBotAllDisconnected;
	if (vargs.size() > 0)
	{
		// send description
		g_pMain->SendHelpDescription(this, "Using Sample : +allbotkill");
		return true;
	}

	g_pMain->m_MapBotList.m_lock.lock();
	auto m_sMapBotListArray = g_pMain->m_MapBotList.m_UserTypeMap;
	g_pMain->m_MapBotList.m_lock.unlock();

	foreach(itr, m_sMapBotListArray)
	{
		CBot* pBot = g_pMain->GetBotPtr(itr->first);
		if (pBot == nullptr)
			continue;

		HandleBotAllDisconnected.push_back(pBot->GetID());
	}

	if (HandleBotAllDisconnected.size() > 0)
	{
		foreach(itr, HandleBotAllDisconnected)
		{
			CBot* pBot = g_pMain->GetBotPtr(*itr);
			if (pBot == nullptr)
				continue;

			pBot->UserInOut(INOUT_OUT);
			g_pMain->RemoveMapBotList(pBot->GetID(), pBot->GetName());
		}
	}
	return true;
}

COMMAND_HANDLER(CUser::HandleBotSpawnMining)
{
	if (!isGM())
		return false;

	if (vargs.size() < 3)
	{
		// send description
		g_pMain->SendHelpDescription(this, "Using Sample : +miningbotspawn Count Minute MinLevel");
		return true;
	}

	int sTime = 0, sLevel = 0, sCount = 0;
	sCount = atoi(vargs.front().c_str());
	vargs.pop_front();
	sTime = atoi(vargs.front().c_str());
	vargs.pop_front();
	sLevel = atoi(vargs.front().c_str());

	if (sCount > 100)
		sCount = 100;

	for (int i = 0; i < sCount; i++)
	{
		float BonX = myrand(1, 5) * 1.0f;
		float BonZ = myrand(1, 5) * 1.0f;
		g_pMain->SpawnEventBotMining(sTime, GetZoneID(), GetX() + BonX, GetY(), GetZ() + BonZ, sLevel);
	}
	return true;
}

COMMAND_HANDLER(CUser::HandleBotSpawnMerchant)
{
	if (!isGM())
		return false;

	if (vargs.size() < 1)
	{
		// send description
		g_pMain->SendHelpDescription(this, "Using Sample : +merchantbotspawn sIndex");
		return true;
	}

	DateTime time;
	int sIndex = 0;
	sIndex = atoi(vargs.front().c_str());

	uint16 m_sSocketID = g_pMain->SpawnEventBotMerchant(3600, GetZoneID(), GetX(), GetY(), GetZ(), 0, MIN_LEVEL_ARDREAM);
	CBot* pBot = g_pMain->GetBotPtr(m_sSocketID);
	if (pBot == nullptr)
		return true;

	_BOT_MERCHANT_ITEM* pBotMerchantTable = g_pMain->m_ArtificialMerchantArray.GetData(sIndex);
	if (pBotMerchantTable == nullptr)
	{
		pBot->UserInOut(INOUT_OUT);
		g_pMain->RemoveMapBotList(pBot->GetID(), pBot->GetName());
		return true;
	}

	pBot->m_bPremiumMerchant = (bool)pBotMerchantTable->BotMerchantType;

	Packet result(WIZ_MERCHANT, uint8(MERCHANT_INSERT));
	result << uint16(1) << pBotMerchantTable->BotMerchantMessage << pBot->GetID()
		<< pBot->m_bPremiumMerchant;

	if (!pBotMerchantTable->BotMerchantMessage.empty())
		pBot->MerchantChat = string_format("%s(Location:%d,%d)", pBotMerchantTable->BotMerchantMessage.c_str(), pBot->GetSPosX() / 10, pBot->GetSPosZ() / 10);
	else
		pBot->MerchantChat.clear();

	uint16 totalMerchItems = 0;
	for (int t = 0; t < MAX_MERCH_ITEMS; t++)
	{
		pBot->m_arMerchantItems[t].bCount = pBotMerchantTable->m_MerchantItems[t].bCount;
		pBot->m_arMerchantItems[t].bOriginalSlot = pBotMerchantTable->m_MerchantItems[t].bOriginalSlot;
		pBot->m_arMerchantItems[t].IsSoldOut = pBotMerchantTable->m_MerchantItems[t].IsSoldOut;
		pBot->m_arMerchantItems[t].nNum = pBotMerchantTable->m_MerchantItems[t].nNum;
		pBot->m_arMerchantItems[t].nPrice = pBotMerchantTable->m_MerchantItems[t].nPrice;
		pBot->m_arMerchantItems[t].nSerialNum = pBotMerchantTable->m_MerchantItems[t].nSerialNum;
		pBot->m_arMerchantItems[t].sCount = pBotMerchantTable->m_MerchantItems[t].sCount;
		pBot->m_arMerchantItems[t].sDuration = pBotMerchantTable->m_MerchantItems[t].sDuration;

		if (pBot->m_arMerchantItems[t].nNum != 0 &&
			(pBot->m_arMerchantItems[t].bCount == 0
				|| pBot->m_arMerchantItems[t].bCount < pBotMerchantTable->m_MerchantItems[t].sCount))
			continue;

		result << pBot->m_arMerchantItems[t].nNum;

		if (pBot->m_arMerchantItems[t].nNum > 0)
			totalMerchItems++;
	}

	if (totalMerchItems == 0)
	{
		result.clear();
		result.Initialize(WIZ_MERCHANT);
		result << uint8(MERCHANT_CLOSE) << pBot->GetID();
		pBot->SendToRegion(&result);

		pBot->UserInOut(INOUT_OUT);
		g_pMain->RemoveMapBotList(pBot->GetID(), pBot->GetName());
		return true;
	}

	pBot->m_bMerchantState = MERCHANT_STATE_SELLING;
	pBot->SendToRegion(&result);
	MerchantClose();
	return true;
}

COMMAND_HANDLER(CUser::HandleBotSpawnFishing)
{
	if (!isGM())
		return false;

	if (vargs.size() < 3)
	{
		// send description
		g_pMain->SendHelpDescription(this, "Using Sample : +fishingbotspawn Count Minute MinLevel");
		return true;
	}

	int sTime = 0, sLevel = 0, sCount = 0;
	sCount = atoi(vargs.front().c_str());
	vargs.pop_front();
	sTime = atoi(vargs.front().c_str());
	vargs.pop_front();
	sLevel = atoi(vargs.front().c_str());

	if (sCount > 100)
		sCount = 100;

	for (int i = 0; i < sCount; i++)
	{
		float BonX = myrand(1, 5) * 1.0f;
		float BonZ = myrand(1, 5) * 1.0f;
		g_pMain->SpawnEventBotFishing(sTime, GetZoneID(), GetX() + BonX, GetY(), GetZ() + BonZ, sLevel);
	}
	return true;
}

COMMAND_HANDLER(CUser::HandleBotSpawnFarm)
{
	if (!isGM())
		return false;

	if (vargs.size() < 6)
	{
		// send description
		g_pMain->SendHelpDescription(this, "Using Sample : +farmbotspawn Count Minute MinLevel PartyLider Genie Class");
		return true;
	}

	int sTime = 0, sLevel = 0, sCount = 0, sPartyLider = 0, sGenie = 0, sClass = 0;
	sCount = atoi(vargs.front().c_str());
	vargs.pop_front();
	sTime = atoi(vargs.front().c_str());
	vargs.pop_front();
	sLevel = atoi(vargs.front().c_str());
	vargs.pop_front();
	sPartyLider = atoi(vargs.front().c_str());
	vargs.pop_front();
	sGenie = atoi(vargs.front().c_str());
	vargs.pop_front();
	sClass = atoi(vargs.front().c_str());

	if (sCount > 100)
		sCount = 100;

	for (int i = 0; i < sCount; i++)
	{
		float BonX = myrand(1, 5) * 1.0f;
		float BonZ = myrand(1, 5) * 1.0f;
		g_pMain->SpawnEventBotFarm(sTime, GetZoneID(), GetX() + BonX, GetY(), GetZ() + BonZ, sLevel, sPartyLider, sGenie, sClass);
	}
	return true;
}

COMMAND_HANDLER(CUser::HandleBotSpawnMerchantMove)
{
	if (!isGM())
		return false;

	if (vargs.size() < 3)
	{
		// send description
		g_pMain->SendHelpDescription(this, "Using Sample : +merchantmovebotspawn Count Minute MinLevel");
		return true;
	}

	int sTime = 0, sLevel = 0, sCount = 0;
	sCount = atoi(vargs.front().c_str());
	vargs.pop_front();
	sTime = atoi(vargs.front().c_str());
	vargs.pop_front();
	sLevel = atoi(vargs.front().c_str());

	if (sCount > 100)
		sCount = 100;

	for (int i = 0; i < sCount; i++)
	{
		float BonX = myrand(1, 15) * 1.0f;
		float BonZ = myrand(1, 15) * 1.0f;
		g_pMain->SpawnEventBotMoveProcess(sTime, GetZoneID(), GetX() + BonX, GetY(), GetZ() + BonZ, sLevel);
	}
	return true;
}

COMMAND_HANDLER(CUser::HandleBotSpawnPk)
{
	if (!isGM())
		return false;

	if (vargs.size() < 5)
	{
		// send description
		g_pMain->SendHelpDescription(this, "Using Sample : +pkbotspawn Count Minute MinLevel Nation Class");
		return true;
	}

	int sTime = 0, sLevel = 0, sCount = 0, sNation = 0, sClass = 0;
	sCount = atoi(vargs.front().c_str());
	vargs.pop_front();
	sTime = atoi(vargs.front().c_str());
	vargs.pop_front();
	sLevel = atoi(vargs.front().c_str());
	vargs.pop_front();
	sNation = atoi(vargs.front().c_str());
	vargs.pop_front();
	sClass = atoi(vargs.front().c_str());

	if (sCount > 100)
		sCount = 100;

	for (int i = 0; i < sCount; i++)
	{
		float BonX = myrand(1, 15) * 1.0f;
		float BonZ = myrand(1, 15) * 1.0f;
		g_pMain->SpawnEventBotPk(sTime, GetZoneID(), GetX() + BonX, GetY(), GetZ() + BonZ, sLevel, sNation, sClass, this);
	}
	return true;
}

COMMAND_HANDLER(CUser::HandleMerchantBotCommand)
{
	if (!isGM())
		return true;

	if (vargs.size() < 3) {
		g_pMain->SendHelpDescription(this, "Using Sample : /merchantbot Count Time AreaType.");
		return true;
	}

	uint16 botCount = 0;
	if (!vargs.empty()) { botCount = atoi(vargs.front().c_str()); vargs.pop_front(); }

	if (!botCount || botCount > 100) {
		g_pMain->SendHelpDescription(this, "The maximum number of robots you can buy cannot exceed 100.");
		return true;
	}

	uint32 botTime = 0;
	if (!vargs.empty()) { botTime = atoi(vargs.front().c_str()); vargs.pop_front(); }

	botTime = botTime * SECOND;

	uint32 botArea = 0;
	if (!vargs.empty()) { botArea = atoi(vargs.front().c_str()); vargs.pop_front(); }
	if (!botArea) {
		g_pMain->SendHelpDescription(this, "Bot Area Error.");
		return true;
	}

	BotMerchantAdd(botCount, botTime, botArea);
	return true;
}

void CUser::BotMerchantAdd(uint16 count, uint32 bTime, uint16 type)
{
	struct _list {
		uint32 index;
		_MERCHANT_BOT_INFO pInfo;
		_list(uint32 index, _MERCHANT_BOT_INFO pInfo) {
			this->index = index;
			this->pInfo = pInfo;
		}
	};
	std::vector<_list> mList;

	uint16 counter = 0; uint32 index = 0;
	g_pMain->pBotInfo.mCoordinate.m_lock.lock();
	auto m_bMerchantCoordinateStatus = g_pMain->pBotInfo.mCoordinate.m_UserTypeMap;
	g_pMain->pBotInfo.mCoordinate.m_lock.unlock();

	foreach(itr, m_bMerchantCoordinateStatus)
	{
		auto* pCoordinat = itr->second;
		if (!pCoordinat
			|| pCoordinat->used
			|| pCoordinat->type != type)
			continue;

		if (counter >= count)
			break;

		counter++;
		index = itr->first;
		pCoordinat->used = true;
		mList.push_back(_list(itr->first, *pCoordinat));
	}

	if (mList.empty())
	{
		if (index) {
			auto* pCoord = g_pMain->pBotInfo.mCoordinate.GetData(index);
			if (pCoord)
				pCoord->used = false;
		}
		g_pMain->SendHelpDescription(this, "List is empty!");
		return;
	}

	int atime = 1; ULONGLONG btime = UNIXTIME2 + (ULONGLONG)(atime * bTime);
	g_pMain->m_addbotlistLock.lock();
	for (auto& itr : mList) {
		g_pMain->m_addbotlist.insert(std::make_pair(itr.index, _botadd(itr.index, btime, (uint8)type)));
		btime += (ULONGLONG)(atime * bTime);
	}
	g_pMain->m_addbotlistLock.unlock();
}

void CBot::SendDeathNotice(Unit* pKiller, DeathNoticeType noticeType, bool isToZone /*= true*/)
{
	if (pKiller == nullptr)
		return;

	Packet result(WIZ_CHAT, uint8(DEATH_NOTICE));

	result.SByte();
	result << GetNation()
		<< uint8(noticeType)
		<< pKiller->GetID() // session ID?
		<< pKiller->GetName()
		<< GetID() // session ID?
		<< GetName()
		<< uint16(GetX()) << uint16(GetZ());

	if (isToZone)
		SendToZone(&result, RANGE_20M);
	else
	{
		SendToRegion(&result);
		if (pKiller->isPlayer())
			TO_USER(pKiller)->Send(&result);
	}
}

uint16 CGameServerDlg::SpawnEventAfkBotHandler(int Minute, uint8 byZone, float fX, float fY, float fZ, uint8 minlevel)
{
	m_ArtificialIntelligenceArray.m_lock.lock();
	auto m_sArtificialIntelligenceArray = m_ArtificialIntelligenceArray.m_UserTypeMap;
	m_ArtificialIntelligenceArray.m_lock.unlock();

	foreach(itr, m_sArtificialIntelligenceArray)
	{
		_BOT_DATA* pBotInfo = itr->second;
		if (pBotInfo == nullptr)
			continue;

		if (byZone <= ZONE_ELMORAD && byZone != pBotInfo->m_bNation
			|| (byZone >= ZONE_KARUS_ESLANT
				&& byZone <= ZONE_ELMORAD_ESLANT
				&& byZone != (pBotInfo->m_bNation + 10)))
			continue;

		if (pBotInfo->m_bLevel < minlevel)
			continue;

		// Check if bot already exist
		CBot* pBotCheck = GetBotPtr(pBotInfo->m_sSid);
		if (pBotCheck != nullptr)
			continue;

		CBot* pBot = new CBot();

		pBot->m_strUserID = pBotInfo->m_strUserID;
		pBot->m_bNation = pBotInfo->m_bNation;
		pBot->m_bRace = pBotInfo->m_bRace;
		pBot->m_sClass = pBotInfo->m_sClass;
		pBot->m_nHair = pBotInfo->m_nHair;
		pBot->m_bLevel = pBotInfo->m_bLevel;
		pBot->m_bFace = pBotInfo->m_bFace;
		pBot->m_bKnights = pBotInfo->m_bKnights;
		pBot->m_bFame = pBotInfo->m_bFame;

		memcpy(pBot->m_sItemArray, pBotInfo->m_sItemArray, sizeof(pBot->m_sItemArray));
		memcpy(pBot->m_bstrSkill, pBotInfo->m_bstrSkill, sizeof(pBot->m_bstrSkill));
		memcpy(pBot->m_bStats, pBotInfo->m_bStats, sizeof(pBot->m_bStats));

		pBot->m_sSid = pBotInfo->m_sSid;
		pBot->m_sAchieveCoverTitle = pBotInfo->m_sAchieveCoverTitle;
		pBot->m_reblvl = pBotInfo->m_reblvl;
		pBot->m_iGold = pBotInfo->m_iGold;
		pBot->m_sPoints = pBotInfo->m_sPoints;
		pBot->m_iLoyalty = pBotInfo->m_iLoyalty;
		pBot->m_iLoyaltyMonthly = pBotInfo->m_iLoyaltyMonthly;

		int Random = myrand(0, 10000);
		pBot->m_bMerchantState = MERCHANT_STATE_NONE;
		pBot->LastWarpTime = UNIXTIME + (Minute * 60);
		pBot->m_pMap = GetZoneByID(byZone);
		pBot->m_bZone = byZone;
		pBot->m_sTargetID = int16(-1);
		pBot->m_TargetChanged = false;
		pBot->m_bBlockPrivateChat = false;
		pBot->SetBotAbility();
		pBot->m_BotState = BOT_AFK;
		pBot->SetPosition(fX, fY, fZ);
		pBot->SetRegion(pBot->GetNewRegionX(), pBot->GetNewRegionZ());
		AddMapBotList(pBot);
		pBot->UserInOut(INOUT_IN);
		pBot->StateChangeServerDirect(1, Random > 5000 ? USER_STANDING : USER_SITDOWN);
		return pBot->GetID();
	}
	return 0;
}

uint16 CGameServerDlg::SpawnEventBotMining(int Minute, uint8 byZone, float fX, float fY, float fZ, uint8 minlevel)
{
	m_ArtificialIntelligenceArray.m_lock.lock();
	auto m_sArtificialIntelligenceArray = m_ArtificialIntelligenceArray.m_UserTypeMap;
	m_ArtificialIntelligenceArray.m_lock.unlock();

	foreach(itr, m_sArtificialIntelligenceArray)
	{
		_BOT_DATA* pBotInfo = itr->second;
		if (pBotInfo == nullptr)
			continue;

		if (byZone <= ZONE_ELMORAD && byZone != pBotInfo->m_bNation
			|| (byZone >= ZONE_KARUS_ESLANT
				&& byZone <= ZONE_ELMORAD_ESLANT
				&& byZone != (pBotInfo->m_bNation + 10)))
			continue;

		if (pBotInfo->m_bLevel < minlevel)
			continue;

		// Check if bot already exist
		CBot* pBotCheck = GetBotPtr(pBotInfo->m_sSid);
		if (pBotCheck != nullptr)
			continue;

		CBot* pBot = new CBot();

		pBot->m_strUserID = pBotInfo->m_strUserID;
		pBot->m_bNation = pBotInfo->m_bNation;
		pBot->m_bRace = pBotInfo->m_bRace;
		pBot->m_sClass = pBotInfo->m_sClass;
		pBot->m_nHair = pBotInfo->m_nHair;
		pBot->m_bLevel = pBotInfo->m_bLevel;
		pBot->m_bFace = pBotInfo->m_bFace;
		pBot->m_bKnights = pBotInfo->m_bKnights;
		pBot->m_bFame = pBotInfo->m_bFame;

		memcpy(pBot->m_sItemArray, pBotInfo->m_sItemArray, sizeof(pBot->m_sItemArray));
		memcpy(pBot->m_bstrSkill, pBotInfo->m_bstrSkill, sizeof(pBot->m_bstrSkill));
		memcpy(pBot->m_bStats, pBotInfo->m_bStats, sizeof(pBot->m_bStats));

		pBot->m_sSid = pBotInfo->m_sSid;
		pBot->m_sAchieveCoverTitle = pBotInfo->m_sAchieveCoverTitle;
		pBot->m_reblvl = pBotInfo->m_reblvl;
		pBot->m_iGold = pBotInfo->m_iGold;
		pBot->m_sPoints = pBotInfo->m_sPoints;
		pBot->m_iLoyalty = pBotInfo->m_iLoyalty;
		pBot->m_iLoyaltyMonthly = pBotInfo->m_iLoyaltyMonthly;

		pBot->m_bMerchantState = MERCHANT_STATE_NONE;
		pBot->LastWarpTime = UNIXTIME + (Minute * 60);
		pBot->m_pMap = GetZoneByID(byZone);
		pBot->m_bZone = byZone;

		_ITEM_DATA* pItem = pBot->GetItem(RIGHTHAND);
		if (pItem == nullptr)
		{
			_ITEM_TABLE pTable = GetItemPtr(MATTOCK);
			if (pTable.isnull())
				continue;

			_ITEM_DATA* pTItem = pBot->GetItem(LEFTHAND);
			if (pTItem != nullptr)
				memset(pTItem, 0x00, sizeof(_ITEM_DATA));

			memset(pItem, 0x00, sizeof(_ITEM_DATA));
			pItem->nNum = pTable.m_iNum;
			pItem->nSerialNum = GenerateItemSerial();
			pItem->sCount = 1;
			pItem->sDuration = pTable.m_sDuration;
		}
		else
		{
			_ITEM_TABLE pTable = GetItemPtr(MATTOCK);
			if (pTable.isnull())
				continue;

			_ITEM_DATA* pTItem = pBot->GetItem(LEFTHAND);
			if (pTItem != nullptr)
				memset(pTItem, 0x00, sizeof(_ITEM_DATA));

			memset(pItem, 0x00, sizeof(_ITEM_DATA));
			pItem->nNum = pTable.m_iNum;
			pItem->nSerialNum = GenerateItemSerial();
			pItem->sCount = 1;
			pItem->sDuration = pTable.m_sDuration;
		}

		pBot->m_sTargetID = int16(-1);
		pBot->m_TargetChanged = false;
		pBot->m_bBlockPrivateChat = false;
		pBot->m_bResHpType = USER_MINING;
		pBot->SetBotAbility();
		pBot->m_BotState = BOT_MINING;
		pBot->StateChangeServerDirect(1, USER_MINING);
		pBot->SetPosition(fX, fY, fZ);
		pBot->SetRegion(pBot->GetNewRegionX(), pBot->GetNewRegionZ());
		AddMapBotList(pBot);
		pBot->UserInOut(INOUT_IN);
		return pBot->GetID();
	}
	return true;
}

uint16 CGameServerDlg::SpawnEventBotMerchant(int Minute, uint8 byZone, float fX, float fY, float fZ, int16 nDir, uint8 minlevel)
{
	m_ArtificialIntelligenceArray.m_lock.lock();
	auto m_sArtificialIntelligenceArray = m_ArtificialIntelligenceArray.m_UserTypeMap;
	m_ArtificialIntelligenceArray.m_lock.unlock();

	foreach(itr, m_sArtificialIntelligenceArray)
	{
		_BOT_DATA* pBotInfo = itr->second;
		if (pBotInfo == nullptr)
			continue;

		if (byZone <= ZONE_ELMORAD && byZone != pBotInfo->m_bNation
			|| (byZone >= ZONE_KARUS_ESLANT
				&& byZone <= ZONE_ELMORAD_ESLANT
				&& byZone != (pBotInfo->m_bNation + 10)))
			continue;

		if (pBotInfo->m_bLevel < minlevel)
			continue;

		// Check if bot already exist
		CBot* pBotCheck = GetBotPtr(pBotInfo->m_sSid);
		if (pBotCheck != nullptr)
			continue;

		CBot* pBot = new CBot();

		pBot->m_strUserID = pBotInfo->m_strUserID;
		pBot->m_bNation = pBotInfo->m_bNation;
		pBot->m_bRace = pBotInfo->m_bRace;
		pBot->m_sClass = pBotInfo->m_sClass;
		pBot->m_nHair = pBotInfo->m_nHair;
		pBot->m_bLevel = pBotInfo->m_bLevel;
		pBot->m_bFace = pBotInfo->m_bFace;
		pBot->m_bKnights = pBotInfo->m_bKnights;
		pBot->m_bFame = pBotInfo->m_bFame;

		memcpy(pBot->m_sItemArray, pBotInfo->m_sItemArray, sizeof(pBot->m_sItemArray));
		memcpy(pBot->m_bstrSkill, pBotInfo->m_bstrSkill, sizeof(pBot->m_bstrSkill));
		memcpy(pBot->m_bStats, pBotInfo->m_bStats, sizeof(pBot->m_bStats));

		pBot->m_sSid = pBotInfo->m_sSid;
		pBot->m_sAchieveCoverTitle = pBotInfo->m_sAchieveCoverTitle;
		pBot->m_reblvl = pBotInfo->m_reblvl;
		pBot->m_iGold = pBotInfo->m_iGold;
		pBot->m_sPoints = pBotInfo->m_sPoints;
		pBot->m_iLoyalty = pBotInfo->m_iLoyalty;
		pBot->m_iLoyaltyMonthly = pBotInfo->m_iLoyaltyMonthly;

		int Random = myrand(0, 10000);
		pBot->m_bMerchantState = MERCHANT_STATE_NONE;
		pBot->LastWarpTime = UNIXTIME + (Minute * 60);
		pBot->m_pMap = GetZoneByID(byZone);
		pBot->m_bZone = byZone;
		pBot->m_sDirection = nDir;
		pBot->m_sTargetID = int16(-1);
		pBot->m_TargetChanged = false;
		pBot->m_bBlockPrivateChat = false;
		pBot->SetBotAbility();
		pBot->m_BotState = BOT_MERCHANT;
		pBot->SetPosition(fX, fY, fZ);
		pBot->SetRegion(pBot->GetNewRegionX(), pBot->GetNewRegionZ());
		AddMapBotList(pBot);
		pBot->UserInOut(INOUT_IN);
		pBot->StateChangeServerDirect(1, Random > 5000 ? USER_STANDING : USER_SITDOWN);
		return pBot->GetID();
	}
	return 0;
}

uint16 CGameServerDlg::SpawnEventBotFishing(int Minute, uint8 byZone, float fX, float fY, float fZ, uint8 minlevel)
{
	m_ArtificialIntelligenceArray.m_lock.lock();
	auto m_sArtificialIntelligenceArray = m_ArtificialIntelligenceArray.m_UserTypeMap;
	m_ArtificialIntelligenceArray.m_lock.unlock();

	foreach(itr, m_sArtificialIntelligenceArray)
	{
		_BOT_DATA* pBotInfo = itr->second;
		if (pBotInfo == nullptr)
			continue;

		if (byZone <= ZONE_ELMORAD && byZone != pBotInfo->m_bNation
			|| (byZone >= ZONE_KARUS_ESLANT
				&& byZone <= ZONE_ELMORAD_ESLANT
				&& byZone != (pBotInfo->m_bNation + 10)))
			continue;

		if (pBotInfo->m_bLevel < minlevel)
			continue;

		// Check if bot already exist
		CBot* pBotCheck = GetBotPtr(pBotInfo->m_sSid);
		if (pBotCheck != nullptr)
			continue;

		CBot* pBot = new CBot();

		pBot->m_strUserID = pBotInfo->m_strUserID;
		pBot->m_bNation = pBotInfo->m_bNation;
		pBot->m_bRace = pBotInfo->m_bRace;
		pBot->m_sClass = pBotInfo->m_sClass;
		pBot->m_nHair = pBotInfo->m_nHair;
		pBot->m_bLevel = pBotInfo->m_bLevel;
		pBot->m_bFace = pBotInfo->m_bFace;
		pBot->m_bKnights = pBotInfo->m_bKnights;
		pBot->m_bFame = pBotInfo->m_bFame;

		memcpy(pBot->m_sItemArray, pBotInfo->m_sItemArray, sizeof(pBot->m_sItemArray));
		memcpy(pBot->m_bstrSkill, pBotInfo->m_bstrSkill, sizeof(pBot->m_bstrSkill));
		memcpy(pBot->m_bStats, pBotInfo->m_bStats, sizeof(pBot->m_bStats));

		pBot->m_sSid = pBotInfo->m_sSid;
		pBot->m_sAchieveCoverTitle = pBotInfo->m_sAchieveCoverTitle;
		pBot->m_reblvl = pBotInfo->m_reblvl;
		pBot->m_iGold = pBotInfo->m_iGold;
		pBot->m_sPoints = pBotInfo->m_sPoints;
		pBot->m_iLoyalty = pBotInfo->m_iLoyalty;
		pBot->m_iLoyaltyMonthly = pBotInfo->m_iLoyaltyMonthly;

		pBot->m_bMerchantState = MERCHANT_STATE_NONE;
		pBot->LastWarpTime = UNIXTIME + (Minute * 60);
		pBot->m_pMap = GetZoneByID(byZone);
		pBot->m_bZone = byZone;

		_ITEM_DATA* pItem = pBot->GetItem(RIGHTHAND);
		if (pItem == nullptr)
		{
			_ITEM_TABLE pTable = GetItemPtr(FISHING);
			if (pTable.isnull())
				continue;

			_ITEM_DATA* pTItem = pBot->GetItem(LEFTHAND);
			if (pTItem != nullptr)
				memset(pTItem, 0x00, sizeof(_ITEM_DATA));

			memset(pItem, 0x00, sizeof(_ITEM_DATA));
			pItem->nNum = pTable.m_iNum;
			pItem->nSerialNum = GenerateItemSerial();
			pItem->sCount = 1;
			pItem->sDuration = pTable.m_sDuration;
		}
		else
		{
			_ITEM_TABLE pTable = GetItemPtr(FISHING);
			if (pTable.isnull())
				continue;

			_ITEM_DATA* pTItem = pBot->GetItem(LEFTHAND);
			if (pTItem != nullptr)
				memset(pTItem, 0x00, sizeof(_ITEM_DATA));

			memset(pItem, 0x00, sizeof(_ITEM_DATA));
			pItem->nNum = pTable.m_iNum;
			pItem->nSerialNum = GenerateItemSerial();
			pItem->sCount = 1;
			pItem->sDuration = pTable.m_sDuration;
		}

		pBot->m_sTargetID = int16(-1);
		pBot->m_TargetChanged = false;
		pBot->m_bBlockPrivateChat = false;
		pBot->m_bResHpType = USER_FLASHING;
		pBot->SetBotAbility();
		pBot->m_BotState = BOT_FISHING;
		pBot->StateChangeServerDirect(1, USER_FLASHING);
		pBot->SetPosition(fX, fY, fZ);
		pBot->SetRegion(pBot->GetNewRegionX(), pBot->GetNewRegionZ());
		AddMapBotList(pBot);
		pBot->UserInOut(INOUT_IN);

		return pBot->GetID();
	}
	return true;
}

uint16 CGameServerDlg::SpawnEventBotFarm(int Minute, uint8 byZone, float fX, float fY, float fZ, uint8 minlevel, uint8 sPartyLider, uint8 sGenie, uint8 sClass)
{
	m_ArtificialIntelligenceArray.m_lock.lock();
	auto m_sArtificialIntelligenceArray = m_ArtificialIntelligenceArray.m_UserTypeMap;
	m_ArtificialIntelligenceArray.m_lock.unlock();

	foreach(itr, m_sArtificialIntelligenceArray)
	{
		_BOT_DATA* pBotInfo = itr->second;
		if (pBotInfo == nullptr)
			continue;

		if (byZone <= ZONE_ELMORAD && byZone != pBotInfo->m_bNation
			|| (byZone >= ZONE_KARUS_ESLANT
				&& byZone <= ZONE_ELMORAD_ESLANT
				&& byZone != (pBotInfo->m_bNation + 10)))
			continue;

		if (pBotInfo->m_bLevel < minlevel)
			continue;

		_ITEM_DATA* pItem = pBotInfo->GetItem(RIGHTHAND);
		if (pItem == nullptr)
			continue;

		_ITEM_TABLE pTable = GetItemPtr(pItem->nNum);
		if (pTable.isnull())
			continue;

		if (pTable.isShield()
			|| pTable.isPickaxe()
			|| pTable.isFishing())
			continue;

		if (sClass > 0 && sClass < 15)
		{
			if (sClass == 1 && !pBotInfo->isWarrior())
				continue;

			if (sClass == 2 && !pBotInfo->isRogue())
				continue;

			if (sClass == 3 && !pBotInfo->isMage())
				continue;

			if (sClass == 4 && !pBotInfo->isPriest())
				continue;

			if (sClass == 14 && !pBotInfo->isPortuKurian())
				continue;

			if (sClass > 4 && sClass < 14)
				continue;
		}
		else
			continue;

		// Check if bot already exist
		CBot* pBotCheck = GetBotPtr(pBotInfo->m_sSid);
		if (pBotCheck != nullptr)
			continue;

		CBot* pBot = new CBot();

		pBot->m_strUserID = pBotInfo->m_strUserID;
		pBot->m_bNation = pBotInfo->m_bNation;
		pBot->m_bRace = pBotInfo->m_bRace;
		pBot->m_sClass = pBotInfo->m_sClass;
		pBot->m_nHair = pBotInfo->m_nHair;
		pBot->m_bLevel = pBotInfo->m_bLevel;
		pBot->m_bFace = pBotInfo->m_bFace;
		pBot->m_bKnights = pBotInfo->m_bKnights;
		pBot->m_bFame = pBotInfo->m_bFame;		

		memcpy(pBot->m_sItemArray, pBotInfo->m_sItemArray, sizeof(pBot->m_sItemArray));
		memcpy(pBot->m_bstrSkill, pBotInfo->m_bstrSkill, sizeof(pBot->m_bstrSkill));
		memcpy(pBot->m_bStats, pBotInfo->m_bStats, sizeof(pBot->m_bStats));

		pBot->m_sSid = pBotInfo->m_sSid;
		pBot->m_sAchieveCoverTitle = pBotInfo->m_sAchieveCoverTitle;
		pBot->m_reblvl = pBotInfo->m_reblvl;
		pBot->m_iGold = pBotInfo->m_iGold;
		pBot->m_sPoints = pBotInfo->m_sPoints;
		pBot->m_iLoyalty = pBotInfo->m_iLoyalty;
		pBot->m_iLoyaltyMonthly = pBotInfo->m_iLoyaltyMonthly;

		pBot->m_PlayerKillingLoyaltyDaily = 0;
		pBot->m_PlayerKillingLoyaltyPremiumBonus = 0;

		pBot->m_bMerchantState = MERCHANT_STATE_NONE;
		pBot->LastWarpTime = UNIXTIME + (Minute * MINUTE);
		pBot->m_pMap = GetZoneByID(byZone);
		pBot->m_bZone = byZone;
		pBot->m_sTargetID = int16(-1);
		pBot->m_TargetChanged = false;
		pBot->m_bBlockPrivateChat = false;
		pBot->SetBotAbility();
		pBot->m_BotState = BOT_FARMER;
		pBot->m_bPartyLeader = sPartyLider == 1 ? true : false;
		pBot->m_bGenieStatus = sGenie == 1 ? true : false;
		pBot->SetPosition(fX, fY, fZ);
		pBot->m_oldx = pBot->m_curx;
		pBot->m_oldy = pBot->m_cury;
		pBot->m_oldz = pBot->m_curz;
		pBot->SetRegion(pBot->GetNewRegionX(), pBot->GetNewRegionZ());
		pBot->StateChangeServerDirect(1, USER_STANDING);

		if (byZone == ZONE_RONARK_LAND
			|| byZone == ZONE_RONARK_LAND_BASE
			|| byZone == ZONE_ARDREAM)
		{
			C3DMap* pMap = g_pMain->GetZoneByID(byZone);
			if (pMap == nullptr)
				continue;

			TRACE("%s In the Game Insert Player Ranking ZoneID %d\n", pBot->GetName().c_str(), pBot->GetZoneID());
			pBot->AddBotRank(pMap);
		}

		AddMapBotList(pBot);
		pBot->UserInOut(INOUT_IN);
		return pBot->GetID();
	}
	return true;
}

uint16 CGameServerDlg::SpawnEventBotMoveProcess(int Minute, uint8 byZone, float fX, float fY, float fZ, uint8 minlevel)
{
	m_ArtificialIntelligenceArray.m_lock.lock();
	auto m_sArtificialIntelligenceArray = m_ArtificialIntelligenceArray.m_UserTypeMap;
	m_ArtificialIntelligenceArray.m_lock.unlock();

	foreach(itr, m_sArtificialIntelligenceArray)
	{
		_BOT_DATA* pBotInfo = itr->second;
		if (pBotInfo == nullptr)
			continue;

		if (byZone <= ZONE_ELMORAD && byZone != pBotInfo->m_bNation
			|| (byZone >= ZONE_KARUS_ESLANT
				&& byZone <= ZONE_ELMORAD_ESLANT
				&& byZone != (pBotInfo->m_bNation + 10)))
			continue;

		if (pBotInfo->m_bLevel < minlevel)
			continue;

		// Check if bot already exist
		CBot* pBotCheck = GetBotPtr(pBotInfo->m_sSid);
		if (pBotCheck != nullptr)
			continue;

		CBot* pBot = new CBot();

		pBot->m_strUserID = pBotInfo->m_strUserID;
		pBot->m_bNation = pBotInfo->m_bNation;
		pBot->m_bRace = pBotInfo->m_bRace;
		pBot->m_sClass = pBotInfo->m_sClass;
		pBot->m_nHair = pBotInfo->m_nHair;
		pBot->m_bLevel = pBotInfo->m_bLevel;
		pBot->m_bFace = pBotInfo->m_bFace;
		pBot->m_bKnights = pBotInfo->m_bKnights;
		pBot->m_bFame = pBotInfo->m_bFame;

		memcpy(pBot->m_sItemArray, pBotInfo->m_sItemArray, sizeof(pBot->m_sItemArray));
		memcpy(pBot->m_bstrSkill, pBotInfo->m_bstrSkill, sizeof(pBot->m_bstrSkill));
		memcpy(pBot->m_bStats, pBotInfo->m_bStats, sizeof(pBot->m_bStats));

		pBot->m_sSid = pBotInfo->m_sSid;
		pBot->m_sAchieveCoverTitle = pBotInfo->m_sAchieveCoverTitle;
		pBot->m_reblvl = pBotInfo->m_reblvl;
		pBot->m_iGold = pBotInfo->m_iGold;
		pBot->m_sPoints = pBotInfo->m_sPoints;
		pBot->m_iLoyalty = pBotInfo->m_iLoyalty;
		pBot->m_iLoyaltyMonthly = pBotInfo->m_iLoyaltyMonthly;

		pBot->m_bMerchantState = MERCHANT_STATE_NONE;
		pBot->LastWarpTime = UNIXTIME + (Minute * MINUTE);
		pBot->m_pMap = GetZoneByID(byZone);
		pBot->m_bZone = byZone;
		pBot->m_sTargetID = int16(-1);
		pBot->m_TargetChanged = false;
		pBot->m_bBlockPrivateChat = false;
		pBot->SetBotAbility();
		pBot->m_BotState = BOT_MERCHANT_MOVE;
		pBot->SetPosition(fX, fY, fZ);
		pBot->SetRegion(pBot->GetNewRegionX(), pBot->GetNewRegionZ());
		pBot->StateChangeServerDirect(1, USER_STANDING);
		AddMapBotList(pBot);
		pBot->UserInOut(INOUT_IN);
		return pBot->GetID();
	}
	return true;
}

uint16 CGameServerDlg::SpawnEventBotPk(int Minute, uint8 byZone, float fX, float fY, float fZ, uint8 minlevel, uint8 sNation, uint8 sClass, CUser* pUser, bool startup)
{
	if (!startup && pUser == nullptr)
		return true;

	m_ArtificialIntelligenceArray.m_lock.lock();
	auto m_sArtificialIntelligenceArray = m_ArtificialIntelligenceArray.m_UserTypeMap;
	m_ArtificialIntelligenceArray.m_lock.unlock();

	foreach(itr, m_sArtificialIntelligenceArray)
	{
		_BOT_DATA* pBotInfo = itr->second;
		if (pBotInfo == nullptr)
			continue;

		if (!startup)
		{
			switch (byZone)
			{
			case ZONE_RONARK_LAND:
				if (pBotInfo->m_bLevel < pUser->GetMap()->GetMinLevelReq())
					continue;
				break;
			case ZONE_ARDREAM:
				if (pBotInfo->m_bLevel < pUser->GetMap()->GetMinLevelReq()
					|| pBotInfo->m_bLevel > pUser->GetMap()->GetMaxLevelReq())
					continue;
				break;
			case ZONE_RONARK_LAND_BASE:
				if (pBotInfo->m_bLevel < pUser->GetMap()->GetMinLevelReq()
					|| pBotInfo->m_bLevel > pUser->GetMap()->GetMaxLevelReq())
					continue;
				break;
			case ZONE_DELOS:
			case ZONE_BIFROST:
			case ZONE_SPBATTLE1:
			case ZONE_SPBATTLE2:
			case ZONE_SPBATTLE3:
			case ZONE_SPBATTLE4:
			case ZONE_SPBATTLE5:
			case ZONE_SPBATTLE6:
			case ZONE_SPBATTLE7:
			case ZONE_SPBATTLE8:
			case ZONE_SPBATTLE9:
			case ZONE_SPBATTLE10:
			case ZONE_SPBATTLE11:
			case ZONE_SPBATTLE12:
				if (pBotInfo->m_bLevel < pUser->GetMap()->GetMinLevelReq()
					|| pBotInfo->m_bLevel > pUser->GetMap()->GetMaxLevelReq())
					continue;
				break;
			default:
				continue;
				break;
			}
		}

		if (pBotInfo->m_bLevel < minlevel
			|| pBotInfo->m_bNation != sNation)
			continue;

		_ITEM_DATA* pItem = pBotInfo->GetItem(RIGHTHAND);
		if (pItem == nullptr)
			continue;

		_ITEM_TABLE pTable = GetItemPtr(pItem->nNum);
		if (pTable.isnull())
			continue;

		if (pTable.isShield()
			|| pTable.isPickaxe()
			|| pTable.isFishing())
			continue;

		if (sClass > 0 && sClass < 15)
		{
			if (sClass == 1 && !pBotInfo->isWarrior())
				continue;

			if (sClass == 2 && !pBotInfo->isRogue())
				continue;

			if (sClass == 3 && !pBotInfo->isMage())
				continue;

			if (sClass == 4 && !pBotInfo->isPriest())
				continue;

			if (sClass == 14 && !pBotInfo->isPortuKurian())
				continue;

			if (sClass > 4 && sClass < 14)
				continue;
		}
		else
			continue;

		// Check if bot already exist
		CBot* pBotCheck = GetBotPtr(pBotInfo->m_sSid);
		if (pBotCheck != nullptr)
			continue;

		CBot* pBot = new CBot();

		pBot->m_strUserID = pBotInfo->m_strUserID;
		pBot->m_bNation = pBotInfo->m_bNation;
		pBot->m_bRace = pBotInfo->m_bRace;
		pBot->m_sClass = pBotInfo->m_sClass;
		pBot->m_nHair = pBotInfo->m_nHair;
		pBot->m_bLevel = pBotInfo->m_bLevel;
		pBot->m_bFace = pBotInfo->m_bFace;
		pBot->m_bKnights = pBotInfo->m_bKnights;
		pBot->m_bFame = pBotInfo->m_bFame;

		memcpy(pBot->m_sItemArray, pBotInfo->m_sItemArray, sizeof(pBot->m_sItemArray));
		memcpy(pBot->m_bstrSkill, pBotInfo->m_bstrSkill, sizeof(pBot->m_bstrSkill));
		memcpy(pBot->m_bStats, pBotInfo->m_bStats, sizeof(pBot->m_bStats));

		pBot->m_sSid = pBotInfo->m_sSid;
		pBot->m_sAchieveCoverTitle = pBotInfo->m_sAchieveCoverTitle;
		pBot->m_reblvl = pBotInfo->m_reblvl;
		pBot->m_iGold = pBotInfo->m_iGold;
		pBot->m_sPoints = pBotInfo->m_sPoints;
		pBot->m_iLoyalty = pBotInfo->m_iLoyalty;
		pBot->m_iLoyaltyMonthly = pBotInfo->m_iLoyaltyMonthly;

		pBot->m_PlayerKillingLoyaltyDaily = 0;
		pBot->m_PlayerKillingLoyaltyPremiumBonus = 0;

		pBot->m_bMerchantState = MERCHANT_STATE_NONE;
		pBot->LastWarpTime = UNIXTIME + (Minute * MINUTE);
		pBot->m_pMap = GetZoneByID(byZone);
		pBot->m_bZone = byZone;
		pBot->m_sTargetID = int16(-1);
		pBot->m_TargetChanged = false;
		pBot->m_bBlockPrivateChat = false;
		pBot->m_BotState = BOT_MOVE;
		pBot->m_bPartyLeader = false;
		pBot->m_bGenieStatus = false;
		pBot->m_bKnightsRank = pBotInfo->m_bKareli; //x6
		pBot->m_bPersonalRank = pBotInfo->m_bKaresiz; //x6
		short sx, sz;
		pBot->GetStartPosition(sx, sz);
		fX = sx;
		fZ = sz;

		pBot->isReset(false);
		pBot->SetBotAbility();
		pBot->SetPosition(fX, fY, fZ);
		pBot->SetRegion(pBot->GetNewRegionX(), pBot->GetNewRegionZ());
		pBot->StateChangeServerDirect(1, USER_STANDING);

		//if (pBot->isInPKZone())
		if (byZone == ZONE_RONARK_LAND ||
			byZone == ZONE_RONARK_LAND_BASE ||
			byZone == ZONE_ARDREAM ||
			byZone >= ZONE_SPBATTLE1 && byZone <= ZONE_SPBATTLE12)
		{
			C3DMap* pMap = g_pMain->GetZoneByID(byZone);
			if (pMap == nullptr)
				continue;

			TRACE("%s In the Game Insert Player Ranking ZoneID %d\n", pBot->GetName().c_str(), pBot->GetZoneID());
			pBot->AddBotRank(pMap);
		}

		AddMapBotList(pBot);
		pBot->UserInOut(INOUT_IN);
		return pBot->GetID();

	}
	return true;
}

uint16 CGameServerDlg::SpawnUserBot(int Minute, uint8 byZone, float fX, float fY, float fZ, uint8 Restipi, uint8 minlevel /* = 1*/, int16 direction, uint32 SaveID, uint8 Class, _bot_merchant _merchant)
{

	struct _list {
		uint32 index;
		_MERCHANT_BOT_INFO pInfo;
		_list(uint32 index, _MERCHANT_BOT_INFO pInfo) {
			this->index = index;
			this->pInfo = pInfo;
		}
	};
	std::vector<_list> mList;

	m_ArtificialIntelligenceArray.m_lock.lock();
	auto m_sArtificialIntelligenceArray = m_ArtificialIntelligenceArray.m_UserTypeMap;
	m_ArtificialIntelligenceArray.m_lock.unlock();

	foreach(itr, m_sArtificialIntelligenceArray)
	{
		_BOT_DATA* pBotInfo = itr->second;
		if (pBotInfo == nullptr)
			continue;

		if (byZone <= ZONE_ELMORAD && byZone != pBotInfo->m_bNation
			|| (byZone >= ZONE_KARUS_ESLANT
				&& byZone <= ZONE_ELMORAD_ESLANT
				&& byZone != (pBotInfo->m_bNation + 10)))
			continue;

		if (pBotInfo->m_bLevel < minlevel)
			continue;

		// Check if bot already exist
		CBot* pBotCheck = GetBotPtr(pBotInfo->m_sSid);
		if (pBotCheck != nullptr)
			continue;

		CBot* pBot = new CBot();

		pBot->m_strUserID = pBotInfo->m_strUserID;
		pBot->m_bNation = pBotInfo->m_bNation;
		pBot->m_bRace = pBotInfo->m_bRace;
		pBot->m_sClass = pBotInfo->m_sClass;
		pBot->m_nHair = pBotInfo->m_nHair;
		pBot->m_bLevel = pBotInfo->m_bLevel;
		pBot->m_bFace = pBotInfo->m_bFace;
		pBot->m_bKnights = pBotInfo->m_bKnights;
		pBot->m_bFame = pBotInfo->m_bFame;

		memcpy(pBot->m_sItemArray, pBotInfo->m_sItemArray, sizeof(pBot->m_sItemArray));
		memcpy(pBot->m_bstrSkill, pBotInfo->m_bstrSkill, sizeof(pBot->m_bstrSkill));
		memcpy(pBot->m_bStats, pBotInfo->m_bStats, sizeof(pBot->m_bStats));

		pBot->m_sSid = pBotInfo->m_sSid;
		pBot->m_sAchieveCoverTitle = pBotInfo->m_sAchieveCoverTitle;
		pBot->m_reblvl = pBotInfo->m_reblvl;
		pBot->m_iGold = pBotInfo->m_iGold;
		pBot->m_sPoints = pBotInfo->m_sPoints;
		pBot->m_iLoyalty = pBotInfo->m_iLoyalty;
		pBot->m_iLoyaltyMonthly = pBotInfo->m_iLoyaltyMonthly;

		pBot->m_bMerchantState = (SaveID > 0 ? MERCHANT_STATE_SELLING : MERCHANT_STATE_NONE);
		pBot->LastWarpTime = 0;
		pBot->m_sMerchantAreaType = 0;

		if (Minute > 0)
			pBot->LastWarpTime = UNIXTIME + (Minute * 60);

		pBot->m_sDirection = direction;
		pBot->m_pMap = GetZoneByID(byZone);
		pBot->m_bZone = byZone;
		pBot->m_bMerchantIndex = 0;
		pBot->m_iGold = myrand(1000, 5000000);

		if (Restipi == 13 || Restipi == 14)
		{
			_ITEM_DATA* pItem = &pBot->m_sItemArray[RIGHTHAND];

			_ITEM_TABLE pTable = GetItemPtr(pItem->nNum);
			if (pTable.isnull())
				continue;

			if (Class == 1 && !pBot->isWarrior())
				continue;

			if (Class == 2 && !pBot->isRogue())
				continue;

			if (Class == 3 && !pBot->isMage())
				continue;

			if (Class == 4 && !pBot->isPriest())
				continue;

			if (pBot->isRogue() && !pTable.isBow())
				continue;

			if (pBot->isWarrior()
				|| pTable.isShield()
				|| pTable.isPickaxe()
				|| pTable.isFishing())
				continue;

			pBot->m_bGenieStatus = 1;

			if (Restipi == 14)
				pBot->m_bPartyLeader = true;
			else
				pBot->m_bPartyLeader = false;
		}
		else if (Restipi == 50)
		{
			_BOT_SAVE_DATA* pAuto = g_pMain->m_BotSaveDataArray.GetData(SaveID);
			if (pAuto == nullptr)
				return 0;

			uint16 bResult = 1;
			uint8 MerchantItemleri = 0;
			for (int i = 0; i < MAX_MERCH_ITEMS; i++) { if (pAuto->nNum[i] != 0)					MerchantItemleri++; }

			if (MerchantItemleri == 0)
				return false;

			_MERCH_DATA	m_arNewItems[MAX_MERCH_ITEMS]{};

			if (pAuto->sMerchanType == 0)
			{
				Packet result(WIZ_MERCHANT, uint8(MERCHANT_INSERT));

				for (int i = 0; i < MAX_MERCH_ITEMS; i++)
				{
					int8 sItemSlot = pBot->FindSlotForItem(pAuto->nNum[i], pAuto->sCount[i]);
					if (sItemSlot < 0)
						continue;

					auto* pData = pBot->GetItem(sItemSlot);
					if (!pData
						|| pData->nNum != 0)
						continue;

					pData->nNum = pAuto->nNum[i];
					pData->sCount = pAuto->sCount[i];
					pData->sDuration = pAuto->sDuration[i];
					pData->nSerialNum = pAuto->nSerialNum[i];
					pData->MerchItem = true;

					m_arNewItems[i].sCount = pAuto->sCount[i];
					m_arNewItems[i].bCount = pAuto->sCount[i];
					m_arNewItems[i].nNum = pAuto->nNum[i];
					m_arNewItems[i].IsSoldOut = pAuto->IsSoldOut[i];
					m_arNewItems[i].sDuration = pAuto->sDuration[i];
					m_arNewItems[i].nPrice = pAuto->nPrice[i];
					m_arNewItems[i].nSerialNum = pAuto->nSerialNum[i];
					m_arNewItems[i].bOriginalSlot = sItemSlot;
					m_arNewItems[i].isKC = pAuto->isKc[i];
				}

				uint8 reqcount = 0;
				for (int i = 0; i < MAX_MERCH_ITEMS; i++)
					if (m_arNewItems[i].nNum)
						reqcount++;

				if (!reqcount)
					return false;

				uint8 nRandom = 3;

				if (!pAuto->AdvertMessage.empty())
					pBot->MerchantChat = string_format("%s(Location:%d,%d)", pAuto->AdvertMessage.c_str(), uint16(pAuto->fX), uint16(pAuto->fZ));
				else
					pBot->MerchantChat.clear();


				pBot->m_iLoyalty = myrand(3000, 5000);
				pBot->m_bPremiumMerchant = 0;
				pBot->m_bMerchantState = MERCHANT_STATE_SELLING;
				pBot->m_BotState = BOT_MERCHANT;



				result << bResult << pAuto->AdvertMessage << pBot->GetID()
					<< pBot->m_bPremiumMerchant;

				for (int i = 0; i < MAX_MERCH_ITEMS; i++)
					pBot->m_arMerchantItems[i] = m_arNewItems[i];

				for (int i = 0; i < MAX_MERCH_ITEMS; i++)
					result << pBot->m_arMerchantItems[i].nNum;

				pBot->SendToRegion(&result);
			}

			if (pAuto->sMerchanType == 1)
			{
				Packet result(WIZ_MERCHANT, uint8(MERCHANT_BUY_REGION_INSERT));

				for (int i = 0; i < MAX_MERCH_ITEMS; i++) { pBot->m_arMerchantItems[i].nNum = pAuto->nNum[i];					pBot->m_arMerchantItems[i].sCount = pAuto->sCount[i];					pBot->m_arMerchantItems[i].nPrice = pAuto->nPrice[i];					pBot->m_arMerchantItems[i].sDuration = pAuto->sDuration[i];					pBot->m_arMerchantItems[i].isKC = pAuto->isKc[i]; }
				pBot->m_bMerchantState = MERCHANT_STATE_BUYING;
				pBot->m_BotState = BOT_MERCHANT;
				result << pBot->GetID();

				for (int i = 0; i < 4; i++)
					result << pBot->m_arMerchantItems[i].nNum;

				pBot->SendToRegion(&result);
			}
		}
		else if (Restipi == 1)
		{
			_ITEM_DATA* pItem = &pBot->m_sItemArray[RIGHTHAND];
			if (pItem)
			{
				auto pTable = GetItemPtr(myrand(0, 100) > 50 ? GOLDEN_MATTOCK : MATTOCK);
				if (pTable.isnull())
					continue;

				_ITEM_DATA* pTItem = &pBot->m_sItemArray[LEFTHAND];
				if (pTItem) memset(pTItem, 0x00, sizeof(_ITEM_DATA));

				memset(pItem, 0x00, sizeof(_ITEM_DATA));
				pItem->nNum = pTable.m_iNum;
				pItem->nSerialNum = GenerateItemSerial();
				pItem->sCount = 1;
				pItem->sDuration = pTable.m_sDuration;
				pBot->m_bResHpType = USER_MINING;
			}
		}
		else if (Restipi == 2)
		{
			_ITEM_DATA* pItem = &pBot->m_sItemArray[RIGHTHAND];
			if (pItem)
			{
				auto pTable = GetItemPtr(myrand(0, 100) > 50 ? GOLDEN_FISHING : FISHING);
				if (pTable.isnull())
					continue;

				auto* pTItem = &pBot->m_sItemArray[LEFTHAND];
				if (pTItem != nullptr)
					memset(pTItem, 0x00, sizeof(_ITEM_DATA));

				memset(pItem, 0x00, sizeof(_ITEM_DATA));
				pItem->nNum = pTable.m_iNum;
				pItem->nSerialNum = GenerateItemSerial();
				pItem->sCount = 1;
				pItem->sDuration = pTable.m_sDuration;
				pBot->m_bResHpType = USER_FLASHING;
			}
		}
		else if (Restipi == 3 || Restipi == 4)
			pBot->m_bResHpType = Restipi == 3 ? USER_STANDING : USER_SITDOWN;
		else if (Restipi == 5)
			pBot->m_bResHpType = USER_STANDING;// Random > 5000 ? USER_STANDING : USER_SITDOWN;
		else if (Restipi == 6) {}
		else continue;

		if (Restipi == 6)
		{
			pBot->m_bMerchantIndex = _merchant.index;
			pBot->m_sMerchantAreaType = _merchant.areaType;
			pBot->m_BotState = BOT_MERCHANT;

			uint8 itemcount = 0;
			for (int i = 0; i < MAX_MERCH_ITEMS; i++)
			{
				if (_merchant.merc[i].itemid)
					itemcount++;
			}

			_MERCH_DATA	m_arNewItems[MAX_MERCH_ITEMS]{};
			memset(m_arNewItems, 0, sizeof(m_arNewItems));

			if (!_merchant.isBuy) {

				for (int i = 0; i < MAX_MERCH_ITEMS; i++) {
					if (!_merchant.merc[i].itemid)
						continue;

					int8 sItemSlot = pBot->FindSlotForItem(_merchant.merc[i].itemid, _merchant.merc[i].count);
					if (sItemSlot < 0)
						continue;

					auto pItem = g_pMain->GetItemPtr(_merchant.merc[i].itemid);
					if (pItem.isnull())
						continue;

					auto* pData = pBot->GetItem(sItemSlot);
					if (!pData || (pData->nNum != 0 && !pItem.m_bCountable))
						continue;

					pData->nNum = _merchant.merc[i].itemid;
					pData->sCount += _merchant.merc[i].count;
					pData->sDuration = _merchant.merc[i].pTable.m_sDuration;
					pData->nSerialNum = g_pMain->GenerateItemSerial();
					pData->MerchItem = true;

					m_arNewItems[i].sCount = pData->sCount;
					m_arNewItems[i].bCount = pData->sCount;
					m_arNewItems[i].nNum = pData->nNum;
					m_arNewItems[i].IsSoldOut = false;
					m_arNewItems[i].sDuration = pData->sDuration;
					m_arNewItems[i].nPrice = _merchant.merc[i].price;
					m_arNewItems[i].nSerialNum = pData->nSerialNum;
					m_arNewItems[i].bOriginalSlot = sItemSlot;
					m_arNewItems[i].isKC = _merchant.merc[i].iskc;
				}

				uint8 reqcount = 0;
				for (int i = 0; i < MAX_MERCH_ITEMS; i++)
					if (m_arNewItems[i].nNum)
						reqcount++;

				if (!reqcount)
				{
					if (_merchant.index) {
						auto* pCoord = g_pMain->pBotInfo.mCoordinate.GetData(_merchant.index);
						if (pCoord)
							pCoord->used = false;
					}
					return 0;
				}

				pBot->m_bPremiumMerchant = myrand(0, 100) < 15;
				pBot->m_bMerchantState = MERCHANT_STATE_SELLING;
			}
			else {

				uint32 total_price = 0;
				for (int i = 0; i < MAX_MERCH_ITEMS; i++) {
					m_arNewItems[i].sCount = _merchant.merc[i].count;
					m_arNewItems[i].bCount = _merchant.merc[i].count;
					m_arNewItems[i].nNum = _merchant.merc[i].itemid;
					m_arNewItems[i].sDuration = _merchant.merc[i].pTable.m_sDuration;
					m_arNewItems[i].nPrice = _merchant.merc[i].price;
					m_arNewItems[i].isKC = _merchant.merc[i].iskc;
					m_arNewItems[i].bOriginalSlot = i;
					total_price += _merchant.merc[i].price;
				}

				uint8 reqcount = 0;
				for (int i = 0; i < MAX_MERCH_ITEMS; i++)
					if (m_arNewItems[i].nNum)
						reqcount++;

				if (!reqcount)
				{
					if (_merchant.index) {
						auto* pCoord = g_pMain->pBotInfo.mCoordinate.GetData(_merchant.index);
						if (pCoord)
							pCoord->used = false;
					}
					return 0;
				}

				pBot->m_bPremiumMerchant = myrand(0, 100) < 15;
				pBot->m_bMerchantState = MERCHANT_STATE_BUYING;

				if (pBot->m_iGold < total_price)
					pBot->m_iGold = myrand(total_price, total_price + 5000000);
			}

			for (int i = 0; i < MAX_MERCH_ITEMS; i++)
				pBot->m_arMerchantItems[i] = m_arNewItems[i];
		}

		int Random = myrand(0, 10000);
		pBot->m_sTargetID = int16(-1);
		pBot->m_TargetChanged = false;
		pBot->m_bBlockPrivateChat = false;
		pBot->m_BotState = BOT_AFK;
		pBot->SetBotAbility();
		pBot->SetPosition(fX, fY, fZ);
		pBot->SetRegion(pBot->GetNewRegionX(), pBot->GetNewRegionZ());
		pBot->SetZoneAbilityChange(pBot->GetZoneID());
		AddMapBotList(pBot);
		pBot->UserInOut(INOUT_IN);
		pBot->StateChangeServerDirect(1, Random > 5000 ? USER_STANDING : USER_SITDOWN);
		return pBot->GetID();
	}
	return 0;
}

void CGameServerDlg::BotHandlerMainTimer()
{
	try
	{
		// Drain +startbotwar queue: one party per 100ms tick to avoid burst-spawning
		if (m_PendingWarBotsKarus > 0)
		{
			uint16 batch = (uint16)std::min<uint16>(m_PendingWarBotsKarus, MAX_PARTY_USERS);
			uint16 spawnedNow = SpawnPartyWarBots(KARUS, batch);
			m_PendingWarBotsKarus -= std::min(m_PendingWarBotsKarus, batch);
			if (spawnedNow == 0)
				m_PendingWarBotsKarus = 0;
		}
		if (m_PendingWarBotsElmorad > 0)
		{
			uint16 batch = (uint16)std::min<uint16>(m_PendingWarBotsElmorad, MAX_PARTY_USERS);
			uint16 spawnedNow = SpawnPartyWarBots(ELMORAD, batch);
			m_PendingWarBotsElmorad -= std::min(m_PendingWarBotsElmorad, batch);
			if (spawnedNow == 0)
				m_PendingWarBotsElmorad = 0;
		}

		DWORD checknow = GetTickCount();
		time_t dwDiffTime = 0, dwTickTime = 0, fTime2 = 0, fType4Time = 0;
		std::vector<CBot*> willBeOut;
		fTime2 = getMSTime(); // the current time

		// Lock-copy to avoid data race during burst spawns
		m_MapBotList.m_lock.lock();
		auto m_sMapBotListArray = m_MapBotList.m_UserTypeMap;
		m_MapBotList.m_lock.unlock();

		foreach(itr, m_sMapBotListArray)// Update bot sessions
		{
			CBot* pBot = itr->second;
			if (pBot == nullptr)
				continue;

			if (!pBot->isInGame())
				continue;

			if (pBot->LastWarpTime > 0)
			{
				if (pBot->LastWarpTime < UNIXTIME)
				{
					willBeOut.push_back(pBot);
					continue;
				}
			}

			if (pBot->m_tGameStartTimeSavedMagic != 0 && (UNIXTIME - pBot->m_tGameStartTimeSavedMagic) >= 2)
			{
				pBot->m_tGameStartTimeSavedMagic = 0;

				// Restore scrolls...
				pBot->InitType4();
				pBot->RecastSavedMagic();

				if (pBot->isInPKZone())
					pBot->Type4Change();
			}

			if (pBot->isRegionTargetUp())
				pBot->RegionFindAttackProcess();

			dwTickTime = fTime2 - pBot->m_fHPChangeTime;
			if (2 * SECOND < dwTickTime)
				pBot->HpMpChange();

			if (pBot->hasRival()
				&& pBot->hasRivalryExpired())
				pBot->RemoveRival();

			if (pBot->ReplyStatus == 1 && checknow > pBot->ReplyTime)
			{
				CUser* pUser;
				pUser = g_pMain->GetUserPtr(pBot->ReplyID);
				if (pUser != nullptr)
				{
					std::string strUserID;
					std::string PMdetay = string_format("%s", pBot->ReplyChat.c_str());
					strUserID = pBot->GetName();
					Packet result1;
					ChatPacket::Construct(&result1, PRIVATE_CHAT, &PMdetay, &strUserID, pUser->GetNation());
					pUser->Send(&result1);
				}

				pBot->ReplyTime = 0;
				pBot->ReplyStatus = 0;
				pBot->ReplyID = 0;
				pBot->ReplyChat = "";
			}

			switch (pBot->GetBotState())
			{
			case BOT_MINING:
				pBot->BotMining();
				break;
			case BOT_FISHING:
				pBot->BotFishing();
				break;
			case BOT_MERCHANT:
				pBot->BotMerchant();
				break;
			case BOT_DEAD:
				pBot->Regene(INOUT_IN, pBot->isInPKZone() ? 0 : 112754);
				break;
			case BOT_FARMER:
			case BOT_FARMERS:
				pBot->FindMonsterAttackSlot();
				break;
			case BOT_MOVE:
				pBot->MoveProcessGoDeahTown();
				break;
			case BOT_MERCHANT_MOVE:
				pBot->MerchantMoveProcess();
				break;
			case BOT_AFK:
				break;
			}

			// This may not be necessary, but it keeps behaviour identical.
			if (pBot->GetBotState() != BOT_DEAD)
				pBot->m_fDelayTime = getMSTime();

			time_t dwTickTimeType4 = fTime2 - pBot->m_fHPType4CheckTime;
			if (1 * SECOND < dwTickTimeType4 && pBot->isAlive())
			{
				pBot->HPTimeChangeType3();
				pBot->Type4Duration();
				pBot->CheckSavedMagic();
				pBot->m_fHPType4CheckTime = getMSTime();
			}

			if (pBot->isInPKZone())
			{
				float nMaxSpeed = 34.0f;

				if (pBot->GetFame() == COMMAND_CAPTAIN
					|| pBot->isRogue())
					nMaxSpeed = 67.0f;
				else if (pBot->isWarrior()
					|| pBot->isMage()
					|| pBot->isPriest())
					nMaxSpeed = 50.0f;
				else if (pBot->isPortuKurian())
					nMaxSpeed = 50.0f;

				pBot->m_sSpeed = nMaxSpeed;
			}
			else
				pBot->m_sSpeed = 34.0f;
		}

		if (willBeOut.size() > 0)
		{
			foreach(itr, willBeOut)
			{
				(*itr)->UserInOut(INOUT_OUT);
				g_pMain->RemoveMapBotList((*itr)->GetID(), (*itr)->GetName());
			}
		}
	}
	
	catch (std::exception& ex)
	{
#ifdef _DEBUG
		std::string information = string_format("Fixed a Critical Error with Bot Theard system: %s\n", ex.what());
		printf(information.c_str());
		ASSERT(0); /* fix me */
#endif
	}
}
// -----------------------------------------------------------------------
// Party War Bot System (+startbotwar / +startbot)
// -----------------------------------------------------------------------

// Creates a new CBot from a _BOT_DATA template (without adding to map).
static CBot* CreateBotFromTemplate(_BOT_DATA* pBotInfo, uint8 byZone, float fX, float fY, float fZ)
{
	CBot* pBot = new CBot();
	pBot->m_strUserID = pBotInfo->m_strUserID;
	pBot->m_bNation = pBotInfo->m_bNation;
	pBot->m_bRace = pBotInfo->m_bRace;
	pBot->m_sClass = pBotInfo->m_sClass;
	pBot->m_nHair = pBotInfo->m_nHair;
	pBot->m_bLevel = pBotInfo->m_bLevel;
	pBot->m_bFace = pBotInfo->m_bFace;
	pBot->m_bKnights = pBotInfo->m_bKnights;
	pBot->m_bFame = pBotInfo->m_bFame;
	memcpy(pBot->m_sItemArray, pBotInfo->m_sItemArray, sizeof(pBot->m_sItemArray));
	memcpy(pBot->m_bstrSkill, pBotInfo->m_bstrSkill, sizeof(pBot->m_bstrSkill));
	memcpy(pBot->m_bStats, pBotInfo->m_bStats, sizeof(pBot->m_bStats));
	pBot->m_sSid = pBotInfo->m_sSid;
	pBot->m_sAchieveCoverTitle = pBotInfo->m_sAchieveCoverTitle;
	pBot->m_reblvl = pBotInfo->m_reblvl;
	pBot->m_iGold = pBotInfo->m_iGold;
	pBot->m_sPoints = pBotInfo->m_sPoints;
	pBot->m_iLoyalty = pBotInfo->m_iLoyalty;
	pBot->m_iLoyaltyMonthly = pBotInfo->m_iLoyaltyMonthly;
	pBot->m_PlayerKillingLoyaltyDaily = 0;
	pBot->m_PlayerKillingLoyaltyPremiumBonus = 0;
	pBot->m_bKnightsRank = pBotInfo->m_bKareli;
	pBot->m_bPersonalRank = pBotInfo->m_bKaresiz;
	pBot->m_bGenieStatus = false;
	pBot->isReset(false);
	return pBot;
}

static CBot* SpawnNextWarBot(CGameServerDlg* pMain, uint8 nation, bool wantPriest)
{
	pMain->m_ArtificialIntelligenceArray.m_lock.lock();
	auto templates = pMain->m_ArtificialIntelligenceArray.m_UserTypeMap;
	pMain->m_ArtificialIntelligenceArray.m_lock.unlock();
	_BOT_DATA* pFallback = nullptr;
	foreach(itr, templates)
	{
		_BOT_DATA* pBotInfo = itr->second;
		if (pBotInfo == nullptr || pBotInfo->m_bNation != nation) continue;
		if (pMain->GetBotPtr(pBotInfo->m_sSid) != nullptr) continue;
		_ITEM_DATA* pItem = pBotInfo->GetItem(RIGHTHAND);
		if (pItem == nullptr) continue;
		_ITEM_TABLE pTable = pMain->GetItemPtr(pItem->nNum);
		if (pTable.isnull() || pTable.isShield() || pTable.isPickaxe() || pTable.isFishing()) continue;
		if (pBotInfo->isPriest() == wantPriest)
			return CreateBotFromTemplate(pBotInfo, 0, 0.0f, 0.0f, 0.0f);
		if (pFallback == nullptr)
			pFallback = pBotInfo;
	}
	return pFallback != nullptr ? CreateBotFromTemplate(pFallback, 0, 0.0f, 0.0f, 0.0f) : nullptr;
}

static void FormBotParty(CGameServerDlg* pMain, std::vector<CBot*>& members)
{
	if (members.empty()) return;
	_PARTY_GROUP* pParty = new _PARTY_GROUP;
	pParty->wIndex = pMain->m_sPartyIndex.increment();
	for (size_t i = 0; i < members.size() && i < MAX_PARTY_USERS; i++)
	{
		pParty->uid[i] = (short)members[i]->GetID();
		members[i]->m_bInParty = true;
		members[i]->m_sPartyIndex = pParty->wIndex;
		members[i]->m_bPartyLeader = (i == 0);
	}
	if (!pMain->m_PartyArray.PutData(pParty->wIndex, pParty))
		delete pParty;
}
uint16 CGameServerDlg::SpawnPartyWarBots(uint8 nation, uint16 totalCount)
{
	const uint8 byZone = ZONE_RONARK_LAND;
	C3DMap* pWarMap = GetZoneByID(byZone);
	if (pWarMap == nullptr) return 0;

	uint16 spawned = 0;
	uint8 consecutiveEmptyParties = 0;

	auto placeBot = [&](CBot* pBot, uint8 sharedRoute, float baseX, float baseZ)
	{
		float jitterX = (myrand(0, 1) ? 1.0f : -1.0f) * myrand(1, 120);
		float jitterZ = (myrand(0, 1) ? 1.0f : -1.0f) * myrand(1, 120);
		float spawnX = baseX + jitterX;
		float spawnZ = baseZ + jitterZ;
		float spawnY = pWarMap->m_fInitY > 0.0f ? pWarMap->m_fInitY : pBot->GetY();
		pBot->m_bMerchantState = MERCHANT_STATE_NONE;
		pBot->LastWarpTime = 0;
		pBot->m_pMap = pWarMap;
		pBot->m_bZone = byZone;
		pBot->m_sTargetID = int16(-1);
		pBot->m_TargetChanged = false;
		pBot->m_bBlockPrivateChat = false;
		pBot->m_BotState = BOT_MOVE;
		pBot->s_MoveProcess = sharedRoute;
		pBot->m_MoveState = 1;
		pBot->SetBotAbility();
		pBot->SetPosition(spawnX, spawnY, spawnZ);
		pBot->SetRegion(pBot->GetNewRegionX(), pBot->GetNewRegionZ());
		pBot->StateChangeServerDirect(1, USER_STANDING);
		pBot->AddBotRank(pWarMap);
		AddMapBotList(pBot);
		pBot->UserInOut(INOUT_IN);
	};

	while (spawned < totalCount && consecutiveEmptyParties < 3)
	{
		std::vector<CBot*> partyMembers;
		uint8 sharedRoute = (uint8)myrand(1, 10);
		float partyBaseX = 1375.0f, partyBaseZ = 1099.0f;
		if (!CBot::GetRonarkLandRouteStart(sharedRoute, nation, partyBaseX, partyBaseZ))
			CBot::GetRonarkLandRouteStart(1, nation, partyBaseX, partyBaseZ);

		CBot* pPriest = SpawnNextWarBot(this, nation, true);
		if (pPriest != nullptr)
		{
			placeBot(pPriest, sharedRoute, partyBaseX, partyBaseZ);
			partyMembers.push_back(pPriest);
			spawned++;
		}
		while (partyMembers.size() < MAX_PARTY_USERS && spawned < totalCount)
		{
			CBot* pMember = SpawnNextWarBot(this, nation, false);
			if (pMember == nullptr) break;
			placeBot(pMember, sharedRoute, partyBaseX, partyBaseZ);
			partyMembers.push_back(pMember);
			spawned++;
		}
		if (partyMembers.empty()) { consecutiveEmptyParties++; continue; }
		consecutiveEmptyParties = 0;
		FormBotParty(this, partyMembers);
	}
	return spawned;
}
COMMAND_HANDLER(CUser::HandleStartBotWarCommand)
{
	if (!isGM()) return false;
	uint16 countPerNation = 1000;
	if (!vargs.empty())
	{
		int requested = atoi(vargs.front().c_str());
		vargs.pop_front();
		if (requested > 0)
			countPerNation = (uint16)std::min(requested, 2000);
	}
	g_pMain->m_PendingWarBotsKarus += countPerNation;
	g_pMain->m_PendingWarBotsElmorad += countPerNation;
	g_pMain->SendHelpDescription(this, string_format(
		"[Bot War] Queued %d Karus + %d Elmorad bots for Ronark Land. Trickling in over ~%d seconds.",
		countPerNation, countPerNation, (countPerNation / MAX_PARTY_USERS) * 100 / 1000 + 1));
	return true;
}

COMMAND_HANDLER(CUser::HandleStartAllBotsCommand)
{
	if (!isGM()) return false;
	uint16 countPerActivity = 200;
	if (!vargs.empty())
	{
		int requested = atoi(vargs.front().c_str());
		vargs.pop_front();
		if (requested > 0)
			countPerActivity = (uint16)std::min(requested, 1000);
	}
	const int Minute = 60;
	for (int i = 0; i < countPerActivity; i++)
	{
		float jX = myrand(1,40)*1.0f, jZ = myrand(1,40)*1.0f;
		g_pMain->SpawnEventBotMerchant(Minute, ZONE_MORADON, 642.7f+jX, 11.5f, 374.9f+jZ, 0, MIN_LEVEL_ARDREAM);
	}
	for (int i = 0; i < countPerActivity; i++)
	{
		float jX = myrand(1,40)*1.0f, jZ = myrand(1,40)*1.0f;
		g_pMain->SpawnEventAfkBotHandler(Minute, ZONE_MORADON, 642.7f+jX, 11.5f, 374.9f+jZ, MIN_LEVEL_ARDREAM);
	}
	for (int i = 0; i < countPerActivity; i++)
	{
		float jX = myrand(1,40)*1.0f, jZ = myrand(1,40)*1.0f;
		g_pMain->SpawnEventAfkBotHandler(Minute, ZONE_KARUS, 644.1f+jX, 11.4f, 1653.9f+jZ, MIN_LEVEL_ARDREAM);
	}
	for (int i = 0; i < countPerActivity; i++)
	{
		float jX = myrand(1,40)*1.0f, jZ = myrand(1,40)*1.0f;
		g_pMain->SpawnEventAfkBotHandler(Minute, ZONE_ELMORAD, 1427.3f+jX, 12.4f, 379.5f+jZ, MIN_LEVEL_ARDREAM);
	}
	struct MiningPt { uint8 zone; float x, y, z; };
	static const MiningPt kPts[] = {
		{ ZONE_MORADON,  642.7f, 11.5f,  374.9f },
		{ ZONE_ELMORAD, 1427.3f, 12.4f,  379.5f },
		{ ZONE_ELMORAD, 1697.3f, 23.4f,  570.2f },
		{ ZONE_KARUS,    644.1f, 11.4f, 1653.9f },
		{ ZONE_KARUS,    347.1f, 18.1f, 1471.2f },
	};
	int perPoint = std::max(1, (int)countPerActivity / (int)std::size(kPts));
	for (const auto& sp : kPts)
		for (int i = 0; i < perPoint; i++)
			g_pMain->SpawnEventBotMining(Minute, sp.zone, sp.x+myrand(1,20)*1.0f, sp.y, sp.z+myrand(1,20)*1.0f, MIN_LEVEL_ARDREAM);

	g_pMain->m_PendingWarBotsKarus += countPerActivity;
	g_pMain->m_PendingWarBotsElmorad += countPerActivity;
	g_pMain->SendHelpDescription(this, string_format(
		"[Full Bot Test] Spawned ~%d Moradon/Karus/ElMorad/mining bots. Queued %d+%d Ronark Land war bots.",
		countPerActivity, countPerActivity, countPerActivity));
	return true;
}
