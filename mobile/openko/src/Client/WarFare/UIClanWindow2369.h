// 2369 klan penceresi (ISTIRAP\re_clan_window.istirap): üye listesi WIZ_KNIGHTS_PROCESS|13 ile sunucudan.
#pragma once
#include "UIGeneric2369.h"
#include <vector>
#include <string>

class Packet;

class CUIClanWindow2369 : public CUIGeneric2369
{
public:
	struct Member
	{
		std::string name, memo;
		int fame = 0, level = 0, cls = 0, online = 0;
		uint32_t hours = 0;
	};
	void Open();
	bool ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg) override;
	/// WIZ_KNIGHTS_PROCESS alt op 13 (üye listesi) yanıtı; işlenirse true
	bool OnMemberList(Packet& pkt);
	void Refresh();

private:
	void RequestMembers();
	void SendNotice(const std::string& szNotice);
	static constexpr int ROWS = 5;
	std::vector<Member> m_Members;
	std::string m_szNotice;
	int m_iMax = 0, m_iPage = 0, m_iSelected = -1;
};
