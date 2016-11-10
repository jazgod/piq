#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/timeb.h>
#include <errno.h>	/*declares extern errno*/

#include <GL/gl.h>
#include <GL/glu.h>

#include "vo.h"
#include "vo_extern.h"
#include "SSI.h"
#include "ExecUtil.h"
#include "vo_db.h"

#include "imu.h" // imunzip()
#include "tracks.h"
#include "mgl.h"
#include "srfc.h"
#include "servers.h"
#include "Curl.h"
#include "InstanceData.h"
#include "SurfaceC.h" // Callback2Javascript
#include "pulseserver.h"

#define LINEBUF_SIZE 1024
#ifndef TZNAME_MAX
#define TZNAME_MAX 100
#endif//TZNAME_MAZ



extern int timeflag;
extern time_t delaystartsecs;

extern int CkUserNamePermission( struct InstanceData* id, char *permname );
extern int CkUserNamePermission2( struct InstanceData* id, char *permname );
extern int UpdASDAircraft( struct InstanceData* id, struct feedstruct *record );
extern void LoadPassurData( struct InstanceData* id );

// Thread Handle Mangement Functions.
// Only use handles returned from _beginthreadex, 
// make sure to call _endthreadex at end of thread func
extern void RegisterThreadHandle( struct InstanceData* id, uintptr_t hThread, const char* strFuncName );	// Saves handle to wait for termination during shutdown
extern void UnregisterThreadHandle( struct InstanceData* id, uintptr_t hThread );	// Removes handle to thread already terminated
extern void GetFusedData(struct InstanceData* id);
extern void RetrieveTails(struct InstanceData* id);



unsigned __stdcall AircraftDataThread(PVOID pvoid);

void free_rec_buffer(char** dest, int* dest_size);
void append_rec_buffer(char** dest, int* dest_size, char* src, int src_size);

void ResetASDEXDataBuf(struct InstanceData* id);
void ResetPassurDataBuf(struct InstanceData* id, int passurindex);
void ResetASDDataBuf(struct InstanceData* id);

static int GetASDHdrsZip( struct InstanceData* id, int CkLastTime, int force_refresh, CURL* curl_handle );

static int GetADSBData(  struct InstanceData* id, int force_refresh, CURL* curl_handle );
static int GetAirAsiaData(  struct InstanceData* id, int force_refresh, CURL* curl_handle );
static int GetAirAsiaFlightPlanData(  struct InstanceData* id, int force_refresh, CURL* curl_handle );
static int GetASDData(  struct InstanceData* id, int CkLastTime, int force_refresh, CURL* curl_handle );
static int GetMLATData(  struct InstanceData* id, int force_refresh, CURL* curl_handle );
static int GetPassurData( struct InstanceData* id, int force_refresh, int passurindex, CURL* curl_handle );
static int GetASDEXData(  struct InstanceData* id, int force_refresh, CURL* curl_handle );
static int GetNoiseData(  struct InstanceData* id, int force_refresh, CURL* curl_handle );

#ifdef PERFORMANCE_TESTING
	#define TIME_TRACE_DBG(msg)\
	{\
		SYSTEMTIME t;\
		char s[LINEBUF_SIZE]={0};\
		GetSystemTime(&t);\
		sprintf_s(s, LINEBUF_SIZE, "%s (%d): [%ld] TRACE-%s (%d:%d %d.%d)\n", __FILE__, __LINE__, GetCurrentThreadId(), msg, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);\
		OutputDebugString(s);\
	}
#else
	#define TIME_TRACE_DBG(msg) 
#endif

void ResetAirAsiaDataBuf(struct InstanceData* id)
{
	const DWORD cdwThreadId = GetCurrentThreadId();
	D_CONTROL("THREAD_LOCKING", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) EnterCriticalSection AirAsia Sync\n", cdwThreadId));
	EnterCriticalSection(&id->m_csAirAsiaSync);
	memset(id->m_strAdsbPrevFile, 0, sizeof(id->m_strAirAsiaPrevFile));
	free_rec_buffer(&id->m_pAirAsiaFileBuf, &id->m_lAirAsiaFileNBytes);
	LeaveCriticalSection(&id->m_csAirAsiaSync);
	D_CONTROL("THREAD_LOCKING", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) LeaveCriticalSection AirAsia Sync\n", cdwThreadId));
}

void ResetADSBDataBuf(struct InstanceData* id)
{
	const DWORD cdwThreadId = GetCurrentThreadId();
	D_CONTROL("THREAD_LOCKING", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) EnterCriticalSection ADSB Sync\n", cdwThreadId));
	EnterCriticalSection(&id->m_csADSBSync);
	memset(id->m_strAdsbPrevFile, 0, sizeof(id->m_strAdsbPrevFile));
	free_rec_buffer(&id->m_pADSBFileBuf, &id->m_lADSBFileNBytes);
	LeaveCriticalSection(&id->m_csADSBSync);
	D_CONTROL("THREAD_LOCKING", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) LeaveCriticalSection ADSB Sync\n", cdwThreadId));
}

void ResetASDEXDataBuf(struct InstanceData* id)
{
	const DWORD cdwThreadId = GetCurrentThreadId();
	D_CONTROL("THREAD_LOCKING", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) EnterCriticalSection ASDEX Sync\n", cdwThreadId));
	EnterCriticalSection(&id->m_csASDEXSync);
	memset(id->m_strAsdexPrevFile, 0, sizeof(id->m_strAsdexPrevFile));
	free_rec_buffer(&id->m_pASDEXFileBuf, &id->m_lASDEXFileNBytes);
	LeaveCriticalSection(&id->m_csASDEXSync);
	D_CONTROL("THREAD_LOCKING", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) LeaveCriticalSection ASDEX Sync\n", cdwThreadId));
}

void ResetMLATDataBuf(struct InstanceData* id)
{
	const DWORD cdwThreadId = GetCurrentThreadId();
	D_CONTROL("THREAD_LOCKING", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) EnterCriticalSection MLAT Sync\n", cdwThreadId));
	EnterCriticalSection(&id->m_csMLATSync);
	memset(id->m_strMLATPrevFile, 0, sizeof(id->m_strMLATPrevFile));
	free_rec_buffer(&id->m_pMLATFileBuf, &id->m_lMLATFileNBytes);
	LeaveCriticalSection(&id->m_csMLATSync);
	D_CONTROL("THREAD_LOCKING", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) LeaveCriticalSection MLAT Sync\n", cdwThreadId));
}

void ResetNoiseDataBuf(struct InstanceData* id)
{
	const DWORD cdwThreadId = GetCurrentThreadId();
	D_CONTROL("THREAD_LOCKING", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) EnterCriticalSection Noise Sync\n", cdwThreadId));
	EnterCriticalSection(&id->m_csNoiseSync);
	memset(id->m_strNoisePrevFile, 0, sizeof(id->m_strNoisePrevFile));
	free_rec_buffer(&id->m_pNoiseFileBuf, &id->m_lNoiseFileNBytes);
	LeaveCriticalSection(&id->m_csNoiseSync);
	D_CONTROL("THREAD_LOCKING", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) LeaveCriticalSection Noise Sync\n", cdwThreadId));
}

void ResetPassurDataBuf(struct InstanceData* id, int passurindex)
{
	const DWORD cdwThreadId = GetCurrentThreadId();
	D_CONTROL("THREAD_LOCKING", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) EnterCriticalSection Passur Data Sync\n", cdwThreadId));
	EnterCriticalSection(&id->m_csPassurSync);
	memset(id->m_strPassurPrevFile[passurindex], 0, sizeof(id->m_strPassurPrevFile[passurindex]));
	free_rec_buffer(&(id->m_pPassurfilebufs[passurindex]), &(id->m_lPassurFileNBytes[passurindex]));
	LeaveCriticalSection(&id->m_csPassurSync);
	D_CONTROL("THREAD_LOCKING", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) LeaveCriticalSection Passur Data Sync\n", cdwThreadId));
}

void ResetASDDataBuf(struct InstanceData* id)
{
	const DWORD cdwThreadId = GetCurrentThreadId();
	D_CONTROL("THREAD_LOCKING", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) EnterCriticalSection ASD Sync\n", cdwThreadId));
	EnterCriticalSection(&id->m_csASDSync);
	free_rec_buffer(&id->m_pASDfilebuf, &id->m_nASDFileNBytes);
	LeaveCriticalSection(&id->m_csASDSync);
	D_CONTROL("THREAD_LOCKING", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) LeaveCriticalSection ASD Sync\n", cdwThreadId));
}

void ResetDataBufs(struct InstanceData* id, int passurindex, int reset_asd)
{
	// Reset the last get time so it retrieves new data
	id->m_tLastNoiseGetTime = 0;
	id->m_tNoiseFileTime = 0;
	ResetNoiseDataBuf(id);

	id->m_tLastMLATGetTime = 0;
	id->m_tMLATFileTime = 0;
	memset(&(id->m_sMLATReqStart), 0, sizeof(id->m_sMLATReqStart));
	ResetMLATDataBuf(id);

	id->m_tLastADSBGetTime = 0;
	id->m_tADSBFileTime = 0;
	memset(&(id->m_sADSBReqStart), 0, sizeof(id->m_sADSBReqStart));
	ResetADSBDataBuf(id);

	id->m_tLastAirAsiaGetTime = 0;
	id->m_tAirAsiaFileTime = 0;
	id->m_tAirAsiaFlightPlanFileTime = 0;
	memset(&(id->m_sAirAsiaReqStart), 0, sizeof(id->m_sAirAsiaReqStart));
	ResetAirAsiaDataBuf(id);

	id->m_tLastASDEXGetTime = 0;
	id->m_tASDEXFileTime = 0;
	memset(&(id->m_sASDEXReqStart), 0, sizeof(id->m_sASDEXReqStart));
	ResetASDEXDataBuf(id);

	id->m_tLastPASSURGetTime[passurindex] = 0;
	id->m_tPASSURFileTime = 0;
	memset(&(id->m_sPassurReqStart), 0, sizeof(id->m_sPassurReqStart));
	ResetPassurDataBuf(id, passurindex);

	if (reset_asd) {
		id->m_tLastASDThreadSecs = 0;
		id->m_tLastASDGetTime = 0;
		id->m_tASDFileTime = 0;
		id->m_bASDFirstLoad = TRUE;
		memset(&(id->m_sASDReqStart), 0, sizeof(id->m_sASDReqStart));
		ResetASDDataBuf(id);
	}	
}

unsigned __stdcall ASDPreloadThread(PVOID pvoid)
{
	struct InstanceData* id = (struct InstanceData*)pvoid;
	CURL* curl_handle = NULL;
	const DWORD cdwThreadId = GetCurrentThreadId();

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X):[%d] Thread Created\n", cdwThreadId, id->m_nControlId);

	if (!curl_handle) {
		curl_handle = GetCurlHandle(id, SM_HTTPS);
		if (!curl_handle)
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Calling GetCurlHandle() failed\n", cdwThreadId);
	}

	GetASDHdrsZip(id, TRUE, TRUE, curl_handle);

	curl_easy_cleanup(curl_handle);

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X):[%d] Thread Terminated\n", cdwThreadId, id->m_nControlId);
	
	_endthreadex(0);
	return (0);
}

unsigned __stdcall AircraftDataThread(PVOID pvoid)
{
  static int firsttime = 1;
  struct InstanceData* id = (struct InstanceData*)pvoid;
  DWORD dwTimeout;
  HANDLE hEvents[2] = { id->m_evShutdown, id->m_evAirDataReset };
  int event_code;
  const DWORD cdwThreadId = GetCurrentThreadId();

  vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X):[%d] Thread Created\n", cdwThreadId, id->m_nControlId);

  if ( firsttime ){
    if ( SaveLocalData ){ // IMAP only
      LoadPassurData(id);
    }
    firsttime = 0;
  }

  D_CONTROL("ADT", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Thread loop starting\n", cdwThreadId ));
  
  if(id->m_nDelaySecs){
    //id->m_nDelaySecs += (time(0) - id->m_tReplayStart) + 10;
    id->m_nDelaySecs += (time(0) - id->m_tReplayStart);
  }

  ResetDataBufs(id, 0, 1);

  event_code = WAIT_TIMEOUT;

  while (1) {

	if (WAIT_TIMEOUT != event_code) {
		D_CONTROL("ADT", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) evAirDataReset\n", cdwThreadId ));
		// Adjust the Replay delay to offset thread wait/restart time period
		if(id->m_nDelaySecs){
			id->m_nDelaySecs += (time(0) - id->m_tReplayStart);
		}
		ResetDataBufs(id, 0, id->m_bResetASDTracks);
		ResetEvent(id->m_evAirDataReset);
		SetEvent(id->m_evAirDrawReset); // OK to Render now
	}

	if(id->m_nDelaySecs){
		if(id->m_nReplaySpeed > 5)
			dwTimeout = 1;
		else
			dwTimeout = id->m_dwAdThread/id->m_nReplaySpeed;
	} else {
		if (id->m_bFastUpdates) {
			dwTimeout = 100;
		} else {
			dwTimeout = id->m_dwAdThread;
		}
	}

	// Increment Thread Heart Beat, Watched by RenderStrat
    id->m_dwAirDataCurrBeat++;

    if (!id->m_pCurlHandle) {
		id->m_pCurlHandle = GetCurlHandle(id, SM_HTTPS);
		if (!id->m_pCurlHandle)
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Calling GetCurlHandle() failed\n", cdwThreadId);
	}
    if (!id->m_pCurlHandleASDEX) {
		id->m_pCurlHandleASDEX = GetCurlHandle(id, SM_HTTPS);
		if (!id->m_pCurlHandleASDEX)
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Calling GetCurlHandle() failed\n", cdwThreadId);
	}
    if (!id->m_pCurlHandleADSB) {
		id->m_pCurlHandleADSB = GetCurlHandle(id, SM_HTTPS);
		if (!id->m_pCurlHandleADSB)
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Calling GetCurlHandle() failed\n", cdwThreadId);
	}
    if (!id->m_pCurlHandleAirAsia) {
		id->m_pCurlHandleAirAsia = GetCurlHandle(id, SM_HTTPS);
		if (!id->m_pCurlHandleAirAsia)
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Calling GetCurlHandle() failed\n", cdwThreadId);
	}
    if (!id->m_pCurlHandleMLAT) {
		id->m_pCurlHandleMLAT = GetCurlHandle(id, SM_HTTPS);
		if (!id->m_pCurlHandleMLAT)
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Calling GetCurlHandle() failed\n", cdwThreadId);
	}
    if (!id->m_pCurlHandleNoise) {
		id->m_pCurlHandleNoise = GetCurlHandle(id, SM_HTTPS);
		if (!id->m_pCurlHandleNoise)
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Calling GetCurlHandle() failed\n", cdwThreadId);
	}

 
	if (REPLAY_STATUS_PAUSED != id->m_bReplayStatus)	{

      TIME_TRACE_DBG("START - ADH GetASDZipsThread");
      //vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Calling GetASDZipsThread\n", cdwThreadId );
	  GetASDData(id, TRUE, TRUE, id->m_pCurlHandle);
	  TIME_TRACE_DBG("END - ADH GetASDZipsThread");

	  // Increment Thread Heart Beat, Watched by RenderStrat
	  id->m_dwAirDataCurrBeat++;

	  // Always check for signal events before doing long download
	  event_code = WaitForMultipleObjects(2, hEvents, FALSE, 1);
	  if(WAIT_TIMEOUT != event_code) {
		  if (event_code - WAIT_OBJECT_0 == 1) continue; // evAirDataReset
		  break; // evShutdown
	  }

	  if(id->m_sCurLayout.m_bShowAirAsiaAircraft){
	    // Check for Permissions to download ADSB
	    if( id->m_bAirAsia ){
			static time_t lastfp = 0;
			time_t now = time(NULL);
		    event_code = WaitForMultipleObjects(2, hEvents, FALSE, 1);
		    if(WAIT_TIMEOUT != event_code) {
  		      if (event_code - WAIT_OBJECT_0 == 1) continue; // evAirDataReset
		      break; // evShutdown
		    }

			TIME_TRACE_DBG("START - ADH GetAirAsiaData");
			GetAirAsiaData(id, TRUE, id->m_pCurlHandleAirAsia);
			TIME_TRACE_DBG("END - ADH GetAirAsiaData");
		
			// Only execute every 5 minutes"
			if( now - lastfp > 120 ){
				lastfp = now;
				TIME_TRACE_DBG("START - ADH GetAirAsiaFlightPlanData");
				GetAirAsiaFlightPlanData(id, TRUE, id->m_pCurlHandleAirAsia);
				TIME_TRACE_DBG("END - ADH GetAirAsiaFlightPlanData");
			}
	    }else{
		  Callback2Javascript(id, "AirAsiaStatus:DENIED");		  
		}
	  } else {		
		  Callback2Javascript(id, "AirAsiaStatus:Off");		
	  }

	  // Increment Thread Heart Beat, Watched by RenderStrat
	  id->m_dwAirDataCurrBeat++;

	  if(id->m_sCurLayout.m_bShowADSBAircraft){
	    // Check for Permissions to download ADSB
	    if(id->m_bAdsbAll || id->m_nAdsbArpt){
	      if(!id->m_bAdsbAll && -1 == id->m_nAdsbArpt){
			char strPerm[11] = {0};
			sprintf_s(strPerm, _countof(strPerm), "ADSB_%s", id->m_strAdsbArpt);
			id->m_nAdsbArpt = CkUserNamePermission2(id, strPerm);

			if(!id->m_nAdsbArpt){
				char strArpt[4] = {0};
				char strError[100] = {0};
				if(IsValidAdsbAirport(id, id->m_strAdsbArpt))
				{
					Callback2Javascript(id, "AdsbStatus:DENIED");
				}else{
					Callback2Javascript(id, "AdsbStatus:N/A");
				}
			}
		  }
		  if(id->m_nAdsbArpt){
		    // Always check for signal events before doing long download
		    event_code = WaitForMultipleObjects(2, hEvents, FALSE, 1);
		    if(WAIT_TIMEOUT != event_code) {
  		      if (event_code - WAIT_OBJECT_0 == 1) continue; // evAirDataReset
		      break; // evShutdown
		    }

		    TIME_TRACE_DBG("START - ADH GetADSBData");
		    GetADSBData(id, TRUE, id->m_pCurlHandleADSB);
		    TIME_TRACE_DBG("END - ADH GetADSBData");
		  }
	    }else{
		  memset(id->m_dADSBRequestTimes, 0, sizeof(id->m_dADSBRequestTimes)); // clear request timings
		  if(IsValidAdsbAirport(id, id->m_strAdsbArpt))
		    Callback2Javascript(id, "AdsbStatus:DENIED");
		  else
		    Callback2Javascript(id, "AdsbStatus:N/A");
		}
	  } else {		
		  memset(id->m_dADSBRequestTimes, 0, sizeof(id->m_dADSBRequestTimes)); // clear request timings
		  Callback2Javascript(id, "AdsbStatus:Off");		
	  }
	  // Increment Thread Heart Beat, Watched by RenderStrat
	  id->m_dwAirDataCurrBeat++;

	  // Always check for signal events before doing long download
	  event_code = WaitForMultipleObjects(2, hEvents, FALSE, 1);
	  if(WAIT_TIMEOUT != event_code) {
		  if (event_code - WAIT_OBJECT_0 == 1) continue; // evAirDataReset
		  break; // evShutdown
	  }

	  if(id->m_sCurLayout.m_bShowMLATAircraft){
	    // Check for Permissions to download MLAT
	    if(id->m_bMLATAll || id->m_nMLATArpt){
	      if(!id->m_bMLATAll && -1 == id->m_nMLATArpt){
			char strPerm[11] = {0};
			sprintf_s(strPerm, _countof(strPerm), "MLAT_%s", id->m_strMLATArpt);
			id->m_nMLATArpt = CkUserNamePermission2(id, strPerm);

			if(!id->m_nMLATArpt){
				char strArpt[4] = {0};
				char strError[100] = {0};
				if(IsValidMLATAirport(id, id->m_strMLATArpt))
				{
					Callback2Javascript(id, "MlatStatus:DENIED");
				}else{
					Callback2Javascript(id, "MlatStatus:N/A");
				}
			}
		  }
		  if(id->m_nMLATArpt){
		    // Always check for signal events before doing long download
		    event_code = WaitForMultipleObjects(2, hEvents, FALSE, 1);
		    if(WAIT_TIMEOUT != event_code) {
  		      if (event_code - WAIT_OBJECT_0 == 1) continue; // evAirDataReset
		      break; // evShutdown
		    }

		    TIME_TRACE_DBG("START - ADT GetMLATData");
		    GetMLATData(id, TRUE, id->m_pCurlHandleMLAT);
		    TIME_TRACE_DBG("END - ADT GetMLATData");
		  }
	    }else{
		  memset(id->m_dMLATRequestTimes, 0, sizeof(id->m_dMLATRequestTimes)); // clear request timings
		  if(IsValidMLATAirport(id, id->m_strMLATArpt))
		    Callback2Javascript(id, "MlatStatus:DENIED");
		  else
		    Callback2Javascript(id, "MlatStatus:N/A");
		}
	  } else {		
		  memset(id->m_dADSBRequestTimes, 0, sizeof(id->m_dMLATRequestTimes)); // clear request timings
		  Callback2Javascript(id, "MlatStatus:Off");		
	  }
	  // Increment Thread Heart Beat, Watched by RenderStrat
	  id->m_dwAirDataCurrBeat++;

	  // Always check for signal events before doing long download
	  event_code = WaitForMultipleObjects(2, hEvents, FALSE, 1);
	  if(WAIT_TIMEOUT != event_code) {
		  if (event_code - WAIT_OBJECT_0 == 1) continue; // evAirDataReset
		  break; // evShutdown
	  }

	  //vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Calling GetMoreDataThread\n", cdwThreadId );
	  TIME_TRACE_DBG("START - ADH GetPassurData");
	  if (IsValidPassurAirport(id, id->m_strPassurArpts[0]))
	  {
		  if (strcmp("kulF", id->m_strPassurArpts[0]))
			GetPassurData(id, TRUE, 0, id->m_pCurlHandle);
		  else
			  Callback2Javascript(id, "PassurStatus:N/A");
	  }
	  else
	  {
		  Callback2Javascript(id, "PassurStatus:N/A");
	  }
	  TIME_TRACE_DBG("END - ADH GetPassurData");

	  // Increment Thread Heart Beat, Watched by RenderStrat
	  id->m_dwAirDataCurrBeat++;

  	  // Always check for signal events before doing long download
	  event_code = WaitForMultipleObjects(2, hEvents, FALSE, 1);
	  if(WAIT_TIMEOUT != event_code) {
		  if (event_code - WAIT_OBJECT_0 == 1) continue; // evAirDataReset
		  break; // evShutdown
	  }

	  if(id->m_sCurLayout.m_bShowASDEXAircraft){
	    // Check for Permissions to download ASDEX
		if(id->m_bAsdexAll || id->m_nAsdexArpt){
		  if(!id->m_bAsdexAll && -1 == id->m_nAsdexArpt){
			char strPerm[11] = {0};
			sprintf_s(strPerm, _countof(strPerm), "ASDEX_%s", id->m_strAsdexArpt);
			id->m_nAsdexArpt = CkUserNamePermission2(id, strPerm);

			if(!id->m_nAsdexArpt){
				char strArpt[4] = {0};
				char strError[100] = {0};
				if(IsValidAsdexAirport(id, id->m_strAsdexArpt)){
					Callback2Javascript(id, "AsdexStatus:DENIED");	
				}else{
					Callback2Javascript(id, "AsdexStatus:N/A");
				}
			}
		  }
		  if(id->m_nAsdexArpt){
		    // Always check for signal events before doing long download
		    event_code = WaitForMultipleObjects(2, hEvents, FALSE, 1);
		    if(WAIT_TIMEOUT != event_code) {
  		      if (event_code - WAIT_OBJECT_0 == 1) continue; // evAirDataReset
		      break; // evShutdown
		    }

		    //vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Calling GetASDEXData\n", cdwThreadId );
		    TIME_TRACE_DBG("START - ADH GetASDEXData");
		    GetASDEXData(id, TRUE, id->m_pCurlHandleASDEX);
		    TIME_TRACE_DBG("END - ADH GetASDEXData");
		  }
		}else{
		  memset(id->m_dASDEXRequestTimes, 0, sizeof(id->m_dASDEXRequestTimes)); // clear request timings
		  if(IsValidAsdexAirport(id, id->m_strAsdexArpt))
		    Callback2Javascript(id, "AsdexStatus:DENIED");
		  else
		    Callback2Javascript(id, "AsdexStatus:N/A");
		}
	  } else {
		  memset(id->m_dASDEXRequestTimes, 0, sizeof(id->m_dASDEXRequestTimes)); // clear request timings
		  Callback2Javascript(id, "AsdexStatus:Off");
	  }

	  if ( SaveLocalData ){
        // Always check for signal events before doing long download
		event_code = WaitForMultipleObjects(2, hEvents, FALSE, 1);
		if(WAIT_TIMEOUT != event_code) {
		  if (event_code - WAIT_OBJECT_0 == 1) continue; // evAirDataReset
		  break; // evShutdown
		}
	    TIME_TRACE_DBG("START - ADH GetPassurData");
		GetPassurData(id, TRUE, 1, id->m_pCurlHandle);
		TIME_TRACE_DBG("END - ADH GetPassurData");
	  }	
	}

	// now use longer timeout to sleep a while
    event_code = WaitForMultipleObjects(2, hEvents, FALSE, dwTimeout);
    if(WAIT_TIMEOUT != event_code) {
      if (event_code - WAIT_OBJECT_0 == 1) continue; // evAirDataReset
	  break; // evShutdown
	}

	TIME_TRACE_DBG("START - ADH GetNoiseData");
	GetNoiseData(id, TRUE, id->m_pCurlHandleNoise);
	TIME_TRACE_DBG("END - ADH GetNoiseData");

	// Increment Thread Heart Beat, Watched by RenderStrat
	id->m_dwAirDataCurrBeat++;

	// Always check for signal events before doing long download
	event_code = WaitForMultipleObjects(2, hEvents, FALSE, 1);
	if(WAIT_TIMEOUT != event_code) {
		if (event_code - WAIT_OBJECT_0 == 1) continue; // evAirDataReset
		break; // evShutdown
	}
  }

  if (0 != id->m_hASDPreloadThread) {
	// hopefully never occurs because the thread could be terminated while holding a critical section
	if (TerminateThread((HANDLE)id->m_hASDPreloadThread, 1)) {
	  UnregisterThreadHandle(id, id->m_hASDPreloadThread);
	  CloseHandle((HANDLE)id->m_hASDPreloadThread);
	  id->m_hASDPreloadThread = 0;
	}
  }

  vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X):[%d] Thread Terminated\n", cdwThreadId, id->m_nControlId);
  vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X):[%d] event_code %d\n", cdwThreadId, id->m_nControlId, event_code);
  TIME_TRACE_DBG("AircraftDataThread Completed");
  _endthreadex(0);
  return (0);
}


void free_rec_buffer(char** dest, int* dest_size)
{
	free(*dest);
	*dest = NULL;
	*dest_size = 0;
}


void append_rec_buffer(char** dest, int* dest_size, char* src, int src_size)
{
	char* save_buffer;
	save_buffer = *dest;

	if (*dest_size + src_size > 800000) { // ASD summary data is ~500kb
		/* maximum buffer size */
		return;
	}

	if (NULL == *dest) {
		*dest = malloc(src_size + 1);
		if (NULL == *dest) return;
		memcpy(*dest, src, src_size);
		(*dest)[src_size] = '\0';
		*dest_size = src_size;
		return;
	}

	if ((*dest)[*dest_size-1] != '\n') {
		/* make sure previous record is terminated */
		(*dest)[*dest_size] = '\n';
		++(*dest_size);
	}
	*dest = realloc(*dest, *dest_size + src_size + 1);
	if (NULL != *dest) {
	  memcpy(*dest + *dest_size, src, src_size);
      *dest_size += src_size;
	  (*dest)[*dest_size] = '\0';
	} else {
		*dest = save_buffer;
	}
}


time_t CalcFileTime( struct InstanceData* id, char *url )
{
  char	*sprtr = "/";
  struct tm tmTemp;
  char *str;
  char tokstr[256];
  char *year, *month, *day, *hour;
  char tmpstr[ TMPBUF_SIZE ], timestr[ TMPBUF_SIZE ], timebuf[64];
  char tz0[TZNAME_MAX], tz1[TZNAME_MAX];
  time_t unixsecs, nowsecs, secs;
  long tz;
  int minutes, seconds, len;
  struct tm newtime, new2time;
  int daylighthours, dstbias, tzoffset;
  char *context;
  const DWORD cdwThreadId = GetCurrentThreadId();
	// size_t pReturnValue;
  // char timeZoneName[32];


  // example https://207.252.77.135/sdata/asd2/2006/10/26/19/5900.zip 
  // vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) url='%s'\n", cdwThreadId, url );


	_get_daylight( &daylighthours);
	_get_dstbias( &dstbias );
	_get_timezone( &tzoffset );
  //_get_tzname(&pReturnValue, timeZoneName, sizeof(timeZoneName), 0 );


	// sprintf(timestr, "_daylight=%d _dstbias=%d _timezone=%d", daylighthours, dstbias, tzoffset);

  nowsecs = time(0);
  gmtime_s(&tmTemp, &nowsecs );

  /* ignore 'data/' in beginning */
  if ( !(str = strstr(url, "asd2/" )) ){
    return(0);
  }
    
  str += 5; // point to year
  strncpy_s( tokstr, _countof(tokstr), str, 255 );
  tokstr[255] = '\0';
  
  if ( !(year = strtok_s( tokstr, sprtr, &context ) )){
    return(0);
  }
  if ( !(month = strtok_s ( NULL, sprtr, &context ))){
    return(0);
  }
  if ( !(day = strtok_s ( NULL, sprtr, &context ))){
    return(0);
  }
  if ( !(hour = strtok_s ( NULL, sprtr, &context ))){
    return(0);
  }

  secs = time(0);
  localtime_s( &newtime, &secs );  // set dst

  /* get minutes and seconds */
  if ( !(str = strstr(url, ".zip" )) ){
    vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) could not find .zip in file, bad zipfilename\n", cdwThreadId);
    return(0);
  }

  str -= 4;
  strncpy_s(tmpstr, TMPBUF_SIZE, str, 2 );
  tmpstr[2] = '\0';
  minutes = atoi(tmpstr);
  

  str += 2;
  strncpy_s(tmpstr, TMPBUF_SIZE, str, 2 );
  tmpstr[2] = '\0';
  seconds = atoi(tmpstr);
  
  tmTemp.tm_year = atoi(year) - 1900;
  tmTemp.tm_mon = atoi(month) - 1;
  tmTemp.tm_mday = atoi(day);
  tmTemp.tm_hour = atoi(hour);
  tmTemp.tm_min = minutes;
  tmTemp.tm_sec = seconds;
  tmTemp.tm_isdst = -1; // let mktime figure out dst or not, if 0, then it assumes no DST
  
  unixsecs = mktime( &tmTemp );  // mktime assumes tmptr parameters are local time, so it returns seconds in local time
  _get_timezone(&tz);
  unixsecs -= tz;  // convert back into gmt
  localtime_s( &newtime, &unixsecs );
  
  // see if we are in dst
  secs = time(0);
  localtime_s( &new2time, &secs );
  //sprintf( timestr, "tm_isdst = %d time=%s\n", new2time.tm_isdst, VOTimeFmt(secs, "%Y-%m-%d %H:%M:%S" ) );

  if ( new2time.tm_isdst ){
	unixsecs -= dstbias;
  }

  sprintf_s( timestr, TMPBUF_SIZE, "time=%s isdst=%d dstbias=%d\n ", 
			VOTimeFmt(timebuf, unixsecs, "%Y-%m-%d %H:%M:%S" ), new2time.tm_isdst,  dstbias  );
  _get_tzname(&len, tz0, TZNAME_MAX, 0);
  _get_tzname(&len, tz1, TZNAME_MAX, 1);
  D_CONTROL("ASD_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) time=%s _tzname[0] = '%s' _tzname[1]=%s isdst=%d\n", cdwThreadId, timestr, tz0, tz1,
			new2time.tm_isdst ));

  return( unixsecs );

}


time_t CalcPassurFileTime( struct InstanceData* id, char *url )
{
  char	*sprtr = "/";
  struct tm tmTemp;
  char *str;
  char tokstr[256];
  char *year, *month, *day, *hour;
  char tmpstr[ TMPBUF_SIZE ], timestr[ TMPBUF_SIZE ];
  char tz0[TZNAME_MAX], tz1[TZNAME_MAX];
  time_t unixsecs, nowsecs, secs;
  long tz;
  int minutes, seconds, len;
  struct tm newtime, new2time;
  int daylighthours, dstbias, tzoffset;
  char *context;
  const DWORD cdwThreadId = GetCurrentThreadId();
  static int pflag = 0;

	// size_t pReturnValue;
  // char timeZoneName[32];


  // example https://207.252.77.135/sdata/asd2/2006/10/26/19/5900.zip 
  // vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) url='%s'\n", cdwThreadId, url );


	_get_daylight( &daylighthours);
	_get_dstbias( &dstbias );
	_get_timezone( &tzoffset );
  //_get_tzname(&pReturnValue, timeZoneName, sizeof(timeZoneName), 0 );


	// sprintf(timestr, "_daylight=%d _dstbias=%d _timezone=%d", daylighthours, dstbias, tzoffset);

  nowsecs = time(0);
  gmtime_s( &tmTemp, &nowsecs );

  /* ignore 'data/' in beginning */
  if ( !(str = strstr(url, "sdata/" )) ){
    return(0);
  }
    
  str += 11; // point to year
  strncpy_s( tokstr, _countof(tokstr), str, 255 );
  tokstr[255] = '\0';
  
  if ( !(year = strtok_s( tokstr, sprtr, &context ) )){
    return(0);
  }
  if ( !(month = strtok_s ( NULL, sprtr, &context ))){
    return(0);
  }
  if ( !(day = strtok_s ( NULL, sprtr, &context ))){
    return(0);
  }
  if ( !(hour = strtok_s ( NULL, sprtr, &context ))){
    return(0);
  }

  secs = time(0);
  localtime_s( &newtime, &secs );  // set dst

  /* get minutes and seconds */
  if ( !(str = strstr(url, ".zip" )) ){
    vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) could not find .zip in file, bad zipfilename\n", cdwThreadId);
    return(0);
  }

  str -= 4;
  strncpy_s(tmpstr, TMPBUF_SIZE, str, 2 );
  tmpstr[2] = '\0';
  minutes = atoi(tmpstr);
  

  str += 2;
  strncpy_s(tmpstr, TMPBUF_SIZE, str, 2 );
  tmpstr[2] = '\0';
  seconds = atoi(tmpstr);
  
  tmTemp.tm_year = atoi(year) - 1900;
  tmTemp.tm_mon = atoi(month) - 1;
  tmTemp.tm_mday = atoi(day);
  tmTemp.tm_hour = atoi(hour);
  tmTemp.tm_min = minutes;
  tmTemp.tm_sec = seconds;
  tmTemp.tm_isdst = -1; // let mktime figure out dst or not, if 0, then it assumes no DST
  
  unixsecs = mktime( &tmTemp );  // mktime assumes tmptr parameters are local time, so it returns seconds in local time
  _get_timezone(&tz);
  unixsecs -= tz;  // convert back into gmt
  localtime_s( &newtime, &unixsecs );
  
  // see if we are in dst
  secs = time(0);
  localtime_s( &new2time, &secs );
  //sprintf( timestr, "tm_isdst = %d time=%s\n", new2time.tm_isdst, VOTimeFmt(secs, "%Y-%m-%d %H:%M:%S" ) );

  if ( new2time.tm_isdst ){
	unixsecs -= dstbias;
  }

  if ( pflag ){
	sprintf_s( timestr, TMPBUF_SIZE, "time=%s isdst=%d dstbias=%d\n ", 
			VOTimeFmt( tmpstr, unixsecs, "%Y-%m-%d %H:%M:%S" ), new2time.tm_isdst,  dstbias  );
	_get_tzname(&len, tz0, TZNAME_MAX, 0);
	_get_tzname(&len, tz1, TZNAME_MAX, 1);
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) time=%s _tzname[0] = '%s' _tzname[1]=%s isdst=%d\n", 
		cdwThreadId, timestr, tz0, tz1, new2time.tm_isdst );
  }

  return( unixsecs );

}

time_t CalcPlannedFileTime( struct InstanceData* id, char *url )
{
	char	*sprtr = "/";
	struct tm tmTemp;
	char *str;
	char tokstr[256];
	char *year, *month, *day, *hour;
	char tmpstr[ TMPBUF_SIZE ], timestr[ TMPBUF_SIZE ];
	char tz0[TZNAME_MAX], tz1[TZNAME_MAX];
	time_t unixsecs, nowsecs, secs;
	long tz;
	int minutes, seconds, len;
	struct tm newtime, new2time;
	int daylighthours, dstbias, tzoffset;
	char *context;
	const DWORD cdwThreadId = GetCurrentThreadId();
	static int pflag = 0;

  // example https://207.252.77.135/data/asdex/ordF/2011/11/23/19/etas59.zip 
  // vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) url='%s'\n", cdwThreadId, url );
	_get_daylight( &daylighthours);
	_get_dstbias( &dstbias );
	_get_timezone( &tzoffset );
  //_get_tzname(&pReturnValue, timeZoneName, sizeof(timeZoneName), 0 );

	nowsecs = time(0);
	gmtime_s( &tmTemp, &nowsecs );

	/* ignore 'data/' in beginning */
	if ( !(str = strstr(url, "asdex/" )) ){
		return(0);
	}
    
	str += 11; // point to year
	strncpy_s( tokstr, _countof(tokstr), str, 255 );
	tokstr[255] = '\0';
  
	if ( !(year = strtok_s( tokstr, sprtr, &context ) )){
		return(0);
	}
	if ( !(month = strtok_s ( NULL, sprtr, &context ))){
		return(0);
	}
	if ( !(day = strtok_s ( NULL, sprtr, &context ))){
		return(0);
	}
	if ( !(hour = strtok_s ( NULL, sprtr, &context ))){
		return(0);
	}

	secs = time(0);
	localtime_s( &newtime, &secs );  // set dst

	/* get minutes and seconds */
	if ( !(str = strstr(url, ".zip" )) ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) could not find .zip in file, bad zipfilename\n", cdwThreadId);
		return(0);
	}

	str -= 2;
	strncpy_s(tmpstr, TMPBUF_SIZE, str, 2 );
	tmpstr[2] = '\0';
	minutes = atoi(tmpstr);
  
	seconds = 0;
  
	tmTemp.tm_year = atoi(year) - 1900;
	tmTemp.tm_mon = atoi(month) - 1;
	tmTemp.tm_mday = atoi(day);
	tmTemp.tm_hour = atoi(hour);
	tmTemp.tm_min = minutes;
	tmTemp.tm_sec = seconds;
	tmTemp.tm_isdst = -1; // let mktime figure out dst or not, if 0, then it assumes no DST
  
	unixsecs = mktime( &tmTemp );  // mktime assumes tmptr parameters are local time, so it returns seconds in local time
	_get_timezone(&tz);
	unixsecs -= tz;  // convert back into gmt
	localtime_s( &newtime, &unixsecs );
  
	// see if we are in dst
	secs = time(0);
	localtime_s( &new2time, &secs );
	//sprintf( timestr, "tm_isdst = %d time=%s\n", new2time->tm_isdst, VOTimeFmt(secs, "%Y-%m-%d %H:%M:%S" ) );

	if ( new2time.tm_isdst ){
		unixsecs -= dstbias;
	}

	if ( pflag ){
		sprintf_s( timestr, TMPBUF_SIZE, "time=%s isdst=%d dstbias=%d\n ", 
				VOTimeFmt( tmpstr, unixsecs, "%Y-%m-%d %H:%M:%S" ), new2time.tm_isdst,  dstbias  );
		_get_tzname(&len, tz0, TZNAME_MAX, 0);
		_get_tzname(&len, tz1, TZNAME_MAX, 1);
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) time=%s _tzname[0] = '%s' _tzname[1]=%s isdst=%d\n", 
			cdwThreadId, timestr, tz0, tz1, new2time.tm_isdst );
	}

	return( unixsecs );
}

int time_elapsed(struct timeb *t1, double ms)
{
	struct timeb now;
	double request_time;

	if (t1->time > 0) {
		ftime(&now);
		request_time = uts_timediff(t1, &now);
		if (request_time < ms) {
			return(FALSE);
		}
		memset(t1, 0, sizeof(struct timeb));
	}
	return SUCCEED;
}


static int GetASDHdrsZip( struct InstanceData* id, int CkLastTime, int force_refresh, CURL* curl_handle )
{
  char *str;
  time_t secs;
  struct MemoryStruct chunk;
  struct timeb start_time, done_time;
  int delaysecs;
  char zipfilename[256];
  char url[URL_SZ];
  time_t nowsecs;
  char tmpstr[ TMPBUF_SIZE ];
  struct tm tmTemp;
  int loopcount, minutes, hour;
  char *outbufptr = NULL;
  int replay_adjust_secs;
  int unzipbytes;
  const DWORD cdwThreadId = GetCurrentThreadId();
 
  secs = time(0);    

  loopcount = 0;
  do {
    zipfilename[0] = '\0';
    if ( id->m_nDelaySecs || !id->m_bClass1 ){
      if ( !id->m_bClass1 ){
        delaysecs = MAX(id->m_nDelaySecs, 300);
      } else {
        delaysecs = id->m_nDelaySecs;
      }
	  if ( id->m_nDelaySecs && id->m_nReplaySpeed ){
        replay_adjust_secs = (id->m_tClientsTime - delaystartsecs) * (id->m_nReplaySpeed - 1);
		delaysecs -= replay_adjust_secs;
        D_CONTROL("ASD_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) delaysecs reduced by %d\n", cdwThreadId, replay_adjust_secs ));
      }
      sprintf_s(tmpstr, TMPBUF_SIZE, "arpt=%s&replay=%d", "asd2", delaysecs );
    } else {
	  sprintf_s(tmpstr, TMPBUF_SIZE, "arpt=%s&ip=%s&u=%s", "asd2", id->m_strIPAddress, id->m_strCurluser );
    }
    if ( timeflag ){ftime( &start_time );}


    clear_chunk_memory(&chunk);

    vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlFormGetResults - GetAsdSrvr() <%s> tmpstr <%s>.\n", 
			cdwThreadId, GetAsdSrvr(id), tmpstr );

	sprintf_s(url, URL_SZ, "%s/cgi-bin/latest.cgi", GetAsdSrvr(id));

    if ( GetCurlFormGetResults_r( id, curl_handle, &chunk, url, tmpstr, SM_HTTPS, TRUE ) == TRUE )
      noteServerSuccess(id);
    else
      noteServerFailure(id);


	if ( timeflag ) {
      ftime( &done_time );
      MGLGridText(id, statusgrid, 5, 0, "asd www3");
      sprintf_s(tmpstr, TMPBUF_SIZE, "%g", uts_timediff(&start_time, &done_time ));
      MGLGridText(id, statusgrid, 5, 1, tmpstr);
    }
    if ( chunk.memory ){
      strncpy_s( zipfilename, _countof(zipfilename), chunk.memory, 255 );
      free( chunk.memory );
      D_CONTROL("ASD_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) found zipfilename='%s'\n", cdwThreadId, zipfilename ));
    } else {
      vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not get latest.cgi, trying again\n", cdwThreadId);
      sleep(1);
    }
  } while (!strlen(zipfilename) && loopcount++ < 5);
  if ( loopcount >= 5 ){
    vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not get latest.cgi, skipping, loopcount =%d\n", cdwThreadId, loopcount);
    return( FALSE );
  }

  nowsecs = time(0) - id->m_nDelaySecs;
  gmtime_s( &tmTemp, &nowsecs );

  /* ignore 'data/' in beginning */
  if ( (str = strstr(zipfilename, ".zip" )) ){
    str -= 4;
    strncpy_s(tmpstr, TMPBUF_SIZE, str, 2 );
    tmpstr[2] = '\0';
    minutes = atoi(tmpstr);
    str -= 3; // move to hour, always 2 digits
    strncpy_s(tmpstr, TMPBUF_SIZE, str, 2 );
    tmpstr[2] = '\0';
    hour = atoi(tmpstr);

    // must delay since summary file is behind
    minutes -= 2;
    if ( minutes < 0 ){
      minutes = 58;
      hour -= 1;
      if ( hour < 0 ){
        hour = 23;
        (tmTemp.tm_mday)--;
      }
    }
  } else {
    vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) could not find .zip in file, bad zipfilename\n", cdwThreadId);
    return( FALSE );
  }

  tmTemp.tm_hour = hour;
  tmTemp.tm_min = minutes;
  tmTemp.tm_sec = 0;

  //id->m_tASDFileTime = CalcFileTime( id, url );

  sprintf_s(zipfilename, _countof(zipfilename), "asd2/%d/%d/%d/%02d/sum%02d.zip", tmTemp.tm_year + 1900, tmTemp.tm_mon + 1, tmTemp.tm_mday,
        tmTemp.tm_hour, tmTemp.tm_min );
  sprintf_s(url, URL_SZ, "%s/sdata/%s", GetAsdSrvr(id), zipfilename );
  
  if ( timeflag ){ftime( &start_time );}

  clear_chunk_memory(&chunk);

        vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlPage - id <%x> url <%s>.\n", 
			cdwThreadId, id, url );

	if ( GetCurlPage_r( id, curl_handle, &chunk, url, SM_HTTPS, 3, 15L ) == FALSE || !chunk.size ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Curl failed for url '%s'\n", cdwThreadId, url);
		return(FALSE);
	}
    if ( timeflag ) {
      ftime( &done_time );
      MGLGridText(id, statusgrid, 7, 0, "ASD zipget");
      sprintf_s(tmpstr, TMPBUF_SIZE, "%g", uts_timediff(&start_time, &done_time ));
      MGLGridText(id, statusgrid, 7, 1, tmpstr);
    }

	//Win32:11-11-03
    //outfile = fopen("C:/FPWin/temp/data.zip", "wb" );

	// Final Test, make sure downloaded data is valid compressed data so imunzip does not loop forever
	if(chunk.size == 0 || chunk.memory[0] != 'P' || chunk.memory[1] != 'K'){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Invalid downloaded compressed data file for %s\n", cdwThreadId, url);
		free(chunk.memory);
		return (FALSE);
	}

	// use new memory unzip 
	if ( (unzipbytes = imunzip(zipfilename, chunk.memory, chunk.size, &outbufptr )) <= 0 ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) failed to unzip '%s'\n", cdwThreadId, url);
		return(FALSE);
	}
    if ( chunk.memory ){      
      free_chunk_memory(&chunk);
      if ( (0 == id->m_tLastMinuteASDStart) || (secs - id->m_tLastMinuteASDStart >= 60) ){
        // display last minute's download amount
        MGLGridText(id, statusgrid, 6, 0, "asd bpm");
        sprintf_s(tmpstr, TMPBUF_SIZE, "%d", id->m_nASDTotBytes);
        MGLGridText(id, statusgrid, 6, 1, tmpstr);
        id->m_nASDTotBytes = unzipbytes;
        id->m_tLastMinuteASDStart = secs;
      } else {
        id->m_nASDTotBytes += unzipbytes;
      }

    /* clear out buffer */
	clear_chunk_memory(&chunk);

	EnterCriticalSection(&id->m_csASDSync);

	append_rec_buffer(&id->m_pASDfilebuf, &id->m_nASDFileNBytes, outbufptr, unzipbytes);
	
	LeaveCriticalSection(&id->m_csASDSync);

	D_CONTROL("ASD_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) ASDI DATA:\n\n%s\n", cdwThreadId, outbufptr));
  
	free(outbufptr);
	
	return(SUCCEED);

    }

  return(FALSE);
}


static int GetASDData(  struct InstanceData* id, int CkLastTime, int force_refresh, CURL* curl_handle )
{
	time_t secs;
	struct MemoryStruct chunk;
	struct timeb start_time, done_time;
	int delaysecs, ntries;
	char zipfilename[256];
	char url[URL_SZ];
	char tmpstr[ TMPBUF_SIZE ];
	char targetname[256];
	static int sendmailcount;
	static int samecount;
	char *outbufptr = NULL;
	int replay_adjust_secs;
	int unzipbytes;
	DWORD dResult;
	const DWORD cdwThreadId = GetCurrentThreadId();

	if (!curl_handle) return FALSE;

	secs = time(0);

	// This code starts a thread to download ASD summary data for the last 5 minutes to preload all the flights
	if (id->m_bASDFirstLoad) {
		if (0 == id->m_hASDPreloadThread){
			id->m_hASDPreloadThread = _beginthreadex(NULL, 0, ASDPreloadThread, id, 0, NULL );
			RegisterThreadHandle(id, id->m_hASDPreloadThread, "ASDPreloadThread");
		} else {
			dResult = WaitForSingleObject((HANDLE)id->m_hASDPreloadThread, 1);
			if (dResult == WAIT_OBJECT_0) {
				UnregisterThreadHandle(id, id->m_hASDPreloadThread);
				CloseHandle((HANDLE)id->m_hASDPreloadThread);
				id->m_tLastASDThreadSecs = secs;
				id->m_hASDPreloadThread = 0;
				id->m_bASDFirstLoad = 0;
			}
		}
		return(SUCCEED);
	}	

	if (!time_elapsed(&(id->m_sASDReqStart), 1000.0)) return FALSE;

	//vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) plottime=%d\n", cdwThreadId, id->m_tPlotTime );

	// only if not replay mode, and 'fusedtest' user
	if ((id->m_nDelaySecs == 0) && (strcmp(id->m_strCurluser, "fusedtest") == 0)) {
		if (CkLastTime && secs - id->m_tLastASDThreadSecs < 1 ){
			// caller already sleeps for 1 second
			//SleepEx( 1000, FALSE); // be nice and don't hog cpu just waiting for loop
			return(SUCCEED);
		}
  		GetFusedData(curl_handle);
		// continue processing to set Gfiletime
	}
	if ( CkLastTime ){
		// see if we are in fast replay mode
		if ( id->m_nDelaySecs ){
			// we are in replay mode, use plottime to retrieve next file
			if ( id->m_tASDFileTime < id->m_tPlotTime ){
				// file is behind, get the next file
				D_CONTROL("ASD_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) file is behind, Gfiletime=%d plottime=%d\n", 
					cdwThreadId, id->m_tASDFileTime, id->m_tPlotTime ));
			} else if ( secs - id->m_tLastASDThreadSecs < 10 ){
				/* we already got it */
				if(id->m_sCurLayout.m_bShowASDAircraft){
					Callback2Javascript(id, "AsdiStatus:OK");
				}else{
					Callback2Javascript(id, "AsdiStatus:Off");
				}
				return(SUCCEED);
			}
		} else if ( secs - id->m_tLastASDThreadSecs < 10 ){
			/* we already got it */
			if(id->m_sCurLayout.m_bShowASDAircraft){
				Callback2Javascript(id, "AsdiStatus:OK");					
			}else{
				Callback2Javascript(id, "AsdiStatus:Off");
			}
			return(SUCCEED);
		}
	}
	id->m_tLastASDThreadSecs = secs;

	
	ntries = 0;
	do {
		zipfilename[0] = '\0';
		if ( id->m_nDelaySecs || !id->m_bClass1 ){
			if ( !id->m_bClass1 ){
				delaysecs = MAX(id->m_nDelaySecs, 300);
			} else {
				delaysecs = id->m_nDelaySecs;
			}
			if ( id->m_nDelaySecs && id->m_nReplaySpeed ){
				replay_adjust_secs = (id->m_tClientsTime - delaystartsecs) * (id->m_nReplaySpeed - 1);
				delaysecs -= replay_adjust_secs;
				D_CONTROL("ASD_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) delaysecs reduced by %d\n", cdwThreadId, replay_adjust_secs ));
			}
			sprintf_s(tmpstr, TMPBUF_SIZE, "arpt=%s&replay=%d", "asd2", delaysecs );
		} else {
			sprintf_s(tmpstr, TMPBUF_SIZE, "arpt=%s&ip=%s&u=%s", "asd2" , id->m_strIPAddress, id->m_strCurluser );
		}
		if ( timeflag ){ftime( &start_time );}
		chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
		chunk.size = 0;    /* no data at this point */
		setCurlTimeout(curl_handle, 4L);

		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlFormGetResults - GetAsdSrvr() <%s> tmpstr <%s>.\n", 
			cdwThreadId, GetAsdSrvr(id), tmpstr );
		sprintf_s(url, URL_SZ, "%s/cgi-bin/latest.cgi", GetAsdSrvr(id));

		if(	GetCurlFormGetResults_r( id, curl_handle, &chunk, url, tmpstr, SM_HTTPS, TRUE) )
		{
			NoteAsdServerSuccess(id);
			if ( timeflag ) {
				ftime( &done_time );
				MGLGridText(id, statusgrid, 5, 0, "asd www3");
				sprintf_s(tmpstr, TMPBUF_SIZE, "%g", uts_timediff(&start_time, &done_time ));
				MGLGridText(id, statusgrid, 5, 1, tmpstr);
			}
			if ( chunk.memory ){
				strncpy_s( zipfilename, _countof(zipfilename), chunk.memory, 255 );
				free( chunk.memory );
				chunk.memory = NULL;
				chunk.size = 0;
			} else {
				NoteAsdServerFailure(id);
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not get latest.cgi, trying again\n", cdwThreadId);
			}
		}else{
			NoteAsdServerFailure(id);
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not get latest.cgi, trying again\n", cdwThreadId);
		}
	} while (!strlen(zipfilename) && ntries++ < 3 );

	if ( ntries >= 3 ){
		/* failed to get file */
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) FAILED ntries =%d, could not open %s\n", cdwThreadId, ntries, tmpstr );

		// send email about the error, only if user is imaputest
		if ( !strcmp( id->m_sLogin.User, "imaputest" ) ){
			// send email alert
			if ( sendmailcount < 4 ){
				sendmailcount++;
				//SendMail("mattmarcella@passur.com", "mattmarcella@passur.com", "Insight Problem",
				//  "Failed to retrieve latest.cgi\n");
			}
		}
		return(FALSE);
	}

	/* clear out buffer */
	chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
	chunk.size = 0;    /* no data at this point */

	/* ignore 'data/' in beginning */
	strcpy_s(targetname, _countof(targetname), &zipfilename[5]);
	sprintf_s(url, URL_SZ, "%s/sdata/%s", GetAsdSrvr(id), targetname );
	//strcpy( LastZipName, targetname );

	if ( !strcmp( url, id->m_strPrevASDUrl ) ){
		/* we already got this file */
		D_CONTROL("ASD_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Already got this file(%s), skipping\n", cdwThreadId, url));
		ftime(&(id->m_sASDReqStart));
		if ( samecount++ > 10 ){
			if ( !strcmp( id->m_sLogin.User, "imaputest" ) ){
				// send email alert
				if ( sendmailcount < 4 ){
					sendmailcount++;
					//SendMail("mattmarcella@passur.com", "mattmarcella@passur.com", "Insight Problem",
					//  "Same file retrieved more than 10 times\n");
				}
			}
		}

		return( SUCCEED );
	}
	samecount = 0;
	strcpy_s(id->m_strPrevASDUrl, URL_SZ, url);
	sendmailcount = 0;  // found a good file, so reset email counter

	if ( !strncmp( id->m_strUserName, "sm", 2 ) ){  // take anything for now
		_snprintf_s( id->m_strLatestPassurURL, URL_SZ, _TRUNCATE, "ASD: %s" ,url );
	}
	if ( timeflag ){ftime( &start_time );}

	id->m_tASDFileTime = CalcFileTime( id, url );

	if ( GetCurlPage_r( id, curl_handle, &chunk, url, SM_HTTPS, 3, 4L) ){
		// make sure size if large enough, may be problem if zip wasn't done fast enough
		if ( chunk.size < 500 ){
			// try again
			free( chunk.memory );
			chunk.memory = NULL;
			vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) size smaller than expected for asd file, size=%d, try again\n",
				cdwThreadId, chunk.size );
			if ( !GetCurlPage_r( id, curl_handle, &chunk, url, SM_HTTPS, 3, 4L ) ){
				return( FALSE );
			}
		}

		if ( timeflag ) {
			ftime( &done_time );
			MGLGridText(id, statusgrid, 7, 0, "ASD zipget");
			sprintf_s(tmpstr, TMPBUF_SIZE, "%g", uts_timediff(&start_time, &done_time ));
			MGLGridText(id, statusgrid, 7, 1, tmpstr);
			MGLGridText(id, statusgrid, 23, 0, "asd name");
			MGLGridText(id, statusgrid, 23, 1, url);
		}

		// Final Test, make sure downloaded data is valid compressed data so imunzip does not loop forever
		if( chunk.memory && chunk.memory[0] != 'P' && chunk.memory[1] != 'K'){
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Invalid downloaded compressed data file for %s\n", cdwThreadId, url);
			free_chunk_memory(&chunk);
			return(FAIL);
		}

		// use new memory unzip 
		if ( (unzipbytes = imunzip(zipfilename, chunk.memory, chunk.size, &outbufptr )) <= 0 ){
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) failed to unzip '%s'\n", cdwThreadId, url);
			free_chunk_memory(&chunk);
			return(FAIL);
		}
		if ( chunk.memory ){
			if ( (0 == id->m_tLastMinuteASDStart) || (secs - id->m_tLastMinuteASDStart >= 60) ){
				// display last minute's download amount
				MGLGridText(id, statusgrid, 6, 0, "asd bpm");
				sprintf_s(tmpstr, TMPBUF_SIZE, "%d", id->m_nASDTotBytes);
				MGLGridText(id, statusgrid, 6, 1, tmpstr);
				id->m_nASDTotBytes = unzipbytes;
				id->m_tLastMinuteASDStart = secs;
			} else {
				id->m_nASDTotBytes += unzipbytes;
			}
		}
		/* clear out buffer */
		free_chunk_memory(&chunk);

		
		if ( id->m_bDHSPerm ){
			// only dhs allowed to see type 20 records
			RetrieveTails(id);
		}

		EnterCriticalSection(&id->m_csASDSync);

		append_rec_buffer(&id->m_pASDfilebuf, &id->m_nASDFileNBytes, outbufptr, unzipbytes);
		
		LeaveCriticalSection(&id->m_csASDSync);

		D_CONTROL("ASD_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) ASDI DATA:\n\n%s\nASDI END\n", cdwThreadId, outbufptr));

		free(outbufptr);
		if(id->m_sCurLayout.m_bShowASDAircraft){
			Callback2Javascript(id, "AsdiStatus:OK");
		}else{
			Callback2Javascript(id, "AsdiStatus:Off");
		}

		return( SUCCEED );
	}

	if(id->m_sCurLayout.m_bShowASDAircraft){
		Callback2Javascript(id, "AsdiStatus:FAIL");
	}else{
		Callback2Javascript(id, "AsdiStatus:Off");
	}

	return( FAIL );
}


static int get_latest_name(struct InstanceData* id, int passurindex, char* tmpstr, CURL* curl_handle)
{
	int LoopMax = 20;
	int loopcount;
	int delaysecs;
	int replay_adjust_secs;
	char buffer[SERVERNAME_SZ] = {0};
	struct MemoryStruct chunk;
	const DWORD cdwThreadId = GetCurrentThreadId();

	loopcount = 0;
	do {
		id->m_strPassurZipFilename[passurindex][0] = '\0';
		//vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) loopcount=%d\n", cdwThreadId, loopcount);
		if ( id->m_nDelaySecs || !id->m_bClass1 ){
			if ( !id->m_bClass1 ){
				delaysecs = MAX(id->m_nDelaySecs, 300);
			} else {
				delaysecs = id->m_nDelaySecs;
			}
			if ( id->m_nDelaySecs && id->m_nReplaySpeed && delaystartsecs){
				replay_adjust_secs = (int) (id->m_tClientsTime - delaystartsecs) *  (id->m_nReplaySpeed - 1);
				delaysecs -= replay_adjust_secs;
				D_CONTROL("PASSUR_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) delaysecs reduced by %d\n", cdwThreadId, replay_adjust_secs ));
			}
			if ( delaysecs < 0 || (delaysecs < 300 && !id->m_bClass1) ){
				/* we already got it */
				D_CONTROL("PASSUR_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) delaysecs =%d, in future, replay past current time\n",
					cdwThreadId, delaysecs ));
				return(1);
			}
			sprintf_s(tmpstr, TMPBUF_SIZE, "arpt=%s&replay=%d", id->m_strPassurArpts[passurindex], delaysecs );
		} else {
			sprintf_s(tmpstr, TMPBUF_SIZE, "arpt=%s", id->m_strPassurArpts[passurindex] );
		}

		clear_chunk_memory(&chunk);
		setCurlTimeout(curl_handle, 4L);

		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlFormGetResults - GetPassurSrvr() <%s> tmpstr <%s>.\n", 
			cdwThreadId, GetPassurSrvr(id), tmpstr );
		sprintf_s(buffer, SERVERNAME_SZ, "https://%s/cgi-bin/latest.cgi", GetPassurSrvr(id));

		if ( GetCurlFormGetResults_r( id, curl_handle, &chunk,
				buffer, tmpstr, SM_HTTPS, TRUE ) == TRUE )
			NotePassurServerSuccess(id);
		else
			NotePassurServerFailure(id);
		//if ( timeflag ) {
			//ftime( &done_time );
			//MGLGridText(id, statusgrid, 8, 0, "passur");
			//sprintf(tmpstr, "%g", uts_timediff(&start_time, &done_time ));
			//MGLGridText(id, statusgrid, 8, 1, tmpstr);
		//}
		if ( chunk.memory ){
			strncpy_s( id->m_strPassurZipFilename[passurindex], FILENAME_SZ, 
               chunk.memory,
               sizeof(id->m_strPassurZipFilename[passurindex])/sizeof(id->m_strPassurZipFilename[passurindex][0]) );
			id->m_strPassurZipFilename[passurindex][sizeof(id->m_strPassurZipFilename[passurindex])-1] = '\0';
			free_chunk_memory(&chunk);
		} else {
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not get latest.cgi, trying again\n", cdwThreadId);
		}
	} while (!strlen(id->m_strPassurZipFilename[passurindex]) && loopcount++ < LoopMax );

	if ( loopcount >= LoopMax ){
		noteServerFailure(id);
		return(-1);
	}

	return 0;
}


int GetBeaconData(  struct InstanceData* id,  CURL *curl_handle, char *url, char **Passurfilebuf, long *PassurFileNBytes, int checkstatus )
{
  struct MemoryStruct chunk;
  char *str;
  int len;
  char beaconurl[ 256 ];
  const DWORD cdwThreadId = GetCurrentThreadId();
  static int errorcount;
  static char beaconairport[5];
  static int beaconcode_status = TRUE;

  vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) url <%s>\n", cdwThreadId, url);

  if ( checkstatus && beaconcode_status == FALSE && !strcasecmp( beaconairport, id->m_strPassurArpts[0] ) ){
    // no beacon code data for this airport
    return( FALSE );
  }

  if (!url /*|| !(*Passurfilebuf)*/ ){
    vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Bad parameters= NULL\n", cdwThreadId);
    return( FALSE );
  }

  vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Get beacon data for '%s'\n", cdwThreadId, url );

  if ( (str = strstr(url, ".zip") ) ){
    len = str - url;
    if ( len > 256 ){
      vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) url too large '%s'\n", cdwThreadId, url );
      return( FALSE );
    }
    strncpy_s( beaconurl, _countof(beaconurl), url, len );
    beaconurl[len] = '\0';
    strcat_s( beaconurl, _countof(beaconurl), "B.txt" );
  } else {
    // bad filename
	vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) bad filename\n", cdwThreadId);
    return( FALSE );
  }

  clear_chunk_memory(&chunk);
  if ( GetCurlPage_r( id, curl_handle, &chunk, beaconurl, SM_HTTPS, 1, 4L ) ){
    if ( !chunk.size || !chunk.memory ){
      vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Error, null chunk size\n", cdwThreadId );
      return( FALSE );
    }
    if ( strstr( chunk.memory, "404 Not Found" ) ||
       strstr( chunk.memory, "no data" ) ){
      // no data found, so skip
      errorcount++;
      if ( errorcount > 20 ){
        // give up , no beacon code data for this airport
        beaconcode_status = FALSE;
        strcpy_s( beaconairport, _countof(beaconairport), id->m_strPassurArpts[0] );
      }

      free_chunk_memory(&chunk);
	  return( FALSE );
    } else {
      // reset error counter
      errorcount = 0;
      beaconcode_status = TRUE;
    }

	append_rec_buffer(Passurfilebuf, PassurFileNBytes, chunk.memory, chunk.size);
	
	free_chunk_memory(&chunk);
  } else {
    vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Error, Curl failed\n", cdwThreadId);
  }

  return( TRUE );
}


static int unzip_and_save_asdex_data(struct InstanceData* id, struct MemoryStruct* chunk, char* url)
{
	char tname[256];
	char* str;
	int i, c, lastslash;
	char *outbufptr = NULL;
	long unziplen;
	const DWORD cdwThreadId = GetCurrentThreadId();

	str = id->m_strAsdexZipFilename;

	i = 0;
	lastslash = -1;
	while ( (c = *str++) != '\0' ){
		if ( c == '/' ){ lastslash = i; }
		i++;
	}
	if ( lastslash == -1 ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not create textfilename\n", cdwThreadId);
		return( FALSE );
	}
	strncpy_s(tname, _countof(tname), &id->m_strAsdexZipFilename[lastslash+1], _TRUNCATE );

	// Test, make sure downloaded data is valid compressed data so imunzip does not loop forever
	if(chunk->memory[0] != 'P' && chunk->memory[1] != 'K'){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Invalid downloaded compressed data file for %s\n", cdwThreadId, url);
		free_chunk_memory(chunk);
		return (FALSE);
	}

	if ( (unziplen = imunzip( tname, chunk->memory, chunk->size, &outbufptr )) <= 0 ){
		if(unziplen == -2){
			strcpy_s(id->m_strPrevASDEXUrl, URL_SZ, url);
			D_CONTROL("ASDEX_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) No ASDE-X data available for %s\n", cdwThreadId, url ));
		}else{
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) imunzip failed for %s\n", cdwThreadId, url );
		}
		free_chunk_memory(chunk);
		return( FALSE );
	}
	free_chunk_memory(chunk);

	strcpy_s(id->m_strPrevASDEXUrl, URL_SZ, url);

	EnterCriticalSection(&id->m_csASDEXSync);

	append_rec_buffer(&id->m_pASDEXFileBuf, &id->m_lASDEXFileNBytes, outbufptr, unziplen);
	
	//vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) ASDEXFileNBytes=%d url=%s\n", id->m_lASDEXFileNBytes, url );

	LeaveCriticalSection(&id->m_csASDEXSync);

	D_CONTROL("ASDEX_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) ASDE-X DATA:\n\n%s\n", cdwThreadId, outbufptr));

	free( outbufptr );

	return( SUCCEED );
}

static int unzip_and_save_lmg_data(struct InstanceData* id, struct MemoryStruct* chunk, char* url)
{
	char tname[256];
	char* str;
	int i, c, lastslash;
	char *outbufptr = NULL;
	long unziplen;
	const DWORD cdwThreadId = GetCurrentThreadId();

	str = id->m_strLMGZipFilename;

	i = 0;
	lastslash = -1;
	while ( (c = *str++) != '\0' ){
		if ( c == '/' ){ lastslash = i; }
		i++;
	}
	if ( lastslash == -1 ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not create textfilename\n", cdwThreadId);
		return( FALSE );
	}
	strncpy_s(tname, _countof(tname), &id->m_strLMGZipFilename[lastslash+1], _TRUNCATE );

	// Test, make sure downloaded data is valid compressed data so imunzip does not loop forever
	if(chunk->memory[0] != 'P' && chunk->memory[1] != 'K'){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Invalid downloaded compressed data file for %s\n", cdwThreadId, url);
		free_chunk_memory(chunk);
		return (FALSE);
	}

	if ( (unziplen = imunzip( tname, chunk->memory, chunk->size, &outbufptr )) <= 0 ){
		if(unziplen == -2){
			strcpy_s(id->m_strPrevLMGUrl, URL_SZ, url);
			D_CONTROL("LMG_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) No LMG data available for %s\n", cdwThreadId, url ));
		}else{
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) imunzip failed for %s\n", cdwThreadId, url );
		}
		free_chunk_memory(chunk);
		return( FALSE );
	}
	free_chunk_memory(chunk);

	strcpy_s(id->m_strPrevLMGUrl, URL_SZ, url);

	EnterCriticalSection(&id->m_csLMGSync);

	append_rec_buffer(&id->m_pLMGFileBuf, &id->m_lLMGFileNBytes, outbufptr, unziplen);
	
	LeaveCriticalSection(&id->m_csLMGSync);

	D_CONTROL("LMG_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) LMG DATA:\n\n%s\n", cdwThreadId, outbufptr));

	free( outbufptr );

	return( SUCCEED );
}


static int unzip_and_save_adsb_data(struct InstanceData* id, struct MemoryStruct* chunk, char* url)
{
	char tname[256];
	char* str;
	int i, c, lastslash;
	char *outbufptr = NULL;
	long unziplen;
	const DWORD cdwThreadId = GetCurrentThreadId();

	str = id->m_strAdsbZipFilename;

	i = 0;
	lastslash = -1;
	while ( (c = *str++) != '\0' ){
		if ( c == '/' ){ lastslash = i; }
		i++;
	}
	if ( lastslash == -1 ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not create textfilename\n", cdwThreadId);
		return( FALSE );
	}
	strncpy_s(tname, _countof(tname), &id->m_strAdsbZipFilename[lastslash+1], _TRUNCATE );

	// Test, make sure downloaded data is valid compressed data so imunzip does not loop forever
	if(chunk->memory[0] != 'P' && chunk->memory[1] != 'K'){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Invalid downloaded compressed data file for %s\n", cdwThreadId, url);
		free_chunk_memory(chunk);
		return (FALSE);
	}

	if ( (unziplen = imunzip( tname, chunk->memory, chunk->size, &outbufptr )) <= 0 ){
		if(unziplen == -2){
			strcpy_s(id->m_strPrevADSBUrl, URL_SZ, url);
			D_CONTROL("ADSB_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) No ADS-B data available for %s\n", cdwThreadId, url ));
		}else{
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) imunzip failed for %s\n", cdwThreadId, url );
		}
		free_chunk_memory(chunk);
		return( FALSE );
	}
	free_chunk_memory(chunk);

	strcpy_s(id->m_strPrevADSBUrl, URL_SZ, url);

	EnterCriticalSection(&id->m_csADSBSync);

	append_rec_buffer(&id->m_pADSBFileBuf, &id->m_lADSBFileNBytes, outbufptr, unziplen);
	
	//vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) ADSBFileNBytes=%d url=%s\n", id->m_lADSBFileNBytes, url );

	LeaveCriticalSection(&id->m_csADSBSync);

	D_CONTROL("ADSB_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) ADS-B DATA:\n\n%s\n", cdwThreadId, outbufptr));

	free( outbufptr );

	return( SUCCEED );
}

static int unzip_and_save_airasia_data(struct InstanceData* id, struct MemoryStruct* chunk, char* url)
{
	char tname[256];
	char* str;
	int i, c, lastslash;
	char *outbufptr = NULL;
	long unziplen;
	const DWORD cdwThreadId = GetCurrentThreadId();

	str = id->m_strAirAsiaZipFilename;

	i = 0;
	lastslash = -1;
	while ( (c = *str++) != '\0' ){
		if ( c == '/' ){ lastslash = i; }
		i++;
	}
	if ( lastslash == -1 ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not create textfilename\n", cdwThreadId);
		return( FALSE );
	}
	strncpy_s(tname, _countof(tname), &id->m_strAirAsiaZipFilename[lastslash+1], _TRUNCATE );

	// Test, make sure downloaded data is valid compressed data so imunzip does not loop forever
	if(chunk->memory[0] != 'P' && chunk->memory[1] != 'K'){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Invalid downloaded compressed data file for %s\n", cdwThreadId, url);
		free_chunk_memory(chunk);
		return (FALSE);
	}

	if ( (unziplen = imunzip( tname, chunk->memory, chunk->size, &outbufptr )) <= 0 ){
		if(unziplen == -2){
			strcpy_s(id->m_strPrevAirAsiaUrl, URL_SZ, url);
			D_CONTROL("AIRASIA_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) No AirAsia data available for %s\n", cdwThreadId, url ));
		}else{
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) imunzip failed for %s\n", cdwThreadId, url );
		}
		free_chunk_memory(chunk);
		return( FALSE );
	}
	free_chunk_memory(chunk);

	strcpy_s(id->m_strPrevAirAsiaUrl, URL_SZ, url);

	EnterCriticalSection(&id->m_csAirAsiaSync);

	append_rec_buffer(&id->m_pAirAsiaFileBuf, &id->m_lAirAsiaFileNBytes, outbufptr, unziplen);
	
	D_CONTROL("AIRASIA_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) AirAsiaFileNBytes=%ld url=%s\n", cdwThreadId, id->m_lAirAsiaFileNBytes, url ));

	LeaveCriticalSection(&id->m_csAirAsiaSync);

	D_CONTROL("AIRASIA_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) AirAsia DATA:\n\n%s\n", cdwThreadId, outbufptr));

	free( outbufptr );

	return( SUCCEED );
}

static int unzip_and_save_mlat_data(struct InstanceData* id, struct MemoryStruct* chunk, char* url)
{
	char tname[256];
	char* str;
	int i, c, lastslash;
	char *outbufptr = NULL;
	long unziplen;
	const DWORD cdwThreadId = GetCurrentThreadId();

	str = id->m_strMLATZipFilename;

	i = 0;
	lastslash = -1;
	while ( (c = *str++) != '\0' ){
		if ( c == '/' ){ lastslash = i; }
		i++;
	}
	if ( lastslash == -1 ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not create textfilename\n", cdwThreadId);
		return( FALSE );
	}
	strncpy_s(tname, _countof(tname), &id->m_strMLATZipFilename[lastslash+1], _TRUNCATE );

	// Test, make sure downloaded data is valid compressed data so imunzip does not loop forever
	if(chunk->memory[0] != 'P' && chunk->memory[1] != 'K'){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Invalid downloaded compressed data file for %s\n", cdwThreadId, url);
		free_chunk_memory(chunk);
		return (FALSE);
	}

	if ( (unziplen = imunzip( tname, chunk->memory, chunk->size, &outbufptr )) <= 0 ){
		if(unziplen == -2){
			strcpy_s(id->m_strPrevMLATUrl, URL_SZ, url);
			D_CONTROL("MLAT_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) No ADS-B data available for %s\n", cdwThreadId, url ));
		}else{
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) imunzip failed for %s\n", cdwThreadId, url );
		}
		free_chunk_memory(chunk);
		return( FALSE );
	}
	free_chunk_memory(chunk);

	strcpy_s(id->m_strPrevMLATUrl, URL_SZ, url);

	EnterCriticalSection(&id->m_csMLATSync);

	append_rec_buffer(&id->m_pMLATFileBuf, &id->m_lMLATFileNBytes, outbufptr, unziplen);
	
	//vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) ADSBFileNBytes=%d url=%s\n", id->m_lADSBFileNBytes, url );

	LeaveCriticalSection(&id->m_csMLATSync);

	D_CONTROL("MLAT_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) M-LAT DATA:\n\n%s\n", cdwThreadId, outbufptr));

	free( outbufptr );

	return( SUCCEED );
}

static int unzip_and_save_noise_data(struct InstanceData* id, struct MemoryStruct* chunk, char* url)
{
	char tname[256];
	char* str;
	int i, c, lastslash;
	char *outbufptr = NULL;
	long unziplen;
	const DWORD cdwThreadId = GetCurrentThreadId();

	str = id->m_strNoiseZipFilename;

	i = 0;
	lastslash = -1;
	while ( (c = *str++) != '\0' ){
		if ( c == '/' ){ lastslash = i; }
		i++;
	}
	if ( lastslash == -1 ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not create textfilename\n", cdwThreadId);
		return( FALSE );
	}
	strncpy_s(tname, _countof(tname), &id->m_strNoiseZipFilename[lastslash+1], _TRUNCATE );

	// Test, make sure downloaded data is valid compressed data so imunzip does not loop forever
	if(chunk->memory[0] != 'P' && chunk->memory[1] != 'K'){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Invalid downloaded compressed data file for %s\n", cdwThreadId, url);
		free_chunk_memory(chunk);
		return (FALSE);
	}

	if ( (unziplen = imunzip( tname, chunk->memory, chunk->size, &outbufptr )) <= 0 ){
		if(unziplen == -2){
			strcpy_s(id->m_strPrevNoiseUrl, URL_SZ, url);
			D_CONTROL("ADSB_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) No Noise data available for %s\n", cdwThreadId, url ));
		}else{
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) imunzip failed for %s\n", cdwThreadId, url );
		}
		free_chunk_memory(chunk);
		return( FALSE );
	}
	free_chunk_memory(chunk);

	strcpy_s(id->m_strPrevNoiseUrl, URL_SZ, url);

	EnterCriticalSection(&id->m_csNoiseSync);

	append_rec_buffer(&id->m_pNoiseFileBuf, &id->m_lNoiseFileNBytes, outbufptr, unziplen);
	
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) NoiseFileNBytes=%d url=%s\n", id->m_lNoiseFileNBytes, url );

	LeaveCriticalSection(&id->m_csNoiseSync);

	D_CONTROL("NOISE_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) NOISE DATA:\n\n%s\n", cdwThreadId, outbufptr));

	free( outbufptr );

	return( SUCCEED );
}


static int unzip_and_save_passur_data(struct InstanceData* id, int passurindex, char* cur_arpt, CURL* curl_handle, struct MemoryStruct* chunk, char* url)
{
	char tname[256];
	char* str;
	int i, c, lastslash;
	char *Passurfilebuf;
	int unziplen;
	struct timeb start_time, end_time;
	const DWORD cdwThreadId = GetCurrentThreadId();

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) url <%s>\n", cdwThreadId, url);

	str = id->m_strPassurZipFilename[passurindex];

	i = 0;
	lastslash = -1;
	while ( (c = *str++) != '\0' ){
		if ( c == '/' ){ lastslash = i; }
		i++;
	}
	if ( lastslash == -1 ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not create textfilename\n", cdwThreadId);
		return( FALSE );
	}
	strncpy_s(tname, _countof(tname), &id->m_strPassurZipFilename[passurindex][lastslash+1], _TRUNCATE );

	// Final Test, make sure downloaded data is valid compressed data so imunzip does not loop forever
	if(chunk->memory[0] != 'P' && chunk->memory[1] != 'K'){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Invalid downloaded compressed data file for %s\n", cdwThreadId, url);
		free_chunk_memory(chunk);
		return( FALSE );
	}

	Passurfilebuf = NULL;
	if ((unziplen = imunzip(tname, chunk->memory, chunk->size, &Passurfilebuf)) <= 0) {
		////Debug to dump contents of failed imunzip
		if (IS_D_CONTROL("DBG_DMP_ZIP")){
			FILE* f = NULL;
			char filename[TMPBUF_SIZE] = { 0 };
			sprintf_s(filename, TMPBUF_SIZE, "%s\\logs\\test.zip", id->m_strPassurOutPath);
			if (!fopen_s(&f, filename, "wb")){
				vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) writing chunk.size=%d\n", cdwThreadId, chunk->size);
				fwrite(chunk->memory, 1, chunk->size, f);
				fclose(f);
			}
		}		
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) imunzip failed for %s\n", cdwThreadId, url);
		free_chunk_memory(chunk);
		unziplen = 0;
		// Removed FALSE Return to allow fall thru to always get Beacon data
		//return( FALSE );
	}

	free_chunk_memory(chunk);

	// append beacon data to outbufptr
	if ( id->m_bClass1 && curl_handle ) {
		ftime(&start_time);
		// third parameter is checkstatus. Set to FALSE since it only checks for main passur
		// When FALSE, program should ignore errors, OK for DHS application
		GetBeaconData( id, curl_handle, url, &Passurfilebuf, &unziplen, FALSE );
		ftime(&end_time);
		D_CONTROL("PASSUR_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) %gms GetBeaconData\n", 
			cdwThreadId, uts_timediff(&start_time, &end_time)));
	}

	EnterCriticalSection(&id->m_csPassurSync);

	// Handle when PASSUR Airport changes after we started download.
	if(strcmp(cur_arpt, id->m_strPassurArpts[passurindex])){
		free(Passurfilebuf);
		Passurfilebuf = NULL;
		unziplen = 0;
		if (id->m_pPassurfilebufs[passurindex]) {
			free(id->m_pPassurfilebufs[passurindex]);
			id->m_pPassurfilebufs[passurindex] = NULL;
		}
		LeaveCriticalSection(&id->m_csPassurSync);
		return(SUCCEED);
	}	

	append_rec_buffer(&(id->m_pPassurfilebufs[passurindex]), &(id->m_lPassurFileNBytes[passurindex]), Passurfilebuf, unziplen);

	LeaveCriticalSection(&id->m_csPassurSync);

	D_CONTROL("PASSUR_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) PASSUR DATA:\n\n%s\n", cdwThreadId, Passurfilebuf));

	free(Passurfilebuf);

	return(SUCCEED);
}


static void find_filename_in_hdrs(struct MemoryStruct hdrs, char* filename)
{
	char* str;
	char* end;
	int len;

	filename[0] = '\0';
	str = strstr(hdrs.memory, "Content-Disposition: ");
	if (!str) return;
	while (*str && *str != '=' && *str != '\n') str++;
	if (*str != '=') return;
	str++;
	end = strchr(str, '\n');
	if (NULL == end) return;
	end--;
	if (*end == '\r') end--;
	len = end-str+1;
	memcpy(filename, str, len);
	filename[len] = '\0';
}


static void find_filepath_in_hdrs(struct MemoryStruct hdrs, char* filepath)
{
	char* str;
	char* end;
	int len;

	filepath[0] = '\0';
	str = strstr(hdrs.memory, "Content-Location: ");
	if (!str) return;
	str += strlen("Content-Location: ");
	end = strchr(str, '\n');
	if (NULL == end) return;
	end--;
	if (*end == '\r') end--;
	len = end-str+1;
	memcpy(filepath, str, len);
	filepath[len] = '\0';
}

static void save_request_time(double request_time, double* PrevRequest, double* RequestTimes, int* RequestIndex, int RequestNum)
{
	if (request_time < 2 * (*PrevRequest)) {
		// keep time if it's not a one-time huge spike
		RequestTimes[*RequestIndex] = request_time / RequestNum; // divide now so we only have to sum later
		*RequestIndex = (*RequestIndex + 1) % RequestNum;
	}
	*PrevRequest = request_time;
}

static double get_average_request_time(double* RequestTimes, int RequestNum)
{
	int i;
	double request_average;
	request_average = 0.0;
	for (i = 0; i < RequestNum; i++) {
		request_average += RequestTimes[i];
	}
	return request_average;
}

static int GetPassurDataFastUpdates( struct InstanceData* id, int force_refresh, int passurindex, CURL* curl_handle )
{
	struct MemoryStruct chunk;
	struct MemoryStruct hdrs;
	char url[URL_SZ];
	char tmpstr[ TMPBUF_SIZE ];
	char buffer[ SERVERNAME_SZ ] = {0};
	char *outbufptr = NULL;
	char prevfile[256];
	char cur_arpt[5] = {0};	
	struct timeb curl_start_time, curl_end_time;
	double request_time, request_average;
	const DWORD cdwThreadId = GetCurrentThreadId();

	if (!curl_handle) return FALSE;

	if (!time_elapsed(&(id->m_sPassurReqStart), 1000.0)) return FALSE;

	id->m_tClientsTime = time(0);

	EnterCriticalSection(&id->m_csPassurSync);

	// Save current airport
	strcpy_s(cur_arpt, _countof(cur_arpt), id->m_strPassurArpts[passurindex]);

	sprintf_s(tmpstr, TMPBUF_SIZE, "arpt=%s", id->m_strPassurArpts[passurindex]);

	if (id->m_strPassurPrevFile[passurindex][0] != '\0') {
		strcat_s(tmpstr, TMPBUF_SIZE, "&prevfile=");
		strcat_s(tmpstr, TMPBUF_SIZE,id->m_strPassurPrevFile[passurindex]);
	}

	LeaveCriticalSection(&id->m_csPassurSync);

	ftime( &curl_start_time);
	
	/* clear out buffer */
	clear_chunk_memory(&chunk);
	clear_chunk_memory(&hdrs);

	D_CONTROL("PASSUR_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) GetPassurSrvr() <%s>.\n", cdwThreadId,  GetPassurSrvr(id)));
	setCurlTimeout(curl_handle, 4L);

    vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlFormGetResultsHdrs_r - GetPassurSrvr() <%s> tmpstr <%s>.\n", 
		cdwThreadId, GetPassurSrvr(id), tmpstr );
	sprintf_s(buffer, SERVERNAME_SZ, "https://%s/fcgi/fastupdates.fcg", GetPassurSrvr(id));

	if ( GetCurlFormGetResultsHdrs_r(id, curl_handle, &chunk, &hdrs,
		buffer, tmpstr, SM_HTTPS, TRUE ) == TRUE ) {
		NotePassurServerSuccess(id);
	} else {
		NotePassurServerFailure(id);
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) failed tmpstr=%s\n", cdwThreadId, tmpstr);
		free_chunk_memory(&chunk);
		free_chunk_memory(&hdrs);
		return( FALSE );
	} 

	ftime( &curl_end_time );

	// save the request time for a moving average
	request_time = uts_timediff(&curl_start_time, &curl_end_time);
	save_request_time(request_time, &(id->m_dPassurPrevRequest), id->m_dPassurRequestTimes,
		&(id->m_nPassurRequestIndex), id->m_nPassurRequestNum);
	// check the average request time
	request_average = get_average_request_time(id->m_dPassurRequestTimes, id->m_nPassurRequestNum);
	if (request_average > 600.0) {
		// internet connection too slow for fast updates
		id->m_bFastUpdates = FALSE;
		//Callback2Javascript(id, "FastUpdates:OFF");
		//memset(id->m_dPassurRequestTimes, 0, sizeof(id->m_dPassurRequestTimes));
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) FastUpdates disabled (slow connection)\n", cdwThreadId); 
	}
	D_CONTROL("PASSUR_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) time %2gms avg %2gms for %s?%s\n",
		cdwThreadId, request_time, request_average, GetPassurSrvr(id), tmpstr));

	if (!hdrs.size){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) failed tmpstr=%s, empty header returned\n", cdwThreadId, tmpstr);
		return(FALSE);
	}
	find_filename_in_hdrs(hdrs, prevfile);
	find_filepath_in_hdrs(hdrs, id->m_strPassurZipFilename[passurindex]);

	if (prevfile[0] == '\0') {
		// filename not in http headers - request failed
		// retry in 1 second
		free_chunk_memory(&hdrs);
		free_chunk_memory(&chunk);
		id->m_sPassurReqStart = curl_end_time;
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) failed tmpstr=%s, filename not in headers\n", cdwThreadId, tmpstr);
		return(FALSE); 
	}

	free_chunk_memory(&hdrs);
	
	EnterCriticalSection(&id->m_csPassurSync);
	// if airport didn't change, save prevfile for next time
	if (strcmp(cur_arpt, id->m_strPassurArpts[passurindex]) == 0) {
		strcpy_s(id->m_strPassurPrevFile[passurindex], FILENAME_SZ, prevfile);
	}
	LeaveCriticalSection(&id->m_csPassurSync);

	sprintf_s(url, URL_SZ, "https://%s/sdata/%s", GetPassurSrvr(id), id->m_strPassurZipFilename[passurindex]+6); // for retrieving beacon data

	if ( !strncmp( id->m_strUserName, "sm", 2 ) ){  // take anything for now
		if('\x0A' == url[strlen(url)-1])url[strlen(url)-1] = 0; // removes newline		
		_snprintf_s( id->m_strLatestPassurURL, URL_SZ, _TRUNCATE, "PASSUR: %s", url );
		//vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) got  %s\n", cdwThreadId, url);
	}

	if ( !chunk.size || !chunk.memory ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Error, null chunk size\n", cdwThreadId );
		return( FALSE );
	}

	unzip_and_save_passur_data(id, passurindex, cur_arpt, curl_handle, &chunk, url);

    return( SUCCEED );
}


static int GetPassurData( struct InstanceData* id, int force_refresh, int passurindex, CURL* curl_handle )
{
	time_t secs;
	struct MemoryStruct chunk;
	int CkLastTime = TRUE; // always check time interval for passur data
	int i;
	static char prevurl[MAX_PASSURS][URL_SZ];
	char targetname[256];
	char url[URL_SZ];
	char tmpstr[ TMPBUF_SIZE ];
	static int reccount, ecount;
	char cur_arpt[5] = {0};
	struct timeb curl_start_time, curl_end_time;
	double request_time, asdex_request_average, passur_request_average;
	const DWORD cdwThreadId = GetCurrentThreadId();

	if(!id->m_sCurLayout.m_bShowPassurAircraft) {
		memset(id->m_dPassurRequestTimes, 0, sizeof(id->m_dPassurRequestTimes)); // clear request timings
		Callback2Javascript(id, "PassurStatus:Off");
		return(SUCCEED);
	}

	if (!curl_handle) return FALSE;

	if ( !strncmp( id->m_strUserName, "sm", 2 ) ){  // take anything for now
		id->m_bClass1 = TRUE;
	}

	if (id->m_bFastUpdates && id->m_bClass1 && !id->m_nDelaySecs) {
		return GetPassurDataFastUpdates(id, force_refresh, passurindex, curl_handle);
	}

	secs = time(0);

	id->m_tClientsTime = secs;	
	
	// Save current airport
	strcpy_s(cur_arpt, _countof(cur_arpt), id->m_strPassurArpts[passurindex]);

	if ( CkLastTime ){
		// see if we are in fast replay mode
		if ( id->m_nDelaySecs ){
			// we are in replay mode, use Plottime to retrieve next file
			if ( id->m_tPASSURFileTime < id->m_tPlotTime ){
				// file is behind, get the next file
				D_CONTROL("PASSUR_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) file is behind, Gfiletime=%d plottime=%d\n", 
					cdwThreadId, id->m_tPASSURFileTime, id->m_tPlotTime ));
			} else if ( secs - id->m_tLastPASSURGetTime[passurindex] < 5 ){
				/* we already got it */
				if(id->m_bClass1)
					Callback2Javascript(id, "PassurStatus:Slow");
				else
					Callback2Javascript(id, "PassurStatus:OK");
				return(SUCCEED);
			}
		} else if ( secs - id->m_tLastPASSURGetTime[passurindex] < 5 ){  // new passur data every 5 seconds -- one per thread
			/* we already got it */
			Callback2Javascript(id, "PassurStatus:Slow");
			return(SUCCEED);
		}
	}

	id->m_tLastPASSURGetTime[passurindex] = secs;

	i = get_latest_name(id, passurindex, tmpstr, curl_handle);
	if (i < 0) return(FALSE);   // download failed
	if (i > 0) return(SUCCEED); // end of replay

	strcpy_s(targetname, _countof(targetname), &id->m_strPassurZipFilename[passurindex][5]);
	sprintf_s(url, URL_SZ, "https://%s/sdata/%s", GetPassurSrvr(id), targetname );

	if ( !strcmp( url, prevurl[passurindex] ) ){
		/* we already got this file */
		D_CONTROL("PASSUR_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Already got this file(%s), skipping\n", cdwThreadId, url));
		return( SUCCEED );
	}
	strcpy_s(prevurl[passurindex], URL_SZ, url);

	if ( !strncmp( id->m_strUserName, "sm", 2 ) ){  // take anything for now
		_snprintf_s( id->m_strLatestPassurURL, URL_SZ, _TRUNCATE, "PASSUR: %s", url );
	}	

	id->m_tPASSURFileTime = CalcPassurFileTime( id, url );  // Gfiletime used for replays

	ftime( &curl_start_time);

	/* clear out buffer */
	chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
	chunk.size = 0;    /* no data at this point */

	if ( !GetCurlPage_r( id, curl_handle, &chunk, url, SM_HTTPS, 3, 4L ) ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Curl failed for %s\n", cdwThreadId, url );
		NotePassurServerFailure(id);		
		return( FALSE );
	}
	else {
		NotePassurServerSuccess(id);
	}

	// make sure size if large enough, may be problem if zip wasn't done fast enough
	if ( chunk.size < 100 ){
		// try again
		free_chunk_memory(&chunk);
		if ( !GetCurlPage_r( id, curl_handle, &chunk, url, SM_HTTPS, 3, 4L ) ){
			free_chunk_memory(&chunk);
			return( FALSE );
		}
	}

	ftime( &curl_end_time);
	if ( timeflag ) {
		MGLGridText(id, statusgrid, 9, 0, "passur zip");
		sprintf_s(tmpstr, TMPBUF_SIZE, "%g", uts_timediff(&curl_start_time, &curl_end_time ));
		MGLGridText(id, statusgrid, 9, 1, tmpstr);
	}

	if ( chunk.memory && chunk.size < 500 ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Small passur file found '%s'\n", cdwThreadId, chunk.memory );
	}
	if ( !chunk.size || !chunk.memory ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Error, null chunk size\n", cdwThreadId );
		return( FALSE );
	}

	unzip_and_save_passur_data(id, passurindex, cur_arpt, curl_handle, &chunk, url);
	
	if (id->m_bClass1 && !id->m_nDelaySecs) {
		// save the request time for a moving average
		request_time = uts_timediff(&curl_start_time, &curl_end_time);
		save_request_time(request_time, &(id->m_dPassurPrevRequest), id->m_dPassurRequestTimes,
			&(id->m_nPassurRequestIndex), id->m_nPassurRequestNum);
		// recalculate the average request time
		passur_request_average = get_average_request_time(id->m_dPassurRequestTimes, id->m_nPassurRequestNum);
		asdex_request_average = get_average_request_time(id->m_dASDEXRequestTimes, id->m_nASDEXRequestNum);
		if (passur_request_average < 500.0 && asdex_request_average < 500.0) {
			// internet connection fast enough to turn on fast updates
			id->m_bFastUpdates = TRUE;
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) FastUpdates enabled\n", cdwThreadId); 
		}
		D_CONTROL("PASSUR_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) time %2gms avg %2gms for %s\n",
			cdwThreadId, request_time, passur_request_average, url));
	}

	return( SUCCEED );
}

static int GetLMGData(  struct InstanceData* id )
{
	time_t secs, ft;
	struct MemoryStruct chunk;
	int CkLastTime = TRUE; // always check time interval for passur data
	int delaysecs;
	int replay_adjust_secs;
	char url[URL_SZ];
	char ft_buf[URL_SZ] = {0};
	char *outbufptr = NULL;
	char *p = 0;
	char tmpstr[ TMPBUF_SIZE ];
	int loopcount;
	int urlLen;
	char arpt[5] = {0};
	char LatestLMGcginame[256];
	static CURL* curl_handle = NULL;
	const DWORD cdwThreadId = GetCurrentThreadId();
	
	sprintf_s(arpt, 5, "K%c%c%c", id->m_strAsdexArpt[0], id->m_strAsdexArpt[1], id->m_strAsdexArpt[2]);
	StrUpper( arpt );

	if( !IsValidLMGAirport(id, arpt) ){
		return FALSE;
	}

	if( !curl_handle ) {
		if(!(curl_handle = GetCurlHandle(id, SM_HTTPS))){
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create CURL handle!\n", cdwThreadId);
			return FALSE;
		}
	}
	
	secs = time(0);
	
	if ( CkLastTime ){
		// see if we are in fast replay mode
		if ( id->m_nDelaySecs ){
			// we are in replay mode, use Plottime to retrieve next file
			// get ASDEX data 2 seconds ahead of plottime to prevent pauses
			if ( id->m_tASDEXFileTime < id->m_tPlotTime+2 ){
				// file is behind, get the next file
				D_CONTROL("LMG_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) file is behind, Gfiletime=%d plottime=%d\n",
					cdwThreadId, id->m_tASDEXFileTime, id->m_tPlotTime ));
			} else if ( secs - id->m_tLastASDEXGetTime < 5 ){
				/* we already got it */
				//if(id->m_bClass1)
				//	Callback2Javascript(id, "AsdexStatus:Slow");
				//else
				//	Callback2Javascript(id, "AsdexStatus:OK");
				return(SUCCEED);
			}
		} else if ( secs - id->m_tLastASDEXGetTime < 5 ){  // new passur data every 5 seconds -- one per thread
			/* we already got it */
			//Callback2Javascript(id, "AsdexStatus:Slow");
			return(SUCCEED);
		}
	}

	loopcount = 0;
	do {
		id->m_strLMGZipFilename[0] = '\0';
		if ( id->m_nDelaySecs || !id->m_bClass1 ){
			if ( !id->m_bClass1 ){
				delaysecs = MAX(id->m_nDelaySecs, 300);
			} else {
				delaysecs = id->m_nDelaySecs;
			}
			if ( id->m_nDelaySecs && id->m_nReplaySpeed && delaystartsecs){
				replay_adjust_secs = (int) (id->m_tClientsTime - delaystartsecs) *  (id->m_nReplaySpeed - 1);
				delaysecs -= replay_adjust_secs;
				D_CONTROL("LMG_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) delaysecs reduced by %d\n", cdwThreadId, replay_adjust_secs ));
			}
			if ( delaysecs < 0 || (delaysecs < 300 && !id->m_bClass1) ){
				// we already got it 
				D_CONTROL("LMG_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) delaysecs =%d, in future, replay past current time\n",
					cdwThreadId, delaysecs ));
				//if(id->m_bClass1)
				//	Callback2Javascript(id, "AsdexStatus:Slow");
				//else
				//	Callback2Javascript(id, "AsdexStatus:OK");
				return(SUCCEED);
			}
			// get ASDEX data 2 seconds ahead of plottime to prevent pauses
			sprintf_s(tmpstr, TMPBUF_SIZE, "arpt=%s&feed=%s&replay=%d", arpt, "lmg", delaysecs+2 );
		} else {
			sprintf_s(tmpstr, TMPBUF_SIZE, "arpt=%s&feed=%s", arpt, "lmg" );
		}
	
		clear_chunk_memory(&chunk);

		setCurlTimeout(curl_handle, 4L);
		sprintf_s(LatestLMGcginame, _countof(LatestLMGcginame), "%s/cgi-bin/latest.cgi",  GetLMGSrvr(id));

        vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlFormGetResults - LatestLMGcginame <%s> tmpstr <%s>.\n", 
			cdwThreadId, LatestLMGcginame, tmpstr );

		if ( GetCurlFormGetResults_r( id, curl_handle, &chunk, LatestLMGcginame, tmpstr, SM_HTTPS, TRUE ) == FALSE ) {
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Call to GetCurlFormGetResults for url: <%s> failed, switching srvrs & trying again.\n", 
				cdwThreadId, LatestLMGcginame);
			GetNxtLMGSrvr(id);
		}
		
		if ( chunk.memory ){
			strncpy_s( id->m_strLMGZipFilename, FILENAME_SZ, chunk.memory, 255 );
			free_chunk_memory( &chunk );
		} else {
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not get latest.cgi, trying again\n", cdwThreadId);
		}
	} while (!strlen(id->m_strLMGZipFilename) && ++loopcount < 5 );
	if (loopcount >= 5) {
		NoteLMGServerFailure(id);
		return( FALSE );
	}	

	/* clear out buffer */
	chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
	chunk.size = 0;    /* no data at this point */

	sprintf_s(url, URL_SZ, "%s/%s", GetLMGSrvr(id), id->m_strLMGZipFilename ); 
	D_CONTROL("LMG_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) LMG url <%s>\n", cdwThreadId, url ));

	urlLen = strlen(id->m_strPrevLMGUrl);		
	// Used strncmp becuase the url returned from above may have ending newline character, which causes strcmp to fail
	if ( !strncmp( url, id->m_strPrevLMGUrl, (urlLen > 0 ? urlLen: 1)) ){
		/* we already got this file */
		D_CONTROL("LMG_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Already got this file(%s), skipping\n", cdwThreadId, url));
		// Fail Counts greater than 3 mean we are no longer getting new files, asde-x feed is down.
		if(++id->m_dwLMGFailCount > 3){
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Server succeeded but data file is empty, %s LMG feed to PASSUR may be down.\n", 
				cdwThreadId, arpt);
			NoteAsdexServerFailure(id);
		}
		return( SUCCEED );
	}
	id->m_dwLMGFailCount = 0L;
	
	if ( !strncmp( id->m_strUserName, "sm", 2 ) ){  // take anything for now
		if('\x0A' == url[strlen(url)-1])url[strlen(url)-1] = 0; // removes newline		
		_snprintf_s( id->m_strLatestPassurURL, URL_SZ, _TRUNCATE, "LMG: %s", url );
		D_CONTROL("LMG_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) %s", cdwThreadId, id->m_strLatestPassurURL));
	}

	// Adjust ft_buf to look like an asd2 buffer
	p = strstr(url, arpt);
	if ( p ){
		sprintf_s(ft_buf, URL_SZ, "asd2/%s", p + 5);
	}

	ft = CalcFileTime(id, ft_buf);
	if ( !id->m_tPlotTime ) id->m_tPlotTime = time(0);
	if ( (id->m_tPlotTime - ft) > 20 ){
		D_CONTROL("LMG_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) File time is too old for use, no recent tracks from (%s), skipping\n", cdwThreadId, url));
		strcpy_s(id->m_strPrevLMGUrl, URL_SZ, url);
		return ( SUCCEED );
	}
	if ( !GetCurlPage_r( id, curl_handle, &chunk, url, SM_HTTPS, 3, 4L ) ){
	
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Curl failed for <%>s\n", cdwThreadId, url );
		NoteLMGServerFailure(id);
		free_chunk_memory(&chunk);
		return( FALSE );
	}
	
	if ( !chunk.size || !chunk.memory ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Error, null chunk size\n", cdwThreadId );
		NoteLMGServerFailure(id);
		return( FALSE );
	}

	NoteLMGServerSuccess(id);
	unzip_and_save_lmg_data(id, &chunk, url);

	return( SUCCEED );
}


static int GetASDEXDataFastUpdates( struct InstanceData* id, int force_refresh, CURL* curl_handle )
{
	struct MemoryStruct chunk;
	struct MemoryStruct hdrs;
	char url[URL_SZ];
	char ASDEXFastUpdatesCGIName[256];
	char tmpstr[ TMPBUF_SIZE ];
	char *outbufptr = NULL;
	char prevfile[256];
	char cur_arpt[5] = {0};	
	struct timeb curl_start_time, curl_end_time;
	double request_time, request_average;
	const DWORD cdwThreadId = GetCurrentThreadId();

	if (!curl_handle) return FALSE;

	if (!time_elapsed(&(id->m_sASDEXReqStart), 1000.0)) return FALSE;
	
	id->m_tClientsTime = time(0);

	EnterCriticalSection(&id->m_csASDEXSync);

	// Save current airport
	strcpy_s(cur_arpt, _countof(cur_arpt), id->m_strAsdexArpt);

	sprintf_s(tmpstr, TMPBUF_SIZE, "arpt=%s&feed=%s", id->m_strAsdexArpt, "asdex");

	if (id->m_strAsdexPrevFile[0] != '\0') {
		strcat_s(tmpstr, TMPBUF_SIZE, "&prevfile=");
		strcat_s(tmpstr, TMPBUF_SIZE, id->m_strAsdexPrevFile);
	}

	LeaveCriticalSection(&id->m_csASDEXSync);

	ftime( &curl_start_time);
	
	/* clear out buffer */
	clear_chunk_memory(&chunk);
	clear_chunk_memory(&hdrs);

	setCurlTimeout(curl_handle, 4L);

	sprintf_s(ASDEXFastUpdatesCGIName, _countof(ASDEXFastUpdatesCGIName), "%s/fcgi/fastupdates.fcg", GetAsdexSrvr(id)); 

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlFormGetResultsHdrs_r - ASDEXFastUpdatesCGIName <%s> tmpstr <%s>.\n", 
		cdwThreadId, ASDEXFastUpdatesCGIName, tmpstr );

	if ( GetCurlFormGetResultsHdrs_r(id, curl_handle, &chunk, &hdrs,
		ASDEXFastUpdatesCGIName, tmpstr, SM_HTTPS, TRUE ) == FALSE ) {

	  vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) <%s> failed tmpstr.<%s>\n", cdwThreadId, ASDEXFastUpdatesCGIName, tmpstr);

	  NoteAsdexServerFailure(id);
	  free_chunk_memory(&chunk);
	  free_chunk_memory(&hdrs);
	  return( FALSE );
	} 

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Success <%s>.\n", cdwThreadId, ASDEXFastUpdatesCGIName);

	ftime( &curl_end_time );

	// save the request time for a moving average
	request_time = uts_timediff(&curl_start_time, &curl_end_time);
	save_request_time(request_time, &(id->m_dASDEXPrevRequest), id->m_dASDEXRequestTimes,
		&(id->m_nASDEXRequestIndex), id->m_nASDEXRequestNum);
	// check the average request time
	request_average = get_average_request_time(id->m_dASDEXRequestTimes, id->m_nASDEXRequestNum);
	if (request_average > 600.0) {
		// internet connection too slow for fast updates
		id->m_bFastUpdates = FALSE;
		//Callback2Javascript(id, "FastUpdates:OFF");
		//memset(id->m_dASDEXRequestTimes, 0, sizeof(id->m_dASDEXRequestTimes));
		vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) FastUpdates disabled (slow connection)\n", cdwThreadId); 
	}
	D_CONTROL("ASDEX_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) time %2gms avg %2gms for <%s>\n",
		cdwThreadId, request_time, request_average, ASDEXFastUpdatesCGIName));

	if ( !hdrs.size ) return( FALSE );

	find_filename_in_hdrs(hdrs, prevfile);
	find_filepath_in_hdrs(hdrs, id->m_strAsdexZipFilename);

	free_chunk_memory(&hdrs);

	if (prevfile[0] == '\0') {
		// filename not in http headers - request failed
		// retry in 1 second
		free_chunk_memory(&chunk);
		id->m_sASDEXReqStart = curl_end_time;
		// Fail Counts greater than 3 mean we are no longer getting new data, asde-x feed is down.
		if(++id->m_dwAsdexFailCount > 3){
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Server succeeded but data file is empty, %s ASDE-X feed to PASSUR may be down.\n", 
				cdwThreadId, id->m_strAsdexArpt);
			NoteAsdexServerFailure(id);
		}
		return FALSE; 
	}
	id->m_dwAsdexFailCount = 0L;
	
	EnterCriticalSection(&id->m_csASDEXSync);
	// if airport didn't change, save prevfile for next time
	if (strcmp(cur_arpt, id->m_strAsdexArpt) == 0) {
		strcpy_s(id->m_strAsdexPrevFile, FILENAME_SZ, prevfile);
	}
	LeaveCriticalSection(&id->m_csASDEXSync);

	sprintf_s(url, URL_SZ, "%s%s%s", GetAsdexSrvr(id),  "/sdata", id->m_strAsdexZipFilename ); 
	
	D_CONTROL("ASDEX_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) ASDEX url <%s>\n", cdwThreadId, url ));

	if ( !strncmp( id->m_strUserName, "sm", 2 ) ){  // take anything for now
		if('\x0A' == url[strlen(url)-1])url[strlen(url)-1] = 0; // removes newline		
		_snprintf_s( id->m_strLatestPassurURL, URL_SZ, _TRUNCATE, "ASDE-X: %s", url );
		D_CONTROL("ASDEX_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) got  %s\n", cdwThreadId, url));
	}	

	if ( !chunk.size || !chunk.memory ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Error, null chunk size\n", cdwThreadId );
		NoteAsdexServerFailure(id);
		return( FALSE );
	}
	NoteAsdexServerSuccess(id);
	unzip_and_save_asdex_data(id, &chunk, url);	

	GetLMGData(id);
	
    return( SUCCEED );
}


static int GetASDEXData(  struct InstanceData* id, int force_refresh, CURL* curl_handle )
{
	time_t secs;
	struct MemoryStruct chunk;
	int CkLastTime = TRUE; // always check time interval for passur data
	int delaysecs;
	int replay_adjust_secs;
	char url[URL_SZ];
	char *outbufptr = NULL;
	char tmpstr[ TMPBUF_SIZE ];
	int loopcount;
	int urlLen;
	char LatestAsdexcginame[256];
	struct timeb curl_start_time, curl_end_time;
	double request_time, passur_request_average, asdex_request_average;
	const DWORD cdwThreadId = GetCurrentThreadId();

	if(!id->m_sCurLayout.m_bShowASDEXAircraft) {
		memset(id->m_dASDEXRequestTimes, 0, sizeof(id->m_dASDEXRequestTimes)); // clear request timings
		Callback2Javascript(id, "AsdexStatus:Off");
		return(SUCCEED);
	}

	if (!curl_handle) return FALSE;

	if( !IsValidAsdexAirport(id, id->m_strAsdexArpt) ){
		Callback2Javascript(id, "AsdexStatus:N/A");
		return FALSE;
	}

	if (id->m_bFastUpdates && id->m_bClass1 && !id->m_nDelaySecs) {
		return GetASDEXDataFastUpdates(id, force_refresh, curl_handle);
	}

	secs = time(0);
	
	if ( CkLastTime ){
		// see if we are in fast replay mode
		if ( id->m_nDelaySecs ){
			// we are in replay mode, use Plottime to retrieve next file
			// get ASDEX data 2 seconds ahead of plottime to prevent pauses
			if ( id->m_tASDEXFileTime < id->m_tPlotTime+2 ){
				// file is behind, get the next file
				D_CONTROL("ASDEX_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) file is behind, Gfiletime=%d plottime=%d\n",
					cdwThreadId, id->m_tASDEXFileTime, id->m_tPlotTime ));
			} else if ( secs - id->m_tLastASDEXGetTime < 5 ){
				/* we already got it */
				if(id->m_bClass1)
					Callback2Javascript(id, "AsdexStatus:Slow");
				else
					Callback2Javascript(id, "AsdexStatus:OK");
				return(SUCCEED);
			}
		} else if ( secs - id->m_tLastASDEXGetTime < 5 ){  // new passur data every 5 seconds -- one per thread
			/* we already got it */
			//Callback2Javascript(id, "AsdexStatus:Slow");
			return(SUCCEED);
		}
	}

	id->m_tLastASDEXGetTime = secs;

	loopcount = 0;
	do {
		id->m_strAsdexZipFilename[0] = '\0';
		if ( id->m_nDelaySecs || !id->m_bClass1 ){
			if ( !id->m_bClass1 ){
				delaysecs = MAX(id->m_nDelaySecs, 300);
			} else {
				delaysecs = id->m_nDelaySecs;
			}
			if ( id->m_nDelaySecs && id->m_nReplaySpeed && delaystartsecs){
				replay_adjust_secs = (int) (id->m_tClientsTime - delaystartsecs) *  (id->m_nReplaySpeed - 1);
				delaysecs -= replay_adjust_secs;
				D_CONTROL("ASDEX_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) delaysecs reduced by %d\n", cdwThreadId, replay_adjust_secs ));
			}
			if ( delaysecs < 0 || (delaysecs < 300 && !id->m_bClass1) ){
				// we already got it 
				D_CONTROL("ASDEX_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) delaysecs =%d, in future, replay past current time\n",
					cdwThreadId, delaysecs ));
				if(id->m_bClass1)
					Callback2Javascript(id, "AsdexStatus:Slow");
				else
					Callback2Javascript(id, "AsdexStatus:OK");
				return(SUCCEED);
			}
			// get ASDEX data 2 seconds ahead of plottime to prevent pauses
			sprintf_s(tmpstr, TMPBUF_SIZE, "arpt=%s&feed=%s&replay=%d", id->m_strAsdexArpt, "asdex", delaysecs+2 );
		} else {
			sprintf_s(tmpstr, TMPBUF_SIZE, "arpt=%s&feed=%s", id->m_strAsdexArpt, "asdex" );
		}
	
		clear_chunk_memory(&chunk);

		setCurlTimeout(curl_handle, 4L);
		sprintf_s(LatestAsdexcginame, _countof(LatestAsdexcginame), "%s/cgi-bin/latest.cgi",  GetAsdexSrvr(id));

        vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlFormGetResults - LatestAsdexcginame <%s> tmpstr <%s>.\n", 
			cdwThreadId, LatestAsdexcginame, tmpstr );

		if ( GetCurlFormGetResults_r( id, curl_handle, &chunk, LatestAsdexcginame, tmpstr, SM_HTTPS, TRUE ) == FALSE ) {
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Call to GetCurlFormGetResults for url: <%s> failed, switching srvrs & trying again.\n", 
				cdwThreadId, LatestAsdexcginame);

			/*---------------------------------------------------------------------*/
			/* Swithch to next ASDEX server                                        */
			/*---------------------------------------------------------------------*/
		
			GetNxtAsdexSrvr(id);
		}
		
		if ( chunk.memory ){
			strncpy_s( id->m_strAsdexZipFilename, FILENAME_SZ, chunk.memory, 255 );
			free_chunk_memory( &chunk );
		} else {
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not get latest.cgi, trying again\n", cdwThreadId);
		}
	} while (!strlen(id->m_strAsdexZipFilename) && ++loopcount < 5 );
	if (loopcount >= 5) {
		NoteAsdexServerFailure(id);
		return( FALSE );
	}	
	ftime( &curl_start_time);

	/* clear out buffer */
	chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
	chunk.size = 0;    /* no data at this point */

	sprintf_s(url, URL_SZ, "%s/sdata/%s", GetAsdexSrvr(id), id->m_strAsdexZipFilename ); 
	D_CONTROL("ASDEX_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) ASDEX url <%s>\n", cdwThreadId, url ));

	urlLen = strlen(id->m_strPrevASDEXUrl);		
	// Used strncmp becuase the url returned from above may have ending newline character, which causes strcmp to fail
	if ( !strncmp( url, id->m_strPrevASDEXUrl, (urlLen > 0 ? urlLen: 1)) ){
		/* we already got this file */
		D_CONTROL("ASDEX_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Already got this file(%s), skipping\n", cdwThreadId, url));
		// Fail Counts greater than 3 mean we are no longer getting new files, asde-x feed is down.
		if(++id->m_dwAsdexFailCount > 3){
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Server succeeded but data file is empty, %s ASDE-X feed to PASSUR may be down.\n", 
				cdwThreadId, id->m_strAsdexArpt);
			NoteAsdexServerFailure(id);
		}
		return( SUCCEED );
	}
	id->m_dwAsdexFailCount = 0L;
	
	if ( !strncmp( id->m_strUserName, "sm", 2 ) ){  // take anything for now
		if('\x0A' == url[strlen(url)-1])url[strlen(url)-1] = 0; // removes newline		
		_snprintf_s( id->m_strLatestPassurURL, URL_SZ, _TRUNCATE, "ASDE-X: %s", url );
		D_CONTROL("ASDEX_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) %s", cdwThreadId, id->m_strLatestPassurURL));
	}

	if ( !GetCurlPage_r( id, curl_handle, &chunk, url, SM_HTTPS, 3, 4L ) ){
	
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Curl failed for <%>s\n", cdwThreadId, url );
		NoteAsdexServerFailure(id);
		free_chunk_memory(&chunk);
		return( FALSE );
	}
	
	ftime( &curl_end_time);

	if ( !chunk.size || !chunk.memory ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Error, null chunk size\n", cdwThreadId );
		NoteAsdexServerFailure(id);
		return( FALSE );
	}

	// ASDE-X files can be much smaller than ASD/PASSUR files
	if ( chunk.memory && chunk.size < 140 ){
	  vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Small ASDE-X file found '%s'\n", cdwThreadId, chunk.memory );
		free_chunk_memory(&chunk);
		NoteAsdexServerFailure(id);
		return (FALSE);
	}	
	NoteAsdexServerSuccess(id);
	unzip_and_save_asdex_data(id, &chunk, url);

	if (id->m_bClass1 && !id->m_nDelaySecs) {
		// save the request time for a moving average
		request_time = uts_timediff(&curl_start_time, &curl_end_time);
		save_request_time(request_time, &(id->m_dASDEXPrevRequest), id->m_dASDEXRequestTimes,
			&(id->m_nASDEXRequestIndex), id->m_nASDEXRequestNum);
		// recalculate the average request time
		passur_request_average = get_average_request_time(id->m_dPassurRequestTimes, id->m_nPassurRequestNum);
		asdex_request_average = get_average_request_time(id->m_dASDEXRequestTimes, id->m_nASDEXRequestNum);
		if (passur_request_average < 500.0 && asdex_request_average < 500.0) {
			// internet connection fast enough to turn on fast updates
			id->m_bFastUpdates = TRUE;
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) FastUpdates enabled\n", cdwThreadId); 
		}
		D_CONTROL("ASDEX_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) time %2gms avg %2gms for <%s>\n",
			cdwThreadId, request_time, asdex_request_average, url));
	}

	GetLMGData(id);

	return( SUCCEED );
}

static int GetADSBDataFastUpdates( struct InstanceData* id, int force_refresh, CURL* curl_handle )
{
	struct MemoryStruct chunk;
	struct MemoryStruct hdrs;
	char url[URL_SZ];
	char ADSBFastUpdatesCGIName[256];
	char tmpstr[ TMPBUF_SIZE ];
	char *outbufptr = NULL;
	char prevfile[256];
	char cur_arpt[5] = {0};	
	struct timeb curl_start_time, curl_end_time;
	double request_time, request_average;
	int bValidMlatArpt = FALSE;
	const DWORD cdwThreadId = GetCurrentThreadId();

	if (!curl_handle) return FALSE;

	if (!time_elapsed(&(id->m_sADSBReqStart), 1000.0)) return FALSE;
	
	id->m_tClientsTime = time(0);

	EnterCriticalSection(&id->m_csADSBSync);

	// Save current airport
	strcpy_s(cur_arpt, _countof(cur_arpt), id->m_strAdsbArpt);

	sprintf_s(tmpstr, TMPBUF_SIZE, "arpt=%s&feed=%s", id->m_strAdsbArpt, "adsb");

	if (id->m_strAdsbPrevFile[0] != '\0') {
		strcat_s(tmpstr, TMPBUF_SIZE, "&prevfile=");
		strcat_s(tmpstr, TMPBUF_SIZE, id->m_strAdsbPrevFile);
	}

	LeaveCriticalSection(&id->m_csADSBSync);

	ftime( &curl_start_time);
	
	/* clear out buffer */
	clear_chunk_memory(&chunk);
	clear_chunk_memory(&hdrs);

	setCurlTimeout(curl_handle, 4L);

	// Forces pulling of ADSB data from same server as MLAT data server
	bValidMlatArpt = IsValidMLATAirport(id, id->m_strAdsbArpt);
	if(bValidMlatArpt)
		sprintf_s(ADSBFastUpdatesCGIName, _countof(ADSBFastUpdatesCGIName), "%s/fcgi/fastupdates.fcg", GetMlatSrvr(id)); 
	else
		sprintf_s(ADSBFastUpdatesCGIName, _countof(ADSBFastUpdatesCGIName), "%s/fcgi/fastupdates.fcg", GetAdsbSrvr(id)); 

    vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlFormGetResultsHdrs_r - ADSBFastUpdatesCGIName <%s> tmpstr <%s>.\n", 
			cdwThreadId, ADSBFastUpdatesCGIName, tmpstr );

	if ( GetCurlFormGetResultsHdrs_r(id, curl_handle, &chunk, &hdrs,
		ADSBFastUpdatesCGIName, tmpstr, SM_HTTPS, TRUE ) == FALSE ) {

	  vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) <%s> failed tmpstr.<%s>\n", cdwThreadId, ADSBFastUpdatesCGIName, tmpstr);

	  //Callback2Javascript(id, "AsdexStatus:FAIL");
	  // 
	  // Note server failure and get alternate if available
	  //

	  NoteAdsbServerFailure(id);
	  free_chunk_memory(&chunk);
	  free_chunk_memory(&hdrs);
	  return( FALSE );
	} 

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Success <%s>.\n", cdwThreadId, ADSBFastUpdatesCGIName);

	ftime( &curl_end_time );

	// save the request time for a moving average
	request_time = uts_timediff(&curl_start_time, &curl_end_time);
	save_request_time(request_time, &(id->m_dADSBPrevRequest), id->m_dADSBRequestTimes,
		&(id->m_nADSBRequestIndex), id->m_nADSBRequestNum);
	// check the average request time
	request_average = get_average_request_time(id->m_dADSBRequestTimes, id->m_nADSBRequestNum);
	if (request_average > 600.0) {
		// internet connection too slow for fast updates
		id->m_bFastUpdates = FALSE;
		//Callback2Javascript(id, "FastUpdates:OFF");
		//memset(id->m_dASDEXRequestTimes, 0, sizeof(id->m_dASDEXRequestTimes));
		vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) FastUpdates disabled (slow connection)\n", cdwThreadId); 
	}
	D_CONTROL("ADSB_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) time %2gms avg %2gms for <%s>\n",
		cdwThreadId, request_time, request_average, ADSBFastUpdatesCGIName));

	
	find_filename_in_hdrs(hdrs, prevfile);
	find_filepath_in_hdrs(hdrs, id->m_strAdsbZipFilename);

	free_chunk_memory(&hdrs);

	if (prevfile[0] == '\0') {
		// filename not in http headers - request failed
		// retry in 1 second
		free_chunk_memory(&chunk);
		id->m_sADSBReqStart = curl_end_time;
		// Fail Counts greater than 3 mean we are no longer getting new data, asde-x feed is down.
		if(++id->m_dwAdsbFailCount > 3){
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Server succeeded but data file is empty, %s ADS-B feed to PASSUR may be down.\n", 
				cdwThreadId, id->m_strAdsbArpt);
			NoteAdsbServerFailure(id);
		}	
		return FALSE; 
	}
	id->m_dwAdsbFailCount = 0L;
	
	EnterCriticalSection(&id->m_csADSBSync);
	// if airport didn't change, save prevfile for next time
	if (strcmp(cur_arpt, id->m_strAdsbArpt) == 0) {
		strcpy_s(id->m_strAdsbPrevFile, FILENAME_SZ, prevfile);
	}
	LeaveCriticalSection(&id->m_csADSBSync);

	if(bValidMlatArpt)
		sprintf_s(url, URL_SZ, "%s%s", GetMlatSrvr(id),  id->m_strAdsbZipFilename ); 
	else
		sprintf_s(url, URL_SZ, "%s%s", GetAdsbSrvr(id),  id->m_strAdsbZipFilename ); 
	
	D_CONTROL("ADSB_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) ADSB url <%s>\n", cdwThreadId, url ));

	if ( !strncmp( id->m_strUserName, "sm", 2 ) ){  // take anything for now
		if('\x0A' == url[strlen(url)-1])url[strlen(url)-1] = 0; // removes newline		
		_snprintf_s( id->m_strLatestPassurURL, URL_SZ, _TRUNCATE, "ADSB-X: %s", url );
		D_CONTROL("ADSB_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) got  %s\n", cdwThreadId, url));
	}	

	if ( !chunk.size || !chunk.memory ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Error, null chunk size\n", cdwThreadId );
		NoteAdsbServerFailure(id);
		return( FALSE );
	}
	NoteAdsbServerSuccess(id);
	unzip_and_save_adsb_data(id, &chunk, url);
    return( SUCCEED );
}


static int GetADSBData(  struct InstanceData* id, int force_refresh, CURL* curl_handle )
{
	time_t secs;
	struct MemoryStruct chunk;
	int CkLastTime = TRUE; // always check time interval for passur data
	int delaysecs;
	int replay_adjust_secs;
	char url[URL_SZ];
	char *outbufptr = NULL;
	char tmpstr[ TMPBUF_SIZE ];
	int loopcount;
	int urlLen;
	char LatestAdsbcginame[256];
	struct timeb curl_start_time, curl_end_time;
	double request_time, passur_request_average, asdex_request_average;
	int bValidMlatArpt = FALSE;
	const DWORD cdwThreadId = GetCurrentThreadId();
	
	if(!id->m_sCurLayout.m_bShowADSBAircraft) {
		memset(id->m_dADSBRequestTimes, 0, sizeof(id->m_dADSBRequestTimes)); // clear request timings
		Callback2Javascript(id, "AdsbStatus:Off");
		return(SUCCEED);
	}

	if (!curl_handle) return FALSE;

	if( !IsValidAdsbAirport(id, id->m_strAdsbArpt) ){
		Callback2Javascript(id, "AdsbStatus:N/A");
		return FALSE;
	}

	if (id->m_bFastUpdates && id->m_bClass1 && !id->m_nDelaySecs) {
		return GetADSBDataFastUpdates(id, force_refresh, curl_handle);
	}

	secs = time(0);
	
	if ( CkLastTime ){
		// see if we are in fast replay mode
		if ( id->m_nDelaySecs ){
			// we are in replay mode, use Plottime to retrieve next file
			// get ASDEX data 2 seconds ahead of plottime to prevent pauses
			if ( id->m_tADSBFileTime < id->m_tPlotTime+2 ){
				// file is behind, get the next file
				D_CONTROL("ADSB_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) file is behind, Gfiletime=%d plottime=%d\n",
					cdwThreadId, id->m_tADSBFileTime, id->m_tPlotTime ));
			} else if ( secs - id->m_tLastADSBGetTime < 5 ){
				/* we already got it */
				if(id->m_bClass1)
					Callback2Javascript(id, "AdsbStatus:Slow");
				else
					Callback2Javascript(id, "AdsbStatus:OK");
				return(SUCCEED);
			}
		} else if ( secs - id->m_tLastADSBGetTime < 5 ){  // new passur data every 5 seconds -- one per thread
			/* we already got it */
			//Callback2Javascript(id, "AsdexStatus:Slow");
			return(SUCCEED);
		}
	}

	id->m_tLastADSBGetTime = secs;

	loopcount = 0;
	do {
		id->m_strAdsbZipFilename[0] = '\0';
		if ( id->m_nDelaySecs || !id->m_bClass1 ){
			if ( !id->m_bClass1 ){
				delaysecs = MAX(id->m_nDelaySecs, 300);
			} else {
				delaysecs = id->m_nDelaySecs;
			}
			if ( id->m_nDelaySecs && id->m_nReplaySpeed && delaystartsecs){
				replay_adjust_secs = (int) (id->m_tClientsTime - delaystartsecs) *  (id->m_nReplaySpeed - 1);
				delaysecs -= replay_adjust_secs;
				D_CONTROL("ADSB_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) delaysecs reduced by %d\n", cdwThreadId, replay_adjust_secs ));
			}
			if ( delaysecs < 0 || (delaysecs < 300 && !id->m_bClass1) ){
				// we already got it 
				D_CONTROL("ADSB_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) delaysecs =%d, in future, replay past current time\n",
					cdwThreadId, delaysecs ));
				if(id->m_bClass1)
					Callback2Javascript(id, "AdsbStatus:Slow");
				else
					Callback2Javascript(id, "AdsbStatus:OK");
				return(SUCCEED);
			}
			// get ADSB data 2 seconds ahead of plottime to prevent pauses
			sprintf_s(tmpstr, TMPBUF_SIZE, "arpt=%s&feed=%s&replay=%d", id->m_strAdsbArpt, "adsb", delaysecs+2 );
		} else {
			sprintf_s(tmpstr, TMPBUF_SIZE, "arpt=%s&feed=%s", id->m_strAdsbArpt, "adsb" );
		}
	
		clear_chunk_memory(&chunk);

		setCurlTimeout(curl_handle, 4L);
		// Forces pulling of ADSB data from same server as MLAT data server
		bValidMlatArpt = IsValidMLATAirport(id, id->m_strAdsbArpt);
		if(bValidMlatArpt)
			sprintf_s(LatestAdsbcginame, _countof(LatestAdsbcginame), "%s/cgi-bin/latest.cgi", GetMlatSrvr(id)); 
		else
			sprintf_s(LatestAdsbcginame, _countof(LatestAdsbcginame), "%s/cgi-bin/latest.cgi",  GetAdsbSrvr(id));

        vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlFormGetResults - LatestAdsbcginame <%s> tmpstr <%s>.\n", 
			cdwThreadId, LatestAdsbcginame, tmpstr );

		if ( GetCurlFormGetResults_r( id, curl_handle, &chunk, LatestAdsbcginame, tmpstr, SM_HTTPS, TRUE ) == FALSE ) {
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Call to GetCurlFormGetResults for url: <%s> failed, switching srvrs & trying again.\n", 
				cdwThreadId, LatestAdsbcginame);

			/*---------------------------------------------------------------------*/
			/* Switch to next ADSB server                                        */
			/*---------------------------------------------------------------------*/
		
			GetNxtAdsbSrvr(id);
		}
		
		if ( chunk.memory && !strstr( chunk.memory, "404 Not Found" ) ){
			strncpy_s( id->m_strAdsbZipFilename, FILENAME_SZ, chunk.memory, 255 );
			free_chunk_memory( &chunk );
		} else {
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not get latest.cgi, trying again\n", cdwThreadId);
		}
	} while (!strlen(id->m_strAdsbZipFilename) && ++loopcount < 5 );
	if (loopcount >= 5) {
		NoteAdsbServerFailure(id);
		return( FALSE );
	}
	
	ftime( &curl_start_time);

	/* clear out buffer */
	chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
	chunk.size = 0;    /* no data at this point */

	if(bValidMlatArpt)
		sprintf_s(url, URL_SZ, "%s/sdata/%s", GetMlatSrvr(id), id->m_strAdsbZipFilename ); 
	else
		sprintf_s(url, URL_SZ, "%s/sdata/%s", GetAdsbSrvr(id), id->m_strAdsbZipFilename ); 
	D_CONTROL("ADSB_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) ADSB url <%s>\n", cdwThreadId, url ));

	urlLen = strlen(id->m_strPrevADSBUrl);		
	// Used strncmp becuase the url returned from above may have ending newline character, which causes strcmp to fail
	if ( !strncmp( url, id->m_strPrevADSBUrl, (urlLen > 0 ? urlLen: 1)) ){
		/* we already got this file */
		D_CONTROL("ADSB_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Already got this file(%s), skipping\n", cdwThreadId, url));
		// Fail Counts greater than 3 mean we are no longer getting new files, ads-b feed is down.
		if(++id->m_dwAdsbFailCount > 3){
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Server succeeded but data file is empty, %s ADS-B feed to PASSUR may be down.\n", 
				cdwThreadId, id->m_strAdsbArpt);
			NoteAdsbServerFailure(id);
		}
		return( SUCCEED );
	}
	id->m_dwAdsbFailCount = 0L;
	
	if ( !strncmp( id->m_strUserName, "sm", 2 ) ){  // take anything for now
		if('\x0A' == url[strlen(url)-1])url[strlen(url)-1] = 0; // removes newline		
		_snprintf_s( id->m_strLatestPassurURL, URL_SZ, _TRUNCATE, "ADS-B: %s", url );
		D_CONTROL("ADSB_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) %s", cdwThreadId, id->m_strLatestPassurURL));
	}

	if ( !GetCurlPage_r( id, curl_handle, &chunk, url, SM_HTTPS, 3, 4L ) ){
	
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Curl failed for <%>s\n", cdwThreadId, url );
		NoteAdsbServerFailure(id);
		free_chunk_memory(&chunk);
		return( FALSE );
	}

	ftime( &curl_end_time);

	if ( !chunk.size || !chunk.memory ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Error, null chunk size\n", cdwThreadId );
		NoteAdsbServerFailure(id);
		return( FALSE );
	}

	// ADS-B files can be much smaller than ASD/PASSUR files
	//@@@ CHECK
	if ( chunk.memory && chunk.size < 140 ){
	  vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Small ADS-B file found '%s'\n", cdwThreadId, chunk.memory );
		free_chunk_memory(&chunk);
		NoteAdsbServerFailure(id);
		return (FALSE);
	}	
	NoteAdsbServerSuccess(id);
	unzip_and_save_adsb_data(id, &chunk, url);

	if (id->m_bClass1 && !id->m_nDelaySecs) {
		// save the request time for a moving average
		request_time = uts_timediff(&curl_start_time, &curl_end_time);
		save_request_time(request_time, &(id->m_dADSBPrevRequest), id->m_dADSBRequestTimes,
			&(id->m_nADSBRequestIndex), id->m_nADSBRequestNum);
		// recalculate the average request time
		passur_request_average = get_average_request_time(id->m_dPassurRequestTimes, id->m_nPassurRequestNum);
		asdex_request_average = get_average_request_time(id->m_dADSBRequestTimes, id->m_nADSBRequestNum);
		if (passur_request_average < 500.0 && asdex_request_average < 500.0) {
			// internet connection fast enough to turn on fast updates
			id->m_bFastUpdates = TRUE;
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) FastUpdates enabled\n", cdwThreadId); 
		}
		D_CONTROL("ADSB_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) time %2gms avg %2gms for <%s>\n",
			cdwThreadId, request_time, asdex_request_average, url));
	}

	//if(id->m_bClass1)
	//	Callback2Javascript(id, "AsdexStatus:Slow");
	//else
	//	Callback2Javascript(id, "AsdexStatus:OK");
	return( SUCCEED );
}

static int GetAirAsiaData(  struct InstanceData* id, int force_refresh, CURL* curl_handle )
{
	time_t secs;
	struct MemoryStruct chunk;
	int CkLastTime = TRUE; // always check time interval for passur data
	int delaysecs;
	int replay_adjust_secs;
	char url[URL_SZ];
	char *outbufptr = NULL;
	char tmpstr[ TMPBUF_SIZE ];
	int loopcount;
	int urlLen;
	char LatestAirAsiacginame[256];
	struct timeb curl_start_time, curl_end_time;
	const DWORD cdwThreadId = GetCurrentThreadId();
	
	if(!id->m_sCurLayout.m_bShowAirAsiaAircraft) {
		return(SUCCEED);
	}

	if (!curl_handle) return FALSE;

	secs = time(0);
	
	if ( CkLastTime ){
		// see if we are in fast replay mode
		if ( id->m_nDelaySecs ){
			// we are in replay mode, use Plottime to retrieve next file
			// get AirAsia data 2 seconds ahead of plottime to prevent pauses
			if ( id->m_tAirAsiaFileTime < id->m_tPlotTime+2 ){
				// file is behind, get the next file
				D_CONTROL("AIRASIA_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) file is behind, Gfiletime=%d plottime=%d\n",
					cdwThreadId, id->m_tAirAsiaFileTime, id->m_tPlotTime ));
			} else if ( secs - id->m_tLastAirAsiaGetTime < 5 ){
				/* we already got it */
				//if(id->m_bClass1)
				//	Callback2Javascript(id, "AirAsiaStatus:Slow");
				//else
				Callback2Javascript(id, "AirAsiaStatus:OK");
				return(SUCCEED);
			}
		} else if ( secs - id->m_tLastAirAsiaGetTime < 5 ){
			/* we already got it */
			//Callback2Javascript(id, "AirAsiaStatus:Slow");
			return(SUCCEED);
		}
	}

	id->m_tLastAirAsiaGetTime = secs;

	loopcount = 0;
	do {
		id->m_strAirAsiaZipFilename[0] = '\0';
		if ( id->m_nDelaySecs || !id->m_bClass1 ){
			if ( !id->m_bClass1 ){
				delaysecs = MAX(id->m_nDelaySecs, 300);
			} else {
				delaysecs = id->m_nDelaySecs;
			}
			if ( id->m_nDelaySecs && id->m_nReplaySpeed && delaystartsecs){
				replay_adjust_secs = (int) (id->m_tClientsTime - delaystartsecs) *  (id->m_nReplaySpeed - 1);
				delaysecs -= replay_adjust_secs;
				D_CONTROL("AIRASIA_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) delaysecs reduced by %d\n", cdwThreadId, replay_adjust_secs ));
			}
			if ( delaysecs < 0 || (delaysecs < 300 && !id->m_bClass1) ){
				// we already got it 
				D_CONTROL("AIRASIA_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) delaysecs =%d, in future, replay past current time\n",
					cdwThreadId, delaysecs ));
				//if(id->m_bClass1)
				//	Callback2Javascript(id, "AirAsiaStatus:Slow");
				//else
				Callback2Javascript(id, "AirAsiaStatus:OK");
				return(SUCCEED);
			}
			// get Air Asia data 2 seconds ahead of plottime to prevent pauses
			sprintf_s(tmpstr, TMPBUF_SIZE, "arpt=%s&feed=%s&replay=%d", "KUL"/*id->m_strAdsbArpt*/, "airasia-tracks", delaysecs+2 );
		} else {
			sprintf_s(tmpstr, TMPBUF_SIZE, "arpt=%s&feed=%s", "KUL"/*id->m_strAdsbArpt*/, "airasia-tracks" );
		}
	
		clear_chunk_memory(&chunk);

		setCurlTimeout(curl_handle, 4L);
		sprintf_s(LatestAirAsiacginame, _countof(LatestAirAsiacginame), "%s/cgi-bin/latest.cgi",  GetAirAsiaSrvr(id));

        vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlFormGetResults - LatestAirAsiacginame <%s> tmpstr <%s>.\n", 
			cdwThreadId, LatestAirAsiacginame, tmpstr );

		if ( GetCurlFormGetResults_r( id, curl_handle, &chunk, LatestAirAsiacginame, tmpstr, SM_HTTPS, TRUE ) == FALSE ) {
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Call to GetCurlFormGetResults for url: <%s> failed, switching srvrs & trying again.\n", 
				cdwThreadId, LatestAirAsiacginame);
	
			GetNxtAirAsiaSrvr(id);
		}
		
		if ( chunk.memory && !strstr( chunk.memory, "404 Not Found" ) ){
			strncpy_s( id->m_strAirAsiaZipFilename, FILENAME_SZ, chunk.memory, 255 );
			free_chunk_memory( &chunk );
		} else {
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not get latest.cgi, trying again\n", cdwThreadId);
		}
	} while (!strlen(id->m_strAirAsiaZipFilename) && ++loopcount < 5 );
	if (loopcount >= 5) {
		NoteAirAsiaServerFailure(id);
		return( FALSE );
	}
	
	ftime( &curl_start_time);


	clear_chunk_memory(&chunk);

	sprintf_s(url, URL_SZ, "%s/%s", GetAirAsiaSrvr(id), id->m_strAirAsiaZipFilename ); 
	D_CONTROL("AIRASIA_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) AirAsia url <%s>\n", cdwThreadId, url ));

	urlLen = strlen(id->m_strPrevAirAsiaUrl);		
	// Used strncmp becuase the url returned from above may have ending newline character, which causes strcmp to fail
	if ( !strncmp( url, id->m_strPrevAirAsiaUrl, (urlLen > 0 ? urlLen: 1)) ){
		/* we already got this file */
		D_CONTROL("AIRASIA_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Already got this file(%s), skipping\n", cdwThreadId, url));
		// Fail Counts greater than 72 mean we are no longer getting new files, air asia feed is down.
		if(++id->m_dwAirAsiaFailCount > 72){
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Server succeeded but data file has not changed in 6 minutes.\n", 
				cdwThreadId);
			NoteAirAsiaServerFailure(id);
		}
		return( SUCCEED );
	}
	id->m_dwAirAsiaFailCount = 0L;
	
	if ( !strncmp( id->m_strUserName, "sm", 2 ) ){  // take anything for now
		if('\x0A' == url[strlen(url)-1])url[strlen(url)-1] = 0; // removes newline		
		_snprintf_s( id->m_strLatestPassurURL, URL_SZ, _TRUNCATE, "AirAsia: %s", url );
		D_CONTROL("AIRASIA_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) %s\n", cdwThreadId, id->m_strLatestPassurURL));
	}

	if ( !GetCurlPage_r( id, curl_handle, &chunk, url, SM_HTTPS, 3, 4L ) ){
	
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Curl failed for <%>s\n", cdwThreadId, url );
		NoteAirAsiaServerFailure(id);
		free_chunk_memory(&chunk);
		return( FALSE );
	}

	ftime( &curl_end_time);

	if ( !chunk.size || !chunk.memory ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Error, null chunk size\n", cdwThreadId );
		NoteAirAsiaServerFailure(id);
		return( FALSE );
	}

	NoteAirAsiaServerSuccess(id);
	unzip_and_save_airasia_data(id, &chunk, url);
	
	Callback2Javascript(id, "AirAsiaStatus:OK");
	return( SUCCEED );
}

static int GetAirAsiaFlightPlanData(  struct InstanceData* id, int force_refresh, CURL* curl_handle )
{
	time_t secs;
	struct MemoryStruct chunk;
	int CkLastTime = TRUE; // always check time interval for passur data
	int delaysecs;
	int replay_adjust_secs;
	char *outbufptr = NULL;
	char tmpstr[ TMPBUF_SIZE ];
	int loopcount;
	char LatestAirAsiacginame[256];
	int got_data = FALSE;
	const DWORD cdwThreadId = GetCurrentThreadId();
	
	if(!id->m_sCurLayout.m_bShowAirAsiaAircraft) {
		return(SUCCEED);
	}

	if (!curl_handle) return FALSE;

	secs = time(0);
	
	if ( CkLastTime ){
		// see if we are in fast replay mode
		if ( id->m_nDelaySecs ){
			// we are in replay mode, use Plottime to retrieve next file
			// get AirAsia data 2 seconds ahead of plottime to prevent pauses
			if ( id->m_tAirAsiaFlightPlanFileTime < id->m_tPlotTime+2 ){
				// file is behind, get the next file
				D_CONTROL("AIRASIA_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) file is behind, Gfiletime=%d plottime=%d\n",
					cdwThreadId, id->m_tAirAsiaFlightPlanFileTime, id->m_tPlotTime ));
			} else if ( secs - id->m_tLastAirAsiaFlightPlanGetTime < 5 ){
				/* we already got it */
				//if(id->m_bClass1)
				//	Callback2Javascript(id, "AirAsiaStatus:Slow");
				//else
				Callback2Javascript(id, "AirAsiaStatus:OK");
				return(SUCCEED);
			}
		} else if ( secs - id->m_tLastAirAsiaFlightPlanGetTime < 5 ){
			/* we already got it */
			//Callback2Javascript(id, "AirAsiaStatus:Slow");
			return(SUCCEED);
		}
	}

	id->m_tLastAirAsiaFlightPlanGetTime = secs;

	loopcount = 0;
	do {
		if ( id->m_nDelaySecs || !id->m_bClass1 ){
			if ( !id->m_bClass1 ){
				delaysecs = MAX(id->m_nDelaySecs, 300);
			} else {
				delaysecs = id->m_nDelaySecs;
			}
			if ( id->m_nDelaySecs && id->m_nReplaySpeed && delaystartsecs){
				replay_adjust_secs = (int) (id->m_tClientsTime - delaystartsecs) *  (id->m_nReplaySpeed - 1);
				delaysecs -= replay_adjust_secs;
				D_CONTROL("AIRASIA_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) delaysecs reduced by %d\n", cdwThreadId, replay_adjust_secs ));
			}
			if ( delaysecs < 0 || (delaysecs < 300 && !id->m_bClass1) ){
				// we already got it 
				D_CONTROL("AIRASIA_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) delaysecs =%d, in future, replay past current time\n",
					cdwThreadId, delaysecs ));
				//if(id->m_bClass1)
				//	Callback2Javascript(id, "AirAsiaStatus:Slow");
				//else
				Callback2Javascript(id, "AirAsiaStatus:OK");
				return(SUCCEED);
			}
			// get Air Asia data 2 seconds ahead of plottime to prevent pauses
			sprintf_s(tmpstr, TMPBUF_SIZE, "Action=AirAsiaFlightPlan&delay=%d", delaysecs+2 );
		} else {
			sprintf_s(tmpstr, TMPBUF_SIZE, "Action=AirAsiaFlightPlan" );
		}
	
		clear_chunk_memory(&chunk);

		setCurlTimeout(curl_handle, 4L);
		sprintf_s(LatestAirAsiacginame, _countof(LatestAirAsiacginame), "%s/fcgi/AirAsia.fcg",  GetAirAsiaSrvr(id));

        vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlFormGetResults - LatestAirAsiacginame <%s> tmpstr <%s>.\n", 
			cdwThreadId, LatestAirAsiacginame, tmpstr );

		if ( GetCurlFormGetResults_r( id, curl_handle, &chunk, LatestAirAsiacginame, tmpstr, SM_HTTPS, TRUE ) == FALSE ) {
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Call to GetCurlFormGetResults for url: <%s> failed, switching srvrs & trying again.\n", 
				cdwThreadId, LatestAirAsiacginame);
	
			GetNxtAirAsiaSrvr(id);
		}
		
		if ( chunk.memory && !strstr( chunk.memory, "404 Not Found" ) ){
			got_data = TRUE;
			EnterCriticalSection(&id->m_csAirAsiaSync);

			append_rec_buffer(&id->m_pAirAsiaFileBuf, &id->m_lAirAsiaFileNBytes, chunk.memory, chunk.size);
			
			D_CONTROL("AIRASIA_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) AirAsiaFileNBytes=%ld\n", cdwThreadId, id->m_lAirAsiaFileNBytes ));

			LeaveCriticalSection(&id->m_csAirAsiaSync);

			D_CONTROL("AIRASIA_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) AirAsia DATA:\n\n%s\n", cdwThreadId, chunk.memory));

			free_chunk_memory( &chunk );
		} else {
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not get latest.cgi, trying again\n", cdwThreadId);
		}
	} while (!got_data && ++loopcount < 5 );
	if (loopcount >= 5) {
		NoteAirAsiaServerFailure(id);
		return( FALSE );
	}
	
	NoteAirAsiaServerSuccess(id);
	
	Callback2Javascript(id, "AirAsiaStatus:OK");
	return( SUCCEED );
}

static int GetMLATDataFastUpdates( struct InstanceData* id, int force_refresh, CURL* curl_handle )
{
	struct MemoryStruct chunk;
	struct MemoryStruct hdrs;
	char url[URL_SZ];
	char MLATFastUpdatesCGIName[256];
	char tmpstr[ TMPBUF_SIZE ];
	char *outbufptr = NULL;
	char prevfile[256];
	char cur_arpt[5] = {0};	
	struct timeb curl_start_time, curl_end_time;
	double request_time, request_average;
	const DWORD cdwThreadId = GetCurrentThreadId();

	if (!curl_handle) return FALSE;

	if (!time_elapsed(&(id->m_sMLATReqStart), 1000.0)) return FALSE;
	
	id->m_tClientsTime = time(0);

	EnterCriticalSection(&id->m_csMLATSync);

	// Save current airport
	strcpy_s(cur_arpt, _countof(cur_arpt), id->m_strMLATArpt);

	sprintf_s(tmpstr, TMPBUF_SIZE, "arpt=%s&feed=%s", id->m_strMLATArpt, "mlat");

	if (id->m_strMLATPrevFile[0] != '\0') {
		strcat_s(tmpstr, TMPBUF_SIZE, "&prevfile=");
		strcat_s(tmpstr, TMPBUF_SIZE, id->m_strMLATPrevFile);
	}

	LeaveCriticalSection(&id->m_csMLATSync);

	ftime( &curl_start_time);
	
	/* clear out buffer */
	clear_chunk_memory(&chunk);
	clear_chunk_memory(&hdrs);

	setCurlTimeout(curl_handle, 4L);

	sprintf_s(MLATFastUpdatesCGIName, _countof(MLATFastUpdatesCGIName), "%s/fcgi/fastupdates.fcg", GetMlatSrvr(id)); 
	
    vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlFormGetResultsHdrs_r - MLATFastUpdatesCGIName <%s> tmpstr <%s>.\n", 
			cdwThreadId, MLATFastUpdatesCGIName, tmpstr );

	if ( GetCurlFormGetResultsHdrs_r(id, curl_handle, &chunk, &hdrs,
		MLATFastUpdatesCGIName, tmpstr, SM_HTTPS, TRUE ) == FALSE ) {

	  vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) <%s> failed tmpstr.<%s>\n", cdwThreadId, MLATFastUpdatesCGIName, tmpstr);

	  //Callback2Javascript(id, "AsdexStatus:FAIL");
	  // 
	  // Note server failure and get alternate if available
	  //

	  NoteMlatServerFailure(id);
	  free_chunk_memory(&chunk);
	  free_chunk_memory(&hdrs);
	  return( FALSE );
	} 

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Success <%s>.\n", cdwThreadId, MLATFastUpdatesCGIName);

	ftime( &curl_end_time );

	// save the request time for a moving average
	request_time = uts_timediff(&curl_start_time, &curl_end_time);
	save_request_time(request_time, &(id->m_dMLATPrevRequest), id->m_dMLATRequestTimes,
		&(id->m_nMLATRequestIndex), id->m_nMLATRequestNum);
	// check the average request time
	request_average = get_average_request_time(id->m_dMLATRequestTimes, id->m_nMLATRequestNum);
	if (request_average > 600.0) {
		// internet connection too slow for fast updates
		id->m_bFastUpdates = FALSE;
		//Callback2Javascript(id, "FastUpdates:OFF");
		//memset(id->m_dASDEXRequestTimes, 0, sizeof(id->m_dASDEXRequestTimes));
		vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) FastUpdates disabled (slow connection)\n", cdwThreadId); 
	}
	D_CONTROL("MLAT_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) time %2gms avg %2gms for <%s>\n",
		cdwThreadId, request_time, request_average, MLATFastUpdatesCGIName));

	
	find_filename_in_hdrs(hdrs, prevfile);
	find_filepath_in_hdrs(hdrs, id->m_strMLATZipFilename);

	free_chunk_memory(&hdrs);

	if (prevfile[0] == '\0') {
		// filename not in http headers - request failed
		// retry in 1 second
		free_chunk_memory(&chunk);
		id->m_sMLATReqStart = curl_end_time;
		// Fail Counts greater than 3 mean we are no longer getting new data, asde-x feed is down.
		if(++id->m_dwMLATFailCount > 3){
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Server succeeded but data file is empty, %s M-LAT feed may be down.\n", 
				cdwThreadId, id->m_strMLATArpt);
			NoteMlatServerFailure(id);
		}	
		return FALSE; 
	}
	id->m_dwMLATFailCount = 0L;
	
	EnterCriticalSection(&id->m_csMLATSync);
	// if airport didn't change, save prevfile for next time
	if (strcmp(cur_arpt, id->m_strMLATArpt) == 0) {
		strcpy_s(id->m_strMLATPrevFile, FILENAME_SZ, prevfile);
	}
	LeaveCriticalSection(&id->m_csMLATSync);

	sprintf_s(url, URL_SZ, "%s%s", GetMlatSrvr(id),  id->m_strMLATZipFilename ); 
	
	D_CONTROL("MLAT_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) MLAT url <%s>\n", cdwThreadId, url ));

	if ( !strncmp( id->m_strUserName, "sm", 2 ) ){  // take anything for now
		if('\x0A' == url[strlen(url)-1])url[strlen(url)-1] = 0; // removes newline		
		_snprintf_s( id->m_strLatestPassurURL, URL_SZ, _TRUNCATE, "M-LAT: %s", url );
		D_CONTROL("MLAT_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) got  %s\n", cdwThreadId, url));
	}	

	if ( !chunk.size || !chunk.memory ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Error, null chunk size\n", cdwThreadId );
		NoteMlatServerFailure(id);
		return( FALSE );
	}
	NoteMlatServerSuccess(id);
	unzip_and_save_mlat_data(id, &chunk, url);
    return( SUCCEED );
}


static int GetMLATData(  struct InstanceData* id, int force_refresh, CURL* curl_handle )
{
	time_t secs;
	struct MemoryStruct chunk;
	int CkLastTime = TRUE; // always check time interval for passur data
	int delaysecs;
	int replay_adjust_secs;
	char url[URL_SZ];
	char *outbufptr = NULL;
	char tmpstr[ TMPBUF_SIZE ];
	int loopcount;
	int urlLen;
	char LatestMlatcginame[256];
	struct timeb curl_start_time, curl_end_time;
	double request_time, passur_request_average, asdex_request_average;
	const DWORD cdwThreadId = GetCurrentThreadId();
	
	if(!id->m_sCurLayout.m_bShowMLATAircraft) {
		memset(id->m_dMLATRequestTimes, 0, sizeof(id->m_dMLATRequestTimes)); // clear request timings
		Callback2Javascript(id, "MlatStatus:Off");
		return(SUCCEED);
	}

	if (!curl_handle) return FALSE;

	if( !IsValidMLATAirport(id, id->m_strMLATArpt) ){
		Callback2Javascript(id, "MlatStatus:N/A");
		return FALSE;
	}
	
	if (id->m_bFastUpdates && id->m_bClass1 && !id->m_nDelaySecs) {
		return GetMLATDataFastUpdates(id, force_refresh, curl_handle);
	}

	secs = time(0);
	
	if ( CkLastTime ){
		// see if we are in fast replay mode
		if ( id->m_nDelaySecs ){
			// we are in replay mode, use Plottime to retrieve next file
			// get ASDEX data 2 seconds ahead of plottime to prevent pauses
			if ( id->m_tMLATFileTime < id->m_tPlotTime+2 ){
				// file is behind, get the next file
				D_CONTROL("MLAT_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) file is behind, Gfiletime=%d plottime=%d\n",
					cdwThreadId, id->m_tMLATFileTime, id->m_tPlotTime ));
			} else if ( secs - id->m_tLastMLATGetTime < 5 ){
				/* we already got it */
				if(id->m_bClass1)
					Callback2Javascript(id, "MlatStatus:Slow");
				else
					Callback2Javascript(id, "MlatStatus:OK");
				return(SUCCEED);
			}
		} else if ( secs - id->m_tLastMLATGetTime < 5 ){  // new passur data every 5 seconds -- one per thread
			/* we already got it */
			Callback2Javascript(id, "MlatStatus:Slow");
			return(SUCCEED);
		}
	}

	id->m_tLastMLATGetTime = secs;

	loopcount = 0;
	do {
		id->m_strMLATZipFilename[0] = '\0';
		if ( id->m_nDelaySecs || !id->m_bClass1 ){
			if ( !id->m_bClass1 ){
				delaysecs = MAX(id->m_nDelaySecs, 300);
			} else {
				delaysecs = id->m_nDelaySecs;
			}
			if ( id->m_nDelaySecs && id->m_nReplaySpeed && delaystartsecs){
				replay_adjust_secs = (int) (id->m_tClientsTime - delaystartsecs) *  (id->m_nReplaySpeed - 1);
				delaysecs -= replay_adjust_secs;
				D_CONTROL("MLAT_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) delaysecs reduced by %d\n", cdwThreadId, replay_adjust_secs ));
			}
			if ( delaysecs < 0 || (delaysecs < 300 && !id->m_bClass1) ){
				// we already got it 
				D_CONTROL("MLAT_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) delaysecs =%d, in future, replay past current time\n",
					cdwThreadId, delaysecs ));
				if(id->m_bClass1)
					Callback2Javascript(id, "MlatStatus:Slow");
				else
					Callback2Javascript(id, "MlatStatus:OK");
				return(SUCCEED);
			}
			// get ADSB data 2 seconds ahead of plottime to prevent pauses
			sprintf_s(tmpstr, TMPBUF_SIZE, "arpt=%s&feed=%s&replay=%d", id->m_strMLATArpt, "mlat", delaysecs+2 );
		} else {
			sprintf_s(tmpstr, TMPBUF_SIZE, "arpt=%s&feed=%s", id->m_strMLATArpt, "mlat" );
		}
	
		clear_chunk_memory(&chunk);

		setCurlTimeout(curl_handle, 4L);
		sprintf_s(LatestMlatcginame, _countof(LatestMlatcginame), "%s/cgi-bin/latest.cgi",  GetMlatSrvr(id));
		
        vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlFormGetResults - LatestMlatcginame <%s> tmpstr <%s>.\n", 
			cdwThreadId, LatestMlatcginame, tmpstr );

		if ( GetCurlFormGetResults_r( id, curl_handle, &chunk, LatestMlatcginame, tmpstr, SM_HTTPS, TRUE ) == FALSE ) {
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Call to GetCurlFormGetResults for url: <%s> failed, switching srvrs & trying again.\n", 
				cdwThreadId, LatestMlatcginame);

			/*---------------------------------------------------------------------*/
			/* Switch to next ADSB server                                        */
			/*---------------------------------------------------------------------*/
		
			GetNxtAdsbSrvr(id);
		}
		
		if ( chunk.memory && !strstr( chunk.memory, "404 Not Found" ) ){
			strncpy_s( id->m_strMLATZipFilename, FILENAME_SZ, chunk.memory, 255 );
			free_chunk_memory( &chunk );
		} else {
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not get latest.cgi, trying again\n", cdwThreadId);
		}
	} while (!strlen(id->m_strMLATZipFilename) && ++loopcount < 5 );
	if (loopcount >= 5) {
		NoteMlatServerFailure(id);
		return( FALSE );
	}
	
	ftime( &curl_start_time);

	/* clear out buffer */
	chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
	chunk.size = 0;    /* no data at this point */

	sprintf_s(url, URL_SZ, "%s/%s", GetMlatSrvr(id), id->m_strMLATZipFilename ); 
	D_CONTROL("MLAT_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) ADSB url <%s>\n", cdwThreadId, url ));

	urlLen = strlen(id->m_strPrevMLATUrl);		
	// Used strncmp becuase the url returned from above may have ending newline character, which causes strcmp to fail
	if ( !strncmp( url, id->m_strPrevMLATUrl, (urlLen > 0 ? urlLen: 1)) ){
		/* we already got this file */
		D_CONTROL("MLAT_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Already got this file(%s), skipping\n", cdwThreadId, url));
		// Fail Counts greater than 3 mean we are no longer getting new files, ads-b feed is down.
		if(++id->m_dwMLATFailCount > 3){
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Server succeeded but data file is empty, %s MLAT feed may be down.\n", 
				cdwThreadId, id->m_strMLATArpt);
			NoteMlatServerFailure(id);
		}
		return( SUCCEED );
	}
	id->m_dwMLATFailCount = 0L;
	
	if ( !strncmp( id->m_strUserName, "sm", 2 ) ){  // take anything for now
		if('\x0A' == url[strlen(url)-1])url[strlen(url)-1] = 0; // removes newline		
		_snprintf_s( id->m_strLatestPassurURL, URL_SZ, _TRUNCATE, "MLAT: %s", url );
		D_CONTROL("MLAT_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) %s", cdwThreadId, id->m_strLatestPassurURL));
	}

	if ( !GetCurlPage_r( id, curl_handle, &chunk, url, SM_HTTPS, 3, 4L ) ){
	
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Curl failed for <%>s\n", cdwThreadId, url );
		NoteMlatServerFailure(id);
		free_chunk_memory(&chunk);
		return( FALSE );
	}

	ftime( &curl_end_time);

	if ( !chunk.size || !chunk.memory ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Error, null chunk size\n", cdwThreadId );
		NoteMlatServerFailure(id);
		return( FALSE );
	}

	// MLAT files can be much smaller than ASD/PASSUR files
	//@@@ CHECK
	if ( chunk.memory && chunk.size < 140 ){
	  vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Small M-LAT file found '%s'\n", cdwThreadId, chunk.memory );
		free_chunk_memory(&chunk);
		NoteMlatServerFailure(id);
		return (FALSE);
	}	
	NoteMlatServerSuccess(id);
	unzip_and_save_mlat_data(id, &chunk, url);

	if (id->m_bClass1 && !id->m_nDelaySecs) {
		// save the request time for a moving average
		request_time = uts_timediff(&curl_start_time, &curl_end_time);
		save_request_time(request_time, &(id->m_dMLATPrevRequest), id->m_dMLATRequestTimes,
			&(id->m_nMLATRequestIndex), id->m_nMLATRequestNum);
		// recalculate the average request time
		passur_request_average = get_average_request_time(id->m_dPassurRequestTimes, id->m_nPassurRequestNum);
		asdex_request_average = get_average_request_time(id->m_dADSBRequestTimes, id->m_nADSBRequestNum);
		if (passur_request_average < 500.0 && asdex_request_average < 500.0) {
			// internet connection fast enough to turn on fast updates
			id->m_bFastUpdates = TRUE;
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) FastUpdates enabled\n", cdwThreadId); 
		}
		D_CONTROL("MLAT_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) time %2gms avg %2gms for <%s>\n",
			cdwThreadId, request_time, asdex_request_average, url));
	}

	//if(id->m_bClass1)
	//	Callback2Javascript(id, "AsdexStatus:Slow");
	//else
	//	Callback2Javascript(id, "AsdexStatus:OK");
	return( SUCCEED );
}

static int GetNoiseData(  struct InstanceData* id, int force_refresh, CURL* curl_handle )
{
	time_t secs;
	struct MemoryStruct chunk;
	int delaysecs;
	int replay_adjust_secs;
	char url[URL_SZ];
	char *outbufptr = NULL;
	char tmpstr[ TMPBUF_SIZE ];
	int loopcount = 0;
	int urlLen;
	char LatestNoisecginame[256];
	const DWORD cdwThreadId = GetCurrentThreadId();

	if(!id->m_sCurLayout.m_bShowNoiseAircraft) {
		return(SUCCEED);
	}
	if (!curl_handle) return FALSE;

	secs = time(0);
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) DelaySecs(%d), Class1(%d).\n", 
				cdwThreadId, id->m_nDelaySecs, id->m_bClass1 );	

	// Create Base Noise Data Path
	sprintf_s(tmpstr, TMPBUF_SIZE, "data/noiseclass2/%s", id->m_strNoiseArpt );
	
	if( id->m_nDelaySecs || !id->m_bClass1 ){
		struct tm tmFile;
		if ( !id->m_bClass1 ){
			delaysecs = MAX(id->m_nDelaySecs, 300);
		} else {
			delaysecs = id->m_nDelaySecs;
		}
		id->m_strNoiseZipFilename[0] = '\0';
		if ( id->m_nDelaySecs && id->m_nReplaySpeed && delaystartsecs){
			replay_adjust_secs = (int) (id->m_tClientsTime - delaystartsecs) *  (id->m_nReplaySpeed - 1);
			delaysecs -= replay_adjust_secs;
			D_CONTROL("NOISE_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) delaysecs reduced by %d\n", cdwThreadId, replay_adjust_secs ));
		}
		
		id->m_tLastNoiseGetTime = secs - delaysecs;
		if(!gmtime_s(&tmFile, &id->m_tLastNoiseGetTime)){
			// build Filename from tm struct
			sprintf_s(id->m_strNoiseZipFilename, 128, "%d/%d/%d/%02d/%02d.zip", 
				tmFile.tm_year + 1900,
				tmFile.tm_mon + 1,
				tmFile.tm_mday,
				tmFile.tm_hour,
				tmFile.tm_min);
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Created Latest Noise Zip File Path <%s>.\n", 
					cdwThreadId, id->m_strNoiseZipFilename );	
		}else{
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) gmtime_s failed for <%s>.\n", 
				cdwThreadId, id->m_tLastNoiseGetTime );
			return (FAIL);
		}

	}else{
		id->m_tLastNoiseGetTime = secs;
		loopcount = 0;
		do {
			id->m_strNoiseZipFilename[0] = '\0';
			
			clear_chunk_memory(&chunk);

			sprintf_s(LatestNoisecginame, _countof(LatestNoisecginame), "%s/%s/latest.txt",  GetNoiseSrvr(id), tmpstr);

			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlFormGetResults - LatesNoisecginame <%s>.\n", 
				cdwThreadId, LatestNoisecginame);

			if ( GetCurlPage_r( id, curl_handle, &chunk, LatestNoisecginame, SM_HTTPS, 1, 60 ) == FALSE ) {
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Call to GetCurlPage for url: <%s> failed, switching srvrs & trying again.\n", 
					cdwThreadId, LatestNoisecginame);
				GetNxtNoiseSrvr(id);
			}
			
			if ( chunk.memory && chunk.size < FILENAME_SZ && !strstr(chunk.memory, "404 Not Found")){
				strncpy_s( id->m_strNoiseZipFilename, FILENAME_SZ, chunk.memory, _TRUNCATE );
				free_chunk_memory( &chunk );
				vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Retrieved Latest Noise Zip File Path <%s>.\n", 
					cdwThreadId, id->m_strNoiseZipFilename );	
			} else {
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not get latest.txt, trying again (%d)\n", cdwThreadId, loopcount);
				free_chunk_memory( &chunk );
			}
		} while (!strlen(id->m_strNoiseZipFilename) && ++loopcount < 3 );

		if (loopcount >= 3) {
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not get latest.txt\n", cdwThreadId);		
			return( FALSE );
		}
	}	

	/* clear out buffer */
	chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
	chunk.size = 0;    /* no data at this point */

	sprintf_s(url, URL_SZ, "%s/%s/%s", GetNoiseSrvr(id), tmpstr, id->m_strNoiseZipFilename ); 
	D_CONTROL("NOISE_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Noise url <%s>\n", cdwThreadId, url ));

	urlLen = strlen(id->m_strPrevNoiseUrl);		
	// Used strncmp becuase the url returned from above may have ending newline character, which causes strcmp to fail
	if ( !strncmp( url, id->m_strPrevNoiseUrl, (urlLen > 0 ? urlLen: 1)) ){
		/* we already got this file */
		D_CONTROL("NOISE_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Already got this file(%s), skipping\n", cdwThreadId, url));
		return( SUCCEED );
	}
	
	if ( !strncmp( id->m_strUserName, "sm", 2 ) ){  // take anything for now
		_snprintf_s( id->m_strLatestPassurURL, URL_SZ, _TRUNCATE, "NOISE: %s", url );
		D_CONTROL("NOISE_DNLD", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) %s", cdwThreadId, id->m_strLatestPassurURL));
	}

	loopcount = 0;
	do{
		if ( !GetCurlPage_r( id, curl_handle, &chunk, url, SM_HTTPS, 1, 59L ) ){
		
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Call to GetCurlPage for url: <%s> failed, switching srvrs & trying again.\n", 
					cdwThreadId, url);
			free_chunk_memory(&chunk);
			GetNxtNoiseSrvr(id);
		}
	}while((!chunk.size || !chunk.memory) && ++loopcount < 3);

	if (loopcount >= 3) {
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not get url:<%s>\n", cdwThreadId, url);		
		return( FALSE );
	}else if ( !chunk.size || !chunk.memory ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Error, null chunk size\n", cdwThreadId );
		return( FALSE );
	}

	unzip_and_save_noise_data(id, &chunk, url);

	return( SUCCEED );
}
