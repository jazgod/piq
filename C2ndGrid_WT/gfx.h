#ifndef __GFX_H__
#define __GFX_H__

/**
 *==============================================================================
 *
 * Filename:         gfx.h
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

#define FIX_RECORD_SIZE 1428
#define AWY_RECORD_SIZE 225

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

char *prog;
// void error            OF((const char *msg));

// Macro's
#define PtInRange(x,y)  ( (x) >= WorldXmin && (x) <= WorldXmax && (y) >= WorldYmin && (y) <= WorldYmax )

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
#define FILTER_AIRLINES_SIZE  256
#define FILTER_ORIGINS_SIZE  256 
#define FILTER_DESTINATIONS_SIZE  256 
#define FILTER_FLIGHTNUMBERS_SIZE 512
#define FILTER_ACTYPES_SIZE 256

#define MAX_LAYOUTS 32

#define ARPT_CODE_SIZE 5
#define MAX_AIRWAYS 43000


#define MAX_STARS  883

#define MAX_FIXES 32758
#define MAX_NAVS 3200

// 15apr #define MAX_TRACK_POINTS 20


// 11jul2005 increased to 20 #define MAX_ASD_TRACK_POINTS 5
#define MAX_ASD_TRACK_POINTS 20
// 17oct2006 increased to 100 #define MAX_PASSUR_TRACK_POINTS 5
#define MAX_PASSUR_TRACK_POINTS 100



#define FLTNUM_SIZE 8
#define ORIG_DES_SIZE 5 
#define MXAPCHAR 16
#define AIRLINE_NAME_SIZE 4

#define IDM_FILTERS_NAME1   9900
#define IDM_FILTERS_EDIT_NAME1   9600

#define IDM_LAYOUT_NAME1    9950
#define IDM_LAYOUT_EDIT_NAME1    9650


#define NUM_MAPS 4


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

#define ACTYPE_SIZE 6
#define O_D_SIZE 5


#define N_BEACON_CODES 4096


#define N_TRACK_ID   15000

#define N_PASSUR_TRACK_ID   10000



typedef struct playstruct {
	int gmode;
	int turn; // 0=red, else blue
} GPLAY;

extern GPLAY game;


typedef struct locstruct {
	int row;
	int col;
	int pindex; // piece index
} LOC;

typedef struct piecestruct {
	int rank;
	int col;
	int row;
	int status;
	int known;
	float colorv[3];
} PIECE;

extern PIECE piece[80];

typedef struct iconstruct {
  float x;
  float y;
  float width;
  float height;
  int state;  // on, off , rollover
} GUIICON;

typedef struct fixstruct {
  char fixname[7];
  float lat;
  float lng;
} FIX;


typedef struct starstruct {
  char starname[7];
  float lat;
  float lng;
} STARSID;



typedef struct navstruct {
  char navname[5];
  float lat;
  float lng;
} NAV;

#define MAX_ROUTE_SIZE 256


typedef struct arpt_struct {
  char icao[5];
  // char *name;
  float lat;
  float lng;
} ARPT;


struct feedstruct {
  int rectype;
  char airport[ MXAPCHAR + 1];
  time_t eventtime;
  time_t eta;
  time_t etd;
  unsigned beacon;
  int TrackID;
  int feedx;
  int feedy;
  int pointnum;
  int prevlng;  /* from asd only */
  int prevlat;  /* from asd only */
  time_t prevtime;
  char FltNum[ FLTNUM_SIZE ];
  char origin[O_D_SIZE];
  char destin[O_D_SIZE];
  int altitude;
  char actype[ACTYPE_SIZE];
  int speed;
  int velocity_x;
  int velocity_y;
  int latsecs;
  int longsecs;
  char qualifier;
  char src_radar[5];
  char cid[4];
  char route[ MAX_ROUTE_SIZE ];
};


typedef struct filterstruct {
	char filtername[ FILTER_NAME_SIZE ];
	char airlines[ FILTER_AIRLINES_SIZE ];
	char origins[ FILTER_ORIGINS_SIZE ];
	char destinations[ FILTER_DESTINATIONS_SIZE ];
	int AltRadio; // 0 = GT, 1 = LT, 2 = all altitudes
	int altitude;
	int colorindex;
	int IsIncludeFilter; // true or false
	int IsActive;  // if TRUE, then the user activated this filter
  char flightnumbers[ FILTER_FLIGHTNUMBERS_SIZE ];
  char matchall;
  char actypes[ FILTER_ACTYPES_SIZE ];
} FILTERS;


#define LAYOUT_PASSUR_SIZE 5

extern void LoadGame(char *openingname, int offset );

typedef struct ll_struct {
  struct ll_struct *np;
  //double x;
  //double y;
  double vt[3]; /* need to make a new structure? */
} points;

typedef struct poly_struct {
  char *polyname;
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



//struct liststruct {
//  char *name;
//  int show;
//  poly *poly;
//} listshow[ N_GEN_POLY_LISTS ];


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
#define STRAT_YSIZE 600

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


// MAX_PASSURS for the maximum number of passurs to be tracked simultaneously, for DHS
#define MAX_PASSURS 3  

extern int GeomInsideCircle( double x, double y, double xcenter, double ycenter, double RadiusNM );
extern int CalcASDLatLon( struct feedstruct *record, double *xptr, double *yptr, double *prevxptr, double *prevyptr );

int dummyparm;
void SmoothHistTracks(VO *TrackVO, char *src_radar);

extern void MoveLeft();
extern void MoveRight();
extern void MoveDown();
extern void MoveUp();
extern int SetHomeCoordinates(int whole_world);
extern int ResetPolyColors();
extern int GetETAs();

extern int CountTrills( void *, int * );
extern int RectanglesOverlap( double ax1, double ay1, double ax2, double ay2 , 
                  double bx1, double by1, double bx2, double by2 );
extern time_t Cnv2ClosestSecs( int hour, int minutes );
extern VO *NewTrack();
extern VO *NewHdrTrack();
extern int GetAirlineInfo(int loaddata);
extern int GetArrsByAirport(int loaddata);
extern int GetDepsByAirport(int loaddata);
extern void UpdateSplashWin(GLfloat x, GLfloat y, GLfloat angle, GLfloat AirSize);

extern int AirportCmd( char *linebuf );
extern void AddFilterMenuItems( HMENU hMenu );
extern void UpdateStatusGridWin();
extern void FIDSWinChangeSize(GLsizei w, GLsizei h);
extern void GridWinChangeSize(GLsizei w, GLsizei h);
extern int MakeMaps();
extern void SetASDDelaytime();

extern int ForceRefresh;
extern HWND MainWin;
extern HWND SplashWin;
extern int UserIsAvitat;
extern int UserIsAirCell;
extern int UserIsAmstat;
extern int UseGLLists;
extern GUIICON guiicon[];
extern double radarlat[], radarlong[];
extern time_t LastKnownTime;
extern int pcount;
extern int RunwaysLoaded;
extern HMENU filtermenu;
extern HMENU filteredit;
extern int compareArpt( const void *arg1, const void *arg2 );
extern int FAABallSize;
extern int g_ThreadIsLoading;
extern VO *GetASDAirVO();
extern char FFFltRoute[];
extern int MaxTagHeight, MaxTagWidth;

extern int g_WXThreadsLoaded;
extern int projflag;
extern int BoxRealX2, BoxRealY2;

extern VO *AllFlightsVO;
extern GLfloat FIDSBarColor[3];
extern const GLfloat white[3];
extern struct timeb wxload_time;
extern double LatOrig;
extern double LngOrig;

extern int MenuFilterCount;
	
extern char TailNotInAir[10];
extern time_t TailNotInAirTime;
	
extern HGLRC hRC_StatusGridWin;               // Permanent Rendering context
extern HDC hDC_StatusGridWin;                 // Private GDI Device context
extern GW *FilterGW;
extern GW *ETAGW;
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
extern int ShowRangeRings;
extern int PassurStarted;
extern int UseMapColors;
#ifdef WIN32
double rint( double dval );
#endif

extern time_t lasttzevent;
extern time_t lasthdrevent;

extern struct timeb last_motion_time;



extern char pulse_db_arpt[ORIG_DES_SIZE];  /* pulse db prefix, for etas, i.e. sanb, jfkb, tpa, tpab... */
extern char passur_arpt[ORIG_DES_SIZE];  /* used for passur directory for Am data */


extern int SaveLocalData;

extern MGLGRID *statusgrid;  // pointer to the status grid object
extern MGLGRID *fidsgrid;  // pointer to the status grid object

extern int browserflag;
extern int etaflag;  // collect eta data if window has been opened
extern int fidsflag;

extern VO *SelectedAirVO;



typedef struct rgbvalue {
  double  red;
  double  green;
  double  blue;
} RGBVALUE;  

#define MAX_PATH_SZ 256
#endif
