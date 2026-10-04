#pragma once
#include "stdafx.h"

class Discord {
private:
	int64_t StartTime;
	DiscordEventHandlers handlers;
	DiscordRichPresence discordPresence;
public:
	Discord();
	void Initialize();
	void Update(bool inGame = false);
};
