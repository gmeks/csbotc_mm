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

#ifndef _INCLUDE_META_HOOKS_H
#define _INCLUDE_META_HOOKS_H
#define	FIND_IFACE(func, assn_var, num_var, name, type) \
	do { \
	if ( (assn_var=(type)((ismm->func())(name, NULL))) != NULL ) { \
	num = 0; \
	break; \
		} \
		if (num >= 999) \
		break; \
		} while ( num_var=ismm->FormatIface(name, sizeof(name)-1) ); \
		if (!assn_var) { \
		if (error) \
		snprintf(error, maxlen, "Could not find interface %s", name); \
		return false; \
			}

//Declare the hooks we will be using.  Hooking will not compile without these.
//The macro naming scheme is SH_DECL_HOOKn[_void].
//If you have 5 parameters, it would be HOOK5.  If the function is void, add _void.
//It stands for "SourceHook, Declare Hook".
SH_DECL_HOOK3_void(IServerGameDLL, ServerActivate, SH_NOATTRIB, 0, edict_t *, int, int);
SH_DECL_HOOK1_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool);
SH_DECL_HOOK1_void(IServerGameClients, ClientDisconnect, SH_NOATTRIB, 0, edict_t *);
SH_DECL_HOOK2_void(IServerGameClients, ClientPutInServer, SH_NOATTRIB, 0, edict_t *, char const *);
SH_DECL_HOOK1_void(IServerGameClients, SetCommandClient, SH_NOATTRIB, 0, int);
SH_DECL_HOOK2(IGameEventManager2, FireEvent, SH_NOATTRIB, 0, bool, IGameEvent *, bool);
SH_DECL_HOOK1_void(ConCommand, Dispatch, SH_NOATTRIB, 0, CCommand const &);
SH_DECL_HOOK2_void(IServerGameClients, ClientCommand, SH_NOATTRIB, 0, edict_t *, const CCommand &);
#endif //_INCLUDE_META_HOOKS_H
