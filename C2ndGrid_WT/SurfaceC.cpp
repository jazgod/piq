// Surface.cpp : Implementation of CSurface
#include "stdafx.h"

#include <alc.h>
#include <al.h>

#include "json.h"
#include "InstanceData.h"
#include "SurfaceC.h"
#include "CWaves.h"
#include "resource.h"
#include "bitmapfont.h"
#include "ARTCC.h"
#include "Routes.h"
#include "regions.h"
#include "Sounds.h"
#include "AcTypes.h"
#include <io.h>
#include <stdarg.h>
#include <string>
#include <math.h>

#ifndef PI
#define PI           3.14159265358979323846	/* PI number */
#endif

#define CURSOR_ARROW	0
#define CURSOR_WAIT		1
#define CURSOR_HAND		2

#define WM_APPLOG	(WM_USER + 2)

struct LOG_MSG{	
	std::string m_strAppName;
	std::string m_strVersion;
	int m_nInstance;
	std::string m_strOS;
	std::string m_strBrowser;
	std::string m_strUsername;
	std::string m_strSessionId;
	std::string m_strLevel;
	std::string m_strCategory;
	std::string m_strLogTime;
	std::string m_strLogData;
};

extern "C"{
#include "vo.h"
#include "mgl.h"
#include "Curl.h"
#include "srfc.h"
#include "pulseserver.h"
#include "shares.h"
}


#ifdef DEBUG_TRACING
	#define CTRACE_DBG(f, fn, msg) \
		{ vo_log_info(f, fn, "(0x%X):[%d]-%s\n", GetCurrentThreadId(), id->m_nControlId, msg); }
#else
	#define CTRACE_DBG(f, fn, msg) \
	{ }
#endif//DEBUG_TRACING

// deprecated int g_nMode; //@@ Need to Review, used in windows api callback

// C Exposed Functions 
extern "C"{
	// This File Prototypes
	int InitAL(struct InstanceData* id);
	void SetStatusText(struct InstanceData* id, const char* strText);

	// externs from dbf.c
	int LoadInternationalArpts( struct InstanceData* id );

	// DataThread.c
	time_t CalcFileTime( struct InstanceData* id, char *url );

	// ctracks.c prototypes
	void buttonmotion(struct InstanceData* id, int x, int y);
	void motion(InstanceData* id, int x, int y);

	// srfc.c prototypes
	int ColorsSetup(struct InstanceData* id);
	int DefCreateFont(struct InstanceData* id);
	void InitActiveXCallbacks(struct InstanceData* id);
	void InitRollGrids( struct InstanceData* id );
	int IsGuiMouseEvent(struct InstanceData* id, int b, int m, int x, int y, int s);
	void LoadAirportRingData( struct InstanceData* id );
	void LoadAirways( struct InstanceData* id );
	void LoadBinaryTriangles( struct InstanceData* id );
	void LoadBorders(struct InstanceData* id);
	int LoadGLTextures(struct InstanceData* id);
	void LoadHiResMap(struct InstanceData* id);
	void LoadLakes( struct InstanceData* id );
	void LoadROI(struct InstanceData* id);
	void LoadStars( struct InstanceData* id );
	void myRmouse(struct InstanceData* id, int b, int m, int x, int y);
	void mymouse(struct InstanceData* id, int b, int m, int x, int y);
	void PerformActions(struct InstanceData* id);
	int ProcessKeydown( struct InstanceData* id, WPARAM wParam , int keystate);
	void RenderScene(struct InstanceData* id);
	int scroll_thumb_cb(struct InstanceData* id, int x, int y, struct guistruct* g );
	int ZoomIn(struct InstanceData* id, POINT* pt);
	int ZoomOut(struct InstanceData* id);
	void LoadAirlineCodes(struct InstanceData* id);

	extern int UseTriangleList;

	// externs from nexrad.c
	int LoadWxTex(struct InstanceData* id);

	// externs from server.c
	char *GetAsdSrvr(struct InstanceData* id);
	char* GetNxtSurfSrvr(struct InstanceData* id);

	void CalcTimeAdjustment(struct InstanceData* id);
	
	void RegisterThreadHandle(struct InstanceData* id, uintptr_t hThread, const char* strFuncName)
	{
		id->OuterRegisterThreadHandle(id, hThread, strFuncName);
	}
	void UnregisterThreadHandle(struct InstanceData* id, uintptr_t hThread)
	{
		id->OuterUnregisterThreadHandle(id, hThread);
	}
	int GetConfigValueInt(struct InstanceData* id, const char* strName, int nDefault)
	{
		char strBuffer[100] = {0};
		int nResult = nDefault;
		if(GetConfigValue(id, strName, strBuffer, 100)){
			nResult = atoi(strBuffer);
		}
		return nResult;
	}
	int GetConfigValue(struct InstanceData* id, const char* strName, char* value, int len)
	{
		return id->OuterGetConfigValue(id, strName, value, len);
	}
	int GetJavascriptValue(struct InstanceData* id, const char* name, char* value, int len)
	{
		return id->OuterGetJavascriptValue(id, name, value, len);
	}
	int IsDebugNameDefined(struct InstanceData* id, const char* strDebugName)
	{
		return id->OuterIsDebugNameDefined(id, strDebugName);
	}
	int IsMapSupported(struct InstanceData* id, const char* strMapName)
	{
		return id->OuterIsMapSupported(id, strMapName);
	}
	void Callback2Javascript(struct InstanceData* id, char* s)
	{
		int bSend = TRUE;
		const char* cAsdiStatus = "AsdiStatus";
		const char* cAsdexStatus = "AsdexStatus";
		const char* cAirAsiaStatus = "AirAsiaStatus";
		const char* cAdsbStatus = "AdsbStatus";
		const char* cInternetStatus = "InternetStatus";
		const char* cPassurStatus = "PassurStatus";
		const char* cMlatStatus = "MlatStatus";
		// Attempt to limit the number of callbacks if the status does not change
		if(!strncmp(s, cAsdexStatus, strlen(cAsdexStatus))){
			if(strcmp(id->m_strAsdexStatus, s)){
				if(id->m_bInitialized) 
					strncpy_s(id->m_strAsdexStatus, 100, s, _TRUNCATE);
			}else{
				// Status is the same, do not resend
				bSend = FALSE;
			}
		}else if(!strncmp(s, cAdsbStatus, strlen(cAdsbStatus))){
			if(strcmp(id->m_strAdsbStatus, s)){
				if(id->m_bInitialized) 
					strncpy_s(id->m_strAdsbStatus, 100, s, _TRUNCATE);
			}else{
				// Status is the same, do not resend
				bSend = FALSE;
			}
		}else if(!strncmp(s, cAirAsiaStatus, strlen(cAirAsiaStatus))){
			if(strcmp(id->m_strAirAsiaStatus, s)){
				if(id->m_bInitialized) 
					strncpy_s(id->m_strAirAsiaStatus, 100, s, _TRUNCATE);
			}else{
				// Status is the same, do not resend
				bSend = FALSE;
			}
		} else if (!strncmp(s, cPassurStatus, strlen(cPassurStatus))) {
			if (strcmp(id->m_strPassurStatus, s)) {
				if (id->m_bInitialized)
					strncpy_s(id->m_strPassurStatus, 100, s, _TRUNCATE);
			} else {
				// Status is the same, do not resend
				bSend = FALSE;
			}
		} else if (!strncmp(s, cAsdiStatus, strlen(cAsdiStatus))) {
			if (strcmp(id->m_strAsdiStatus, s)) {
				if (id->m_bInitialized)
					strncpy_s(id->m_strAsdiStatus, 100, s, _TRUNCATE);
			} else {
				// Status is the same, do not resend
				bSend = FALSE;
			}
		} else if (!strncmp(s, cMlatStatus, strlen(cMlatStatus))) {
			if (strcmp(id->m_strMlatStatus, s)) {
				if (id->m_bInitialized)
					strncpy_s(id->m_strMlatStatus, 100, s, _TRUNCATE);
			} else {
				// Status is the same, do not resend
				bSend = FALSE;
			}
		} else if (!strncmp(s, cInternetStatus, strlen(cInternetStatus))) {
			if(strcmp(id->m_strInternetStatus, s)){
				if(id->m_bInitialized) 
					strncpy_s(id->m_strInternetStatus, 100, s, _TRUNCATE);
			}else{
				// Status is the same, do not resend
				bSend = FALSE;
			}
		}
		if(bSend)
			id->OuterCallback2Javascript(id, s);
	}
	int GetLocalFile( struct InstanceData* id, const char* localpath, const char* filename, const char* remotepath, int eMode)
	{
		return id->OuterGetLocalFile( id, localpath, filename, remotepath, eMode);
	}
	void RegisterGetDataCallback(struct InstanceData* id, char* szType, CnGetDataCb pfn)
	{
		id->OuterRegisterGetDataCallback(szType, pfn);
	}
	void RegisterSendMessageCallback(struct InstanceData* id, char* p, CnSendMessageCb pfn)
	{
		id->OuterRegisterSendMessageCallback(p, pfn);
	}

	// Use local LogToServer, have more control of modification
	//void LogToServer(struct InstanceData* id, LOG_LEVEL eLevel, const char* strCategory, const char* strLogData)
	//{
	//	id->OuterLogToServer(id, eLevel, strCategory, strLogData);
	//}


unsigned _stdcall AppLogThread2(PVOID pvoid)
{
	static int s_nLogErrorCount = 0;
	struct InstanceData* id = (struct InstanceData*)pvoid;
	const DWORD cdwThreadId = GetCurrentThreadId();
	int nControlId = id->m_nControlId;
	MSG uMsg;
	DWORD dwResult = WAIT_TIMEOUT;
	HANDLE hEnd = id->m_evAppLogEnd;
	char strServerName[132] = {0};
	strcpy_s(strServerName, 132, id->m_strHttpDBservername[id->m_nDBServerIndex]);
	id->m_dwAppLogId = cdwThreadId;

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X):[%d] Thread Started\n", cdwThreadId, nControlId);

	// Create the message loop
	PeekMessage(&uMsg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	SetEvent(id->m_evAppLogStart);

	do
	{
		dwResult = WaitForSingleObject(hEnd, 1000);
		while(PeekMessage(&uMsg, NULL, 0,0, PM_REMOVE)){
			if(WM_APPLOG == uMsg.message){
				struct LOG_MSG* pLogMsg = (struct LOG_MSG*)uMsg.lParam;
				char tmpstr[8192] = {0};
				char* logdata = curl_escape(pLogMsg->m_strLogData.c_str(), pLogMsg->m_strLogData.length());
				char* os = curl_escape(pLogMsg->m_strOS.c_str(), pLogMsg->m_strOS.length());
				char* browser = curl_escape(pLogMsg->m_strBrowser.c_str(), pLogMsg->m_strBrowser.length());
				int nRetries = 1;
				int bSuccess = FALSE;				
				struct MemoryStruct m;				
				clear_chunk_memory(&m);

				_snprintf_s(tmpstr, _countof( tmpstr ), _TRUNCATE, "Action=AppLog&appname=%s&version=%s&instance=%d&os=%s&browser=%s&username=%s&sessionid=%s&level=%s&category=%s&logtime=%s&logdata=%s", 
					pLogMsg->m_strAppName.c_str(), pLogMsg->m_strVersion.c_str(), pLogMsg->m_nInstance, os, browser, pLogMsg->m_strUsername.c_str(), pLogMsg->m_strSessionId.c_str(),   
					pLogMsg->m_strLevel.c_str(), pLogMsg->m_strCategory.c_str(), pLogMsg->m_strLogTime.c_str(), logdata);

				curl_free(browser);
				curl_free(os);
				curl_free(logdata);

				do
				{
					if(!GetCurlFormPostResults( id, &m, strServerName, tmpstr, SM_HTTPS, TRUE )){
						if(s_nLogErrorCount < 3){
							vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to send application log entry to server, attempt #%d.\n", cdwThreadId, nRetries);
						}
					}					

					if(m.memory){
						if(!strncmp("failed", m.memory, 6)){
							if(s_nLogErrorCount < 3){
								vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to send application log entry to server, attempt #%d <%s>.\n", 
									cdwThreadId, nRetries, m.memory);
							}
						}else{
							
							bSuccess = TRUE;
							if(s_nLogErrorCount > 3){
								vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Communication to LogServer restored, remote logging re-enabled.\n", cdwThreadId);
							}
							s_nLogErrorCount = 0;
							vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Logged <%s:%s> to server.\n", cdwThreadId, pLogMsg->m_strCategory.c_str(), pLogMsg->m_strLogData.c_str());
						}
						free_chunk_memory(&m);
					}
				}while( !bSuccess && (++nRetries < 3) );
				if(!bSuccess){
					++s_nLogErrorCount;
					if(s_nLogErrorCount < 3){
						vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to send application log entry <%s:%s> to server.\n", cdwThreadId,
							pLogMsg->m_strCategory.c_str(), pLogMsg->m_strLogData.c_str());
					}else if(s_nLogErrorCount == 3){
						vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Communication to LogServer is down, remote logging errors temporarily disabled.\n", cdwThreadId);
					}
				}
				delete pLogMsg;			
			}
		}
	}while(WAIT_TIMEOUT == dwResult);
	
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X):[%d] Thread Terminated\n", cdwThreadId, nControlId);	_endthreadex(0);
	return 0;
}


void LogToServer(struct InstanceData* id, LOG_LEVEL eLevel, const char* strCategory, const char* strLogData)
{
	const DWORD cdwThreadId = GetCurrentThreadId();
	if(!id->m_hThreadAppLog){
		id->m_evAppLogStart = CreateEvent(NULL, TRUE, FALSE, NULL);
		id->m_evAppLogEnd = CreateEvent(NULL, TRUE, FALSE, NULL);
		id->m_hThreadAppLog = _beginthreadex(NULL, 0, AppLogThread2, id, 0, NULL);		
		WaitForSingleObject(id->m_evAppLogStart, INFINITE);
		CloseHandle(id->m_evAppLogStart);
		id->m_evAppLogStart = NULL;
	}

	if(id->m_dwAppLogId){
		struct LOG_MSG* pLogMsg = new struct LOG_MSG;
		char strTime[50] = {0};
		time_t now = time(NULL);
		ctime_s(strTime, 50, &now);
		pLogMsg->m_strAppName = "WebTracker";
		pLogMsg->m_strVersion = std::string(id->m_strSurfaceVersion) + ":" + std::string(id->m_strWebTrackerVersion) + "-v" + std::string(version) + " " + std::string(builddate);
		pLogMsg->m_nInstance = id->m_nControlId;
		pLogMsg->m_strOS = id->m_strOSVersion;
		pLogMsg->m_strBrowser = id->m_strBrowserVersion;
		pLogMsg->m_strUsername = id->m_strUserName;
		pLogMsg->m_strSessionId = id->m_strSessionId;
		switch(eLevel){
			case LOG_DEBUG:   pLogMsg->m_strLevel = "LOG_DEBUG"; break;     //debug-level message
			case LOG_INFO:    pLogMsg->m_strLevel = "LOG_INFO"; break;      //informational message
			case LOG_NOTICE:  pLogMsg->m_strLevel = "LOG_NOTICE"; break;    //normal, but significant, condition
			case LOG_WARNING: pLogMsg->m_strLevel = "LOG_WARNING"; break;   //warning conditions
			case LOG_ERR:     pLogMsg->m_strLevel = "LOG_ERR"; break;       //error conditions
			case LOG_CRIT:    pLogMsg->m_strLevel = "LOG_CRIT"; break;      //critical conditions
		}
		pLogMsg->m_strCategory = strCategory;
		pLogMsg->m_strLogTime = strTime; 
		pLogMsg->m_strLogData = strLogData;

		if(PostThreadMessage(id->m_dwAppLogId, WM_APPLOG, 0, (LPARAM)pLogMsg)){
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X):\n", cdwThreadId, id->m_nControlId);
		}else{
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X):\n", cdwThreadId, id->m_nControlId);
		}
	}
}

// This thread is to load the VO data from large downloaded files and is only needed to be done once
unsigned _stdcall BackgroundDataThread(PVOID pvoid)
{
	struct InstanceData* id = (struct InstanceData*)pvoid;
	const DWORD cdwThreadId = GetCurrentThreadId();
	extern VO  *LoadIATA_ICAO_Arpts(struct InstanceData* id);
	extern VO *IATAAirportsVO;

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X):[%d] Thread Created\n", cdwThreadId, id->m_nControlId);

	do
	{
		// Load these here to speed up initial loading
		if( !UseTriangleList )
			// load this here if we are not using a calllist
			LoadBinaryTriangles( id );

		if(!id->m_bALLoaded){
			InitAL(id);
		}

		if(!id->m_bArptLoaded){
			if(WAIT_TIMEOUT != WaitForSingleObject(id->m_evShutdown, 1)) break; 
			LoadInternationalArpts(id);
			IATAAirportsVO = LoadIATA_ICAO_Arpts(id);
		}
		if(!id->m_bAcTypesLoaded){
			if(WAIT_TIMEOUT != WaitForSingleObject(id->m_evShutdown, 1)) break; 
			LoadAcTypes(id);
		}
		if ( !g_bNavLoaded ){
			if(WAIT_TIMEOUT != WaitForSingleObject(id->m_evShutdown, 1)) break; 
			LoadNav(id);
		}
		if ( !g_bFixesLoaded ){
			if(WAIT_TIMEOUT != WaitForSingleObject(id->m_evShutdown, 1)) break; 
			LoadFixes(id);
		}
		if ( !g_bAirwaysLoaded ){
			if(WAIT_TIMEOUT != WaitForSingleObject(id->m_evShutdown, 1)) break; 
			LoadAirways(id);
		}
		if ( !g_bStarsLoaded ){
			if(WAIT_TIMEOUT != WaitForSingleObject(id->m_evShutdown, 1)) break; 
			LoadStars(id);
		}
		if(!id->m_bAirlineCodesLoaded){
			if(WAIT_TIMEOUT != WaitForSingleObject(id->m_evShutdown, 1)) break; 
			LoadAirlineCodes(id);
		}
		// Re-try until all are loaded
	}while(!(id->m_bArptLoaded && g_bNavLoaded && g_bFixesLoaded && g_bAirwaysLoaded && g_bStarsLoaded) && 
		WAIT_TIMEOUT == WaitForSingleObject(id->m_evShutdown, id->m_dwBkThread));

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X):[%d] Thread Terminated\n", cdwThreadId, id->m_nControlId);
	_endthreadex(0);
	return 0;
}

unsigned _stdcall WeatherThread(PVOID pvoid)
{
	struct InstanceData* id = (struct InstanceData*)pvoid;
	const DWORD cdwThreadId = GetCurrentThreadId();
	int n5MinEnd = (int)(300000/id->m_dwBkThread);
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X):[%d] Thread Created\n", cdwThreadId, id->m_nControlId);

	if(wglMakeCurrent(id->m_hDC, id->m_hRCPassive[1]))
	{
		int nWeatherCount = -1;
		do
		{		
			// The following code attempts to limit the weather download to once every 5 minutes
			// with a succesful weather download, failures will continue to retry wich may
			// happen on start-up
			if(id->m_sCurLayout.m_bShowWeather){
				if(!(++nWeatherCount)){
					
					if(WAIT_TIMEOUT != WaitForSingleObject(id->m_evShutdown, 1)) break; 

					if(!LoadWxTex(id))
						nWeatherCount = -1;
				}else{
					if(nWeatherCount > n5MinEnd)
						nWeatherCount = -1;
				}
			}
		}while(WAIT_TIMEOUT == WaitForSingleObject(id->m_evShutdown, id->m_dwBkThread));
	}
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X):[%d] Thread Terminated\n", cdwThreadId, id->m_nControlId);
	_endthreadex(0);
	return 0;

}

unsigned _stdcall BackgroundThread(PVOID pvoid)
{
	struct InstanceData* id = (struct InstanceData*)pvoid;
	const DWORD cdwThreadId = GetCurrentThreadId();
	int n5MinEnd = (int)(300000/id->m_dwBkThread);
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Thread Created\n", cdwThreadId);
	

	// Kick off Background Data Thread
	RegisterThreadHandle(id, _beginthreadex(NULL, 0, HistoricalROIDataThread, id, 0, NULL), "HistoricalROIDataThread");
	RegisterThreadHandle(id, _beginthreadex(NULL, 0, BackgroundDataThread, id, 0, NULL), "BackgroundDataThread");
	RegisterThreadHandle(id, _beginthreadex(NULL, 0, WeatherThread, id, 0, NULL), "WeatherThread");

	InitRollGrids(id); // need to set even if tags off for selected aircraft 

	if(wglMakeCurrent(id->m_hDC, id->m_hRCPassive[0]))
	{
		int nActionCount = -1;
		SetStatusText(id, "Loading Fonts ...");
		BuildFreeTypeFonts(id);
		DefCreateFont(id);	
		BuildFont(id);
		BuildCustomFonts(id);		
		//BuildGUIFonts(id);
		//BuildVerdanaFonts(id);
		//SetGUIFontIndex(id, 2, 0);
		SetFontIndex(id, 2);
		
		SetStatusText(id, "Loading Aircraft Images ...");
		LoadGLTextures(id);	

		SetStatusText(id, "Loading World Map Components ...");
		if( UseTriangleList )
			// Load this here if we are using a calllist
			LoadBinaryTriangles( id );

		LoadBorders(id);
		id->m_bBasicGraphicsLoaded = TRUE;

		LoadLakes( id );
		LoadARTCCs( id );
		LoadDPs( id );
		LoadSTARs( id );
		
		do{
			CalcTimeAdjustment(id);
			
			if(WAIT_TIMEOUT != WaitForSingleObject(id->m_evShutdown, 1)) break;		
		
			if(!id->m_bROILoaded){
				LoadROI(id);
			}

			// Check for termination prior to each Load function since they do server CURL calls which may be long
			// and we need to handle quick termination.
			if(WAIT_TIMEOUT != WaitForSingleObject(id->m_evShutdown, 1)) break; 
			LoadHiResMap(id);
			
			if(WAIT_TIMEOUT != WaitForSingleObject(id->m_evShutdown, 1)) break; 
			LoadRunways(id);

			if(WAIT_TIMEOUT != WaitForSingleObject(id->m_evShutdown, 1)) break; 
			LoadAirportRingData( id );

			// Do Perform Actions every 5 seconds
			if(!(++nActionCount)){
				if(WAIT_TIMEOUT != WaitForSingleObject(id->m_evShutdown, 1)) break; 
				PerformActions(id);
				LoadSharedMessages(id);
			}
			else if(nActionCount > 5)
				nActionCount = -1;	

			// Load Borders needs to be in the loop for refresh behavior, but moved to end because we perform initial call outside loop (above)
			if (WAIT_TIMEOUT != WaitForSingleObject(id->m_evShutdown, 1)) break;
			LoadBorders(id);

		}while(WAIT_TIMEOUT == WaitForSingleObject(id->m_evShutdown, id->m_dwBkThread));
	}
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Thread Terminated\n", cdwThreadId);
	_endthreadex(0);
	return 0;
}


void CalcTimeAdjustment(struct InstanceData* id)
{
	char url[URL_SZ] = {0};
	char tmpstr[ TMPBUF_SIZE ];
	const DWORD cdwThreadId = GetCurrentThreadId();
	struct MemoryStruct chunk;
	chunk.memory=NULL;
	chunk.size = 0;
	// sprintf_s(tmpstr, TMPBUF_SIZE, "arpt=%s&ip=%s&u=%s", "asd2", id->m_strIPAddress, id->m_strCurluser );
	// ip address is null, so research, but for now use empty string
	sprintf_s(tmpstr, TMPBUF_SIZE, "arpt=%s&ip=%s&u=%s", "asd2", "", id->m_strCurluser);
	sprintf_s(url, URL_SZ, "%s/cgi-bin/latest.cgi", GetAsdSrvr(id));
	if(GetCurlFormGetResults( id, &chunk, url, tmpstr, SM_HTTPS, TRUE )){
		if(chunk.size){
			time_t now = time(0);
			time_t server = CalcFileTime( id, chunk.memory );

			int diff = server - now;
			if(0 == id->m_tServerAdjSecs){
				// Set the adjustment if it has not been set
				id->m_tServerAdjSecs = diff;
				ATLTRACE(L"%s(%d): ADJ(%d), SVR(%d), LOC(%d)\n", __FILE__, __LINE__, id->m_tServerAdjSecs, server, now);
				vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) ADJ(%d), SVR(%d), LOC(%d)\n", cdwThreadId, id->m_tServerAdjSecs, server, now);
			}else if(abs(diff - id->m_tServerAdjSecs) > 60){
				// Only change the time if difference is greater than a minute (don't want it the adj jumping around)
				id->m_tServerAdjSecs = diff;
				ATLTRACE(L"%s(%d): ADJ(%d), SVR(%d), LOC(%d)\n", __FILE__, __LINE__, id->m_tServerAdjSecs, server, now);
				vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) ADJ(%d), SVR(%d), LOC(%d)\n", cdwThreadId, id->m_tServerAdjSecs, server, now);
			}
			free(chunk.memory);
		}
	}
}

int CkUserNamePermission2( struct InstanceData* id, char *permname )
{
	int i = 0;
	int nResult = FALSE;
	const char* perm = NULL;
	const DWORD cdwThreadId = GetCurrentThreadId();

	if ( !permname || !id->m_pstrPerms ){
		return( nResult );
	}

	for(i = 0; i < id->m_nPermCount; i++){
		perm = (id->m_pstrPerms + (i * PERM_SZ));
		if(!strcmp(perm, permname))
		{
			nResult = TRUE;
			break;
		}
	}

	return( nResult );
}

int CkUserNamePermission( struct InstanceData* id, char *permname )
{
  enum json_tokener_error jerror;	
  struct json_object_iter jiter;
  struct MemoryStruct chunk;
  char tmpstr[ TMPBUF_SIZE ];
  int nResult = FALSE;
  const DWORD cdwThreadId = GetCurrentThreadId();

  if ( !permname ){
    return( nResult );
  }

  clear_chunk_memory(&chunk);

  if ( !GetJavascriptValue(id, "username", id->m_strUserName, USERNAME_SZ)){
	strcpy_s(id->m_strUserName, USERNAME_SZ, "unknown");
  }	

  sprintf_s(tmpstr, TMPBUF_SIZE, "Action=GetSurfPermissions&username=%s&perm0=%s", id->m_strUserName, permname );

  if(!GetCurlFormGetResults( id, &chunk, id->m_strHttpDBservername[id->m_nDBServerIndex], tmpstr, SM_HTTPS, TRUE )){
	  vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to load user permssions<%s:%s>\n", cdwThreadId, id->m_strUserName, permname);	  
	  GetNxtSurfSrvr(id);
  }

  if ( chunk.memory ){
	struct json_object* jobj = json_tokener_parse_verbose(chunk.memory, &jerror);
	if(json_tokener_success == jerror && json_object_is_type(jobj, json_type_object)){
		json_object_object_foreachC(jobj, jiter){
			if(!strcmp(permname, jiter.key)){
				nResult = json_object_get_boolean(jiter.val);
				vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) %s Status (%s)\n", cdwThreadId, permname, (nResult ? "TRUE" : "FALSE"));
			}else if(!strcmp("_class", jiter.key) && !strcmp("Error", json_object_get_string(jiter.val))){
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to load user permssions <%s>\n", cdwThreadId, json_object_get_string(json_object_object_get(jobj,"description")));
			}
		}
		json_object_put(jobj);
	}else{
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to load user permssions<%s:%s>\n", cdwThreadId, id->m_strUserName, permname);
	}
    free_chunk_memory(&chunk);
  }
  // should never get here
  return( nResult );
}

int CkPermission( struct InstanceData* id, char *permname )
{
  struct MemoryStruct chunk;
  char tmpstr[ TMPBUF_SIZE ];
  char outbuf[ TMPBUF_SIZE ];
  char username[USERNAME_SZ] = {0};
  const DWORD cdwThreadId = GetCurrentThreadId();

  if ( !permname ){
    return( FALSE );
  }

  chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
  chunk.size = 0;    /* no data at this point */
	
  if ( !GetJavascriptValue(id, "username", username, USERNAME_SZ)){
    strcpy_s(username, USERNAME_SZ, "unknown");
  }	

  sprintf_s(tmpstr, TMPBUF_SIZE, "Action=CkPermission&username=%s&permname=%s", username, permname );

  if ( !GetCurlFormGetResults( id, &chunk, GetPulseSrvr(id), tmpstr, SM_HTTPS, TRUE ) ){
	  vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not get User Permission data from %s, switching srvrs to %s\n",
	  cdwThreadId, GetPulseSrvr(id), GetNxtPulseSrvr(id) );
  }else{
    if ( chunk.memory ){
	  strncpy_s( outbuf, TMPBUF_SIZE, chunk.memory, _TRUNCATE );
	  free( chunk.memory );
	  if ( strstr( outbuf, "granted" ) ){
        return( TRUE );
	  }
	} else {
	  vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) No Permission data returned from server\n", cdwThreadId);
	}

  }
  // should never get here
  return( FALSE );
}


// OpenAL Members
ALCdevice*	m_pALDevice;
ALCcontext* m_pALContext;
const int SOUNDFILE_NAME = 50;
const char SOUNDFILES[SOUNDFILE_COUNT][SOUNDFILE_NAME] = {
	"cannonball_splash_1.wav",
	"cartoon_oh_boy_2.wav",
	"cartoon_uh_oh_2.wav",
	"computer_bleep_2.wav",
	"ding_1.wav",
	"ding_sec.wav",
	"IM00.wav",
	"jetplanef16.wav",
	"magical_shimmer_1.wav",
	"medium_cheer_1.wav",
	"pluck_1.wav",
	"psst_1.wav",
	"psst_2.wav",
	"robot_incoming_transmission_1.wav",
	"sonar_1.wav",
	"suspenseful_note_2.wav",
	"tone_1.wav",
	"tone_2.wav" };
ALuint		m_ALBuffer[SOUNDFILE_COUNT];
ALuint		m_ALSource;


void PlayLocalSound(struct InstanceData* id, unsigned int Sound)
{
	ALint iState;
	const DWORD cdwThreadId = GetCurrentThreadId();

	if(!m_pALDevice){
		InitAL(id);
	}

	alGetSourcei( m_ALSource, AL_SOURCE_STATE, &iState);
	if( iState != AL_STOPPED )
	{
		alSourceStop(m_ALSource);
	}

	alSourcei( m_ALSource, AL_BUFFER, m_ALBuffer[Sound] );
	alSourcePlay( m_ALSource );
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Playing sound\n", cdwThreadId);
}

int BufferSoundFile(struct InstanceData* id, ALuint alBuffer, const char* filename)
{
	ALenum alError;
	const DWORD cdwThreadId = GetCurrentThreadId();
	// Make sure we have the sound file local, This file is for local testing only and is not meant for production.
	char soundfile[MAX_PATH_SZ] = {0};
	sprintf_s(soundfile, MAX_PATH_SZ, "%s%s", id->m_strSoundDir, filename );
	if(!GetLocalFile(id, id->m_strSoundDir, filename, id->m_strHttpSoundPath, SM_HTTPS ))
	{
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__,"(0x%X) Problem accessing %s\n", cdwThreadId,soundfile);
		return FALSE;
	}

	// Read Wave file and load it into buffer
	CWaves Wave;
	WAVEID WaveId;
	unsigned long  uDataSize, uFrequency, uFormat;
	void *pData;	
	if(WR_OK == Wave.LoadWaveFile(soundfile, &WaveId))
	{
		if(WR_OK == Wave.GetWaveSize(WaveId, &uDataSize) &&
		   WR_OK == Wave.GetWaveData(WaveId, &pData) &&
		   WR_OK == Wave.GetWaveFrequency(WaveId, &uFrequency) &&
		   WR_OK == Wave.GetWaveALBufferFormat(WaveId, (PFNALGETENUMVALUE)&alGetEnumValue, &uFormat))
		{			
			alGetError();
			alBufferData(alBuffer, uFormat, pData, uDataSize, uFrequency);
			if( (alError = alGetError()) != AL_NO_ERROR)
			{
				vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Problem loading wavefile %s data (%d).\n", cdwThreadId, soundfile, alError);
				Wave.DeleteWaveFile(WaveId);
				return FALSE;
			}						
		}
		else
		{
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Problem accessing wavefile %s data.\n", cdwThreadId, soundfile);
			Wave.DeleteWaveFile(WaveId);
			return FALSE;
		}
		Wave.DeleteWaveFile(WaveId);
	}
	else
	{
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to load wavefile %s data.\n", cdwThreadId, soundfile);
		return FALSE;
	}
	return TRUE;
}

int InitAL(struct InstanceData* id)
{
	// Initialize OpenAL
	if(id->m_bALLoaded){
		ALenum alError;
		const DWORD cdwThreadId = GetCurrentThreadId();
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Initializing OpenAL\n", cdwThreadId);
		m_pALDevice = alcOpenDevice(NULL); // select the "preferred device"
		if (m_pALDevice) {
			m_pALContext = alcCreateContext(m_pALDevice, NULL);
			alcMakeContextCurrent(m_pALContext);
		}

		// Generate Buffers
		alGetError(); // clear error code
		alGenBuffers(SOUNDFILE_COUNT, m_ALBuffer);	
		if ((alError = alGetError()) != AL_NO_ERROR)
		{
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Error in alGenBuffers(%d)\n", cdwThreadId, alError);
			return FALSE;
		}

		// Generate a Source to playback the Buffer
		alGetError(); // clear error code
		alGenSources( 1, &m_ALSource );
		if ((alError = alGetError()) != AL_NO_ERROR)
		{
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Error in alGenSources(%d)\n", cdwThreadId, alError);
			return FALSE;
		}

		for(int i = 0; i < SOUNDFILE_COUNT; i++){
			// Buffer Sounds
			BufferSoundFile(id, m_ALBuffer[i], SOUNDFILES[i]);		
		}

		// Source is ready to play!
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) OpenAL Initialized!\n", cdwThreadId);
	}
	id->m_bALLoaded = TRUE;
	return id->m_bALLoaded;	
}

void ReleaseAL(struct InstanceData* id)
{
	// Release OpenAL
	if(m_pALDevice)
	{
		alDeleteSources(1, &m_ALSource);
		alDeleteBuffers(SOUNDFILE_COUNT, m_ALBuffer);
		m_pALContext = alcGetCurrentContext();
		m_pALDevice = alcGetContextsDevice(m_pALContext);
		alcMakeContextCurrent(NULL);
		alcDestroyContext(m_pALContext);
		alcCloseDevice(m_pALDevice);
	}
}




BOOL CALLBACK MessageDlgCB(HWND hdlg, UINT message, WPARAM wParam, LPARAM lparam)
{
	//char tmpbuf[64];
	//HWND hwndList;


	//@@@ Might need to use TLS instead of static here.
	static struct InstanceData* id = NULL;

  switch (message)
  {
    // Initialize the dialog box
  case WM_INITDIALOG:
    // SetDlgItemText(hdlg, IDC_GRAPHICS_SPEED, nicetext );
		id = (struct InstanceData*) lparam;
	
    return( TRUE );
    //break;

    // Process command messages
  case WM_COMMAND:
    {
      // Validate and Make the changes

      switch( LOWORD(wParam) )
      {
      case IDOK:
				// Update
				EndDialog(hdlg,TRUE);
        break;
      case IDCANCEL:
				EndDialog(hdlg,TRUE);
        break;
			default:
				return FALSE;
			}
			return TRUE;
		}
		break;

    // Closed from sysbox
  case WM_CLOSE:
		
    EndDialog(hdlg,TRUE);

    break;
  }

  return FALSE;
}




BOOL CALLBACK Message2DlgBlockReplayCB(HWND hdlg, UINT message, WPARAM wParam, LPARAM lparam)
{
	char tmpbuf[64];
	extern void LoadBlockFlights( HWND hDlg );
	HWND hwndList;
	int iIndex, iLength, nSelItems, nSelItemsInBuffer;
	extern int BlockSelectedBuffer[MAX_HISTORICAL_TRACKS];
	// extern int HistoricalCount;
	unsigned int parval, lword, hword;
	extern VO *HisVO;

	//@@@ Might need to use TLS instead of static here.
	static struct InstanceData* id = NULL;

  switch (message)
  {
    // Initialize the dialog box
  case WM_INITDIALOG:
    // SetDlgItemText(hdlg, IDC_GRAPHICS_SPEED, nicetext );
		id = (struct InstanceData*)lparam;
	
    return( TRUE );
    //break;

    // Process command messages
  case WM_COMMAND:
    {
      // Validate and Make the changes

      switch( LOWORD(wParam) )
      {
      case IDOK:
				// Update
				// update the flight to show only the selected flights in the list, hide others but do not delete
				hwndList = GetDlgItem(hdlg, IDC_BLOCK_FLIGHTS_LIST );
				nSelItems = SendMessage(hwndList, LB_GETSELCOUNT, 0, 0); 
				nSelItemsInBuffer = SendMessage(hwndList, LB_GETSELITEMS, MAX_HISTORICAL_TRACKS, (LPARAM) BlockSelectedBuffer); 

				if ( !nSelItems ) BlockSelectedBuffer[0] = -1;
        break;
      case IDCANCEL:
				EndDialog(hdlg,TRUE);
				// clear the polygon
				//FreeEditPolys();
				//id->m_nMode = MODE_NONE;
        break;
			case IDC_BLOCK_FLIGHTS_LIST:
				{
					switch (HIWORD(wParam)) 
					{ 
					case LBN_SELCHANGE:
						{
							hwndList = GetDlgItem(hdlg, IDC_BLOCK_FLIGHTS_LIST );
							iIndex = SendMessage(hwndList, LB_GETCURSEL, 0, 0 );
							iLength = SendMessage(hwndList, LB_GETTEXTLEN, iIndex, 0 ) + 1;
							SendMessage(hwndList, LB_GETTEXT, iIndex, (LPARAM) tmpbuf ); 

							// update BlockSelectedBuffer to reflect change
							nSelItemsInBuffer = SendMessage(hwndList, LB_GETSELITEMS, MAX_HISTORICAL_TRACKS, (LPARAM) BlockSelectedBuffer);
							if ( nSelItemsInBuffer == 0 ){
								BlockSelectedBuffer[0] = -1;
							}

						}
				 }
				}
				return( TRUE );
			case IDC_SELECT_ALL:
				//  select all the rows in the list box for display
				hwndList = GetDlgItem(hdlg, IDC_BLOCK_FLIGHTS_LIST );
				if ( HisVO && HisVO->count ){
					SendMessage(hwndList, LB_SELITEMRANGEEX, 0, HisVO->count - 1);
					nSelItemsInBuffer = SendMessage(hwndList, LB_GETSELITEMS, MAX_HISTORICAL_TRACKS, (LPARAM) BlockSelectedBuffer);
					if ( nSelItemsInBuffer == 0 ){
						BlockSelectedBuffer[0] = -1;
					}
				}
				break;
			case IDC_CLEAR_ALL:
				//  select all the rows in the list box for display
				hwndList = GetDlgItem(hdlg, IDC_BLOCK_FLIGHTS_LIST );
				nSelItems = SendMessage(hwndList, LB_GETSELCOUNT, 0, 0); 
				lword = 0;  // low order bits are for the start index
				hword = (HisVO->count - 1) << 16;  // last item to deselect
				parval = hword + lword;
				SendMessage( hwndList, LB_SELITEMRANGE, 0, parval);
				BlockSelectedBuffer[0] = -1;
				break;
			default:
				return FALSE;
			}
			return TRUE;
		}
		break;

    // Closed from sysbox
  case WM_CLOSE:
		//FreeEditPolys();
    EndDialog(hdlg,TRUE);
		//id->m_nMode = MODE_NONE;
    break;
  }

  return FALSE;
}



BOOL CALLBACK Message2DlgPolyCB(HWND hdlg, UINT message, WPARAM wParam, LPARAM lparam)
{
	char  regionname[LOGIN_MAX_SZ];
	regionname[0] = '\0';
	//@@@ Might need to use TLS instead of static here.
	static struct InstanceData* id = NULL;

  switch (message)
  {
    // Initialize the dialog box
  case WM_INITDIALOG:
    // SetDlgItemText(hdlg, IDC_GRAPHICS_SPEED, nicetext );
	id = (struct InstanceData*)lparam;
    return( TRUE );
    //break;

    // Process command messages
  case WM_COMMAND:
    {
      // Validate and Make the changes

      switch( LOWORD(wParam) )
      {
      case IDOK:
				GetDlgItemText(hdlg, IDC_REGIONNAME, regionname, LOGIN_MAX_SZ - 1 );
				SaveRegion(PolyID, regionname );
				EndDialog(hdlg,TRUE);
				FreeEditPolys();
				// force reload of all regions for this airport
				FreeRegions(id);
				id->m_nMode = MODE_NONE;
        break;
      case IDCANCEL:
				EndDialog(hdlg,TRUE);
				// clear the polygon
				FreeEditPolys();
				id->m_nMode = MODE_NONE;
        break;
	  case IDC_DELETE_POLY:
				GetDlgItemText(hdlg, IDC_REGIONNAME, regionname, LOGIN_MAX_SZ - 1 );
				DeleteRegion(PolyID, regionname );
				EndDialog(hdlg,TRUE);
				FreeEditPolys();
				// force reload of all regions for this airport
				FreeRegions(id);
				id->m_nMode = MODE_NONE;
        break;
      default:
        return FALSE;
      }
      return TRUE;
    }
    break;

    // Closed from sysbox
  case WM_CLOSE:
		FreeEditPolys();
    EndDialog(hdlg,TRUE);
		id->m_nMode = MODE_NONE;
    break;
  }

  return FALSE;
}


void SetStatusText(struct InstanceData* id, const char* strText)
{
	if(TryEnterCriticalSection(&id->m_csStatusSync)){	
		strncpy_s(id->m_strStatusString, STATUSSTRING_SZ, strText, _TRUNCATE);
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Setting Status String to \"%s\"\n", GetCurrentThreadId(), strText);
		LeaveCriticalSection(&id->m_csStatusSync);
	}
}

void UpdateScreen(struct InstanceData* id)
{
	if ( id->m_hDC && id->m_hRCActive ){
		wglMakeCurrent(id->m_hDC, id->m_hRCActive);
		RenderScene(id);
		SwapBuffers(id->m_hDC);
	}
}

void SetLocalCursor(struct InstanceData* id, int nCursor)
{
	SetCursor(id->m_hCursors[nCursor]);
}

BOOL OnLButtonDown(struct InstanceData* id, WPARAM wParam, LPARAM lParam)
{
	BOOL bHandled = FALSE;
	// Need to Filter out Mouse Messages to only those withing the OpenGL Window
	CPoint point(lParam);
	SetCapture(id->m_hWnd);
	if(GetFocus() == id->m_hWnd)
	{
		int mouseisdown = 1, mousestat = 0;	
		id->m_nPtCount = 0;
		id->m_pt[id->m_nPtCount].x = point.x;
		id->m_pt[id->m_nPtCount].y = point.y;
		//sprintf(str, "x=%d y=%d", m_pt[iCount].x, m_pt[iCount].y );
		mymouse( id, mousestat, mouseisdown, (int)id->m_pt[id->m_nPtCount].x, (int)id->m_pt[id->m_nPtCount].y  );
		if ( id->m_nPtCount < MAX_MOUSE_POINTS - 1) id->m_nPtCount++;
		UpdateScreen(id);			
	}
	else
		SetFocus(id->m_hWnd);
	return 0;
}

BOOL OnLButtonDblClk(struct InstanceData* id, WPARAM wParam, LPARAM lParam)
{
	CPoint point(lParam);
	if(!MGLIsGuiMouseEvent(id, 0, 1, point.x, id->m_nHeight - point.y ))
	{
		if(wParam & MK_SHIFT)
			ZoomOut(id);
		else
			ZoomIn(id, &point);
	}
	UpdateScreen(id);
	return 0;
}

BOOL OnLButtonUp(struct InstanceData* id, WPARAM wParam, LPARAM lParam)
{
	// Need to Filter out Mouse Messages to only those withing the OpenGL Window
	CPoint point(lParam);
	if(id->m_nScrollCapture)
		id->m_nScrollCapture = 0;
	int mouseisdown = 0, mousestat = 0;	
	id->m_nPtCount = 0;
	id->m_pt[id->m_nPtCount].x = point.x;
	id->m_pt[id->m_nPtCount].y = point.y;
	
	mymouse( id, mousestat, mouseisdown, (int)id->m_pt[id->m_nPtCount].x, (int)id->m_pt[id->m_nPtCount].y  );
	if ( id->m_nPtCount < MAX_MOUSE_POINTS - 1) 
		id->m_nPtCount++;
	UpdateScreen(id);
	ReleaseCapture( );
	return 0;
}



BOOL OnRButtonUp(struct InstanceData* id, WPARAM wParam, LPARAM lParam)
{
	SetLocalCursor(id, CURSOR_ARROW);
	id->m_ptRight = CPoint(0,0);
	id->m_bRightMouseScroll = FALSE;	
	ReleaseCapture( );

	return TRUE;
}

BOOL OnRButtonDown(struct InstanceData* id, WPARAM wParam, LPARAM lParam)
{
	CPoint point(lParam);

	// Need to Filter out Mouse Messages to only those withing the OpenGL Window
	SetCapture(id->m_hWnd);
	SetLocalCursor(id, CURSOR_HAND);
	id->m_ptRight = CPoint(lParam);
	id->m_bRightMouseScroll = TRUE;

	int mouseisdown = 1, mousestat = 0;	
	
	//sprintf(str, "x=%d y=%d", m_pt[iCount].x, m_pt[iCount].y );
	myRmouse( id, mousestat, mouseisdown, (int) point.x, (int) point.y  );

	return TRUE;
}

BOOL OnMouseMove(struct InstanceData* id, WPARAM wParam, LPARAM lParam)
{
	char tmpstr[256];
	// Need to Filter out MouseMove Messages to only those withing the OpenGL Window
	CPoint point(lParam);
	if(id->m_nScrollCapture){
		// scroll_thumb_cb(id, point.x, id->m_nHeight - point.y, NULL  );
		UpdateScreen(id);
	}

	int mouseisdown = 1, mousestat = 0;	
	if ( wParam && MK_LBUTTON && id->m_nPtCount < (MAX_MOUSE_POINTS - 1) && id->m_nPtCount > 0 ){
		mousestat = 0;
		mouseisdown = 1;
		id->m_pt[id->m_nPtCount].x = point.x;
		id->m_pt[id->m_nPtCount].y = point.y;		
		
		buttonmotion( id, (int)id->m_pt[id->m_nPtCount].x, (int)id->m_pt[id->m_nPtCount].y  );
		if ( id->m_nPtCount < MAX_MOUSE_POINTS - 1) 
			id->m_nPtCount++;
		UpdateScreen(id);
	} else {
		motion( id, LOWORD(lParam), HIWORD(lParam) );			
		if ( id->m_nMeasureX1 != -1 ){
			UpdateScreen(id);
		}
	}
	if(id->m_bRightMouseScroll)
	{
		SetLocalCursor(id, CURSOR_HAND);
		CSize s(id->m_ptRight.x - point.x, id->m_ptRight.y - point.y);
		if (!(0 == s.cx && 0 == s.cy)){
		double xrange =   id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin;
		double yrange =  id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin;
		double dAtan2 = atan2((double)(point.y - id->m_ptRight.y), (double)(id->m_ptRight.x - point.x));
		double dOrgAngle = 180 + (dAtan2)*180/PI;
		double ratio = yrange/xrange;
		double fRadius = sqrt(pow(id->m_ptRight.x - point.x,2.0) + pow(id->m_ptRight.y - point.y,2.0));
		double dRadians = (dOrgAngle - id->m_glfRotateAngle) * PI / 180.0f;

		xrange = xrange * cos(dRadians) * fRadius/id->m_nWidth;
		yrange = yrange * sin(dRadians) * fRadius/id->m_nHeight;
			
		id->m_sCurLayout.m_dWorldYmin -= yrange;
		id->m_sCurLayout.m_dWorldYmax -= yrange;

		id->m_sCurLayout.m_dWorldXmin -= xrange;
		id->m_sCurLayout.m_dWorldXmax -= xrange;

		id->m_sCurLayout.m_dWorldYSize = (id->m_sCurLayout.m_dWorldYmax - id->m_sCurLayout.m_dWorldYmin);
		id->m_sCurLayout.m_dWorldXSize = (id->m_sCurLayout.m_dWorldXmax - id->m_sCurLayout.m_dWorldXmin);
	
		id->m_ptRight = point;
		id->m_bClearScreen = TRUE;
	    id->m_nMode = MODE_NONE; // user is dragging , not measuring using 2 right clicks
		id->m_nMeasureX1 = -1;
		sprintf_s(tmpstr, 256, "End WorldMax(%f, %f), WorldMin(%f, %f)\n", id->m_sCurLayout.m_dWorldXmax, id->m_sCurLayout.m_dWorldYmax, 
			id->m_sCurLayout.m_dWorldXmin, id->m_sCurLayout.m_dWorldYmin );
		OutputDebugString( tmpstr );
		UpdateScreen(id);}
	}
	return 0;
}



BOOL OnMouseHWheel(struct InstanceData* id, WPARAM wParam, LPARAM lParam)
{
	BOOL bHandled = FALSE;
	int fwKeys = GET_KEYSTATE_WPARAM(wParam);
	int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	if(zDelta > 0) // Wheel Moved Right
	{
		do
		{
			bHandled = ProcessKeydown( id, VK_RIGHT, fwKeys );
		}
		while(zDelta -= WHEEL_DELTA);
	}
	else // Wheel Moved Left
	{
		do
		{
			bHandled = ProcessKeydown( id, VK_LEFT, fwKeys );
		}
		while(zDelta += WHEEL_DELTA);
	}
	UpdateScreen(id);
	return bHandled;
}

BOOL OnMouseWheel(struct InstanceData* id, WPARAM wParam, LPARAM lParam)
{
	BOOL bHandled = FALSE;
	int fwKeys = GET_KEYSTATE_WPARAM(wParam);
	int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	
	if(zDelta > 0) // Wheel Moved Up
	{
		do
		{
			ZoomIn(id, NULL);
			zDelta -= WHEEL_DELTA;
		}
		while(zDelta > 0);
	}
	else // Wheel Moved Down
	{
		do
		{
			ZoomOut(id);
			zDelta += WHEEL_DELTA;
		}
		while(zDelta < 0);
	}
	UpdateScreen(id);
	return 0;
}


BOOL OnKeyDown(struct InstanceData* id, WPARAM wParam, LPARAM lParam)
{
	BOOL bHandled = FALSE;
	const DWORD cdwThreadId = GetCurrentThreadId();
	SHORT vk_control = GetKeyState(VK_CONTROL);
	SHORT vk_menu = GetKeyState(VK_MENU);
	D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) wParam(%d), lParam(%ld), VK_CONTROL(%d), VK_MENU(%d)\n", cdwThreadId, wParam, lParam, vk_control, vk_menu));

#ifdef OLDWAY
	moved to processkeydown so grid can process 'Y'
	if('Y' == wParam && vk_control < 1 /*&& vk_menu < 1*/)
	{
		// For Quicklaunch of the surface logfile.
		HINSTANCE hRet = ShellExecute(
        HWND_DESKTOP,					//Parent window
        "open",							//Operation to perform
        "notepad.exe",					//Path to program
        id->m_strLogfileName,   //Parameters
        NULL,							//Default directory
        SW_SHOWNORMAL);					//How to open
		
		//The function returns a HINSTANCE (not really useful in this case)
		//So therefore, to test its result, we cast it to a LONG.
		//Any value over 32 represents success!

		if((LONG)hRet <= 32)
		{
			char strError[95] = {0};
			strerror_s(strError, 95, errno);
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to display logfile (%s), %s\n", cdwThreadId, id->m_strLogfileName, strError);
		}
		bHandled = TRUE;
	}
	else 

	if('S' == wParam && vk_control < 1)
	{
		if( IS_D_CONTROL("SOUND_TEST") ){
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Attempting to play sound\n", cdwThreadId);
			ALint iState;
			if(!m_pALDevice){
				InitAL(id);
			}
			alGetSourcei( m_ALSource, AL_SOURCE_STATE, &iState);
			if(iState != AL_PLAYING)
			{			
				alSourcePlay( m_ALSource );
				vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Playing sound\n", cdwThreadId);
			}
		}

		bHandled = TRUE;
	}
	else
	{
#endif
		
	bHandled = ProcessKeydown( id, wParam, GetKeyState(VK_SHIFT) );
	UpdateScreen(id);
	
	return bHandled;
}



int WindowProc(struct InstanceData* id, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Hook-up/Handle Windows Events Here for WebTracker dll
	switch(uMsg){
		case WM_KEYDOWN: return OnKeyDown(id, wParam, lParam);
		case WM_LBUTTONDBLCLK: return OnLButtonDblClk(id, wParam, lParam); 
		case WM_LBUTTONDOWN: return OnLButtonDown(id, wParam, lParam); 
		case WM_LBUTTONUP: return OnLButtonUp(id, wParam, lParam); 
		case WM_MOUSEHWHEEL: return OnMouseHWheel(id, wParam, lParam);
		case WM_MOUSEMOVE: return OnMouseMove(id, wParam, lParam);
		case WM_MOUSEWHEEL: return OnMouseWheel(id, wParam, lParam);		
		case WM_RBUTTONDOWN: return OnRButtonDown(id, wParam, lParam); 
		case WM_RBUTTONUP: return OnRButtonUp(id, wParam, lParam); 
	}
	return TRUE;
}

void CreateDir(char* Path)
{
	char DirName[MAX_PATH_SZ + 128];
	char* p = Path;
	char* q = DirName;
	BOOL bResult = 0;
	DWORD dwError = 0;	

	while(*p)
	{
		if (('\\' == *p) || ('/' == *p))
		{
			if (':' != *(p-1))
			{
				bResult = CreateDirectory(DirName, NULL);
				if(!bResult)
				{
					dwError = GetLastError( );
					if(dwError != ERROR_ALREADY_EXISTS)
						ATLTRACE("Failure in CreateDir %ld", dwError);
				}
			}
		}
		*q++ = *p++;
		*q = '\0';
	}
	bResult = CreateDirectory(DirName, NULL);
	if(!bResult)
	{
		dwError = GetLastError( );
		if(dwError != ERROR_ALREADY_EXISTS)
			ATLTRACE("Failure in CreateDir %ld", dwError);
	}
}

unsigned _stdcall WebTrackerInitThread(PVOID pvoid)
{
	char strConfigValue[100] = {0};
	struct InstanceData* id = (struct InstanceData*)pvoid;
	const DWORD cdwThreadId = GetCurrentThreadId();
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Thread Created\n", cdwThreadId);

	// Initialize From ConfigFile
	//MAX_REGION_SECS: Defines Time to delete ROI Record (Really only used to prevent new records created with old times)
	if(GetConfigValue(id, "MAX_REGION_SECS", strConfigValue, 100)){
		id->m_nMaxRegionSecs = atoi(strConfigValue);
	}
	
	// Initialize From ConfigFile
	//MAX_REGION_SECS: Defines Time to end ROI Record due to not recieving track updates for flight id
	if(GetConfigValue(id, "MAX_REGION_TIMEOUT", strConfigValue, 100)){
		id->m_nMaxRegionTimeout = atoi(strConfigValue);	
	}	

	InitActiveXCallbacks(id);
	ColorsSetup(id);

	SetStatusText(id, "Loading Images, please wait.......");
	do
	{
		// Also need to have a valid window size before letting UI know we are up.
		if(id->m_bSized && id->m_bInitialized){
			Callback2Javascript(id, "WebTrackerLoaded");
			break;
		}
	}
	while(WAIT_TIMEOUT == WaitForSingleObject(id->m_evShutdown, 1000));	

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Thread Terminated\n", cdwThreadId);
	_endthreadex(0);
	return 0;
}

char* GetLastErrorString(char* szErrorString, int nLen, BOOL bIncludeValue)
{
	ZeroMemory(szErrorString, nLen * sizeof(char));
	LPTSTR lpMsgBuf;
	DWORD dwError = GetLastError( );
	DWORD dwLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&lpMsgBuf,
        0, NULL );
	
	if(bIncludeValue)
		_snprintf_s(szErrorString, nLen, _TRUNCATE, "%ld:%s", dwError, lpMsgBuf);
	else
		strncpy_s(szErrorString, nLen, lpMsgBuf, _TRUNCATE);
	LocalFree(lpMsgBuf);
	return szErrorString;
}


void ClosePrevLogFile(struct InstanceData* id)
{
	char szError[100] = {0};
	const DWORD cdwThreadId = GetCurrentThreadId();
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Curr(0x%X), Prev(0x%X)\n", cdwThreadId, id->m_pfVoLogFile, id->m_pfVoLogFilePrev);
	
	// check to see if prev log file still open 
	if (id->m_pfVoLogFilePrev) {
	   // Yes, close it
	   if(fclose( id->m_pfVoLogFilePrev )){
		   GetLastErrorString(szError, 100, TRUE);
		   vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Error closing previous logfile (%s)\n", cdwThreadId, szError);
		   ATLTRACE("ClosePrevLogFile: %s\n", szError);
	   }
	   id->m_pfVoLogFilePrev  = NULL;
	}

	if (strlen(id->m_strLogfileNamePrev)) {
		// Remove the old log file
		if(remove( id->m_strLogfileNamePrev )){
			char szError[200] = {0};
			ATLTRACE("SetupNextLogFileName: %s\n", GetLastErrorString(szError, 200, TRUE));
		}
		id->m_strLogfileNamePrev[0] = '\0';
	}
}  

void RotateLogFile(struct InstanceData* id)
{
	char newformat[256];
	const DWORD cdwThreadId = GetCurrentThreadId();
	
	// Copy current log file name to prev log file name
	if(strlen(id->m_strLogfileName)){
		strcpy_s(id->m_strLogfileNamePrev, 256, id->m_strLogfileName);
	}

	// Copy current log file FP to prev log file FP
	id->m_pfVoLogFilePrev = id->m_pfVoLogFile;

	strcpy_s(newformat, 256, id->m_strLogFileNameSuffix);
	strcat_s(newformat, 256, id->m_strErrFileDateFmt);

	// Create new log files for the next interval 
	id->m_pfVoLogFile = vo_init_files( newformat, id->m_strErrFilename, 0, 1 );
	sprintf_s(id->m_strLogfileName, 256, "%s_%s", id->m_strErrFilename, newformat);
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Web Tracker Build Version <%s>\n", cdwThreadId, id->m_strWebTrackerVersion);
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Web Tracker Build Timestamp <%s %s>.\n", cdwThreadId, __DATE__, __TIME__);
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Curr(0x%X), Prev(0x%X)\n", cdwThreadId, id->m_pfVoLogFile, id->m_pfVoLogFilePrev);
}  

}
