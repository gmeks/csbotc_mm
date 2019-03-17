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

#ifndef _INCLUDE_MENUS_ADMINMENU
#define _INCLUDE_MENUS_ADMINMENU

#include "BATMenu.h"

//extern menuId g_AdminMenu;
class CSBotControlMenu : public IMenu
{
public:
	bool Display(BATMenuBuilder *make, int playerIndex);
	MenuSelectionType MenuChoice(player_t player, int option);

private:
	char const *GetDifficulty();
	char const *GetBoolString(bool MyBool);

	void SetNextDifficulty(int id);
	void SetNextChatter(int id);
	void SetNextFreeze(int id);
	void SetNextAutokill(int id);
	void SetNextJoinTeam(int id);

	void AddBot(int id);
	void RemoveBot(int id);
};
#endif