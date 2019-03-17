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

#include "oslink.h"
#include "CSBotControl.h"
#include "time.h"
#include "cvars.h"
#include "convar.h"
#include "const.h"
#include <string.h>
#include <bitbuf.h>
#include <ctype.h>
#include "recipientfilters.h"

#define GAME_DLL 1
#include "cbase.h"
//#pragma warning(disable:4996)

extern int g_MaxClients;
extern int g_ModIndex;
extern bool g_IsConnected[MAXPLAYERS+1];
extern ConstPlayerInfo UserInfo[MAXPLAYERS+1];
extern ModSettingsStruct g_ModSettings;
extern AdminInterface *m_AdminManager;


void CSBotControl::SendCSay(int id,const char *msg, ...) 
{
	char vafmt[192];
	va_list ap;
	va_start(ap, msg);
	_vsnprintf(vafmt,191,msg,ap);
	va_end(ap);

	for(int i=1;i<=g_MaxClients;i++)	
	{
		if(IsUserConnected(i) && UserInfo[i].IsBot == true)
			m_Engine->ClientPrintf(UserInfo[i].PlayerEdict,vafmt);
	}

	if(g_ModIndex != MOD_CSTRIKE && g_ModIndex != MOD_DOD)
	{
		RecipientFilter mrf;
		
		if(id == 0)
			mrf.AddAllPlayers(g_MaxClients);
		else
			mrf.AddPlayer(id);

		bf_write *msg = m_Engine->UserMessageBegin((RecipientFilter *)&mrf,g_ModSettings.TextMsg);
		//msg->WriteByte(4); //4
		//msg->WriteString(temp);
		msg->WriteByte( 0); //textparms.channel
		msg->WriteFloat( -1 ); // textparms.x ( -1 = center )
		msg->WriteFloat( -.25 ); // textparms.y ( -1 = center )
		msg->WriteByte( 0xFF ); //textparms.r1
		msg->WriteByte( 0x00 ); //textparms.g1
		msg->WriteByte( 0x00 ); //textparms.b1
		msg->WriteByte( 0xFF ); //textparms.a2
		msg->WriteByte( 0xFF ); //textparms.r2
		msg->WriteByte( 0xFF ); //textparms.g2
		msg->WriteByte( 0xFF ); //textparms.b2
		msg->WriteByte( 0xFF ); //textparms.a2
		msg->WriteByte( 0x00); //textparms.effect
		msg->WriteFloat( 0 ); //textparms.fadeinTime
		msg->WriteFloat( 0 ); //textparms.fadeoutTime
		msg->WriteFloat( 10 ); //textparms.holdtime
		msg->WriteFloat( 45 ); //textparms.fxtime
		msg->WriteString( vafmt ); //Message
		m_Engine->MessageEnd();
	}
	else
		MessagePlayer(id,vafmt);
}
bool CSBotControl::IsUserAlive(int id)
{
	CPlayerState *pPlayerState = m_ServerClients->GetPlayerState(UserInfo[id].PlayerEdict);
	if(!pPlayerState)
		return false;

	if(!pPlayerState->deadflag) // This function returns true if dead. so we just reverse it. Why some funtions are IsDead and some are IsAlive beats me
		return true;

	return false;
}
bool CSBotControl::IsUserConnected(int id)
{
	if(!g_IsConnected[id])		// We save the poor cpu some work and check our internal bool first
		return false;

	edict_t *e = m_Engine->PEntityOfEntIndex(id);
	if(!e || e->IsFree() )
		return false;

	//IPlayerInfo *pInfo = g_BATCore.PlayerInfo()->GetPlayerInfo(e);	
	IPlayerInfo *pInfo = m_InfoMngr->GetPlayerInfo(e);
	if(!pInfo || !pInfo->IsConnected())
		return false;
	return true;
}
int CSBotControl::GetUserTeam(int id)
{
	edict_t *e = m_Engine->PEntityOfEntIndex(id);
	if(!e || e->IsFree() || !IsUserConnected(id))
		return TEAM_NOT_CONNECTED;

	IPlayerInfo *pInfo = m_InfoMngr->GetPlayerInfo(e);
	return pInfo->GetTeamIndex();
}
void CSBotControl::GetModName()		// This function gets the actual mod name, and not the full pathname
{
	char path[128];
	char ModName[48];
	m_Engine->GetGameDir(path,127);

	const char *modname = path;
	for (const char *iter = path; *iter; ++iter)
		{
		if (*iter == '/' || *iter == '\\') // or something
			modname = iter + 1;
		}
	strncpy(ModName,modname,47);

	if(strcmp(ModName,"cstrike") == 0)
		g_ModIndex = MOD_CSTRIKE;
	else if(strcmp(ModName,"dod") == 0 || stricmp(ModName,"DODC") == 0)
		g_ModIndex = MOD_DOD;
	else if(strcmp(ModName,"hl2mp") == 0)
		g_ModIndex = MOD_HL2MP;
	else 
		g_ModIndex = MOD_NONE;	
}
int CSBotControl::GetMsgNum(char const *Msg)
{
	char temp[40];
	int dontdoit=0;
	int MaxScan= 20;
	if(g_ModIndex == MOD_CSTRIKE)
		MaxScan = 25;

	for (int i=1;i<=MaxScan;i++) 
	{
		m_ServerDll->GetUserMessageInfo(i,temp,39,dontdoit);
		if(strcmp(Msg,temp) == 0)
			return i;
	}
	return -1;
}

void CSBotControl::SetupModSpesficInfo()	// Gets the menu message id, for the running mod
{
	GetModName();

	g_ModSettings.HintText = GetMsgNum("HintText");
	g_ModSettings.HudMsg = GetMsgNum("HudMsg");
	g_ModSettings.MenuMsg = GetMsgNum("ShowMenu");
	g_ModSettings.TextMsg = GetMsgNum("TextMsg");
	g_ModSettings.SayText = GetMsgNum("SayText");

	if(g_ModIndex == MOD_CSTRIKE || g_ModIndex == MOD_DOD || g_ModIndex == MOD_HL2CTF)
		g_ModSettings.MenuSupport = true;
	else 
		g_ModSettings.MenuSupport = false;		
}
void CSBotControl::ServerCommand(const char *fmt, ...)
{
	char buffer[512];
	va_list ap;
	va_start(ap, fmt);
	_vsnprintf(buffer, sizeof(buffer)-2, fmt, ap);
	va_end(ap);
	strcat(buffer,"\n");
	m_Engine->ServerCommand(buffer);
}
int CSBotControl::FindPlayer(int UserID)	// Finds the player index based on userid.
{
	for(int i=1;i<=g_MaxClients;i++)
	{
		if(g_IsConnected[i] == true)
		{
			if(UserID == UserInfo[i].Userid )	// Name or steamid matches TargetInfo so we return the index of the player
			{
				return i;
			}
		}
	}
	return -1;
}
void CSBotControl::MessagePlayer(int index, const char *msg, ...)
{
	 RecipientFilter rf;
	 if (index > g_MaxClients || index < 0)
		 return;

	 if (index == 0)
	 {
		 rf.AddAllPlayers(g_MaxClients);
	 } else {
		rf.AddPlayer(index);
	 }
	 rf.MakeReliable();

	 char vafmt[192];
	 va_list ap;
	 va_start(ap, msg);
	 int len = _vsnprintf(vafmt, 189, msg, ap);
	 va_end(ap);
	 len += _snprintf(&(vafmt[len]),191-len," \n");

	 bf_write *buffer = m_Engine->UserMessageBegin(static_cast<IRecipientFilter *>(&rf), g_ModSettings.SayText);
	 buffer->WriteByte(0);
	 buffer->WriteString(vafmt);
	 buffer->WriteByte(1);
	 m_Engine->MessageEnd();
}
const char *CSBotControl::GetPlayerName(int id)
{
	return m_Engine->GetClientConVarValue(id,"name");
}
bool CSBotControl::HasAdminImmunity(int id)
{
	if(m_AdminManager == NULL)
		return false;

	return m_AdminManager->HasFlag(id,"immunity");
}
void CSBotControl::LogPrint( const char *msg, ...)
{
	char vafmt[192];
	va_list ap;
	va_start(ap, msg);
	int len = _vsnprintf(vafmt, 189, msg, ap);
	va_end(ap);
	len += _snprintf(&(vafmt[len]),191-len," \n");

	m_Engine->LogPrint(vafmt);
}
/***************************************** Bellow here comes the String util functions ***************************************************/
char* StrUtil::StrRemoveQuotes(char *text)
{
	int len = strlen(text);

	for(int i=0;i<len;i++)
	{
		if(text[i] == '\"')
			text[i] = 0;
	}
	return text;
}
int StrUtil::GetFirstIndexOfChar(char *Text,int MaxLen,unsigned char t)
{
	int TextIndex = 0;
	bool WSS = false; // WhiteSpaceSearch
	if(StrIsSpace(t))
		WSS = true;

	while(TextIndex <= MaxLen)
	{
		if(WSS)		// If the caller is searching for a whitespace, we make sure to search for the other whitespaces to.
		{
			if(StrIsSpace(Text[TextIndex]))
				return TextIndex;
			else
				TextIndex++;
		}
		else if(Text[TextIndex] == t)
		{
			return TextIndex;
		}
		else
			TextIndex++;
	}
	return -1;
}
int StrUtil::StrReplace(char *str, const char *from, const char *to, int maxlen) // Function from sslice 
{
	char  *pstr   = str;
	int   fromlen = Q_strlen(from);
	int   tolen   = Q_strlen(to);
	int	  RC=0;		// Removed count

	while (*pstr != '\0' && pstr - str < maxlen) {
		if (Q_strncmp(pstr, from, fromlen) != 0) {
			*pstr++;
			continue;
		}
		Q_memmove(pstr + tolen, pstr + fromlen, maxlen - ((pstr + tolen) - str) - 1);
		Q_memcpy(pstr, to, tolen);
		pstr += tolen;
		RC++;
	}
	return RC;
}
void StrUtil::StrTrim(char *buffer)
{
	StrTrimLeft(buffer);
	StrTrimRight(buffer);	
}
void StrUtil::StrTrimRight(char *buffer)
{
	// Make sure buffer isn't null
	if (buffer)
	{
		// Loop through buffer backwards while replacing whitespace chars with null chars
		for (int i = (int)strlen(buffer) - 1; i >= 0; i--)
		{
			if (StrIsSpace(buffer[i]))
				buffer[i] = '\0';
			else
				break;
		}
	}
}
bool StrUtil::StrIsSpace(unsigned char b)
{
	switch (b)
	{
	case ' ': 
		return true;
	case '\n':
		return true;
	case '\r':
		return true;
	case '\0': 
		return true;
	}
	return false;
}
/*
More info can be found here: Hashes
http://en.wikipedia.org/wiki/Duff's_device
*/
unsigned long StrUtil::StrHash(register char *str, register int len) 
{
	register unsigned long n = 0;

#define HASHC	n = *str++ + 65587 * n  // The other number is better somehow, magic i guess ( Old num: 65599 )

	if (len > 0) {
		register int loop = (len + 8 - 1) >> 3;

		switch(len & (8 - 1)) {
		case 0:	do {
			HASHC;	case 7:	HASHC;
		case 6:	HASHC;	case 5:	HASHC;
		case 4:	HASHC;	case 3:	HASHC;
		case 2:	HASHC;	case 1:	HASHC;
				} while (--loop);
		}
	}
	return n;
}
void StrUtil::StrTrimLeft(char *buffer)
{
	// Let's think of this as our iterator
	char *i = buffer;

	// Make sure the buffer isn't null
	if (i && *i)
	{
		// Add up number of whitespace characters
		while(StrIsSpace(*i))
			i++;

		// If whitespace chars in buffer then adjust string so first non-whitespace char is at start of buffer
		// :TODO: change this to not use memcpy()!
		if (i != buffer)
			memcpy(buffer, i, (strlen(i) + 1) * sizeof(char));
	}
}