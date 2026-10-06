// 2369 Power-Up Store (re_powerupstore.istirap): liste ve kategoriler sunucudan XSafe (0xE9) alt paketleriyle gelir
// (XGuard.cpp XSafe_SendPUS: PUS 0xA8, PusCat 0xD6, CASHCHANGE 0xA9). Satın alma: XSafe | PUS | 1 | u32 pusID | u8 adet.
#pragma once
#include "UIGeneric2369.h"
#include <vector>
#include <string>
#include <cstdint>

class Packet;

class CUIPowerUpStore2369 : public CUIGeneric2369
{
public:
	struct Item
	{
		uint32_t id = 0, itemID = 0, price = 0, buyCount = 0;
		uint8_t cat = 0, priceType = 0; // 0 = Knight Cash, 1 = TL
	};
	struct Category
	{
		uint32_t id = 0;
		std::string name;
		uint8_t status = 0;
	};

	CUIPowerUpStore2369() = default;
	~CUIPowerUpStore2369() override = default;

	bool LoadCentered(const std::string& szFile, int iScreenW, int iScreenH, const char* szLogName);
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	void Open();

	/// WIZ_XSAFE paketi: PUS/PusCat/CASHCHANGE ise işler ve true döner; değilse okuma konumunu geri alır ve false döner
	bool OnXSafePacket(Packet& pkt);

	static constexpr int SLOTS_PER_PAGE = 16; // items1..items4 × item1..item4
	static constexpr int TAB_COUNT      = 5;  // btn_tab1..btn_tab5

private:
	void RequestList();
	void SendPurchase(uint32_t dwPusID, uint8_t byCount);
	void Refresh();
	std::vector<const Item*> VisibleItems() const;
	int SlotOfSender(CN3UIBase* pSender) const;
	void FillSlot(CN3UIBase* pSlot, const Item* pItem);
	std::string ItemName(uint32_t dwItemID) const;
	std::string ItemIcon(uint32_t dwItemID) const;

	std::vector<Item> m_Items;
	std::vector<Category> m_Cats;
	std::vector<const Item*> m_PageItems; // şu an gösterilen 16 yuva
	int m_iTab        = 0;  // seçili sekme (kategori dizini)
	int m_iPage       = 0;
	bool m_bRequested = false;
	bool m_bListed    = false;
	uint32_t m_dwCash = 0, m_dwTL = 0;
};
