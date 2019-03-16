/* ======== Basic Admin tool ========
* Copyright (C) 2004-2005 Erling K. Sæterdal
* No warranties of any kind
*
* License: zlib/libpng
*
* Author(s): Erling K. Sæterdal ( EKS )
* Credits:
*	Menu code based on code from CSDM ( http://www.tcwonline.org/~dvander/cssdm ) Created by BAILOPAN
*	Adminloading function from Xaphan ( http://www.sourcemod.net/forums/viewtopic.php?p=25807 ) Created by devicenull
* Helping on misc errors/functions: BAILOPAN,sslice,devicenull,PMOnoTo,cybermind ( most who idle in #sourcemod on GameSurge realy )
* ============================ */

#ifndef _INCLUDE_CONST_H
#define _INCLUDE_CONST_H

#include "edict.h"

#define CSB_VERSION	"1.0.5"
#define CSB_DEBUG 0

#define RECONNECT_REMEMEBERCOUNT 5
#define TEAM_NOT_CONNECTED -1

#define MAXPLAYERS ABSOLUTE_PLAYER_LIMIT +1

// ---------- GameFrame()
#define TASK_CHECKTIME 1.0			// This is how often the plugin checks if any tasks should be executed

#define MENUSYSTEM_NOTWORKING -1

enum MenuPages
{
	MOD_NONE=0,
	MOD_CSTRIKE,
	MOD_DOD,
	MOD_HL2MP,
	MOD_HL2CTF,
};
typedef struct 
{
	int MenuMsg;		// The id of the MenuMsg
	int HintText;		// HintText
	int TextMsg;		// TextMsg
	int HudMsg;			// HudMsg
	int SayText;		// SayMsg
	bool MenuSupport;
}ModSettingsStruct;
typedef struct 
{
	char Steamid[MAX_NETWORKID_LENGTH+1];
	int Userid;
	bool IsAlive;	// If the user is alive
	bool IsBot;		// is false if a player is a hltv or a bot
	edict_t *PlayerEdict;
}ConstPlayerInfo;
#endif //_INCLUDE_CONST_H