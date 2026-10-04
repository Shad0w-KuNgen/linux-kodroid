#include "TableManager.h"

CTableManager::CTableManager()
{
}

CTableManager::~CTableManager()
{
}

typedef struct __TABLE_TEXT
{
	uint32_t m_iID;
	std::string m_strText;
} TABLE_TEXT;

static CN3TableBase<TABLE_ITEM_BASIC> tbl_item;
static CN3TableBase<TABLE_MOB> mobs;
static CN3TableBase<TABLE_NPC> npcs;
static CN3TableBase<TABLE_TEXT> texts;
static CN3TableBase<TABLE_ITEM_EXT> ext[45];

typedef TABLE_ITEM_BASIC* (__thiscall* tGetItemBase)(DWORD, DWORD);
typedef TABLE_ITEM_EXT*(__thiscall* tGetItemExtBase)(DWORD, DWORD);
typedef TABLE_MOB* (__thiscall* tGetMobBase)(DWORD, DWORD);


const DWORD KO_ITCA = 0x004BAD30; // 2369
const DWORD KO_ITCB = 0x004BADA0; // 2369
const DWORD KO_ITPA = 0x00F3E688; // 2369
const DWORD KO_ITPB = 0x00F3E690; // 2369
const DWORD KO_MOBC = 0x00520A00; // 2369
const DWORD KO_MOBA = 0x00F3E7C8; // 2369

tGetItemBase __GetItemBase = (tGetItemBase)(KO_ITCA);
tGetItemExtBase __GetItemExtBase = (tGetItemExtBase)(KO_ITCB);
tGetMobBase __GetMobBase = (tGetMobBase)(KO_MOBC);

TABLE_ITEM_BASIC* GetItemBase(DWORD ItemID) {

	if (!ItemID)
		return 0;

	return __GetItemBase(*(DWORD*)(KO_ITPA), ItemID / 1000 * 1000);
}

TABLE_ITEM_EXT* GetItemExtBase(DWORD dwItemNum, DWORD bExtIndex) {

	if (!dwItemNum || dwItemNum / 1000000000 >= 4 || bExtIndex > 44)
		return NULL;

	DWORD* extTbl = (DWORD*)KO_ITPB;

	return __GetItemExtBase(extTbl[bExtIndex], dwItemNum >= 1000000000 ? dwItemNum % 1000 + 1000 : dwItemNum % 1000);

}

TABLE_MOB* GetMobBase(DWORD MobID)
{
	if (!MobID)
		return 0;

	return __GetMobBase(*(DWORD*)(KO_MOBA), MobID);
}

void LoadNpcTbl()
{
	string szFN = xorstr("Data\\npcs.tbl");
	npcs.LoadFromFile(szFN.c_str());
}

void CTableManager::Init()
{
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)LoadNpcTbl, NULL, NULL, NULL);
}

uint32 getItemOriginIDExAS(uint32 sID)
{
	std::string test = std::to_string(sID);
	if (test.length() <= 9) {
		return std::atoi((std::to_string(sID).substr(0, 6) + xorstr("000")).c_str());
	}
	else {
		return std::atoi((std::to_string(sID).substr(0, 7) + xorstr("000")).c_str());
	}
}

TABLE_ITEM_BASIC * CTableManager::getItemData(uint32 ID) 
{
	return GetItemBase(ID);
}

std::map<uint32_t, TABLE_ITEM_BASIC>* CTableManager::GetItemTable() 
{
	return tbl_item.GetTable();
}

TABLE_MOB * CTableManager::getMobData(uint32 ID) 
{
	return GetMobBase(ID);
}

TABLE_NPC * CTableManager::getNpcData(uint32 ID)
{
	TABLE_NPC * d = npcs.Find(ID);
	return (d == NULL ? nullptr : d);
}

TABLE_ITEM_EXT * CTableManager::getExtData(uint32 extNum, uint32 ID) 
{
	return GetItemExtBase(ID, extNum);
}

std::map<uint32_t, TABLE_MOB*> m_Mobs;

std::map<uint32_t, TABLE_MOB*> CTableManager::GetMobTable() {
	if (m_Mobs.empty())
	{
		for (uint32 i = 100; i < 9974; i++)
		{
			if (auto mob = getMobData(i))
			{
				m_Mobs.insert({ mob->ID, mob });
			}
		}
	}
	return m_Mobs;
}

std::map<uint32_t, TABLE_NPC>* CTableManager::GetNpcTable() {
	return npcs.GetTable();
}
