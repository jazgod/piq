// WT.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include <crypto.h>
#include <io.h>
#include "InstanceData.h"
#include "SSLThreadSupport.h"
#include "dbg_mem.h"
#include "ARTCC.h"

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

extern FILE* vo_init_files(  char* strFileDateFmt, char* strErrFilename, int debug_mode, int debug_flag );
extern int vo_log_info( ... );
extern int vo_log_error( ... );
extern unsigned _stdcall BackgroundThread(PVOID pvoid);
extern void CreateDir(char* Path);
extern void FreeGUI(struct InstanceData* id);
extern char *GetPulseSrvr(struct InstanceData* id);
extern char *GetNxtPulseSrvr(struct InstanceData* id);
extern char *GetLastErrorStr();
extern char* GetLastErrorString(char* szErrorString, int nLen, BOOL bIncludeValue);
extern void GetSurfServerLists(struct InstanceData* id);
extern int IsGuiMouseEvent(struct InstanceData* id, int b, int m, int x, int y, int s);
extern void LoadUserData(struct InstanceData* id);
extern void LogToServer(struct InstanceData* id, enum LOG_LEVEL eLevel, const char* strCategory, const char* strLogData);
//extern BOOL _stdcall Message2DlgPolyCB(HWND hdlg, UINT message, WPARAM wParam, LPARAM lparam);
extern void mymouse(struct InstanceData* id, int b, int m, int x, int y);
extern void myRmouse(struct InstanceData* id, int b, int m, int x, int y);
extern void ReleaseAL(struct InstanceData* id);
extern void RenderScene(struct InstanceData* id);
extern int resizeWindow( struct InstanceData* id, int width, int height );
extern void	setinsightServerVars( struct InstanceData* id );
extern void setWebTrackerServerVars( struct InstanceData* id );
extern void RotateLogFile(struct InstanceData* id);
extern void ClosePrevLogFile(struct InstanceData* id);
extern unsigned _stdcall WebTrackerInitThread(PVOID pvoid);
extern int WindowProc(struct InstanceData* id, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern char version[];
extern char builddate[];

int g_nInstanceCount = 0; // Count of NPAPI Instances
int g_nInstanceCountTotal = 0; // Count of TOTAL NPAPI Instances while DLL is Loaded

__declspec(dllexport) void __cdecl GetDLLVersion(DWORD* pdwMajor, DWORD* pdwMinor)
{
	// Initialize to minimum 1.0.0.1 in case of errors
	*pdwMajor = MAKELONG(0,1);
	*pdwMinor = MAKELONG(1,0);
	TCHAR szFilename[100] = {0};
	HMODULE hModule = NULL;
	if(GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)GetDLLVersion, &hModule)){
		DWORD dwResult = GetModuleFileName(hModule, szFilename, 100);		
		if(dwResult != 0)
		{
			DWORD dwHandle = NULL;
			DWORD dwSize = GetFileVersionInfoSize(szFilename, &dwHandle);
			if(dwSize)
			{
				LPVOID lpData = new TCHAR[dwSize];
				if(GetFileVersionInfo(szFilename, dwHandle, dwSize, lpData))
				{
					VS_FIXEDFILEINFO *pBuffer = NULL;
					UINT uLen = 0;
					if(VerQueryValue(lpData, "\\", (LPVOID*)&pBuffer, &uLen))
					{
						*pdwMajor = pBuffer->dwFileVersionMS;
						*pdwMinor = pBuffer->dwFileVersionLS;
					}
				}
				delete lpData;
			}			
		}
	}else{
		OutputDebugString(GetLastErrorStr());
	}
}

void local_free( void* p )
{
	free(p);
}

void TempLoadUserLayout(struct InstanceData* id)
{
	// DO Nothing for now, can be used to perform other duties if needed. No longer used to load layouts.
}

void SavePreviousLogs(struct InstanceData* id)
{
	// Do NOT use vo_log here, it is not yet available.
	char strPreFile[MAX_PATH] = {0};
	char strNewFile[MAX_PATH] = {0};
	
	// Remove Old Logfile's no longer used.
	sprintf_s(strPreFile, MAX_PATH, "%s\\logs\\LogSurfaceWT_.txt", id->m_strPassurOutPath);
	if( 0 == _access_s(strPreFile, 0) ){
		remove(strPreFile);
	}

	sprintf_s(strPreFile, MAX_PATH, "%s\\logs\\LogSurfaceWT_.bak", id->m_strPassurOutPath);
	if( 0 == _access_s(strPreFile, 0) ){
		remove(strPreFile);
	}

	sprintf_s(strPreFile, MAX_PATH, "%s\\logs\\LogSurfaceWT_A_.txt", id->m_strPassurOutPath);
	if( 0 == _access_s(strPreFile, 0) ){
		remove(strPreFile);
	}

	sprintf_s(strPreFile, MAX_PATH, "%s\\logs\\LogSurfaceWT_A_.bak", id->m_strPassurOutPath);
	if( 0 == _access_s(strPreFile, 0) ){
		remove(strPreFile);
	}

	// Check for previous log file
	sprintf_s(strPreFile, MAX_PATH, "%s\\logs\\LogSurfaceWT%d_.txt", id->m_strPassurOutPath, id->m_nInstanceId);
	if( 0 == _access_s(strPreFile, 0) ){
		// Check for previous backup
		sprintf_s(strNewFile, MAX_PATH, "%s\\logs\\LogSurfaceWT%d_.bak", id->m_strPassurOutPath, id->m_nInstanceId);	
		if( 0 == _access_s(strNewFile, 0) ){
			// Remove old backup
			remove(strNewFile);
		}
		// Save previous logfile
		rename(strPreFile, strNewFile);
	}
	
	// Check for previous rollover log file
	sprintf_s(strPreFile, MAX_PATH, "%s\\logs\\LogSurfaceWT%d_A_.txt", id->m_strPassurOutPath, id->m_nInstanceId);
	if( 0 == _access_s(strPreFile, 0) ){
		// Check for previous rollover backup
		sprintf_s(strNewFile, MAX_PATH, "%s\\logs\\LogSurfaceWT%d_A_.bak", id->m_strPassurOutPath, id->m_nInstanceId);	
		if( 0 == _access_s(strNewFile, 0) ){
			// Remove old rollover backup
			remove(strNewFile);
		}
		// Save previous rollover logfile
		rename(strPreFile, strNewFile);
	}
}

__declspec(dllexport) void* __cdecl Initialize(struct InstanceData** ppInst, int nSize)
{
	// Create InstanceData for this dll, return to caller
	struct InstanceData* id =  NULL;
	struct InstanceData* pTemp =  *ppInst;
	const DWORD cdwThreadId = GetCurrentThreadId();
	dbg_mem_init();

	id = (struct InstanceData*) calloc(1, sizeof(struct InstanceData));
	InitializeInstanceData(id);
	memcpy(id, pTemp, nSize);
	id->m_nInstanceId = ++g_nInstanceCountTotal;
	
	SavePreviousLogs(id);

	_snprintf_s(id->m_strErrFilename, 128, _TRUNCATE, "%s\\logs\\LogSurfaceWT%d", id->m_strPassurOutPath, id->m_nInstanceId);
	id->m_pfVoLogFile = vo_init_files( id->m_strErrFileDateFmt, id->m_strErrFilename, 0, 1 );
	sprintf_s(id->m_strLogfileName, 256, "%s_%s", id->m_strErrFilename, id->m_strErrFileDateFmt);
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Starting Initialize in WT.dll.\n", cdwThreadId);
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Web Tracker DLL Version <%s>.\n", cdwThreadId, id->m_strWebTrackerVersion);
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Web Tracker DLL Timestamp <%s %s>.\n", cdwThreadId, __DATE__, __TIME__);
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Web Tracker Version <%s>.\n", cdwThreadId, version);
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Web Tracker Build Date <%s>.\n", cdwThreadId, builddate);

	// Connect Function Pointers to wt.dll addresses
	id->InnerBackgroundThread = BackgroundThread;
	id->InnerClosePrevLogFile = ClosePrevLogFile;
	id->InnerFree = local_free;
	
	// Upgrade, Swapping Load of User filters with LoadUserData enhancement
	id->InnerLoadUserFilters = LoadUserData;
	id->InnerLoadUserLayout = TempLoadUserLayout;

	id->InnerRenderScene = RenderScene;
	id->InnerResizeWindow = resizeWindow;
	id->InnerRotateLogFile = RotateLogFile;
	id->InnerWindowProc = WindowProc;
	*ppInst = id;

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Initialize complete in WT.dll.\n", cdwThreadId);
	return (pTemp);
}

__declspec(dllexport) void __cdecl Shutdown(struct InstanceData* id)
{
	FILE* pfVoLogFile = id->m_pfVoLogFile;
	DWORD dwResult = 0;		
	const DWORD cdwThreadId = GetCurrentThreadId();
	vo_log_info(pfVoLogFile, __FUNCTION__, "(0x%X) Starting Shutdown in WT.dll.\n", cdwThreadId);	

	if(0 == --g_nInstanceCount){
		ReleaseARTCCs( id );
		ReleaseAL(id);
		FreeGUI(id);
	}
	// Release AppLogThread Here, after all other threads have completed
	vo_log_info(pfVoLogFile, __FUNCTION__, "(0x%X) Terminating AppLogThread2 in WT.dll.\n", cdwThreadId);	
	SetEvent(id->m_evAppLogEnd);		
	dwResult = WaitForSingleObject((HANDLE)id->m_hThreadAppLog, INFINITE);
	if(WAIT_OBJECT_0 != dwResult){
		char buffer[200] = {0};
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) WaitForSingleObject failed for AppLogThread 0x%X - 0x%X %s\n", cdwThreadId, id->m_hThreadAppLog, dwResult, GetLastErrorString(buffer, 200, TRUE));
	}
	CloseHandle(id->m_evAppLogEnd);
	id->m_evAppLogEnd = NULL;
	CloseHandle((HANDLE)id->m_hThreadAppLog);
	id->m_hThreadAppLog = NULL;

	// Release InstanceData* allocated in dll
	ReleaseInstanceData(id);
	free(id);

	vo_log_info(pfVoLogFile, __FUNCTION__, "(0x%X) Shutdown complete in WT.dll.\n", cdwThreadId);
	// Close logfile pointer
	fclose(pfVoLogFile);

	if(0 == --g_nInstanceCount){
		openssl_thread_cleanup( );
	}
	
	dbg_mem_complete();
}

__declspec(dllexport) void __cdecl Startup(struct InstanceData* id)
{
	const DWORD cdwThreadId = GetCurrentThreadId();

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Startup in WT.dll.\n", cdwThreadId);
	
	// Initialize OpenSSL for Thread Support	
	if(1 == ++g_nInstanceCount)
		openssl_thread_setup( );
	
	// create the folder for the cached files (per user)
	sprintf_s(id->m_strCachedDir, MAX_PATH_SZ, "%s\\data\\users\\%s", id->m_strPassurOutPath, id->m_strUserName);
	CreateDir(id->m_strCachedDir);

	// create the folder for the video files
	sprintf_s(id->m_strVideoDir, MAX_PATH_SZ-1, "%s\\video", id->m_strPassurOutPath);
	CreateDir(id->m_strVideoDir);

	setWebTrackerServerVars(id);
	GetSurfServerLists(id);
	setinsightServerVars(id);

	LogToServer(id, LOG_INFO, "STARTUP", "Web Tracker started.");
	
	_beginthreadex(NULL, 0, WebTrackerInitThread, id, 0, NULL);

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Startup complete in WT.dll.\n", cdwThreadId);
}

#ifdef __cplusplus
}
#endif
