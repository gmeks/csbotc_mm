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
#include "cvars.h"

CSBVars g_CSBVars;

ConVar g_VarCSBVersion("csb_version",CSB_VERSION, FCVAR_SPONLY|FCVAR_REPLICATED|FCVAR_NOTIFY, "The version of CS BOT Control");
ConVar g_VarCSBAutoKill("csb_autokill", "1", 0, "If bots should be auto killed when no human players are alive");
ConVar g_VarCSBAutoAdd("csb_autoadd", "6", 0, "What the minimum amount of players are, if player count is bellow this bots are automaticly added");

bool CSBVars::RegisterConCommandBase(ConCommandBase *pVar)
{
	//this will work on any type of concmd!
	//pVar->Create(

	return META_REGCVAR(pVar);
}
int CSBVars::GetAutoAddCount()
{
	return g_VarCSBAutoAdd.GetInt();
}
void CSBVars::SetAutoAddCount(int NewValue)
{
	g_VarCSBAutoAdd.SetValue(NewValue);
}
bool CSBVars::AutoKill()
{
	if(g_VarCSBAutoKill.GetInt() == 0)
		return false;
	
	return true;
}
void CSBVars::AutoKill(bool NewValue)
{
	g_VarCSBAutoKill.SetValue(NewValue);
}