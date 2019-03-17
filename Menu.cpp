/* ======== Basic Admin tool ========
* Copyright (C) 2004-2005 Erling K. Sæterdal
* No warranties of any kind
*
* License: zlib/libpng
*
* Author(s): Erling K. Sæterdal ( EKS )
* Credits:
*	Menu code based on code from CSDM ( http://www.tcwonline.org/~dvander/cssdm ) Created by BAILOPAN
*	Adminloading function from Xaphan ( http://www.sourcemod.net/forums/viewtopic.php?p=25807 ) Created by Devicenull
* Helping on misc errors/functions: BAILOPAN,sslice,devicenull,PMOnoTo,cybermind ( most who idle in #sourcemod on GameSurge realy )
* ============================ */

#include "CSBotControl.h"
#include "Menu.h"
#include "BATMenu.h"

#define GAME_DLL 1
#include "cbase.h"

extern int g_ForgiveTKMenu;
extern int g_MaxClients;
extern bool g_HasMenuOpen[MAXPLAYERS+1];
extern BATMenuMngr g_MenuMngr;
extern ModSettingsStruct g_ModSettings;

extern ConstPlayerInfo UserInfo[MAXPLAYERS+1];
extern bool g_IsConnected[MAXPLAYERS+1];

extern ConVar *g_pJoinAfterPlayer;
extern ConVar *g_pDifficulty;
extern ConVar *g_pFreeze;
extern ConVar *g_pChatter;
extern ConVar *g_pJoinTeam;
extern ConVar *g_pQuota;
extern ConVar *g_pCheats;

extern CSBVars g_CSBVars;
extern CSBotControl g_CSBCore;

bool CSBotControlMenu::Display(BATMenuBuilder *make, int playerIndex)
{
	make->SetKeys( (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<5) | (1<<6) | (1<<7)| (1<<8) | (1<<9));
	g_HasMenuOpen[playerIndex] = true;

	make->SetTitle("CS Bot Control");

	make->AddOption("BOT Difficulty: %s",GetDifficulty());
	make->AddOption("BOT Chatter: %s",g_pChatter->GetString());
	make->AddOption("BOT Freeze: %s",GetBoolString(g_pFreeze->GetBool()));
	make->AddOption("BOT Auto kill: %s",GetBoolString(g_CSBVars.AutoKill()));
	make->AddOption("BOT Join Team: %s",g_pJoinTeam->GetString());
	make->AddOption("Kill all bots");
	make->AddOption("Add BOT");
	make->AddOption("Remove BOT");
	make->AddOption("\n");
	make->AddOption("Exit");
	return true;
}
MenuSelectionType CSBotControlMenu::MenuChoice(player_t player, int option)
{
	int id = player.index;
	g_HasMenuOpen[id] = false;
	bool BadSelection= false;

	switch(option)
	{
	case 1:
		SetNextDifficulty(id);
		break;
	case 2:
		SetNextChatter(id);
		break;
	case 3:
	    SetNextFreeze(id);
		break;
	case 4:
	    SetNextAutokill(id);
		break;
	case 5:
		SetNextJoinTeam(id);
		break;
	case 6:
		g_CSBCore.ServerCommand("bot_kill");
		g_CSBCore.MessagePlayer(0,"[CSBotControl] %s slayed all bots",g_CSBCore.GetPlayerName(id));
		break;
	case 7:
		AddBot(id);
		break;
	case 8:
		RemoveBot(id);
		break;
	case 10:
		return MENUSELECTION_GOOD;

	default:
		BadSelection = true;
	    break;
	}

	g_MenuMngr.ShowMenu(id,g_ForgiveTKMenu);
	if(BadSelection)
		return MENUSELECTION_BAD;
	return MENUSELECTION_GOOD;
}

char const *CSBotControlMenu::GetDifficulty()
{
	int CvarValue = g_pDifficulty->GetInt();
	switch(CvarValue)
	{
	case 0:
		return "Easy";
	case 1:
		return "Normal";
	case 2:
		return "Hard";
	case 3:
		return "Expert";
	default:
		return NULL;
	}
}
void CSBotControlMenu::SetNextDifficulty(int id)
{
	int CvarValue = g_pDifficulty->GetInt() + 1;
	if(CvarValue > 3)
		CvarValue = 0;
	g_pDifficulty->SetValue(CvarValue);

	g_CSBCore.MessagePlayer(0,"[CSBotControl] %s changed the bot difficulty to: %s",g_CSBCore.GetPlayerName(id),GetDifficulty());
}
void CSBotControlMenu::SetNextFreeze(int id)
{
	if(g_pFreeze->GetInt() == 0)
	{
		g_pFreeze->SetValue(1);
		g_CSBCore.MessagePlayer(0,"[CSBotControl] %s has frozen the bots",g_CSBCore.GetPlayerName(id));
	}
	else
	{
		g_pFreeze->SetValue(0);
		g_CSBCore.MessagePlayer(0,"[CSBotControl] %s has unfrozen the bots",g_CSBCore.GetPlayerName(id));
	}
}
void CSBotControlMenu::SetNextAutokill(int id)
{
	if(g_CSBVars.AutoKill())
		g_CSBVars.AutoKill(false);
	else
		g_CSBVars.AutoKill(true);

	g_CSBCore.MessagePlayer(0,"[CSBotControl] %s changed Bots auto kill to: %s",g_CSBCore.GetPlayerName(id),GetBoolString(g_CSBVars.AutoKill()));
}
void CSBotControlMenu::SetNextChatter(int id)
{
	const char *CvarValue = g_pChatter->GetString();

	// 'off', 'radio', 'minimal', or 'normal'
	if(strcmp(CvarValue,"off") == 0)
		g_pChatter->SetValue("radio");
	else if(strcmp(CvarValue,"radio") == 0)
		g_pChatter->SetValue("minimal");
	else if(strcmp(CvarValue,"minimal") == 0)
		g_pChatter->SetValue("normal");
	else //if(strcmp(CvarValue,"normal") == 0)
		g_pChatter->SetValue("off");

	g_CSBCore.MessagePlayer(0,"[CSBotControl] %s changed the bots chatter setting to: %s",g_CSBCore.GetPlayerName(id),g_pChatter->GetString());
}
void CSBotControlMenu::SetNextJoinTeam(int id)
{
	const char *CvarValue = g_pJoinTeam->GetString();

	if(strcmp(CvarValue,"any") == 0)
		g_pJoinTeam->SetValue("T");
	else if(strcmp(CvarValue,"T") == 0)
		g_pJoinTeam->SetValue("CT");
	else if(strcmp(CvarValue,"CT") == 0)
		g_pJoinTeam->SetValue("any");

	g_CSBCore.MessagePlayer(0,"[CSBotControl] %s changed the settings to bots only will join: %s team",g_CSBCore.GetPlayerName(id),g_pJoinTeam->GetString());
}
char const * CSBotControlMenu::GetBoolString(bool MyBool)
{	
	if(!MyBool)
		return "Off";
	
	return "On";
}
void CSBotControlMenu::AddBot(int id)
{
	int AutoAddCount = g_CSBVars.GetAutoAddCount();
	int BotQuota = g_pQuota->GetInt()+1;
	if(AutoAddCount > 0)
		g_CSBVars.SetAutoAddCount(AutoAddCount+1);
	
	g_pQuota->SetValue(BotQuota);
	g_CSBCore.MessagePlayer(0,"[CSBotControl] %s changed to bot count to %d",g_CSBCore.GetPlayerName(id),BotQuota);
}
void CSBotControlMenu::RemoveBot(int id)
{
	int AutoAddCount = g_CSBVars.GetAutoAddCount();
	int BotQuota = g_pQuota->GetInt() - 1;
	if(BotQuota == -1)
		return;

	if(AutoAddCount > -1)
		g_CSBVars.SetAutoAddCount((AutoAddCount-1));
	
	g_CSBCore.MessagePlayer(0,"[CSBotControl] %s changed to bot count to %d",g_CSBCore.GetPlayerName(id),BotQuota);
	g_pQuota->SetValue(BotQuota);
}