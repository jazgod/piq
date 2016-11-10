// servers.c
// This file implements inSight server list functions


#include <string.h>
#include "Curl.h"
#include "servers.h"
#include "LoginDialog.h"
#include "pulseserver.h"
#include "InstanceData.h"
#include "json.h"

#define SERVER_PATH_SZ 100

// FTrack Server Variables
static char **g_pFTracksServers;	// Dynamic FTrack ServerList
static int g_nFTrackSrvrCnt = 0;	// FTrack Server Count
static int g_nFTrackSvrFailCnt;		// Failure count for FTrack Server
static time_t g_tFtrackSvrFailTime;	// Fail time for FTrack Server

// Tail Server Variables
static char **g_pTailFileServers;	// Dynamic Tails ServerList
static int g_nTailFileSrvrCnt = 0;	// Tails Server Count
static int g_nTailSvrFailCnt;		// Failure count for Tails Server
static time_t g_tTailSvrFailTime;	// Fail time for Tails Server

// Weather (Wx) Server Variables
static char **g_pWxServers;
static int g_nWxSrvrCnt = 0;
static int g_nWxSvrFailCnt;
static time_t g_tWxSvrFailTime;

// PulseTrack Server Variables
extern char **g_pPulseServers;
extern int g_nPulseSrvrCnt;
//int g_nPulseSvrFailCnt;			// Failure count for Pulse Server
//time_t g_tPulseSvrFailTime;		// Fail time for Pulse Server

// ASDE-X Server Variables
static char **g_pAsdexServers;
static int g_nAsdexSrvrCnt = 0;
static time_t g_tAsdexSvrFailTime;	// Fail time for Asdex Server// ADS-B Server Variables
static int g_nAsdexSvrFailCnt;		// Failure count for ASDEX Server

// ASD Server Variables
static char **g_pAsdServers;
static int g_nAsdSrvrCnt = 0;
static time_t g_tAsdSvrFailTime;	// Fail time for ASD Server Variables
static int g_nAsdSvrFailCnt;		// Failure count for ASD Server

// ADS-B Server Variables
static char **g_pAdsbServers;
static int g_nAdsbSrvrCnt = 0;
static time_t g_tAdsbSvrFailTime;	// Fail time for Adsb Server
static int g_nAdsbSvrFailCnt;		// Failure count for ADS-B Server

// AirAsia Server Variables
static char **g_pAirAsiaServers;
static int g_nAirAsiaSrvrCnt = 0;
static time_t g_tAirAsiaSvrFailTime;	// Fail time for AirAsia Server
static int g_nAirAsiaSvrFailCnt;		// Failure count for AirAsia data feed Server

// MLAT Server Variables
static char **g_pMlatServers;
static int g_nMlatSrvrCnt = 0;
static time_t g_tMlatSvrFailTime;	// Fail time for MLAT Server
static int g_nMlatSvrFailCnt;		// Failure count for MLAT Server

// LMG Server Variables
static char **g_pLMGServers;
static int g_nLMGSrvrCnt = 0;
static time_t g_tLMGSvrFailTime;	// Fail time for LMG Server
static int g_nLMGSvrFailCnt;		// Failure count for LMG Server

// Passur Server Variables
static char **g_pPassurServers;
static int g_nPassurSrvrCnt = 0;
static time_t g_tPassurSvrFailTime;	// Fail time for Passur Server
static int g_nPassurSvrFailCnt;		// Failure count for Passur Server

// Noise Server Variables
static char **g_pNoiseServers;
static int g_nNoiseSrvrCnt = 0;
// NOT CURRENTLY USED
//static time_t g_tNoiseSvrFailTime;	// Fail time for Noise Server
//static int g_nNoiseSvrFailCnt;		// Failure count for Noise Server

static char** g_pInsightServers;
static int g_nInsightSrvrCnt = 0;
	
static char *AsdexSrvrs[] = {
		"https://www50.passur.com",
		"https://www51.passur.com",
		"https://www10.passur.com",
		"https://www10.passur.net",
		"https://www15.passur.com",
		"https://www15.passur.net" };

static char *AsdSrvrs[] = {
		"https://www50.passur.com",
		"https://www53.passur.com",
		"https://209.208.43.19",
		"https://209.208.43.16" };

static char *AdsbSrvrs[] = {
		"https://www15.passur.com",
		"https://www15.passur.net" };

static char *AirAsiaSrvrs[] = {
		"https://www52.passur.com" };

static char *MlatSrvrs[] = {
		"https://www54.passur.com" };

static char *LMGSrvrs[] = {
		"https://www52.passur.com" };

static char *PassurSrvrs[] = {
		"www53.passur.com",
		"www50.passur.com",
		"209.208.43.19",
		"209.208.43.16" };

static char *CurrentFTracksServers[] = {
   	    "https://www50.passur.com/fcgi/FusedTracks.fcg",
		"https://www53.passur.net/fcgi/FusedTracks.fcg",
		"https://www3.passur.com/fcgi/FusedTracks.fcg",
		"https://www3.passur.net/fcgi/FusedTracks.fcg"};

static char *CurrentTailFileServers[] = {
		"https://www50.passur.com/sdata",
		"https://www53.passur.net/sdata",
		"https://www10.passur.com/sdata",
		"https://www10.passur.net/sdata"};

static char *WxSrvrs[] = {
        "https://www50.passur.com", 
        "https://www53.passur.com",
		"https://www8.passur.com",
		"https://www8.passur.net" };

static char *NoiseSrvrs[] = {
	"https://www54.passur.com" }; 

int CrntFTrackSrvr = 0;				// Current ftrack server index
int CrntTailsSrvr = 0;				// Current tails server index
int CrntWxSrvr = 0;		   			// Current wx server index
int CrntAsdexSrvr = 0;   			// Current asdex server index
int CrntAsdSrvr = 0;   				// Current ASD server index
int CrntAdsbSrvr = 0;   			// Current adsb server index
int CrntAirAsiaSrvr = 0;   			// Current AirAsia server index
int CrntMlatSrvr = 0;   			// Current MLAT server index
int CrntNoiseSrvr = 0;   			// Current noise server index
int CrntLMGSrvr = 0;   				// Current Last Mile Gear server index
int CrntPassurSrvr = 0;   				// Current Passur server index

static	char	*servers[SERVER_MAXLIST];	// List of servers
static	int		serverIndex;				// Index into servers[]
static	int		serverCount;				// Size of servers[]
static	int		firstTime=1;				// Init flag
static	struct	MemoryStruct chunk;			// Curl results buffer
static	int		serverFailures;				// Count of consecutive server failures

void LogToServer(struct InstanceData* id, enum LOG_LEVEL eLevel, const char* strCategory, const char* strLogData);
int GetConfigValueInt(struct InstanceData* id, const char* strName, int nDefault);
int               vo_log_info( va_alist );
int               vo_log_error( va_alist );
int               vo_log_warning( va_alist );

extern int IsDebugNameDefined(struct InstanceData* id, const char* strDebugName);
#define D_CONTROL(vcolname, exp) {if ( 5 == id->m_nDebugControl || ( id->m_nDebugControl && IsDebugNameDefined(id, vcolname)) ) {exp;}}

static int connectToServer(struct InstanceData* id, char * server_url);
BOOL LoadPassurAirports( struct InstanceData* id );
void SetUpPassur(struct InstanceData* id);
/*
** char * getServerAddr ( FLAG )
**
**	Returns:	On Success:	Server URL to be used
**	        	On Failure:	NULL
**
**	FLAG Vlaues:
**			SERVER_CURRENT		// Server currently being used
**			SERVER_NEXT			// Next server on the list
**			SERVER_PRIMARY		// Asigned primary server (first one on the list).
**			SERVER_REFRESH		// Updated server list (get a new list)
**
** NS 11/2006
*/

char *getServerAddr( struct InstanceData* id, int flag ) 
{
    int	rc;
	const DWORD cdwThreadId = GetCurrentThreadId();

    D_CONTROL("SERVER_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) firstTime <%d> flag = <%d>.\n", cdwThreadId, firstTime, flag));

    /*-------------------------------------------------*/
    /* If first time or SERVER_REFRESH request         */
    /*-------------------------------------------------*/

    if ( (!g_nInsightSrvrCnt && firstTime) || (!g_nInsightSrvrCnt && (flag == SERVER_REFRESH ))) {

      /*-----------------------------------------------*/
      /* Attempt to connect to Primary server list     */
      /*-----------------------------------------------*/

      rc = connectToServer(id, SERVER_URL_PRIMARY);
      D_CONTROL("SERVER_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Primary rc = <%d>\n", cdwThreadId, rc ));

      /*-----------------------------------------------*/
      /* Check connection results                      */
      /*-----------------------------------------------*/

      if (rc == FALSE) {

        vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Primary connection to server failed.\n", cdwThreadId);

	/*---------------------------------------------*/
	/* Attempt to connect to Secondary server list */
	/*---------------------------------------------*/

	    rc = connectToServer(id, SERVER_URL_SECONDARY);
        D_CONTROL("SERVER_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Secondary rc = '%d'\n", cdwThreadId, rc ));
      }

      /*-----------------------------------------------*/
      /* Check to see if PRIMARY or SECONDARY list was */
      /* successfully retrieved                        */
      /*-----------------------------------------------*/

      if (rc == TRUE) {
         serverIndex = 0;
      } else {
         vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to get server list. Using defaults\n", cdwThreadId);
         serverCount = 0;
		 servers[0] = "www50.passur.com";
		 servers[1] = "www8.passur.com";
      }

      /*-----------------------------------------------*/
      /* Clear first time flag                         */
      /*-----------------------------------------------*/

      if ( *id->m_sLogin.User ) {
	     firstTime = 0;
      }  

     D_CONTROL("SERVER_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) returning Server[0] = '%s'\n", cdwThreadId, servers[0] ));

     return ( servers[0] );

    } 

    switch ( flag ) {
    case SERVER_CURRENT:
      break;
    case SERVER_NEXT:
		{
			if(g_nInsightSrvrCnt)
				serverIndex = ++serverIndex > (g_nInsightSrvrCnt-1)? 0: serverIndex;
			else
				serverIndex = ++serverIndex > serverCount? 0: serverIndex;
		}
      break;
    case SERVER_PRIMARY:
      serverIndex = 0;
      break;
    default:
      // same as SERVER_CURRENT
      break;
    }    

	if(g_nInsightSrvrCnt){
		D_CONTROL("SERVER_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) returning server_Index <%d> servers <%s>.\n", 
			cdwThreadId, serverIndex, g_pInsightServers[serverIndex]));
		return ( g_pInsightServers[serverIndex] );
	}

    D_CONTROL("SERVER_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) returning server_Index <%d> servers <%s>.\n", 
	    cdwThreadId, serverIndex, servers[serverIndex]));
    return ( servers[serverIndex] );
}

void setWebTrackerServerVars(struct InstanceData* id) {
	char *p = NULL;
	const DWORD cdwThreadId = GetCurrentThreadId();

	// Initialize DB Servernames here for WT.dll use
	strcpy_s(id->m_strHttpDBservername[0], SERVERNAME_SZ, id->m_strDoNotUse);	
	strcpy_s(id->m_strHttpDBservername[1], SERVERNAME_SZ, id->m_strDoNotUse);

	// This makes one DB servername (path to Surf.fcg) use .com and one use .net
	if(p = strstr(id->m_strHttpDBservername[1], ".com")){
		*(p+1) = 'n';
		*(p+2) = 'e';
		*(p+3) = 't';
	}/*else if(p = strstr(id->m_strHttpDBservername[0], ".net")){
		*(p+1) = 'c';
		*(p+2) = 'o';
		*(p+3) = 'm';
	}*/
	D_CONTROL("SERVER_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Primary m_strHttpDBservername = <%s>\n", cdwThreadId, id->m_strHttpDBservername[0]));
	D_CONTROL("SERVER_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Alternate m_strHttpDBservername = <%s>\n", cdwThreadId, id->m_strHttpDBservername[1]));
}

void setinsightServerVars(struct InstanceData* id) {
  char *srvr, *p = NULL;
  int len;
  const DWORD cdwThreadId = GetCurrentThreadId();

  // no log in so hardwire username
  strcpy_s(id->m_sLogin.User, LOGIN_MAX_SZ, "fwuser");
  strcpy_s(id->m_sLogin.Password, LOGIN_MAX_SZ, "rhino");

  D_CONTROL("SERVER_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Logging Test\n", cdwThreadId));
  D_CONTROL("SERVER_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) firstTime <%d>.\n", cdwThreadId, firstTime));
  D_CONTROL("SERVER_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) flag = <%d>.\n", cdwThreadId, SERVER_CURRENT));
  D_CONTROL("SERVER_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) firstTime <%d> flag = <%d>.\n", cdwThreadId, firstTime, SERVER_CURRENT));
  srvr=getServerAddr(id, SERVER_CURRENT);

  if ( (srvr=getServerAddr(id, SERVER_CURRENT)) && strlen( srvr ) ) {  
	len = (strlen(srvr));

    D_CONTROL("SERVER_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) srvr string length <%d> Found server='%s'\n", cdwThreadId, len, srvr));
  
    // Force AirportVO to be reloaded from new PulseTrack server
	if( LoadPassurAirports( id ) )
		SetUpPassur( id );

  } else {
	  vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to get server URL. Not changing anything.\n", cdwThreadId);
  }

  D_CONTROL("SERVER_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Current ASDI Server <%s>\n", cdwThreadId, GetAsdSrvr(id)));
  D_CONTROL("SERVER_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Current ADS-B Server <%s>\n", cdwThreadId, GetAdsbSrvr(id)));
  D_CONTROL("SERVER_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Current ASDE-X Server <%s>\n", cdwThreadId, GetAsdexSrvr(id)));
  D_CONTROL("SERVER_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CurrentPulseServer = <%s>\n", cdwThreadId, GetPulseSrvr(id)));
  D_CONTROL("SERVER_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CurrentFTracksServer = <%s>\n", cdwThreadId, GetFTrackSrvr(id)));
  D_CONTROL("SERVER_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CurrentMLATServer = <%s>\n", cdwThreadId, GetMlatSrvr(id)));
  D_CONTROL("SERVER_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CurrentPassurServer = <%s>\n", cdwThreadId, GetPassurSrvr(id)));
  D_CONTROL("SERVER_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CurrentTailFilesServer = <%s>\n", cdwThreadId, GetTailsSrvr(id)));
  D_CONTROL("SERVER_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CurrentNoiseServer = <%s>\n", cdwThreadId, GetNoiseSrvr(id)));

} /* End of void setinsightServerVars() */


void noteServerSuccess(struct InstanceData* id) {

  if ( firstTime ) {
    getServerAddr(id,  SERVER_REFRESH);
  }
  serverFailures = 0;
  if(!id->m_bClass1)
    Callback2Javascript(id, "PassurStatus:Slow");
  else
    Callback2Javascript(id, "PassurStatus:OK");
}


void
noteServerFailure(struct InstanceData* id) {
  const DWORD cdwThreadId = GetCurrentThreadId();
  vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) serverFailures=%d failcount is %d\n", cdwThreadId, serverFailures + 1, SERVER_SWITCH_PNT );
	if ( ++serverFailures > SERVER_SWITCH_PNT ) {
		char buffer[STATUSSTRING_SZ] = {0};
		char *srvr1 = getServerAddr(id, SERVER_CURRENT);
		char *srvr2 = getServerAddr(id, SERVER_NEXT);
		serverFailures = 0;
		setinsightServerVars(id);		
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Switched server to %s\n", cdwThreadId, srvr2);
		_snprintf_s(buffer, STATUSSTRING_SZ, _TRUNCATE, "Server %s failed %d times, switching to %s.", srvr1, SERVER_SWITCH_PNT, srvr2);
		LogToServer(id, LOG_ERR, "NETWORK", buffer);
	}	
}

static int connectToServer(struct InstanceData* id, char * server_url)
{
   int	i, rc;
   char posts[SERVER_POSTS_SZ];
   char *context;
   const DWORD cdwThreadId = GetCurrentThreadId();

   vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) server_url <%s>\n", cdwThreadId, server_url);

   sprintf_s(posts, SERVER_POSTS_SZ, "%s%s", SERVER_POSTS, id->m_sLogin.User);

   /*------------------------------------------------------------*/
   /* Release the memory if it was assigned previously           */
   /*------------------------------------------------------------*/

   if ( chunk.memory ) {
     free ( chunk.memory );
     chunk.memory = NULL;
     chunk.size = 0;

     /* Clear out the servers list */
     for (i=0; i < SERVER_MAXLIST; i++ ) {
       servers[i] = NULL;
     }
   }

   /*-----------------------------------------------------------------*/
   /* Attempt to connect multiple times in case of errors             */
   /*-----------------------------------------------------------------*/

   for (i = serverIndex = 0; i < 3; i++ ) {

     /*---------------------------------------------------------------*/
     /* Get Curl Results                                              */ 
     /*---------------------------------------------------------------*/

     vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlFormGetResults - server_url <%s> posts <%s>.\n", 
			cdwThreadId, server_url, posts );

     rc = GetCurlFormGetResults( id, (void *) &chunk, server_url, posts, SM_HTTP, FALSE );
     if ( (rc != FALSE) && chunk.memory && ((! strstr(chunk.memory, "Not Found")))) {

       /*-------------------------------------------------------------*/
       /* Log the response from the Curl                              */
       /*-------------------------------------------------------------*/

       vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Server list = <%s>\n", cdwThreadId, chunk.memory );

       /*-------------------------------------------------------------*/
       /* Parse the server from the list                              */
       /*-------------------------------------------------------------*/

       if ( (servers[serverIndex++] = strtok_s(chunk.memory, ",", &context))) {

	 /*-----------------------------------------------------*/
	 /* Parse the next server from the response             */
	 /*-----------------------------------------------------*/

	 while ( (servers[serverIndex++] = strtok_s((char * )NULL, ",", &context))
		 && serverIndex < SERVER_MAXLIST );
       }

       /*-------------------------------------------------------------*/
       /* Calc number of servers in response                          */ 
       /*-------------------------------------------------------------*/

       serverCount = serverIndex - 1;
       break; 

     } else {

       vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not get Primary Server list...i = <%d> serverIndex = <%d> Trying again.\n", 
		cdwThreadId, i, serverIndex);

       /*-------------------------------------------------------------*/
       /* Wait a couple of seconds before trying again                */
       /*-------------------------------------------------------------*/

       Sleep ( 2000 );
     }
   }

   /*-----------------------------------------------------------------*/
   /* Validate the retrieval of the server records.   
   /*-----------------------------------------------------------------*/   
   if( !serverCount ){
	   vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not get Server list, serverCount = <%d>.\n", 
		   cdwThreadId, serverCount); 
	   return(FALSE);
   }

   vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) serverIndex: <%d> serverCount: <%d>\n", cdwThreadId, serverIndex, serverCount);

   /*-----------------------------------------------------------------*/
   /* Validate the length of each of the server records.   This is    */
   /* necessary because the ISP might respond with an error other     */
   /* than NOT FOUND                                                  */
   /*-----------------------------------------------------------------*/

   for (i=0; i < serverIndex -1; i++ ) {

     int length;
		
     if (servers[i] == NULL) {
	   vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Warning:  servers entry is NULL\n", cdwThreadId);
	   serverIndex = 0;
       return(FALSE);
     }

     length = strlen(servers[i]);
	      
     if ((length == 0) || (length > 132)) {
       
       /*-------------------------------------------------------------*/
       /* If length zero or greater than 132 then record is invalid   */
       /*-------------------------------------------------------------*/	      

       vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Warning: Invalid servers [%d] record: <%s> length = <%d>.\n", cdwThreadId, i, servers[i], length);

       serverIndex = 0;
       return(FALSE);
     }
   } 

   vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Servers list has been validated - number of entries <%d>\n", cdwThreadId, serverIndex);

   return(TRUE);
}
/*Used to set CrntFtrackSrv tozero.
Currently used in GetBlockModeReplay function in srf.c
*/
void SetCrntFtrackSrvCnt0()
{
	CrntFTrackSrvr = 0;
}
/*Obtain count of ServerList
Currently used in GetBlockModeReplay function in srf.c
*/
int GetFTrackSrvCnt()
{
	if (g_nFTrackSrvrCnt){
		//retun count of dynamic list: g_pFtracksServer 
		return(g_nFTrackSrvrCnt);
	}
	else{
		//retun count of static list: CurrentFtracksServers
		return(4);
	}
}

char* GetFTrackSrvr(struct InstanceData* id)
{
	if(g_nFTrackSrvrCnt){
		D_CONTROL("SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Returning: %s\n", GetCurrentThreadId(), g_pFTracksServers[CrntFTrackSrvr]));
		return ( g_pFTracksServers[CrntFTrackSrvr] );
	}
	D_CONTROL("SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Returning: %s\n", GetCurrentThreadId(), CurrentFTracksServers[CrntFTrackSrvr]));
	return ( CurrentFTracksServers[CrntFTrackSrvr] );
}


char* GetNxtFTrackSrvr(struct InstanceData* id)
{
	time_t now = time(NULL);
	int nTimeOut = GetConfigValueInt(id, "SERVER_TIMEOUT", 300);
	int nCount = GetConfigValueInt(id, "SERVER_TIMEOUT_COUNT", 5);

	if(!g_tFtrackSvrFailTime || (now - g_tFtrackSvrFailTime) > nTimeOut){
		g_nFTrackSvrFailCnt = 1;
		g_tFtrackSvrFailTime = now;
	}else{
		++g_nFTrackSvrFailCnt;
		if(g_nFTrackSvrFailCnt > nCount){
			char buffer[STATUSSTRING_SZ] = {0};
			g_nFTrackSvrFailCnt = 1;
			_snprintf_s(buffer, STATUSSTRING_SZ, _TRUNCATE, "FTrack Server failed 5 times in %d secs.", (now - g_tFtrackSvrFailTime));
			LogToServer(id, LOG_ERR, "NETWORK", buffer);
		}
	}
	if(g_nFTrackSrvrCnt){
		CrntFTrackSrvr = CrntFTrackSrvr < (g_nFTrackSrvrCnt - 1)? CrntFTrackSrvr + 1: 0;
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntFTrackSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntFTrackSrvr, g_pFTracksServers[CrntFTrackSrvr]);
		return ( g_pFTracksServers[CrntFTrackSrvr] );
	}
	CrntFTrackSrvr = CrntFTrackSrvr < (FTRACK_SRVR_LIST_SZ - 1)? CrntFTrackSrvr + 1: 0;
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntFTrackSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntFTrackSrvr, CurrentFTracksServers[CrntFTrackSrvr]);
	return ( CurrentFTracksServers[CrntFTrackSrvr] );
}

char* GetTailsSrvr(struct InstanceData* id)
{
	if( g_nTailFileSrvrCnt ){
		D_CONTROL("SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Returning: %s\n", GetCurrentThreadId(), g_pTailFileServers[CrntTailsSrvr]));
		return ( g_pTailFileServers[CrntTailsSrvr] );
	}
	D_CONTROL("SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Returning: %s\n", GetCurrentThreadId(), CurrentTailFileServers[CrntTailsSrvr]));
	return ( CurrentTailFileServers[CrntTailsSrvr] );
}


char* GetNxtTailsSrvr(struct InstanceData* id)
{
	time_t now = time(NULL);
	int nTimeOut = GetConfigValueInt(id, "SERVER_TIMEOUT", 300);
	int nCount = GetConfigValueInt(id, "SERVER_TIMEOUT_COUNT", 5);
	if(!g_tTailSvrFailTime || (now - g_tTailSvrFailTime) > nTimeOut){
		g_nTailSvrFailCnt = 1;
		g_tTailSvrFailTime = now;
	}else{
		++g_nTailSvrFailCnt;
		if(g_nTailSvrFailCnt > nCount){
			char buffer[STATUSSTRING_SZ] = {0};
			g_nTailSvrFailCnt = 1;
			_snprintf_s(buffer, STATUSSTRING_SZ, _TRUNCATE, "Tails Server failed 5 times in %d secs.", (now - g_tTailSvrFailTime));
			LogToServer(id, LOG_ERR, "NETWORK", buffer);
		}
	}
	if(g_nTailFileSrvrCnt){
		CrntTailsSrvr = CrntTailsSrvr < (g_nTailFileSrvrCnt - 1)? CrntTailsSrvr + 1: 0;
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntTailsSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntTailsSrvr, g_pTailFileServers[CrntTailsSrvr]);
		return ( g_pTailFileServers[CrntTailsSrvr] );
	}
	CrntTailsSrvr = CrntTailsSrvr < (TAILS_SRVR_LIST_SZ - 1)? CrntTailsSrvr + 1: 0;
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntTailsSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntTailsSrvr, CurrentTailFileServers[CrntTailsSrvr]);
	return ( CurrentTailFileServers[CrntTailsSrvr] );
}

char* GetWxSrvr(struct InstanceData* id)
{
	if( g_nWxSrvrCnt ){
		D_CONTROL("SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Returning: %s\n", GetCurrentThreadId(), g_pWxServers[CrntWxSrvr]));
		return ( g_pWxServers[CrntWxSrvr] );
	}

	D_CONTROL("SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Returning: %s\n", GetCurrentThreadId(), WxSrvrs[CrntWxSrvr]));
	return ( WxSrvrs[CrntWxSrvr] );
}

char* GetNxtWxSrvr(struct InstanceData* id)
{
	time_t now = time(NULL);
	int nTimeOut = GetConfigValueInt(id, "SERVER_TIMEOUT", 300);
	int nCount = GetConfigValueInt(id, "SERVER_TIMEOUT_COUNT", 5);
	if(!g_tWxSvrFailTime || (now - g_tWxSvrFailTime) > nTimeOut){
		g_nWxSvrFailCnt = 1;
		g_tWxSvrFailTime = now;
	}else{
		++g_nWxSvrFailCnt;
		if(g_nWxSvrFailCnt > nCount){
			char buffer[STATUSSTRING_SZ] = {0};
			g_nWxSvrFailCnt = 1;
			_snprintf_s(buffer, STATUSSTRING_SZ, _TRUNCATE, "WX Server failed 5 times in %d secs.", (now - g_tWxSvrFailTime));
			LogToServer(id, LOG_ERR, "NETWORK", buffer);
		}
	}
	if( g_nWxSrvrCnt ){
		CrntWxSrvr = CrntWxSrvr < (g_nWxSrvrCnt-1)? CrntWxSrvr+1: 0;
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntWxSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntWxSrvr, g_pWxServers[CrntWxSrvr]);
		return ( g_pWxServers[CrntWxSrvr] );
	}

	CrntWxSrvr = CrntWxSrvr < (WX_SRVR_LIST_SZ-1)? CrntWxSrvr+1: 0;
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntWxSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntWxSrvr, WxSrvrs[CrntWxSrvr]);
	return ( WxSrvrs[CrntWxSrvr] );
}

char* GetNxtSurfSrvr(struct InstanceData* id) { 
	id->m_nDBServerIndex = id->m_nDBServerIndex ? 0 : 1;
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntSurfSrvr=%d, Returning: %s\n", GetCurrentThreadId(), id->m_nDBServerIndex, id->m_strHttpDBservername[id->m_nDBServerIndex]);
	return (id->m_strHttpDBservername[id->m_nDBServerIndex]);
}

void FreeServerList(char*** server, int nCount)
{
	int i = 0;
	for(i = 0; i < nCount; i++){
		free((*server)[i]);
		(*server)[i] = NULL;
	}
	free(*server);
	*server = NULL;
}

void FreeServersLists(struct InstanceData* id)
{
	FreeServerList(&g_pFTracksServers, g_nFTrackSrvrCnt);
	FreeServerList(&g_pTailFileServers, g_nTailFileSrvrCnt);
	FreeServerList(&g_pWxServers, g_nWxSrvrCnt);
	FreeServerList(&g_pPulseServers, g_nPulseSrvrCnt);
	FreeServerList(&g_pAsdexServers, g_nAsdexSrvrCnt);
	FreeServerList(&g_pAsdServers, g_nAsdSrvrCnt);
	FreeServerList(&g_pAdsbServers, g_nAdsbSrvrCnt);
	FreeServerList(&g_pAirAsiaServers, g_nAirAsiaSrvrCnt);
	FreeServerList(&g_pMlatServers, g_nMlatSrvrCnt);
	FreeServerList(&g_pPassurServers, g_nPassurSrvrCnt);
	FreeServerList(&g_pInsightServers, g_nInsightSrvrCnt);
	FreeServerList(&g_pNoiseServers, g_nNoiseSrvrCnt);	
}

int BuildServerList(char*** server, struct json_object* jarr){
	int i = 0; int cnt = 0;
	if(json_type_array == json_object_get_type(jarr)){
		cnt = json_object_array_length(jarr);
		if(cnt > 0){
			*server = (char**)calloc(cnt, sizeof(char*));
			for(i = 0; i < cnt; i++){
				struct json_object* jelem = json_object_array_get_idx(jarr, i);
				(*server)[i] =  (char*)calloc(SERVER_PATH_SZ, sizeof(char));
				strcpy_s((*server)[i],SERVER_PATH_SZ,json_object_get_string(jelem));
			}
		}
	}
	return ( cnt );
}

int ProcessJSONServerLists(struct InstanceData* id, struct json_object* jobj) {
	struct json_object_iter jiter;
	const DWORD cdwThreadId = GetCurrentThreadId();
	json_object_object_foreachC(jobj, jiter){
		if(!strcmp(jiter.key, "_class")){
			// make sure not error
			if(!strcmp("Error", json_object_get_string(jiter.val))){
				struct json_object* jerr = json_object_object_get(jobj, "description");
				if(jerr){
					vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Error loading Surface Server Lists for user \'%s\', \'%s\'.\n", 
						cdwThreadId, id->m_strUserName, json_object_get_string(jerr));
				}else{
					vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Error loading Surface Server Lists for user \'%s\'.\n", 
						cdwThreadId, id->m_strUserName);
				}
				return (FALSE);
			}
		}else if(!strcmp(jiter.key, "FTracks")){
			g_nFTrackSrvrCnt = BuildServerList(&g_pFTracksServers, jiter.val);
		}else if(!strcmp(jiter.key, "Tail")){
			g_nTailFileSrvrCnt = BuildServerList(&g_pTailFileServers, jiter.val);
		}else if(!strcmp(jiter.key, "Wx")){
			g_nWxSrvrCnt = BuildServerList(&g_pWxServers, jiter.val);
		}else if(!strcmp(jiter.key, "Pulse")){
			g_nPulseSrvrCnt = BuildServerList(&g_pPulseServers, jiter.val);
		}else if(!strcmp(jiter.key, "Asdex")){
			g_nAsdexSrvrCnt = BuildServerList(&g_pAsdexServers, jiter.val);
		}else if(!strcmp(jiter.key, "Asd")){
			g_nAsdSrvrCnt = BuildServerList(&g_pAsdServers, jiter.val);
		}else if(!strcmp(jiter.key, "Adsb")){
			g_nAdsbSrvrCnt = BuildServerList(&g_pAdsbServers, jiter.val);
		}else if(!strcmp(jiter.key, "AirAsia")){
			g_nAirAsiaSrvrCnt = BuildServerList(&g_pAirAsiaServers, jiter.val);
		}else if(!strcmp(jiter.key, "Mlat")){
			g_nMlatSrvrCnt = BuildServerList(&g_pMlatServers, jiter.val);
		}else if(!strcmp(jiter.key, "Passur")){
			g_nPassurSrvrCnt = BuildServerList(&g_pPassurServers, jiter.val);
		}else if(!strcmp(jiter.key, "Noise")){
			g_nNoiseSrvrCnt = BuildServerList(&g_pNoiseServers, jiter.val);
		}else if(!strcmp(jiter.key, "Insight")){
			g_nInsightSrvrCnt = BuildServerList(&g_pInsightServers, jiter.val);
		}else
			vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Ignoring unused element <%s>\n", cdwThreadId, jiter.key);
	}
	return (TRUE);
}

void GetSurfServerLists(struct InstanceData* id) {
	enum json_tokener_error jerror;
	struct json_object* jobj = NULL;
	char tmpstr[8192] = {0};
	char strPath[MAX_PATH_SZ] = {0};
	struct MemoryStruct chunk;
	int nRetry = 0, nLen = 0, nPos = 0;
	int bSucceeded = FALSE;
	const char* cstrDefault = "-servers-";
	const DWORD cdwThreadId = GetCurrentThreadId();

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Getting Server List for user \'%s\'.\n", cdwThreadId, id->m_strUserName );
	_snprintf_s(tmpstr, _countof(tmpstr), _TRUNCATE, "Action=GetSurfServers&username=%s", id->m_strUserName );

	// Try to load data from db 3 times
	do{
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlFormGetResults - id->m_strHttpDBservername <%s> tmpstr <%s>.\n", 
			cdwThreadId, id->m_strHttpDBservername[id->m_nDBServerIndex], tmpstr );

		clear_chunk_memory(&chunk);
		if(!GetCurlFormGetResults( id, &chunk, id->m_strHttpDBservername[id->m_nDBServerIndex], tmpstr, SM_HTTP, TRUE )){
			vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to load Surface Server Lists for user \'%s\', attempt <%d>.\n", 
				cdwThreadId, id->m_strUserName, nRetry );
		}else{
			// Check for valid data from server
			if(chunk.memory){
				if(!strstr(chunk.memory, "404 Not Found")){
					char* data = curl_unescape(chunk.memory, strlen(chunk.memory));
					jobj = json_tokener_parse_verbose(data, &jerror);
					if(json_tokener_success == jerror){						
						if(ProcessJSONServerLists(id, jobj)){
							// cache as file in case we later fail.
							sprintf_s(strPath, MAX_PATH_SZ, "%s\\%s.json", id->m_strCachedDir, cstrDefault);
							if(json_object_to_file(strPath, jobj)){
								vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to save Layout \"%s\" to file \"%s\"!\n", cdwThreadId, cstrDefault, strPath);
							}						
							bSucceeded = TRUE;
						} else
							vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to load Surface Server Lists for user \'%s\', attempt <%d>.\n", 
								cdwThreadId, id->m_strUserName, nRetry );
						json_object_put(jobj);
					}else{
						vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to load Surface Server Lists for user \'%s\', attempt <%d>.\n", 
							cdwThreadId, id->m_strUserName, nRetry );
					}					
					curl_free(data);
				}else{
					vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to load Surface Server Lists for user \'%s\', attempt <%d>.\n", 
						cdwThreadId, id->m_strUserName, nRetry );
				}				
				free_chunk_memory( &chunk );
			}else{
				vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to load Surface Server Lists for user \'%s\', attempt <%d>.\n", 
					cdwThreadId, id->m_strUserName, nRetry );
			}
		}
	}while(!bSucceeded && (++nRetry < 3));

	if(!bSucceeded){
		// Try from cache
		vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Trying to load Surface Server Lists for user \'%s\' from local cache.\n", 
							cdwThreadId, id->m_strUserName );
		sprintf_s(strPath, MAX_PATH_SZ, "%s\\%s.json", id->m_strCachedDir, cstrDefault);
		jobj = json_object_from_file(strPath);
		if(jobj && -1 != ((int)jobj)){
			vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Using Surface Server Lists for user \'%s\' from local cache.\n", 
							cdwThreadId, id->m_strUserName );
			ProcessJSONServerLists(id, jobj);
			json_object_put(jobj);
		}
	}
}

char *GetAsdexSrvr(struct InstanceData* id) 
{
	if( g_nAsdexSrvrCnt ){
		D_CONTROL("SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Returning: %s\n", GetCurrentThreadId(), g_pAsdexServers[CrntAsdexSrvr]));
		return ( g_pAsdexServers[CrntAsdexSrvr] );
	}

	D_CONTROL("SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Returning: %s\n", GetCurrentThreadId(), AsdexSrvrs[CrntAsdexSrvr]));
	return ( AsdexSrvrs[CrntAsdexSrvr] );
}

char* GetNxtAsdexSrvr(struct InstanceData* id)
{
	if( g_nAsdexSrvrCnt ){
		CrntAsdexSrvr = CrntAsdexSrvr < (g_nAsdexSrvrCnt-1)? CrntAsdexSrvr+1: 0;
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntAsdexSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntAsdexSrvr, g_pAsdexServers[CrntAsdexSrvr]);
		return ( g_pAsdexServers[CrntAsdexSrvr] );
	}
	CrntAsdexSrvr = CrntAsdexSrvr < (ASDEX_SRVR_LIST_SZ-1)? CrntAsdexSrvr+1: 0;
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntAsdexSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntAsdexSrvr, AsdexSrvrs[CrntAsdexSrvr]);
	return ( AsdexSrvrs[CrntAsdexSrvr] );
}

char *GetAsdSrvr(struct InstanceData* id) 
{
	if( g_nAsdSrvrCnt ){
		D_CONTROL("ASD_SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) (A) Returning: %s\n", GetCurrentThreadId(), g_pAsdServers[CrntAsdSrvr]));
		return ( g_pAsdServers[CrntAsdSrvr] );
	}

	D_CONTROL("ASD_SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) (B) Returning: %s\n", GetCurrentThreadId(), AsdSrvrs[CrntAsdSrvr]));
	return ( AsdSrvrs[CrntAsdSrvr] );
}

char* GetNxtAsdSrvr(struct InstanceData* id)
{
	if( g_nAsdSrvrCnt ){
		CrntAsdSrvr = CrntAsdSrvr < (g_nAsdSrvrCnt-1)? CrntAsdSrvr+1: 0;
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntAsdSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntAsdSrvr, g_pAsdServers[CrntAsdSrvr]);
		return ( g_pAsdServers[CrntAsdSrvr] );
	}
	CrntAsdSrvr = CrntAsdSrvr < (ASD_SRVR_LIST_SZ-1)? CrntAsdSrvr+1: 0;
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntAsdSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntAsdSrvr, AsdSrvrs[CrntAsdSrvr]);
	return ( AsdSrvrs[CrntAsdSrvr] );
}


char *GetNoiseSrvr(struct InstanceData* id) 
{
	if( g_nNoiseSrvrCnt ){
		D_CONTROL("SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Returning: %s\n", GetCurrentThreadId(), g_pNoiseServers[CrntNoiseSrvr]));
		return ( g_pNoiseServers[CrntNoiseSrvr] );
	}

	D_CONTROL("SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Returning: %s\n", GetCurrentThreadId(), AsdexSrvrs[CrntNoiseSrvr]));
	return ( NoiseSrvrs[CrntNoiseSrvr] );
}

char* GetNxtNoiseSrvr(struct InstanceData* id)
{
	if( g_nNoiseSrvrCnt ){
		CrntNoiseSrvr = CrntNoiseSrvr < (g_nNoiseSrvrCnt-1)? CrntNoiseSrvr+1: 0;
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntNoiseSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntNoiseSrvr, g_pNoiseServers[CrntNoiseSrvr]);
		return ( g_pNoiseServers[CrntNoiseSrvr] );
	}
	CrntNoiseSrvr = CrntNoiseSrvr < (NOISE_SRVR_LIST_SZ-1)? CrntNoiseSrvr+1: 0;
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntNoiseSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntNoiseSrvr, NoiseSrvrs[CrntNoiseSrvr]);
	return ( NoiseSrvrs[CrntNoiseSrvr] );
}


void NoteAsdexServerSuccess(struct InstanceData* id) {
	time_t now = time(NULL);
	g_tAsdexSvrFailTime = now;
	g_nAsdexSvrFailCnt = 0;
	if(!id->m_bClass1)
		Callback2Javascript(id, "AsdexStatus:Slow");
	else
		Callback2Javascript(id, "AsdexStatus:OK");
}

void NoteAsdexServerFailure(struct InstanceData* id) {
	const DWORD cdwThreadId = GetCurrentThreadId();
	time_t now = time(NULL);
	int nTimeOut = GetConfigValueInt(id, "SERVER_TIMEOUT", 30);
	int nCount = GetConfigValueInt(id, "SERVER_TIMEOUT_COUNT", 3);

	g_nAsdexSvrFailCnt++;
	
	if( !g_tAsdexSvrFailTime )
		g_tAsdexSvrFailTime = now;

	if( g_nAsdexSvrFailCnt >= nCount || (now - g_tAsdexSvrFailTime) > nTimeOut){
		char buffer[STATUSSTRING_SZ] = {0};
		char *srvr1 = GetAsdexSrvr(id);
		char *srvr2 = GetNxtAsdexSrvr(id);
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) ASDEX Server Failures=%d, switched server from %s to %s\n", cdwThreadId, g_nAsdexSvrFailCnt, srvr1, srvr2);
		
		_snprintf_s(buffer, STATUSSTRING_SZ, _TRUNCATE, "Asdex Server %s failed %d times, last success or server change was %d secs ago, switching to server %s.", 
			srvr1, g_nAsdexSvrFailCnt, (now - g_tAsdexSvrFailTime), srvr2);
		LogToServer(id, LOG_ERR, "NETWORK", buffer);
		Callback2Javascript(id, "AsdexStatus:FAIL");
		g_tAsdexSvrFailTime = now;
		g_nAsdexSvrFailCnt = 0;
	}
}

void NoteAsdServerSuccess(struct InstanceData* id) {
	time_t now = time(NULL);
	g_tAsdSvrFailTime = now;
	g_nAsdSvrFailCnt = 0;
	if(!id->m_bClass1)
		Callback2Javascript(id, "AsdiStatus:Slow");
	else
		Callback2Javascript(id, "AsdiStatus:OK");
}

void NoteAsdServerFailure(struct InstanceData* id) {
	const DWORD cdwThreadId = GetCurrentThreadId();
	time_t now = time(NULL);
	int nTimeOut = GetConfigValueInt(id, "SERVER_TIMEOUT", 30);
	int nCount = GetConfigValueInt(id, "SERVER_TIMEOUT_COUNT", 3);

	g_nAsdSvrFailCnt++;
	
	if( !g_tAsdSvrFailTime )
		g_tAsdSvrFailTime = now;

	if( g_nAsdSvrFailCnt >= nCount || (now - g_tAsdSvrFailTime) > nTimeOut){
		char buffer[STATUSSTRING_SZ] = {0};
		char *srvr1 = GetAsdSrvr(id);
		char *srvr2 = GetNxtAsdSrvr(id);
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) ASD Server Failures=%d, switched server from %s to %s\n", cdwThreadId, g_nAsdSvrFailCnt, srvr1, srvr2);
		
		_snprintf_s(buffer, STATUSSTRING_SZ, _TRUNCATE, "ASD Server %s failed %d times, last success or server change was %d secs ago, switching to server %s.", 
			srvr1, g_nAsdSvrFailCnt, (now - g_tAsdSvrFailTime), srvr2);
		LogToServer(id, LOG_ERR, "NETWORK", buffer);
		Callback2Javascript(id, "AsdiStatus:FAIL");
		g_tAsdSvrFailTime = now;
		g_nAsdSvrFailCnt = 0;
	}
}

void NotePassurServerSuccess(struct InstanceData* id) {
	time_t now = time(NULL);
	g_tAsdSvrFailTime = now;
	g_nAsdSvrFailCnt = 0;
	if(!id->m_bClass1)
		Callback2Javascript(id, "PassurStatus:Slow");
	else
		Callback2Javascript(id, "PassurStatus:OK");
}

void NotePassurServerFailure(struct InstanceData* id) {
	const DWORD cdwThreadId = GetCurrentThreadId();
	time_t now = time(NULL);
	int nTimeOut = GetConfigValueInt(id, "SERVER_TIMEOUT", 30);
	int nCount = GetConfigValueInt(id, "SERVER_TIMEOUT_COUNT", 3);

	g_nPassurSvrFailCnt++;
	
	if( !g_tPassurSvrFailTime )
		g_tPassurSvrFailTime = now;

	if( g_nPassurSvrFailCnt >= nCount || (now - g_tPassurSvrFailTime) > nTimeOut){
		char buffer[STATUSSTRING_SZ] = {0};
		char *srvr1 = GetPassurSrvr(id);
		char *srvr2 = GetNxtPassurSrvr(id);
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) PASSUR Server Failures=%d, switched server from %s to %s\n", cdwThreadId, g_nPassurSvrFailCnt, srvr1, srvr2);
		
		_snprintf_s(buffer, STATUSSTRING_SZ, _TRUNCATE, "PASSUR Server %s failed %d times, last success or server change was %d secs ago, switching to server %s.", 
			srvr1, g_nPassurSvrFailCnt, (now - g_tPassurSvrFailTime), srvr2);
		LogToServer(id, LOG_ERR, "NETWORK", buffer);
		Callback2Javascript(id, "PassurStatus:FAIL");
		g_tPassurSvrFailTime = now;
		g_nPassurSvrFailCnt = 0;
	}
}


void NoteAdsbServerSuccess(struct InstanceData* id) {
	time_t now = time(NULL);
	g_tAdsbSvrFailTime = now;
	g_nAdsbSvrFailCnt = 0;
	if(!id->m_bClass1)
		Callback2Javascript(id, "AdsbStatus:Slow");
	else
		Callback2Javascript(id, "AdsbStatus:OK");
}

void NoteAdsbServerFailure(struct InstanceData* id) {
	const DWORD cdwThreadId = GetCurrentThreadId();
	time_t now = time(NULL);
	int nTimeOut = GetConfigValueInt(id, "SERVER_TIMEOUT", 30);
	int nCount = GetConfigValueInt(id, "SERVER_TIMEOUT_COUNT", 3);

	g_nAdsbSvrFailCnt++;
	
	if( !g_tAdsbSvrFailTime )
		g_tAdsbSvrFailTime = now;

	if( g_nAdsbSvrFailCnt >= nCount || (now - g_tAdsbSvrFailTime) > nTimeOut){
		char buffer[STATUSSTRING_SZ] = {0};
		char *srvr1 = GetAdsbSrvr(id);
		char *srvr2 = GetNxtAdsbSrvr(id);
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) ADSB Server Failures=%d, switched server from %s to %s\n", cdwThreadId, g_nAdsbSvrFailCnt, srvr1, srvr2);
		
		_snprintf_s(buffer, STATUSSTRING_SZ, _TRUNCATE, "Adsb Server %s failed %d times, last success or server change was %d secs ago, switching to server %s.", 
			srvr1, g_nAdsbSvrFailCnt, (now - g_tAdsbSvrFailTime), srvr2);
		LogToServer(id, LOG_ERR, "NETWORK", buffer);
		Callback2Javascript(id, "AdsbStatus:FAIL");
		g_tAdsbSvrFailTime = now;
		g_nAdsbSvrFailCnt = 0;
	}
}


char *GetAdsbSrvr(struct InstanceData* id) 
{
	if( g_nAdsbSrvrCnt ){
		D_CONTROL("SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Returning: %s\n", GetCurrentThreadId(), g_pAdsbServers[CrntAdsbSrvr]));
		return ( g_pAdsbServers[CrntAdsbSrvr] );
	}

	D_CONTROL("SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Returning: %s\n", GetCurrentThreadId(), AdsbSrvrs[CrntAdsbSrvr]));
	return ( AdsbSrvrs[CrntAdsbSrvr] );
}

/*
** char * GetNxtAdsbSrvr()
**
**	Returns:	Next server URL in the list of servers
**				
**	Cuerrent ADSB server index is also set to next server (or first if
**  currently pointing to last server).
**
*/

char* GetNxtAdsbSrvr(struct InstanceData* id)
{
	time_t now = time(NULL);
	int nTimeOut = GetConfigValueInt(id, "SERVER_TIMEOUT", 300);
	int nCount = GetConfigValueInt(id, "SERVER_TIMEOUT_COUNT", 5);

	if(!g_tAdsbSvrFailTime || (now - g_tAdsbSvrFailTime) > nTimeOut){
		g_nAdsbSvrFailCnt = 1;
		g_tAdsbSvrFailTime = now;
	}else{
		++g_nAdsbSvrFailCnt;
		if(g_nAdsbSvrFailCnt > nCount){
			char buffer[STATUSSTRING_SZ] = {0};
			g_nAdsbSvrFailCnt = 1;
			_snprintf_s(buffer, STATUSSTRING_SZ, _TRUNCATE, "ADS-B Server failed 5 times in %d secs.", (now - g_tAdsbSvrFailTime));
			LogToServer(id, LOG_ERR, "NETWORK", buffer);
		}
	}
	if( g_nAdsbSrvrCnt ){
		CrntAdsbSrvr = CrntAdsbSrvr < (g_nAdsbSrvrCnt-1)? CrntAdsbSrvr+1: 0;
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntAdsbSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntAdsbSrvr, g_pAdsbServers[CrntAdsbSrvr]);
		return ( g_pAdsbServers[CrntAdsbSrvr] );
	}
	CrntAdsbSrvr = CrntAdsbSrvr < (ADSB_SRVR_LIST_SZ-1)? CrntAdsbSrvr+1: 0;
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntAdsbSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntAdsbSrvr, AdsbSrvrs[CrntAdsbSrvr]);
	return ( AdsbSrvrs[CrntAdsbSrvr] );
}

char *GetPassurSrvr(struct InstanceData* id) 
{
	if( g_nPassurSrvrCnt ){
		D_CONTROL("SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Returning: %s\n", GetCurrentThreadId(), g_pPassurServers[CrntPassurSrvr]));
		return ( g_pPassurServers[CrntPassurSrvr] );
	}

	D_CONTROL("SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Returning: %s\n", GetCurrentThreadId(), PassurSrvrs[CrntPassurSrvr]));
	return ( PassurSrvrs[CrntPassurSrvr] );
}

/*
** char * GetNxtPassurSrvr()
**
**	Returns:	Next server URL in the list of servers
**				
**	Cuerrent Passur server index is also set to next server (or first if
**  currently pointing to last server).
**
*/

char* GetNxtPassurSrvr(struct InstanceData* id)
{
	time_t now = time(NULL);
	int nTimeOut = GetConfigValueInt(id, "SERVER_TIMEOUT", 300);
	int nCount = GetConfigValueInt(id, "SERVER_TIMEOUT_COUNT", 5);

	if(!g_tPassurSvrFailTime || (now - g_tPassurSvrFailTime) > nTimeOut){
		g_nPassurSvrFailCnt = 1;
		g_tPassurSvrFailTime = now;
	}else{
		++g_nPassurSvrFailCnt;
		if(g_nPassurSvrFailCnt > nCount){
			char buffer[STATUSSTRING_SZ] = {0};
			g_nPassurSvrFailCnt = 1;
			_snprintf_s(buffer, STATUSSTRING_SZ, _TRUNCATE, "ADS-B Server failed 5 times in %d secs.", (now - g_tPassurSvrFailTime));
			LogToServer(id, LOG_ERR, "NETWORK", buffer);
		}
	}
	if( g_nPassurSrvrCnt ){
		CrntPassurSrvr = CrntPassurSrvr < (g_nPassurSrvrCnt-1)? CrntPassurSrvr+1: 0;
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntPassurSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntPassurSrvr, g_pPassurServers[CrntPassurSrvr]);
		return ( g_pPassurServers[CrntPassurSrvr] );
	}
	CrntPassurSrvr = CrntPassurSrvr < (PASSUR_SRVR_LIST_SZ-1)? CrntPassurSrvr+1: 0;
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntPassurSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntPassurSrvr, PassurSrvrs[CrntPassurSrvr]);
	return ( PassurSrvrs[CrntPassurSrvr] );
}


void NoteAirAsiaServerSuccess(struct InstanceData* id) {
	time_t now = time(NULL);
	g_tAirAsiaSvrFailTime = now;
	g_nAirAsiaSvrFailCnt = 0;
	Callback2Javascript(id, "AirAsiaStatus:OK");
}

void NoteAirAsiaServerFailure(struct InstanceData* id) {
	const DWORD cdwThreadId = GetCurrentThreadId();
	time_t now = time(NULL);
	int nTimeOut = GetConfigValueInt(id, "SERVER_TIMEOUT", 30);
	int nCount = GetConfigValueInt(id, "SERVER_TIMEOUT_COUNT", 3);

	g_nAirAsiaSvrFailCnt++;
	
	if( !g_tAirAsiaSvrFailTime )
		g_tAirAsiaSvrFailTime = now;

	if( g_nAirAsiaSvrFailCnt >= nCount || (now - g_tAirAsiaSvrFailTime) > nTimeOut){
		char buffer[STATUSSTRING_SZ] = {0};
		char *srvr1 = GetAirAsiaSrvr(id);
		char *srvr2 = GetNxtAirAsiaSrvr(id);
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) AirAsia Server Failures=%d, switched server from %s to %s\n", cdwThreadId, g_nAirAsiaSvrFailCnt, srvr1, srvr2);
		
		_snprintf_s(buffer, STATUSSTRING_SZ, _TRUNCATE, "AirAsia Server %s failed %d times, last success or server change was %d secs ago, switching to server %s.", 
			srvr1, g_nAirAsiaSvrFailCnt, (now - g_tAirAsiaSvrFailTime), srvr2);
		LogToServer(id, LOG_ERR, "NETWORK", buffer);
		Callback2Javascript(id, "AirAsiaStatus:FAIL");
		g_tAirAsiaSvrFailTime = now;
		g_nAirAsiaSvrFailCnt = 0;
	}
}


char *GetAirAsiaSrvr(struct InstanceData* id) 
{
	if( g_nAirAsiaSrvrCnt ){
		D_CONTROL("SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Returning: %s\n", GetCurrentThreadId(), g_pAirAsiaServers[CrntAirAsiaSrvr]));
		return ( g_pAirAsiaServers[CrntAirAsiaSrvr] );
	}

	D_CONTROL("SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Returning: %s\n", GetCurrentThreadId(), AirAsiaSrvrs[CrntAirAsiaSrvr]));
	return ( AirAsiaSrvrs[CrntAirAsiaSrvr] );
}

char* GetNxtAirAsiaSrvr(struct InstanceData* id)
{
	time_t now = time(NULL);
	int nTimeOut = GetConfigValueInt(id, "SERVER_TIMEOUT", 300);
	int nCount = GetConfigValueInt(id, "SERVER_TIMEOUT_COUNT", 5);

	if(!g_tAirAsiaSvrFailTime || (now - g_tAirAsiaSvrFailTime) > nTimeOut){
		g_nAirAsiaSvrFailCnt = 1;
		g_tAirAsiaSvrFailTime = now;
	}else{
		++g_nAirAsiaSvrFailCnt;
		if(g_nAirAsiaSvrFailCnt > nCount){
			char buffer[STATUSSTRING_SZ] = {0};
			g_nAirAsiaSvrFailCnt = 1;
			_snprintf_s(buffer, STATUSSTRING_SZ, _TRUNCATE, "AirAsia Server failed 5 times in %d secs.", (now - g_tAirAsiaSvrFailTime));
			LogToServer(id, LOG_ERR, "NETWORK", buffer);
		}
	}
	if( g_nAirAsiaSrvrCnt ){
		CrntAirAsiaSrvr = CrntAirAsiaSrvr < (g_nAirAsiaSrvrCnt-1)? CrntAirAsiaSrvr+1: 0;
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntAirAsiaSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntAirAsiaSrvr, g_pAirAsiaServers[CrntAirAsiaSrvr]);
		return ( g_pAirAsiaServers[CrntAirAsiaSrvr] );
	}
	CrntAirAsiaSrvr = CrntAirAsiaSrvr < (AIRASIA_SRVR_LIST_SZ-1)? CrntAirAsiaSrvr+1: 0;
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntAirAsiaSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntAirAsiaSrvr, AirAsiaSrvrs[CrntAirAsiaSrvr]);
	return ( AirAsiaSrvrs[CrntAirAsiaSrvr] );
}

void NoteMlatServerSuccess(struct InstanceData* id) {
	time_t now = time(NULL);
	g_tMlatSvrFailTime = now;
	g_nMlatSvrFailCnt = 0;
	if(!id->m_bClass1)
		Callback2Javascript(id, "MlatStatus:Slow");
	else
		Callback2Javascript(id, "MlatStatus:OK");
}

void NoteMlatServerFailure(struct InstanceData* id) {
	const DWORD cdwThreadId = GetCurrentThreadId();
	time_t now = time(NULL);
	int nTimeOut = GetConfigValueInt(id, "SERVER_TIMEOUT", 30);
	int nCount = GetConfigValueInt(id, "SERVER_TIMEOUT_COUNT", 3);

	g_nMlatSvrFailCnt++;
	
	if( !g_tMlatSvrFailTime )
		g_tMlatSvrFailTime = now;

	if( g_nMlatSvrFailCnt >= nCount || (now - g_tMlatSvrFailTime) > nTimeOut){
		char buffer[STATUSSTRING_SZ] = {0};
		char *srvr1 = GetMlatSrvr(id);
		char *srvr2 = GetNxtMlatSrvr(id);
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X)  MLAT Server Failures=%d, switched server from %s to %s\n", cdwThreadId, g_nMlatSvrFailCnt, srvr1, srvr2);
		
		_snprintf_s(buffer, STATUSSTRING_SZ, _TRUNCATE, "MLAT Server %s failed %d times, last success or server change was %d secs ago, switching to server %s.", 
			srvr1, g_nMlatSvrFailCnt, (now - g_tMlatSvrFailTime), srvr2);
		LogToServer(id, LOG_ERR, "NETWORK", buffer);
		//Callback2Javascript(id, "AdsbStatus:FAIL");
		g_tMlatSvrFailTime = now;
		g_nMlatSvrFailCnt = 0;
	}
}


char *GetMlatSrvr(struct InstanceData* id) 
{
	if( g_nMlatSrvrCnt ){
		D_CONTROL("SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Returning: %s\n", GetCurrentThreadId(), g_pMlatServers[CrntMlatSrvr]));
		return ( g_pMlatServers[CrntMlatSrvr] );
	}

	D_CONTROL("SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Returning: %s\n", GetCurrentThreadId(), MlatSrvrs[CrntMlatSrvr]));
	return ( MlatSrvrs[CrntMlatSrvr] );
}

/*
** char * GetNxtMlatSrvr()
**
**	Returns:	Next server URL in the list of servers
**				
**	Cuerrent MLAT server index is also set to next server (or first if
**  currently pointing to last server).
**
*/

char* GetNxtMlatSrvr(struct InstanceData* id)
{
	time_t now = time(NULL);
	int nTimeOut = GetConfigValueInt(id, "SERVER_TIMEOUT", 300);
	int nCount = GetConfigValueInt(id, "SERVER_TIMEOUT_COUNT", 5);

	if(!g_tMlatSvrFailTime || (now - g_tMlatSvrFailTime) > nTimeOut){
		g_nMlatSvrFailCnt = 1;
		g_tMlatSvrFailTime = now;
	}else{
		++g_nMlatSvrFailCnt;
		if(g_nMlatSvrFailCnt > nCount){
			char buffer[STATUSSTRING_SZ] = {0};
			g_nMlatSvrFailCnt = 1;
			_snprintf_s(buffer, STATUSSTRING_SZ, _TRUNCATE, "MLAT Server failed 5 times in %d secs.", (now - g_tMlatSvrFailTime));
			LogToServer(id, LOG_ERR, "NETWORK", buffer);
		}
	}
	if( g_nMlatSrvrCnt ){
		CrntMlatSrvr = CrntMlatSrvr < (g_nMlatSrvrCnt-1)? CrntMlatSrvr+1: 0;
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntMlatSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntMlatSrvr, g_pMlatServers[CrntMlatSrvr]);
		return ( g_pMlatServers[CrntMlatSrvr] );
	}
	CrntMlatSrvr = CrntMlatSrvr < (MLAT_SRVR_LIST_SZ-1)? CrntMlatSrvr+1: 0;
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntMlatSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntMlatSrvr, MlatSrvrs[CrntMlatSrvr]);
	return ( MlatSrvrs[CrntMlatSrvr] );
}

void NoteLMGServerSuccess(struct InstanceData* id) {
	time_t now = time(NULL);
	g_tLMGSvrFailTime = now;
	g_nLMGSvrFailCnt = 0;
	//if(!id->m_bClass1)
	//	Callback2Javascript(id, "LMGStatus:Slow");
	//else
	//	Callback2Javascript(id, "LMGStatus:OK");
}

void NoteLMGServerFailure(struct InstanceData* id) {
	const DWORD cdwThreadId = GetCurrentThreadId();
	time_t now = time(NULL);
	int nTimeOut = GetConfigValueInt(id, "SERVER_TIMEOUT", 30);
	int nCount = GetConfigValueInt(id, "SERVER_TIMEOUT_COUNT", 3);

	g_nLMGSvrFailCnt++;
	
	if( !g_tLMGSvrFailTime )
		g_tLMGSvrFailTime = now;

	if( g_nLMGSvrFailCnt >= nCount || (now - g_tLMGSvrFailTime) > nTimeOut){
		char buffer[STATUSSTRING_SZ] = {0};
		char *srvr1 = GetLMGSrvr(id);
		char *srvr2 = GetNxtLMGSrvr(id);
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X)  LMG Server Failures=%d, switched server from %s to %s\n", cdwThreadId, g_nLMGSvrFailCnt, srvr1, srvr2);
		
		_snprintf_s(buffer, STATUSSTRING_SZ, _TRUNCATE, "LMG Server %s failed %d times, last success or server change was %d secs ago, switching to server %s.", 
			srvr1, g_nLMGSvrFailCnt, (now - g_tLMGSvrFailTime), srvr2);
		LogToServer(id, LOG_ERR, "NETWORK", buffer);
		//Callback2Javascript(id, "AdsbStatus:FAIL");
		g_tLMGSvrFailTime = now;
		g_nLMGSvrFailCnt = 0;
	}
}


char *GetLMGSrvr(struct InstanceData* id) 
{
	if( g_nLMGSrvrCnt ){
		D_CONTROL("SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Returning: %s\n", GetCurrentThreadId(), g_pLMGServers[CrntLMGSrvr]));
		return ( g_pLMGServers[CrntLMGSrvr] );
	}

	D_CONTROL("SERVER_NAMES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Returning: %s\n", GetCurrentThreadId(), LMGSrvrs[CrntLMGSrvr]));
	return ( LMGSrvrs[CrntLMGSrvr] );
}

/*
** char * GetNxtLMGSrvr()
**
**	Returns:	Next server URL in the list of servers
**				
**	Cuerrent LMG server index is also set to next server (or first if
**  currently pointing to last server).
**
*/

char* GetNxtLMGSrvr(struct InstanceData* id)
{
	time_t now = time(NULL);
	int nTimeOut = GetConfigValueInt(id, "SERVER_TIMEOUT", 300);
	int nCount = GetConfigValueInt(id, "SERVER_TIMEOUT_COUNT", 5);

	if(!g_tLMGSvrFailTime || (now - g_tLMGSvrFailTime) > nTimeOut){
		g_nLMGSvrFailCnt = 1;
		g_tLMGSvrFailTime = now;
	}else{
		++g_nLMGSvrFailCnt;
		if(g_nLMGSvrFailCnt > nCount){
			char buffer[STATUSSTRING_SZ] = {0};
			g_nLMGSvrFailCnt = 1;
			_snprintf_s(buffer, STATUSSTRING_SZ, _TRUNCATE, "LMG Server failed 5 times in %d secs.", (now - g_tLMGSvrFailTime));
			LogToServer(id, LOG_ERR, "NETWORK", buffer);
		}
	}
	if( g_nLMGSrvrCnt ){
		CrntLMGSrvr = CrntLMGSrvr < (g_nLMGSrvrCnt-1)? CrntLMGSrvr+1: 0;
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntLMGSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntLMGSrvr, g_pLMGServers[CrntLMGSrvr]);
		return ( g_pLMGServers[CrntLMGSrvr] );
	}
	CrntLMGSrvr = CrntLMGSrvr < (LMG_SRVR_LIST_SZ-1)? CrntLMGSrvr+1: 0;
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Set CrntLMGSrvr=%d, Returning: %s\n", GetCurrentThreadId(), CrntLMGSrvr, LMGSrvrs[CrntLMGSrvr]);
	return ( LMGSrvrs[CrntLMGSrvr] );
}
