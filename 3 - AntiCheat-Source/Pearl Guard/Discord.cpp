#include "Discord.h"

extern std::string GetZoneName(uint16 zoneID);
std::string a = "";
std::string b = "";
std::string c = "";

std::string GetName(DWORD obj)
{
	DWORD nameLen = *(DWORD*)((DWORD)obj + KO_OFF_NAMELEN);
	char* name = new char[nameLen + 1]{ 0 };
	if (nameLen < 16)
		memcpy(name, (char*)((DWORD)obj + KO_OFF_NAME), nameLen);
	else
		memcpy(name, (char*)*(DWORD*)((DWORD)obj + KO_OFF_NAME), nameLen);
	std::string n = name;
	std::free(name);
	return n;
}

std::string GetZoneName(uint16 zoneID)
{
	switch (zoneID)
	{
	case 0: return xorstr("All Zones"); break;
	case 1: return xorstr("Luferson Castle"); break;
	case 2: return xorstr("El Morad Castle"); break;
	case 5: return xorstr("Luferson Castle"); break;
	case 6: return xorstr("Luferson Castle"); break;
	case 7: return xorstr("El Morad Castle"); break;
	case 8: return xorstr("El Morad Castle"); break;
	case 11: return xorstr("Eslant"); break;
	case 12: return xorstr("Eslant"); break;
	case 13: return xorstr("Eslant"); break;
	case 14: return xorstr("Eslant"); break;
	case 15: return xorstr("Eslant"); break;
	case 16: return xorstr("Eslant"); break;
	case 18: return xorstr("Old Karus"); break;
	case 21: return xorstr("Moradon"); break;
	case 22: return xorstr("Moradon"); break;
	case 23: return xorstr("Moradon"); break;
	case 24: return xorstr("Moradon"); break;
	case 25: return xorstr("Moradon"); break;
	case 28: return xorstr("Old Human"); break;
	case 29: return xorstr("Old Moradon"); break;
	case 30: return xorstr("Delos"); break;
	case 31: return xorstr("Bifrost"); break;
	case 32: return xorstr("Desperation Abyss"); break;
	case 33: return xorstr("Hell Abyss"); break;
	case 34: return xorstr("Felankor Lair"); break;
	case 35: return xorstr("Delos Basement"); break;
	case 36: return xorstr("Test"); break;
	case 48: return xorstr("Battle Arena"); break;
	case 51: return xorstr("Orc Prisoner Quest"); break;
	case 52: return xorstr("Blood Don Quest"); break;
	case 53: return xorstr("Goblin Quest"); break;
	case 54: return xorstr("Cape Quest"); break;
	case 55: return xorstr("Forgotten Temple"); break;
	case 61: return xorstr("Napies Gorge"); break;
	case 62: return xorstr("Alseids Prairie"); break;
	case 63: return xorstr("Nieds Triangle"); break;
	case 64: return xorstr("Nereids Island"); break;
	case 65: return xorstr("Moradon War"); break;
	case 66: return xorstr("Oreads"); break;
	case 67: return xorstr("Test Zone"); break;
	case 69: return xorstr("Snow War"); break;
	case 71: return xorstr("Colony Zone"); break;
	case 72: return xorstr("Ardream"); break;
	case 73: return xorstr("Ronark Land Base"); break;
	case 74: return xorstr("Test Zone"); break;
	case 75: return xorstr("Krowaz Domion"); break;
	case 76: return xorstr("Knight Royale"); break;
	case 77: return xorstr("Clan War Ardream"); break;
	case 78: return xorstr("Clan War Ronark Land"); break;
	case 81: return xorstr("Monster Stone"); break;
	case 82: return xorstr("Monster Stone"); break;
	case 83: return xorstr("Monster Stone"); break;
	case 84: return xorstr("Border Defance War"); break;
	case 85: return xorstr("Chaos Dengueon"); break;
	case 86: return xorstr("Under The Castle"); break;
	case 87: return xorstr("Juraid Mountain"); break;
	case 88: return xorstr("Lk War"); break;
	case 89: return xorstr("Dungeon Defence"); break;
	case 91: return xorstr("Zindan War"); break;
	case 92: return xorstr("Prison"); break;
	case 93: return xorstr("Isillion Lair"); break;
	case 94: return xorstr("Felankor Lair"); break;
	case 95: return xorstr("Drakis Tower"); break;
	case 96: return xorstr("Party War"); break;
	case 97: return xorstr("Party War"); break;
	case 98: return xorstr("Party War"); break;
	case 99: return xorstr("Party War"); break;
	case 105: return xorstr("Zindan War"); break;
	case 106: return xorstr("Piana War"); break;
	case 107: return xorstr("Breath War"); break;
	case 108: return xorstr("Colony Castle War"); break;
	case 109: return xorstr("Moradon War"); break;
	case 110: return xorstr("Snow War"); break;
	case 111: return xorstr("Koba War"); break;
	default: return xorstr("Unknown"); break;
	}
	return xorstr("Unknown");
}

Discord::Discord()
{
	memset(&handlers, 0, sizeof(handlers));
	memset(&discordPresence, 0, sizeof(discordPresence));
	StartTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void Discord::Initialize()
{
	Discord_Initialize("1139371849646604298", &handlers, 1, "0");		// Myko Network discord api

}

void Discord::Update(bool inGame)
{
	memset(&discordPresence, 0, sizeof(discordPresence));
	discordPresence.largeImageKey = "ISTIRAP ";				// Myko Network discord logo
	discordPresence.largeImageText = "ISTIRAP";				// Myko Network discord logo


	if (inGame) {
		DWORD KO_CHR = *(DWORD*)KO_PTR_CHR;
		if (!KO_CHR) 
			return;

		uint8 lvl = *(uint8*)(KO_CHR + KO_OFF_LEVEL);
		uint16 zoneid = *(uint16*)(KO_CHR + KO_OFF_ZONE);

		a = string_format("ISTIRAP AntiCheat System");
		b = string_format("Nick : %s - Lvl :%d", GetName(KO_CHR).c_str(), lvl).c_str();
		c = GetZoneName(zoneid).c_str();
		discordPresence.state = b.c_str();
		discordPresence.details = a.c_str();
	}
	else {
		discordPresence.state = "Oyuna Giris Yapiyor";
		discordPresence.details = "";
		discordPresence.details = "";
	}
	discordPresence.startTimestamp = StartTime;
	discordPresence.endTimestamp = NULL;

	Discord_UpdatePresence(&discordPresence);
}