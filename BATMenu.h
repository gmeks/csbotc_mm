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

#ifndef _INCLUDE_BATMENU_H
#define _INCLUDE_BATMENU_H

#include <iserverplugin.h>
#include "IMenu.h"
#include "const.h"

//typedef	unsigned int menuId;
struct menu_t
{
	IMenu *menu;
	int id;
};
// The Sounds played in the menus
#define SOUND_CHOICEMADE "buttons/button14.wav"
#define SOUND_INVALID "buttons/button18.wav"

#define PLAYSOUND  "play "
#define PLAYSOUND_GOOD PLAYSOUND SOUND_CHOICEMADE
#define PLAYSOUND_BAD PLAYSOUND SOUND_INVALID

class BATMenuMngr : public IServerPluginCallbacks
{
public:
	int RegisterMenu(IMenu *menu);
	void ShowMenu(int player, menuId menuid);
	void MenuChoice(int playerIndex, int choice);

	virtual bool			Load(	CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory ) {return true;};
	virtual void			Unload( void ) {};
	virtual void			Pause( void ) {};
	virtual void			UnPause( void ) {};
	virtual const char     *GetPluginDescription( void ) {return "FakeInfo"; };      
	virtual void			LevelInit( char const *pMapName ) {};
	virtual void			ServerActivate( edict_t *pEdictList, int edictCount, int clientMax ) {};
	virtual void			GameFrame( bool simulating ) {};
	virtual void			LevelShutdown( void ) {};
	virtual void			ClientActive( edict_t *pEntity ) {};
	virtual void			ClientDisconnect( edict_t *pEntity ) {};
	virtual void			ClientPutInServer( edict_t *pEntity, char const *playername ) {};
	virtual void			SetCommandClient( int index ) {};
	virtual void			ClientSettingsChanged( edict_t *pEdict ) {};
	virtual PLUGIN_RESULT	ClientConnect( bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen )  {return PLUGIN_CONTINUE; };
	virtual PLUGIN_RESULT	ClientCommand( edict_t *pEntity )  {return PLUGIN_CONTINUE; }; 
	virtual PLUGIN_RESULT	NetworkIDValidated( const char *pszUserName, const char *pszNetworkID ) {return PLUGIN_CONTINUE; };
	virtual void FireGameEvent( KeyValues * event ) {};
	virtual int GetCommandIndex() { return 0;}

#if BAT_DEBUG == 1
	//void ShowESCMenu(int id);
#endif

private:

};
#endif //_INCLUDE_CVARS_H