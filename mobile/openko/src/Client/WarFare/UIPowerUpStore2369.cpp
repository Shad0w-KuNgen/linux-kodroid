#include "StdAfx.h"
#include "UIPowerUpStore2369.h"
#include "GameBase.h"
#include "GameProcedure.h"
#include "APISocket.h"
#include "PlayerMySelf.h"
#include "KoProtocol.h"
#include <N3Base/LogWriter.h>
#include <N3Base/N3UIImage.h>
#include <N3Base/N3UIString.h>
#include <N3Base/N3UIButton.h>
#include <shared/packets.h>
#include <shared/ByteBuffer.h>
#include <algorithm>
#include <cctype>

namespace
{
constexpr uint8_t XSAFE_PUS        = 0xA8;
constexpr uint8_t XSAFE_CASHCHANGE = 0xA9;
constexpr uint8_t XSAFE_PUSCAT     = 0xD6;

std::string Lower(std::string s)
{
	for (char& c : s)
		c = (char) std::tolower((unsigned char) c);
	return s;
}
} // namespace

bool CUIPowerUpStore2369::LoadCentered(const std::string& szFile, int iScreenW, int iScreenH, const char* szLogName)
{
	if (!CUIGeneric2369::LoadCentered(szFile, iScreenW, iScreenH, szLogName))
		return false;
	// Bu sürümde kullanılmayan alt gruplar (iade listesi, sepet onayı, satın alma onayı, sepet satırları, ESN)
	for (const char* szHide : {"base_refund", "Basket_confirm", "shopping_confirm", "purchase_list1", "purchase_list2", "purchase_list3",
			 "purchase_list4", "purchase_list5", "purchase_list6", "purchase_list7", "btn_refund", "btn_reseller", "btn_buyall",
			 "btn_clearall", "btn_useesn", "esncode", "text_basket_total_price", "edit_search", "btn_search"})
		if (CN3UIBase* p = GetChildByID(szHide))
			p->SetVisible(false);
	// Her yuvadaki "sepete ekle" düğmesi gizli (sepet yok), satın al kalır
	for (int g = 1; g <= 4; g++)
	{
		CN3UIBase* pGroup = GetChildByID(fmt::format("items{}", g));
		if (pGroup == nullptr)
			continue;
		for (int i = 1; i <= 4; i++)
			if (CN3UIBase* pSlot = pGroup->GetChildByID(fmt::format("item{}", i)))
			{
				if (CN3UIBase* pBasket = pSlot->GetChildByID("btn_add_basket"))
					pBasket->SetVisible(false);
				pSlot->SetVisible(false);
			}
	}
	Refresh();
	return true;
}

void CUIPowerUpStore2369::Open()
{
	CUIGeneric2369::Open();
	if (!m_bListed)
		RequestList();
	Refresh();
}

void CUIPowerUpStore2369::RequestList()
{
	if (CGameProcedure::s_pSocket == nullptr)
		return;
	uint8_t byBuff[4];
	int iOffset = 0;
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_XSAFE);
	CAPISocket::MP_AddByte(byBuff, iOffset, XSAFE_PUS);
	CAPISocket::MP_AddByte(byBuff, iOffset, 0); // 0 = liste iste (XSafe_PusRequest)
	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
	m_bRequested = true;
	CLogWriter::Write("PUS: liste istendi (XSafe PUS 0)");
}

void CUIPowerUpStore2369::SendPurchase(uint32_t dwPusID, uint8_t byCount)
{
	if (CGameProcedure::s_pSocket == nullptr)
		return;
	uint8_t byBuff[12];
	int iOffset = 0;
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_XSAFE);
	CAPISocket::MP_AddByte(byBuff, iOffset, XSAFE_PUS);
	CAPISocket::MP_AddByte(byBuff, iOffset, 1); // 1 = satın al
	CAPISocket::MP_AddDword(byBuff, iOffset, dwPusID);
	CAPISocket::MP_AddByte(byBuff, iOffset, byCount);
	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
	CLogWriter::Write("PUS: satın alma istendi (kayıt {}, adet {})", dwPusID, (int) byCount);
}

bool CUIPowerUpStore2369::OnXSafePacket(Packet& pkt)
{
	size_t iPos = pkt.rpos();
	if (pkt.size() < iPos + 1)
		return false;
	uint8_t bySub = pkt.read<uint8_t>();
	auto remaining = [&](size_t n) { return pkt.size() >= pkt.rpos() + n; };
	switch (bySub)
	{
		case XSAFE_PUS:
		{
			if (!remaining(4))
				break;
			uint32_t dwCount = pkt.read<uint32_t>();
			m_Items.clear();
			for (uint32_t i = 0; i < dwCount && remaining(4 + 4 + 4 + 1 + 4 + 1); i++)
			{
				Item it;
				it.id        = pkt.read<uint32_t>();
				it.itemID    = pkt.read<uint32_t>();
				it.price     = pkt.read<uint32_t>();
				it.cat       = pkt.read<uint8_t>();
				it.buyCount  = pkt.read<uint32_t>();
				it.priceType = pkt.read<uint8_t>();
				m_Items.push_back(it);
			}
			m_bListed = true;
			CLogWriter::Write("PUS: {} kayıt alındı ({} bayt)", m_Items.size(), pkt.size());
			Refresh();
			return true;
		}
		case XSAFE_PUSCAT:
		{
			if (!remaining(4))
				break;
			uint32_t dwCount = pkt.read<uint32_t>();
			m_Cats.clear();
			for (uint32_t i = 0; i < dwCount && remaining(4 + 2); i++)
			{
				Category c;
				c.id = pkt.read<uint32_t>();
				if (!pkt.readString(c.name))
					break;
				if (!remaining(1))
					break;
				c.status = pkt.read<uint8_t>();
				m_Cats.push_back(c);
			}
			std::string szList;
			for (const Category& c : m_Cats)
				szList += fmt::format("[{} {} {}] ", c.id, c.name, (int) c.status);
			CLogWriter::Write("PUS: {} kategori alındı: {}", m_Cats.size(), szList);
			Refresh();
			return true;
		}
		case XSAFE_CASHCHANGE:
		{
			if (!remaining(4))
				break;
			m_dwCash = pkt.read<uint32_t>();
			if (remaining(4))
				m_dwTL = pkt.read<uint32_t>();
			Refresh();
			return true;
		}
		default:
			break;
	}
	pkt.rpos(iPos);
	return false;
}

std::vector<const CUIPowerUpStore2369::Item*> CUIPowerUpStore2369::VisibleItems() const
{
	std::vector<const Item*> v;
	int iCatID = (m_iTab >= 0 && m_iTab < (int) m_Cats.size()) ? (int) m_Cats[m_iTab].id : -1;
	for (const Item& it : m_Items)
		if (iCatID < 0 || (int) it.cat == iCatID)
			v.push_back(&it);
	return v;
}

std::string CUIPowerUpStore2369::ItemName(uint32_t dwItemID) const
{
	__TABLE_ITEM_BASIC* pItem = CGameBase::s_pTbl_Items_Basic.Find(dwItemID / 1000 * 1000);
	if (pItem == nullptr)
		return fmt::format("#{}", dwItemID);
	__TABLE_ITEM_EXT* pExt = nullptr;
	if (pItem->byExtIndex >= 0 && pItem->byExtIndex < MAX_ITEM_EXTENSION)
		pExt = CGameBase::s_pTbl_Items_Exts[pItem->byExtIndex].Find(dwItemID % 1000);
	if (pExt != nullptr && !pExt->szHeader.empty())
		return pExt->szHeader + " " + pItem->szName;
	return pItem->szName;
}

std::string CUIPowerUpStore2369::ItemIcon(uint32_t dwItemID) const
{
	__TABLE_ITEM_BASIC* pItem = CGameBase::s_pTbl_Items_Basic.Find(dwItemID / 1000 * 1000);
	if (pItem == nullptr)
		return {};
	__TABLE_ITEM_EXT* pExt = nullptr;
	if (pItem->byExtIndex >= 0 && pItem->byExtIndex < MAX_ITEM_EXTENSION)
		pExt = CGameBase::s_pTbl_Items_Exts[pItem->byExtIndex].Find(dwItemID % 1000);
	if (pExt == nullptr)
		return {};
	std::string szIcon;
	e_PartPosition ePart = PART_POS_UNKNOWN;
	e_PlugPosition ePlug = PLUG_POS_UNKNOWN;
	CGameBase::MakeResrcFileNameForUPC(pItem, pExt, nullptr, &szIcon, ePart, ePlug);
	return szIcon;
}

void CUIPowerUpStore2369::FillSlot(CN3UIBase* pSlot, const Item* pItem)
{
	if (pSlot == nullptr)
		return;
	if (pItem == nullptr)
	{
		pSlot->SetVisible(false);
		return;
	}
	pSlot->SetVisible(true);
	if (CN3UIString* p = pSlot->GetChildByID<CN3UIString>("item_name"))
		p->SetString(ItemName(pItem->itemID));
	if (CN3UIString* p = pSlot->GetChildByID<CN3UIString>("item_price"))
		p->SetString(fmt::format("Price: {} {}", pItem->price, pItem->priceType == 0 ? "KC" : "TL"));
	if (CN3UIString* p = pSlot->GetChildByID<CN3UIString>("item_quantitiy"))
		p->SetString(fmt::format("Quantity: {}", pItem->buyCount));
	if (CN3UIImage* p = pSlot->GetChildByID<CN3UIImage>("item_icon"))
	{
		std::string szIcon = ItemIcon(pItem->itemID);
		if (!szIcon.empty())
		{
			p->SetTex(szIcon);
			p->SetUVRect(0.0f, 0.0f, 45.0f / 64.0f, 45.0f / 64.0f);
		}
	}
	if (CN3UIBase* p = pSlot->GetChildByID("btn_add_basket"))
		p->SetVisible(false);
}

void CUIPowerUpStore2369::Refresh()
{
	if (GetChildren().empty())
		return;
	// Sekmeler: kategoriler (en çok 5)
	for (int t = 0; t < TAB_COUNT; t++)
	{
		CN3UIBase* pTab = GetChildByID(fmt::format("btn_tab{}", t + 1));
		if (pTab == nullptr)
			continue;
		bool bHas = t < (int) m_Cats.size();
		pTab->SetVisible(bHas);
		if (CN3UIString* pTxt = pTab->GetChildByID<CN3UIString>("txt"))
			pTxt->SetString(bHas ? m_Cats[t].name : "");
		if (CN3UIButton* pBtn = dynamic_cast<CN3UIButton*>(pTab))
			pBtn->SetState(t == m_iTab ? UI_STATE_BUTTON_DOWN : UI_STATE_BUTTON_NORMAL);
	}

	std::vector<const Item*> v = VisibleItems();
	int iPages                 = std::max(1, ((int) v.size() + SLOTS_PER_PAGE - 1) / SLOTS_PER_PAGE);
	m_iPage                    = std::clamp(m_iPage, 0, iPages - 1);
	m_PageItems.assign(SLOTS_PER_PAGE, nullptr);
	for (int s = 0; s < SLOTS_PER_PAGE; s++)
	{
		int idx = m_iPage * SLOTS_PER_PAGE + s;
		m_PageItems[s] = idx < (int) v.size() ? v[idx] : nullptr;
		CN3UIBase* pGroup = GetChildByID(fmt::format("items{}", s / 4 + 1));
		CN3UIBase* pSlot  = pGroup ? pGroup->GetChildByID(fmt::format("item{}", s % 4 + 1)) : nullptr;
		FillSlot(pSlot, m_PageItems[s]);
	}
	if (CN3UIString* p = GetChildByID<CN3UIString>("txt_page"))
		p->SetString(fmt::format("{} / {}", m_iPage + 1, iPages));
	if (CN3UIString* p = GetChildByID<CN3UIString>("txt_cash"))
		p->SetString(fmt::format("{} KC", m_dwCash));
	if (CN3UIString* p = GetChildByID<CN3UIString>("txt_tl_balance"))
	{
		p->SetVisible(true);
		p->SetString(fmt::format("{} TL", m_dwTL));
	}
	if (m_bRequested && !m_bListed)
		if (CN3UIString* p = GetChildByID<CN3UIString>("txt_page"))
			p->SetString("...");
}

int CUIPowerUpStore2369::SlotOfSender(CN3UIBase* pSender) const
{
	// Gönderen düğme → ebeveyn zinciri: item{n} (yuva) → items{m} (grup)
	CN3UIBase* pSlot = pSender ? pSender->GetParent() : nullptr;
	if (pSlot == nullptr)
		return -1;
	CN3UIBase* pGroup = pSlot->GetParent();
	if (pGroup == nullptr)
		return -1;
	std::string szSlot = Lower(pSlot->m_szID), szGroup = Lower(pGroup->m_szID);
	if (szSlot.rfind("item", 0) != 0 || szGroup.rfind("items", 0) != 0 || szSlot.size() < 5 || szGroup.size() < 6)
		return -1;
	int i = std::atoi(szSlot.c_str() + 4), g = std::atoi(szGroup.c_str() + 5);
	if (i < 1 || i > 4 || g < 1 || g > 4)
		return -1;
	return (g - 1) * 4 + (i - 1);
}

bool CUIPowerUpStore2369::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr)
		return false;
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		std::string szID = Lower(pSender->m_szID);
		if (szID == "btn_next" || szID == "btn_previous")
		{
			m_iPage += (szID == "btn_next") ? 1 : -1;
			Refresh();
			return true;
		}
		if (szID.rfind("btn_tab", 0) == 0 && szID.size() == 8)
		{
			int t = szID[7] - '1';
			if (t >= 0 && t < TAB_COUNT && t < (int) m_Cats.size())
			{
				m_iTab  = t;
				m_iPage = 0;
				Refresh();
			}
			return true;
		}
		if (szID == "btn_purchase")
		{
			int s = SlotOfSender(pSender);
			if (s >= 0 && s < (int) m_PageItems.size() && m_PageItems[s] != nullptr)
				SendPurchase(m_PageItems[s]->id, 1);
			else
				CLogWriter::Write("PUS: satın al düğmesi yuvaya eşlenemedi ({})", pSender->m_szID);
			return true;
		}
	}
	return CUIGeneric2369::ReceiveMessage(pSender, dwMsg); // kapat/iptal düğmeleri
}
