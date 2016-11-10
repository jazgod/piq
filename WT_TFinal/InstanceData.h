#ifndef _INSTANCE_DATA_H_
#define _INSTANCE_DATA_H_
/**
 *==============================================================================
 *
 * Filename:         InstanceData.h
 *
 *==============================================================================
 *
 *  Copyright 2011  PASSUR Aerospace Inc. All Rights Reserved
 *
 *==============================================================================
 *
 * Description:
 *    Shared Resource Data Structure
 *
 *==============================================================================
 * 
 * Revision History:
 *
 * MCT 7/22/11 Created
 *==============================================================================
 */

#include <windows.h> 
#include <stdio.h>
#include <GL/gl.h>
#include <curl/curl.h>
#include <sys/timeb.h>
#include "structs.h"
#include "SurfaceC.h"
#include "Wingdi.h"
#include "vfw.h"
#include "AVILib.h"



#ifdef __cplusplus 

// C++ recognizes ellipses, C the va_alist parameter
#ifndef va_alist
#define va_alist ...
#endif

extern "C"{
#endif 

enum LOG_LEVEL{  
    LOG_DEBUG,      //debug-level message
    LOG_INFO,       //informational message
    LOG_NOTICE,     //normal, but significant, condition
    LOG_WARNING,    //warning conditions
    LOG_ERR,        //error conditions
	LOG_CRIT       //critical conditions
};

#ifndef MAX_PASSURS
#define MAX_PASSURS 3  
#endif//MAX_PASSURS

#define MAX_X_GRIDS 24
#define MAX_Y_GRIDS 24
#define N_AIRCRAFT_COLORS 17
#define N_VEHICLE_COLORS 16
#define N_AC_GROUPS 4
	
#define BROWSER_SZ 100
#define ARPTNAME_SZ 5
#define CERT_FILENAME_SZ 30
#define FILENAME_SZ 128
#define FOLDERNAME_SZ 15
#define FONTCOUNT 20
#define GUI_FONTCOUNT 20
#define LINE_BUFSIZE 4096
#define PASSIVE_RC_COUNT 10
#define NAMESIZE 512
#define SERVERNAME_SZ 132
#define SESSIONID_SZ 65
#define STATUSSTRING_SZ 100
#define USERNAME_SZ 13
#define URL_SZ 256
#define VERSION_SZ 100
#define MAX_PATH_SZ 256
#define PERM_SZ 25

#define REPLAY_STATUS_STOPPED	0
#define REPLAY_STATUS_PLAYING	1
#define REPLAY_STATUS_PAUSED	2

typedef struct OpenGLFont
{
	HFONT		m_hFont;
	TEXTMETRIC	m_tmCustom;
	GLuint		m_glListBase;
} OPENGLFONT;

typedef struct FreeTypeFont
{
	float	m_fHeight;
	GLuint* m_glTextures;
	GLuint	m_glListBase;
} FREETYPEFONT;

typedef struct settingsstruct {
  int TrailCount;
  int GraphicsNiceValue;
  int MapGraphics;
  int RefreshSpeed;
  int Extrapolation;
  int WxIntensityIVal;
  int TimeDisplayLocal;  // if TRUE, display all times in local computer time, otherwise ZULU
  int DisplayDegrees;   // if TRUE, display bottom border location in degrees,minutes and seconds, otherwise as decimal
  int ShowTerrain;		// True - display shaded relief terrain, otherwise just plain fill
  int TerrainColor;		// color set to use for terrain
  int LKP;
} USERSETTINGS;

typedef enum {
	MODE_NONE = 0,
	MODE_POLY,
	MODE_MEASURE,
	MODE_LAST /* never use */
} MODEcodes;

#define LOGIN_MAX_SZ	256		// Max user id and passwd string lengths
typedef struct CLoginRecord
{
	char	User[LOGIN_MAX_SZ];
	char	Password[LOGIN_MAX_SZ];
} LOGIN_RECORD;

typedef struct linkstruct {
  struct linkstruct *np;
  char mapfilename[NAMESIZE];
  GLfloat x1, y1, x2, y2;
  int maptexture;
  int nStatus;					// Short-circuit status, 0 - Ok, 1 - Error
} MAPLL;

typedef struct custom_map_data {
  struct custom_map_data *np;
  char arpt[ARPTNAME_SZ];		// Airport Identifier
  int start;					// Start Index Value
  int count;					// Count of Maps
  int type;						// GL_POLYGON or GL_LINE_STRIP
} CUSTOM_MAP_DATA;
extern int g_nMode; //@@ Need to Review, used in windows api callback


// Instance Data Struct, Used to share resources between functions, w/o using globals
struct InstanceData{
	/////////////////////////////////////////////////////////////////////////////
	// npSM.dll Component, DO NOT MODIFY unless OuterInstanceData struct changes
	// ORDER/Type MATTERS! Must be exact copy of InstanceData from OuterInstanceData!
	/////////////////////////////////////////////////////////////////////////////
		int m_nVersion;		// Version number for this struct
		// Function Pointers
		// The Following are used to call to SM ActiveX shell
		void (*OuterRegisterThreadHandle)(struct InstanceData* id, uintptr_t hThread, const char* strFuncName);
		void (*OuterUnregisterThreadHandle)(struct InstanceData* id, uintptr_t hThread);
		int (*OuterGetConfigValue)(struct InstanceData* id, const char* strName, char* value, int len);
		int (*OuterGetJavascriptValue)(struct InstanceData* id, const char* name, char* value, int len);
		int (*OuterGetLocalFile)( struct InstanceData* id, const char* localpath, const char* filename, const char* remotepath, int eMode);
		int (*OuterIsDebugNameDefined)(struct InstanceData* id, const char* strDebugName);
		int (*OuterIsMapSupported)(struct InstanceData* id, const char* strMapName);
		void (*OuterCallback2Javascript)(struct InstanceData* id, char* s);
		void (*OuterRegisterGetDataCallback)(char* szType, CnGetDataCb pfn);
		void (*OuterRegisterSendMessageCallback)(char* p, CnSendMessageCb pfn);
		void (*OuterGLPrint)(const char *fmt, ...);										// Custom GL "Print" Routine
		void (*OuterGLPrintCustom)(const char *fmt, ...);								// Custom GL "Print" Routine
		void (*OuterLogToServer)(struct InstanceData* id, enum LOG_LEVEL eLevel, const char* strCategory, const char* strLogData);

		// The following are used to call into the WT Dll
		unsigned (_stdcall *InnerBackgroundThread)(PVOID pvoid);
		void (*InnerClosePrevLogFile)(struct InstanceData* id);
		void (*InnerFree)(void* pMemory);
		void (*InnerLoadUserFilters)(struct InstanceData* id);
		void (*InnerLoadUserLayout)(struct InstanceData* id);
		void (*InnerRenderScene)(struct InstanceData* id);
		int (*InnerResizeWindow)( struct InstanceData* id, int width, int height );
		void (*InnerRotateLogFile)(struct InstanceData* id);
		int (*InnerWindowProc)(struct InstanceData* id, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		// 
		void* m_pControl;						// Used to refer back to control object	
		//
		int m_bOnRefresh;						// Refresh Webpage flag
		int m_bWTLoaded;						// WT.dll Loaded flag
		int m_bInitialized;						// Initialize flag
		int m_bUserHasLondon;					// London Flag
		int m_bClass1;							// Class1 User status flag
		int m_bDHSPerm;							// user has DHS permissions	
		int m_bEnableScrollGUI;
		int m_bTimestampLogFile;				// Enable Timestamp of logfiles
		int m_bSized;							// Control has Height and Width	
		int m_nControlId;						// Control Count Identifier for serializing startup and shutdown
		int m_nDebugControl;					// VO Debug Mode Flag, replaces debug_control
		int m_nNetworkTest;						// Network Test flag
		int m_nWidth;							// Window Width
		int m_nHeight;							// Window Height	
		int m_nMaxFilters;						// Maximum number of filters supported
		int m_nMeasureX1;
		int m_bClearScreen;
		int m_nMode;
		int m_nScrollCapture;
		int m_nViewPortWidth;					// Screen Viewport Width
		int m_nViewPortHeight;					// Screen Viewport Height
		int m_nViewPortXOrig;					// Screen Vieport X Origin
		int m_nViewPortYOrig;					// Screen Vieport Y Origin
		int m_nLogRecycleSeconds;  				// Log recycle in Seconds
		int m_bAsdexAll;						// User has access to all ASDEX data feeds
		int m_nAsdexArpt;						// User has access to a single ASDEX feed (per airport, -1 NotSet, 0 FALSE, 1 TRUE)		
		
		//DWORDS
		DWORD m_dwBkThread;						// Background Thread Timeout
		DWORD m_dwAdThread;						// Aircraft Data Thread Timeout
		DWORD m_dwTimer;						// Graphics Loop Timer Value
		DWORD g_dwDoNotUse1;					// AppLog Id for Message Passing
		
		//doubles
		double m_dWorldXmin;					// World in lat/lng coordinates	
		double m_dWorldXmax;					// World in lat/lng coordinates	
		double m_dWorldYmin;					// World in lat/lng coordinates	
		double m_dWorldYmax;					// World in lat/lng coordinates	
		double m_dWorldXSize;					// World in lat/lng dimension
		double m_dWorldYSize;					// World in lat/lng dimension
		double m_dWidth;
		double m_dHeight;
		
		// FILE*
		// NOTE: These file pointers can only be used by npSM.dll, not WT.dll because we statically link to
		//		 the c-lib in each dll (each has it's own file pointer mgmt routines)
		FILE* m_pfDoNotUse;						// VO Logfile Pointer 
		FILE* m_pfDoNotUse2;					// Previous VO Logfile Pointer
		
		// Handles
		HWND  m_hWnd;							// Window Handle
		HWND  m_hMainWnd;						// Main Window Handle	
		HDC   m_hDC;							// Device Context to Window
		HGLRC m_hRCActive;						// OpenGL Render Context For Visual Drawing
		HGLRC m_hRCPassive[PASSIVE_RC_COUNT];	// OpenGL Render Context For Loading Graphics
		HANDLE m_evShutdown;					// Shutdown Event Signal
		HPALETTE m_hPalette;					// ActiveX Color Palette
		HINSTANCE m_hInstance;					// Application Instance Handle
		HCURSOR m_hCursors[3];					// Handle to Used cursors
		HMODULE m_hWTModule;					// Handle to WT dll
		HANDLE m_evDoNotUse1;					// Event to signal AppLog has been started
		HANDLE m_evDoNotUse2;					// Event to signal AppLog to exit
		uintptr_t m_hDoNotUse1;					// handle to AppLog Thread
			
		// Strings/Char arrays
		char m_strOSVersion[VERSION_SZ];			// OS Version
		char m_strBrowserVersion[BROWSER_SZ];		// Browser Version
		char m_strSurfaceVersion[VERSION_SZ];		 // npSM.Dll Version
		char m_strWebTrackerVersion[VERSION_SZ];	 // WT.Dll Version
		char m_strCurlupwd[256]; // Curl user passwd
		char m_strCurluser[64];  // Curl login
		char m_strCurlpwd[64];   // Curl passwd
		char m_strHttpActiveXservername[SERVERNAME_SZ];	// Server Pathname
		char m_strStatusString[STATUSSTRING_SZ];			// Startup status text
		char m_strHttpBasePath[FOLDERNAME_SZ];				// Server application base path 'surf'
		char m_strDoNotUse[SERVERNAME_SZ];	
		char m_strPassurOutPath[512];
		char m_strIExplorerPath[512];	
		char m_strUserName[USERNAME_SZ];
		char m_strSessionId[SESSIONID_SZ];
		char m_strCertFilePath[FILENAME_SZ];			// SSL certificate file path
		char m_strCertFileName[CERT_FILENAME_SZ];
		char m_strSysDir[256];	
		char m_strHttpSysPath[FOLDERNAME_SZ];
		char m_strLibPath[128];
		char m_strDoNotUse1[256];				// log file name, recycles itself to avoid filling disk 
		char m_strLogfileInfo[256];				// log file name, recycles itself to avoid filling disk 
		char m_strDoNotUse2[256];				// log file previous name
		char m_strLogFileNameSuffix[128];
		char m_strDoNotUse3[128];				// Used in place of G_err_filename
		char m_strErrFileDateFmt[128];			// Used in place of GErrFileDateFmt
		char m_strLatestExecName[128];
		char m_strWxZipname[256];
		char m_strImageDir[256];
		char m_strMapDir[256];
		char m_strFontDir[256];
		char m_strSoundDir[256];
		
		// structs
		void* m_sInetProxyInfo;					//INTERNET_PROXY_INFO*	
		void* m_sCurlCachedProxyOpts;			//struct curlCachedProxyOpt*	
		
		TEXTMETRIC m_tmDoNotUse;
		LOGIN_RECORD m_sLogin;

		CRITICAL_SECTION m_csFileSync;			// Synchronize File List Elements 
		CRITICAL_SECTION m_csStatusSync;		// Synchronize Status String
	/////////////////////////////////////////////////////////////////////////////
	// npSM.dll Component, DO NOT MODIFY unless OuterInstanceData struct changes
	/////////////////////////////////////////////////////////////////////////////
	// Changes to this struct can only be added below this line

	// Handles
	HANDLE m_evAppLogStart;					// Event to signal AppLog has been started
	HANDLE m_evAppLogEnd;					// Event to signal AppLog to exit
	HWND m_hDlgDialog1;						// Test Dialog ... Maybe Remove
	HWND m_hDlgPolyDialog;					// Polygon Dialog
	HWND m_hDlgBlockReplayDialog;			// Block replay dialog
	HWND m_hDlgMessage;						// messages
	HFONT m_hCustomFont;					// Windows Font ID
	HFONT m_hGUIFont;					// Windows Font ID for opengl gui text
	HFONT m_hverdanaFont;					// Windows Font ID for opengl gui text
	HFONT m_hGridFont;						// Windows Font ID


	uintptr_t m_hAirDataThread;	// Thread Handle for Aircraft Data Thread Reader
	uintptr_t m_hASDPreloadThread;	// Thread Handle for Preloading ASD Data
	uintptr_t m_hThreadAppLog;				// handle to AppLog Thread
	uintptr_t m_hRecordVideoThread;			// handle to Record Video Thread
	
	// Syncronization
	CRITICAL_SECTION m_csSync;			// Synchronize shared map shapes
	CRITICAL_SECTION m_csASDSync;		// Synchronize ASD Data 
	CRITICAL_SECTION m_csPassurSync;	// Synchronize Passur Data 
	CRITICAL_SECTION m_csADSBSync;		// Synchronize ADSB Data 
    CRITICAL_SECTION m_csAirAsiaSync;	// Synchronize AirAsia Data 
	CRITICAL_SECTION m_csASDEXSync;		// Synchronize ASDEX Data 
    CRITICAL_SECTION m_csLMGSync;		// Synchronize LMG Data 
	CRITICAL_SECTION m_csMLATSync;		// Synchronize MLAT Data 
    CRITICAL_SECTION m_csPlanned;       // Synchronize Planned Departures and Arrivals
	CRITICAL_SECTION m_csEta;			// Synchronize ETA VO usage
	CRITICAL_SECTION m_csArptRangeRing;     // Synchronize Airport Range Ring VO usage
	CRITICAL_SECTION m_csROI;			// Synchronize ROI VO usage
	CRITICAL_SECTION m_csROIHist;		// Synchronize ROI History VO usage
	CRITICAL_SECTION m_csBlock;			// for block mode replau
	CRITICAL_SECTION m_csNoiseSync;		// Synchronize Noise Data 
	CRITICAL_SECTION m_csAirportsVO;	// Synchronize Loading of Airports Data for PASSUR server.
	CRITICAL_SECTION m_csAirlineCodesVO; // Synchronize Loading of Airline Codes Data.
	CRITICAL_SECTION m_csDBFSync;		// Synchronize DBF File Handling, not thread safe.


	HANDLE m_evPlannedReset;			// Planned ETA Thread Event Signal
	HANDLE m_evAirDataReset;			// Reset Data Thread Event Signal
	HANDLE m_evAirDrawReset;			// Reset Draw Thread Event Signal
	HANDLE m_evRecVideoReset;           // Reset Record Video Thread Event Signal
	
	// Integers
	int m_nInstanceId;			// DLL Load Lifetime Instance Value
	int m_bROILoaded;			// Force Reloading of ROI Points
	int m_bResetTracks;			// Reset Tracks flag (Replay)
	int m_bResetASDTracks;		// Reset ASD Tracks flag (TRUE for Replay, FALSE for changing Passur)
	int m_nDelaySecs;			// Replay Time Delay in seconds
	int m_nReplaySpeed;			// Replay Speed  (1=normal, 2 = clock moves twice as fast, 3=triple, etc.)
	int m_nFilterCount;			// Current number of filters
	int m_nActiveFilters;		// Number of Filters in Use (Enabled)
	int m_nTagFilterCount;		// Current number of tag filters
	int m_nMaxTagFilters;		// Maximum number of tag filters supported		
	int m_nActiveTagFilters;	// Number of tag Filters in Use (Enabled)
	int m_nSelectedAircraftType;		// Selected Aircraft Type Id
	int m_nSelectedTrackID;		// Selected Aircraft Track Id
	int m_nSelectedIsTrail;		// TRUE, if the selected feature is a trail point and not an Aircraft
	int m_nSelectedAirIndex;	// Selected Aircraft Index
	int m_nASDFileNBytes;		// ASD Data Buffer Length
	int m_nASDTotBytes;			// ASD Data Buffer Bytes
	int m_bReplayStatus;		// Replay Mode Status Flag 
	int m_bShowRoads;			// Show Roads
	int m_bShowFlightID;		// Show Flight ID
	int m_bShowCompression;		// Show Compression
	int m_bShowDepGates;		// Show Departure Gates
	int m_bShowBorder;			// Show Border around holds
	int m_bShowAirways;			// Show Airways on Map
	int m_bFastUpdates;			// Fast Updates
	int m_bBuildRangeRings;		// Initial build range rings flag
	int m_bArptLoaded;			// Airport info loaded flag
	int m_bEtaThread;			// Eta Thread flag
	int m_bRightMouseScroll;
	int m_nArptCount;			// Airport Counts
	int m_nMouseX;				// Mouse X coordinate
	int m_nMouseY;				// Mouse Y coordinate
	int m_nBoxX1;				// Drawing Box X1 Corner
	int m_nBoxX2;				// Drawing Box X2 Corner
	int m_nBoxY1;				// Drawing Box Y1 Corner
	int m_nBoxY2;				// Drawing Box Y2 Corner
	int m_nBox;					// Drawing Box flag
	int m_nBoxCount;			// Number of Boxes?
	int m_nGridXSize;			// Large Tag Grid X Size
	int m_nGridYSize;			// Large Tag Grid Y Size
	int m_nNumXGrids;			// Large Tag Grid X Count
	int m_nNumYGrids;			// Large Tag Grid Y Count
	int m_bASDFirstLoad;		// Load ASD flights on Startup
	int m_nPassurRequestIndex;	// PASSUR Fast Update Request Index
	int m_nPassurRequestNum;	// PASSUR Fast Update Request Count
	int m_nASDEXRequestIndex;	// ASDEX Fast Update Request Index
	int m_nASDEXRequestNum;		// ASDEX Fast Update Request Count			
	int m_nADSBRequestIndex;	// ADSB Fast Update Request Index
	int m_nADSBRequestNum;		// ADSB Fast Update Request Count			
	int m_nMLATRequestIndex;	// MLAT Fast Update Request Index
	int m_nMLATRequestNum;		// MLAT Fast Update Request Count			
	int m_nPtCount;				// Mouse Point Position	
	int m_nROIColors;           // Number of defined region outline colors
	int m_nPermCount;
	int* m_pActiveFilterPriorityIndices;	// Filter Priority Index array
	int m_nRegions;
	int m_nQuitRegionCount;		// Count for retry attempts to get regions for an airport.
	int m_nMaxRegionSecs;		// Seconds for which to remove old region entries.
	int m_nMaxRegionTimeout;	// Seconds for which to kill region entries that have not received a track point.
	int m_nHistRegionSecs;		// Seconds for which to retreive old region entries.
	int m_nRegionTimeoutDelay;	// Seconds to delay moving or deleting Region Entries
	int m_nDBServerIndex;		// Index into m_strHttpDBservername, only for WT.dll
	int m_bAdsbAll;				// User has access to all ADSB data feeds
	int m_nAdsbArpt;			// User has access to a single ADSB feed (per airport, -1 NotSet, 0 FALSE, 1 TRUE)
	int m_bMLATAll;				// User has access to all MLAT data feeds
	int m_bAirAsia;				// User has access to AirAsia data feeds
	int m_nMLATArpt;			// User has access to a single MLAT feed (per airport, -1 NotSet, 0 FALSE, 1 TRUE)
	int m_nCustFontIdx;			// Font Index
	int m_nGUIFontIdx;			// Font Index for opengl GUI text
	int m_nverdanaFontIdx;			// Font Index for opengl GUI text
	int m_nVehicles;			// User has enabled UNKN or MISC for Surface ASDE-X data
	int m_nVColorIndex;			// User has enabled UNKN or MISC for Surface ASDE-X data
	int m_bDisableVTag;			// Disable Showing Vehicle Tags
	int m_nArptAlt;				// Airport Ground Altitude.
	int m_nRoiHistSecs;			// Number of seconds to retrieve tracks
	int m_nAllSurface;			// Flag to show or Hide All non-matching ASDE-X filtered tracks
	int m_nROITimeDisplayLocal;	// Current ROI tracking of Time Display Format
	int m_nRegionAlertEnabled;		// Region Alert Enabled
	int m_nRegionOTOYellow;		// Region Out to Off Medium Alert Threshold
	int m_nRegionOTORed;		// Region Out to Off High Alert Threshold
	int m_nRegionOTIYellow;		// Region On to In Medium Alert Threshold
	int m_nRegionOTIRed;		// Region On to In High Alert Threshold
	int m_nTarmacDelayEnabled;	// TarmacDelay Alert Enabled
	int m_nTarmacDelayOTOYellow;// TarmacDelay Out to Off Medium Alert Threshold
	int m_nTarmacDelayOTORed;	// TarmacDelay Out to Off High Alert Threshold
	int m_nTarmacDelayOTIYellow;// TarmacDelay On to In Medium Alert Threshold
	int m_nTarmacDelayOTIRed;	// TarmacDelay On to In High Alert Threshold
	int m_bAirlineCodesLoaded;  // Airline Codes loaded flag
	int m_bALLoaded;			// AL Library Loaded
	int m_bAcTypesLoaded;		// Aircraft Types data file Loaded
	int m_selectedaircraftcolor;//mtm
	int m_nGroupID;				// User Group ID for Shared message processing
	int m_bIconsLoaded;			// Flag to indicate Aircraft icons are completely loaded
	int m_bFontLoaded;			// Flag to indicate Fonts completely loaded
	int m_bBasicGraphicsLoaded; // Flag to indicate when OpenGL has enough textures to start displaying a reasonble WT screen
	int m_bIsPassurTais;		// Temp flag set when the passur feed is actually from TAIS so as to skip smoothing.
	int m_bCitrixUser;			// Flag to determine if user is running WT from Citrix

	///mtm//
	int flag_check;
	int click;
	int row_numb, col_numb;
	int resize;

	//sy 
	int filter_check;


	time_t m_tServerAdjSecs;		// Adjusted Machine Time value from Server
	time_t m_tReplayStart;			// Replay Start Time
	time_t m_tPlotTime;				// Application Plot Time
	time_t m_tClientsTime;			// used for throttling
	time_t m_tDelayTime;			// Delay Time
	time_t m_tAirDataBeatTime;		// Aircraft Data Heart Beat Time
	time_t m_tASDFileTime;			// ASD Data file time
	time_t m_tLatestASDtime;		// Latest ASD Data time
	time_t m_tLastASDThreadSecs;	// Latest ASD Thread time
	time_t m_tPASSURFileTime;		// Passur file time
	time_t m_tADSBFileTime;			// ADSB file time
	time_t m_tAirAsiaFileTime;		// AirAsia file time
	time_t m_tAirAsiaFlightPlanFileTime;	// AirAsia FlightPlan file time
	time_t m_tMLATFileTime;			// ADSB file time
	time_t m_tASDEXFileTime;		// ASDE-X file time
	time_t m_tLastADSBGetTime;		// Latest ADS-B Thread time
	time_t m_tLastAirAsiaGetTime;	// Latest AirAsia Thread time
	time_t m_tLastAirAsiaFlightPlanGetTime;	// Latest AirAsia FlightPlan Thread time
	time_t m_tLastMLATGetTime;		// Latest MLAT Thread time
	time_t m_tLastASDEXGetTime;		// Latest ASDE-X Thread time
	time_t m_tLastMinuteASDStart;	// Latest ASD Start Time
	time_t m_tLastPASSURGetTime[ MAX_PASSURS ];		// Latest PASSUR File Times
	time_t m_tLastASDGetTime;		// Latest ASD Thread time
	time_t m_tNoiseFileTime;		// Noise file time
	time_t m_tLastNoiseGetTime;		// Latest Noise Thread time
	
	time_t m_tlasttailsecs;			// last try time for tails

	time_t m_tPlannedFileTime;		// Planned Arr and Dep FileTime
	time_t m_tLastPlannedGetTime;	// Last Planned Arr and Dep Retrieval Time
    time_t m_LastADSBFillTags;              // ADSB Fill Tags time
	time_t m_LastASDFillTags;               // ASD Fill Tags time
	time_t m_LastASDEXFillTags;             // ASDEX Fill Tags time
	time_t m_LastMLATFillTags;              // MLAT Fill Tags time
	time_t m_LastPassurFillTags;            // PASSUR Fill Tags time

	// Doubles
	double m_dPlotMSecs;		// 
	double m_dDelayMSecs;
	double m_dPrevX1;
	double m_dPrevY1;
	double m_dPassurPrevRequest;			// PASSUR Fast Update Previous Request time
	double m_dPassurRequestTimes[30];		// PASSUR Fast Update Previous Request Times
	double m_dASDEXPrevRequest;				// ASDEX Fast Update Previous Request time
	double m_dASDEXRequestTimes[30];		// ASDEX Fast Update Previous Request Times
	double m_dADSBPrevRequest;				// ADSB Fast Update Previous Request time
	double m_dADSBRequestTimes[30];			// ADSB Fast Update Previous Request Times
	double m_dMLATPrevRequest;				// MLAT Fast Update Previous Request time
	double m_dMLATRequestTimes[30];			// MLAT Fast Update Previous Request Times
	double m_dLatitudes[ MAX_PASSURS ];
	double m_dLongitudes[ MAX_PASSURS ];
	double m_dZeroX[ MAX_PASSURS ];
	double m_dZeroY[ MAX_PASSURS ];
	double m_dAltitudes[ MAX_PASSURS ];



	// OpenGL 
	GLuint m_gluCustomBase;			// Base Display List For The Custom Font Set
	GLuint m_gluGUIBase;			// Base Display List For The Opengl GUI font set
	GLuint m_gluverdanaBase;			// Base Display List For The Opengl verdana GUI font set
	GLuint m_gluBase;				// Base Display List For The Font Set
	GLuint m_gluBordersList;		// GL List for Borders
	GLuint m_gluLakeList;			// GL List for Lakes
	GLuint m_gluLakeLines;			// GL List for Lake borders
	GLuint m_gluRunwayList;
	GLuint m_gluRunwayLines;
	GLuint m_glTrianglesList;
	GLuint m_gluGuiTextures[10];
	GLuint m_gluAircraftTextures[N_AC_GROUPS][N_AIRCRAFT_COLORS];   // Storage For Textures
	GLuint m_gluVehicleTextures[N_VEHICLE_COLORS];   // Storage For Textures
	GLuint m_gluWXTextures[150];		// Storage For Weather Textures
	GLuint m_gluFontOffset; 
	GLubyte m_gluAirImages[N_AC_GROUPS][ N_AIRCRAFT_COLORS ][64][64][4];
	GLubyte m_gluVehImages[ N_VEHICLE_COLORS ][64][64][4];
	GLfloat m_glfRotateAngle;
	
	
	
	// DWORDS
	DWORD m_dwAirDataLastBeat;
	DWORD m_dwAirDataCurrBeat;
	DWORD m_dwAsdexFailCount;
	DWORD m_dwAdsbFailCount;
	DWORD m_dwAirAsiaFailCount;
	DWORD m_dwAirAsiaFlightPlanFailCount;
	DWORD m_dwAppLogId;						// AppLog Id for Message Passing
	DWORD m_dwMLATFailCount;
	DWORD m_dwLMGFailCount;
	long m_lADSBFileNBytes;	// Data Buffer Length 
	long m_lAirAsiaFileNBytes;	// Data Buffer Length 
	long m_lASDEXFileNBytes;	// Data Buffer Length 
	long m_lLMGFileNBytes;	// Data Buffer Length 
	long m_lPassurFileNBytes[ MAX_PASSURS ];
	long m_ltrackseq;
	long m_lMLATFileNBytes;		// Data Buffer Length for MLAT Data
	long m_lNoiseFileNBytes;	// Data Buffer Length 
	

	// Strings
	char m_strErrFilename[128];				// Used in place of G_err_filename
	char m_strLogfileName[256];				// log file name, recycles itself to avoid filling disk 		
	char m_strLogfileNamePrev[256];			// log file previous name
	char* m_pSelectedAircraft;
	char* m_pASDfilebuf;	// Data Buffer for ASDEX data
	char* m_pADSBFileBuf;	// Data Buffer for ADSB data
	char* m_pAirAsiaFileBuf;// Data Buffer for AirAsia data
	char* m_pMLATFileBuf;	// Data Buffer for MLAT data
	char* m_pASDEXFileBuf;	// Data Buffer for ASDEX data
	char* m_pLMGFileBuf;	// Data Buffer for LMG data
	char* m_pPassurfilebufs[ MAX_PASSURS ];
	char* m_pGTailBuf;		// Tail string buffer
	char* m_pGRegions;				// used to store message to send to ROI Grid display
	char* m_pGRegionsHistorical;	// used to store historical message to send to ROI Grid display
	char* m_pGRegionsDelete;		// used to store deleted message to send to ROI Grid display
	char* m_pGRegionHistTemp;		// used to store intermediate region JSON string
	char* m_pGRegionDelTemp;		// used to store intermediate region JSON string
	char m_strIPAddress;	// IP Address
	char m_strAirportMap[ARPTNAME_SZ]; // Identify which airport to show, default to JFK
	char m_strPrevAirportMap[ARPTNAME_SZ];
	char m_strHttpDBservername[2][SERVERNAME_SZ];			
	char m_strFltRoute[512];
	char m_strPrevADSBUrl[URL_SZ];
	char m_strPrevAirAsiaUrl[URL_SZ];
	char m_strPrevAirAsiaFlightPlanUrl[URL_SZ];
	char m_strPrevLMGUrl[URL_SZ];
	char m_strPrevMLATUrl[URL_SZ];
	char m_strPrevASDUrl[URL_SZ];
	char m_strPrevASDEXUrl[URL_SZ];
	char m_strPrevPASSURUrls[MAX_PASSURS][256];
	char m_strPassurZipFilename[ MAX_PASSURS ][FILENAME_SZ];
	char m_strPassurPrevFile[ MAX_PASSURS ][FILENAME_SZ];
	char m_strPassurArpts[ MAX_PASSURS ][ 5 ];
	char m_strLatestPassurURL[URL_SZ];
	char m_strAsdexservername[SERVERNAME_SZ];
	char m_strAsdexZipFilename[FILENAME_SZ];
	char m_strAsdexPrevFile[FILENAME_SZ];
	char m_strAdsbZipFilename[FILENAME_SZ];
	char m_strAdsbPrevFile[FILENAME_SZ];
	char m_strAirAsiaZipFilename[FILENAME_SZ];
	char m_strAirAsiaPrevFile[FILENAME_SZ];
	char m_strLMGPrevFile[FILENAME_SZ];
	char m_strLMGZipFilename[FILENAME_SZ];
	char m_strMLATPrevFile[FILENAME_SZ];
	char m_strMLATZipFilename[FILENAME_SZ];
	char m_strNoiseZipFilename[FILENAME_SZ];
	char m_strNoisePrevFile[FILENAME_SZ];
	char m_strPrevNoiseUrl[URL_SZ];
	char* m_pNoiseFileBuf;					// Data Buffer for Noise data
	char m_strPlannedZipFilename[256];
	char m_strPrevPlannedUrl[256];
	char m_strAlertRollPanelStr[TAG_LENGTH];
	char m_strRollPanelStr1[TAG_LENGTH];
	char m_strRollPanelStr2[TAG_LENGTH];
	char m_strRollPanelStr3[TAG_LENGTH];
	char m_strRollPanelStr4[TAG_LENGTH];
	char m_strRollPanelStr5[TAG_LENGTH];
	char m_arGridUsed[ MAX_X_GRIDS ][ MAX_Y_GRIDS ]; // set if grid used by another tag
	char m_arGridAircraftUsed[ MAX_X_GRIDS ][ MAX_Y_GRIDS ]; // Set if grid over an aircraft 
	char m_strHttpImagePath[FOLDERNAME_SZ];
	char m_strHttpMapPath[FOLDERNAME_SZ];
	char m_strHttpFontPath[FOLDERNAME_SZ];
	char m_strHttpSoundPath[FOLDERNAME_SZ];
	char m_strASDSharedFilename[256];
	char m_strHeader[128];
	char m_strLastZipName[128];
	char m_strAsdexArpt[ 5 ];
	char m_strRegionArpt[ 6 ];				// Current Regions Airport
	char m_strAsdexStatus[100];
	char m_strAsdiStatus[100];
	char m_strAdsbStatus[100];
	char m_strAirAsiaStatus[100];
	char m_strInternetStatus[100];
	char m_strMlatStatus[100];
	char m_strPassurStatus[100];
	char m_strPrevTailStr[256];
	char m_strPrevTailStrDefault[256];
	char m_strTailStrFltNum[8];
	char m_strCachedDir[MAX_PATH_SZ];
	char* m_pstrPerms;
	char* m_pstrLayouts;
	char m_strAdsbArpt[ 5 ];
	char m_strNoiseArpt[ 5 ];
	char m_strROIArpt[ 5 ];
	char m_strMLATArpt[ 5 ];
	char m_strVideoDir[MAX_PATH_SZ];
	char m_strLatestErrorURL[URL_SZ];
	char m_selectedaircraft[20];
	
	
	// File
	FILE* m_pASDDataFile;					// ASD Data File Pointer	
	FILE* m_pfVoLogFile;					// VO Logfile Pointer
	FILE* m_pfVoLogFilePrev;				// Previous VO Logfile Pointer
		
	// Structs
	OPENGLFONT m_arFonts[FONTCOUNT];
	OPENGLFONT m_guiFonts[GUI_FONTCOUNT];  // for new gui
	OPENGLFONT m_verdanaFonts[GUI_FONTCOUNT];  // for new gui
	FREETYPEFONT m_arFTFonts[FONTCOUNT];
	TEXTMETRIC m_tmBitmap;
	TEXTMETRIC m_tmCustom;
	TEXTMETRIC m_tmGUICustom;
	TEXTMETRIC m_tmverdanaCustom;
	POINT m_pt[100];
	POINT m_ptRight;
	void* m_pFilters;				//FILTERS*
	void* m_pTagFilters;			//TAG FILTERS*
	void* m_sMapList;				//struct clist_struct*
	void* m_pSelectedAirP;			//AIR**
	void* m_pPASSURAirs;			//AIR**[MAX_PASSURS]
	void* m_pASDAir;				//AIR**
	void* m_pASDEXAir;				//AIR**
	void* m_pLMGAir;				//AIR** for LMG Vehicles
	void* m_pADSBAir;				//AIR**
	void* m_pAirAsiaAir;			//AIR** for AirAsia
	void* m_pNoiseAir;				//AIR**
	void* m_pMLATAir;				//AIR**
	void* m_pGateAir;				//AIR** for Managing Gate Aircraft  
	void* m_pVList;					//VLL*
	void* m_pPassurIndexMap;		//std::hash_map<string,int>*	
	void* m_pASDIndexMap;			//std::hash_map<string,int>*
	void* m_pADSBBeaconVO;			//VO*    
    void* m_pADSBIndexMap;			//std::hash_map<string,int>*
	void* m_pMLATBeaconVO;			//VO*    
    void* m_pMLATIndexMap;			//std::hash_map<string,int>*
	void* m_pASDEXBeaconVO;			//VO*    
	void* m_pASDEXIndexMap;			//std::hash_map<string,int>*
	void* m_pRangeRingVO;			//VO*
	void* m_pTempRangeRingVO;		//VO* - Used for loading Changes across threads
	MAPLL* m_pPngLL;				//MAPLL*
	MAPLL* m_pPngCurp;				//MAPLL*
	void* m_pArpts;
	void* m_pETAVO;					//VO*
	void* m_pPlannedDepVO;			//VO*
	void* m_pPlannedArrVO;			//VO*
	void* m_pTDVO;					//VO*
	void* m_pROIColors;             //REGION_OUTLINE_COLOR*
	LAYOUTS m_sCurLayout;			//LAYOUTS
	USERSETTINGS m_sUserSettings;
	PREGION **m_pRegions;
	void* m_pROIDataVO;				// VO*
	void* m_pROIVO;					// VO*
	void* m_pVertexData;			// clist* handle
	void* m_pAirportsVO;			// VO*
	void* m_pROIHistVO;				// VO* Holds Historical Data
	void* m_pROINewActiveVO;		// VO* Holds Database read Active Data.
	void* m_pROINewHistVO;			// VO* Holds ActiveX Processed Completed Data.
	void* m_pROIDelVO;				// VO* Holds ActiveX Processed Completed Data.
	void* m_pGateInVO;				// VO* Holds Gate in Notifications.
	void* m_pGateOutVO;				// VO* Holds Gate Out Notifications.
	void* m_pAirlineCodes;          // VO* Holds Airline codes
	void* m_pDMVO;					// VO* Holds Diverted Aircraft Data
	void* m_pDMXVO;					// VO* Holds Diverted Aircraft Data for Diversions Grid (at some point we may want to combine with DMVO )
	void* m_pGateProcessVO;			// VO* Holds Processed lines of Gate Data;
	void *GFDO;
	void *GSA;
	struct timeb m_sPassurReqStart;
	struct timeb m_sADSBReqStart;
	struct timeb m_sAirAsiaReqStart;
	struct timeb m_sMLATReqStart;
	struct timeb m_sASDEXReqStart;
	struct timeb m_sASDReqStart;
	CUSTOM_MAP_DATA* m_pCustomMapData; // Holds info about user's available custom maps


	// Curl Handles
	CURL* m_pCurlHandle;
	CURL* m_pCurlHandleASDEX;
	CURL* m_pCurlHandleADSB;
	CURL* m_pCurlHandleAirAsia;
	CURL* m_pCurlHandleMLAT;
	CURL* m_pcurl_handle;
	CURL* m_pCurlHandleNoise;
	

	//Grids that were Global included in id structure(Solved the grid in multiple windows not being independent)
	void* m_poglgui;
	void* DataAppGrid;
	void* DataAppCreater;
	void *SecondTabGrid;
	void *SaveGrid;
	void *WarningGrid;
	void *ConfirmGrid;
	void *CriteriaConfirmGrid;
	void *DataTypeMenuGrid;
	void *Grid_Report;
	void *Grid_List;
	void *Gridnames;
	void *CalendarAppGrid;
	void *SearchGrid;
//	gui* test;
    // RecordVideo
    AVILIB avilib;                  // Structure for AVI video recording
};

void InitializeInstanceData(struct InstanceData* id);
void ReleaseInstanceData(struct InstanceData* id);
#ifdef __cplusplus 
}
#endif 

#endif//_INSTANCE_DATA_H_
