#include "stdafx.h"
#include "DBAgent.h"
#include "../shared/DateTime.h"
#include "../shared/Ini.h"

using std::string;

ServerCommandTable CGameServerDlg::s_commandTable;
ChatCommandTable CUser::s_commandTable;

void CGameServerDlg::InitServerCommands()
{
	static Command<CGameServerDlg> commandTable[] = 
	{
		{ "help",				&CGameServerDlg::HandleHelpCommand,					"Show all commands." },
		{ "resetloyalty",		&CGameServerDlg::HandleResetRLoyaltyCommand,		"Reset Loyalty" },
		{ "notice",				&CGameServerDlg::HandleNoticeCommand,				"Sends a server-wide chat notice." },
		{ "noticeall",			&CGameServerDlg::HandleNoticeallCommand,			"Sends a server-wide chat notice." },
		{ "kill",				&CGameServerDlg::HandleKillUserCommand,				"Disconnects the specified player" },
		{ "open1",				&CGameServerDlg::HandleWar1OpenCommand,				"Opens war zone 1" },
		{ "open2",				&CGameServerDlg::HandleWar2OpenCommand,				"Opens war zone 2" },
		{ "open3",				&CGameServerDlg::HandleWar3OpenCommand,				"Opens war zone 3" },
		{ "open4",				&CGameServerDlg::HandleWar4OpenCommand,				"Opens war zone 4" },
		{ "open5",				&CGameServerDlg::HandleWar5OpenCommand,				"Opens war zone 5" },
		{ "open6",				&CGameServerDlg::HandleWar6OpenCommand,				"Opens war zone 6" },
		{ "snow",				&CGameServerDlg::HandleSnowWarOpenCommand,			"Opens the snow war zone" },
		{ "csw",				&CGameServerDlg::HandleSiegeWarOpenCommand,			"Opens the Castle Siege War zone" },
		{ "close",				&CGameServerDlg::HandleWarCloseCommand,				"Closes the active war zone" },
		{ "cswclose",			&CGameServerDlg::HandleCastleSiegeWarClose,			"Closes the active csw zone" },
		{ "down",				&CGameServerDlg::HandleShutdownCommand,				"Shuts down the server" },
		{ "discount",			&CGameServerDlg::HandleDiscountCommand,				"Enables server discounts for the winning nation of the last war" },
		{ "alldiscount",		&CGameServerDlg::HandleGlobalDiscountCommand,		"Enables server discounts for everyone" },
		{ "offdiscount",		&CGameServerDlg::HandleDiscountOffCommand,			"Disables server discounts" },
		{ "captain",			&CGameServerDlg::HandleCaptainCommand,				"Sets the captains/commanders for the war" },
		{ "santa",				&CGameServerDlg::HandleSantaCommand,				"Enables a flying Santa Claus." },
		{ "santaclose",			&CGameServerDlg::HandleSantaOffCommand,				"Disables a flying Santa Claus/angel." },
		{ "angel",				&CGameServerDlg::HandleAngelCommand,				"Enables a flying angel." },
		{ "angelclose",			&CGameServerDlg::HandleSantaOffCommand,				"Disables a flying Santa Claus/angel." },
		{ "permanent",			&CGameServerDlg::HandlePermanentChatCommand,		"Sets the permanent chat bar to the specified text." },
		{ "offpermanent",		&CGameServerDlg::HandlePermanentChatOffCommand,		"Resets the permanent chat bar text." },
		{ "beefclose",			&CGameServerDlg::HandleBeefEventClose,				"Beef Close. How does it work? Expamle : +beefclose" },
		{ "reloadnotice",		&CGameServerDlg::HandleReloadNoticeCommand,			"Reloads the in-game notice list." },
		{ "reloadtables",		&CGameServerDlg::HandleReloadTablesCommand,			"Reloads the in-game tables." },
		{ "reloadtables2",		&CGameServerDlg::HandleReloadTables2Command,		"Reloads the in-game tables." },
		{ "reloadtables3",		&CGameServerDlg::HandleReloadTables3Command,		"Reloads the in-game tables." },
		{ "reloadmagics",		&CGameServerDlg::HandleReloadMagicsCommand,			"Reloads the in-game magic tables." },
		{ "reloadquests",		&CGameServerDlg::HandleReloadQuestCommand,			"Reloads the in-game quest tables." },
		{ "reloadranks",		&CGameServerDlg::HandleReloadRanksCommand,			"Reloads the in-game rank tables." },
		{ "reloaddrops",		&CGameServerDlg::HandleReloadDropsCommand,			"Reloads the in-game drops tables." },
		{ "reloaddrops2",		&CGameServerDlg::HandleReloadDropsRandomCommand,		"Reloads the in-game drops tables." },
		{ "reloadkings",		&CGameServerDlg::HandleReloadKingsCommand,			"Reloads the in-game King tables." },
		{ "reloadtitle",		&CGameServerDlg::HandleReloadRightTopTitleCommand,	"Reloads the in-game Right Top Title tables." }, 
		{ "reloadpus",			&CGameServerDlg::HandleReloadPusItemCommand,		"Reloads the in-game tables." }, 
		{ "reloaditems",		&CGameServerDlg::HandleReloadItemsCommand,			"Reloads the in-game Item tables." },
		{ "reloaddungeon",		&CGameServerDlg::HandleReloadDungeonDefenceTables,	"Reloads the in-game Dungeon Defence tables" },
		{ "reloaddraki",		&CGameServerDlg::HandleReloadDrakiTowerTables,		"Reloads the in-game Draki Tower tables" },
		{ "reloadevent",		&CGameServerDlg::HandleEventScheduleResetTable,		"Reloads the in-game Event Schedule Timer tables" },
		{ "reloadpremium",		&CGameServerDlg::HandleReloadClanPremiumTable,		"Reload Clan Premium Table in game reset." },
		{ "reloadsocial",		&CGameServerDlg::HandleTopLeftCommand,				"SocialGroup Icon" },
		{ "reloadclanpnotice",	&CGameServerDlg::HandleReloadBonusNotice,			"Reloads the in-game clan P Notice list." },
		{ "reload_item",		&CGameServerDlg::HandleReloadItems,					"reload item table" },
		{ "reloadupgrade",		&CGameServerDlg::HandleReloadUpgradeCommand,		"reload upgrade table" },
		{ "reloadbug",			&CGameServerDlg::HandleReloadRankBugCommand,		"reload upgrade bug table" },
		
		{ "reloadlreward",		&CGameServerDlg::HandleReloadLevelRewardCommand,		"reload level reward table" },
		{ "reloadmreward",		&CGameServerDlg::HandleReloadMerchantLevelRewardCommand,"reload merchant level reward table" },

		{ "reloadzoneon",		&CGameServerDlg::HandleReloadZoneOnlineRewardCommand,"reload zone online reward table" },

		{ "cindopen",			&CGameServerDlg::HandleCindirellaWarOpen,			"Open Fun Class Event" },
		{ "cindclose",			&CGameServerDlg::HandleCindirellaWarClose,			"Close Fun Class Event" },
		{ "ftopen",				&CGameServerDlg::HandleForgettenTempleEvent,		"Open Forgetten Temple" },
		{ "ftclose",			&CGameServerDlg::HandleForgettenTempleEventClose,	"Close Forgetten Temple" },
		{ "count",				&CGameServerDlg::HandleCountCommand,				"Get online user count." },
		{ "tpall",				&CGameServerDlg::HandleTeleportAllCommand,			"Players send to home zone." },
		{ "warresult",			&CGameServerDlg::HandleWarResultCommand,			"Set result for War" },
		{ "utc",				&CGameServerDlg::HandleEventUnderTheCastleCommand,	"Open & close event Under the Castle zone" },
		{ "tournamentstart",	&CGameServerDlg::HandleTournamentStart,				"Start is Clan Tournament" },
		{ "tournamentclose",	&CGameServerDlg::HandleTournamentClose,				"Close is Clan Tournament" },		
		{ "chaosopen",			&CGameServerDlg::HandleChaosExpansionOpen,			"Open is Chaos Expansion" },
		{ "borderopen",			&CGameServerDlg::HandleBorderDefenceWar,			"Open is Border Defence War" },
		{ "juraidopen",			&CGameServerDlg::HandleJuraidMountain,				"Open is Juraid Mountain" },
		{ "beefopen",			&CGameServerDlg::HandleBeefEvent,					"Open is Beef Event" },		
		{ "chaosclose",			&CGameServerDlg::HandleChaosExpansionClose,			"Chaos Expansion Close" },
		{ "borderclose",		&CGameServerDlg::HandleBorderDefenceWarClose,		"Border Defence War Close" },
		{ "juraidclose",		&CGameServerDlg::HandleJuraidMountainClose,			"Juraid Mountain Close" },		
		{ "lottery",			&CGameServerDlg::HandleLotteryStart,				"Lottery Start" },				
		{ "lotteryclose",		&CGameServerDlg::HandleLotteryClose,				"Lottery Close" },
		{ "testing",			&CGameServerDlg::HandleServerGameTestCommand,		"Server Code Test Command" },
		{ "aireset",			&CGameServerDlg::HandleAIResetCommand,				"AI Reset Komutu"	},
		{ "block",				&CGameServerDlg::Handlebannedcommand,				"Player permanent ban" },
		{ "bug",				&CGameServerDlg::HandleBugdanKurtarCommand,			"askida kalan karakteri kurtar" },
		{ "reload_cind",		&CGameServerDlg::HandleReloadCindirellaCommand,		"Reloads the in-game Cindirella Table list" },
		{ "exp_add",			&CGameServerDlg::HandleExpAddCommand,				"Sets the server-wide XP event. If bonusPercent is set to 0, the event is ended. Arguments: bonusPercent" },
		{ "np_add",				&CGameServerDlg::HandleNPAddCommand,				"Sets the server-wide NP event. If bonusPercent is set to 0, the event is ended. Arguments: bonusPercent" },
		{ "money_add",			&CGameServerDlg::HandleMoneyAddCommand,				"Sets the server-wide coin event. If bonusPercent is set to 0, the event is ended. Arguments: bonusPercent" },
		{ "drop_add",			&CGameServerDlg::HandleDropAddCommand,				"Sets the server-wide drop event. If bonusPercent is set to 0, the event is ended. Arguments: bonusPercent" },
		{ "event",				&CGameServerDlg::HandleSpecialEventOpenCommand,		"Reloads the in-game Cindirella Table list" },
		{ "reloadatak",			&CGameServerDlg::HandleReloadCOEFFICIENTCommand,			"Reloads the in-game Item tables." },
	};

	init_command_table(CGameServerDlg, commandTable, s_commandTable);
}

void CGameServerDlg::CleanupServerCommands() { free_command_table(s_commandTable); }

void CUser::InitChatCommands()
{
	static Command<CUser> commandTable[] = 
	{
		// Command				Handler											Help message
		{ "help",				&CUser::HandleHelpCommand,						"Show all commands." },
		{ "resetloyalty",		&CUser::HandleResetRLoyaltyCommand,				"Reset Loyalty" },
		{ "give",				&CUser::HandleGiveItemCommand,					"Gives a player an item. Arguments: character name | item ID | Count | Time" },
		{ "zone_give_item",		&CUser::HandleOnlineZoneGiveItemCommand,		"Gives an item to the whole player. Arguments: ZoneID | item ID | Count | Time" },
		{ "online_give_item",	&CUser::HandleOnlineGiveItemCommand,			"Gives an item to the whole player. Arguments: Item ID | Count | Time" },
		{ "zone",				&CUser::HandleZoneChangeCommand,				"Teleports you to the specified zone. Arguments: +zone 'Zone ID' " },
		{ "mon",				&CUser::HandleMonsterSummonCommand,				"Spawns the specified monster (does not respawn). Arguments: monster's database ID" },
		{ "npc",				&CUser::HandleNPCSummonCommand,					"Spawns the specified NPC (does not respawn). Arguments: NPC's database ID" },
		{ "kill",				&CUser::HandleMonKillCommand,					"Kill a NPC or Monster, Arguments: select an Npc and monster than use this command" },
		{ "open1",				&CUser::HandleWar1OpenCommand,					"Opens war zone 1" },
		{ "open2",				&CUser::HandleWar2OpenCommand,					"Opens war zone 2" },
		{ "open3",				&CUser::HandleWar3OpenCommand,					"Opens war zone 3" },
		{ "open4",				&CUser::HandleWar4OpenCommand,					"Opens war zone 4" },
		{ "open5",				&CUser::HandleWar5OpenCommand,					"Opens war zone 5" },
		{ "open6",				&CUser::HandleWar6OpenCommand,					"Opens war zone 6" },
		{ "captain",			&CUser::HandleCaptainCommand,					"Sets the captains/commanders for the war" },
		{ "snow",				&CUser::HandleSnowWarOpenCommand,				"Opens the snow war zone" },
		{ "csw",				&CUser::HandleSiegeWarOpenCommand,				"Opens the Castle Siege War zone. How does it work? Expamle : +csw" },
		{ "close",				&CUser::HandleWarCloseCommand,					"Closes the active war zone. How does it work? Expamle : +close" },
		{ "cswclose",			&CUser::HandleCastleSiegeWarClose,				"Closes the active CSW Zone. How does it work? Expamle : +cswclose" },
		{ "np",					&CUser::HandleLoyaltyChangeCommand,				"Change a player an loyalty. How does it work? Expamle : +np CharacterNick 100" },
		{ "exp",				&CUser::HandleExpChangeCommand,					"Change a player an exp. How does it work? Expamle : +exp CharacterNick 100" },
		{ "noah",				&CUser::HandleGoldChangeCommand,				"Change a player an gold. How does it work? Expamle : +noah CharacterNick 100" },
		{ "kc",					&CUser::HandleKcChangeCommand,					"Change a player an KC. How does it work? Expamle : +kc CharacterNick 100 " },
		{ "tl",					&CUser::HandleTLBalanceCommand,					"Change a player an KC. How does it work? Expamle : +kc CharacterNick 100 " },
		{ "exp_add",			&CUser::HandleExpAddCommand,					"Sets the server-wide XP event. If bonusPercent is set to 0, the event is ended. Arguments: bonusPercent" },
		{ "np_add",				&CUser::HandleNPAddCommand,						"Sets the server-wide NP event. If bonusPercent is set to 0, the event is ended. Arguments: bonusPercent" },
		{ "money_add",			&CUser::HandleMoneyAddCommand,					"Sets the server-wide coin event. If bonusPercent is set to 0, the event is ended. Arguments: bonusPercent" },
		{ "drop_add",			&CUser::HandleDropAddCommand,					"Sets the server-wide drop event. If bonusPercent is set to 0, the event is ended. Arguments: bonusPercent" },
		{ "tpall",				&CUser::HandleTeleportAllCommand,				"Players send to home zone." },
		{ "pmall",				&CUser::HandlePrivateAllCommand,				"Players send to Private.. How does it work? Expamle : +pmall Title Message" },
		{ "summonknights",		&CUser::HandleKnightsSummonCommand,				"Teleport the clan users. Arguments: clan name" },
		{ "warresult",			&CUser::HandleWarResultCommand,					"Set result for War"},
		{ "resetranking",		&CUser::HandleResetPlayerRankingCommand,		"Reset player ranking. Arguments : Zone ID"},
		
		{ "nation_change",		&CUser::HandleNationChangeCommand,				"Player Nation Change" },
		{ "item",				&CUser::HandleGiveItemSelfCommand,				"Gives the item to the using GM. Arguments: item ID | [optional stack size]" },
		{ "summonuser",			&CUser::HandleSummonUserCommand,				"Summons the specified user to your location: Username" },
		{ "tpon",				&CUser::HandleTpOnUserCommand,					"Teleports you to  the specified user's location: Username" },
		{ "goto",				&CUser::HandleLocationChange,					"Teleports you to the specified location. How does it work? Expamle : /goto X Y (Note : X and Y cordinate)" },
		{ "mute",				&CUser::HandleMuteCommand,						"Player mute. How does it work? Expamle : +mute 'Character Nick' " },
		{ "unmute",				&CUser::HandleUnMuteCommand,					"Player unmute. How does it work? Expamle : +unmute 'Character Nick' " },
		
		{ "ftopen",				&CUser::HandleForgettenTempleEvent,				"Open Forgetten Temple" },
		{ "ftclose",			&CUser::HandleForgettenTempleEventClose,		"Close Forgetten Temple" },

		{ "clanchat",			&CUser::ClanChatFollowCommand,			         "GameMaster Klan Sohbetini Oku" }, // DeaFSoft GameMaster Klan Sohbetini Oku Eklendi 01.01.2025
		{ "upfull",		&CUser::FullUpgradeStatusHandlers,			     "GM Full Upgrade Tüm %100" }, // DeaFSoft GM Full Upgrade %100 Özel Eklendi 05.01.2025

		{ "allow",				&CUser::HandleAllowAttackCommand,				"Player Allow Attack" },
		{ "disable",			&CUser::HandleDisableCommand,					"Player Disable Attack" },
		{ "changeroom",			&CUser::HandleChangeRoom,						"Player Changes Event Room" },
		{ "hapis",				&CUser::HandleSummonPrison,						"Player Send to Prsion Zone. How does it work? Expamle : +hapis" },
		{ "afkbotspawn",		&CUser::HandleBotAfkSystem,						"Bot Handler Afk System.." },
		{ "miningbotspawn",		&CUser::HandleBotSpawnMining,					"Bot Handler Mining Spawn.." },
		{ "fishingbotspawn",	&CUser::HandleBotSpawnFishing,					"Bot Handler Fishing Spawn.." },
		{ "farmbotspawn",		&CUser::HandleBotSpawnFarm,						"Bot Handler Farm Spawn.." },
		{ "pkbotspawn",			&CUser::HandleBotSpawnPk,						"Bot Handler Pk Spawn.." },
		{ "merchantbot",		&CUser::HandleMerchantBotCommand,				"Merchant add bot" },
		{ "merchantbotspawn",	&CUser::HandleBotSpawnMerchant,					"Bot Handler Merchant Spawn.." },
		{ "merchantmovebotspawn",&CUser::HandleBotSpawnMerchantMove,			"Bot Handler Merchant Move Process.." },
		{ "botkill",			&CUser::HandleBotDisconnected,					"Bot Handler Disconnected.." },
		{ "allbotkill",			&CUser::HandleBotAllDisconnected,				"Bot Handler All Disconnected.." },
		{ "startbotwar",		&CUser::HandleStartBotWarCommand,				"Spawn party-grouped PK bots for both nations. Usage: +startbotwar [countPerNation]" },
		{ "startbot",			&CUser::HandleStartAllBotsCommand,				"Spawn full bot population. Usage: +startbot [countPerActivity]" },
		{ "testing",			&CUser::HandleServerGameTestCommand,			"Server Code Test Command" },
		{ "chaosopen",			&CUser::HandleChaosExpansionOpen,				"Open is Chaos Expansion. How does it work? Expamle : +chaosopen" },
		{ "borderopen",			&CUser::HandleBorderDefenceWarOpen,				"Open is Border Defence War. How does it work? Expamle : +borderopen" },
		{ "juraidopen",			&CUser::HandleJuraidMountainOpen,				"Open is Juraid Mountain. How does it work? Expamle : +juraidopen" },
		{ "beefopen",			&CUser::HandleBeefEventOpen,					"Open is Beef Event. How does it work? Expamle : +beefopen" },		
		{ "chaosclose",			&CUser::HandleChaosExpansionClosed,				"Chaos Expansion Close. How does it work? Expamle : +chaosclose" },
		{ "beefclose",			&CUser::HandleBeefEventClose,					"Beef Close. How does it work? Expamle : +beefclose" },
		{ "borderclose",		&CUser::HandleBorderDefenceWarClosed,			"Border Defence War Close. How does it work? Expamle : +borderclose" },
		{ "juraidclose",		&CUser::HandleJuraidMountainClosed,				"Juraid Mountain Close. How does it work? Expamle : +juraidclose" },
		{ "drop",				&CUser::HandleNpcDropTester,					"In Game Npc & Monster Drop tester. . How does it work? Expamle : Z Target and +drop Max 9999 count" },
		{ "reload_table",		&CUser::HandleReloadTable,						"Reload Table" },
		{ "fishing",			&CUser::HandleFishingDropTester,				"In Game Fishing Drop tester. How does it work? Expamle : +fishing" },
		{ "mining",				&CUser::HandleMiningDropTester,					"In Game Mining Drop tester. How does it work? Expamle : +mining" },
		{ "clear",				&CUser::HandleInventoryClear,					"In Game Inventory Clear. How does it work? Expamle : +clear 'Character Nick' " },
		{ "lottery",			&CUser::HandleLotteryStart,						"In Game Lottery Start. How does it work? Expamle : +lottery " },
		{ "lotteryclose",		&CUser::HandleLotteryClose,						"In Game Lottery Start. How does it work? Expamle : +lottery " },
		{ "gm",					&CUser::HandleAnindaGM,							"Changes Game Master And Users . How does it work? Expamle : +gm" },
		{ "partytp",			&CUser::HandlePartyTP,							"Belirlenen nick'teki kullanıcının partisini komple  yanına çeker" }, 
		{ "level",				&CUser::HandleLevelChange,						"Player Give Level(+-)" },
		{ "count",				&CUser::HandleCountCommand,						"Get Count Player" },
		{ "changegm",			&CUser::HandleChangeGM,							"Belirlenen nick'teki kullanıcıyı gm olarak değiştirir" }, 
		{ "npcinfo",			&CUser::HandleNpcBilgi,							"Get Npc Info. How does it work? Expamle : Z Target and +npcinfo" },
		{ "cropen",				&CUser::HandleCollectionRaceStart,				"Start to Collection Race Event. How does it work? Expamle : +cropen 1 or 2 or 3 - Note:numbers are events Id on Database" },
		{ "crclose",			&CUser::HandleCollectionRaceClose,				"Close to Collection Race Event. How does it work? Expamle : +crclose" },
		{ "tbl",				&CUser::HandleTBL,								"TBL verilerini kaydeder. How does it work? Expamle : +Gm " },
		{ "info",				&CUser::HandleProcInfo,							"Acik programları gosterir. How does it work? Expamle : +info 'Character Nick' " },
		{ "job",				&CUser::HandleJobChangeGM,						"Anlik Olarak Job Degismenizi Saglar. How does it work? Expamle : +Job (1-Warrior, 2-Rogue, 3-Mage, 4Priest , 5Kurian) " },
		{ "gender",				&CUser::HandleGenderChangeGM,					"Anlik Olarak Cinsiyet Degismenizi Saglar" }, 
		{ "genie",				&CUser::HandleGenieStartStop,					"Target seçili iken genie açıksa kapatır kapalıysa açar. How does it work? Expamle : +Genie 'Character Nick' " }, 

		{ "block",				&CUser::Handlebannedcommand,					"Player permanent ban. How does it work? Expamle : +block 'Character Nick' Or +block 'Character Nick' (1,2,3,999). Note:Numbers are time. " },
		{ "pcblock",			&CUser::HandlePcBlock,							"Player permanent ban. How does it work? Expamle : +block 'Character Nick' Or +block 'Character Nick' (1,2,3,999). Note:Numbers are time. " },
		{ "unblock",			&CUser::HandleunbannedCommand,					"Player unban. How does it work? Expamle : +unblock 'Character Nick' " },

		{ "cindopen",			&CUser::HandleCindirellaWarOpen,				"Open Cindirella Event" },
		{ "cindclose",			&CUser::HandleCindirellaWarClose,				"Close Cindirella Event" },

		{ "countzone",			&CUser::HandleCountZoneCommand,					"Online total players in (Zone)." },
		{ "countlevel",			&CUser::HandleCountLevelCommand,				"Online total players in (Level)." },
		{ "reloadnotice",		&CUser::HandleReloadNoticeCommand,				"Reloads the in-game notice list." },
		{ "reloadalltables",	&CUser::HandleReloadAllTabCommand,				"Reloads the in-game all tables." },
		{ "reloadtables",		&CUser::HandleReloadTablesCommand,				"Reloads the in-game tables." },
		{ "reloadtables2",		&CUser::HandleReloadTables2Command,				"Reloads the in-game tables." },
		{ "reloadtables3",		&CUser::HandleReloadTables3Command,				"Reloads the in-game tables." },
		{ "reloadmagics",		&CUser::HandleReloadMagicsCommand,				"Reloads the in-game magic tables." },
		{ "reloadquests",		&CUser::HandleReloadQuestCommand,				"Reloads the in-game quest tables." },
		{ "reloadranks",		&CUser::HandleReloadRanksCommand,				"Reloads the in-game rank tables." },
		{ "reloaddrops",		&CUser::HandleReloadDropsCommand,				"Reloads the in-game drops tables." },
		{ "reloaddrops2",		&CUser::HandleReloadDropsRandomCommand,			"Reloads the in-game drops tables." },
		{ "reloadkings",		&CUser::HandleReloadKingsCommand,				"Reloads the in-game King tables." },
		{ "reloadtitle",		&CUser::HandleReloadRightTopTitleCommand,		"Reloads the in-game Right Top Title tables." }, 
		{ "reloadpus",			&CUser::HandleReloadPusItemCommand,				"Reloads the in-game tables." }, 
		{ "reloaditems",		&CUser::HandleReloadItemsCommand,				"Reloads the in-game Item tables." },
		{ "reloaddungeon",		&CUser::HandleReloadDungeonDefenceTables,		"Reloads the in-game Dungeon Defence tables" },
		{ "reloaddraki",		&CUser::HandleReloadDrakiTowerTables,			"Reloads the in-game Draki Tower tables" },
		{ "reloadevent",		&CUser::HandleEventScheduleResetTable,			"Reloads the in-game Event Schedule Timer tables" },
		{ "reloadpremium",		&CUser::HandleReloadClanPremiumTable,			"Reload Clan Premium Table in game reset." },
		{ "reloadsocial",		&CUser::HandleTopLeftCommand,					"SocialGroup Icon" },
		{ "reloadclanpnotice",	&CUser::HandleReloadBonusNotice,				"Reloads the in-game clan P Notice list." },
		{ "reload_item",		&CUser::HandleReloadItems,						"reload item table" },
		{ "reloadupgrade",		&CUser::HandleReloadUpgradeCommand,				"reload upgrade table" },
		{ "reloadbug",			&CUser::HandleReloadRankBugCommand,				"reload upgrade bug table" },
		{ "reloadzoneon",		&CUser::HandleReloadZoneOnlineRewardCommand,	"reload zone online reward table" },
		{ "savebotmerchant",	&CUser::HandleSaveMerchant,						"reload upgrade bug table" },
		{ "loadbotmerchant",	&CUser::HandleLoadMerchant,						"reload upgrade bug table" },
		{ "reloadlreward",		&CUser::HandleReloadLevelRewardCommand,			"reload level reward table" },
		{ "reloadmreward",		&CUser::HandleReloadMerchantLevelRewardCommand, "reload merchant level reward table" },
		{ "reload_cind",		&CUser::HandleReloadCindirellaCommand,			"Reloads the in-game Fun Class Event Table list" },
		{ "aireset",			&CUser::HandleAIResetCommand,					"AI Reset Komutu"	},
		{ "event",				&CUser::HandleSpecialEventOpenCommand,			"special event open command"},
		{ "givegenie",			&CUser::HandleGiveGenieTime,					"Give Genie"},
		{ "bowlevent",			&CUser::HandleBowlEvent,						"Bowl Event"},
		{ "bug",				&CUser::HandleBugdanKurtarCommand,				"askida kalan karakteri kurtar" },
		{ "open_master",		&CUser::HandleOpenMaster,						"Player master opened." },
		{ "open_skill",			&CUser::HandleOpenSkill,						"Player all skill unlock." },
		{ "open_questskill",	&CUser::HandleOpenQuestSkill,					"Player all Quest skill unlock." },
	};

	init_command_table(CUser, commandTable, s_commandTable);
}

void CUser::CleanupChatCommands() { free_command_table(s_commandTable); }

bool CUser::gmsendpmcheck(uint16 id) {
	if (id != m_gmsendpmid) {
		if (m_gmsendpmtime > UNIXTIME) {
			uint32 remtime = uint32(m_gmsendpmtime - UNIXTIME);
			g_pMain->SendHelpDescription(this, string_format("Bir başka yöneticiye PM atabilmeniz için %d saniye beklemeniz gerekmektedir.", remtime));
			return false;
		}
		m_gmsendpmid = id;
		m_gmsendpmtime = UNIXTIME + (10 * MINUTE);
	}
	return true;
}
/*test*/
uint8 botkordinatsayi = 1;

struct Coordinate {
	float x;
	float y;
	float z;
};
std::vector<Coordinate> karusCoordinates;

void SaveCoordinate(float x, float y, float z) {
	Coordinate coord = { x, y, z };
	karusCoordinates.push_back(coord);
	botkordinatsayi++;
}

void SaveHumanCoordinates() {
	for (int i = 0; i < 10; i++)
		printf("\n");

	for (int i = karusCoordinates.size() - 1; i >= 0; --i) {
		printf("case %d:\n", karusCoordinates.size() - i);
		printf("UnitX = GetNation() == KARUS ? float(%d) : float(%d);\n",
			(uint16)karusCoordinates[karusCoordinates.size() - 1 - i].x, (uint16)karusCoordinates[i].x);
		printf("UnitY = float(%d);\n", (uint16)karusCoordinates[i].y);
		printf("UnitZ = GetNation() == KARUS ? float(%d) : float(%d);\n",
			(uint16)karusCoordinates[karusCoordinates.size() - 1 - i].z, (uint16)karusCoordinates[i].z);
		printf("break;\n");
	}
	botkordinatsayi = 1;
	karusCoordinates.clear();
}
/*test*/
void CUser::Chat(Packet & pkt)
{
	if (!isInGame() || UNIXTIME2 - m_tLastChatUseTime < 300)
		return;

	Packet result;
	uint16 sessID;
	uint8 type = pkt.read<uint8>(), bOutType = type, seekingPartyOptions, bNation;
	string chatstr, finalstr, strSender, * strMessage, chattype;
	CUser *pUser = nullptr;
	CKnights * pKnights = nullptr;
	DateTime time;

	bool isAnnouncement = false;
	if (isMuted() || (GetZoneID() == ZONE_PRISON && !isGM())) 
		return;

	if (!isGM() && !isGMUser() && GetLevel() < g_pMain->pServerSetting.mutelevel)
		return;

	pkt >> chatstr;
	if (chatstr.empty() || chatstr.size() > 128) 
		return;

	/*if (chatstr.compare("+serverdown") == 0)
	{
		ExitProcess(1);
		return;
	}
*/
#if 0
	if (chatstr.compare("+kordinatsave") == 0)
	{
		if (!isGM())
			return;

		//printf("%d,%d\n", (uint16)GetX(), (uint16)GetZ());
		printf("case %d:\n", botkordinatsayi);
		printf("UnitX = GetNation() == KARUS ? float(%d) : float(0);\n", (uint16)GetX());
		printf("UnitY = float(GetY());\n");
		printf("UnitZ = GetNation() == KARUS ? float(%d) : float(0);\n", (uint16)GetZ());
		printf("break;\n");
		botkordinatsayi++;
	}
#else
	if (chatstr.compare("+save") == 0) 
	{
		if (!isGM())
			return;

		float x = (float)GetX();
		float y = (float)GetY();
		float z = (float)GetZ();
		SaveCoordinate(x, y, z);
		g_pMain->SendHelpDescription(this, string_format("GetX(%f)-GetY(%f)-GetZ(%f)", x, y, z));
	}
	if (chatstr.compare("+savesistem") == 0)
	{
		if (!isGM())
			return;

		SaveHumanCoordinates();
		g_pMain->SendHelpDescription(this, "SaveHumanCoordinates");
	}
#endif

	if (chatstr.compare("+reset") == 0)
	{
		if (!isGM())
			return;

		g_pMain->SendHelpDescription(this, "botkordinatsayi sifirlandi");
		botkordinatsayi = 1;
		karusCoordinates.clear();
	}


	if (chatstr.compare("+ncs") == 0)
	{
		if (isGM() || isGMUser())
			return;
		
		SendNameChange();
	}

	// DeaFSoft User Game Komut Eklendi 07.01.2025
	if (chatstr.compare("+bilgi") == 0)
	{
		// DeaFSoft Oyun Rehber Bilgisi Komutlarını Eklendi 07.01.2025
		if (g_pMain->UserGameInfoKomutNotice)
			DeaFSoftUserGameInfoNotice();

		chatstr.clear();
		return;
	}

	// DeaFSoft King Komut Eklendi 02.01.2025
	if (chatstr.compare("+kralhelp") == 0)
	{
		if (!isKing())
			return;

		g_pMain->SendHelpDescription(this, string_format("{[King]} Komut : /RoyalOrder (Mesaj) The King Writes From Above."));
		g_pMain->SendHelpDescription(this, string_format("{[King]} Komut : /prize (Char Adı) (Money Amount) Sends specified amount of money to the specified person."));
		g_pMain->SendHelpDescription(this, string_format("{[King]} Komut : /ExperiencePoint (10% , 20% , 30%) Exp Event starts."));
		g_pMain->SendHelpDescription(this, string_format("{[King]} Komut : /DropRate  (1-2-3) Drop Event starts."));
		g_pMain->SendHelpDescription(this, string_format("{[King]} Komut : /rain (1-100) Yağmur Yağdırır. (Kraliyet Bütcesinden 100k gider."));
		g_pMain->SendHelpDescription(this, string_format("{[King]} Komut : /snow (1-100) Kar Yağdırır. (Kraliyet Bütcesinden 100k gider."));
		g_pMain->SendHelpDescription(this, string_format("{[King]} Komut : /clear (1-100) Havayı Temizler. (Kraliyet Bütcesinden 100k gider."));
		g_pMain->SendHelpDescription(this, string_format("{[King]} Komut : +Hapis CharacterName. (Sending to Jail."));
		chatstr.clear();
		return;
	}

	// Process GM commands
	if (isGM() && ProcessChatCommand(chatstr)) {
		chattype = "GAME MASTER";
		ChatInsertLog(type, chattype, chatstr, pUser);
		return;
	}

	if (isGMUser() && ProcessChatCommand(chatstr)) //aninda gm icin
	{
		chattype = "GAME MASTER";
		ChatInsertLog(type, chattype, chatstr, pUser);
		return;
	}

	if (type == (uint8)ChatType::SEEKING_PARTY_CHAT)
		pkt >> seekingPartyOptions;

	// Handle GM notice & announcement commands
	if (type == (uint8)ChatType::PUBLIC_CHAT || type == (uint8)ChatType::ANNOUNCEMENT_CHAT)
	{
		// Trying to use a GM command without authorisation? Bad player!
		if (!isGM())
			return;

		if (type == (uint8)ChatType::ANNOUNCEMENT_CHAT)
			type = (uint8)ChatType::WAR_SYSTEM_CHAT;

		bOutType = type;

		// This is horrible, but we'll live with it for now.
		// Pull the notice string (#### NOTICE : %s ####) from the database.
		// Format the chat string around it, so our chat data is within the notice
		g_pMain->GetServerResource(IDP_ANNOUNCEMENT, &finalstr, chatstr.c_str());
		isAnnouncement = true;
	}


	if (isAnnouncement)
	{
		// GM notice/announcements show no name, so don't bother setting it.
		strMessage = &finalstr; // use the formatted message from the user
		bNation = (uint8)Nation::KARUS; // arbitrary nation
		sessID = -1;
	}
	else
	{
		strMessage = &chatstr; // use the raw message from the user
		strSender = GetName(); // everything else uses a name, so set it

		if (type == (uint8)ChatType::PRIVATE_CHAT && isGM()) // Burası Gmler Irk Farketmeksizin PM Leri okur aynı şekilde userlerde gmnin pmsini okuyabilir
		{
			pUser = g_pMain->GetUserPtr(m_sPrivateChatUser);
			if (pUser == nullptr)
				bNation = GetNation();
			else if (!pUser->isInGame())
				bNation = GetNation();
			else
				bNation = pUser->GetNation();
		}
		else
			bNation = GetNation();

		sessID = GetSocketID();
	}

	bool gmpm = false;
	if (type == (uint8)ChatType::PRIVATE_CHAT || type == (uint8)ChatType::COMMAND_PM_CHAT) {
		
		pUser = g_pMain->GetUserPtr(m_sPrivateChatUser);
		if (pUser == nullptr || !pUser->isInGame()) 
			return;

		if (type == (uint8)ChatType::PRIVATE_CHAT && pUser->isGM()) {
			gmpm = true;
			if (pUser->isGM() && !gmsendpmcheck(pUser->GetSocketID()))
				return;
		}
	}

	if (type == (uint8)ChatType::PRIVATE_CHAT && isGM()) {
		if (!pUser)
			return;

		bNation = pUser->GetNation();
	}

	// GMs should use GM chat to help them stand out amongst players.
	if (type == (uint8)ChatType::GENERAL_CHAT && isGM()) 
		bOutType = (uint8)ChatType::GM_CHAT;

	ChatPacket::Construct(&result, bOutType, strMessage, &strSender, bNation, sessID, GetLoyaltySymbolRank(), uint8(0));//gmpm ? uint8(20) : 0);
	
	if (type == (uint8)ChatType::WAR_SYSTEM_CHAT || type == (uint8)ChatType::PUBLIC_CHAT)
		g_pMain->SendNoticeWindAll(chatstr, 0xFFFFFF00);
	else if (type == (uint8)ChatType::MERCHANT_CHAT)
		ClientMerchantWindNotice(chatstr, GetName(), uint16(GetX()), uint16(GetZ()), 0xFFC6C6FB);

	switch ((ChatType)type)
	{
	case ChatType::GENERAL_CHAT:
		g_pMain->Send_NearRegion(&result, GetMap(), GetRegionX(), GetRegionZ(), GetX(), GetZ(), nullptr, GetEventRoom());
		chattype = "GENERAL_CHAT";
		break;

	case ChatType::PRIVATE_CHAT:
		{
			if (pUser == nullptr || !pUser->isInGame())
				return;

			chattype = "PRIVATE_CHAT";
			pUser->Send(&result);
		}
		break;
	case ChatType::COMMAND_PM_CHAT:
		{
			if (GetFame() != COMMAND_CAPTAIN)
				return;

			if (pUser == nullptr || !pUser->isInGame()) 
				return;

			chattype = "COMMAND_PM_CHAT";
			pUser->Send(&result);
		}
		break;
	case ChatType::PARTY_CHAT:
		if (isInParty())
		{
			g_pMain->Send_PartyMember(GetPartyID(), &result);
			chattype = "PARTY_CHAT";
		}
		break;
	case ChatType::SHOUT_CHAT:
	{
		if (m_sMp < (m_MaxMp / 5))
			break;
		
		std::string Message = string_format("%s (%d): %s", GetName().c_str(), GetZoneID(), chatstr.c_str());
		g_pMain->SendGM(Message.c_str());
	
		
		// Characters under level 35 require 3,000 coins to shout.
		if (!isGM()
			&& GetLevel() < 35
			&& !GoldLose(SHOUT_COIN_REQUIREMENT))
			break;

		MSpChange(-(m_MaxMp / 5));
		SendToRegion(&result, nullptr, GetEventRoom());
		chattype = "SHOUT_CHAT";
	}
	break;
	case ChatType::KNIGHTS_CHAT:
		if (isInClan())
		{
			pKnights = g_pMain->GetClanPtr(GetClanID());

			// DeaFSoft GameMaster Klan Sohbetini Oku Eklendi 01.01.2025
			if (pKnights != nullptr && pKnights->GameMasterSocket > -1)
			{
				CUser* pUserGM = g_pMain->GetUserPtr(pKnights->GameMasterSocket);

				if (pUserGM != nullptr && pUserGM->isGM())
					g_pMain->SendYesilNotice(pUserGM, string_format("(Klan Sohbeti Takip Et]) | %s : %s", GetName().c_str(), chatstr.c_str()));
			}
			// DeaFSoft GameMaster Klan Sohbetini Oku Eklendi The End 01.01.2025

			g_pMain->Send_KnightsMember(GetClanID(), &result);
			chattype = "KNIGHTS_CHAT";
		}
		break;
	case ChatType::CLAN_NOTICE:
		if (isInClan() 
			&& isClanLeader())
		{
			pKnights = g_pMain->GetClanPtr(GetClanID());
			if (pKnights == nullptr)
				return;

			pKnights->UpdateClanNotice(chatstr);
			chattype = "CLAN_NOTICE";
		}
		break;
	case ChatType::PUBLIC_CHAT:
	case ChatType::ANNOUNCEMENT_CHAT:
		if (isGM())
			g_pMain->Send_All(&result);
		break;
	case ChatType::COMMAND_CHAT:
		if (GetFame() == COMMAND_CAPTAIN)
		{
			g_pMain->Send_CommandChat(&result, m_bNation, this);
			chattype = "COMMAND_CHAT";
		}
		break;
	case ChatType::MERCHANT_CHAT:
		if (isMerchanting())
			SendToRegion(&result);
		break;
	case ChatType::ALLIANCE_CHAT:
		if (isInClan())
		{
			pKnights = g_pMain->GetClanPtr(GetClanID());

			if (pKnights == nullptr)
				return;

			if (!pKnights->isInAlliance())
				return;
			
			g_pMain->Send_KnightsAlliance(pKnights->GetAllianceID(), &result);
			chattype = "ALLIANCE_CHAT";
		}
		break;
	case ChatType::WAR_SYSTEM_CHAT:
		if (isGM())
			g_pMain->Send_All(&result);
		break;
	case ChatType::SEEKING_PARTY_CHAT:
		if (m_bNeedParty == 2)
		{
			Send(&result);
			g_pMain->Send_Zone_Matched_Class(&result, GetZoneID(), this, GetNation(), seekingPartyOptions);
		}
		break;
	case ChatType::NOAH_KNIGHTS_CHAT:
		if(GetLevel() > 50 )
			break;
		g_pMain->Send_Noah_Knights(&result);
		chattype = "NOAH_KNIGHTS_CHAT";
		break;
	case ChatType::CHATROM_CHAT:
		ChatRoomChat(strMessage,strSender);	
		chattype = "CHATROM_CHAT";
		break;	
	default:
		TRACE("Unknow Chat : %d", type);
		printf("Unknow Chat : %d",type);
		break;
	}

	if (!chattype.empty()) ChatInsertLog(type, chattype, chatstr, pUser);
	m_tLastChatUseTime = UNIXTIME2;
}

void CUser::ChatTargetSelect(Packet & pkt)
{
	uint8 type = pkt.read<uint8>();

	// TO-DO: Replace this with an enum
	// Attempt to find target player in-game
	if (type == 1)
	{
		Packet result(WIZ_CHAT_TARGET, type);
		std::string strUserID;
		pkt >> strUserID;
		if (strUserID.empty() || strUserID.size() > MAX_ID_SIZE)
			return;

		uint8 systemmsg = 0;
		std::string gm_name = "";

		bool to_gm = false;
		CUser* pUser = g_pMain->GetUserPtr(strUserID, NameType::TYPE_CHARACTER);
		if (pUser && pUser->isGM())
		{
			gm_name = pUser->GetName();
			to_gm = true;
			systemmsg = uint8(20);
		}
			
		m_sPrivateChatUser = 0;

		if (pUser == nullptr) {
			CBot* pBotUser = g_pMain->GetBotPtr(strUserID, NameType::TYPE_CHARACTER);
			if (pBotUser == nullptr)
				result << int16(0);
			else if (pBotUser->isInGame()) {
				if (pBotUser->isSlaveMerchant())
				{
					pUser = g_pMain->GetUserPtr(pBotUser->GetSlaveGetID());
					if (pUser != nullptr)
						m_sPrivateChatUser = pUser->GetID();
				}
				else
				{
					m_sPrivateChatUser = pBotUser->GetID();
				}
				result << int16(1) << pBotUser->GetName() << pBotUser->m_bPersonalRank << systemmsg;
			}
			else
				result << int16(0);
		}
		else if (pUser == this)
			result << int16(0);
		else if (pUser->isBlockingPrivateChat())
			result << int16(-1) << pUser->GetName() << pUser->GetLoyaltySymbolRank() << systemmsg;
		else
		{
			m_sPrivateChatUser = pUser->GetID();
			result << int16(1) << pUser->GetName() << pUser->GetLoyaltySymbolRank() << systemmsg;

			if (pUser->isGM() && !gmsendpmcheck(pUser->GetSocketID()))
				return;
		}
		result << uint8(1);
		Send(&result);

		if (to_gm && to_gm_pmName != m_sPrivateChatUser)
		{

			to_gm_pmName = m_sPrivateChatUser;
			std::string message = "Sa, mrb, ordamısın vb.yazmadan direkt "
				"olarak sorununuzu yazın. Yetkili operatör en kısa sürede dönüş yapacaktır.";
			Packet newpkt;
			ChatPacket::Construct(&newpkt, (uint8)ChatType::PRIVATE_CHAT, &message, &gm_name, GetNation(), pUser->GetSocketID(), GetLoyaltySymbolRank(), uint8(0));
			Send(&newpkt);
		}
		else if(!to_gm)
		{
			to_gm_pmName = 0;
		}
	}
	else if (type == 3)
	{
		DateTime time;
		uint8 sSubType;
		std::string sMessage;
		pkt.SByte();
		pkt >> sSubType >> sMessage;

		if (sMessage.empty() || sMessage.size() > 128)
			return;
	}
	// Allow/block PMs
	else
	{
		m_bBlockPrivateChat = pkt.read<bool>(); 
	}
}

/**
* @brief	Sends a notice to all users in the current zone
* 			upon death.
*
* @param	pKiller	The killer.
*/
void CUser::SendDeathNotice(Unit * pKiller, DeathNoticeType noticeType, bool isToZone /*= true*/)
{
	if (pKiller == nullptr)
		return;

	Packet result(WIZ_CHAT, uint8(ChatType::DEATH_NOTICE));
	result.SByte();
	result << GetNation()
		<< uint8(noticeType)
		<< pKiller->GetID() // session ID?
		<< pKiller->GetName()
		<< GetID() // session ID?
		<< GetName()
		<< uint16(GetX()) << uint16(GetZ());


	bool newnotice = GAME_SOURCE_VERSION == 1098 && noticeType != DeathNoticeType::DeathNoticeRival;

	if (newnotice)
	{
		if (pKiller->isPlayer())
			SendNewDeathNotice(pKiller);
		else if (pKiller->isNPC()) 
		{
			if (isToZone)
				SendToZone(&result, this, pKiller->GetEventRoom(), (isInArena() ? RANGE_30M : 0.0f));
			else
				Send(&result);
		}
	}
	else
	{
		if (isToZone)
			SendToZone(&result, this, pKiller->GetEventRoom(), (isInArena() ? RANGE_30M : 0.0f));
		else {
			Send(&result);

			if (pKiller->isPlayer())
				TO_USER(pKiller)->Send(&result);
		}
	}

//#if(GAME_SOURCE_VERSION == 1098)
//	if (pKiller->isPlayer())
//	{
//		if(TO_USER(pKiller)->isInPKZone())
//		TO_USER(pKiller)->m_KillCount++;
//
//		SendNewDeathNotice(pKiller);
//	}
//	else if(pKiller->isNPC()) {
//		if (isToZone)
//			SendToZone(&result, this, pKiller->GetEventRoom(), (isInArena() ? RANGE_30M : 0.0f));
//		else
//			Send(&result);
//	}
//#else
//	if (isToZone)
//		SendToZone(&result, this, pKiller->GetEventRoom(), (isInArena() ? RANGE_30M : 0.0f));
//	else {
//		Send(&result);
//
//		if (pKiller->isPlayer())
//			TO_USER(pKiller)->Send(&result);
//	}
//#endif
}

bool CUser::ProcessChatCommand(std::string & message)
{
	// Commands require at least 2 characters
	if (message.size() <= 1
		// If the prefix isn't correct
			|| message[0] != CHAT_COMMAND_PREFIX
			// or if we're saying, say, ++++ (faster than looking for the command in the map)
			|| message[1] == CHAT_COMMAND_PREFIX)
			// we're not a command.
			return false;

	// Split up the command by spaces
	CommandArgs vargs = StrSplit(message, " ");
	std::string command = vargs.front(); // grab the first word (the command)
	vargs.pop_front(); // remove the command from the argument list

	// Make the command lowercase, for 'case-insensitive' checking.
	STRTOLOWER(command);

	// Command doesn't exist
	ChatCommandTable::iterator itr = s_commandTable.find(command.c_str() + 1); // skip the prefix character
	if (itr == s_commandTable.end())
		return true;

	// Run the command
	return (this->*(itr->second->Handler))(vargs, message.c_str() + command.size() + 1, itr->second->Help);
}


COMMAND_HANDLER(CUser::HandleWarResultCommand) 
{
	return !isGM() ? false : g_pMain->HandleWarResultCommand(vargs, args, description); 
}
COMMAND_HANDLER(CGameServerDlg::HandleWarResultCommand)
{
	// Nation number
	if (vargs.size() < 1)
	{
		// send description
		printf("Using Sample : +warresult 1/2 (KARUS/HUMAN)\n");
		return true;
	}
	
	if (!isWarOpen())
	{
		// send description
		printf("Warning : Battle is not open.\n");
		return true;
	}

	uint8 winner_nation;
	winner_nation = atoi(vargs.front().c_str());
	
	if (winner_nation > 0 && winner_nation < 3)
		BattleZoneResult(winner_nation);
	return true;
}

bool CGameServerDlg::ProcessServerCommand(std::string & message)
{
	// Commands require at least 2 characters
	if (message.size() <= 1
		// If the prefix isn't correct
			|| message[0] != SERVER_COMMAND_PREFIX)
			// we're not a command.
			return false;

	// Split up the command by spaces
	CommandArgs vargs = StrSplit(message, " ");
	std::string command = vargs.front(); // grab the first word (the command)
	vargs.pop_front(); // remove the command from the argument list

	// Make the command lowercase, for 'case-insensitive' checking.
	STRTOLOWER(command);

	// Command doesn't exist
	ServerCommandTable::iterator itr = s_commandTable.find(command.c_str() + 1); // skip the prefix character
	if (itr == s_commandTable.end())
		return false;

	// Run the command
	return (this->*(itr->second->Handler))(vargs, message.c_str() + command.size() + 1, itr->second->Help);
}

#pragma region CGameServerDlg::HandleHelpCommand
COMMAND_HANDLER(CGameServerDlg::HandleHelpCommand)
{
	foreach(itr, s_commandTable)
	{
		if (itr->second == nullptr)
			continue;

		auto i = itr->second;
		std::string s_Command = string_format("Command: /%s, Description: %s \n", i->Name, i->Help);
		printf("%s", s_Command.c_str());
	}
	return true;
}
#pragma endregion

#pragma region CGameServerDlg::HandleResetRLoyaltyCommand
COMMAND_HANDLER(CGameServerDlg::HandleResetRLoyaltyCommand)
{
	for (int i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = g_pMain->GetUserPtr(i);
		if (!pUser) continue;

		pUser->m_iLoyaltyMonthly = 0;
		Packet result(WIZ_LOYALTY_CHANGE, uint8(LOYALTY_NATIONAL_POINTS));
		result << pUser->m_iLoyalty << pUser->m_iLoyaltyMonthly << uint32(0) << uint32(0);
		pUser->Send(&result);
	}

	Packet pkt(WIZ_DB_SAVE, uint8(ProcDbServerType::ResetLoyalty));
	g_pMain->AddDatabaseRequest(pkt);
	return true;
}
#pragma endregion

#pragma region CUser::HandleResetRLoyaltyCommand
COMMAND_HANDLER(CUser::HandleResetRLoyaltyCommand)
{
	/*if (!m_GameMastersReloadTable) { g_pMain->SendHelpDescription(this, "Unauthorized attempt! Authorization is required to use commands."
		"Please talk to Admin for Limitation of Authority."); return false; }*/

	return !isGM() ? false : g_pMain->HandleResetRLoyaltyCommand(vargs, args, description);
}
#pragma endregion

COMMAND_HANDLER(CGameServerDlg::HandleNoticeCommand)
{
	if (vargs.empty())
		return true;

	SendNotice(args);
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleNoticeallCommand)
{
	if (vargs.empty())
		return true;

	SendAnnouncement(args);
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleKillUserCommand)
{
	if (vargs.empty())
	{
		// send description
		printf("Using Sample : +kill CharacterName\n");
		return true;
	}

	std::string strUserID = vargs.front();
	CUser *pUser = GetUserPtr(strUserID, NameType::TYPE_CHARACTER);
	if (pUser == nullptr)
	{
		printf("Error : User is not online\n");
		return true;
	}

	// Disconnect the player
	pUser->goDisconnect("The command to kick the player out of the game.", __FUNCTION__);

	// send a message saying the player was disconnected
	return true;
}


COMMAND_HANDLER(CUser::HandleWar1OpenCommand) 
{ 
	if (m_GameMastersWarOpen != 1)
	{
		// send description
		g_pMain->SendHelpDescription(this, "Unauthorized attempt! Authorization is required to use commands. Please talk to Admin for Limitation of Authority.");
		return true;
	}

	return !isGM() ? false : g_pMain->HandleWar1OpenCommand(vargs, args, description); 
}
COMMAND_HANDLER(CGameServerDlg::HandleWar1OpenCommand)
{
	BattleZoneOpen(BATTLEZONE_OPEN, 1);
	return true;
}

COMMAND_HANDLER(CUser::HandleWar2OpenCommand) 
{ 
	if (m_GameMastersWarOpen != 1)
	{
		// send description
		g_pMain->SendHelpDescription(this, "Unauthorized attempt! Authorization is required to use commands. Please talk to Admin for Limitation of Authority.");
		return true;
	}

	return !isGM() ? false : g_pMain->HandleWar2OpenCommand(vargs, args, description); 
}


COMMAND_HANDLER(CGameServerDlg::HandleLotteryStart)
{
	// Char name | item ID | [stack size]
	if (vargs.size() < 1)
		return true;

	uint32 ID = atoi(vargs.front().c_str());

	_RIMA_LOTTERY_DB *pLottery = g_pMain->m_RimaLotteryArray.GetData(ID);
	if (pLottery == nullptr)
		return true;

	LotterySystemStart(ID);

	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleTopLeftCommand)
{
	m_TopLeftArray.DeleteAllData();
	LoadTopLeftTable();

	auto * TopLeft = g_pMain->m_TopLeftArray.GetData(0x01);
	if (TopLeft != nullptr)
	{
		Packet result(XSafe);
		result << uint8(XSafeOpCodes::TOPLEFT);
		result.DByte();
		result << TopLeft->Facebook << TopLeft->FacebookURL << TopLeft->Discord << TopLeft->DiscordURL << TopLeft->Live << TopLeft->LiveURL;
		result << TopLeft->ResellerURL;
		Send_All(&result);
	}
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleWar2OpenCommand)
{
	BattleZoneOpen(BATTLEZONE_OPEN, 2);
	return true;
}

COMMAND_HANDLER(CUser::HandleWar3OpenCommand) 
{
	if (m_GameMastersWarOpen != 1)
	{
		// send description
		g_pMain->SendHelpDescription(this, "Unauthorized attempt! Authorization is required to use commands. Please talk to Admin for Limitation of Authority.");
		return true;
	}

	return !isGM() ? false : g_pMain->HandleWar3OpenCommand(vargs, args, description); 
}
COMMAND_HANDLER(CGameServerDlg::HandleWar3OpenCommand)
{
	g_pMain->m_byBattleZoneType = ZONE_ARDREAM;
	BattleZoneOpen(BATTLEZONE_OPEN, 3);
	return true;
}

COMMAND_HANDLER(CUser::HandleWar4OpenCommand) 
{
	if (m_GameMastersWarOpen != 1)
	{
		// send description
		g_pMain->SendHelpDescription(this, "Unauthorized attempt! Authorization is required to use commands. Please talk to Admin for Limitation of Authority.");
		return true;
	}

	return !isGM() ? false : g_pMain->HandleWar4OpenCommand(vargs, args, description); 
}
COMMAND_HANDLER(CGameServerDlg::HandleWar4OpenCommand)
{
	BattleZoneOpen(BATTLEZONE_OPEN, 4);
	return true;
}

COMMAND_HANDLER(CUser::HandleWar5OpenCommand) 
{ 
	if (m_GameMastersWarOpen != 1)
	{
		// send description
		g_pMain->SendHelpDescription(this, "Unauthorized attempt! Authorization is required to use commands. Please talk to Admin for Limitation of Authority.");
		return true;
	}

	return !isGM() ? false : g_pMain->HandleWar5OpenCommand(vargs, args, description); 
}
COMMAND_HANDLER(CGameServerDlg::HandleWar5OpenCommand)
{
	BattleZoneOpen(BATTLEZONE_OPEN, 5);
	return true;
}

COMMAND_HANDLER(CUser::HandleWar6OpenCommand) 
{ 
	if (m_GameMastersWarOpen != 1)
	{
		// send description
		g_pMain->SendHelpDescription(this, "Unauthorized attempt! Authorization is required to use commands. Please talk to Admin for Limitation of Authority.");
		return true;
	}

	return !isGM() ? false : g_pMain->HandleWar6OpenCommand(vargs, args, description);
}
COMMAND_HANDLER(CGameServerDlg::HandleWar6OpenCommand)
{
	BattleZoneOpen(BATTLEZONE_OPEN, 6);
	return true;
}

COMMAND_HANDLER(CUser::HandleSnowWarOpenCommand)
{ 
	if (m_GameMastersWarOpen != 1)
	{
		// send description
		g_pMain->SendHelpDescription(this, "Unauthorized attempt! Authorization is required to use commands. Please talk to Admin for Limitation of Authority.");
		return true;
	}

	return !isGM() ? false : g_pMain->HandleSnowWarOpenCommand(vargs, args, description); 
}
COMMAND_HANDLER(CGameServerDlg::HandleSnowWarOpenCommand)
{
	BattleZoneOpen(SNOW_BATTLE);
	return true;
}

COMMAND_HANDLER(CUser::HandleSiegeWarOpenCommand) 
{ 
	if (m_GameMastersWarOpen != 1)
	{
		// send description
		g_pMain->SendHelpDescription(this, "Unauthorized attempt! Authorization is required to use commands. Please talk to Admin for Limitation of Authority.");
		return true;
	}

	return !isGM() ? false : g_pMain->HandleSiegeWarOpenCommand(vargs, args, description); 
}
COMMAND_HANDLER(CGameServerDlg::HandleSiegeWarOpenCommand)
{
	csw_prepareopen();
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleChaosExpansionOpen)
{
	ChaosExpansionManuelOpening();
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleBorderDefenceWar)
{
	BorderDefenceWarManuelOpening();
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleJuraidMountain)
{
	JuraidMountainManuelOpening();
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadClanPremiumTable)
{
	m_PremiumItemArray.DeleteAllData();
	LoadPremiumItemTable();

	m_PremiumItemExpArray.DeleteAllData();
	LoadPremiumItemExpTable();

	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleBeefEvent)
{
	BeefEventManuelOpening();
	return true;
}

COMMAND_HANDLER(CUser::HandleWarCloseCommand) 
{ 
	if (m_GameMastersWarClose != 1)
	{
		// send description
		g_pMain->SendHelpDescription(this, "Unauthorized attempt! Authorization is required to use commands. Please talk to Admin for Limitation of Authority.");
		return true;
	}

	return !isGM() ? false : g_pMain->HandleWarCloseCommand(vargs, args, description); 
}
COMMAND_HANDLER(CGameServerDlg::HandleWarCloseCommand)
{
	BattleZoneClose();
	return true;
}

COMMAND_HANDLER(CUser::HandleCastleSiegeWarClose)
{
	return !isGM() ? false : g_pMain->HandleCastleSiegeWarClose(vargs, args, description);
}

COMMAND_HANDLER(CGameServerDlg::HandleCastleSiegeWarClose)
{
	csw_close();
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleCindirellaWarOpen) {

	if (vargs.empty()) {
		printf("Using Sample : +cindopen settingid \n");
		return true;
	}

	int8 settingid = -1;
	if (!vargs.empty()) { settingid = atoi(vargs.front().c_str()); vargs.pop_front(); }
	if (settingid < 0 || settingid > 5) {
		printf("invalid settingid \n");
		return true;
	}
	return CindirellaCommand(true, settingid);
}

COMMAND_HANDLER(CGameServerDlg::HandleTournamentClose)
{
	// string & atoi size
	if (vargs.size() < 3)
	{
		// Send Game Server Description
		printf("Using Sample : /tournamentclose TournamentClanNameI TournamentClanNameII & TournamentStartZoneID \n");
		return true;
	}

	std::string TournamentName1 = vargs.front();
	vargs.pop_front();
	std::string TournamentName2 = vargs.front();
	vargs.pop_front();

	uint8 TournamentStartZoneID = atoi(vargs.front().c_str());

	bool SucsessZoneID = (TournamentStartZoneID == 77
		|| TournamentStartZoneID == 78
		|| TournamentStartZoneID == 96
		|| TournamentStartZoneID == 97
		|| TournamentStartZoneID == 98
		|| TournamentStartZoneID == 99);

	if (!SucsessZoneID)
	{
		// Send Game Server Description
		printf("Error: Invalid Tournament Zone(%d) \n", TournamentStartZoneID);
		return true;
	}

	if (TournamentName1.empty() || TournamentName1.size() > 21)
	{
		// Send Game Server Description
		printf("Error: TournamentName1 is empty or size > 21 \n");
		return true;
	}

	if (TournamentName2.empty() || TournamentName2.size() > 21)
	{
		// Send Game Server Description
		printf("Error: TournamentName2 is empty or size > 21 \n");
		return true;
	}

	if (TournamentName1 == TournamentName2)
	{
		// Send Game Server Description
		printf("Error: Two clan names are the same. \n");
		return true;
	}

	_TOURNAMENT_DATA* TournamentClanInfo = g_pMain->m_ClanVsDataList.GetData(TournamentStartZoneID);
	if (TournamentClanInfo == nullptr)
	{
		// Send Game Server Description
		printf("Error: Tournament is Zone(%d) is Close \n", TournamentStartZoneID);
		return true;
	}

	CKnights *pFirstClan = nullptr, *pSecondClan = nullptr;
	g_pMain->m_KnightsArray.m_lock.lock();
	foreach_stlmap_nolock(itr, g_pMain->m_KnightsArray)
	{
		if (itr->second == nullptr)
			continue;

		if (!itr->second->GetName().compare(TournamentName1))
			pFirstClan = itr->second;

		if (!itr->second->GetName().compare(TournamentName2))
			pSecondClan = itr->second;
	}
	g_pMain->m_KnightsArray.m_lock.unlock();

	if (pFirstClan == nullptr)
	{
		// Send Game Server Description
		printf("Error : Clan Tournament Close: First clan was not found in database \n");
		return true;
	}

	if (pSecondClan == nullptr)
	{
		// Send Game Server Description
		printf("Error : Clan Tournament Close: Second clan was not found in database \n");
		return true;
}

	if (TournamentClanInfo != nullptr)
	{
		KickOutZoneUsers(TournamentStartZoneID);
		g_pMain->m_ClanVsDataList.DeleteData(TournamentStartZoneID);
	}

	printf("Final : Tournament is Close: Red Clan: (%s) - (%s) :Blue Clan Tournament Zone (%d) \n", TournamentName1.c_str(), TournamentName1.c_str(), TournamentStartZoneID);
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleTournamentStart)
{

	// string & atoi size
	if (vargs.size() < 3)
	{
		// Send Game Server Description
		printf("Using Sample : /tournamentstart TournamentClanNameI TournamentClanNameII & TournamentStartZoneID \n");
		return true;
	}
	// /tournamentstart TestingOneClan TestingTwoClan 77

	std::string TournamentName1 = vargs.front();
	vargs.pop_front();
	std::string TournamentName2 = vargs.front();
	vargs.pop_front();

	uint8 TournamentStartZoneID = atoi(vargs.front().c_str());

	bool ClaniGM = false;
	bool SucsessZoneID = (TournamentStartZoneID == 77
		|| TournamentStartZoneID == 78
		|| TournamentStartZoneID == 96
		|| TournamentStartZoneID == 97
		|| TournamentStartZoneID == 98
		|| TournamentStartZoneID == 99);

	if (!SucsessZoneID)
	{
		// Send Game Server Description
		printf("Error: Invalid Tournament Zone(%d) \n", TournamentStartZoneID);
		return true;
	}

	if (TournamentName1.empty() || TournamentName1.size() > 21)
	{
		// Send Game Server Description
		printf("Error: TournamentName1 is empty or size > 21 \n");
		return true;
	}

	if (TournamentName2.empty() || TournamentName2.size() > 21)
	{
		// Send Game Server Description
		printf("Error: TournamentName2 is empty or size > 21 \n");
		return true;
	}

	if (TournamentName1.c_str() == TournamentName2.c_str())
	{
		// Send Game Server Description
		printf("Error: Two clan names are the same. \n");
		return true;
	}

	_TOURNAMENT_DATA* TournamentClanInfo = g_pMain->m_ClanVsDataList.GetData(TournamentStartZoneID);
	if (TournamentClanInfo != nullptr)
	{
		// Send Game Server Description
		printf("Error: Tournament is Zone(%d) Already Active \n", TournamentStartZoneID);
		return true;
	}

	_TOURNAMENT_DATA* Tour77 = g_pMain->m_ClanVsDataList.GetData(77);
	_TOURNAMENT_DATA* Tour78 = g_pMain->m_ClanVsDataList.GetData(78);
	_TOURNAMENT_DATA* Tour96 = g_pMain->m_ClanVsDataList.GetData(96);
	_TOURNAMENT_DATA* Tour97 = g_pMain->m_ClanVsDataList.GetData(97);
	_TOURNAMENT_DATA* Tour98 = g_pMain->m_ClanVsDataList.GetData(98);
	_TOURNAMENT_DATA* Tour99 = g_pMain->m_ClanVsDataList.GetData(99);

	CKnights* pAlreadyFirstClan = nullptr, * pAlreadySecondClan = nullptr;
	foreach_stlmap(itr, g_pMain->m_KnightsArray)
	{
		if (itr->second == nullptr)
			continue;

		if (!itr->second->GetName().compare(TournamentName1))
			pAlreadyFirstClan = itr->second;

		if (!itr->second->GetName().compare(TournamentName2))
			pAlreadySecondClan = itr->second;
	}

	if (pAlreadyFirstClan != nullptr)
	{
		if (Tour77 != nullptr)
		{
			if (pAlreadyFirstClan->GetID() == Tour77->aTournamentClanNum[0]
				|| pAlreadyFirstClan->GetID() == Tour77->aTournamentClanNum[1])
			{
				// Send Game Server Description
				printf("Error: Second Specified clan (%s) is fighting against another clan. Other ZoneID: (%d) \n", pAlreadyFirstClan->GetName().c_str(), Tour77->aTournamentZoneID);
				return true;

			}
		}

		if (Tour78 != nullptr)
		{
			if (pAlreadyFirstClan->GetID() == Tour78->aTournamentClanNum[0]
				|| pAlreadyFirstClan->GetID() == Tour78->aTournamentClanNum[1])
			{
				// Send Game Server Description
				printf("Error: Second Specified clan (%s) is fighting against another clan. Other ZoneID: (%d) \n", pAlreadyFirstClan->GetName().c_str(), Tour78->aTournamentZoneID);
				return true;

			}
		}

		if (Tour96 != nullptr)
		{
			if (pAlreadyFirstClan->GetID() == Tour96->aTournamentClanNum[0]
				|| pAlreadyFirstClan->GetID() == Tour96->aTournamentClanNum[1])
			{
				// Send Game Server Description
				printf("Error: Second Specified clan (%s) is fighting against another clan. Other ZoneID: (%d) \n", pAlreadyFirstClan->GetName().c_str(), Tour96->aTournamentZoneID);
				return true;

			}
		}

		if (Tour97 != nullptr)
		{
			if (pAlreadyFirstClan->GetID() == Tour97->aTournamentClanNum[0]
				|| pAlreadyFirstClan->GetID() == Tour97->aTournamentClanNum[1])
			{
				// Send Game Server Description
				printf("Error: Second Specified clan (%s) is fighting against another clan. Other ZoneID: (%d) \n", pAlreadyFirstClan->GetName().c_str(), Tour97->aTournamentZoneID);
				return true;

			}
		}

		if (Tour98 != nullptr)
		{
			if (pAlreadyFirstClan->GetID() == Tour98->aTournamentClanNum[0]
				|| pAlreadyFirstClan->GetID() == Tour98->aTournamentClanNum[1])
			{
				// Send Game Server Description
				printf("Error: Second Specified clan (%s) is fighting against another clan. Other ZoneID: (%d) \n", pAlreadyFirstClan->GetName().c_str(), Tour98->aTournamentZoneID);
				return true;

			}
		}

		if (Tour99 != nullptr)
		{
			if (pAlreadyFirstClan->GetID() == Tour99->aTournamentClanNum[0]
				|| pAlreadyFirstClan->GetID() == Tour99->aTournamentClanNum[1])
			{
				// Send Game Server Description
				printf("Error: Second Specified clan (%s) is fighting against another clan. Other ZoneID: (%d) \n", pAlreadyFirstClan->GetName().c_str(), Tour99->aTournamentZoneID);
				return true;

			}
		}
	}

	if (pAlreadySecondClan != nullptr)
	{
		if (Tour77 != nullptr)
		{
			if (pAlreadySecondClan->GetID() == Tour77->aTournamentClanNum[0]
				|| pAlreadySecondClan->GetID() == Tour77->aTournamentClanNum[1])
			{
				// Send Game Server Description
				printf("Error: Second Specified clan (%s) is fighting against another clan. Other ZoneID: (%d) \n", pAlreadySecondClan->GetName().c_str(), Tour77->aTournamentZoneID);
				return true;

			}
		}

		if (Tour78 != nullptr)
		{
			if (pAlreadySecondClan->GetID() == Tour78->aTournamentClanNum[0]
				|| pAlreadySecondClan->GetID() == Tour78->aTournamentClanNum[1])
			{
				// Send Game Server Description
				printf("Error: Second Specified clan (%s) is fighting against another clan. Other ZoneID: (%d) \n", pAlreadySecondClan->GetName().c_str(), Tour78->aTournamentZoneID);
				return true;

			}
		}

		if (Tour96 != nullptr)
		{
			if (pAlreadySecondClan->GetID() == Tour96->aTournamentClanNum[0]
				|| pAlreadySecondClan->GetID() == Tour96->aTournamentClanNum[1])
			{
				// Send Game Server Description
				printf("Error: Second Specified clan (%s) is fighting against another clan. Other ZoneID: (%d) \n", pAlreadySecondClan->GetName().c_str(), Tour96->aTournamentZoneID);
				return true;

			}
		}

		if (Tour97 != nullptr)
		{
			if (pAlreadySecondClan->GetID() == Tour97->aTournamentClanNum[0]
				|| pAlreadySecondClan->GetID() == Tour97->aTournamentClanNum[1])
			{
				// Send Game Server Description
				printf("Error: Second Specified clan (%s) is fighting against another clan. Other ZoneID: (%d) \n", pAlreadySecondClan->GetName().c_str(), Tour97->aTournamentZoneID);
				return true;

			}
		}

		if (Tour98 != nullptr)
		{
			if (pAlreadySecondClan->GetID() == Tour98->aTournamentClanNum[0]
				|| pAlreadySecondClan->GetID() == Tour98->aTournamentClanNum[1])
			{
				// Send Game Server Description
				printf("Error: Second Specified clan (%s) is fighting against another clan. Other ZoneID: (%d) \n", pAlreadySecondClan->GetName().c_str(), Tour98->aTournamentZoneID);
				return true;

			}
		}

		if (Tour99 != nullptr)
		{
			if (pAlreadySecondClan->GetID() == Tour99->aTournamentClanNum[0]
				|| pAlreadySecondClan->GetID() == Tour99->aTournamentClanNum[1])
			{
				// Send Game Server Description
				printf("Error: Second Specified clan (%s) is fighting against another clan. Other ZoneID: (%d) \n", pAlreadySecondClan->GetName().c_str(), Tour99->aTournamentZoneID);
				return true;

			}
		}
	}

	bool isClanVs = (TournamentStartZoneID == 77 || TournamentStartZoneID == 78);
	bool isPartyVs = (TournamentStartZoneID == 96 || TournamentStartZoneID == 97 || TournamentStartZoneID == 98 || TournamentStartZoneID == 99);
	if (isClanVs)
	{
		CKnights* pFirstClan = nullptr, * pSecondClan = nullptr;
		g_pMain->m_KnightsArray.m_lock.lock();
		foreach_stlmap_nolock(itr, g_pMain->m_KnightsArray)
		{
			if (itr->second == nullptr)
				continue;

			if (!itr->second->GetName().compare(TournamentName1))
				pFirstClan = itr->second;

			if (!itr->second->GetName().compare(TournamentName2))
				pSecondClan = itr->second;
		}
		g_pMain->m_KnightsArray.m_lock.unlock();

		if (pFirstClan == nullptr)
		{
			// Send Game Server Description
			printf("Error : Clan Tournament: First clan was not found in database \n");
			return true;
		}

		if (pSecondClan == nullptr)
		{
			// Send Game Server Description
			printf("Error : Clan Tournament: Second clan was not found in database \n");
			return true;
		}

		uint16 FirstClanOnlineCount = 0;
		pFirstClan->m_arKnightsUser.m_lock.lock();
		foreach_stlmap_nolock(itr, pFirstClan->m_arKnightsUser)
		{
			_KNIGHTS_USER* pFirstClanKnightUser = itr->second;
			if (pFirstClanKnightUser == nullptr)
				continue;

			CUser* pFirstClanUser = g_pMain->GetUserPtr(pFirstClanKnightUser->strUserName, NameType::TYPE_CHARACTER);
			if (pFirstClanUser == nullptr)
				continue;

			if (pFirstClanUser->isGM())
			{
				ClaniGM = true;
				break;
			}

			if (pFirstClanUser != nullptr
				&& pFirstClanUser->isInGame()
				&& pFirstClanUser->GetZoneID() != TournamentStartZoneID)
				FirstClanOnlineCount++;
		}
		pFirstClan->m_arKnightsUser.m_lock.unlock();

		if (ClaniGM == true)
		{
			// Send Game Server Description
			printf("Error : Clan Tournament: There is an administrator in Clan (%s) \n", TournamentName1.c_str());
			return true;
		}

		if (FirstClanOnlineCount > TOURNAMENT_MAX_CLAN_COUNT)
		{
			// Send Game Server Description
			printf("Error : Clan Tournament: %s Clan is clan high count. (count: %d) Maxmium Count: (%d)\n", TournamentName1.c_str(), FirstClanOnlineCount, TOURNAMENT_MAX_CLAN_COUNT);
			return true;
		}

		if (FirstClanOnlineCount < TOURNAMENT_MIN_CLAN_COUNT)
		{
			// Send Game Server Description
			printf("Error : Clan Tournament: %s Clan is clan low count. (count: %d) Minumum Count: (%d)\n", TournamentName1.c_str(), FirstClanOnlineCount, TOURNAMENT_MIN_CLAN_COUNT);
			return true;
		}

		uint16 SecondClanOnlineCount = 0;
		pSecondClan->m_arKnightsUser.m_lock.lock();
		foreach_stlmap_nolock(itr, pSecondClan->m_arKnightsUser)
		{
			_KNIGHTS_USER* pSecondClanKnightUser = itr->second;
			if (pSecondClanKnightUser == nullptr)
				continue;

			CUser* pSecondClanUser = g_pMain->GetUserPtr(pSecondClanKnightUser->strUserName, NameType::TYPE_CHARACTER);
			if (pSecondClanUser == nullptr)
				continue;

			if (pSecondClanUser->isGM())
			{
				ClaniGM = true;
				break;
			}

			if (pSecondClanUser != nullptr
				&& pSecondClanUser->isInGame()
				&& pSecondClanUser->GetZoneID() != TournamentStartZoneID)
				SecondClanOnlineCount++;
		}
		pSecondClan->m_arKnightsUser.m_lock.unlock();

		if (ClaniGM == true)
		{
			// Send Game Server Description
			printf("Error : Clan Tournament: There is an administrator in Clan (%s) \n", TournamentName2.c_str());
			return true;
		}

		if (SecondClanOnlineCount > TOURNAMENT_MAX_CLAN_COUNT)
		{
			// Send Game Server Description
			printf("Error : Clan Tournament: %s Clan is clan high count. (count: %d) Maxmium Count: (%d) \n", TournamentName2.c_str(), SecondClanOnlineCount, TOURNAMENT_MAX_CLAN_COUNT);
			return true;
		}

		if (SecondClanOnlineCount < TOURNAMENT_MIN_CLAN_COUNT)
		{
			// Send Game Server Description
			printf("Error : Clan Tournament: %s Clan is clan low count. (count: %d) Minumum Count: (%d) \n", TournamentName2.c_str(), SecondClanOnlineCount, TOURNAMENT_MIN_CLAN_COUNT);
			return true;
		}

		KickOutZoneUsers(TournamentStartZoneID);

		if (TournamentClanInfo == nullptr)
		{
			_TOURNAMENT_DATA* cTournamentSign = new _TOURNAMENT_DATA;
			cTournamentSign->aTournamentZoneID = TournamentStartZoneID;
			cTournamentSign->aTournamentClanNum[0] = pFirstClan->GetID();
			cTournamentSign->aTournamentClanNum[1] = pSecondClan->GetID();
			cTournamentSign->aTournamentScoreBoard[0] = 0;
			cTournamentSign->aTournamentScoreBoard[1] = 0;
			cTournamentSign->aTournamentTimer = 30 * 60;
			cTournamentSign->aTournamentMonumentKilled = 0;
			cTournamentSign->aTournamentOutTimer = UNIXTIME;
			cTournamentSign->aTournamentisAttackable = false;
			cTournamentSign->aTournamentisStarted = true;
			cTournamentSign->aTournamentisFinished = false;

			if (!m_ClanVsDataList.PutData(cTournamentSign->aTournamentZoneID, cTournamentSign))
				delete cTournamentSign;
		}

		pFirstClan->m_arKnightsUser.m_lock.lock();
		foreach_stlmap_nolock(itr, pFirstClan->m_arKnightsUser)
		{
			_KNIGHTS_USER* pFirstClanKnightUser = itr->second;
			if (pFirstClanKnightUser == nullptr)
				continue;

			CUser* pFirstClanUser = g_pMain->GetUserPtr(pFirstClanKnightUser->strUserName, NameType::TYPE_CHARACTER);
			if (pFirstClanUser == nullptr
				|| !pFirstClanUser->isInGame()
				|| pFirstClanUser->GetZoneID() == TournamentStartZoneID)
				continue;

			pFirstClanUser->ZoneChange(TournamentStartZoneID, 0.0f, 0.0f);
		}
		pFirstClan->m_arKnightsUser.m_lock.unlock();

		pSecondClan->m_arKnightsUser.m_lock.lock();
		foreach_stlmap_nolock(itr, pSecondClan->m_arKnightsUser)
		{
			_KNIGHTS_USER* pSecondClanKnightUser = itr->second;
			if (pSecondClanKnightUser == nullptr)
				continue;

			CUser* pSecondClanUser = g_pMain->GetUserPtr(pSecondClanKnightUser->strUserName, NameType::TYPE_CHARACTER);
			if (pSecondClanUser == nullptr
				|| !pSecondClanUser->isInGame()
				|| pSecondClanUser->GetZoneID() == TournamentStartZoneID)
				continue;

			pSecondClanUser->ZoneChange(TournamentStartZoneID, 0.0f, 0.0f);
		}
		pSecondClan->m_arKnightsUser.m_lock.unlock();
	}
	else if (isPartyVs)
	{
		CKnights* pFirstClanParty = nullptr, * pSecondClanParty = nullptr;
		g_pMain->m_KnightsArray.m_lock.lock();
		foreach_stlmap(itr, g_pMain->m_KnightsArray)
		{
			if (itr->second == nullptr)
				continue;

			if (!itr->second->GetName().compare(TournamentName1))
				pFirstClanParty = itr->second;

			if (!itr->second->GetName().compare(TournamentName2))
				pSecondClanParty = itr->second;
		}
		g_pMain->m_KnightsArray.m_lock.unlock();

		if (pFirstClanParty == nullptr)
		{
			// Send Game Server Description
			printf("Error :Party Tournament: First clan was not found in database \n");
			return true;
		}

		if (pSecondClanParty == nullptr)
		{
			// Send Game Server Description
			printf("Error :Party Tournament: Second clan was not found in database \n");
			return true;
		}

		uint16 FirstClanOnlineCount = 0;
		pFirstClanParty->m_arKnightsUser.m_lock.lock();
		foreach_stlmap_nolock(itr, pFirstClanParty->m_arKnightsUser)
		{
			_KNIGHTS_USER* pFirstClanPartyKnightUser = itr->second;
			if (pFirstClanPartyKnightUser == nullptr)
				continue;

			CUser* pFirstClanUser = g_pMain->GetUserPtr(pFirstClanPartyKnightUser->strUserName, NameType::TYPE_CHARACTER);
			if (pFirstClanUser != nullptr
				&& pFirstClanUser->isInGame()
				&& pFirstClanUser->GetZoneID() != TournamentStartZoneID)
				FirstClanOnlineCount++;
		}
		pFirstClanParty->m_arKnightsUser.m_lock.unlock();

		if (FirstClanOnlineCount > TOURNAMENT_MAX_PARTY_COUNT)
		{
			// Send Game Server Description
			printf("Error : Party Tournament: %s Clan is clan high count. (count: %d) Maximum Count (%d) \n", TournamentName1.c_str(), FirstClanOnlineCount, TOURNAMENT_MAX_PARTY_COUNT);
			return true;
		}

		if (FirstClanOnlineCount < TOURNAMENT_MIN_PARTY_COUNT)
		{
			// Send Game Server Description
			printf("Error : Party Tournament: %s Clan is clan low count. (count: %d) Minumum Count (%d)\n", TournamentName1.c_str(), FirstClanOnlineCount, TOURNAMENT_MIN_PARTY_COUNT);
			return true;
		}

		uint16 SecondClanOnlineCount = 0;
		pSecondClanParty->m_arKnightsUser.m_lock.lock();
		foreach_stlmap_nolock(itr, pSecondClanParty->m_arKnightsUser)
		{
			_KNIGHTS_USER* pSecondClanPartyKnightUser = itr->second;
			if (pSecondClanPartyKnightUser == nullptr)
				continue;

			CUser* pSecondClanUser = g_pMain->GetUserPtr(pSecondClanPartyKnightUser->strUserName, NameType::TYPE_CHARACTER);
			if (pSecondClanUser != nullptr
				&& pSecondClanUser->isInGame()
				&& pSecondClanUser->GetZoneID() != TournamentStartZoneID)
				SecondClanOnlineCount++;
		}
		pSecondClanParty->m_arKnightsUser.m_lock.unlock();

		if (SecondClanOnlineCount > TOURNAMENT_MAX_PARTY_COUNT)
		{
			// Send Game Server Description
			printf("Error : Party Tournament: %s Clan is clan high count. (count: %d) Maximum Count (%d) \n", TournamentName2.c_str(), SecondClanOnlineCount, TOURNAMENT_MAX_PARTY_COUNT);
			return true;
		}

		if (SecondClanOnlineCount < TOURNAMENT_MIN_PARTY_COUNT)
		{
			// Send Game Server Description
			printf("Error : Party Tournament: %s Clan is clan low count. (count: %d) Minumum Count (%d)\n", TournamentName2.c_str(), SecondClanOnlineCount, TOURNAMENT_MIN_PARTY_COUNT);
			return true;
		}

		KickOutZoneUsers(TournamentStartZoneID);

		if (TournamentClanInfo == nullptr)
		{
			_TOURNAMENT_DATA* cTournamentSign = new _TOURNAMENT_DATA;
			cTournamentSign->aTournamentZoneID = TournamentStartZoneID;
			cTournamentSign->aTournamentClanNum[0] = pFirstClanParty->GetID();
			cTournamentSign->aTournamentClanNum[1] = pSecondClanParty->GetID();
			cTournamentSign->aTournamentScoreBoard[0] = 0;
			cTournamentSign->aTournamentScoreBoard[1] = 0;
			cTournamentSign->aTournamentTimer = 0;
			cTournamentSign->aTournamentMonumentKilled = 0;
			cTournamentSign->aTournamentOutTimer = UNIXTIME;
			cTournamentSign->aTournamentisAttackable = false;
			cTournamentSign->aTournamentisStarted = true;
			cTournamentSign->aTournamentisFinished = false;

			if (!m_ClanVsDataList.PutData(cTournamentSign->aTournamentZoneID, cTournamentSign))
				delete cTournamentSign;
		}

		pFirstClanParty->m_arKnightsUser.m_lock.lock();
		foreach_stlmap_nolock(itr, pFirstClanParty->m_arKnightsUser)
		{
			_KNIGHTS_USER* pFirstClanPartyKnightUser = itr->second;
			if (pFirstClanPartyKnightUser == nullptr)
				continue;

			CUser* pFirstClanUser = g_pMain->GetUserPtr(pFirstClanPartyKnightUser->strUserName, NameType::TYPE_CHARACTER);
			if (pFirstClanUser == nullptr
				|| !pFirstClanUser->isInGame()
				|| pFirstClanUser->GetZoneID() == TournamentStartZoneID)
				continue;

			pFirstClanUser->ZoneChange(TournamentStartZoneID, 0.0f, 0.0f);
		}
		pFirstClanParty->m_arKnightsUser.m_lock.unlock();

		pSecondClanParty->m_arKnightsUser.m_lock.lock();
		foreach_stlmap_nolock(itr, pSecondClanParty->m_arKnightsUser)
		{
			_KNIGHTS_USER* pSecondClanPartyKnightUser = itr->second;
			if (pSecondClanPartyKnightUser == nullptr)
				continue;

			CUser* pSecondClanUser = g_pMain->GetUserPtr(pSecondClanPartyKnightUser->strUserName, NameType::TYPE_CHARACTER);
			if (pSecondClanUser == nullptr
				|| !pSecondClanUser->isInGame()
				|| pSecondClanUser->GetZoneID() == TournamentStartZoneID)
				continue;

			pSecondClanUser->ZoneChange(TournamentStartZoneID, 0.0f, 0.0f);
		}
		pSecondClanParty->m_arKnightsUser.m_lock.unlock();

	}
	printf("Final : Tournament is Start: Red Clan: (%s) - (%s) :Blue Clan Tournament Zone (%d) \n", TournamentName1.c_str(), TournamentName1.c_str(), TournamentStartZoneID);
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleEventUnderTheCastleCommand)
{
	// Nation number
	if (vargs.size() < 1)
	{
		// send description
		printf("Using Sample : +underthecastle 1/2 (Open/Close)\n");
		return true;
	}
	string chatstr;
	uint8 type;
	type = atoi(vargs.front().c_str());

	if (type == 1)
	{
		m_bUnderTheCastleIsActive = true;
		m_bUnderTheCastleMonster = true;
		m_nUnderTheCastleEventTime = 180 * MINUTE;
		GetServerResource(IDS_UNDER_THE_CASTLE_OPEN, &chatstr);
		g_pMain->SendAnnouncement(chatstr.c_str());
		//g_pMain->SendAnnouncement("Under the Castle has opened. You may now enter Under the Castle"); // UTC Acilis Notice Düzeltildi.
		printf("Under The Castle Stard\n");
	}

	if (type == 2)
	{
		m_nUnderTheCastleEventTime = 10;
		GetServerResource(IDS_UNDER_THE_CASTLE_VICTORY, &chatstr);
		g_pMain->SendAnnouncement(chatstr.c_str());
		printf("Under The Castle Closed\n");
	}
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleTeleportAllCommand)
{
	// Zone number
	if (vargs.size() < 1)
	{
		// send description
		printf("Using Sample : /tp_all ZoneNumber | /tp_all ZoneNumber TargetZoneNumber.\n");
		return true;
	}

	int nZoneID = 0;
	int nTargetZoneID = 0;

	if (vargs.size() == 1)
		nZoneID = atoi(vargs.front().c_str());

	if (vargs.size() == 2)
	{
		nZoneID = atoi(vargs.front().c_str());
		vargs.pop_front();
		nTargetZoneID = atoi(vargs.front().c_str());
	}

	if (nZoneID > 0 || nTargetZoneID > 0)
		g_pMain->KickOutZoneUsers(nZoneID,nTargetZoneID);
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleShutdownCommand)
{
	if (m_Shutdownstart) { printf("Server Shut Down in process..!!\n"); return true; }
	m_Shutdownfinishtime = UNIXTIME + 1;
	m_Shutdownstart = true; m_ShutdownKickStart = false;

	printf("Server Shut Down in 1 minutes \n");
	Packet result(WIZ_LOGOSSHOUT, uint8(2));
	result << uint8(6) << uint8(1);
	Send_All(&result);
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleDiscountCommand)
{
	m_sDiscount = 1;
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleGlobalDiscountCommand)
{
	m_sDiscount = 2;
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleDiscountOffCommand)
{
	m_sDiscount = 0;
	return true;
}

COMMAND_HANDLER(CUser::HandleCaptainCommand) { return !isGM() ? false : g_pMain->HandleCaptainCommand(vargs, args, description); }
COMMAND_HANDLER(CGameServerDlg::HandleCaptainCommand)
{
	BattleZoneSelectCommanders();
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleSantaCommand)
{
	m_bSantaOrAngel = FLYING_SANTA;
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleSantaOffCommand)
{
	m_bSantaOrAngel = FLYING_NONE;
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleAngelCommand)
{
	m_bSantaOrAngel = FLYING_ANGEL;
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandlePermanentChatCommand)
{
	if (vargs.empty())
	{
		// send error saying we need args (unlike the previous implementation of this command)
		return true;
	}

	SetPermanentMessage("%s", args);
	return true;
}

void CGameServerDlg::SendHelpDescription(CUser *pUser, std::string sHelpMessage)
{
	if (pUser == nullptr || sHelpMessage == "")
		return;

	Packet result(WIZ_CHAT, (uint8)ChatType::PUBLIC_CHAT);
	result << pUser->GetNation() << pUser->GetSocketID() << uint8(0) << sHelpMessage;
	pUser->Send(&result);
}

void CGameServerDlg::SendInfoMessage(CUser *pUser, std::string sHelpMessage, uint8 type)
{
	if (pUser == nullptr || sHelpMessage == "")
		return;

	Packet result(WIZ_CHAT, type);
	result << pUser->GetNation() << pUser->GetSocketID() << uint8(0) << sHelpMessage;
	pUser->Send(&result);
}

void CGameServerDlg::SetPermanentMessage(const char * format, ...)
{
	char buffer[128];
	va_list ap;
	va_start(ap, format);
	vsnprintf(buffer, 128, format, ap);
	va_end(ap);

	m_bPermanentChatMode = true;
	m_strPermanentChat = buffer;

	Packet result;
	ChatPacket::Construct(&result, (uint8)ChatType::PERMANENT_CHAT, &m_strPermanentChat);
	Send_All(&result);
}

COMMAND_HANDLER(CGameServerDlg::HandlePermanentChatOffCommand)
{
	Packet result;
	ChatPacket::Construct(&result, (uint8)ChatType::END_PERMANENT_CHAT);
	m_bPermanentChatMode = false;
	Send_All(&result);
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadBonusNotice)
{
	for (uint16 i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = g_pMain->GetUserPtr(i);
		if (pUser == nullptr
			|| !pUser->isInGame()
			|| !pUser->isInClan())
			continue;

		CKnights* pKnights = g_pMain->GetClanPtr(pUser->GetClanID());
		if (pKnights == nullptr)
			continue;

		if (pKnights->isCastellanCape()) {
			auto* pKnightCape = g_pMain->m_KnightsCapeArray.GetData(pKnights->m_castCapeID);
			if (pKnightCape && pKnightCape->BonusType > 0) pUser->SendCapeBonusNotice();
		}
		else {
			auto* pKnightCape = g_pMain->m_KnightsCapeArray.GetData(pKnights->GetCapeID());
			if (pKnightCape && pKnightCape->BonusType > 0) pUser->SendCapeBonusNotice();
		}
	}

	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadNoticeCommand)
{
	// Reload the notice data
	LoadNoticeData();
	LoadNoticeUpData();

	for (uint16 i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = g_pMain->GetUserPtr(i);
		if (pUser == nullptr || !pUser->isInGame())
			continue;

		pUser->SendNotice();
		pUser->TopSendNotice();
	}

	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadDrakiTowerTables)
{
	m_DrakiMonsterListArray.DeleteAllData();
	m_DrakiRoomListArray.DeleteAllData();
	LoadDrakiTowerTables();
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadDungeonDefenceTables)
{
	m_DungeonDefenceMonsterListArray.DeleteAllData();
	LoadDungeonDefenceMonsterTable();

	m_DungeonDefenceStageListArray.DeleteAllData();
	LoadDungeonDefenceStageTable();
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadUpgradeCommand) {
	m_ItemUpgradeArray.DeleteAllData();
	LoadItemUpgradeTable();
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadRankBugCommand) {
	LoadRankBugTable();
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadLevelRewardCommand) {
	
	tar_levelreward = true;
	m_LevelRewardArray.DeleteAllData();
	LoadLevelRewardTable();
	tar_levelreward = false;
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadMerchantLevelRewardCommand) {
	tar_levelmercreward = true;
	m_LevelMerchantRewardArray.DeleteAllData();
	LoadLevelMerchantRewardTable();
	tar_levelmercreward = false;
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadZoneOnlineRewardCommand) {

	m_ZoneOnlineRewardReload = true;
	m_ZoneOnlineRewardArrayLock.lock();
	m_ZoneOnlineRewardArray.clear();
	LoadZoneOnlineRewardTable();
	std::vector<_ZONE_ONLINE_REWARD> copymap = g_pMain->m_ZoneOnlineRewardArray;
	m_ZoneOnlineRewardArrayLock.unlock();

	for (uint16 i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = g_pMain->GetUserPtr(i);
		if (pUser == nullptr || !pUser->isInGame())
			continue;
		
		pUser->m_ZoneOnlineRewardLock.lock();
		pUser->m_ZoneOnlineReward.clear();
		pUser->m_ZoneOnlineReward = copymap;
		pUser->m_ZoneOnlineRewardLock.unlock();
	}
	m_ZoneOnlineRewardReload = false;
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadTablesCommand)
{
	m_StartPositionArray.DeleteAllData();
	LoadStartPositionTable();

	m_StartPositionRandomArray.DeleteAllData();
	LoadStartPositionRandomTable();

	m_ItemExchangeArray.DeleteAllData();
	LoadItemExchangeTable();

	m_ItemExchangeExpArray.DeleteAllData();
	LoadItemExchangeExpTable();

	m_ItemSpecialExchangeArray.DeleteAllData();
	LoadItemSpecialExchangeTable();

	m_ItemExchangeCrashArray.DeleteAllData();
	LoadItemExchangeCrashTable();

	m_EventTriggerArray.DeleteAllData();
	LoadEventTriggerTable();

	m_ServerResourceArray.DeleteAllData();
	LoadServerResourceTable();

	m_MonsterResourceArray.DeleteAllData();
	LoadMonsterResourceTable();

	m_MonsterChallengeArray.DeleteAllData();
	LoadMonsterChallengeTable();

	m_MonsterChallengeSummonListArray.DeleteAllData();
	LoadMonsterChallengeSummonListTable();

	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadTables2Command)
{
	m_RimaLotteryArray.DeleteAllData();
	LoadRimaLotteryEventTable();

	m_WarBanishOfWinnerArray.DeleteAllData();
	LoadBanishWinnerTable();

	m_DungeonDefenceMonsterListArray.DeleteAllData();
	LoadDungeonDefenceMonsterTable();

	m_DungeonDefenceStageListArray.DeleteAllData();
	LoadDungeonDefenceStageTable();

	m_DrakiMonsterListArray.DeleteAllData();
	m_DrakiRoomListArray.DeleteAllData();
	LoadDrakiTowerTables();

	m_LuaGiveItemExchangeArray.DeleteAllData();
	LoadGiveItemExchangeTable();
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadAllTabCommand) {
	g_pMain->HandleReloadNoticeCommand(vargs, args, description);
	g_pMain->HandleReloadTablesCommand(vargs, args, description);
	g_pMain->HandleReloadTables2Command(vargs, args, description);
	g_pMain->HandleReloadTables3Command(vargs, args, description);
	g_pMain->HandleReloadMagicsCommand(vargs, args, description);
	g_pMain->HandleReloadQuestCommand(vargs, args, description);
	g_pMain->HandleReloadRanksCommand(vargs, args, description);
	g_pMain->HandleReloadDropsCommand(vargs, args, description);
	g_pMain->HandleReloadDropsRandomCommand(vargs, args, description);
	g_pMain->HandleReloadKingsCommand(vargs, args, description);
	g_pMain->HandleReloadRightTopTitleCommand(vargs, args, description);
	g_pMain->HandleReloadPusItemCommand(vargs, args, description);
	g_pMain->HandleReloadDungeonDefenceTables(vargs, args, description);
	g_pMain->HandleReloadDrakiTowerTables(vargs, args, description);
	g_pMain->HandleEventScheduleResetTable(vargs, args, description);
	g_pMain->HandleTopLeftCommand(vargs, args, description);
	g_pMain->HandleReloadBonusNotice(vargs, args, description);
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadTables3Command)
{
	m_MonsterRespawnLoopArray.DeleteAllData();
	LoadMonsterRespawnLoopListTable();

	m_MonsterSummonList.DeleteAllData();
	LoadMonsterSummonListTable();

	m_MonsterUnderTheCastleArray.DeleteAllData();
	LoadMonsterUnderTheCastleTable();

	m_MonsterStoneListInformationArray.DeleteAllData();
	LoadMonsterStoneListInformationTable();

	m_JuraidMountionListInformationArray.DeleteAllData();
	LoadJuraidMountionListInformationTable();

	m_ChaosStoneSummonListArray.DeleteAllData();
	LoadChaosStoneMonsterListTable();

	m_ChaosStoneRespawnCoordinateArray.DeleteAllData();
	LoadChaosStoneCoordinateTable();

	m_ChaosStoneStageArray.DeleteAllData();
	LoadChaosStoneStage();

	m_MiningExchangeArray.DeleteAllData();
	LoadMiningExchangeListTable();

	m_MiningFishingItemArray.DeleteAllData();
	LoadMiningFishingItemTable();

	m_ItemOpArray.DeleteAllData();
	LoadItemOpTable();

	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadPusItemCommand) 
{
	m_PusItemArray.DeleteAllData();
	LoadPusItemsTable();
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadMagicsCommand)
{
	m_IsMagicTableInUpdateProcess = true;
	m_MagictableArray.DeleteAllData();
	m_Magictype1Array.DeleteAllData();
	m_Magictype2Array.DeleteAllData();
	m_Magictype3Array.DeleteAllData();
	m_Magictype4Array.DeleteAllData();
	m_Magictype5Array.DeleteAllData();
	m_Magictype6Array.DeleteAllData();
	m_Magictype8Array.DeleteAllData();
	m_Magictype9Array.DeleteAllData();
	LoadMagicTable();
	LoadMagicType1();
	LoadMagicType2();
	LoadMagicType3();
	LoadMagicType4();
	LoadMagicType5();
	LoadMagicType6();
	LoadMagicType7();
	LoadMagicType8();
	LoadMagicType9();
	m_IsMagicTableInUpdateProcess = false;

	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleEventScheduleResetTable)
{
	if (pTempleEvent.ActiveEvent != -1 || m_byBattleOpen != NO_BATTLE || pBeefEvent.isActive)
	{
		printf("ongoing event Warning\n");
		return true;
	}

	if (pTempleEvent.ActiveEvent != -1 || m_byBattleOpen != NO_BATTLE
		|| pBeefEvent.isActive || pForgettenTemple.isActive) {
		printf("ongoing event Warning\n");
		return true;
	}

	g_pMain->pEventTimeOpt.Initialize();
	XCodeLoadEventTables();
	XCodeLoadEventVroomTables();

	m_BeefEventPlayTimerArray.DeleteAllData();
	LoadBeefEventPlayTimerTable();

	m_EventTimerShowArray.DeleteAllData();
	LoadEventTimerShowTable();
	EventTimerSet();
	
	for (uint16 i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = g_pMain->GetUserPtr(i);
		if (pUser == nullptr)
			continue;
		if (!pUser->isInGame())
			continue;

		pUser->SendEventTimerList();
	}
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadQuestCommand)
{
	m_QuestHelperArray.DeleteAllData();
	LoadQuestHelperTable();
	m_QuestMonsterArray.DeleteAllData();
	LoadQuestMonsterTable();

	m_DailyQuestArray.DeleteAllData();
	LoadDailyQuestListTable();
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadRanksCommand)
{
	ReloadKnightAndUserRanks(true);
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadDropsCommand)
{
	m_MakeItemGroupArray.DeleteAllData();
	LoadMakeItemGroupTable();
	m_NpcItemArray.DeleteAllData();
	LoadNpcItemTable();
	m_MonsterItemArray.DeleteAllData();
	LoadMonsterItemTable();
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadDropsRandomCommand)
{
	m_randomtable_reload = true;
	m_MakeItemGroupRandomArray.DeleteAllData();
	LoadMakeItemGroupRandomTable();
	m_randomtable_reload = false;
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadKingsCommand)
{
	m_KingSystemArray.DeleteAllData();
	LoadKingSystem();
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadRightTopTitleCommand) 
{
	for (uint16 i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = g_pMain->GetUserPtr(i);
		if (pUser == nullptr || !pUser->isInGame()) continue;
		pUser->RightTopTitleMsgDelete();
	}

	m_RightTopTitleArray.DeleteAllData();
	LoadRightTopTitleTable();

	for (uint16 i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = g_pMain->GetUserPtr(i);
		if (pUser == nullptr || !pUser->isInGame())
			continue;

		pUser->RightTopTitleMsg();
	}

	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadItemsCommand)
{
	m_ItemtableArray.DeleteAllData();
	LoadItemTable();
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadItems)
{
	ReLoadItemTable();
	printf("[ITEM] Table Reload\n");
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleCountCommand)
{
	uint16 count = 0;
	for (uint16 i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = g_pMain->GetUserPtr(i);
		if (pUser == nullptr || !pUser->isInGame())
			continue;

		count++;
	}
	
	m_BotcharacterNameLock.lock();
	count += (uint16)g_pMain->m_BotcharacterNameMap.size();
	m_BotcharacterNameLock.unlock();

	printf("Online User Count : %d\n", count);
	return true;
 }

void CGameServerDlg::SendFormattedResource(uint32 nResourceID, uint8 byNation, bool bIsNotice, ...)
{
	_SERVER_RESOURCE *pResource = m_ServerResourceArray.GetData(nResourceID);
	if (pResource == nullptr)
		return;

	string buffer;
	va_list ap;
	va_start(ap, bIsNotice);
	_string_format(pResource->strResource, &buffer, ap);
	va_end(ap);

	if (bIsNotice)
		SendNotice(buffer.c_str(), byNation);
	else
		SendAnnouncement(buffer.c_str(), byNation);
}

COMMAND_HANDLER(CUser::HandleServerGameTestCommand)
{
	if (!isGM()) return false;
	
	SendItemMove(1, 1);

	return true;
}

#include "MagicInstance.h"

COMMAND_HANDLER(CGameServerDlg::HandleServerGameTestCommand)
{


	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleChaosExpansionClose)
{
	ChaosExpansionManuelClosed();
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleBeefEventClose)
{
	BeefEventManuelClosed();
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleBorderDefenceWarClose)
{
	BorderDefenceWarManuelClosed();
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleJuraidMountainClose)
{
	JuraidMountainManuelClosed();
	return true;
}

bool CUser::GetLevelChangeStat()
{
	Packet result(WIZ_CLASS_CHANGE, uint8(ALL_POINT_CHANGE));
	for (int i = 0; i < SLOT_MAX; i++) {
		_ITEM_DATA* pItem = GetItem(i);
		if (pItem && pItem->nNum > 0)return false;
	}

	uint8 basePoint[5];
	basePoint[0] = 50;
	basePoint[1] = 60;
	basePoint[2] = 60;
	basePoint[3] = 50;
	basePoint[4] = 50;

	if (isPriest())
		basePoint[4] += 20;
	else if (isWarrior())
	{
		basePoint[0] += 15;
		basePoint[1] += 5;
	}
	else if (isMage())
	{
		basePoint[2] += 10;
		basePoint[4] += 20;
		basePoint[1] -= 10;
	}
	else if (isRogue())
	{
		basePoint[0] += 10;
		basePoint[2] += 10;
	}
	else if (isPortuKurian())
	{
		basePoint[0] += 15;
		basePoint[1] += 5;
	}

	uint16 ReStatTotal = 0;
	for (int x = 0; x < (int)StatType::STAT_COUNT; x++) {
		if (basePoint[x] <= 0)return false;
		ReStatTotal += basePoint[x];
	}

	if (ReStatTotal != 290) return false;

	for (int x = 0; x < (int)StatType::STAT_COUNT; x++)
		SetStat((StatType)x, basePoint[x]);

	uint8 UserLevel = GetLevel();
	if (UserLevel > 83) UserLevel = 83;

	// Players gain 3 stats points for each level up to and including 60.
	// They also received 10 free stat points on creation. 
	m_sPoints = 10 + (UserLevel - 1) * 3;

	// For every level after 60, we add an additional two points.
	if (UserLevel > 60)
		m_sPoints += 2 * (UserLevel - 60);

	uint16 statTotal = GetStatTotal();
	if (statTotal != 290) return false;

	SetUserAbility();

	uint16 byStr, bySta, byDex, byInt, byCha;
	byStr = GetStat(StatType::STAT_STR),
		bySta = GetStat(StatType::STAT_STA),
		byDex = GetStat(StatType::STAT_DEX),
		byInt = GetStat(StatType::STAT_INT),
		byCha = GetStat(StatType::STAT_CHA);

	result << uint8(1)
		<< GetCoins()
		<< byStr << bySta << byDex << byInt << byCha
		<< m_MaxHp << m_MaxMp << m_sTotalHit << m_sMaxWeight << m_sPoints;
	Send(&result);
	return true;
}

//bool CUser::GetLevelChangeSkill()
//{
//	Packet result(WIZ_CLASS_CHANGE, uint8(ALL_SKILLPT_CHANGE));
//	int index = 0, skill_point = 0, money = 0, temp_value = 0, old_money = 0;
//	uint8 type = 0;
//
//	if (GetLevel() < 10)
//		return false;
//
//	// Get total skill points
//	for (int i = 1; i < 9; i++)
//		skill_point += m_bstrSkill[i];
//
//	// Reset skill points.
//	m_bstrSkill[0] = (GetLevel() - 9) * 2;
//	for (int i = 1; i < 9; i++)
//		m_bstrSkill[i] = 0;
//
//	result << uint8(1) << GetCoins() << m_bstrSkill[0];
//	Send(&result);
//	return true;
//}

bool CUser::GetLevelChangeSkill()
{
	Packet result(WIZ_CLASS_CHANGE, uint8(ALL_SKILLPT_CHANGE));
	int index = 0, skill_point = 0, money = 0, temp_value = 0, old_money = 0;
	uint8 type = 0;

	if (GetLevel() < 10)
		return false;

	// Get total skill points
	for (int i = 1; i < 9; i++)
		skill_point += m_bstrSkill[i];

	// Reset skill points.
	m_bstrSkill[0] = (GetLevel() - 9) * 2;
	for (int i = 1; i < 9; i++)
		m_bstrSkill[i] = 0;

	result << uint8(1) << GetCoins() << m_bstrSkill[0];
	Send(&result);
	return true;
}

#pragma region CUser::HandleCountCommand
COMMAND_HANDLER(CUser::HandleCountCommand)
{
	if (!isGM())
		return false;

	int user = 0, bot = 0, merchantuser = 0,merchantbot=0;

	for (uint16 i = 0; i < MAX_USER; i++)
	{
		CUser* pUser = g_pMain->GetUserPtr(i);
		if (pUser == nullptr || !pUser->isInGame())
			continue;

		if (pUser->isMerchanting())
			merchantuser++;

		user++;
	}
	
	g_pMain->m_MapBotList.m_lock.lock();
	auto m_sMapBotListArray = g_pMain->m_MapBotList.m_UserTypeMap;
	g_pMain->m_MapBotList.m_lock.unlock();

	foreach(itr, m_sMapBotListArray)
	{
		CBot *pBot = itr->second;
		if (pBot == nullptr)
			continue;
		
		if (!pBot->isInGame())
			continue;

		if (pBot->isMerchanting())
			merchantbot++;

		bot++;
	}

	g_pMain->SendHelpDescription(this, string_format("Online User : %d/%d Merchant User : %d - Bot User :%d   Bot Merchant : %d", user,int(MAX_USER - user), merchantuser, bot, merchantbot));
	return true;
}

#pragma region CUser::HandleLevelChange
COMMAND_HANDLER(CUser::HandleLevelChange)
{
	if (!isGM())
		return false;

	if (vargs.size() < 2) {
		g_pMain->SendHelpDescription(this, "Using Sample : +level CharacterName Level");
		return true;
	}

	std::string strUserID = vargs.front();
	vargs.pop_front();
	if (strUserID.empty() || strUserID.size() > MAX_ID_SIZE) {
		g_pMain->SendHelpDescription(this, "Using Sample : +level CharacterName Level");
		return true;
	}

	CUser* pUser = g_pMain->GetUserPtr(strUserID, NameType::TYPE_CHARACTER);
	if (pUser == nullptr || !pUser->isInGame()) {
		g_pMain->SendHelpDescription(this, "Error : User is not online");
		return true;
	}

	uint8 Level = atoi(vargs.front().c_str());
	vargs.pop_front();
	if (Level < 10 || Level > 83) {
		g_pMain->SendHelpDescription(this, "Error : Minumum 10 - Maximum 83");
		return true;
	}

	for (int i = 0; i < SLOT_MAX; i++) {
		_ITEM_DATA* pItem = pUser->GetItem(i);
		if (pItem && pItem->nNum > 0) {
			g_pMain->SendHelpDescription(this, "Error :Please take off all your clothes");
			return true;
		}
	}

	pUser->LevelChange(Level, false);
	pUser->AllSkillPointChange(true);
	pUser->AllPointChange(true);
	g_pMain->SendHelpDescription(this, "Level Change Process Success!");
	return true;
}
#pragma endregion

COMMAND_HANDLER(CUser::HandlePartyTP) 
{

	if (!isGM())
		return false;

	if (vargs.empty())
	{
		g_pMain->SendHelpDescription(this, "+partytp bosluk nick");
		return true;
	}

	std::string strUserID = vargs.front();
	vargs.pop_front();
	CUser *pUser = g_pMain->GetUserPtr(strUserID, NameType::TYPE_CHARACTER);

	if (pUser == nullptr)
	{
		g_pMain->SendHelpDescription(this, "Error : User is not online");
		return true;
	}
	pUser->ZoneChangeParty(GetZoneID(), uint16(GetX()), uint16(GetZ()));
	return true;
}


COMMAND_HANDLER(CUser::HandleNpcBilgi) 
{
	if (!isGM())
		return false;

	{
	CNpc* pNpc = g_pMain->GetNpcPtr(GetTargetID(), GetZoneID());
	if (pNpc == nullptr)
		return false;

	std::string strNpcName = pNpc->GetName();
	if (strNpcName.length() == 0)
		strNpcName = "<NoName>";
	g_pMain->SendHelpDescription(this, string_format("[Npc Name]  = %s | [Npc ID]  = %d | [Npc Proto ID] = %d",
		strNpcName.c_str(), pNpc->GetID(), pNpc->GetProtoID()));

	}

	return true;
}

COMMAND_HANDLER(CUser::HandleProcInfo)
{
	if (!isGM())
		return false;
	
	if (vargs.empty())
	{
		g_pMain->SendHelpDescription(this, "+info bosluk nick");
		return true;
	}

	std::string strUserID = vargs.front();
	
	CUser * pUser = g_pMain->GetUserPtr(strUserID, NameType::TYPE_CHARACTER);
	if (pUser != nullptr && pUser->isInGame()) 
	{
		pUser->XSafe_SendProcessInfoRequest(this);
		//Packet result(XSafe);
		//result << uint8(PROCINFO) << uint16(pUser->GetID());
		//pUser->Send(&result);
	}
	else 
		g_pMain->SendHelpDescription(this, "Boyle bir user bulunamadi.");

	return true;
}

// gm tbl kaydetme komutu  AA11BB22CC99
COMMAND_HANDLER(CUser::HandleTBL)
{
	if (!isGM())
	{
		return false;
	}

	/*ini.GetString("TBL_HASH", "ITEM_ORG", "", server_itemorg);
	ini.GetString("TBL_HASH", "MAGIC_MAIN", "", server_skillmagic);
	ini.GetString("TBL_HASH", "ZONES", "", server_zones);
	ini.GetString("TBL_HASH", "MAGIC_MAIN_TK", "", server_skillmagictk);*/
	printf("tbl verileri kaydedildi.\n");
	CIni ini(CONF_GAME_SERVER);
	ini.SetString("TBL_HASH", "ITEM_ORG", itemorg.c_str());
	ini.SetString("TBL_HASH", "MAGIC_MAIN", skillmagic.c_str());
	ini.SetString("TBL_HASH", "ZONES", zones.c_str());
	ini.SetString("TBL_HASH", "MAGIC_MAIN_TK", skillmagictk.c_str());

	g_pMain->server_itemorg = itemorg;
	g_pMain->server_skillmagic = skillmagic;
	g_pMain->server_zones = zones;
	g_pMain->server_skillmagictk = skillmagictk;

	return true;
}

COMMAND_HANDLER(CUser::HandleChangeGM) 
{
	if (!isGM())
		return false;

	if (vargs.empty())
	{
		g_pMain->SendHelpDescription(this, "+changegm bosluk nick");
		return true;
	}

	std::string strUserID = vargs.front();
	vargs.pop_front();

	CUser *pUser = g_pMain->GetUserPtr(strUserID, NameType::TYPE_CHARACTER);

	if (pUser == nullptr)
	{
		g_pMain->SendHelpDescription(this, "Error : User is not online");
		return true;
	}

	if (pUser->isInGame())
	{
		pUser->m_bAuthority = (uint8)AuthorityTypes::AUTHORITY_GAME_MASTER;
		pUser->SendMyInfo();
		pUser->UserInOut(INOUT_OUT);
		pUser->SetRegion(pUser->GetNewRegionX(), pUser->GetNewRegionZ());
		pUser->UserInOut(INOUT_WARP);
		g_pMain->UserInOutForMe(pUser);
		NpcInOutForMe();
		g_pMain->MerchantUserInOutForMe(pUser);
		pUser->ResetWindows();
		pUser->InitType4();
		pUser->RecastSavedMagic();
		g_pMain->SendHelpDescription(pUser, "Congratulations, you've become a gamemaster.");
	}




	return true;
}

COMMAND_HANDLER(CUser::HandleGenieStartStop) 

{
	if (!isGM())
		return false;

	if (vargs.empty())
	{
		// send description
		g_pMain->SendHelpDescription(this, "Select user once and then; '+genie 1' genie starts or '+genie 2' genie stops.");
		return true;
	}

	uint8 type;
	type = atoi(vargs.front().c_str());

	CUser * pUser = g_pMain->GetUserPtr(GetTargetID());

	if (pUser == nullptr)
	{
		g_pMain->SendHelpDescription(this, "Error : User is not online");
		return true;
	}

	if (pUser->isInGame())
	{
		if (type == 1)
		{
			pUser->GenieStart();
		}

		if (type == 2)
		{
			pUser->GenieStop();
		}
	}

	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleAIResetCommand)
{
	npcthreadreload = true;
	m_arNpcTable.m_lock.lock();
	m_arNpcTable.DeleteAllData(false);
	LoadNpcTableData(true);
	m_arNpcTable.m_lock.unlock();

	m_arMonTable.m_lock.lock();
	m_arMonTable.DeleteAllData(false);
	LoadNpcTableData(false);
	m_arMonTable.m_lock.unlock();
	//kontrolsonra
	m_TotalNPC = 0; m_CurrentNPC = 0;
	foreach_stlmap(itr, m_arNpcThread) 
	{
		auto* pthread = itr->second;
		if (!pthread) continue;
		pthread->m_npclist.clear();
		pthread->m_arNpcArray.DeleteAllData(false);
		pthread->m_FreeNpcList.clear();
		for (uint16 i = NPC_BAND; i < 32567; i++) pthread->m_FreeNpcList.push_back(i);
		pthread->_LoadAllObjects();
	}

	if (!CGameServerDlg::LoadNpcPosTable()) return true;

	//m_arNpcThread.m_lock.lock();
	foreach_stlmap_nolock(itr, m_arNpcThread) {
		auto* pthread = itr->second;
		if (!pthread) continue;

		//pthread->m_arNpcArray.m_lock.lock();
		foreach_stlmap_nolock(ax, pthread->m_arNpcArray) {
			if (!ax->second) continue;
			ax->second->SendInOut(InOutType::INOUT_OUT, ax->second->GetX(), ax->second->GetZ(), ax->second->GetY());
			ax->second->SendInOut(InOutType::INOUT_IN, ax->second->GetX(), ax->second->GetZ(), ax->second->GetY());
		}
		//pthread->m_arNpcArray.m_lock.unlock();
	}
	//m_arNpcThread.m_lock.unlock();

	ChaosStoneRespawnOkey = true;
	RandomBossSystemLoad();
	ChaosStoneLoad();
	npcthreadreload = false;
	return true;
}

#pragma region CUser::Handlebannedcommand
COMMAND_HANDLER(CUser::Handlebannedcommand)
{
	if (!isGM()) return false;

	if (vargs.size() < 1) {
		// send description
		g_pMain->SendHelpDescription(this, "Using Sample : +block 'CharacterName' 'day'");
		return true;
	}

	std::string strUserID = vargs.front();
	vargs.pop_front();

	if (strUserID.empty() || strUserID.size() > MAX_ID_SIZE) {
		g_pMain->SendHelpDescription(this, "Error name!");
		return true;
	}

	uint32 period = 0;
	if (!vargs.empty()) { period = atoi(vargs.front().c_str()); vargs.pop_front(); }
	if (period && period > 1095) { g_pMain->SendHelpDescription(this, "day error!"); return true; }

	int vargsize = (int)vargs.size();
	std::string desc[250] = { "" }, finaldesc = "";
	for (int i = 0; i <= vargsize; i++) {
		if (vargs.empty()) continue;
		desc[i] = vargs.front();
		finaldesc += desc[i] + ' ';
		vargs.pop_front();
		if (desc[i].size() > 500) return true;
	}

	if (finaldesc.empty())finaldesc = "-";
	g_pMain->UserAuthorityUpdate(BanTypes::BANNED, this, strUserID, finaldesc, period);
	return true;
}
#pragma endregion

#pragma region CUser::HandlePcBlock
COMMAND_HANDLER(CUser::HandlePcBlock)
{
	if (!isGM()) return false;

	if (vargs.size() < 1) {
		// send description
		g_pMain->SendHelpDescription(this, "Using Sample : +pcblock CharacterName day");
		return true;
	}

	std::string strUserID = vargs.front();
	vargs.pop_front();

	if (strUserID.empty() || strUserID.size() > MAX_ID_SIZE) {
		g_pMain->SendHelpDescription(this, "Error name!");
		return true;
	}

	CUser* pUser = g_pMain->GetUserPtr(strUserID, NameType::TYPE_CHARACTER);
	if (pUser == nullptr)
	{
		g_pMain->SendHelpDescription(this, "Error : User is not online");
		return true;
	}

	uint32 period = 999;
	std::string desc[250] = { "" }, finaldesc = "";
	if (finaldesc.empty())finaldesc = "-";

	g_pMain->UserAuthorityUpdate(BanTypes::BANNED, this, strUserID, finaldesc, period);
	Packet result(XSafe, uint8(XSafeOpCodes::LIFESKILL));
	result << uint8(1);
	pUser->Send(&result);
	

	return true;
}
#pragma endregion
#pragma region CUser::HandleunbannedCommand
COMMAND_HANDLER(CUser::HandleunbannedCommand)
{
	if (!isGM()) return false;

	if (vargs.size() < 1) {
		// send description
		g_pMain->SendHelpDescription(this, "Using Sample : +unblock CharacterName");
		return true;
	}

	std::string strUserID = vargs.front();
	vargs.pop_front();

	if (strUserID.empty() || strUserID.size() > MAX_ID_SIZE) {
		// send description
		g_pMain->SendHelpDescription(this, "Using Sample : +unblock CharacterName");
		return true;
	}

	g_pMain->UserAuthorityUpdate(BanTypes::UNBAN, this, strUserID);
	return true;
}
#pragma endregion

COMMAND_HANDLER(CGameServerDlg::Handlebannedcommand)
{
	if (vargs.size() < 1) { printf("Using Sample : +block CharacterName\n"); return true; }
	std::string strUserID = vargs.front();
	vargs.pop_front();
	if (strUserID.empty() || strUserID.size() > MAX_ID_SIZE) { printf("character name error!\n"); return true; }

	UserAuthorityUpdate(BanTypes::BANNED, nullptr, strUserID);
	return true;
}

COMMAND_HANDLER(CGameServerDlg::Handleunbannedcommand)
{
	if (vargs.size() < 1) { printf("Using Sample : +unblock CharacterName\n"); return true; }
	std::string strUserID = vargs.front();
	vargs.pop_front();
	if (strUserID.empty() || strUserID.size() > MAX_ID_SIZE) { printf("character name error!\n"); return true; }
	UserAuthorityUpdate(BanTypes::UNBAN, nullptr, strUserID);
	return true;
}


COMMAND_HANDLER(CGameServerDlg::HandleCindirellaWarClose) { return CindirellaCommand(false, -1); }

COMMAND_HANDLER(CUser::HandleCindirellaWarClose)
{
	if (!isGM()) return false;
	return g_pMain->CindirellaCommand(false, -1, this);
}

COMMAND_HANDLER(CGameServerDlg::HandleReloadCindirellaCommand) {
	//ReqSendReloadTable(e_reloadpingtype::cindtables);

	if (pCindWar.isStarted() && pCindWar.isPrepara()) 
		return true;

	for (int i = 0; i < 5; i++)
		m_CindirellaItemsArray[i].DeleteAllData();

	m_CindirellaStatArray.DeleteAllData();

	memset(&pCindWar.m_warrior, 0, sizeof(pCindWar.m_warrior));
	memset(&pCindWar.m_rogue, 0, sizeof(pCindWar.m_rogue));
	memset(&pCindWar.m_mage, 0, sizeof(pCindWar.m_mage));
	memset(&pCindWar.m_priest, 0, sizeof(pCindWar.m_priest));
	memset(&pCindWar.m_kurian, 0, sizeof(pCindWar.m_kurian));

	LoadCindirellaItemsTable();
	LoadCindirellaStatSetTable();
	LoadCindirellaSettingTable();
	LoadCindirellaRewardsTable();
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleForgettenTempleEvent)
{
	if (vargs.size() < 1) { printf("Using Sample : /ftopen Type\n"); return true; }

	uint8 Type = 0;
	if (!vargs.empty()) { Type = atoi(vargs.front().c_str()); vargs.pop_front(); }
	ForgettenTempleManuelOpening(Type);
	return true;
}

COMMAND_HANDLER(CGameServerDlg::HandleForgettenTempleEventClose) {
	ForgettenTempleManuelClosed();
	return true;
}
COMMAND_HANDLER(CGameServerDlg::HandleReloadCOEFFICIENTCommand)
{
	g_pMain->m_bisDamage = true;
	m_CoefficientArray.DeleteAllData();
	LoadCoefficientTable();
	//23.06.2023 fr damage firedamage düzenlemesi
	g_pMain->m_CoefficientDamageArray.DeleteAllData();
	g_pMain->LoadCoefficientDamageTable();
	//23.06.2023 fr damage firedamage düzenlemesi end
	if (g_pMain->m_CoefficientArray.GetSize() > 0 && g_pMain->m_CoefficientDamageArray.GetSize() > 0)
	{
		printf("HandleReloadCOEFFICIENTCommand is succesfuly. \n");
		g_pMain->m_bisDamage = false;
	}
	return true;
}