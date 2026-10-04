#pragma once
class SearchMonster;
#include "stdafx.h"
#include "Pearl Engine.h"
#include "N3BASE\N3UIBase.h"
#include "N3BASE\N3UIString.h"
#include "N3BASE\N3UIButton.h"
#include "N3BASE\N3UIEdit.h"
#include "N3BASE\N3UIScrollBar.h"

class SearchMonster : public CN3UIBase
{
	CN3UIButton* btn_close, *btn_search, *btn_drops, *btn_previously, *btn_next;
	CN3UIEdit* edit_search;

	CN3UIList* Drop_list;
	CN3UIArea* items[12]{ 0 };
	CN3UIImage* icons[12]{ 0 };

	CN3UIString* txt_monster_name, *str_title , *mob_search, * text_page;

	struct MONSTER_INFO
	{
		uint16 sID;
		std::string strName;
		MONSTER_INFO(uint16 id, std::string name)
		{
			sID = id;
			strName = name;
		}
	};

	struct ItemInfo
	{
		uint32 nItemID;
		CN3UIIcon* icon;
		TABLE_ITEM_BASIC* tbl;
		ItemInfo()
		{
			nItemID = 0;
			icon = NULL;
			tbl = nullptr;
		}
	};

	vector<ItemInfo*> itemsx;

	vector<MONSTER_INFO> monsters;

	uint32 page;
	bool needToReset;
public:
	uint8 m_CurrentPage;
	uint8 m_MaxPage;
	vector<DropItem> m_Drops;
	size_t oldLen;
	SearchMonster();
	~SearchMonster();
	bool Load(HANDLE hFile);
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg);
	uint32_t MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld);
	bool OnKeyPress(int iKey);
	void Close();
	void Open();
	virtual void	OpenWithAnimation(uint32 frameTime = 200);
	void OpenSearchMonster();
	void Tick();
	CN3UIString* GetItemText(CN3UIButton* item);
	void Update(uint16 target, uint8 isMonster, vector<DropItem> drops);
	void Order(uint8 page = 1);
	__IconItemSkill* GetItemStruct(CN3UIImage* img, std::string fileSz, __TABLE_ITEM_BASIC* tbl, uint32 realID);
};