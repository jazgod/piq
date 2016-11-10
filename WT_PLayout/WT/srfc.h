#ifndef __SRFC_H__
#define __SRFC_H__

/**
 *==============================================================================
 *
 * Filename:         srfc.h
 *
 *==============================================================================
 *
 *  Copyright 2006  Megadata All Rights Reserved
 *
 *==============================================================================
 *
 * Description:
 *    Graphics module include file for Insight Application
 *
 *==============================================================================
 * 
 * Revision History:
 *
 * Feb. 21, 2007   E.Safranek
 * Added declaration for IExplorerPath 
 * Added definition for MAX_PATH_SZ
 * Added Revision History
 *==============================================================================
 */

#include "dbg_mem.h"
#include "feedstruct.h"

#define FIX_RECORD_SIZE 1428
#define AWY_RECORD_SIZE 225

#define GG_FLT_ROUTE_SIZE 513  // DB has 512 plus one for \n

#define MAX_MOUSE_POINTS 100

#define TAG_WIDTH  120.0f
#define TAG_HEIGHT  50
#define TRAILER_WIDTH  400
#define TRAILER_HEIGHT 12


//08-04-03
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#define sleep Sleep

//Win32:11-11-03
#define URLEncode urlencode


#define FAA_GREEN 1
#define FAA_RED   2
#define FAA_BLACK 3
#define FAA_ORANGE 4
#define FAA_YELLOW 5


// limit if surpassed on m_dWorldXSize will inhibit display of surface tracks -- purposely left very large to avoid missing tracks
#define ASDEX_DRAW_LIMIT 200.0
extern char *SplashText;

//Unzip:10-14-03
#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)

//Uncompress:09-30-03
#define MAX_NAME_LEN 1024
#ifndef GZ_SUFFIX
#  define GZ_SUFFIX ".gz"
#endif
#define SUFFIX_LEN (sizeof(GZ_SUFFIX)-1)
#define BUFLEN      16384

// Macro's
#define PtInRange(x,y)  ( (x) >= id->m_sCurLayout.m_dWorldXmin && (x) <= id->m_sCurLayout.m_dWorldXmax && (y) >= id->m_sCurLayout.m_dWorldYmin && (y) <= id->m_sCurLayout.m_dWorldYmax )
#define PtInWRange(x,y)  ( id->m_sCurLayout.m_dWorldXmin >= (x) && id->m_sCurLayout.m_dWorldXmax <= (x) && id->m_sCurLayout.m_dWorldYmin >= (y) && id->m_sCurLayout.m_dWorldYmax <= (y))
//glu32.lib, glut32.lib and OpenGL32.lib

#ifndef M_PI
#define M_PI            3.14159265358979323846
#endif


#define MATCH_EXACT 1
#define MATCH_ALL   0


#define FILTER_PASS 0
#define FILTER_FAIL 1
#define INCLUDE_MATCH 1
#define EXCLUDE_MATCH 2



#define PROJ_FALSE 0
#define PROJ_TRUE 1

/* LOOPS_PER_COMMAND_GET is how many frames we display until
   we retrieve the gui commands from the server so at about the current
   frame rate of 5-6 fps, and LOOPS_PER_COMMAND_GET = 6, then
  we will retrieve commands from the server about once per second */
#define LOOPS_PER_COMMAND_GET 6
#define EOD_LINE_SIZE 1024
//#define N_GEN_POLY_LISTS 4096
//  before 11mar2005 #define N_GEN_POLY_LISTS 6600
#define N_GEN_POLY_LISTS 18000

#define NCOLS_AIRPLANE 32
#define NROWS_AIRPLANE 32
#define X_SRC_OFFSET 0
#define Y_SRC_OFFSET 0

#ifndef CALLBACK
#define CALLBACK
#endif

#define LINE_BUFSIZE 4096

#define ASD_DISTANCE 2.0 

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define FILL_POLYS 1
#define NO_POLYS   0

#define FILTER_NAME_SIZE 128
#define FILTER_OWNER_SIZE 40
#define FILTER_AIRLINES_SIZE  256
#define FILTER_ORIGINS_SIZE  256 
#define FILTER_DESTINATIONS_SIZE  256 
#define FILTER_DEPARTUREFIX_SIZE  256 
#define FILTER_DESTGATE_SIZE  256 
#define FILTER_FLIGHTNUMBERS_SIZE 512
#define FILTER_ACTYPES_SIZE 256

#define MAX_FILTERS 200 //Base FILTER Limit, can be increased via Configuration.
#define MAX_LAYOUTS 32

#define MAX_AIRLINE_NAME_SIZE 64
#define ARPT_CODE_SIZE 5
#define MAX_AIRWAYS 90000


#define MAX_STARS  883

#define MAX_FIXES 32758
#define MAX_NAVS 3200

// 15apr #define MAX_TRACK_POINTS 20

// Tracked by Feed
#define TBF_NONE	0
#define TBF_ADSB	1
#define TBF_ASDEX	2
#define TBF_PASSUR	4
#define TBF_ASDI	8
#define TBF_MLAT	16
#define TBF_AIRASIA	32

#define NONE		0
#define ADSB		1
#define ASDEX		2
#define PASSUR		3
#define ASDI		4
#define ASD			4
#define NOISE		5
#define GATE		6
#define MLAT		7
#define AIRASIA		8
#define LMG			9

extern char g_arFeedTypes[10][10];

#define MAX_ASD_TRACK_POINTS 60
#define ASD_TRACK_MODIFIER(p) (p * 3) 
#define MAX_PASSUR_TRACK_POINTS 60
#define PASSUR_TRACK_MODIFIER(p) (p * 12) 
#define MAX_ASDEX_TRACK_POINTS 5
#define ASDEX_TRACK_MODIFIER(p) ((int)(p * 60.0)) 
#define MAX_ADSB_TRACK_POINTS 5
#define ADSB_TRACK_MODIFIER(p)  ((int)(p * 60.0)) 
#define MAX_NOISE_TRACK_POINTS 5
#define NOISE_TRACK_MODIFIER(p) ((int)(p * 60.0)) 
#define MAX_MLAT_TRACK_POINTS 5
#define MLAT_TRACK_MODIFIER(p)  ((int)(p * 60.0)) 
#define MAX_GATE_TRACK_POINTS 5
#define GATE_TRACK_MODIFIER(p)  ((int)(p * 60.0)) 
#define MAX_AIRASIA_TRACK_POINTS 60
#define AIRASIA_TRACK_MODIFIER(p)  ((int)(p / 5.0)) 


#define ORIG_DES_SIZE 5 
#define AIRLINE_NAME_SIZE 4
#define REGION_SIZE 32
#define GATE_SIZE 20
#define RUNWAY_SIZE 30

#define APPNAME_SIZE 65
#define USERNAME_SIZE 65
#define ALERTNAME_SIZE 65
#define PRIORITY_SIZE 65
#define COMMENTS_SIZE 257
#define ALERTSOURCE_SIZE 33
//#define KEYVALUES_SIZE 1025

#define IDM_FILTERS_NAME1   9900
#define IDM_FILTERS_EDIT_NAME1   9600

#define IDM_LAYOUT_NAME1    9950
#define IDM_LAYOUT_EDIT_NAME1    9650



// #define MAP_IMAGE_SIZE 1024
#define MAP_IMAGE_SIZE 2048


#define NUM_SMALL_MAPS 23
#define SMALL_MAP_IMAGE_SIZE 256

/// #define ASD_PREV_SECONDS 300 // need to go back to get all of the flight headers since they are only sent out
                         //      every 5 minutes


#define ASD_PREV_SECONDS 60 // for replays only , go back 5 minutes


#define ETA_GRID_ROWS	60
#define ETA_GRID_COLS	2
#define ETA_GRID_VISIBLE_ROWS 20  // how many are visible w/o scrolling


#define TAG_BORDER 6

// #define FIDS_GRID_ROWS	60
#define FIDS_GRID_ROWS	300

#define FIDS_GRID_COLS	6
#define FIDS_GRID_VISIBLE_ROWS 20  // how many are visible w/o scrolling


#define STATUS_GRID_ROWS	60
#define STATUS_GRID_COLS	2
#define STATUS_GRID_VISIBLE_ROWS 30 


#define MAX_SCROLL_POS 40  // (add one position for 0 total = 3  0,1,2 )
#define STATUS_MAX_SCROLL_POS 30  
#define CUSTOM_MARGIN 2  // margin between text lines for custom font

#define ARR_FIX_SIZE 6

#define N_BEACON_CODES 4096

//MCT: Made all TrackID lists same length (max use for adsb)
//NUMBER of TRACKS
//0x3FFF = 16383
#define N_TRACK_ID   0x3FFF
#define N_ASDI_TRACK_ID   N_TRACK_ID
//NUMBER of PASSUR TRACKS
#define N_PASSUR_TRACK_ID   N_TRACK_ID
//NUMBER of ADSB TRACKS
#define N_ADSB_TRACK_ID   N_TRACK_ID
//NUMBER of AirAsia TRACKS
#define N_AIRASIA_TRACK_ID   N_TRACK_ID
//NUMBER of ASDEX TRACKS
#define N_ASDEX_TRACK_ID   N_TRACK_ID
//NUMBER of MLAT TRACKS
#define N_MLAT_TRACK_ID   N_TRACK_ID
//NUMBER of GATE TRACKS
#define N_GATE_TRACK_ID   N_TRACK_ID
//NUMBER of NOISE TRACKS
#define N_NOISE_TRACK_ID   N_TRACK_ID

// ALERTS
#define ALERT_NONE			0x0000
#define ALERT_TD_OTO		0x0001		// TARMAC DELAY OUT TO OFF
#define ALERT_TD_OTI		0x0002		// TARMAC DELAY ON TO IN
#define ALERT_RG_OTO		0x0004		// TARMAC DELAY ON TO IN
#define ALERT_RG_OTI		0x0008		// TARMAC DELAY ON TO IN

#define ALERT_LEVEL_NONE	0x0000
#define ALERT_LEVEL_LOW		0x0001
#define ALERT_LEVEL_MED		0x0002
#define ALERT_LEVEL_HIGH	0x0004

// structs and typedefs
#define POLY_OPEN   "Open"
#define POLY_CLOSED "Closed"
#define POLY_RESTRICTED "Restricted"
#define POLY_SELECTED "Selected"


typedef struct ll_struct {
  struct ll_struct *np;
  //double x;
  //double y;
  double vt[3]; /* need to make a new structure? */
} points;

typedef struct construction_struct {
	char *category;
	char *area;
	char *segment;
	char *status; // closed, open, etc
} constrc;


typedef struct poly_struct {
  char *polyname;
	int index; // for indexing into dbf file
  float width, height;
  float xmin, xmax, ymin, ymax;
  int count;
  points *ppoints;
  int polyflag; /*  FILL_POLYS or NO_POLYS */
  double red, green, blue;
  char ptype;
  int polylistcount; // startList index for opengl list
  int linelistcount; // startMapList index for opengl line segments
  char showflag;
	void *datastruct; // application defined data structure, can be used by app as needed
} poly;
  
typedef struct polylist_struct {
  struct polylist_struct *np;
  struct polylist_struct *lastpoly;
  float width, height;
  float xmin, xmax, ymin, ymax;
  poly *poly;
} polylist;




typedef struct gridwinstruct {
  HWND GridWin;
  VO *GridVO;
  void (*RefreshCallback)(int myflag);
  HDC hDC_GridWin;
  HGLRC hRC_GridWin;
  int GridWin_id;
  MGLGRID *grid;
  int menu_item;
} GW;



typedef struct trackstruct {
	float x, y;
	int altitude;
	time_t utc;
	int speed;
} PTRACK;

#define DESCRIPTION_SIZE 35
#define FREQUENCY_SIZE 10
#define ID_SIZE 10
#define SERNO_SIZE 10
#define TAG_SIZE 10
#define TYPE_SIZE 20

typedef struct vehiclestruct {
	char Description[DESCRIPTION_SIZE];
	char Frequency[FREQUENCY_SIZE];
	char Id[ID_SIZE];
	char SerNo[SERNO_SIZE];
	char Tag[TAG_SIZE];
	char Type[TYPE_SIZE];
} VINFO;

typedef struct freeformtextstruct {
	char* text;
	int stime;
	int etime;
	int priv;
	char* uuid;
} FFTINFO;

typedef struct airstruct {
	GLfloat CurX;
	GLfloat CurY;
	GLfloat X;
	GLfloat Y;
	GLfloat PrevX, PrevY;
	int utc;
	int beacon;
	int trackid;
	PTRACK *TrackPtr;
	int TrackStart;
	char FltNum[FLTNUM_SIZE + 1];
	char TmpFltNum[FLTNUM_SIZE + 1];
	char Origin[ ORIG_DES_SIZE + 1 ];
	char Destin[ ORIG_DES_SIZE + 1 ];
	char gate[ GATE_SIZE + 1 ];
	char runway[ RUNWAY_SIZE + 1 ];
	char DivDestin[ ORIG_DES_SIZE + 1 ];
	int altitude;
	int eta, ata, atd, etd;
	int speed, fob; 
	char actype[ ACTYPE_SIZE ];
	int IsTracked;
	GLfloat StartMSECS;
	GLfloat EndMSECS, PlotMSECS;
	GLfloat X1, X2, Y1, Y2;
	GLfloat PrevAngle;
	GLfloat DestDis;
	float heading;
	char src_radar[ 6 ];
	char *FltRoute;
	char *FltRouteOrig;
	char *FltRouteWaypoints;
	char Region[REGION_SIZE];
    char ArrivalFix[ARR_FIX_SIZE];
    char DepartureFix[DEP_FIX_SIZE];
	int tcount;
	int max_tracks;
	int icao24; // 24 bit mode s number
	char tail[ N_TAIL_SIZE ];
	int IsBARR;
	int alert;				// Alert Type
	int alert_level;		// Alert Level (Low Med High)
	int td_oto;				// Tarmac Delay Current Seconds Out to Off
	int td_oti;				// Tarmac Delay Current seconds On to In
	int rg_oto;				// Region Current Seconds Out to Off
	int rg_oti;				// Region Current seconds On to In
	int InRegion;			// Simple flag to identify if the aircraft is within any airport region
	int InMlatRegion;		// Simple flag to identify if the aircraft is within an mlat region
	int tt;					// Taxiing Time
	int OnRwy;				// Runway Region
	int evnoddc;			// MLAT - ADSB and Quality of Track value
	int vehicle;
	int utc_gatein;			// timestamp of gate in message 
	int utc_gateout;		// timestamp of gate out message 
	FFTINFO *FFTInfoPtr;		// Free Form or User Defined Text struct
	VINFO *VInfoPtr;
	int color; //mtm
	int check;//mtm
} AIR;

typedef struct filterstruct {
	char filtername[ FILTER_NAME_SIZE ];
	char airlines[ FILTER_AIRLINES_SIZE ];
	char origins[ FILTER_ORIGINS_SIZE ];
	char destinations[ FILTER_DESTINATIONS_SIZE ];
	char fixes[ FILTER_DEPARTUREFIX_SIZE ];
	char gates[ FILTER_DESTGATE_SIZE ];
	int out2off;
	int on2in;
	int delayed;
	int stranded;
	int AltRadio; // 0 = GT, 1 = LT, 2 = all altitudes
	int altitude;
	int colorindex;
	int IsIncludeFilter; // true or false
	int IsActive;  // if TRUE, then the user activated this filter
	char flightnumbers[ FILTER_FLIGHTNUMBERS_SIZE ];
	char matchall;
	char actypes[ FILTER_ACTYPES_SIZE ];
	char owner[ FILTER_OWNER_SIZE ];
} FILTERS;

//struct liststruct {
//  char *name;
//  int show;
//  poly *poly;
//} listshow[ N_GEN_POLY_LISTS ];

extern GLuint Constructionlist;

#define GUI_ICON_ON  1
#define GUI_ICON_OFF 0

#define MAIN_WINDOW_VAL  9999
// #define ETAGridWin_WINDOW_VAL 9990
#define StatusGridWin_WINDOW_VAL 9991
#define FIDS_WINDOW_VAL 9992
#define SPLASH_WINDOW_VAL 9993

#define GRID_WINDOWS_START 10001
#define OPENGL_WINDOWS_START 10500

#define SPLASH_XSIZE 425
#define SPLASH_YSIZE 300
#define SPLASH_XLOC  300
#define SPLASH_YLOC  300

#define STRAT_XLOC  100
#define STRAT_YLOC  100
#define STRAT_XSIZE 800
#define STRAT_YSIZE 800

#define ASD_TIMEOUT_SECS 600  // ASD_TIMEOUT_SECS is the number of seconds to wait before dropping the track.
                              // sometimes 10 or more minutes can elapse between hits, especially if switching radars
#define ASD_TRANS_OCEAN_TIMEOUT_SECS 5400 // some oceanic flights only update every hour or more 5400== 1.5 hours
#define ASD_EXTRAPOLATE_MAX_MSECS  180000     // 60 seconds maximum extrapolation time, if over then plot on last known point
#define PASSUR_EXTRAPOLATE_MAX_MSECS  20000

// if aircraft is close to airport, assume it landed if past timeout
#define ASD_CLOSE_TO_AIRPORT_TIMEOUT  60

// timeout interval is aircraft is far from airport
#define ASD_FAR_AWAY_TIMEOUT 1800


// Passur track timeout much less than ASD
#define PASSUR_TRACK_TIMEOUT 60 
#define ASDEX_TRACK_TIMEOUT 120 
#define ASDEX_INREGION_TRACK_TIMEOUT 900 
#define ASDEX_LKP_TRACK_TIMEOUT 85 
#define ADSB_TRACK_TIMEOUT 60 
#define MLAT_LKP_TRACK_TIMEOUT 85 
#define MLAT_TRACK_TIMEOUT 120 
#define MLAT_INREGION_TRACK_TIMEOUT 180 
#define PASSUR_REPLAY_TIMEOUT 600
#define NOISE_TRACK_TIMEOUT 600 
// Gate IN Timeout 25 Mins 
#define GATE_IN_TRACK_TIMEOUT 1500 
// Gate OUT Timeout is 10 Mins
#define GATE_OUT_TRACK_TIMEOUT 600 
#define AIRASIA_TRACK_TIMEOUT 900 


// MAX_PASSURS for the maximum number of passurs to be tracked simultaneously, for DHS
#define MAX_PASSURS 3  
#define MAX_HISTORICAL_TRACKS 3000

// Javascript to ActiveX Variables
// name = key, "debug", "server_url"...
// returns TRUE if found, fills value with up to len chars, FALSE if not found
extern int GetLocalFile( struct InstanceData* id, const char* localpath, const char* filename, const char* remotepath, int eMode);
extern int GeomInsideCircle( double x, double y, double xcenter, double ycenter, double RadiusNM );
extern int CalcASDLatLon( struct feedstruct *record, double *xptr, double *yptr, double *prevxptr, double *prevyptr );
extern int AddTrail(struct InstanceData* id, AIR *air, int AI, struct feedstruct *record , int isASD, int max_tracks );
extern int CalcLatLon(struct InstanceData* id, int feedx, int feedy, double *xptr, double *yptr, int passurindex);
extern void ProcessFAAXML( struct InstanceData* id, char *xmlstr, int xmlsize );
extern int ZoomIn(struct InstanceData* id, POINT* pt);
extern int ZoomOut(struct InstanceData* id);

extern int LoadShapefile( struct InstanceData* id, char *shapefilename , int polyflag, double red, double green, double blue,
                  double LatLonSizeFilter,  double PolyTolerance, int innerflag,
                  int MinVertices, int exclusion_filter, int ptype );

void SmoothHistTracks(VO *TrackVO, char *src_radar);

extern void MoveLeft(struct InstanceData* id);
extern void MoveRight(struct InstanceData* id);
extern void MoveDown(struct InstanceData* id);
extern void MoveUp(struct InstanceData* id);
extern void MoveAngle(struct InstanceData* id, double dDirection);
extern int SetHomeCoordinates(int whole_world);
extern int ResetPolyColors();

extern void LoadInternationalFixes(struct InstanceData* id);

extern VO *GetETAs();
extern int clipaircraft( struct InstanceData* id, AIR *air );
extern int PixOnScreen( struct InstanceData* id, int xpixel, int ypixel );

extern int CountTrills( void *, int * );
extern int RectanglesOverlap( double ax1, double ay1, double ax2, double ay2 , 
                  double bx1, double by1, double bx2, double by2 );
extern void ResizeWorld( struct InstanceData* id, double x, double y , double NewXSize );
extern time_t Cnv2ClosestSecs( int hour, int minutes );
extern VO *NewTrack();
extern VO *NewHdrTrack();
extern char *ShowTime( struct InstanceData* id, long secs );
extern int GetAirlineInfo(int loaddata);
extern int GetArrsByAirport(int loaddata);
extern int GetDepsByAirport(int loaddata);
extern VO *ProcessPassurAirports( struct InstanceData* id, char *membuf );
extern void UpdateSplashWin(GLfloat x, GLfloat y, GLfloat angle, GLfloat AirSize);

extern int AirportCmd( char *linebuf );
extern void AddFilterMenuItems( HMENU hMenu );
extern void UpdateStatusGridWin();
extern void FIDSWinChangeSize(GLsizei w, GLsizei h);
extern void GridWinChangeSize(GLsizei w, GLsizei h);
extern void mymouse(struct InstanceData* id, int b, int m, int x, int y);
extern int MakeMaps();
extern void LoadRunways( struct InstanceData* id );
extern void GetGUIColorImage( GLfloat *colorvec, int ntexture, int xsize, int ysize , int alpha );
extern int ForceRefresh;

extern void TranslateView( struct InstanceData* id, float x, float y );
extern void FreeEditPolys();



extern void FreeRegions(struct InstanceData* id);

extern void SaveRegion(struct InstanceData* id, char *regionname);
extern void DeleteRegion(struct InstanceData* id, char *regionname);
extern struct InstanceData* PolyID;

extern HWND MainWin;
extern HWND SplashWin;
extern int UserIsAvitat;
extern int UserIsAirCell;
extern int UserIsAmstat;
extern int UseGLLists;
extern void WxGenTexture(struct InstanceData* id, int wx);
extern int RemoveTrack( struct InstanceData* id, AIR **airp, int AI, int nFeedType, int reset_flag );

extern double radarlat[], radarlong[];
extern time_t LastKnownTime;
extern int pcount;
extern int RunwaysLoaded;
extern HMENU filtermenu;
extern HMENU filteredit;
extern int compareArpt( const void *arg1, const void *arg2 );
extern int FAABallSize;
extern int g_ThreadIsLoading;
extern AIR *GetASDAir();
extern char version[];
extern char builddate[];
extern char FFFltRoute[];
extern int MaxTagHeight, MaxTagWidth;

extern int RemoveASDTrack( struct InstanceData* id, AIR **airp, int AI, int reset_flag );
extern int g_WXThreadsLoaded;
extern int projflag;
extern int BoxRealX2, BoxRealY2;

extern VO *AllFlightsVO;
extern GLfloat FIDSBarColor[3];
extern const GLfloat white[3];
extern struct timeb wxload_time;
extern double LatOrig;
extern double LngOrig;

extern int outlineSmooth;
extern int MenuFilterCount;
	
extern char TailNotInAir[10];
extern time_t TailNotInAirTime;
extern const GLfloat gridgrey0[3];
extern HGLRC hRC_StatusGridWin;               // Permanent Rendering context
extern HDC hDC_StatusGridWin;                 // Private GDI Device context
extern GW *FilterGW;

extern GW *NewDepsByAirportGW; 
extern GW *NewArrsByAirportGW; 
extern GW *NewFlightByAirlineGW;


extern HGLRC hRC_FIDSWin;               // Permanent Rendering context
extern HDC hDC_FIDSWin;                 // Private GDI Device context

extern HGLRC hRC_SplashWin;               // Permanent Rendering context
extern HDC hDC_SplashWin;                 // Private GDI Device context

extern int CkAltitude( char *str );

extern char PasswordServer[];
extern HANDLE    hFAAMutex;
extern int iVscrollPos;
extern int iStatusVscrollPos;
extern int iFIDSVscrollPos;
extern int ShowPanel;
extern int ShowIcons;
extern int ShowArptNames;
extern int ShowHighways;
extern int ShowAvitatOverlay;
extern int ShowAirCellOverlay;
extern int ShowAirCellRangeRings;
extern int ShowStatusWin;
extern int ShowClosestPoly;
extern int ShowAirways;
extern int ExtrapolateMaxMSecs;
extern int PassurStarted;
extern int UseMapColors;
#ifdef WIN32
double rint( double dval );
#endif

extern time_t lasttzevent;
extern time_t lasthdrevent;

extern struct timeb last_motion_time;

extern VO *IATAAirportsVO;

extern char pulse_db_arpt[ORIG_DES_SIZE];  /* pulse db prefix, for etas, i.e. sanb, jfkb, tpa, tpab... */
extern char passur_arpt[ORIG_DES_SIZE];  /* used for passur directory for Am data */

extern void FillFDOVO(struct InstanceData* id, MGLGRID *FDOGrid);
extern void FillSAVO(struct InstanceData* id, MGLGRID *FDOGrid);
extern int SaveLocalData;

extern int ReplayCmd( struct InstanceData* id, char *linebuf );


extern MGLGRID *statusgrid;  // pointer to the status grid object
extern MGLGRID *fidsgrid;  // pointer to the status grid object


extern int UpdAircraft( struct InstanceData* id, struct feedstruct *record, int passurindex);

extern int browserflag;
extern int etaflag;  // collect eta data if window has been opened
extern int fidsflag;

extern int ASDAirScreenPos( struct InstanceData* id, AIR **airp, int AI );
extern char pulse_db_arpt[ORIG_DES_SIZE];  /* pulse db prefix, for etas, i.e. sanb, jfkb, tpa, tpab... */
extern char passur_arpt[ORIG_DES_SIZE];  /* used for passur directory for Am data */


typedef struct arpt_struct {
  char icao[5];
  // char *name;
  float lat;
  float lng;
} ARPT;

typedef struct rgbvalue {
  double  red;
  double  green;
  double  blue;
} RGBVALUE;  


typedef struct airwaysstruct {
  char name[6];
  float lat;
  float lng;
  unsigned char node;
  char fixname[5];   // some points in the jetway will have fix names used to partition jetroute into portions flown
  char type;
} AIRWAY;

extern AIRWAY* g_pAirways;			// Array of AIRWAY
extern int g_nAirwayCount;			// Count of Airways
extern int g_bAirwaysLoaded;		// Airway data loaded flag	

typedef struct fixstruct {
  char fixname[7];
  float lat;
  float lng;
} FIX;

extern FIX* g_pFixes;			// Array of all Fixes
extern FIX* g_pFixesLow;		// Array of Low Fixes
extern FIX* g_pFixesHigh;		// Array of High Fixes
extern FIX* g_pFixesTerm;		// Array of Terminal Fixes
extern FIX* g_pFixesPref;		// Array of Preferred Fixes
extern int g_nFixCount;			// Count of Fixes
extern int g_nFixCountLow;		// Count of Low Fixes
extern int g_nFixCountHigh;		// Count of High Fixes
extern int g_nFixCountTerm;		// Count of Terminal Fixes
extern int g_bFixesLoaded;		// Fixes loaded flag
extern int g_nFixCountPref;		// Count of Preferred Fixes

typedef struct starstruct {
  char starname[7];
  float lat;
  float lng;
} STARSID;

extern STARSID* g_pStarSID;		// Array of StarSID
extern int g_nStarCount;		// Count of StarSIDs
extern int g_bStarsLoaded;		// StarSID data loaded flag	

typedef struct navstruct {
  char navname[5];
  float lat;
  float lng;
} NAV;

extern NAV* g_pNavs;			// Array for NAVAIDS	
extern int g_nNavCount;			// Count of NAVS	
extern int g_bNavLoaded;		// Nav data loaded flag	


#define MAX_PATH_SZ 256

extern VO *DelayArptsVO;

// Debugging Variables
extern char debugstr1[256];
extern char debugstr2[256];
extern char debugstr3[256];
extern char debugstr4[256];
extern char testflt[10];


extern int compareFix( const void *arg1, const void *arg2 );
extern void StripLeading0( char *src, char *des, int deslen );
extern AIR **GetASDAirP( struct InstanceData* id );
extern AIR **GetADSBAirP( struct InstanceData* id );
extern AIR **GetASDEXAirP( struct InstanceData* id );
extern AIR **GetLMGAirP( struct InstanceData* id );
extern AIR **GetMLATAirP( struct InstanceData* id );
extern AIR **GetAirP( struct InstanceData* id,int passur_index  );
extern int GetMoreASD( VO *AirVO, int get_seconds, int resetflag );
extern ARPT *FindArptRow( struct InstanceData* id, char *arpt, int match_type );
extern int LoadNav(struct InstanceData* id);
extern void LoadFixes(struct InstanceData* id);

extern struct feedstruct *RdRecord(  struct InstanceData* id, struct feedstruct *record, char* recstr );
extern int IsDebugNameDefined(struct InstanceData* id, const char* strDebugName);
extern int GetJavascriptValue(struct InstanceData* id, const char* name, char* value, int len);
extern int GetConfigValue(struct InstanceData* id, const char* strName, char* value, int len);
extern int IsMapSupported(struct InstanceData* id, const char* strMapName);
extern int IsValidAsdexAirport( struct InstanceData* id, char* strArpt );
extern int IsValidAdsbAirport( struct InstanceData* id, char* strArpt );
extern int IsValidLMGAirport( struct InstanceData* id, char* strArpt );
extern int IsValidMLATAirport( struct InstanceData* id, char* strArpt );
extern int IsValidPassurAirport(struct InstanceData* id, char* strArpt);
extern void LogToServer(struct InstanceData* id, enum LOG_LEVEL eLevel, const char* strCategory, const char* strLogData);

#define D_CONTROL(vcolname, exp) {if ( 5 == id->m_nDebugControl || (id->m_nDebugControl && IsDebugNameDefined(id, vcolname)) ) {exp;}}
#define IS_D_CONTROL(vcolname) ( 5 == id->m_nDebugControl || (id->m_nDebugControl && IsDebugNameDefined(id, vcolname)) )

// ROI Configuration Functions

extern int LoadUserRoiSettings (struct InstanceData *id, char *user, char *arpt, char *configname, GridConfig *roi);
extern int SaveUserRoiSettings(struct InstanceData *id, char *user, char *arpt, char *configname, GridConfig *roi);
extern VO * ListUserRoiConfig (struct InstanceData *id, char *user, char *arpt);
extern int DeleteUserRoiConfig(struct InstanceData *id, char *user, char *arpt, char *configname);
//extern MGLGRID *Gridnames;
//extern MGLGRID *DataAppGrid; 
//extern MGLGRID *GridManager;
//extern MGLGRID *MainTabGrid;//mtm
//extern MGLGRID *SecondTabGrid;//mtm
//extern MGLGRID *DataAppCreater;//mtm


extern void ShowDataViewer(struct InstanceData* id);



#endif
