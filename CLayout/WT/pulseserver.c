
// pulseserver.c
// This file implements inSight pulse and login server list functions


#include <string.h>
#include "pulseserver.h"
#include "InstanceData.h"

extern int               vo_log_info( va_alist );
extern int               vo_log_error( va_alist );

int CrntPulseSrvr = 0;				// Current pulse server index
int g_nPulseSvrFailCnt;		// Failure count for Pulse Server
time_t g_tPulseSvrFailTime;	// Fail time for Pulse Server
char **g_pPulseServers;
int g_nPulseSrvrCnt = 0;

static char *PulseSrvrs[] = {
		"https://www50.passur.com/fcgi/PulseTrack.fcg",
		"https://www51.passur.com/fcgi/PulseTrack.fcg",
		"https://www3.passur.com/fcgi/PulseTrack.fcg",
		"https://www3.passur.net/fcgi/PulseTrack.fcg",
		"https://www11.passur.com/fcgi/PulseTrack.fcg",
		"https://www11.passur.net/fcgi/PulseTrack.fcg" };

void LogToServer(struct InstanceData* id, enum LOG_LEVEL eLevel, const char* strCategory, const char* strLogData);
int GetConfigValueInt(struct InstanceData* id, const char* strName, int nDefault);

extern int IsDebugNameDefined(struct InstanceData* id, const char* strDebugName);
#define D_CONTROL(vcolname, exp) {if ( 5 == id->m_nDebugControl || (id->m_nDebugControl && IsDebugNameDefined(id, vcolname)) ) {exp;}}

/*
** char * GetPulseSrvr()
**
**	Returns:	Server URL to be used
**
*/


char *GetPulseSrvr(struct InstanceData* id) 
{
	if( g_nPulseSrvrCnt ){
		D_CONTROL("SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Returning: %s\n", GetCurrentThreadId(), g_pPulseServers[CrntPulseSrvr]));
		return ( g_pPulseServers[CrntPulseSrvr] );
	}

	D_CONTROL("SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Returning: %s\n", GetCurrentThreadId(), PulseSrvrs[CrntPulseSrvr]));
	return ( PulseSrvrs[CrntPulseSrvr] );
}

/*
** char * GetNxtPulseSrvr()
**
**	Returns:	Next server URL in the list of servers
**				
**	Cuerrent Pulse server index is also set to next server (or first if
**  currently pointing to last server).
**
*/

char* GetNxtPulseSrvr(struct InstanceData* id)
{	
	time_t now = time(NULL);
	int nTimeOut = GetConfigValueInt(id, "SERVER_TIMEOUT", 300);
	int nCount = GetConfigValueInt(id, "SERVER_TIMEOUT_COUNT", 5);

	if(!g_tPulseSvrFailTime || (now - g_tPulseSvrFailTime) > nTimeOut){
		g_nPulseSvrFailCnt = 1;
		g_tPulseSvrFailTime = now;
	}else{
		++g_nPulseSvrFailCnt;
		if(g_nPulseSvrFailCnt > nCount){
			char buffer[STATUSSTRING_SZ] = {0};
			g_nPulseSvrFailCnt = 1;
			_snprintf_s(buffer, STATUSSTRING_SZ, _TRUNCATE, "Pulse Server failed %d times in %d secs.", nCount, (now - g_tPulseSvrFailTime));
			LogToServer(id, LOG_ERR, "NETWORK", buffer);
		}
	}
	if( g_nPulseSrvrCnt ){
		CrntPulseSrvr = CrntPulseSrvr < (g_nPulseSrvrCnt - 1)? CrntPulseSrvr + 1: 0;
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntPulseSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntPulseSrvr, g_pPulseServers[CrntPulseSrvr]);
		return ( g_pPulseServers[CrntPulseSrvr] );
	}	

	CrntPulseSrvr = CrntPulseSrvr < (PULS_SRVR_LIST_SZ - 1)? CrntPulseSrvr + 1: 0;
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntPulseSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntPulseSrvr, PulseSrvrs[CrntPulseSrvr]);
	return ( PulseSrvrs[CrntPulseSrvr] );
}




