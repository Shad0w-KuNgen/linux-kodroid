#include "StdAfx.h"

#if !defined(LOGIN_SCENE_VERSION) || LOGIN_SCENE_VERSION == 1298
#include "UILogin_1298.h"
#include "GameProcLogIn_1298.h"
#include "UIMessageBoxManager.h"
#include "text_resources.h"

#include <N3Base/N3UIEdit.h>
#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIString.h>
#include <N3Base/N3UIList.h>
#include <N3Base/N3UIImage.h>

#include <algorithm>
#include <cctype>
#include <initializer_list>
#include <shellapi.h>

CUILogIn_1298::CUILogIn_1298()
{
	m_pEdit_id             = nullptr;
	m_pEdit_pw             = nullptr;

	m_pBtn_LogIn           = nullptr;
	m_pBtn_Connect         = nullptr;
	m_pBtn_Cancel          = nullptr;
	m_pBtn_Option          = nullptr;
	m_pBtn_Join            = nullptr;

	m_pGroup_Notice_1      = nullptr;
	m_pGroup_Notice_2      = nullptr;
	m_pGroup_Notice_3      = nullptr;

	m_pText_Notice1_Name_1 = nullptr;
	m_pText_Notice1_Text_1 = nullptr;

	m_pText_Notice2_Name_1 = nullptr;
	m_pText_Notice2_Text_1 = nullptr;
	m_pText_Notice2_Name_2 = nullptr;
	m_pText_Notice2_Text_2 = nullptr;

	m_pText_Notice3_Name_1 = nullptr;
	m_pText_Notice3_Text_1 = nullptr;
	m_pText_Notice3_Name_2 = nullptr;
	m_pText_Notice3_Text_2 = nullptr;
	m_pText_Notice3_Name_3 = nullptr;
	m_pText_Notice3_Text_3 = nullptr;

	m_pBtn_NoticeOK_1      = nullptr;
	m_pBtn_NoticeOK_2      = nullptr;
	m_pBtn_NoticeOK_3      = nullptr;

	m_pGroup_ServerList    = nullptr;
	m_pGroup_LogIn         = nullptr;

	m_pStr_Premium         = nullptr;

	m_iSelectedServerIndex = -1;

	for (int i = 0; i < MAX_SERVERS; i++)
	{
		m_pServer_Group[i] = nullptr;
		m_pArrow_Group[i]  = nullptr;
		m_pList_Group[i]   = nullptr;
	}

	m_bIsNewsVisible = false;
	m_bLogIn         = false;
}

CUILogIn_1298::~CUILogIn_1298()
{
}

static std::string LowerID(const CN3UIBase* p)
{
	std::string sz = p ? p->m_szID : std::string();
	for (char& c : sz)
		c = (char) tolower((unsigned char) c);
	return sz;
}

static bool IDHasAny(const std::string& szLower, std::initializer_list<const char*> parts)
{
	for (const char* sz : parts)
		if (szLower.find(sz) != std::string::npos)
			return true;
	return false;
}

static void CollectDescendants(CN3UIBase* p, std::vector<CN3UIBase*>& out)
{
	for (CN3UIBase* pChild : p->GetChildren())
	{
		if (pChild == nullptr)
			continue;
		out.push_back(pChild);
		CollectDescendants(pChild, out);
	}
}

static bool IsDescendantOf(const CN3UIBase* p, const CN3UIBase* pAncestor)
{
	for (const CN3UIBase* q = p ? p->GetParent() : nullptr; q != nullptr; q = q->GetParent())
		if (q == pAncestor)
			return true;
	return false;
}

static void DumpUITree(const CN3UIBase* p, int depth, int maxDepth, std::string& out)
{
	for (const CN3UIBase* pChild : p->GetChildren())
	{
		if (pChild == nullptr)
			continue;
		RECT rc = pChild->GetRegion();
		out += std::string(depth * 2, ' ') + std::to_string((int) pChild->UIType()) + ":" + pChild->m_szID + "["
			   + std::to_string(rc.left) + "," + std::to_string(rc.top) + "," + std::to_string(rc.right) + ","
			   + std::to_string(rc.bottom) + "]";
		if (pChild->UIType() == UI_TYPE_STRING)
			out += "{" + static_cast<const CN3UIString*>(pChild)->GetString() + "}";
		out += "\n";
		if (depth + 1 < maxDepth)
			DumpUITree(pChild, depth + 1, maxDepth, out);
	}
}

bool CUILogIn_1298::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (pSender == nullptr)
		return false;

	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_LogIn && m_pEdit_id != nullptr && m_pEdit_pw != nullptr)
		{
			CGameProcedure::s_pProcLogIn->MsgSend_AccountLogIn(LIC_KNIGHTONLINE);
			return true;
		}
		else if (pSender == m_pBtn_Connect)
		{
			CGameProcedure::s_pProcLogIn->ConnectToGameServer(); // 고른 게임 서버에 접속
			return true;
		}
		else if (pSender == m_pBtn_Cancel)
		{
			PostQuitMessage(0);            // 종료...
			return true;
		}
		else if (pSender == m_pBtn_Option) // 옵션..
		{
			std::string szMsg = fmt::format_text_resource(IDS_CONFIRM_EXECUTE_OPTION);
			CGameProcedure::MessageBoxPost(szMsg, "", MB_YESNO, BEHAVIOR_EXECUTE_OPTION);
			return true;
		}
		else if (pSender == m_pBtn_Join)
		{
			if (!CGameProcedure::s_pProcLogIn->m_szRegistrationSite.empty())
			{
				ShellExecute(nullptr, "open", CGameProcedure::s_pProcLogIn->m_szRegistrationSite.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
			}

			return true;
		}
		else if (pSender == m_pBtn_NoticeOK_1 || pSender == m_pBtn_NoticeOK_2 || pSender == m_pBtn_NoticeOK_3)
		{
			OpenServerList();
			return true;
		}
		else if (m_pGroup_ServerList != nullptr && IsDescendantOf(pSender, m_pGroup_ServerList))
		{
			// 2369/ISTIRAP: eşlenmemiş düğme; ada göre bağlan/iptal, her durumda Log.txt
			std::string sz = LowerID(pSender);
			CLogWriter::Write("CUILogIn_1298: sunucu listesinde düğme '{}' tıklandı", pSender->m_szID);
			if (IDHasAny(sz, { "connect", "ok", "enter", "start", "login", "select" }))
			{
				CGameProcedure::s_pProcLogIn->ConnectToGameServer();
				return true;
			}
		}
		else if (m_pGroup_LogIn != nullptr && IsDescendantOf(pSender, m_pGroup_LogIn))
		{
			CLogWriter::Write("CUILogIn_1298: giriş grubunda eşlenmemiş düğme '{}' tıklandı", pSender->m_szID);
		}
	}
	else if (dwMsg == UIMSG_LIST_SELCHANGE || dwMsg == UIMSG_LIST_DBLCLK)
	{
		if (m_pListCtrl_Servers != nullptr && pSender == m_pListCtrl_Servers)
		{
			SelectServer(m_pListCtrl_Servers->GetCurSel());
			if (dwMsg == UIMSG_LIST_DBLCLK)
				CGameProcedure::s_pProcLogIn->ConnectToGameServer();
			return true;
		}
	}
	// double click on string
	else if (UIMSG_STRING_LDCLICK == dwMsg)
	{
		for (int i = 0; i < MAX_SERVERS; i++)
		{
			if (m_pList_Group[i] == nullptr)
				continue;

			if (pSender == m_pList_Group[i])
			{
				SelectServer(i);
				CGameProcedure::s_pProcLogIn->ConnectToGameServer();
				return true;
			}
		}
	}
	// change color on left click
	else if (UIMSG_STRING_LCLICK == dwMsg)
	{
		for (int i = 0; i < MAX_SERVERS; i++)
		{
			if (m_pList_Group[i] != nullptr && pSender == m_pList_Group[i])
			{
				SelectServer(i);
				return true;
			}
		}
	}
	else if (dwMsg == UIMSG_EDIT_RETURN)
	{
		// TEMP(srmeier): there is a weird issue where the key inputs aren't going
		// through CGameProcedure::ProcessUIKeyInput() so CUILogIn_1298::OnKeyPress() isn't
		// being called...
		if (!m_bLogIn && m_pEdit_id && m_pEdit_pw)
		{
			// Mobil: sanal klavyede Tab yok; ID kutusunda Enter/İleri şifre kutusuna geçer,
			// şifre boşken şifre kutusundan Enter ID'ye döner.
			if (pSender == m_pEdit_id && m_pEdit_pw->GetString().empty())
			{
				m_pEdit_pw->SetFocus();
				return true;
			}
			if (pSender == m_pEdit_pw && m_pEdit_id->GetString().empty())
			{
				m_pEdit_id->SetFocus();
				return true;
			}
			CN3UIBase* pMsgBox = CGameProcedure::s_pMsgBoxMgr->GetFocusMsgBox();
			if (!(pMsgBox && pMsgBox->IsVisible()))
				CGameProcedure::s_pProcLogIn->MsgSend_AccountLogIn(LIC_KNIGHTONLINE);
		}
		else
		{
			return ReceiveMessage(m_pBtn_Connect, UIMSG_BUTTON_CLICK);
		}
	}

	return false;
}

// Birden fazla ID adayından ilk bulunan çocuk (büyük/küçük harf duyarsız; 1.298 ve 2369/ISTIRAP adlandırması)
static CN3UIBase* FindChildAlias(const CN3UIBase* pParent, std::initializer_list<const char*> ids)
{
	for (const char* szID : ids)
	{
		CN3UIBase* p = pParent->GetChildByID(szID);
		if (p != nullptr)
			return p;
	}
	return nullptr;
}

template <typename T>
static T* FindChildAlias(const CN3UIBase* pParent, std::initializer_list<const char*> ids)
{
	for (const char* szID : ids)
	{
		T* p = pParent->GetChildByID<T>(szID);
		if (p != nullptr)
			return p;
	}
	return nullptr;
}

void CUILogIn_1298::BindServerListHeuristic()
{
	if (m_pGroup_ServerList == nullptr)
		return;

	std::string szTree;
	DumpUITree(m_pGroup_ServerList, 1, 4, szTree);
	CLogWriter::Write("CUILogIn_1298: sunucu listesi grubu '{}' alt ağacı (tür:ID[bölge]{{metin}}):\n{}", m_pGroup_ServerList->m_szID, szTree);

	std::vector<CN3UIBase*> all;
	CollectDescendants(m_pGroup_ServerList, all);

	// Bağlan düğmesi
	if (m_pBtn_Connect == nullptr)
	{
		CN3UIButton* pFirst = nullptr;
		for (CN3UIBase* p : all)
		{
			if (p->UIType() != UI_TYPE_BUTTON)
				continue;
			std::string sz = LowerID(p);
			if (IDHasAny(sz, { "connect", "ok", "enter", "start", "login", "select" }))
			{
				m_pBtn_Connect = static_cast<CN3UIButton*>(p);
				break;
			}
			if (pFirst == nullptr && !IDHasAny(sz, { "cancel", "exit", "close", "back", "arrow", "scroll" }))
				pFirst = static_cast<CN3UIButton*>(p);
		}
		if (m_pBtn_Connect == nullptr)
			m_pBtn_Connect = pFirst;
	}

	if (m_pServer_Group[0] == nullptr)
	{
		// a) CN3UIList denetimi
		for (CN3UIBase* p : all)
		{
			if (p->UIType() == UI_TYPE_LIST)
			{
				m_pListCtrl_Servers = static_cast<CN3UIList*>(p);
				break;
			}
		}

		// b) "server<N>" grupları → ilk string çocuğu
		std::vector<std::pair<int, CN3UIBase*>> groups;
		for (CN3UIBase* p : all)
		{
			if (p->UIType() != UI_TYPE_BASE)
				continue;
			std::string sz = LowerID(p);
			size_t pos     = sz.find("server");
			if (pos == std::string::npos)
				continue;
			size_t d = sz.find_first_of("0123456789", pos);
			if (d == std::string::npos)
				continue;
			groups.emplace_back(atoi(sz.c_str() + d), p);
		}
		std::sort(groups.begin(), groups.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

		if (!groups.empty())
		{
			int i = 0;
			for (auto& [num, pGroup] : groups)
			{
				if (i >= MAX_SERVERS)
					break;
				std::vector<CN3UIBase*> kids;
				CollectDescendants(pGroup, kids);
				CN3UIString* pStr = nullptr;
				for (CN3UIBase* k : kids)
					if (k->UIType() == UI_TYPE_STRING && (pStr == nullptr || IDHasAny(LowerID(k), { "list", "server", "name" })))
						pStr = static_cast<CN3UIString*>(k);
				if (pStr == nullptr)
					continue;
				m_pServer_Group[i] = pGroup;
				m_pList_Group[i]   = pStr;
				i++;
			}
		}
		else if (m_pListCtrl_Servers == nullptr)
		{
			// c) string satırları (ID'de server/list/name geçenler, yoksa hepsi), yukarıdan aşağıya
			std::vector<CN3UIString*> strs, named;
			for (CN3UIBase* p : all)
			{
				if (p->UIType() != UI_TYPE_STRING)
					continue;
				strs.push_back(static_cast<CN3UIString*>(p));
				if (IDHasAny(LowerID(p), { "server", "list", "name" }) && !IDHasAny(LowerID(p), { "title", "select" }))
					named.push_back(static_cast<CN3UIString*>(p));
			}
			std::vector<CN3UIString*>& use = named.empty() ? strs : named;
			std::sort(use.begin(), use.end(), [](CN3UIString* a, CN3UIString* b) {
				RECT ra = a->GetRegion(), rb = b->GetRegion();
				return ra.top != rb.top ? ra.top < rb.top : ra.left < rb.left;
			});
			for (size_t i = 0; i < use.size() && i < MAX_SERVERS; i++)
			{
				m_pList_Group[i]   = use[i];
				m_pServer_Group[i] = use[i]; // ortak üst grup gizlenmesin: satırın kendisi gösterilip gizlenir
			}
		}
	}

	std::string szSel;
	for (int i = 0; i < MAX_SERVERS; i++)
		if (m_pList_Group[i] != nullptr)
			szSel += m_pList_Group[i]->m_szID + " ";
	CLogWriter::Write("CUILogIn_1298: sezgisel eşleme: connect={} liste={} satırlar: {}", m_pBtn_Connect ? m_pBtn_Connect->m_szID : "YOK",
		m_pListCtrl_Servers ? m_pListCtrl_Servers->m_szID : "yok", szSel.empty() ? "YOK" : szSel);
}

bool CUILogIn_1298::Load(File& file)
{
	if (!CN3UIBase::Load(file))
		return false;

	// ID araması zaten büyük/küçük harf duyarsız (GetChildByID → strncasecmp); 2369/ISTIRAP adları için takma adlar
	N3_VERIFY_UI_COMPONENT(m_pGroup_LogIn, FindChildAlias(this, { "Group_LogIn", "Group_Login", "Group_Login_01" }));

	if (m_pGroup_LogIn != nullptr)
	{
		N3_VERIFY_UI_COMPONENT(m_pBtn_LogIn, FindChildAlias<CN3UIButton>(m_pGroup_LogIn, { "btn_ok", "btn_login", "btn_connect", "btn_enter" }));
		N3_VERIFY_UI_COMPONENT(m_pBtn_Cancel, FindChildAlias<CN3UIButton>(m_pGroup_LogIn, { "btn_cancel", "btn_exit", "btn_close" }));
		N3_VERIFY_UI_COMPONENT(m_pBtn_Option, FindChildAlias<CN3UIButton>(m_pGroup_LogIn, { "btn_option", "btn_options" }));
		N3_VERIFY_UI_COMPONENT(m_pBtn_Join, FindChildAlias<CN3UIButton>(m_pGroup_LogIn, { "btn_homepage", "btn_join", "btn_register" }));

		N3_VERIFY_UI_COMPONENT(m_pEdit_id, FindChildAlias<CN3UIEdit>(m_pGroup_LogIn, { "Edit_ID", "edit_account", "edit_user", "edit_login" }));
		N3_VERIFY_UI_COMPONENT(m_pEdit_pw, FindChildAlias<CN3UIEdit>(m_pGroup_LogIn, { "Edit_PW", "edit_password", "edit_pass", "edit_pwd" }));

		// Takma adlar da tutmazsa türe/alt dizgeye göre sezgisel seçim: ilk "id/account" edit'i, ilk "pw/pass" edit'i,
		// "ok/login/connect" düğmesi; her durumda grup çocukları Log.txt'ye (tür, ID, uzunluk)
		if (m_pEdit_id == nullptr || m_pEdit_pw == nullptr || m_pBtn_LogIn == nullptr)
		{
			std::string szKids;
			for (CN3UIBase* pChild : m_pGroup_LogIn->GetChildren())
			{
				if (pChild == nullptr)
					continue;
				szKids += std::to_string((int) pChild->UIType()) + ":" + pChild->m_szID + "(" + std::to_string(pChild->m_szID.size()) + ") ";
				std::string szLower = pChild->m_szID;
				for (char& c : szLower)
					c = (char) tolower((unsigned char) c);
				if (pChild->UIType() == UI_TYPE_EDIT)
				{
					bool bPw = szLower.find("pw") != std::string::npos || szLower.find("pass") != std::string::npos;
					if (bPw && m_pEdit_pw == nullptr)
						m_pEdit_pw = static_cast<CN3UIEdit*>(pChild);
					else if (!bPw && m_pEdit_id == nullptr)
						m_pEdit_id = static_cast<CN3UIEdit*>(pChild);
				}
				else if (pChild->UIType() == UI_TYPE_BUTTON && m_pBtn_LogIn == nullptr)
				{
					if (szLower.find("ok") != std::string::npos || szLower.find("login") != std::string::npos
						|| szLower.find("connect") != std::string::npos || szLower.find("enter") != std::string::npos)
						m_pBtn_LogIn = static_cast<CN3UIButton*>(pChild);
				}
			}
			CLogWriter::Write("CUILogIn_1298::Load: {} '{}' çocukları (tür:ID(uzunluk)): {}| seçim: id={} pw={} ok={}", m_szFileName,
				m_pGroup_LogIn->m_szID, szKids, m_pEdit_id ? m_pEdit_id->m_szID : "yok", m_pEdit_pw ? m_pEdit_pw->m_szID : "yok",
				m_pBtn_LogIn ? m_pBtn_LogIn->m_szID : "yok");
		}

		m_pGroup_LogIn->SetVisible(true);
	}

	// get notice boxes
	N3_VERIFY_UI_COMPONENT(m_pGroup_Notice_1, GetChildByID("Group_Notice_1"));
	N3_VERIFY_UI_COMPONENT(m_pGroup_Notice_2, GetChildByID("Group_Notice_2"));
	N3_VERIFY_UI_COMPONENT(m_pGroup_Notice_3, GetChildByID("Group_Notice_3"));

	if (m_pGroup_Notice_1 != nullptr)
	{
		N3_VERIFY_UI_COMPONENT(m_pBtn_NoticeOK_1, m_pGroup_Notice_1->GetChildByID<CN3UIButton>("btn_ok"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice1_Name_1, m_pGroup_Notice_1->GetChildByID<CN3UIString>("text_notice_name_01"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice1_Text_1, m_pGroup_Notice_1->GetChildByID<CN3UIString>("text_notice_01"));

		m_pGroup_Notice_1->SetVisible(false);
	}

	if (m_pGroup_Notice_2 != nullptr)
	{
		N3_VERIFY_UI_COMPONENT(m_pBtn_NoticeOK_2, m_pGroup_Notice_2->GetChildByID<CN3UIButton>("btn_ok"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice2_Name_1, m_pGroup_Notice_2->GetChildByID<CN3UIString>("text_notice_name_01"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice2_Text_1, m_pGroup_Notice_2->GetChildByID<CN3UIString>("text_notice_01"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice2_Name_2, m_pGroup_Notice_2->GetChildByID<CN3UIString>("text_notice_name_02"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice2_Text_2, m_pGroup_Notice_2->GetChildByID<CN3UIString>("text_notice_02"));

		m_pGroup_Notice_2->SetVisible(false);
	}

	if (m_pGroup_Notice_3 != nullptr)
	{
		N3_VERIFY_UI_COMPONENT(m_pBtn_NoticeOK_3, m_pGroup_Notice_3->GetChildByID<CN3UIButton>("btn_ok"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice3_Name_1, m_pGroup_Notice_3->GetChildByID<CN3UIString>("text_notice_name_01"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice3_Text_1, m_pGroup_Notice_3->GetChildByID<CN3UIString>("text_notice_01"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice3_Name_2, m_pGroup_Notice_3->GetChildByID<CN3UIString>("text_notice_name_02"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice3_Text_2, m_pGroup_Notice_3->GetChildByID<CN3UIString>("text_notice_02"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice3_Name_3, m_pGroup_Notice_3->GetChildByID<CN3UIString>("text_notice_name_03"));
		N3_VERIFY_UI_COMPONENT(m_pText_Notice3_Text_3, m_pGroup_Notice_3->GetChildByID<CN3UIString>("text_notice_03"));

		m_pGroup_Notice_3->SetVisible(false);
	}

	N3_VERIFY_UI_COMPONENT(m_pStr_Premium, GetChildByID<CN3UIString>("premium"));

	if (m_pStr_Premium != nullptr)
		m_pStr_Premium->SetVisible(false);

	N3_VERIFY_UI_COMPONENT(m_pGroup_ServerList, FindChildAlias(this, { "Group_ServerList_01", "Group_ServerList", "Group_Server" }));

	if (m_pGroup_ServerList != nullptr)
		m_pGroup_ServerList->SetVisible(false);

	// get List_Server (structure: Group_ServerList_01 -> server_20 -> List_Server )
	// 2369/ISTIRAP giriş arayüzünde bu gruplar olmayabilir: null denetimi (çökme yerine Log.txt)
	for (int i = 0; i < MAX_SERVERS; i++)
	{
		std::string szID = "server_" + std::to_string(i + 1);
		if (m_pGroup_ServerList != nullptr)
		{
			N3_VERIFY_UI_COMPONENT(m_pServer_Group[i], m_pGroup_ServerList->GetChildByID(szID));

			szID = "img_arrow" + std::to_string(i + 1);
			N3_VERIFY_UI_COMPONENT(m_pArrow_Group[i], m_pGroup_ServerList->GetChildByID(szID));
		}

		if (m_pServer_Group[i] != nullptr)
			N3_VERIFY_UI_COMPONENT(m_pList_Group[i], m_pServer_Group[i]->GetChildByID<CN3UIString>("List_Server"));
	}

	if (m_pGroup_ServerList != nullptr)
		N3_VERIFY_UI_COMPONENT(m_pBtn_Connect, FindChildAlias<CN3UIButton>(m_pGroup_ServerList, { "Btn_Connect", "btn_ok", "btn_login", "btn_enter" }));

	if (m_pGroup_ServerList != nullptr && (m_pServer_Group[0] == nullptr || m_pBtn_Connect == nullptr))
		BindServerListHeuristic();

	if (m_pGroup_LogIn == nullptr || m_pGroup_ServerList == nullptr)
	{
		std::string szIDs;
		for (CN3UIBase* pChild : m_Children)
			szIDs += (pChild ? pChild->m_szID + "(" + std::to_string(pChild->m_szID.size()) + ")" : std::string("?")) + " ";
		CLogWriter::Write("CUILogIn_1298::Load: Group_LogIn={} Group_ServerList={} ({}); çocuklar (ID(uzunluk)): {}",
			m_pGroup_LogIn ? "var" : "YOK", m_pGroup_ServerList ? "var" : "YOK", m_szFileName, szIDs);
	}

	return true;
}

void CUILogIn_1298::PositionGroups()
{
	if (m_pGroup_LogIn != nullptr)
		m_pGroup_LogIn->SetPosCenter();

	if (m_pGroup_ServerList != nullptr)
		m_pGroup_ServerList->SetPosCenter();

	if (m_pGroup_Notice_1 != nullptr)
		m_pGroup_Notice_1->SetPosCenter();

	if (m_pGroup_Notice_2 != nullptr)
		m_pGroup_Notice_2->SetPosCenter();

	if (m_pGroup_Notice_3 != nullptr)
		m_pGroup_Notice_3->SetPosCenter();
}

void CUILogIn_1298::AccountIDGet(std::string& szID)
{
	if (m_pEdit_id != nullptr)
		szID = m_pEdit_id->GetString();
	else
		szID.clear();
}

void CUILogIn_1298::AccountPWGet(std::string& szPW)
{
	if (m_pEdit_pw != nullptr)
		szPW = m_pEdit_pw->GetString();
	else
		szPW.clear();
}

void CUILogIn_1298::ConnectButtonSetEnable(bool bEnable)
{
	eUI_STATE eState1 = (bEnable ? UI_STATE_BUTTON_NORMAL : UI_STATE_BUTTON_DISABLE);

	if (m_pBtn_Connect != nullptr)
		m_pBtn_Connect->SetState(eState1);
}

void CUILogIn_1298::FocusToID()
{
	if (m_pEdit_id != nullptr)
		m_pEdit_id->SetFocus();
}

void CUILogIn_1298::FocusCircular()
{
	if (m_pEdit_id == nullptr || m_pEdit_pw == nullptr)
		return;

	if (m_pEdit_id->HaveFocus())
		m_pEdit_pw->SetFocus();
	else
		m_pEdit_id->SetFocus();
}

void CUILogIn_1298::InitEditControls()
{
	if (m_pEdit_id != nullptr)
	{
		m_pEdit_id->SetString("");
		m_pEdit_id->SetFocus();
	}

	if (m_pEdit_pw != nullptr)
		m_pEdit_pw->SetString("");
}

bool CUILogIn_1298::ServerInfoAdd(const __GameServerInfo& GSI)
{
	m_ListServerInfos.push_back(GSI);
	return true;
}

bool CUILogIn_1298::ServerInfoGet(int iIndex, __GameServerInfo& GSI)
{
	if (iIndex < 0 || iIndex >= static_cast<int>(m_ListServerInfos.size()))
		return false;

	GSI = m_ListServerInfos[iIndex];
	return true;
}

bool CUILogIn_1298::ServerInfoGetCur(__GameServerInfo& GSI)
{
	GSI.Init();

	return ServerInfoGet(m_iSelectedServerIndex, GSI);
}

void CUILogIn_1298::ServerInfoUpdate()
{
	if (m_ListServerInfos.empty())
		return;

	// sort(m_ListServerInfos.begin(), m_ListServerInfos.end(), not2(__GameServerInfo()));

	if (m_pListCtrl_Servers != nullptr)
	{
		m_pListCtrl_Servers->ResetContent();
		for (const __GameServerInfo& GSI : m_ListServerInfos)
			m_pListCtrl_Servers->AddString(GSI.szName);
	}

	// show ui of existing servers
	constexpr int NumUserForLine = 3000 / 12;
	int iNumLines                = 1;

	for (size_t i = 0; i < m_ListServerInfos.size(); i++)
	{
		if (m_pServer_Group[i] == nullptr)
			continue;

		if (m_pList_Group[i] != nullptr)
			m_pList_Group[i]->SetString(m_ListServerInfos[i].szName);

		m_pServer_Group[i]->SetVisible(true);

		if (m_pArrow_Group[i] != nullptr)
			m_pArrow_Group[i]->SetVisible(true);

		// hide number of lines with respect to user number
		iNumLines = m_ListServerInfos[i].iConcurrentUserCount / NumUserForLine;

		if (iNumLines < 1)  // minimum 1 lines
			iNumLines = 1;

		if (iNumLines > 12) // uif file has max 12 lines
			iNumLines = 12;

		// ids of lines set as 1,2,3 ... 12 in .uif file
		for (int j = iNumLines + 1; j <= 12; j++)
		{
			// TODO: Pre-load this.
			std::string szID  = std::to_string(j);

			CN3UIBase* pChild = m_pServer_Group[i]->GetChildByID(szID);
			if (pChild != nullptr)
				pChild->SetVisible(false);
		}
	}

	// hide ui of extra servers
	for (size_t i = m_ListServerInfos.size(); i < MAX_SERVERS; i++)
	{
		if (m_pServer_Group[i] != nullptr)
			m_pServer_Group[i]->SetVisible(false);

		if (m_pArrow_Group[i] != nullptr)
			m_pArrow_Group[i]->SetVisible(false);
	}

	// TODO: Show Premium info if user have premium
}

void CUILogIn_1298::AddNews(const std::string& strNews)
{
	std::vector<std::string> titles, messages;

	titles.reserve(MAX_NEWS_COUNT);
	messages.reserve(MAX_NEWS_COUNT);

	size_t searchPos = 0;

	std::string_view messageStartView(NEWS_MESSAGE_START, sizeof(NEWS_MESSAGE_START));
	std::string_view messageEndView(NEWS_MESSAGE_END, sizeof(NEWS_MESSAGE_END));

	// NOTE: The official parsing for this is extremely simple.
	// It really doesn't care about the format it uses; it basically
	// just looks for the first and last #, and ignores anything
	// until the next # is found for the next box, and strips out
	// \r, \n as it goes.
	// Since this means that it ends up including characters it shouldn't,
	// e.g. null-terminators (which happen to not get rendered), we'll just
	// be a touch smarter about this and follow the basic format.
	while (titles.size() < MAX_NEWS_COUNT)
	{
		const size_t titlePos      = searchPos;

		// Find the start of the message
		size_t startOfMessageBlock = strNews.find(messageStartView, searchPos);
		if (startOfMessageBlock == std::string::npos)
			break;

		// The title precedes the message.
		// It's not directly surrounded by anything of its own.
		std::string title             = strNews.substr(titlePos, startOfMessageBlock - titlePos);

		size_t startOfMessage         = startOfMessageBlock + sizeof(NEWS_MESSAGE_START);

		size_t startOfEndMessageBlock = strNews.find(messageEndView, startOfMessage);
		if (startOfEndMessageBlock == std::string::npos)
			break;

		std::string message = strNews.substr(startOfMessage, startOfEndMessageBlock - startOfMessage);

		titles.push_back(std::move(title));
		messages.push_back(std::move(message));

		// jump to next block
		searchPos = startOfEndMessageBlock + sizeof(NEWS_MESSAGE_END);
	}

	// No news, skip to server list
	if (titles.empty())
	{
		m_bIsNewsVisible = false;
		OpenServerList();
	}
	else if (titles.size() == 1)
	{
		if (m_pText_Notice1_Name_1 != nullptr)
			m_pText_Notice1_Name_1->SetString(titles[0]);

		if (m_pText_Notice1_Text_1 != nullptr)
			m_pText_Notice1_Text_1->SetString(messages[0]);

		if (m_pGroup_Notice_1 != nullptr)
			m_pGroup_Notice_1->SetVisible(true);
	}
	else if (titles.size() == 2)
	{
		if (m_pText_Notice2_Name_1 != nullptr)
			m_pText_Notice2_Name_1->SetString(titles[0]);

		if (m_pText_Notice2_Text_1 != nullptr)
			m_pText_Notice2_Text_1->SetString(messages[0]);

		if (m_pText_Notice2_Name_2 != nullptr)
			m_pText_Notice2_Name_2->SetString(titles[1]);

		if (m_pText_Notice2_Text_2 != nullptr)
			m_pText_Notice2_Text_2->SetString(messages[1]);

		if (m_pGroup_Notice_2 != nullptr)
			m_pGroup_Notice_2->SetVisible(true);
	}
	else if (titles.size() == 3)
	{
		if (m_pText_Notice3_Name_1 != nullptr)
			m_pText_Notice3_Name_1->SetString(titles[0]);

		if (m_pText_Notice3_Text_1 != nullptr)
			m_pText_Notice3_Text_1->SetString(messages[0]);

		if (m_pText_Notice3_Name_2 != nullptr)
			m_pText_Notice3_Name_2->SetString(titles[1]);

		if (m_pText_Notice3_Text_2 != nullptr)
			m_pText_Notice3_Text_2->SetString(messages[1]);

		if (m_pText_Notice3_Name_3 != nullptr)
			m_pText_Notice3_Name_3->SetString(titles[2]);

		if (m_pText_Notice3_Text_3 != nullptr)
			m_pText_Notice3_Text_3->SetString(messages[2]);

		if (m_pGroup_Notice_3 != nullptr)
			m_pGroup_Notice_3->SetVisible(true);
	}
}

void CUILogIn_1298::OpenNews()
{
	if (m_bIsNewsVisible)
		return;

	if (m_pGroup_Notice_1 == nullptr || m_pGroup_Notice_2 == nullptr || m_pGroup_Notice_3 == nullptr)
		return;

	// set position of notice boxes
	// if it is done in Load function, they are not centered.
	RECT rc = m_pGroup_Notice_1->GetRegion();
	int iX  = ((int) s_CameraData.vp.Width - (rc.right - rc.left)) / 2;
	int iY  = ((int) s_CameraData.vp.Height - (rc.bottom - rc.top)) / 2;
	m_pGroup_Notice_1->SetPos(iX, iY);

	rc = m_pGroup_Notice_2->GetRegion();
	iX = ((int) s_CameraData.vp.Width - (rc.right - rc.left)) / 2;
	iY = ((int) s_CameraData.vp.Height - (rc.bottom - rc.top)) / 2;
	m_pGroup_Notice_2->SetPos(iX, iY);

	rc = m_pGroup_Notice_3->GetRegion();
	iX = ((int) s_CameraData.vp.Width - (rc.right - rc.left)) / 2;
	iY = ((int) s_CameraData.vp.Height - (rc.bottom - rc.top)) / 2;
	m_pGroup_Notice_3->SetPos(iX, iY);

	m_bIsNewsVisible = true;
}

void CUILogIn_1298::OpenServerList()
{
	if (m_pGroup_ServerList == nullptr)
		return;

	// close all notice boxes
	if (m_pGroup_Notice_1 != nullptr)
		m_pGroup_Notice_1->SetVisible(false);

	if (m_pGroup_Notice_2 != nullptr)
		m_pGroup_Notice_2->SetVisible(false);

	if (m_pGroup_Notice_3 != nullptr)
		m_pGroup_Notice_3->SetVisible(false);

	// 스르륵 열린다!! = open without sound
	if (m_pGroup_ServerList != nullptr)
		m_pGroup_ServerList->SetVisible(true);

	if (m_pStr_Premium != nullptr)
		m_pStr_Premium->SetVisible(true);

	// Select first server by default.
	SelectServer(0);

	// 2369/ISTIRAP: UIF'teki Connect düğmesi varsayılan olarak devre dışı olabilir; sunucu seçimi
	// (varsayılan 0) yapıldığı için düğmeyi zorla etkinleştir
	ConnectButtonSetEnable(true);
	CLogWriter::Write("CUILogIn_1298: sunucu listesi açıldı ({} sunucu, seçili {}, connect düğmesi {}, satır {})",
		m_ListServerInfos.size(), m_iSelectedServerIndex, m_pBtn_Connect ? m_pBtn_Connect->m_szID : "YOK",
		m_pList_Group[0] ? m_pList_Group[0]->m_szID : "YOK");

	m_bIsNewsVisible = false;
}

void CUILogIn_1298::SetVisibleLogInUIs(bool bEnable)
{
	if (m_pGroup_LogIn != nullptr)
		m_pGroup_LogIn->SetVisible(bEnable); // 로그인을 숨긴다..

	// Gizlenen gruptaki edit odağı tutmasın (sanal klavye açık kalıyor, yazı gizli kutuya gidiyordu)
	if (!bEnable)
	{
		CN3UIEdit* pFocused = CN3UIBase::GetFocusedEdit();
		if (pFocused != nullptr && (pFocused == m_pEdit_id || pFocused == m_pEdit_pw))
			pFocused->KillFocus();
	}
}

bool CUILogIn_1298::OnKeyPress(int iKey)
{
	if (!m_bLogIn)
	{
		switch (iKey)
		{
			case DIK_TAB:
				FocusCircular();
				return true;

				// case DIK_NUMPADENTER:
				// case DIK_RETURN:
				//	CGameProcedure::s_pProcLogIn->MsgSend_AccountLogIn(LIC_KNIGHTONLINE);
				//	return true;

			default:
				break;
		}
	}
	else if (m_pGroup_ServerList != nullptr && m_pGroup_ServerList->IsVisible())
	{
		int iServerCount = 0;
		switch (iKey)
		{
			case DIK_UP:
				iServerCount = static_cast<int>(m_ListServerInfos.size());
				if (iServerCount == 0)
					return false;

				--m_iSelectedServerIndex;
				if (m_iSelectedServerIndex < 0)
					m_iSelectedServerIndex = iServerCount - 1;

				SelectServer(m_iSelectedServerIndex);
				return true;

			case DIK_DOWN:
				iServerCount = static_cast<int>(m_ListServerInfos.size());
				if (iServerCount == 0)
					return false;

				++m_iSelectedServerIndex;
				if (m_iSelectedServerIndex >= iServerCount)
					m_iSelectedServerIndex = 0;

				SelectServer(m_iSelectedServerIndex);
				return true;

			case DIK_NUMPADENTER:
			case DIK_RETURN:
				// connect to the selected server if user presses enter at server select screen
				// (2369/ISTIRAP: liste satırı bağlanamamışsa doğrudan seçili/ilk sunucuya)
				if (m_pList_Group[m_iSelectedServerIndex] != nullptr)
					ReceiveMessage(m_pList_Group[m_iSelectedServerIndex], UIMSG_STRING_LDCLICK);
				else
					CGameProcedure::s_pProcLogIn->ConnectToGameServer();
				return true;

			default:
				break;
		}
	}
	else if (m_bIsNewsVisible)
	{
		if (iKey == DIK_RETURN)
		{
			ReceiveMessage(m_pBtn_NoticeOK_1, UIMSG_BUTTON_CLICK);
			return true;
		}
	}

	return CN3UIBase::OnKeyPress(iKey);
}

void CUILogIn_1298::SelectServer(int iServerListIndex)
{
	m_iSelectedServerIndex = std::clamp(iServerListIndex, 0, MAX_SERVERS - 1);

	for (int i = 0; i < MAX_SERVERS; i++)
	{
		if (m_pList_Group[i] == nullptr)
			continue;

		if (i == m_iSelectedServerIndex)
			m_pList_Group[i]->SetColor(D3DCOLOR_XRGB(0, 255, 0));     // green
		else
			m_pList_Group[i]->SetColor(D3DCOLOR_XRGB(255, 255, 255)); // white
	}
}
#endif
