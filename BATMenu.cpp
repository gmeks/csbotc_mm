/* ======== Basic Admin tool ========
* Copyright (C) 2004-2006 Erling K. Sæterdal
* No warranties of any kind
*
* License: zlib/libpng
*
* Author(s): Erling K. Sæterdal ( EKS )
* Credits:
*	Menu code based on code from CSDM ( http://www.tcwonline.org/~dvander/cssdm ) Created by BAILOPAN
*	Helping on misc errors/functions: BAILOPAN,LDuke,sslice,devicenull,PMOnoTo,cybermind ( most who idle in #sourcemod on GameSurge realy )
* ============================ */

#include "BATMenu.h"
#include "CSBotControl.h"
#include "recipientfilters.h"
#include "usermessages.h"
#include "Color.h"
#include "const.h"
#include "sh_string.h"
#include <sh_vector.h>
#include <bitbuf.h>

extern ConstPlayerInfo UserInfo[MAXPLAYERS+2];
extern ModSettingsStruct g_ModSettings;
extern int g_MaxClients;
extern bool g_IsConnected[MAXPLAYERS+1];
extern bool g_HasMenuOpen[MAXPLAYERS+1];

player_t UserInfoMenu[MAXPLAYERS+2];
extern BATMenuMngr g_MenuMngr;
BATMenuBuilder *g_MenuBuilder;

#define MAX_MENUOPTIONLENGTH 48

char g_MenuOption[10][MAX_MENUOPTIONLENGTH+1];
char g_MenuTitle[MAX_MENUOPTIONLENGTH];
int g_MenuKeys;
int g_ActiveOptions=0;
int g_MenyType=1;

void BATMenuBuilder::AddOption(const char *fmt , ...)
{
	if(g_ActiveOptions >= 10)
	{
		//g_BATCore.AddLogEntry("!ERROR! To many options added to a menu, '%s' was ignored",fmt);
		return;
	}	
	char vafmt[MAX_MENUOPTIONLENGTH+1];
	va_list ap;
	va_start(ap, fmt);
	int len = _vsnprintf(vafmt, MAX_MENUOPTIONLENGTH-3, fmt, ap);
	va_end(ap);

	if(strcmp(vafmt,"\n") != 0 )
		len += _snprintf(&(vafmt[len]),MAX_MENUOPTIONLENGTH-len,"\n");
	vafmt[len] = '\0';

	_snprintf(g_MenuOption[g_ActiveOptions],MAX_MENUOPTIONLENGTH,"%s",vafmt);
	g_ActiveOptions++;
}
void BATMenuBuilder::SetTitle(const char *title)
{
	_snprintf(g_MenuTitle,MAX_MENUOPTIONLENGTH,"%s",title);
}
void BATMenuBuilder::SetKeys(int keys)
{
	g_MenuKeys = keys;
}

unsigned int g_ActiveMenus=0;
SourceHook::CVector<menu_t *>g_MenuList;

int BATMenuMngr::RegisterMenu(IMenu *menu)
{
	g_MenuList.push_back(new menu_t);

	g_MenuList[g_ActiveMenus]->menu = menu;
	g_ActiveMenus++;
	return (int)(g_ActiveMenus-1);
}
void BATMenuMngr::ShowMenu(int player, menuId menuid)
{
	UserInfoMenu[player].index = player;
	UserInfoMenu[player].menu = menuid;
	g_MenuList[menuid]->menu->Display(g_MenuBuilder,player);

	char MenuMsg[192];
	int len=0;

	if(g_MenyType == 1)
	{
		RecipientFilter rf;

		if (player == 0) 
		{
			for(int i=1;i<=g_MaxClients;i++) if(g_IsConnected[i] && UserInfo[i].IsBot)
			{
				UserInfoMenu[i].index = i;
				UserInfoMenu[i].menu = menuid;
				g_HasMenuOpen[i] = true;
				rf.AddPlayer(i);
			}
		} else {
			rf.AddPlayer(player);
		}
		rf.MakeReliable();

		len += _snprintf(MenuMsg,191,"%s\n",g_MenuTitle);
		for(int i=0;i<g_ActiveOptions;i++)
		{
			if(strlen(g_MenuOption[i]) == 0 || g_MenuOption[i][0] == '\n')
				len += _snprintf(&(MenuMsg[len]),191-len,"\n");
			else
				len += _snprintf(&(MenuMsg[len]),191-len,"%d. %s\n",i+1,g_MenuOption[i]);
		}

		bf_write *buffer = g_CSBCore.GetEngine()->UserMessageBegin(static_cast<IRecipientFilter *>(&rf),g_ModSettings.MenuMsg);
		buffer->WriteWord(g_MenuKeys);
		buffer->WriteChar(-1);
		buffer->WriteByte(0);
		buffer->WriteString(MenuMsg);
		g_CSBCore.GetEngine()->MessageEnd();
	}
	else
	{
		g_CSBCore.StrReplace(g_MenuTitle,":","",MAX_MENUOPTIONLENGTH);

		KeyValues *kv = new KeyValues( "menu" );
		kv->SetString( "title", g_MenuTitle );
		kv->SetInt( "level", 1 );
		kv->SetColor( "color", Color( 255, 0, 0, 255 ));
		kv->SetInt( "time", 20 );
		kv->SetString( "msg", "Make your selection in the menu" );

		char num[11], msg[MAX_MENUOPTIONLENGTH+1], cmd[MAX_MENUOPTIONLENGTH+1];

		for(int i=0;i<g_ActiveOptions;i++)
		{
			if(strlen(g_MenuOption[i]) == 0 || g_MenuOption[i][0] == '\n') // i >= g_ActiveOptions || 
				continue;
			
			_snprintf( num, 10, "%i", i );
			_snprintf( cmd, MAX_MENUOPTIONLENGTH, "menuselect %i", i+1 );
			_snprintf( msg, MAX_MENUOPTIONLENGTH, "%i. %s", i+1,g_MenuOption[i] );

			KeyValues *item1 = kv->FindKey( num, true );
			item1->SetString( "msg", msg );
			item1->SetString( "command", cmd );
		}
		if(player == 0)
		{
			for(int i=1;i<=g_MaxClients;i++) if(g_IsConnected[i] && UserInfo[i].IsBot)
			{
				UserInfoMenu[i].index = i;
				UserInfoMenu[i].menu = menuid;
				g_HasMenuOpen[i] = true;
				g_CSBCore.GetHelpers()->CreateMessage( UserInfo[i].PlayerEdict, DIALOG_MENU, kv, this );
			}
		}
		else
			g_CSBCore.GetHelpers()->CreateMessage( UserInfo[player].PlayerEdict, DIALOG_MENU, kv, this );

		kv->deleteThis();
	}
	g_ActiveOptions=0;
}
void BATMenuMngr::MenuChoice(int playerIndex, int choice)
{
	menuId menuid = UserInfoMenu[playerIndex].menu;

	switch(g_MenuList[menuid]->menu->MenuChoice(UserInfoMenu[playerIndex], choice))
	{
	case MENUSELECTION_GOOD:
		g_CSBCore.GetEngine()->ClientCommand(UserInfo[playerIndex].PlayerEdict,PLAYSOUND_GOOD);
		break;
	case MENUSELECTION_BAD:
		g_CSBCore.GetEngine()->ClientCommand(UserInfo[playerIndex].PlayerEdict,PLAYSOUND_BAD);
	    break;
	}
}
/*
void BATMenuMngr::ShowESCMenu(int id)
{
	KeyValues *kv = new KeyValues( "menu" );
	kv->SetString( "title", "You've got options, hit ESC" );
	kv->SetInt( "level", 1 );
	kv->SetColor( "color", Color( 255, 0, 0, 255 ));
	kv->SetInt( "time", 20 );
	kv->SetString( "msg", "Pick an option\nOr don't." );

	for( int i = 1; i <= 10; i++ )
	{
		char num[11], msg[10], cmd[10];
		_snprintf( num, sizeof(num), "%i", i );
		_snprintf( msg, sizeof(msg), "Option %i", i );
		_snprintf( cmd, sizeof(cmd), "option%i", i );

		KeyValues *item1 = kv->FindKey( num, true );
		item1->SetString( "msg", msg );
		item1->SetString( "command", cmd );
	}

	g_BATCore.GetHelpers()->CreateMessage( UserInfo[id].PlayerEdict, DIALOG_MENU, kv, this );
	kv->deleteThis();
}
*/