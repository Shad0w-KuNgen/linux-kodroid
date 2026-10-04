#pragma once
class CTableManager;
#include "N3BASE/GameDef.h"
#include "stdafx.h"
#include "N3BASE/N3TableBase.h"

#include "Pearl Engine.h"


class CTableManager
{
public:
	
	CTableManager();
	~CTableManager();
	void Init();
	__TABLE_ITEM_BASIC* getItemData(uint32 ID);
	__TABLE_ITEM_EXT* getExtData(uint32 extNum, uint32 ID);
	std::map<uint32_t, __TABLE_ITEM_BASIC>* GetItemTable();
	TABLE_MOB* getMobData(uint32 ID);
	TABLE_NPC* getNpcData(uint32 ID);

	std::map<uint32_t, TABLE_MOB*>  GetMobTable();
	std::map<uint32_t, TABLE_NPC>* GetNpcTable();
private:

};