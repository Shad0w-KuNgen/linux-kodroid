#include "StdAfx.h"
#include "UIClanWindow2369.h"
#include "GameBase.h"
#include "GameProcedure.h"
#include "GameProcMain.h"
#include "UIChat.h"
#include "APISocket.h"
#include "PlayerMySelf.h"
#include "PacketDef.h"
#include <N3Base/LogWriter.h>
#include <N3Base/N3UIString.h>
#include <N3Base/N3UIEdit.h>
#include <shared/packets.h>
#include <shared/ByteBuffer.h>
#include <cctype>

namespace
{
constexpr uint8_t KO2369_KNIGHTS_MEMBER_REQ   = 13;
constexpr uint8_t KO2369_KNIGHTS_UPDATENOTICE = 80;
std::string Low(std::string s) { for (char& c : s) c = (char) std::tolower((unsigned char) c); return s; }
const char* DutyName(int fame)
{
	switch (fame)
	{
		case 1: return "Lider";
		case 2: return "Yardimci";
		case 3: return "Subay";
		case 5: return "Aday";
		default: return "Uye";
	}
}
} // namespace

void CUIClanWindow2369::Open()
{
	CUIGeneric2369::Open();
	for (const char* g : {"group_memo", "group_notice", "group_purge"})
		if (CN3UIBase* p = GetChildByID(g))
			p->SetVisible(false);
	Refresh();
	RequestMembers();
}

void CUIClanWindow2369::RequestMembers()
{
	if (CGameProcedure::s_pSocket == nullptr)
		return;
	uint8_t byBuff[4];
	int iOffset = 0;
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_KNIGHTS_PROCESS);
	CAPISocket::MP_AddByte(byBuff, iOffset, KO2369_KNIGHTS_MEMBER_REQ);
	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

void CUIClanWindow2369::SendNotice(const std::string& szNotice)
{
	if (CGameProcedure::s_pSocket == nullptr || KO2369_KNIGHTS_UPDATENOTICE == 0 || szNotice.size() > 200)
		return;
	uint8_t byBuff[256];
	int iOffset = 0;
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_KNIGHTS_PROCESS);
	CAPISocket::MP_AddByte(byBuff, iOffset, KO2369_KNIGHTS_UPDATENOTICE);
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t) szNotice.size()); // DByte
	CAPISocket::MP_AddString(byBuff, iOffset, szNotice);
	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
	CLogWriter::Write("Klan duyurusu gönderildi ({} karakter)", szNotice.size());
}

bool CUIClanWindow2369::OnMemberList(Packet& pkt)
{
	// u8 sonuç | (1 ise) u16 boyut | u16 2 | u16 MAX | str16 duyuru | u16 sayı | üye × (str16 ad, u8 fame, u8 0, u8 seviye, u16 sınıf, u8 çevrimiçi, str16 not, u32 saat)
	auto remaining = [&](size_t n) { return pkt.size() >= pkt.rpos() + n; };
	if (!remaining(1))
		return false;
	uint8_t byResult = pkt.read<uint8_t>();
	if (byResult != 1)
	{
		CLogWriter::Write("Klan üye listesi: sonuç {} (2 = klanda değil, 7 = klan yok)", (int) byResult);
		m_Members.clear();
		Refresh();
		return true;
	}
	if (!remaining(6))
		return true;
	pkt.read<uint16_t>(); // boyut
	pkt.read<uint16_t>(); // 2
	m_iMax = pkt.read<uint16_t>();
	if (!pkt.readString(m_szNotice))
		return true;
	if (!remaining(2))
		return true;
	uint16_t wCount = pkt.read<uint16_t>();
	m_Members.clear();
	for (uint16_t i = 0; i < wCount && remaining(2); i++)
	{
		Member mb;
		if (!pkt.readString(mb.name) || !remaining(1 + 1 + 1 + 2 + 1 + 2))
			break;
		mb.fame   = pkt.read<uint8_t>();
		pkt.read<uint8_t>();
		mb.level  = pkt.read<uint8_t>();
		mb.cls    = pkt.read<uint16_t>();
		mb.online = pkt.read<uint8_t>();
		if (!pkt.readString(mb.memo) || !remaining(4))
			break;
		mb.hours = pkt.read<uint32_t>();
		m_Members.push_back(mb);
	}
	CLogWriter::Write("Klan üye listesi: {} üye (en çok {}), duyuru \"{}\"", m_Members.size(), m_iMax, m_szNotice);
	m_iPage = 0;
	Refresh();
	return true;
}

void CUIClanWindow2369::Refresh()
{
	if (GetChildren().empty() || CGameBase::s_pPlayer == nullptr)
		return;
	const __InfoPlayerMySelf& info = CGameBase::s_pPlayer->m_InfoExt;
	if (CN3UIString* p = GetChildByID<CN3UIString>("text_clan_name")) p->SetString(info.szKnights);
	if (CN3UIString* p = GetChildByID<CN3UIString>("text_clan_grade")) p->SetString(fmt::format("{}", info.iKnightsGrade));
	if (CN3UIString* p = GetChildByID<CN3UIString>("text_clan_duty")) p->SetString(DutyName((int) info.eKnightsDuty));
	if (CN3UIString* p = GetChildByID<CN3UIString>("text_notice")) p->SetString(m_szNotice);
	if (CN3UIString* p = GetChildByID<CN3UIString>("text_clan_member_count")) p->SetString(fmt::format("{}", m_iMax));
	if (CN3UIString* p = GetChildByID<CN3UIString>("text_current_member_count")) p->SetString(fmt::format("{}", m_Members.size()));
	int iPages = std::max(1, ((int) m_Members.size() + ROWS - 1) / ROWS);
	m_iPage    = std::clamp(m_iPage, 0, iPages - 1);
	for (int r = 0; r < ROWS; r++)
	{
		CN3UIBase* pRow = GetChildByID(fmt::format("grp_member_list_{}", r));
		if (pRow == nullptr)
			continue;
		int idx = m_iPage * ROWS + r;
		bool bHas = idx < (int) m_Members.size();
		pRow->SetVisible(bHas);
		if (CN3UIBase* pSel = GetChildByID(fmt::format("btn_selected{}", r)))
			pSel->SetVisible(bHas && idx == m_iSelected);
		if (!bHas)
			continue;
		const Member& mb = m_Members[idx];
		if (CN3UIString* p = pRow->GetChildByID<CN3UIString>("text_character")) p->SetString(mb.name);
		if (CN3UIString* p = pRow->GetChildByID<CN3UIString>("text_level")) p->SetString(fmt::format("{}", mb.level));
		if (CN3UIString* p = pRow->GetChildByID<CN3UIString>("text_duty")) p->SetString(DutyName(mb.fame));
		if (CN3UIString* p = pRow->GetChildByID<CN3UIString>("text_memo")) p->SetString(mb.memo);
		if (CN3UIString* p = pRow->GetChildByID<CN3UIString>("text_last_access"))
			p->SetString(mb.online ? "Cevrimici" : fmt::format("{} sa", mb.hours));
		for (int c = 0; c < 10; c++)
			if (CN3UIBase* pImg = pRow->GetChildByID(fmt::format("img_class_{}", c)))
				pImg->SetVisible(c == (mb.cls % 100) / 10 % 10); // sınıf simgesi (kaba eşleme)
	}
}

bool CUIClanWindow2369::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr)
		return false;
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		std::string id = Low(pSender->m_szID);
		CGameProcMain* pMain = CGameProcedure::s_pProcMain;
		const Member* pSel = (m_iSelected >= 0 && m_iSelected < (int) m_Members.size()) ? &m_Members[m_iSelected] : nullptr;
		if (id.rfind("btn_selected", 0) == 0 && id.size() == 13)
		{
			int r = id[12] - '0';
			m_iSelected = m_iPage * ROWS + r;
			Refresh();
			return true;
		}
		if (id == "btn_clan_refresh") { RequestMembers(); return true; }
		if (id == "btn_clan_whisper" && pSel && pMain && pMain->m_pUIChatDlg)
		{
			if (!pMain->m_pUIChatDlg->IsVisible())
				pMain->CommandToggleUIChat();
			pMain->m_pUIChatDlg->SetString("@" + pSel->name + " ");
			pMain->m_pUIChatDlg->SetFocus();
			return true;
		}
		if (id == "btn_clan_party" && pSel && pMain) { pMain->MsgSend_PartyOrForceCreate(pSel->name); return true; }
		if (id == "btn_clan_remove" && pSel && pMain) { pMain->MsgSend_KnightsLeave(pSel->name); RequestMembers(); return true; }
		if (id == "btn_clan_appoint" && pSel && pMain) { pMain->MsgSend_KnightsAppointViceChief(pSel->name); RequestMembers(); return true; }
		if (id == "btn_clan_admit" && pMain)
		{
			// Seçili (hedef) oyuncuyu klana davet et (1298 akışı: hedef kimliği)
			int iTarget = CGameBase::s_pPlayer ? CGameBase::s_pPlayer->m_iIDTarget : -1;
			if (iTarget >= 0)
				pMain->MsgSend_KnightsJoin(iTarget);
			else
				CLogWriter::Write("Klan daveti: önce bir oyuncu seçin");
			return true;
		}
		if (id == "btn_clan_notice")
		{
			if (CN3UIBase* g = GetChildByID("group_notice"))
			{
				g->SetVisible(true);
				if (CN3UIEdit* e = g->GetChildByID<CN3UIEdit>("edit_notice"))
					e->SetString(m_szNotice);
			}
			return true;
		}
		if (id == "btn_notice_ok" || id == "btn_notice_cancel")
		{
			if (CN3UIBase* g = GetChildByID("group_notice"))
			{
				if (id == "btn_notice_ok")
					if (CN3UIEdit* e = g->GetChildByID<CN3UIEdit>("edit_notice"))
						SendNotice(e->GetString());
				g->SetVisible(false);
			}
			return true;
		}
		if (id == "btn_memo_cancel" || id == "btn_purge_cancle")
		{
			if (CN3UIBase* g = GetChildByID(id == "btn_memo_cancel" ? "group_memo" : "group_purge"))
				g->SetVisible(false);
			return true;
		}
		if (id == "btn_clan_memo") { if (CN3UIBase* g = GetChildByID("group_memo")) g->SetVisible(true); return true; }
		if (id == "btn_bank" || id == "btn_clan_ladder_point" || id == "btn_clan_transfer" || id == "btn_memo_ok" || id == "btn_purge_ok")
		{
			CLogWriter::Write("Klan penceresi: \"{}\" sunucuya bağlı değil (XSafe CLANBANK / aktarım)", pSender->m_szID);
			if (CN3UIBase* g = GetChildByID("group_memo")) g->SetVisible(false);
			if (CN3UIBase* g = GetChildByID("group_purge")) g->SetVisible(false);
			return true;
		}
	}
	return CUIGeneric2369::ReceiveMessage(pSender, dwMsg);
}
