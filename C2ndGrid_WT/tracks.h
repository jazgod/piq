#ifndef __TRACKS_H__
#define __TRACKS_H__

/**
 *==============================================================================
 *
 * Filename:         Tracks.h
 *
 *==============================================================================
 *
 *  Copyright 2006  Megadata All Rights Reserved
 *
 *==============================================================================
 *
 * Description:
 *  Contains all defines, structures, typedefs and function declarations
 *  related to the main module Tracks.c.
 *
 *==============================================================================
 * 
 * Revision History:
 *
 *  Oct. 11 2006    E.Safranek
 *  Added file header, declaration for hDlgChangePassword, and CreateReplayDialog/
 *  
  *==============================================================================
 */

#include "LoginDialog.h"


typedef enum {
  CREATE_OPENING = 1,
  PLAY,
} gmodes;


typedef enum {
	S_BOMB = 0,
	S_MARSHALL,
	S_GENERAL,
	S_COLONEL,
	S_MAJOR,
	S_CAPTAIN,
	S_LIEUTENANT,
	S_SERGEANT,
	S_MINOR,
	S_SCOUT,
	S_SPY,
	S_FLAG,
} SRANKS;


/*typedef enum {
  S_DEAD = 1,
  S_ACTIVE,
} */gmodes;


extern char gmessage[];

#define N_WX_RADARS 150

#define ARRDEP_ARR 0
#define ARRDEP_DEP 1

#define FIDS_ARPT_SIZE  64
#define RANGERING_ARPTS_SIZE 64

#define FILFAIL_NOTRACKS  900
#define FILFAIL_FUTURE_TRACKS 901
#define FILFAIL_SEPARATED 902 
#define FILFAIL_OLD_PLOT 903
#define FILFAIL_MISSED 904

#define CHAR24  24
#define CHAR32  32
#define CHAR64  64

#define IDM_PASSUR_AIRPORTS 950

#define IDM_OVERLAY_CUSTOM_ITEMS 1100

#define IDM_OPENINGS_CUSTOM_ITEMS  1120

#define N_ALLFLIGHTS_GRIDROWS  20
#define N_ALLFLIGHTS_GRIDCOLS  6

#define N_FLIGHTS_BY_AIRPORT_GRIDROWS  20
#define N_FLIGHTS_BY_AIRPORT_GRIDCOLS  8

#define GFX_STD   0
#define GFX_LOW   1
#define GFX_HIGH  2

#define NATURAL_GREEN  0
#define DARK_GREEN    1
#define DESERT_BROWN   2
#define SLATE_GRAY     3

#define PASSUR_AND_ASD 1
#define PASSUR_ONLY 0

#define STRG_MENU_FILE			0
#define STRG_MENU_OPTIONS		1

#define PI_MENU_FILE         0
#define PI_MENU_APPLICATION  1
#define PI_MENU_OVERLAYS     2
#define PI_MENU_FILTER       3
#define PI_MENU_LAYOUTS      4
#define PI_MENU_SETTINGS     5
#define PI_MENU_DISPLAY      6
#define PI_MENU_PASSUR       7
#define PI_MENU_REPLAY       8
#define PI_MENU_MAP          9
#define PI_MENU_HELP         10

// extern int ShowLastKnown; // if last known position of an aircraft from FindFlight Must be shown
extern char LastKnownFlightID[9];
extern float LastKnownX;
extern float LastKnownY;
extern char LastKnownInfoStr[24];
extern char LastKnownInfoStr2[24];
extern char LastKnownInfoStr3[24];

extern LRESULT APIENTRY StratWinProc(HWND,UINT,WPARAM,LPARAM);
extern HWND StratWin;
extern GLuint StratBase;

extern GLuint SplashBase;
extern VO *AirportsVO;
extern int WxThreadStarted;
extern int GraphicsNiceValue;
extern char FIDSarpt[];

//extern char curlupwd[], curluser[], curlpwd[];
extern int ShowFilterWin;
//extern VO *HistoricalTracksVO[];
//extern VO *HistoricalHdrsVO[];
//extern int HistoricalCount;
extern VO *HisVO;
extern VO *BestVO; // stores selected HistoricalTrackVO
extern int BestVOCount; // index into HistoricalTracksVO

//extern InitSplashWin(HWND hWnd, int ETAWinWidth, int ETAWinHeight );


// extern HWND ETAGridWin;
extern HWND hDlgReplay;
extern HMENU MainMenu;
extern HWND hDlgChangePassword;

extern HWND FAAPopupWin;

extern char TagLine1[];
extern char TagLine2[];
extern char TagLine3[];
extern char TagLine4[];
extern char TagLine5[];

extern int ForceWxReload;
// defs here


extern char *XMLFindField(char *xmltext, char *ckstr, char *userbuf, int userbufmax );
extern void WxGenTextures(struct InstanceData* id);
extern void WxGenTexture(struct InstanceData* id, int wx);
extern void RangeRingsCmd( /* char *linebuf, char *DistanceStr, char *NumStr */ );
extern int depsgridflag, arrsgridflag, airlinesgridflag;
extern char FIDSairline[ FIDS_ARPT_SIZE ];

void CreateReplayDialog(void);
extern int ShowMyMessage(char *messagetext, int showsecs );

extern void LayoutOn( int LayoutIndex );

extern void LoadOpenings();

extern int ClearMaps();
#endif /* __TRACKS_H__ */



