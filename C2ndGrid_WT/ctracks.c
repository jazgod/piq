#undef UNICODE
#undef _UNICODE
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <sys/timeb.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <alc.h>
#include <al.h>
#include "vo.h"
#include "vo_db.h"

#include "Tracks.h"
#include "mgl.h"
#include "srfc.h"
#include "InstanceData.h"
#include "resource.h"
#include "bitmapfont.h"
#include "Sounds.h"

extern BOOL CALLBACK Message2DlgPolyCB(HWND hdlg, UINT message, WPARAM wParam, LPARAM lparam);
	
extern char current_arpt[];

extern const GLfloat white[3];
extern const GLfloat black[3];
extern const GLfloat red[3];
extern const GLfloat blue[3];
extern const GLfloat grey[3];
extern const GLfloat green[3];
extern const GLfloat yellow[3];

MGLGRID *statusgrid;  // pointer to the status grid object
HMENU filtermenu;

int ShowPanel = TRUE; // Show or Hide Control Panel (scroll, Zoom)


char g_OpenGLDebugString[1000];

int BoxRealX2, BoxRealY2;

struct InstanceData* PolyID;

int ShowStatusWin = FALSE;
int ShowClosestPoly = FALSE;  // for investigating polygon map problems
int projflag = FALSE;  // use projection coordinates or not
int GraphicsNiceValue = 5;

struct timeb last_motion_time;

double LatOrig = 40.6397;
double LngOrig = -73.7789;

int LoadControlTexture(char* filename);

BOOL OpenGLTrace(char* pstr, ...)
{
	BOOL bSuccess = 1;
	va_list args;
	va_start(args, pstr);

	if(pstr)
	{
		if(strlen(pstr) < 1000)
			vsprintf_s(g_OpenGLDebugString, 1000, pstr, args);
		else
			bSuccess = 0;
	}
	else
		g_OpenGLDebugString[0] = 0;
	va_end(args);
	return bSuccess;
}


int ProcessKeydown( struct InstanceData* id, WPARAM wParam , int keystate)
{
	int nProcessed = TRUE;
	extern void FreeEditPolys();	
	const DWORD cdwThreadId = GetCurrentThreadId();

	if ( (nProcessed = MGLIsGridKeydown( id, wParam , keystate ) )){
		return( nProcessed );
	}
	switch ( wParam )
	{
	
	case VK_BACK:
		// go back to previous World coordinates
		// PrevWorldCoordinates(id);
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) VK_BACK Key Pressed\n", GetCurrentThreadId());
		break;
	case VK_ESCAPE:
		/* ESC key was pressed */
		// Quit( 0 );
		break;
	case VK_F1:
		break;
	case VK_RIGHT:
		MoveAngle(id, 180.0);
		break;
	case VK_LEFT:
		MoveAngle(id, 0.0);
		break;
	case VK_UP:
		MoveAngle(id, 270.0);
		break;
	case VK_DOWN:
		MoveAngle(id, 90.0);
		break;
	case VK_HOME:
		if (GetKeyState(VK_SHIFT) < 0) {
			//SetHomeCoordinates(1);
		}
		else {
			//SetHomeCoordinates(0);
		}
		break;
	case 'i':
	case 'I':
	case VK_ADD:
		ZoomIn(id, NULL);
		break;
	case 'o':
	case 'O':
	case VK_SUBTRACT:
		ZoomOut(id);
		break;
	case 'F':
		if ( keystate < 0 ){
			// Upper Case F, Increase Font Size
			if(id->m_nCustFontIdx < (FONTCOUNT - 1) ){
				SetFontIndex(id, id->m_nCustFontIdx + 1);				
			}
			if(id->m_nGUIFontIdx < (GUI_FONTCOUNT - 1) ){
				SetGUIFontIndex(id, id->m_nGUIFontIdx + 1, 0);
			}			
			id->resize = 2;
			MGLDrawGUI(id);
		}else{
			// Lower Case F, Decrease Font Size
			if(id->m_nCustFontIdx > 0){
				SetFontIndex(id, id->m_nCustFontIdx - 1);
			}
			if(id->m_nGUIFontIdx > 0 ){
				SetGUIFontIndex(id, id->m_nGUIFontIdx - 1, 0);
			}			
			id->resize = 2;
			MGLDrawGUI(id);
		}
		break;
	case 'T':
		/* change trail size */
		if ( keystate < 0 ){
			/* upper case T */
			if(id->m_sCurLayout.m_nAirAsiaTrailCount < 60)
				id->m_sCurLayout.m_nAirAsiaTrailCount+= 5;
			if(id->m_sCurLayout.m_nPassurTrailCount < 60)
				id->m_sCurLayout.m_nPassurTrailCount+= 5;
			if(id->m_sCurLayout.m_nAsdiTrailCount < 60)
				id->m_sCurLayout.m_nAsdiTrailCount+= 5;
			if(id->m_sCurLayout.m_dAsdexTrailCount < 1)
				id->m_sCurLayout.m_dAsdexTrailCount += 0.05;
			else if(id->m_sCurLayout.m_dAsdexTrailCount < 5)
				id->m_sCurLayout.m_dAsdexTrailCount += 1;
			if(id->m_sCurLayout.m_dAdsbTrailCount < 1)
				id->m_sCurLayout.m_dAdsbTrailCount += 0.05;
			else if(id->m_sCurLayout.m_dAdsbTrailCount < 5)
				id->m_sCurLayout.m_dAdsbTrailCount += 1;
			if(id->m_sCurLayout.m_dNoiseTrailCount < 1)
				id->m_sCurLayout.m_dNoiseTrailCount += 0.05;
			else if(id->m_sCurLayout.m_dNoiseTrailCount < 5)
				id->m_sCurLayout.m_dNoiseTrailCount += 1;
		} else {
			if(id->m_sCurLayout.m_nAirAsiaTrailCount > 6)
				id->m_sCurLayout.m_nAirAsiaTrailCount-=5;
			if(id->m_sCurLayout.m_nPassurTrailCount > 6)
				id->m_sCurLayout.m_nPassurTrailCount-=5;
			if(id->m_sCurLayout.m_nAsdiTrailCount > 6)
				id->m_sCurLayout.m_nAsdiTrailCount-=5;
			if(id->m_sCurLayout.m_dAsdexTrailCount > 1)
				id->m_sCurLayout.m_dAsdexTrailCount -= 1;
			else if(id->m_sCurLayout.m_dAsdexTrailCount > 0.05)
				id->m_sCurLayout.m_dAsdexTrailCount -= 0.05;
			if(id->m_sCurLayout.m_dAdsbTrailCount > 1)
				id->m_sCurLayout.m_dAdsbTrailCount -= 1;
			else if(id->m_sCurLayout.m_dAdsbTrailCount > 0.05)
				id->m_sCurLayout.m_dAdsbTrailCount -= 0.05;
			if(id->m_sCurLayout.m_dNoiseTrailCount > 1)
				id->m_sCurLayout.m_dNoiseTrailCount -= 1;
			else if(id->m_sCurLayout.m_dNoiseTrailCount > 0.05)
				id->m_sCurLayout.m_dNoiseTrailCount -= 0.05;
		}
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) TrailCounts=<ASDI:%d, PASSUR:%d, ASDE-X:%.2f, ADS-B:%.2f, NOISE:%.2f>\n", GetCurrentThreadId(), 
				id->m_sCurLayout.m_nAsdiTrailCount, id->m_sCurLayout.m_nPassurTrailCount, id->m_sCurLayout.m_dAsdexTrailCount, id->m_sCurLayout.m_dAdsbTrailCount, id->m_sCurLayout.m_dNoiseTrailCount);
		break;
	case 'Z':
		/* make aircraft smaller */
		if ( keystate < 0 ){
			/* upper case Z */
			id->m_sCurLayout.m_nAircraftSize += 1;
		} else {
			if(id->m_sCurLayout.m_nAircraftSize > 1)
				id->m_sCurLayout.m_nAircraftSize -= 1;
		}
		if ( outlineSmooth ){
			outlineSmooth = FALSE;
		} else {
			//  leave off, or make it a real gui setting --    outlineSmooth = TRUE;
		}
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) AircraftSize=<%d>\n", GetCurrentThreadId(), 
				id->m_sCurLayout.m_nAircraftSize);
		break;
	case 'P':
		// only show polygon editor for regions of interest for internal users
		if ( !strcmp( id->m_strUserName, "matt" ) || !strcmp( id->m_strUserName, "jshaffrey" ) 
			|| !strcmp( id->m_strUserName, "fwuser" ) || !strcmp( id->m_strUserName, "twhite" )
			|| !strcmp( id->m_strUserName, "smmike" )){

				if(!id->m_hDlgPolyDialog)
					id->m_hDlgPolyDialog = CreateDialogParam(id->m_hWTModule, MAKEINTRESOURCE (IDD_PolyDialog), /*id->m_hWnd*/NULL, Message2DlgPolyCB, (LPARAM)id );
				ShowWindow(id->m_hDlgPolyDialog, SW_SHOW );
				// get rid of old polys
				FreeEditPolys();
				id->m_nMode = MODE_POLY;
				// old way g_nMode = MODE_POLY;
				PolyID = id;
		}
		break;
	
	case 'D':
		// popup up Data Viewer
		
		ShowDataViewer(id);
		break;
	case 'G':
	case 'g':
		if ( ShowPanel == TRUE ){
			ShowPanel = FALSE;
		} else {
			ShowPanel = TRUE;
		}
		break;
	case 'X':
		//27jul2009 -- need "are you sure popup" first  if(GetKeyState(VK_CONTROL) < 1)
			// force terminate (no cleanup!)
		//	exit(1);
		break;

	case 'Y':
		if ( keystate < 0 ){
			// Upper Case Y
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
		}
		break;
	case 'S':
		if ( keystate < 0 ){
			// Upper Case S
			if( IS_D_CONTROL("SOUND_TEST") ){
				static int n = 0;
				vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Attempting to play sound\n", cdwThreadId);
				PlayLocalSound(id, n++);
				if( n >= SOUNDFILE_COUNT )
					n = 0;
			}
		}
		break;
	case 'C':
		// Reset Rotation View Angle
		id->m_glfRotateAngle = 0.0f;		
		break;
	case 'R':
		if ( keystate < 0 ){
			// Apply Counter-Clockwise Rotation to View
			if( 355.0f == id->m_glfRotateAngle )
				id->m_glfRotateAngle = 0.0f;
			else
				id->m_glfRotateAngle += 5.0f;
		}else{
			// Apply Clockwise Rotation to View
			if( 0.0f == id->m_glfRotateAngle )
				id->m_glfRotateAngle = 355.0f;
			else
				id->m_glfRotateAngle -= 5.0f;
		}
		break;	
	default:
		nProcessed = FALSE;
		break;
	}
	return nProcessed;
}



void buttonmotion(struct InstanceData* id, int x, int y)
{

  // mouse motion only with button down is called here
  char tmpstr[TMPBUF_SIZE];


  if ( MGLIsGuiMotionEvent(id, x, id->m_nHeight - y ) ){
    /* handled by a gui panel */
    return;
  }


  if ( id->m_nBox == TRUE ){
    id->m_nBoxX2 = x;
    id->m_nBoxY2 = id->m_nHeight - y;

    id->m_nBoxCount++;
  }
  if (ShowStatusWin){
    MGLGridText(id, statusgrid, 12, 0, "x,y mouse");
    sprintf_s(tmpstr, TMPBUF_SIZE, "%d, %d", x, y);
    MGLGridText(id, statusgrid, 12, 1, tmpstr);
  }

}


void motion(struct InstanceData* id, int x, int y)
{
  // any movement of mouse is called here
  char tmpstr[TMPBUF_SIZE];
  static int count, i, xmin, xmax, ymin, ymax;


  id->m_nMouseX = x;
  id->m_nMouseY = id->m_nHeight - y;
  if (ShowStatusWin){
    MGLGridText(id, statusgrid, 12, 0, "x,y mouse");
    sprintf_s(tmpstr, TMPBUF_SIZE, "%d, %d", id->m_nMouseX, id->m_nMouseY);
    MGLGridText(id, statusgrid, 12, 1, tmpstr);
    MGLGridText(id, statusgrid, 13, 0, "m count");
    sprintf_s(tmpstr, TMPBUF_SIZE, "%d", count++);
    MGLGridText(id, statusgrid, 13, 1, tmpstr);
  }

  //if ( id->m_bShowFAAStatus ){
    // see if we are over a Status icon
    //for (i = 0; DelayArptsVO && i < DelayArptsVO->count; i++ ){
     // xmin = VV(DelayArptsVO, i, "xpix", int );
     // ymin = VV(DelayArptsVO, i, "ypix", int );
      //xmax = xmin + FAABallSize;
      //ymax = ymin + FAABallSize;
      //if ( id->m_nMouseX >= xmin && id->m_nMouseX <= xmax && id->m_nMouseY >= ymin && id->m_nMouseY <= ymax ){
        // overlaps , show popup
       // vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Found rollover mouse x=%d y=%d\n", GetCurrentThreadId(), id->m_nMouseX, id->m_nMouseY );
        // OLDWAY_WAIT_FOR_REFRESH ShowFAAPopup( DelayArptsVO, i );
       // ForceRefresh = TRUE;
        //break;
      //}
    //}
  //}
  ftime( &last_motion_time );

}

int ShowMyMessage(char *messagetext, int showsecs )
{

  //MyMessageStart = time(0);
  //MyMessageEnd = MyMessageStart + showsecs;
  //SetDlgItemText(hDlgMessage, IDC_MESSAGE_TEXT,  messagetext );
  //ShowWindow( hDlgMessage , SW_SHOW );
  return TRUE;
}


double rint( double dval )
{
   double fval;

   fval = floor( dval );
   if ( dval - fval >= 0.5 ){
	   return( ceil( dval ) );
   } else {
	   return( floor( dval ) );
   }

}


