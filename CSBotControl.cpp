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
* ============================
*/
#include "convar.h"
#include "oslink.h"
#include "BATInterface.h"
#include "CSBotControl.h"
#include "Menu.h"
#include "meta_hooks.h"
#include "cvars.h"
#include "const.h"
#include "BATMenu.h"
#include <time.h>

#include "recipientfilters.h"
#include <bitbuf.h>

//This has all of the necessary hook declarations.  Read it!
#include "meta_hooks.h"

int g_IndexOfLastUserCmd;
int g_MaxClients;
int g_ModIndex;

int g_AlivePlayers;
int g_AliveBots;
int g_BotCount;
int g_PlayerCount;

int g_ForgiveTKMenu;

float g_FlTime;
bool g_IsConnected[MAXPLAYERS+1];
bool g_HasMenuOpen[MAXPLAYERS+1];
bool g_FirstLoad;
bool g_FoundInterface=false;

ConVar *g_pJoinAfterPlayer;
ConVar *g_pDifficulty;
ConVar *g_pFreeze;
ConVar *g_pChatter;
ConVar *g_pJoinTeam;
ConVar *g_pQuota;
ConVar *g_pCheats;

CSBotControl g_CSBCore;
ConstPlayerInfo UserInfo[MAXPLAYERS+1];
ModSettingsStruct g_ModSettings;
AdminInterface *m_AdminManager;
BATMenuMngr g_MenuMngr;

PLUGIN_EXPOSE(CSBotControl, g_CSBCore);

void CSBotControl::ServerActivate(edict_t *pEdictList, int edictCount, int clientMax)
{
	LoadPluginSettings(clientMax);
	CheckBotCount();
	RETURN_META(MRES_IGNORED);
}
void CSBotControl::ClientDisconnect(edict_t *pEntity)
{
	int id = m_Engine->IndexOfEdict(pEntity);
	if(g_IsConnected[id] == true)
	{
		if(strcmp(UserInfo[id].Steamid,"BOT") == 0 || strcmp(UserInfo[id].Steamid,"HLTV") == 0 ) // || strcmp(UserInfo[id].steamid,"STEAM_ID_LAN") == 0
			g_BotCount--;
		else 
			g_PlayerCount--;
	}
	g_IsConnected[id] = false;

	CheckBotCount();
	RETURN_META(MRES_IGNORED);
}
void CSBotControl::ClientPutInServer(edict_t *pEntity, char const *playername)
{
	int id = m_Engine->IndexOfEdict(pEntity);
	g_IsConnected[id] = true;
	UserInfo[id].PlayerEdict = pEntity;
	UserInfo[id].Userid = m_Engine->GetPlayerUserId(pEntity);

	const char * SteamID = m_Engine->GetPlayerNetworkIDString(pEntity);
	if(!SteamID || strlen(SteamID) == 0)
	{
		LogPrint("CSBotControl: ERROR Failed to find a valid steamid for player %d - %s",id,playername);		
		return;
	}
	sprintf(UserInfo[id].Steamid,"%s",SteamID);

	if(strcmp(UserInfo[id].Steamid,"BOT") == 0 || strcmp(UserInfo[id].Steamid,"HLTV") == 0 ) // || strcmp(UserInfo[id].steamid,"STEAM_ID_LAN") == 0
	{
		UserInfo[id].IsBot = true;
		g_BotCount++;
	}
	else 
	{
		UserInfo[id].IsBot = false;
		g_PlayerCount++;
	}
	CheckBotCount();

	if(g_AlivePlayers == 0 && g_CSBVars.AutoKill() && g_PlayerCount > 0)
	{
		g_CSBCore.ServerCommand("bot_kill");
		g_CSBCore.MessagePlayer(0,"[CSBotControl] All human players are dead, automatically killing bots");
	}
	RETURN_META(MRES_IGNORED);
}
void CSBotControl::CheckBotCount()
{
	int MinPlayers = g_CSBVars.GetAutoAddCount();
	if(MinPlayers < 0)
		return;
	
	if(g_PlayerCount >= MinPlayers && g_BotCount == 0) // We are above the minimum required players, and no bots on the server. So no action is required
		return;
	
	int NewBotCount = MinPlayers - g_PlayerCount;
	if(NewBotCount < 0) // A error happened realy. we dont set negative values
		return;
	g_pQuota->SetValue(NewBotCount);
}
void CSBotControl::SetCommandClient(int index)
{
	//META_LOG(g_PLAPI, "SetCommandClient() called: index=%d", index);
	g_IndexOfLastUserCmd = index +1;
	RETURN_META(MRES_IGNORED);
}
void CSBotControl::GameFrame(bool simulating) // We dont hook GameFrame, we leave it in here incase we ever need some timer system 
{
	RETURN_META(MRES_IGNORED);
}
void CSBotControl::ClientCommand(edict_t *pEntity, CCommand const &ccmd)
{
	if(! (0 < ccmd.ArgC())) RETURN_META(MRES_IGNORED);

	int id = m_Engine->IndexOfEdict(pEntity);
	const char *command = ccmd.Arg(0);

	if (strcmp(command,"admin_botmenu") == 0 && g_FoundInterface && m_AdminManager->HasFlag(id,"botmenu"))
	{
		g_MenuMngr.ShowMenu(id,g_ForgiveTKMenu);
		RETURN_META(MRES_SUPERCEDE);
	}

	if(g_HasMenuOpen[id] == false)
		RETURN_META(MRES_IGNORED);

	if (strcmp(command,"menuselect") == 0)
	{
		if(! (1 < ccmd.ArgC())) RETURN_META(MRES_IGNORED);
		char const *arg1 = ccmd.Arg(0);
		//catch menu commands
		if (arg1)
		{
			int arg = atoi(arg1);
			if(arg < 1 || arg > 10) // Make sure makes no invalid selection.
				return;

			g_MenuMngr.MenuChoice(id, arg);
			RETURN_META(MRES_SUPERCEDE);
		}
	}
	RETURN_META(MRES_IGNORED);
}
void CSBotControl::FireGameEvent(IGameEvent *event)
{
	if (!event || !event->GetName())
		return;

	const char *EventName = event->GetName();

	if(strcmp(EventName,"player_death") == 0)
	{
		int victim = g_CSBCore.FindPlayer(event->GetInt("userid"));
		//int attacker = g_CSBCore.FindPlayer(event->GetInt("attacker"));
		if(UserInfo[victim].IsBot)
			g_AliveBots--;
		else
		{
			g_AlivePlayers--;
			if(g_AlivePlayers == 0 && g_CSBVars.AutoKill() && g_PlayerCount > 0)
			{
				g_CSBCore.ServerCommand("bot_kill");
				g_CSBCore.MessagePlayer(0,"[CSBotControl] All human players are dead, automatically killing bots");
			}
		}
	}
	else if(strcmp(EventName,"player_spawn") == 0)
	{
		int victim = g_CSBCore.FindPlayer(event->GetInt("userid"));
#if CSB_DEBUG == 1
		g_CSBCore.ServerCommand("echo %s spawned",g_CSBCore.GetPlayerName(victim));
#endif

		if(UserInfo[victim].IsBot)
			g_AliveBots++;
		else
			g_AlivePlayers++;
	}
	else if(strcmp(EventName,"round_end") == 0)
	{
#if CSB_DEBUG == 1
		g_CSBCore.ServerCommand("echo Round is restarted");
#endif
		g_AliveBots=0;
		g_AlivePlayers=0;
	}
	return;
}
void Say_handler()
{
	/*
	bool isTeamSay = false;
	const char *firstword = g_CSBCore.GetEngine()->Cmd_Argv(1);

	int id = g_IndexOfLastUserCmd;
	if(stricmp(firstword,"ff") == 0 || stricmp(firstword,"/ff") == 0)
	{

	}
	*/
}
void CSBotControl::LoadPluginSettings(int clientMax)
{	
	if(g_FirstLoad == false)
	{
		SetupModSpesficInfo();
		m_Sound->PrecacheSound(SOUND_CHOICEMADE,true);
		m_Sound->PrecacheSound(SOUND_INVALID,true);


		g_FirstLoad = true;
		g_MaxClients = clientMax;

		CSBotControlMenu *CSBCMenu = new CSBotControlMenu;
		g_ForgiveTKMenu = g_MenuMngr.RegisterMenu(CSBCMenu);

		m_GameEventManager->AddListener(this,"player_death",true);
		m_GameEventManager->AddListener(this,"player_spawn",true);
		m_GameEventManager->AddListener(this,"round_end",true);
		
		bool CvarHooks = HookCvars();
		if(!CvarHooks || g_ModIndex != MOD_CSTRIKE)
		{
			ServerCommand("echo [CSBotControl] Something failed when loading this plugin, if you loaded the plugin on CSS then check for a plugin update");
			LogPrint("[CSBotControl] Something failed when loading this plugin, if you loaded the plugin on CSS then check for a plugin update");
			ServerCommand("meta unload %i",g_PLID);
			return;
			//int i = g_PLID;
		}
	}
	for(int i=1;i<=g_MaxClients;i++)
	{
		g_IsConnected[i] = false;
		g_HasMenuOpen[i] = false;
	}
	
	LogPrint("[CSBotControl] Game event hooking active");
}
bool CSBotControl::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_ANY(GetServerFactory, m_ServerDll, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_ANY(GetServerFactory, m_ServerClients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetServerFactory, m_InfoMngr, IPlayerInfoManager, INTERFACEVERSION_PLAYERINFOMANAGER);
	GET_V_IFACE_CURRENT(GetServerFactory, m_BotMngr, IBotManager, INTERFACEVERSION_PLAYERBOTMANAGER);

	GET_V_IFACE_CURRENT(GetEngineFactory, m_Engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, m_GameEventManager, IGameEventManager2, INTERFACEVERSION_GAMEEVENTSMANAGER2);
	GET_V_IFACE_CURRENT(GetEngineFactory, m_ICvar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, m_Helpers, IServerPluginHelpers, INTERFACEVERSION_ISERVERPLUGINHELPERS);
	GET_V_IFACE_CURRENT(GetEngineFactory, m_Sound, IEngineSound, IENGINESOUND_SERVER_INTERFACE_VERSION);	
	
	//EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CPluginBotManager, IBotManager, , s_BotManager);
	
	//Init our cvars/concmds
#warning("fix it")
	// ConCommandBaseMgr::OneTimeInit(&g_CSBVars);

	//We're hooking the following things as POST, in order to seem like Server Plugins.
	//However, I don't actually know if Valve has done server plugins as POST or not.
	//Change the last parameter to 'false' in order to change this to PRE.
	//SH_ADD_HOOK_MEMFUNC means "SourceHook, Add Hook, Member Function".

	//Hook LevelInit to our function
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, ServerActivate, m_ServerDll, &g_CSBCore, &CSBotControl::ServerActivate, true);		//Hook GameFrame to our function
	//SH_ADD_HOOK_MEMFUNC(IServerGameDLL, GameFrame, m_ServerDll, &g_ForgiveTKCore, &CSBotControl::GameFrame, true);				//Hook LevelShutdown to our function -- this makes more sense as pre I guess
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, m_ServerClients, &g_CSBCore, &CSBotControl::ClientDisconnect, true);		//Hook ClientPutInServer to our function
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, m_ServerClients, &g_CSBCore, &CSBotControl::ClientPutInServer, true);	//Hook SetCommandClient to our function
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, SetCommandClient, m_ServerClients, &g_CSBCore, &CSBotControl::SetCommandClient, true);		//Hook ClientSettingsChanged to our function
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientCommand, m_ServerClients, &g_CSBCore, &CSBotControl::ClientCommand, false);	//This hook is a static hook, no member function


	//The following functions are pre handled, because that's how they are in IServerPluginCallbacks

		
	m_Engine_CC = SH_GET_CALLCLASS(m_Engine); // invoking their hooks (when needed).
	
	SH_CALL(m_Engine_CC, &IVEngineServer::LogPrint)("All hooks started!\n");
	if(late)
	{
		LoadPluginSettings(ismm->GetCGlobals()->maxClients);
		LogPrint("[CSBotControl] Late load is not really supported, plugin will function properly after 1 mapchange");
	}
	return true;
}
bool CSBotControl::Unload(char *error, size_t maxlen)
{
	
	//IT IS CRUCIAL THAT YOU REMOVE CVARS.
	//As of Metamod:Source 1.00-RC2, it will automatically remove them for you.
	//But this is only if you've registered them correctly!

	//Make sure we remove any hooks we did... this may not be necessary since
	//SourceHook is capable of unloading plugins' hooks itself, but just to be safe.
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, ServerActivate, m_ServerDll, &g_CSBCore, &CSBotControl::ServerActivate, true);
	//SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, GameFrame, m_ServerDll, &g_ForgiveTKCore, &CSBotControl::GameFrame, true);
	//SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientActive, m_ServerClients, &g_ForgiveTKCore, &CSBotControl::ClientActive, true);
	
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, m_ServerClients, &g_CSBCore, &CSBotControl::ClientDisconnect, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, m_ServerClients, &g_CSBCore, &CSBotControl::ClientPutInServer, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, SetCommandClient, m_ServerClients, &g_CSBCore, &CSBotControl::SetCommandClient, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientCommand, m_ServerClients, &g_CSBCore, &CSBotControl::ClientCommand, false);

	

	//this, sourcehook does not keep track of.  we must do this.
	SH_RELEASE_CALLCLASS(m_Engine_CC);

	m_GameEventManager->RemoveListener(this);
	if(m_AdminManager) // We remove the admin interface hook
	{
		m_AdminManager->RemoveListner(this);
		m_AdminManager = NULL;
	}


	return true;
}
void CSBotControl::AllPluginsLoaded()
{
	//we don't really need this for anything other than interplugin communication
	//and that's not used in this plugin.
	//If we really wanted, we could override the factories so other plugins can request
	// interfaces we make.  In this callback, the plugin could be assured that either
	// the interfaces it requires were either loaded in another plugin or not.
	PluginId id2; 
	//AdminInterfaceListner *AdminInterfacePointer;

	if(m_AdminManager) // We dont need to find the AdminInterface again, we allready found it once.
		return;
	void *ptr = g_SMAPI->MetaFactory("AdminInterface", NULL, &id2); 
	
	if (!ptr) 
	{
		LogPrint("[CSBotControl] Did not find AdminInterface, plugin will not check admin rights"); 
	} else {
		m_AdminManager = (AdminInterface *)ptr;
		m_AdminManager->AddEventListner(this);
		g_FoundInterface = true;
		int InterfaceVersion = m_AdminManager->GetInterfaceVersion();
		if(!RegisterNewAcccessFlag())
			LogPrint("[CSBotControl] There was a error trying to register the custom access flag"); 

		if(InterfaceVersion == ADMININTERFACE_VERSION)
			LogPrint("[CSBotControl] Found AdminInterface %s (Interface version: %d)",m_AdminManager->GetModName(),InterfaceVersion); 
		else
			LogPrint("[CSBotControl] Found AdminInterface, but interface was NOT the expected version: Was %d Expected %d, this can create problems. Please update the plugin with the lowest interface version",m_AdminManager->GetModName(),InterfaceVersion,ADMININTERFACE_VERSION); 
	}
}
bool CSBotControl::RegisterNewAcccessFlag()
{
	if(m_AdminManager == NULL)
		return false;

	return m_AdminManager->RegisterFlag("Not used","botmenu","The right to access bot menus");	
}
void CSBotControl::OnAdminInterfaceUnload()
{
	g_FoundInterface = false;
	m_AdminManager = NULL;
	//g_CSBCore.ServerCommand("echo AdminInterface was unloaded");
}

void CSBotControl::Client_Authorized(int id)
{
#if CSB_DEBUG == 1
	ServerCommand("echo [ForgiveTK Debug]Client_Authorized: %d",id);
#endif
}

bool CSBotControl::HookCvars()
{
	ConCommandBase *pCmd = m_ICvar->GetCommands();
	while (pCmd)
	{
		if (!(pCmd->IsCommand())) 
		{
			if(stricmp(pCmd->GetName(),"bot_join_after_player") == 0)
				g_pJoinAfterPlayer = (ConVar *)pCmd;
			else if(stricmp(pCmd->GetName(),"bot_difficulty") == 0)
				g_pDifficulty = (ConVar *)pCmd;
			else if(stricmp(pCmd->GetName(),"bot_stop") == 0)
				g_pFreeze = (ConVar *)pCmd;
			else if(stricmp(pCmd->GetName(),"bot_chatter") == 0)
				g_pChatter = (ConVar *)pCmd;
			else if(stricmp(pCmd->GetName(),"bot_join_team") == 0)
				g_pJoinTeam = (ConVar *)pCmd;
			else if(stricmp(pCmd->GetName(),"bot_quota") == 0)
				g_pQuota = (ConVar *)pCmd;			
			else if(stricmp(pCmd->GetName(),"sv_cheats") == 0)
				g_pCheats = (ConVar *)pCmd;			
		}
		pCmd = const_cast<ConCommandBase *>(pCmd->GetNext());
	}
#warning("do something better here")
	// g_pFreeze->SetVarFlags((g_pFreeze->GetVarFlags() - FCVAR_CHEAT));
	if(!g_pJoinAfterPlayer || !g_pDifficulty || !g_pFreeze|| !g_pChatter )
		return false;
	return true;
}