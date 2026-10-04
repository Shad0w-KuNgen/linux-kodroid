#include "UIHPBar.h"
time_t nModul = 0;
CUIHPBarPlug::CUIHPBarPlug()
{
	m_dVTableAddr = NULL;

	vector<int>offsets;
	offsets.push_back(0x3E0);
	offsets.push_back(0);

	m_dVTableAddr = Engine->rdword(KO_DLG, offsets);
	MaxExp = 0;
	nflexptime = 0;
	text_nick = 0;
	text_level = 0;
	text_cordinate = 0;
	progress_flame = 0;
	Text_burning = 0;
	base_burning_frame = 0;
	ParseUIElements();
	time_t UNIXTIME;
	tm g_localTime;
	UNIXTIME = time(nullptr);
	g_localTime = *localtime(&UNIXTIME);
	nModul = UNIXTIME + 60;
}

CUIHPBarPlug::~CUIHPBarPlug()
{
}

void CUIHPBarPlug::ParseUIElements()
{
	text_hp = Engine->GetChildByID(m_dVTableAddr, "Text_HP");
	text_mp = Engine->GetChildByID(m_dVTableAddr, "Text_MSP");
#if (HOOK_SOURCE_VERSION == 1098 || HOOK_SOURCE_VERSION == 1534)
	if (HOOK_SOURCE_VERSION == 1098) {
		str_exp = Engine->GetChildByID(m_dVTableAddr, "str_exp");
		text_exp = Engine->GetChildByID(m_dVTableAddr, "Text_ExpP");
		progress_exp = Engine->GetChildByID(m_dVTableAddr, "progress_exp");
		progress_Sexp = Engine->GetChildByID(m_dVTableAddr, "progress_Sexp");
		Engine->SetProgressRange(progress_exp, 0, 100.0f); // 0 min deðer 100 max deðer progressbar için
		Engine->SetProgressRange(progress_Sexp, 0, 100.0f);
	}
	text_nick = Engine->GetChildByID(m_dVTableAddr, "text_nick");
	text_level = Engine->GetChildByID(m_dVTableAddr, "text_level");
#else
	Text_burning = Engine->GetChildByID(m_dVTableAddr, "Text_burning");
	base_burning_frame = Engine->GetChildByID(m_dVTableAddr, "base_burning_frame");
	progress_flame = Engine->GetChildByID(base_burning_frame, "Progress_burning");
	Engine->SetProgressRange(progress_flame, 0, float(burntime*3)); // 0 min deðer 100 max deðer progressbar için
	Engine->SetString(Text_burning, string_format("Lv 0"));
#endif
}

void CUIHPBarPlug::HPChange(uint16 hp, uint16 maxhp)
{
	return;
	if (maxhp == 0)
		return;

	if (m_dVTableAddr != NULL) {
		std::string str = xorstr("%s/%s (%d%%)");
		int percent = (int)ceil((hp * 100) / maxhp);
		std::string max = Engine->StringHelper->NumberFormat(maxhp);
		std::string cur = Engine->StringHelper->NumberFormat(hp);
		Engine->SetString(text_hp, string_format(str, cur.c_str(), max.c_str(), percent));
	}
}

void CUIHPBarPlug::MPChange(uint16 mp, uint16 maxmp)
{
	return;

	if (maxmp == 0)
		return;

	if (m_dVTableAddr != NULL) {
		std::string str = xorstr("%s/%s (%d%%)");
		int percent = (int)ceil((mp * 100) / maxmp);
		std::string max = Engine->StringHelper->NumberFormat(maxmp);
		std::string cur = Engine->StringHelper->NumberFormat(mp);
		Engine->SetString(text_mp, string_format(str, cur.c_str(), max.c_str(), percent));
	}
}

void CUIHPBarPlug::ExpChange(int64 exp, int64 maxexp) {
#if (HOOK_SOURCE_VERSION == 1098)
	if (m_dVTableAddr != NULL) {
		std::string yaz = "";
		std::vector<std::string> out;
		yaz = Engine->GetString(Engine->uiTaskbarMain->str_exp).c_str();

		Engine->str_replace(yaz, "EXP:", "");
		Engine->str_replace(yaz, "%", "");
		Engine->str_split(yaz.c_str(), '.', out);

		int percent = (int)ceil((exp * 100) / maxexp);

		if (out.size() == 2)
		{
			Engine->SetCurValue(progress_Sexp, (float)atoi(out[1].c_str()), 15.0f);
			Engine->SetString(str_exp, string_format("%d.%s", percent, out[1].c_str()));
		}
		Engine->SetCurValue(progress_exp, (float)percent, 15.0f);
	}
#endif
}

void CUIHPBarPlug::BurningChange(uint16 ilevel) {
#if (HOOK_SOURCE_VERSION == 2369)
	if (m_dVTableAddr == NULL)
		return;

	if (ilevel > 3)
		ilevel = 3;

	nflexptime = float(burntime * ilevel);

	std::string text = ilevel < 3 ? std::to_string(ilevel) : "Max";
	Engine->SetCurValue(progress_flame, nflexptime, 15.0f);
	Engine->SetString(Text_burning, string_format("Lv %s", text.c_str()));
#endif
}

long timeLaps = 0;

extern std::string GetName(DWORD obj);
extern std::string GetZoneName(uint16 zoneID);
int nTime = 0;

void CUIHPBarPlug::Tick()
{
	if (timeLaps > clock() - 200)
		return;

	timeLaps = clock();

	DWORD KO_CHR = *(DWORD*)KO_PTR_CHR;

	

#if (HOOK_SOURCE_VERSION == 1098 || HOOK_SOURCE_VERSION == 1534)
	
	if (text_nick)
		Engine->SetString(text_nick, GetName(KO_CHR).c_str());

	if (text_level) {
		uint8 lvl = *(uint8*)(KO_CHR + KO_OFF_LEVEL);
		if (HOOK_SOURCE_VERSION == 1098)
			Engine->SetString(text_level, string_format("Level : %d", lvl));
		else
			Engine->SetString(text_level, string_format(" %d", lvl));
	}

	/*if (text_cordinate)
	{
		Engine->SetVisible(text_cordinate, false);
		float x, z;
		x = *(float*)(KO_CHR + KO_OFF_X);
		z = *(float*)(KO_CHR + KO_OFF_Z);
		Engine->SetString(text_cordinate, string_format("%s %d, %d", GetZoneName(*(uint16*)(KO_CHR + KO_OFF_ZONE)).c_str(), static_cast<int>(x), static_cast<int>(z)));
	}*/
#else
	time_t UNIXTIME;
	tm g_localTime;
	UNIXTIME = time(nullptr);
	g_localTime = *localtime(&UNIXTIME);

	if (Engine->Player.burninglevel < 3 && UNIXTIME > nModul)
	{
		nModul = UNIXTIME + burntime;
		nTime++;
		nflexptime++;
		//Engine->SetCurValue(Engine->uiHPBarPlug->progress_flame, (float)nflexptime, 15.0f);
		Engine->SetCurValue(Engine->uiHPBarPlug->progress_flame, (float)nflexptime, 15.0f);

		if (nTime > (burntime - 1)) {
			Engine->Player.burninglevel++;
			std::string text = Engine->Player.burninglevel < 3 ? std::to_string(Engine->Player.burninglevel) : "Max";
			Engine->SetString(Text_burning, string_format("Lv %s", text.c_str()));
			nTime = 0;
		}
	}
#endif
}