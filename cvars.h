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

#ifndef _INCLUDE_CVARS_H
#define _INCLUDE_CVARS_H

#include "convar.h"

class CSBVars : public IConCommandBaseAccessor
{
public:
	virtual bool RegisterConCommandBase(ConCommandBase *pVar);
	bool AutoKill();
	void AutoKill(bool NewValue);
	void SetAutoAddCount(int NewValue);
	int GetAutoAddCount();
private:
	
};

extern CSBVars g_CSBVars;

#endif //_INCLUDE_CVARS_H
