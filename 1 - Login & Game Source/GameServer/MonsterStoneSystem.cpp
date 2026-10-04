#include "stdafx.h"

void CUser::SendMonsterStoneFail(uint8 errorid) {
	Packet msevent(WIZ_EVENT, uint8(MONSTER_STONE));
	msevent << errorid;
	Send(&msevent);
}

#pragma region CGameServerDlg::MonsterStoneProcess(Packet & pkt)
void CUser::MonsterStoneProcess(Packet& pkt)
{
	if (!isInGame() || isTrading() || isMerchanting()
		|| isSellingMerchantingPreparing() || isFishing() || isMining())
		return;

	if (pCindWar.isEventUser() && g_pMain->isCindirellaZone(GetZoneID()))
		return SendMonsterStoneFail(1);

	uint32 itemid; pkt >> itemid;

	if (!g_pMain->pServerSetting.monsterstone_status) {
		g_pMain->SendHelpDescription(this, "Monster Stone map is in maintenance mode.");
		//g_pMain->SendHelpDescription(this, "Monster Stone haritasý bakým modundadýr.");
		return SendMonsterStoneFail(1);
	}
		
	Packet msevent(WIZ_EVENT, uint8(MONSTER_STONE));
	msevent << itemid;

	if (itemid != 300144036 && itemid != 300145037 && itemid != 300146038 && itemid != 900144023 && itemid != 300145039)	
		return;

	if (GetHealth() < (GetMaxHealth() / 2))
		return SendMonsterStoneFail(9);

	if (GetZoneID() == ZONE_PRISON || isInMonsterStoneZone() || isInTempleEventZone())
		return SendMonsterStoneFail(1);

	if (m_ismsevent || GetEventRoom() > 0 || !CheckExistItem(itemid, 1))
		return SendMonsterStoneFail(5);

	int roomid = -1;
	for (int i = 0; i < MAX_MONSTER_STONE_ROOM; i++) {
		auto& pRoomInfo = g_pMain->m_TempleEventMonsterStoneRoomList[i];
		if (pRoomInfo.isnull() || pRoomInfo.Active || pRoomInfo.mUserList.size())
			continue;

		roomid = i;
		break;
	}

	if (roomid < 0 || roomid >= MAX_MONSTER_STONE_ROOM)
		return;

	auto& pRoom = g_pMain->m_TempleEventMonsterStoneRoomList[roomid];
	if (pRoom.isnull() || pRoom.Active || pRoom.roomid != roomid)
		return;

	if (itemid == 300145039)
	{

		if (!isInParty() || !isPartyLeader())
			return SendMonsterStoneFail(5);
		uint8 bZoneID = ZONE_STONE3, bFamilly = 0;
		_PARTY_GROUP* pParty = g_pMain->GetPartyPtr(GetPartyID());
		if (pParty == nullptr)
			return SendMonsterStoneFail(5);

		for (int i = 0; i < GetPartyMemberAmount(); i++) {
			CUser* pUser = g_pMain->GetUserPtr(pParty->uid[i]);
			if (pUser == nullptr || !pUser->isInGame())
				continue;

			if (GetNation() != pUser->GetNation())
				return SendMonsterStoneFail(5);
		}
		std::vector< _MONSTER_STONE_LIST_INFORMATION > mlist;
		g_pMain->m_MonsterStoneListInformationArray.m_lock.lock();
		foreach_stlmap_nolock(itr, g_pMain->m_MonsterStoneListInformationArray) {
			if (itr->second == nullptr || itr->second->ZoneID != bZoneID || itr->second->Family != bFamilly)
				continue;

			mlist.push_back(*itr->second);
		}
		g_pMain->m_MonsterStoneListInformationArray.m_lock.unlock();

		if (/*mlist.empty() ||*/ !RobItem(itemid, 1))
			return SendMonsterStoneFail(5);
		pRoom.Active = true;
		pRoom.StartTime = UNIXTIME;
		pRoom.FinishTime = pRoom.StartTime + MONSTER_STONE_TIME;
		pRoom.StartZoneID = GetZoneID();
		pRoom.zoneid = bZoneID;
		pRoom.MonsterFamily = bFamilly;
		pRoom.isBossKilled = false;
		pRoom.WaitingTime = 0;
		pRoom.bPartyMonsterStone = true;
		foreach(aaa, mlist)
			g_pMain->SpawnEventNpc(aaa->sSid, aaa->bType == 0 ? true : false, aaa->ZoneID, aaa->X, aaa->Y, aaa->Z, aaa->sCount, 0,
				MONSTER_STONE_DEAD_TIME, 0, -1, pRoom.roomid + 1, aaa->byDirection, 1, 0, (uint16)SpawnEventType::MonsterStone, aaa->isBoss);
		for (int i = 0; i < MAX_PARTY_USERS; i++)
		{
			CUser* pUser = g_pMain->GetUserPtr(pParty->uid[i]);
			if (pUser == nullptr || pUser->GetSocketID() == GetSocketID()) continue;

			pRoom.mUserList.push_back(pUser->GetSocketID());
			pUser->ZoneChange(pRoom.zoneid, 0.0f, 0.0f, pRoom.roomid + 1);
			pUser->MonsterStoneTimerScreen(MONSTER_STONE_TIME);

		}
		pRoom.mUserList.push_back(GetSocketID());
		ZoneChange(pRoom.zoneid, 0.0f, 0.0f, pRoom.roomid + 1);
		auto* pPartyb = g_pMain->CreateParty(this);
		if (pPartyb == nullptr)
			return SendMonsterStoneFail(5);

		this->EventPartyCreate();
		foreach(itr, pRoom.mUserList)
		{
			if (*itr != this->GetSocketID())
				this->EventPartyInvitationCheck(*itr);
		}

	}
	
	else

	if (g_pMain->pServerSetting.new_monsterstone) {
		if (itemid == 300144036 || itemid == 300145037 || itemid == 300146038) {
			uint8 bZoneID = 0, bFamilly = 0;
			if (itemid == 300144036) bZoneID = ZONE_STONE1;
			else if (itemid == 300145037) bZoneID = ZONE_STONE2;
			else if (itemid == 300146038) bZoneID = ZONE_STONE3;

			if (!isInMonsterStoneZone(bZoneID))
				return SendMonsterStoneFail(5);

			if (bZoneID == ZONE_STONE1) bFamilly = myrand(1, 4);
			else if (bZoneID == ZONE_STONE2) bFamilly = myrand(5, 9);
			else if (bZoneID == ZONE_STONE3) bFamilly = myrand(10, 13);

			if (bZoneID == ZONE_STONE1) {
				if (bFamilly != 1 && bFamilly != 2 && bFamilly != 3
					&& bFamilly != 4) {
					printf("Monster Stone Zone Fail: Stone 1\n");
					return SendMonsterStoneFail(5);
				}
			}
			else if (bZoneID == ZONE_STONE2) {
				if (bFamilly != 5 && bFamilly != 6 && bFamilly != 7 &&
					bFamilly != 8 && bFamilly != 9) {
					printf("Monster Stone Zone Fail: Stone 2\n");
					return SendMonsterStoneFail(5);
				}
			}
			else if (bZoneID == ZONE_STONE3) {
				if (bFamilly != 10 && bFamilly != 11 && bFamilly != 12
					&& bFamilly != 13) {
					printf("Monster Stone Zone Fail: Stone 3\n");
					return SendMonsterStoneFail(5);
				}
			}

			std::vector< _MONSTER_STONE_LIST_INFORMATION > mlist;
			g_pMain->m_MonsterStoneListInformationArray.m_lock.lock();
			foreach_stlmap_nolock(itr, g_pMain->m_MonsterStoneListInformationArray) {
				if (itr->second == nullptr || itr->second->ZoneID != bZoneID || itr->second->Family != bFamilly)
					continue;

				mlist.push_back(*itr->second);
			}
			g_pMain->m_MonsterStoneListInformationArray.m_lock.unlock();

			if (mlist.empty() || !RobItem(itemid, 1))
				return SendMonsterStoneFail(5);

			pRoom.Active = true;
			pRoom.StartTime = UNIXTIME;
			pRoom.FinishTime = pRoom.StartTime + MONSTER_STONE_TIME;
			pRoom.StartZoneID = GetZoneID();
			pRoom.zoneid = bZoneID;
			pRoom.MonsterFamily = bFamilly;
			pRoom.isBossKilled = false;
			pRoom.WaitingTime = 0;
			pRoom.mUserList.push_back(GetSocketID());

			foreach(aaa, mlist)
				g_pMain->SpawnEventNpc(aaa->sSid, aaa->bType == 0 ? true : false, aaa->ZoneID, aaa->X, aaa->Y, aaa->Z, aaa->sCount, 0,
					MONSTER_STONE_DEAD_TIME, 0, -1, pRoom.roomid + 1, aaa->byDirection, 1, 0, (uint16)SpawnEventType::MonsterStone, aaa->isBoss);

			if (pRoom.zoneid == ZONE_STONE1) {
				g_pMain->SpawnEventNpc(16062, false, (uint8)pRoom.zoneid, 204, 0, 201, 1, 0, MONSTER_STONE_DEAD_TIME, 3, -1, pRoom.roomid + 1, 0, 0, 0);
				g_pMain->SpawnEventNpc(12117, false, (uint8)pRoom.zoneid, 204, 0, 197, 1, 0, MONSTER_STONE_DEAD_TIME, 3, -1, pRoom.roomid + 1, 0, 0, 0);
				g_pMain->SpawnEventNpc(31508, false, (uint8)pRoom.zoneid, 204, 0, 193, 1, 0, MONSTER_STONE_DEAD_TIME, 3, -1, pRoom.roomid + 1, 0, 0, 0);
			}
			else if (pRoom.zoneid == ZONE_STONE2) {
				g_pMain->SpawnEventNpc(16062, false, (uint8)pRoom.zoneid, 203, 0, 202, 1, 0, MONSTER_STONE_DEAD_TIME, 3, -1, pRoom.roomid + 1, 0, 0, 0);
				g_pMain->SpawnEventNpc(12117, false, (uint8)pRoom.zoneid, 203, 0, 197, 1, 0, MONSTER_STONE_DEAD_TIME, 3, -1, pRoom.roomid + 1, 0, 0, 0);
				g_pMain->SpawnEventNpc(31508, false, (uint8)pRoom.zoneid, 203, 0, 193, 1, 0, MONSTER_STONE_DEAD_TIME, 3, -1, pRoom.roomid + 1, 0, 0, 0);
			}
			else if (pRoom.zoneid == ZONE_STONE3) {
				g_pMain->SpawnEventNpc(16062, false, (uint8)pRoom.zoneid, 204, 0, 207, 1, 0, MONSTER_STONE_DEAD_TIME, 3, -1, pRoom.roomid + 1, 0, 0, 0);
				g_pMain->SpawnEventNpc(12117, false, (uint8)pRoom.zoneid, 204, 0, 200, 1, 0, MONSTER_STONE_DEAD_TIME, 3, -1, pRoom.roomid + 1, 0, 0, 0);
				g_pMain->SpawnEventNpc(31508, false, (uint8)pRoom.zoneid, 204, 0, 194, 1, 0, MONSTER_STONE_DEAD_TIME, 3, -1, pRoom.roomid + 1, 0, 0, 0);
			}

			if (!ZoneChange(pRoom.zoneid, 0.0f, 0.0f, pRoom.roomid + 1))
				return SendMonsterStoneFail(5);
		}
		else return;
	}
	else {
		if (itemid == 900144023) {

			uint8 bZoneID = 0, bFamilly = 0;
			if (GetLevel() >= 20 && GetLevel() <= 29)
			{
				bZoneID = ZONE_STONE1;
				bFamilly = 1;
			}
			else if (GetLevel() >= 30 && GetLevel() <= 35)
			{
				bZoneID = ZONE_STONE1;
				bFamilly = 2;
			}
			else if (GetLevel() >= 36 && GetLevel() <= 40)
			{
				bZoneID = ZONE_STONE1;
				bFamilly = 3;
			}
			else if (GetLevel() >= 41 && GetLevel() <= 46)
			{
				bZoneID = ZONE_STONE1;
				bFamilly = 4;
			}
			else if (GetLevel() >= 47 && GetLevel() <= 55)
			{
				bZoneID = (uint8)myrand(ZONE_STONE1, ZONE_STONE2);

				if (bZoneID == ZONE_STONE1)
					bFamilly = 4;
				else
					bFamilly = 5;
			}
			else if (GetLevel() >= 56 && GetLevel() <= 60)
			{
				bZoneID = ZONE_STONE2;
				bFamilly = (uint8)myrand(6, 8);
			}
			else if (GetLevel() >= 61 && GetLevel() <= 66)
			{
				bZoneID = ZONE_STONE2;
				bFamilly = (uint8)myrand(8, 9);
			}
			else if (GetLevel() >= 67 && GetLevel() <= 70)
			{
				bFamilly = (uint8)myrand(9, 10);

				if (bFamilly == 9)
					bZoneID = ZONE_STONE2;
				else
					bZoneID = ZONE_STONE3;
			}
			else if (GetLevel() >= 71 && GetLevel() <= 74)
			{
				bZoneID = ZONE_STONE3;
				bFamilly = (uint8)myrand(10, 12);
			}
			else if (GetLevel() >= 75 && GetLevel() <= g_pMain->m_byMaxLevel)
			{
				bZoneID = ZONE_STONE3;
				bFamilly = 13;
			}

			if (!bFamilly)
				return;

			if (!isInMonsterStoneZone(bZoneID))
				return SendMonsterStoneFail(5);

			if (bZoneID == ZONE_STONE1) {
				if (bFamilly != 1 && bFamilly != 2 && bFamilly != 3
					&& bFamilly != 4) {
					printf("Monster Stone Zone Fail: Stone 1\n");
					return SendMonsterStoneFail(5);
				}
			}
			else if (bZoneID == ZONE_STONE2) {
				if (bFamilly != 5 && bFamilly != 6 && bFamilly != 7 &&
					bFamilly != 8 && bFamilly != 9) {
					printf("Monster Stone Zone Fail: Stone 2\n");
					return SendMonsterStoneFail(5);
				}
			}
			else if (bZoneID == ZONE_STONE3) {
				if (bFamilly != 10 && bFamilly != 11 && bFamilly != 12
					&& bFamilly != 13) {
					printf("Monster Stone Zone Fail: Stone 3\n");
					return SendMonsterStoneFail(5);
				}
			}

			std::vector< _MONSTER_STONE_LIST_INFORMATION > mlist;
			g_pMain->m_MonsterStoneListInformationArray.m_lock.lock();
			foreach_stlmap_nolock(itr, g_pMain->m_MonsterStoneListInformationArray) {
				if (itr->second == nullptr || itr->second->ZoneID != bZoneID || itr->second->Family != bFamilly)
					continue;

				mlist.push_back(*itr->second);
			}
			g_pMain->m_MonsterStoneListInformationArray.m_lock.unlock();

			if (mlist.empty() || !RobItem(900144023, 1))
				return SendMonsterStoneFail(5);

			pRoom.Active = true;
			pRoom.StartTime = UNIXTIME;
			pRoom.FinishTime = pRoom.StartTime + MONSTER_STONE_TIME;
			pRoom.StartZoneID = GetZoneID();
			pRoom.zoneid = bZoneID;
			pRoom.MonsterFamily = bFamilly;
			pRoom.isBossKilled = false;
			pRoom.WaitingTime = 0;
			pRoom.mUserList.push_back(GetSocketID());

			foreach(aaa, mlist)
				g_pMain->SpawnEventNpc(aaa->sSid, aaa->bType == 0 ? true : false, aaa->ZoneID, aaa->X, aaa->Y, aaa->Z, aaa->sCount, 0,
					MONSTER_STONE_DEAD_TIME, 0, -1, pRoom.roomid + 1, aaa->byDirection, 1, 0, (uint16)SpawnEventType::MonsterStone, aaa->isBoss);

			if (pRoom.zoneid == ZONE_STONE1) {
				g_pMain->SpawnEventNpc(16062, false, (uint8)pRoom.zoneid, 204, 0, 201, 1, 0, MONSTER_STONE_DEAD_TIME, 3, -1, pRoom.roomid + 1, 0, 0, 0);
				g_pMain->SpawnEventNpc(12117, false, (uint8)pRoom.zoneid, 204, 0, 197, 1, 0, MONSTER_STONE_DEAD_TIME, 3, -1, pRoom.roomid + 1, 0, 0, 0);
				g_pMain->SpawnEventNpc(31508, false, (uint8)pRoom.zoneid, 204, 0, 193, 1, 0, MONSTER_STONE_DEAD_TIME, 3, -1, pRoom.roomid + 1, 0, 0, 0);
			}
			else if (pRoom.zoneid == ZONE_STONE2) {
				g_pMain->SpawnEventNpc(16062, false, (uint8)pRoom.zoneid, 203, 0, 202, 1, 0, MONSTER_STONE_DEAD_TIME, 3, -1, pRoom.roomid + 1, 0, 0, 0);
				g_pMain->SpawnEventNpc(12117, false, (uint8)pRoom.zoneid, 203, 0, 197, 1, 0, MONSTER_STONE_DEAD_TIME, 3, -1, pRoom.roomid + 1, 0, 0, 0);
				g_pMain->SpawnEventNpc(31508, false, (uint8)pRoom.zoneid, 203, 0, 193, 1, 0, MONSTER_STONE_DEAD_TIME, 3, -1, pRoom.roomid + 1, 0, 0, 0);
			}
			else if (pRoom.zoneid == ZONE_STONE3) {
				g_pMain->SpawnEventNpc(16062, false, (uint8)pRoom.zoneid, 204, 0, 207, 1, 0, MONSTER_STONE_DEAD_TIME, 3, -1, pRoom.roomid + 1, 0, 0, 0);
				g_pMain->SpawnEventNpc(12117, false, (uint8)pRoom.zoneid, 204, 0, 200, 1, 0, MONSTER_STONE_DEAD_TIME, 3, -1, pRoom.roomid + 1, 0, 0, 0);
				g_pMain->SpawnEventNpc(31508, false, (uint8)pRoom.zoneid, 204, 0, 194, 1, 0, MONSTER_STONE_DEAD_TIME, 3, -1, pRoom.roomid + 1, 0, 0, 0);
			}

			if (!ZoneChange(pRoom.zoneid, 0.0f, 0.0f, pRoom.roomid + 1))
				return SendMonsterStoneFail(5);
		}
		else return;
	}
	
	m_ismsevent = true;
	MonsterStoneTimerScreen(MONSTER_STONE_TIME);
}
#pragma endregion

#pragma region CGameServerDlg::TempleMonsterStoneTimer()
void CGameServerDlg::TempleMonsterStoneTimer()
{
	for (int i = 0; i < MAX_MONSTER_STONE_ROOM; i++) {
		auto& pRoomInfo = g_pMain->m_TempleEventMonsterStoneRoomList[i];
		if (pRoomInfo.isnull() || !pRoomInfo.Active)
			continue;

		uint32 remtime = 0, waittime = 0;
		if (pRoomInfo.FinishTime > UNIXTIME) 
			remtime = uint32(pRoomInfo.FinishTime - UNIXTIME);
		
		if (pRoomInfo.WaitingTime > UNIXTIME)
			waittime = uint32(pRoomInfo.WaitingTime - UNIXTIME);

		if ((!pRoomInfo.isBossKilled && !remtime) || (pRoomInfo.isBossKilled && !waittime)) {
			pRoomInfo.Active = false;
			TempleMonsterStoneAutoResetRoom(pRoomInfo);
		}
	}
}
#pragma endregion

#pragma region CGameServerDlg::TempleMonsterStoneAutoResetRoom(_MONSTER_STONE_INFO &pRoom)
void CGameServerDlg::TempleMonsterStoneAutoResetRoom(_MONSTER_STONE_INFO& pRoom)
{
	if (pRoom.isnull())
		return;

	foreach(aaakiki, pRoom.mUserList) {
		CUser* pUser = g_pMain->GetUserPtr(*aaakiki);
		if (pUser == nullptr || !pUser->isInGame() || !pUser->isInMonsterStoneZone()) 
			continue;
		
		pUser->m_ismsevent = false;
		pUser->ZoneChange(ZONE_MORADON, 0.0f, 0.0f, 0);
	}

	TempleMonsterStoneResetNpcs(pRoom.roomid, pRoom.zoneid);
	pRoom.reset();
}
#pragma endregion

#pragma region CGameServerDlg::TempleMonsterStoneResetNpcs(int16 roomid, uint8 zoneid)
void CGameServerDlg::TempleMonsterStoneResetNpcs(int16 roomid, uint8 zoneid)
{
	if (roomid == -1)
		return;

	CNpcThread* zoneitrThread = g_pMain->m_arNpcThread.GetData(zoneid);
	if (zoneitrThread == nullptr)
		return;

	foreach_stlmap(itr, zoneitrThread->m_arNpcArray) {
		CNpc* pNpc = TO_NPC(itr->second);
		if (pNpc == nullptr || pNpc->isDead() || pNpc->GetZoneID() != zoneid || pNpc->GetEventRoom() != (roomid + 1))
			continue;

		pNpc->Dead();
	}
}
#pragma endregion

#pragma region CGameServerDlg::TempleMonsterStoneItemExitRoom()
void CUser::TempleMonsterStoneItemExitRoom()
{
	if (GetEventRoom() < 1 || GetEventRoom() > MAX_TEMPLE_QUEST_EVENT_ROOM)
		return;

	auto& pRoomInfo = g_pMain->m_TempleEventMonsterStoneRoomList[GetEventRoom() - 1];
	if (pRoomInfo.isnull() || !pRoomInfo.Active)
		return;

	m_bEventRoom = 0;
	m_ismsevent = false;
	g_pMain->TempleMonsterStoneResetNpcs(pRoomInfo.roomid, pRoomInfo.zoneid);
	pRoomInfo.reset();
}
#pragma endregion


#pragma region CNpc::MonsterStoneKillProcess(CUser *pUser)
void CNpc::MonsterStoneKillProcess(CUser *pUser)
{
	if (pUser == nullptr || e_stype != e_summontype::m_MonsterStoneBoss || GetEventRoom() < 1 || GetEventRoom() > MAX_MONSTER_STONE_ROOM)
		return;

	auto& pRoomInfo = g_pMain->m_TempleEventMonsterStoneRoomList[GetEventRoom() - 1];
	if (pRoomInfo.isnull() || !pRoomInfo.Active || pRoomInfo.isBossKilled)
		return;

	pRoomInfo.isBossKilled = true;
	pRoomInfo.WaitingTime = UNIXTIME + MONSTER_STONE_FINISH_TIME;
	Packet result(WIZ_EVENT);
	result << uint8(TEMPLE_EVENT_FINISH) << uint8(0x11) << uint8(0x00) << uint8(0x65) << uint8(0x14) << uint32(0x00);
	pUser->Send(&result);

	result.clear();
	result.Initialize(WIZ_QUEST);
	result << uint8(2) << uint16(209) << uint8(0);
	pUser->Send(&result);
}
#pragma endregion

#pragma region CUser::MonsterStoneTimerScreen(uint16 Time)
void CUser::MonsterStoneTimerScreen(uint16 Time)
{
	bool bIsNeutralZone = (GetZoneID() >= ZONE_STONE1 && GetZoneID() <= ZONE_STONE3);

	if (!bIsNeutralZone || !isInGame())
		return;

	Packet result;

	result.Initialize(WIZ_BIFROST);
	result << uint8(MONSTER_SQUARD) << uint16(Time);
	Send(&result);

	result.clear();
	result.Initialize(WIZ_SELECT_MSG);
	result << uint16(0) << uint8(7) << uint64(0) << uint8(9) << uint16(0) << uint8(0) << uint8(11) << uint16(Time) << uint16(0);
	Send(&result);
}
#pragma endregion