#ifndef _INCLUDE_IMENU_H
#define _INCLUDE_IMENU_H

#include <interface.h>
#include <eiface.h>
#include "const.h"
#include "BATMenu.h"

enum MenuSelectionType
{
	MENUSELECTION_GOOD,
	MENUSELECTION_BAD,
	MENUSELECTION_NOSOUND,
};

class BATMenuBuilder
{
public:
	void SetTitle(const char *title);
	void AddOption(const char *fmt , ...);
	void SetKeys(int keys);

private:

};

typedef	unsigned int	menuId;

struct player_t
{
	menuId menu;
	int index;
	int data;
};
class IMenu
{
public:
	virtual ~IMenu() { };
public:
	virtual bool Display(BATMenuBuilder *make, int playerIndex) =0;
	virtual MenuSelectionType MenuChoice(player_t player, int option) =0;
};
#endif //_INCLUDE_IMENU_H
