/**
 *==============================================================================
 *
 * Filename:         nexrad.c
 *
 *==============================================================================
 *
 *  Copyright 2006  Megadata All Rights Reserved
 *
 *==============================================================================
 *
 * Description:
 *    NEXRAD weather display module
 *
 *==============================================================================
 * 
 * Revision History:
 *
 *  Jan. 03, 2007   E.Safranek
 *  Changes for Mantis Issue 104 Install problems for Windows "Limited" users
 *  Corrected pathname for text file and removed unnecessary searching for
 *  last slash in GetWxZip.
 *==============================================================================
 */
#include < windows.h >
#include "unzip.h"
#ifndef M_PI
#define M_PI            3.14159265358979323846
#endif
#include "gd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <errno.h>	/*declares extern errno*/
#include <math.h>
#include <Math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "vo.h"
#include "vo_extern.h"
#include "SSI.h"
#include "ExecUtil.h"
#include "vo_db.h"
#include <curl/curl.h>
#include "TexFont.h"
#include "shapefil.h"
#include "Tracks.h"
#include "nexrad.h"
#include "mgl.h"
#include "gfx.h"
#include "imu.h"
#include "InstanceData.h"
#include "Curl.h"
#include "servers.h"
#include "dbg_mem.h"

extern int IsDebugNameDefined(struct InstanceData* id, const char* strDebugName);
#define D_CONTROL(vcolname, exp) {if ( 5 == id->m_nDebugControl || (id->m_nDebugControl && IsDebugNameDefined(id, vcolname)) ) {exp;}}

int WxAlpha = 255;
GLfloat WxIntensity = 1.0;
int g_WXThreadsLoaded;
int g_ThreadIsLoading;

#define WX_BUFSIZE  100000
extern int check_exist_file(char *);

extern void clear_chunk_memory(struct MemoryStruct* chunk);
extern void free_chunk_memory(struct MemoryStruct* chunk);
extern void RotateView(struct InstanceData* id, double xMax, double xMin, double yMax, double yMin);

int NActiveRadars;
double radarlat[N_WX_RADARS], radarlong[N_WX_RADARS];
GLubyte wximage[N_WX_RADARS][WX_IMAGE_SIZE][WX_IMAGE_SIZE][4];
GLfloat trans[3];

struct point{
	double x;
	double y;
};

struct rect
{
	struct point left;
	struct point right;
};

// extern GLubyte wximage[WX_IMAGE_SIZE][WX_IMAGE_SIZE][4];

int radbuf[ MAX_RADIALS ][MAX_BINS];
int raslinebuf[ MAX_RADIALS ][MAX_BINS];
int iazmuth[ MAX_RADIALS ];
static int nradials;
double KmPerBin;  /* 230km / # of range bins */
double NMiPerBin;
int nbins;  /* number of bins as read from wx file */


extern char wxservername[];

int DBScaleLow[16] = { 0, 0, 0, 0,
		       0, 0, 0, 0,
		       0, 0, 1, 2,
		       3, 4, 4, 5 };



char *WxStations[] = { "kokx", "kenx", "kbgm", "kbuf", "kbhx",
		       "klrx", "kdox", "kfsx", "ksrx", "kggw",
		       "kgld", "kgjx", "kgrr", "ktfx", "kgrb",
		       "kgsp", "kuex", "khgx", "khtx", "kind",
		       "kjkl", "kdgx", "kjax", "keax", "kbyx", 
		       "kmrx", "karx", "kpoe", "klch", "kesx",
		       "klzk", "kvtx", "klvx", "klbb", "kjgx",
		       "kmqt", "kmax", "kmlb", "knqa", "kamx",
		       "kmaf", "kmkx", "kmpx", "kmsx", "kmob",
		       "kmxx", "kmhx", "kohx", "ktlx", "kapx",
		       "klnx", "kiwx", "kvnx", "koax", "kpah",
		       "kabr", "kdyx", "kabx", "kfdr", "kama",
		       "keyx", "kpdt", "kevx", "kdix", "kiwa", 
		       "kpbz", "ksfx", "kgyx", "krtx", "kpux",
		       "kdvn", "krax", "kudx", "krgx", "kriw",
		       "kfcx", "kdax", "kmtx", "ksjt", "knkx",
		       "kmux", "khnx", "tjua", "ksox", "kvbx",
		       "katx", "kshv", "kfsd", "kotx", "ksgf",
		       "klsx", "kvax", "ktlh", "ktbw", "ktwx",
		       "kemx", "kinx", "kakq", "ktyx", "khdx",
		       "kict", "kltx", "kgwx", "kyux",
		       "kffc", "kewx", "klwx", "kblx", "kbmx",
		       "kbis", "kmbx", "kcbx", "kbox", "kbro",
		       "kcxx", "kcbw", "kicx", "kilx", "kccx",
		       "kclx", "krlx", "kcys", "klot", "kbbx",
		       "kiln", "kcle", "kfdx", "kcae", "kcrp",
		       "kfws", "kdfx", "kftg", "kdmx", "kdtx",
		       "kddc", "keox", "kdlh", "kmvx", "kepz", 
           "klix", "paec", "papd", "pahg", "pabc",
           "pahc", "paih", "pacg", 0 };

// TRUE - intersection
// FALSE - no intersection
int Intersects(struct rect r1,struct rect r2)
{
	int res = TRUE;
    int x_check = (r1.left.x > r2.right.x || r2.left.x > r1.right.x);
    int y_check = (r1.right.y > r2.left.y || r2.right.y > r1.left.y);

    if( x_check && y_check )
    {	
       res = FALSE;   
    }
    return res;
}

void OpenGLState( struct InstanceData* id )
{
	GLenum glEnums[] = {GL_ALPHA_TEST, GL_AUTO_NORMAL, GL_BLEND, //GL_CLIP_PLANEi See glClipPlane  
						GL_COLOR_ARRAY, GL_COLOR_LOGIC_OP,  GL_COLOR_MATERIAL,
						GL_CULL_FACE, GL_DEPTH_TEST, GL_DITHER, 
						GL_FOG,	GL_INDEX_ARRAY, GL_INDEX_LOGIC_OP, //GL_LIGHTi See glLightModel and glLight  
						GL_LIGHTING, GL_LINE_SMOOTH, GL_LINE_STIPPLE, 
						GL_MAP1_COLOR_4, GL_MAP1_INDEX,	GL_MAP1_NORMAL, 
						GL_MAP1_TEXTURE_COORD_1, GL_MAP1_TEXTURE_COORD_2, GL_MAP1_TEXTURE_COORD_3, 
						GL_MAP1_TEXTURE_COORD_4, GL_MAP1_VERTEX_3, GL_MAP1_VERTEX_4, 
						GL_MAP2_COLOR_4, GL_MAP2_INDEX ,GL_MAP2_NORMAL,
						GL_MAP2_TEXTURE_COORD_1, GL_MAP2_TEXTURE_COORD_2, GL_MAP2_TEXTURE_COORD_3,
						GL_MAP2_TEXTURE_COORD_4, GL_MAP2_VERTEX_3, GL_MAP2_VERTEX_4, 
						GL_NORMAL_ARRAY, GL_NORMALIZE, GL_POINT_SMOOTH, 
						GL_POLYGON_OFFSET_FILL, GL_POLYGON_OFFSET_LINE, GL_POLYGON_OFFSET_POINT, 
						GL_POLYGON_SMOOTH, GL_POLYGON_STIPPLE, GL_SCISSOR_TEST,
						GL_STENCIL_TEST, GL_TEXTURE_1D, GL_TEXTURE_2D, 
						GL_TEXTURE_COORD_ARRAY, GL_TEXTURE_GEN_Q, GL_TEXTURE_GEN_R, 
						GL_TEXTURE_GEN_S, GL_TEXTURE_GEN_T, GL_VERTEX_ARRAY };
	BOOL glState[60] = {0};
	int n;
	for(n = 0; n < (sizeof(glEnums)/sizeof(GLenum)); n++){
		if(glIsEnabled(glEnums[n]) == GL_TRUE)
			glState[n] = TRUE;
	}
	{
		GLint mode;
		GLint colors[4] = {0};
		const char* pMode = "";
		glGetTexEnviv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, &mode);
		switch(mode){
			case GL_MODULATE: pMode = "GL_MODULATE"; break;
			case GL_DECAL: pMode = "GL_DECAL"; break;
			case GL_BLEND: pMode = "GL_BLEND"; break;
		}
		glGetTexEnviv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR, colors);
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Mode<%s>, Colors<%d,%d,%d,%d> \n", GetCurrentThreadId(), 
			pMode, colors[0], colors[1], colors[2], colors[3]);
	}
}

void CreateGifFromWx(struct InstanceData* id, int wx)
{
  gdImagePtr im;
  int color;
  int row, col;
  int red, green, blue;
  char filename[256] = {0};
  FILE* out;

  im = gdImageCreate(WX_IMAGE_SIZE, WX_IMAGE_SIZE);
  
  for ( row = 0; row < WX_IMAGE_SIZE; row++ ){
    for ( col = 0; col < WX_IMAGE_SIZE; col++ ){
	  red = wximage[wx][WX_IMAGE_SIZE - row - 1][col][0];
      green = wximage[wx][WX_IMAGE_SIZE - row - 1][col][1];
      blue = wximage[wx][WX_IMAGE_SIZE - row - 1][col][2];
	  color = gdImageColorExact(im, red, green, blue); 
	  if(color == -1)
		color = gdImageColorAllocate(im, red, green, blue); 
	  gdImageSetPixel(im, WX_IMAGE_SIZE - row - 1, col, color);
	}
  }
  
  sprintf_s(filename, _countof(filename), "%s\\weather%d.gif", id->m_strPassurOutPath, wx);
  fopen_s(&out, filename, "w+b");
  gdImageGif(im, out);
  fclose(out);
  
  gdImageDestroy(im);

}


time_t LastGoodWxSecs;

void ResetWxAlpha()
{
  int row, col, wx;


  for ( wx = 0; wx <  NActiveRadars; wx++ ){
    for ( row = 0; row < WX_IMAGE_SIZE; row++ ){
      for ( col = 0; col < WX_IMAGE_SIZE; col++ ){
        
        if ( !wximage[wx][row][col][0] &&
          !wximage[wx][row][col][1] &&
          !wximage[wx][row][col][2] ){
          wximage[wx][row][col][3] = WxAlpha;
        }
      }
    }
  }
}

int GetRasWxTexImage( int nrows, int wx, int dbscale )
{
  int row, col, c, alpha = 255;
  unsigned char red[16], green[16], blue[16];
  int ARows, ACols, pflag = 0, rasrow, rascol;
  double WxNMilesPerPixel, scaled;
  
  ARows = WX_IMAGE_SIZE;
  ACols = WX_IMAGE_SIZE;
  WxNMilesPerPixel =  (double) WX_IMAGE_NMILES / ARows;
  
  red[0] = 0; blue[0] = 0; green[0] = 0; 
  red[1] = 0x87; blue[1] = 0xFA; green[1] = 0xCE;
  red[2] = 30; blue[2] = 255; green[2] = 144;
  red[3] = 0; blue[3] = 255; green[3] = 0;
  
  red[4] = 0; blue[4] = 0; green[4] = 255;
  red[5] = 0; blue[5] = 0x33; green[5] = 0xCC;
  red[6] = 0; blue[6] = 0; green[6] = 0x99;
  red[7] = 0xff; blue[7] = 0; green[7] = 255;
  
  red[8] = 0xCC; blue[8] = 0; green[8] = 0x99;
  red[9] = 0xff; blue[9] = 0; green[9] = 0x99;
  red[10] = 0xff; blue[10] = 0; green[10] = 0;
  red[11] = 0xdc; blue[11] = 0x3c; green[11] = 0x14;
  
  red[12] = 0xb2; blue[12] = 0x22; green[12] = 0x22;
  red[13] = 0xff; blue[13] = 0xff; green[13] = 0;
  red[14] = 0x80; blue[14] = 0x80; green[14] = 0;
  red[15] = 0xff; blue[15] = 0xff; green[15] = 0xff;
  
  
  scaled = (double) nrows / WX_IMAGE_SIZE;
  for ( row = 0; row < ARows; row++ ){
    for ( col = 0; col < ACols; col++ ){
      
      /* scale the values to get the correct pixel value */
      rasrow = (int)rint(row * scaled);
      rascol = (int)rint(col * scaled);
      c = raslinebuf[rasrow][rascol];
      if ( c < 0 || c > 15 ){
        /* out of range */
        return( FALSE );
      }
      if ( dbscale == 0 ){
        /* low range, adjust values downward */
        c = DBScaleLow[c];
      }
      
      wximage[wx][ARows - row - 1][col][0] = red[c];
      wximage[wx][ARows - row - 1][col][1] = green[c];
      wximage[wx][ARows - row - 1][col][2] = blue[c];
      
      if ( red[c] || green[c] || blue[c] ){
        alpha = WxAlpha;		
      } else {        
        alpha = 0;
      }
	  wximage[wx][ARows - row - 1][col][3] = alpha;
    }
  }
  
  return( TRUE );
}



void GetWxTexImage( struct InstanceData* id, int wx )
{
  int row, col, c, alpha = 255;
  unsigned char red[16], green[16], blue[16];
  int ARows, ACols, nradial, bestrad, radial_bin, pflag = 0;
  double xcenter, ycenter, x, y, rads, degrees, bestdiff, az,
    nm_point_distance, WxNMilesPerPixel, diffval;

  ARows = WX_IMAGE_SIZE;
  ACols = WX_IMAGE_SIZE;
  WxNMilesPerPixel =  (double) WX_IMAGE_NMILES / ARows;

  
  red[0] = 0; blue[0] = 0; green[0] = 0;
  red[1] = 0x87; blue[1] = 0xFA; green[1] = 0xCE;
  red[2] = 30; blue[2] = 255; green[2] = 144;
  red[3] = 0; blue[3] = 255; green[3] = 0;

  red[4] = 0; blue[4] = 0; green[4] = 255;
  red[5] = 0; blue[5] = 0x33; green[5] = 0xCC;
  red[6] = 0; blue[6] = 0; green[6] = 0x99;
  red[7] = 0xff; blue[7] = 0; green[7] = 255;

  red[8] = 0xCC; blue[8] = 0; green[8] = 0x99;
  red[9] = 0xff; blue[9] = 0; green[9] = 0x99;
  red[10] = 0xff; blue[10] = 0; green[10] = 0;
  red[11] = 0xdc; blue[11] = 0x3c; green[11] = 0x14;

  red[12] = 0xb2; blue[12] = 0x22; green[12] = 0x22;
  red[13] = 0xff; blue[13] = 0xff; green[13] = 0;
  red[14] = 0x80; blue[14] = 0x80; green[14] = 0;
  red[15] = 0xff; blue[15] = 0xff; green[15] = 0xff;

  xcenter = (double) ARows / 2.0;
  ycenter = (double) ACols / 2.0;

  for ( row = 0; row < ARows; row++ ){
    for ( col = 0; col < ACols; col++ ){
      
      x = col - xcenter;
      y = row - ycenter;
      rads = atan2(x, y); /* reverse y and x for this coordinate system */
      degrees = rads * 180.0 / M_PI;
      
      
      /* find closest radial to this degrees */
      if ( degrees < 0.0 ){
        degrees += 360.0;
      }
      bestdiff = 9.0e20;
      bestrad = 0;
      for ( nradial = 0; nradial < nradials; nradial++ ){
        az = iazmuth[ nradial ];
        diffval = fabs( ((double) az / 10.0) - degrees );
        if ( diffval < bestdiff ){
          bestrad = nradial;
          bestdiff = diffval;
        }
      }

      
      /* find distance along radius to current point */
      nm_point_distance = sqrt( (x * x) + (y * y) ) * WxNMilesPerPixel;
      radial_bin = (int)rint( nm_point_distance / NMiPerBin );
      
      if ( radial_bin >= 0 && radial_bin < nbins &&
        bestrad >= 0 && bestrad < nradials ){
        c = radbuf[bestrad][radial_bin];
        wximage[wx][row][col][0] = red[c];
        wximage[wx][row][col][1] = green[c];
        wximage[wx][row][col][2] = blue[c];
        
        if ( red[c] || green[c] || blue[c] ){
          alpha = WxAlpha;
        } else {
          alpha = 0;
        }
        wximage[wx][row][col][3] = alpha;
        
        if ( pflag ){
          printf("GetWxTexImage, bestrad=%d radial_bin=%d\n",
            bestrad, radial_bin );
          printf("nm_point_distance=%g NMiPerBin=%g WxNMilesPerPixel=%g\n",
            nm_point_distance, NMiPerBin, WxNMilesPerPixel );
          printf("image[%d][%d]=%d, color=%d red=%x gr=%x bl=%x al=%d\n",
            row, col, c, radbuf[row][col], red[c], green[c], blue[c], alpha);
          fflush(stdout);
        }
      } else {
        /* values are out of range */
		D_CONTROL("WX", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Bad indexes into radbuf, bestrad=%d nradials=%d "
            " radial_bin=%d nbins=%d x=%g y=%g nm_point_distance=%g\n",
            GetCurrentThreadId(), bestrad, nradials, radial_bin, nbins, x, y, nm_point_distance));
        wximage[wx][row][col][0] = 0;
        wximage[wx][row][col][1] = 0;
        wximage[wx][row][col][2] = 0;
      }
    }
  }

}



LoadRasterWx( struct InstanceData* id, unsigned char *ptr, unsigned char *hdrbuf, int wx, int dbscale )
{
  int i, msglen, elev,
    iscale5, nc, n, nr, nrun_marker, nruns, npos1, npos2, nbyte,
    runlen, colorval, bufsize;
  int BlockLen, Len1, TrailerLen, nrows, pixcount;
  int icatt[ N_COLORS ];
  int icats[ N_COLORS ];
  unsigned char *buf = NULL;
  int ival0, ival1;
  int days_since_1970, secs_since_midnight;
  int pflag = 0;
  time_t secs, filesecs;
  const DWORD cdwThreadId = GetCurrentThreadId();

  D_CONTROL("WX", vo_log_info(id->m_pfVoLogFile, __FUNCTION__,  "(0x%X) WxStations[%d] = %s\n", cdwThreadId, wx, WxStations[wx] ));

  // bcopy not part of win32api, use memcpy
  // modified 22dec2003 mhm: bcopy( ptr, &hdrbuf[150], RAS_HDR_SIZE );
  memcpy(&hdrbuf[150], ptr, RAS_HDR_SIZE );
  ptr += RAS_HDR_SIZE;
  
  if ( 5 == id->m_nDebugControl || (id->m_nDebugControl && IsDebugNameDefined(id, "WX")) ){
    for ( i = 150; i < 158; i++ ){
      printf("hdr[%d] = %d c = %c\n",
	     i, hdrbuf[i], hdrbuf[i] );
	  vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) WxStations[%d] = %s, hdr[%d] = %d, c = %c\n",
	    cdwThreadId, wx, WxStations[wx] , i, hdrbuf[i], hdrbuf[i] );
    }
  }

  msglen = (hdrbuf[8] << 24) + (hdrbuf[9] << 16) + (hdrbuf[10] << 8) +
    hdrbuf[11];
  printf("msglen=%d\n", msglen );
  D_CONTROL("WX", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) WxStations[%d] = %s, msglen=%d\n",
	    cdwThreadId, wx, WxStations[wx] , msglen ));

  days_since_1970 = hdrbuf[2] * 256 + hdrbuf[3] - 1;
  D_CONTROL("WX", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) WxStations[%d] = %s, days_since_1970=%d\n",
	    cdwThreadId, wx, WxStations[wx] , days_since_1970 ));
  secs_since_midnight = (hdrbuf[4] << 24) + (hdrbuf[5] << 16) + (hdrbuf[6] << 8) +
    hdrbuf[7];
  D_CONTROL("WX", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) WxStations[%d] = %s, secs_since_midnight=%d\n",
	    cdwThreadId, wx, WxStations[wx] , secs_since_midnight ));

  secs = time(0);		       
  filesecs = (days_since_1970 * 24 * 60 * 60) + secs_since_midnight;
  if ( secs - filesecs > 3600 ){
    /* file is more than one hour old */
    D_CONTROL("WX", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) WxStations[%d] = %s, File is more than 1 hour old = %d secs <now:%d,file:%d>\n",
	    cdwThreadId, wx, WxStations[wx] , secs - filesecs, secs, filesecs ));
    return( FAIL );
  }

  BlockLen = (hdrbuf[132] << 24) + (hdrbuf[133] << 16) + (hdrbuf[134] << 8) +
    hdrbuf[135];
  
  Len1 = 136 + BlockLen;
  TrailerLen = msglen - Len1;
  
  bufsize = msglen - HDR_SIZE;

  //printf("Len1=%d TrailerLen=%d msglen=%d BlockLen=%d bufsize=%d\n",
	 //Len1, TrailerLen, msglen, BlockLen, bufsize);
  //fflush( stdout );

  
  elev = (hdrbuf[58] << 8) + hdrbuf[59];

  iscale5 = 0xA002;
  
  /* assign category threshold values. Scale up values if
     indicated by comparison of ICATT(0) to ISCALE5 */
  nc = 0;
  for (n = 60; n < 91; n += 2){
    icatt[nc] = (hdrbuf[n] << 8) + hdrbuf[n + 1];
    ival0 = hdrbuf[n + 1];
    ival1 = hdrbuf[n];
    if ( ival0 % 2 == 0 ){
      icats[nc] = ival0;
    } else {
      icats[nc] = -1 * ival0;
    }
    nc++;
		if ( nc > 15 ) nc = 15;
  }
  if (icatt[0] >= iscale5 ){
    for ( nc = 0; nc < 16; nc++ ){
      icats[nc] = 5 * icats[nc];
    }
  }
  /* ADJUST '0' POSITION CATEGORY BACK TO 0 ('BACKGROUND COLOR')  */
  icats[0] = 0;

  nrows = (hdrbuf[154] << 8) + hdrbuf[155];
  
  if ( bufsize <= 0 || bufsize > 200000 ){
    vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Bad bufsize=%d, station=%s\n",
      cdwThreadId, bufsize, WxStations[wx]);
    return( FALSE );
  }
  
  if ( !(buf = vo_malloc(bufsize)) ){
    vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Bad malloc of bufsize=%d, station=%s\n",
      cdwThreadId, bufsize, WxStations[wx]);
    return( FALSE );
  }


  // mhm 22dec2003 bcopy( ptr, buf, bufsize - RAS_HDR_SIZE );
  memcpy( buf, ptr, bufsize - RAS_HDR_SIZE );

  if ( pflag ){
    printf("bufsize=%d\n", bufsize );
  }
  if ( pflag ){
    for ( i = 0; i < 100; i++ ){
      printf("Raster buf[%d] = %d c = %c\n", i, buf[i], buf[i] );
    }
  }
  nrun_marker = 0;
  nr = 0;
  while ( nr < nrows ){
    pixcount = 0;
    ival0 = buf[nrun_marker+1];
    ival1 = buf[nrun_marker];
    
    /* get number of runs in this raster row */
    nruns = (buf[nrun_marker] << 8) + buf[nrun_marker+1];
    npos1 = nrun_marker + 2;
    npos2 = npos1 + nruns;
    if ( npos2 > bufsize - RAS_HDR_SIZE ){
      vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Bad nruns=%d npos2=%d bufsize = %d, WxStations[%d] = %s\n", 
        cdwThreadId, nruns, npos2, bufsize, wx, WxStations[wx]);
      s_free( buf );
      return( FAIL );
    }
  
    D_CONTROL("WX", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) nrun_marker=%d, nrow=%d, nruns=%d, npos1=%d npos2=%d\n",
        cdwThreadId, nrun_marker, nr, nruns, npos1, npos2 ));
    
    for ( nbyte = npos1; nbyte < npos2 && nbyte < bufsize; nbyte++ ){
      runlen = buf[nbyte] >> 4;
      colorval = buf[nbyte] & 0x0f;
      for ( i = 0; i < runlen; i++ ){
        raslinebuf[nr][pixcount++] = colorval;
      }
    }
    nrun_marker = npos2;
    
    nr++;
    pixcount = 0;
  }

  s_free( buf );

  /* create a texture with the wx data */
  GetRasWxTexImage( nrows, wx, dbscale );
  WxGenTexture( id, wx );  
  return( TRUE );
}



char *GetWxZip( struct InstanceData* id, struct MemoryStruct *chunk )
{
  static unsigned char *wxbuf;
	char *outbufptr = NULL;
	int unzipbytes;
	const DWORD cdwThreadId = GetCurrentThreadId();


	// use new memory unzip 
	if ( !chunk || !chunk->size ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Invalid buffer'%s'\n", cdwThreadId, id->m_strWxZipname);
		return(NULL);
	}

	if ( (unzipbytes = imunzip(id->m_strWxZipname, chunk->memory, chunk->size, &outbufptr )) <= 0 ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) failed to unzip '%s'\n", cdwThreadId, id->m_strWxZipname);
		return(NULL);
	}

  if ( !wxbuf ){
    wxbuf = malloc(WX_BUFSIZE);
  }
  wxbuf[ WX_BUFSIZE - 1 ] = '\0';
  
	if ( unzipbytes <=  WX_BUFSIZE ){
		memcpy( wxbuf, outbufptr, unzipbytes );
		wxbuf[unzipbytes] = '\0';
	} else {
		D_CONTROL("WX", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) unzipbytes >  WX_BUFSIZE\n", cdwThreadId ));
		free( outbufptr );
		return( NULL );
	}
 
	free( outbufptr );

  return( wxbuf );
}


ClearWx( struct InstanceData* id, int wx )
{
  int row, col, ARows, ACols, alpha;


  ARows = WX_IMAGE_SIZE;
  ACols = WX_IMAGE_SIZE;

  for ( row = 0; row < ARows; row++ ){
    for ( col = 0; col < ACols; col++ ){
      
     
      //wximage[wx][ARows - row - 1][col][0] = 0;
      //wximage[wx][ARows - row - 1][col][1] = 0;
      // wximage[wx][ARows - row - 1][col][2] = 0;
      
      alpha = 0;
      wximage[wx][ARows - row - 1][col][3] = alpha;
    }
  }
  WxGenTexture( id, wx );
}

int LoadWxTex(struct InstanceData* id)
{
  int ipcode, i, msglen, elev,
    iscale5, nc, n, nr, nrun_marker, nruns, npos1, npos2, radialbin, nbyte,
    runlen, colorval, rbin, bufsize, ival;
  unsigned char hdrbuf[HDR_SIZE + RAS_HDR_SIZE];
  unsigned char *ptr;
  int RadialBlockLen, Len1, TrailerLen;
  int icatt[ N_COLORS ];
  int icats[ N_COLORS ];
  int ival0, ival1, dbscale;
  unsigned char *buf = NULL;
  struct MemoryStruct chunk;
  char wxfilename[256];
  char url[256];
  int wx;
  int datalevel[16];
  int pflag = 0;
  const DWORD cdwThreadId = GetCurrentThreadId();
  
  /* load in the weather file in radial format and make a texture */
  /* test raster file is /home/packages/sn.0126 */
  /* test radial file is /home/packages/sn.0160 */

  if(!NActiveRadars)
	  return (FALSE);

  for ( wx = 0; wx < NActiveRadars; wx++ ){
    if(WAIT_TIMEOUT != WaitForSingleObject(id->m_evShutdown, 1))
      break;

    /* clear out buffer */
    chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
    chunk.size = 0;    /* no data at this point */
    
    sprintf_s(wxfilename, _countof(wxfilename), "data/DC.radar/%s/latestzip", WxStations[wx] );
    sprintf_s(url,_countof(url), "%s/%s", GetWxSrvr(id), wxfilename );
    D_CONTROL("WX", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) WxStations <%s> wxfilename <%s> url <%s>\n", cdwThreadId, WxStations[wx], url, wxfilename));
	 
    D_CONTROL("WX", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlPage - id <%x> tmpstr <%s>.\n", 
		cdwThreadId, id, url ));

    if ( GetCurlPage( id, &chunk, url, SM_HTTPS, 1, 10L ) == FALSE ){
      vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Wx read failed for '%s'\n", cdwThreadId, url );
	  // Only Change weather server if the network fails, 404 means the wx data is just not on server
	  if(!chunk.memory || (chunk.memory && !strstr(chunk.memory, "404 Not Found" ))) 
	    GetNxtWxSrvr(id);
	  free_chunk_memory(&chunk);
      continue;
    }
    if ( chunk.memory && strstr(chunk.memory, "404 Not Found" ) ){
      vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Wx file not found for '%s'\n", cdwThreadId, url );
	  free_chunk_memory(&chunk);
      continue;
    }
	if ( chunk.memory && strstr(chunk.memory, "401 Authorization Required" ) ){
      vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Wx file server requires authorization for '%s'\n", cdwThreadId, url );
	  free_chunk_memory(&chunk);
	  SleepEx(1000, FALSE);
      return (FALSE);
    }
    
	ptr = GetWxZip( id, &chunk );
	free_chunk_memory(&chunk);
    if ( !ptr ){
      vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) GetWxZip read failed for '%s'\n", cdwThreadId, url );
      continue;
    }
   
    /* lop off the file header information */
    ptr += FILE_HDR_SIZE;
    
    // 22dec2003 mhm: bcopy( ptr, hdrbuf, HDR_SIZE );
    memcpy( hdrbuf, ptr, HDR_SIZE );
    
    ptr += HDR_SIZE;
    
    if ( strstr(hdrbuf, "404 Not Found" ) ){
      vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Wx 404 Not Found read failed for '%s'\n", cdwThreadId, url );
      continue;
    }
    
    ipcode = hdrbuf[136] * 256 + hdrbuf[137];
    KmPerBin = ( (hdrbuf[146] << 8) + hdrbuf[147]) * .001;
    NMiPerBin = KmPerBin * NMILES_PER_KM;
    
    ival = (hdrbuf[20] << 24) + (hdrbuf[21] << 16) + (hdrbuf[22] << 8) +
      hdrbuf[23];
    radarlat[wx] = (double) ival / 1000.0;
    ival = (hdrbuf[24] << 24) + (hdrbuf[25] << 16) + (hdrbuf[26] << 8) +
      hdrbuf[27];
    radarlong[wx] = (double) ival / 1000.0;
    
    if ( pflag ){
      for ( i = 0; i < 158; i++ ){
        printf("hdr[%d] = %d c = %c\n",
          i, hdrbuf[i], hdrbuf[i] );
      }
      /*  get data level breakpoint values */
      printf("\ndata levels for wx[%d] = %s\n", wx, WxStations[wx] );
    }
    
    for (i = 0; i < 16; i++ ){
      datalevel[i] = ( hdrbuf[ 60 + (i * 2)] << 8 ) + (hdrbuf[ 61 + (i * 2)] );
      
      if ( pflag ){
        printf("data level[%d] = %d\n", i, datalevel[i] );
      }
    }
    if ( datalevel[1] > 256 ){
      /* low level scale */
      dbscale = 0;
    } else {
      dbscale = 1;
    }
    
    
    if ( pflag ){
      printf("ipcode=%d\n");
      fflush(stdout);
    }
    
    if ( ipcode != 0xAF1F ){
      if ( pflag ){
        printf("Raster type, ipcode = %x\n", ipcode );
      }
    } else {
      /* 16-level radial graphic products */
    }
    if ( ipcode == 0xba07 ){
      /* 16-level raster graphic product */
      LoadRasterWx( id, ptr, hdrbuf, wx, dbscale );
      continue;
    }
    msglen = (hdrbuf[8] << 24) + (hdrbuf[9] << 16) + (hdrbuf[10] << 8) +
      hdrbuf[11];
    if ( pflag ){
      printf("msglen=%d\n", msglen );
    }
    RadialBlockLen = (hdrbuf[132] << 24) + (hdrbuf[133] << 16) + (hdrbuf[134] << 8) +
      hdrbuf[135];
    
    Len1 = 136 + RadialBlockLen;
    TrailerLen = msglen - Len1;
    
    bufsize = msglen - HDR_SIZE;
    
    if ( pflag ){
      printf("Len1=%d TrailerLen=%d msglen=%d RadialBlockLen=%d bufsize=%d\n",
        Len1, TrailerLen, msglen, RadialBlockLen, bufsize);
      fflush( stdout );
    }
    /* find number of radials in the product and elevation angle */
    nbins = (hdrbuf[140] << 8) + hdrbuf[141];
    
    if ( pflag ){
      printf("nbins=%d\n", nbins);
    }
    
    nradials = (hdrbuf[148] << 8) + hdrbuf[149];
    
    elev = (hdrbuf[58] << 8) + hdrbuf[59];
    
    iscale5 = 0xA002;
    
    /* assign category threshold values. Scale up values if
    indicated by comparison of ICATT(0) to ISCALE5 */
    nc = 0;
    for (n = 60; n < 91; n += 2){
      icatt[nc] = (hdrbuf[n] << 8) + hdrbuf[n + 1];
      ival0 = hdrbuf[n + 1];
      ival1 = hdrbuf[n];
      if ( ival0 % 2 == 0 ){
        icats[nc] = ival0;
      } else {
        icats[nc] = -1 * ival0;
      }
      nc++;
			if ( nc > 15 ) nc = 15;
    }
    if (icatt[0] >= iscale5 ){
      for ( nc = 0; nc < 16; nc++ ){
        icats[nc] = 5 * icats[nc];
      }
    }
    /* ADJUST '0' POSITION CATEGORY BACK TO 0 ('BACKGROUND COLOR')  */
    icats[0] = 0;
    
    nr = 0;
    nrun_marker = 0;
    
    if ( bufsize <= 0 || bufsize > 200000 ){
      vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Bad bufsize=%d, station=%s\n",
        cdwThreadId, bufsize, WxStations[wx]);
      continue;
    }
    if ( !(buf = vo_malloc(bufsize)) ){
      vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Bad malloc of bufsize=%d, station=%s\n",
        cdwThreadId, bufsize, WxStations[wx]);
      continue;
    }

    ptr += HDR_SIZE;
    // 22dec2003 mhm: bcopy( ptr, buf, bufsize );
    memcpy( buf, ptr, bufsize );
    
    while ( nr < nradials ){
      ival0 = buf[nrun_marker+1];
      ival1 = buf[nrun_marker];
      
      /* get number of runs in this radial */
      nruns = 2 * ( (buf[nrun_marker] << 8) + buf[nrun_marker+1] );
      iazmuth[nr] = (buf[nrun_marker + 2] << 8) + buf[nrun_marker+3];
      npos1 = nrun_marker + 6;
      npos2 = npos1 + nruns - 1;
      
      /* get run length from high order bits and color value
      from low-order bits of this byte, then plug into radial array */
      if ( pflag ){
        printf("radial %d iazmuth=%d\n", nr, iazmuth[nr] );
      }
      radialbin = 0;
      
      for ( nbyte = npos1; nbyte <= npos2; nbyte++ ){
        runlen = buf[nbyte] >> 4;
        colorval = buf[nbyte] & 0x0f;
        
        for ( rbin = radialbin; rbin < radialbin + runlen; rbin++ ){
          radbuf[nr][rbin] = colorval;
        }
        radialbin = rbin;
      }
      nrun_marker = npos2 + 1;
      nr++;
    }
    
    s_free( buf );
    
    /* create a texture with the wx data */
    GetWxTexImage( id, wx );
    // cant do in thres WxGenTexture( wx );
  }  
  
  return(TRUE);
}






int ThreadLoadWxTex(struct InstanceData* id)
{
  int ipcode, i, msglen, elev,
    iscale5, nc, n, nr, nrun_marker, nruns, npos1, npos2, radialbin, nbyte,
    runlen, colorval, rbin, bufsize, ival;
  unsigned char hdrbuf[HDR_SIZE + RAS_HDR_SIZE];
  unsigned char *ptr;
  int RadialBlockLen, Len1, TrailerLen;
  int icatt[ N_COLORS ];
  int icats[ N_COLORS ];
  int ival0, ival1, dbscale;
  unsigned char *buf = NULL;
  struct MemoryStruct chunk;
  char wxfilename[256];
  char url[256];
  int wx;
  int datalevel[16];
  int pflag = 0;
  const DWORD cdwThreadId = GetCurrentThreadId();
  
  /* load in the weather file in radial format and make a texture */
  /* test raster file is /home/packages/sn.0126 */
  /* test radial file is /home/packages/sn.0160 */
  D_CONTROL("WX", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Starting\n", cdwThreadId));

  NActiveRadars = 0;
  for ( wx = 0; wx < N_WX_RADARS; wx++ ){
    if ( !WxStations[wx] ){
      break;
    }
    NActiveRadars++;
  }
  
  for ( wx = 0; wx < NActiveRadars; wx++ ){
    /* clear out buffer */
    chunk.memory=NULL; /* we expect realloc(NULL, size) to work */
    chunk.size = 0;    /* no data at this point */
    
    sprintf_s(wxfilename, _countof(wxfilename), "data/DC.radar/%s/latestzip", WxStations[wx] );
    sprintf_s(url, _countof(url), "%s/%s", GetWxSrvr(id), wxfilename );
    D_CONTROL("WX", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) WxStations <%s> wxfilename <%s> url <%s>\n", cdwThreadId, WxStations[wx], url, wxfilename));

    D_CONTROL("WX", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlPage - id <%x> tmpstr <%s>.\n", 
		cdwThreadId, id, url ) );

    if ( GetCurlPage( id, &chunk, url, SM_HTTPS, 1, 10L ) == FALSE ){
      vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Wx read failed for '%s'\n", cdwThreadId, url );
      ClearWx( id, wx );
	  GetNxtWxSrvr(id);
      if ( chunk.memory ){
        free( chunk.memory );
        chunk.memory = NULL;
      }
      continue;
    } else {
      // not thread safe? MGLGridText(statusgrid, 22, 0, "wxname");
      //sprintf(tmpstr, "%s", WxStations[wx]);
      //MGLGridText(statusgrid, 22, 1, tmpstr);
		}

    if ( chunk.size && chunk.memory && strstr(chunk.memory, "404 Not Found" ) ){
      vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) 404 Not Found, Wx file not found for '%s'\n", cdwThreadId, url );
      // clear out old weather
      ClearWx( id, wx );
      if ( chunk.memory ){
        free( chunk.memory );
        chunk.memory = NULL;
      }
      continue;
    }
    
	ptr = GetWxZip( id, &chunk );
	free_chunk_memory(&chunk);
    if ( !ptr ) {
      vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) GetWxZip read failed for '%s'\n", cdwThreadId, url );
      ClearWx( id, wx );
      continue;
    }
    LastGoodWxSecs = time(0);

    /* lop off the file header information */
    ptr += FILE_HDR_SIZE;
    
    // 22dec2003 mhm: bcopy( ptr, hdrbuf, HDR_SIZE );
    memcpy( hdrbuf, ptr, HDR_SIZE - 1 );
		hdrbuf[HDR_SIZE - 1] = '\0';
    
    ptr += HDR_SIZE;
    
    if ( strstr(hdrbuf, "404 Not Found" ) ){
      vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Wx 404 Not Found read failed for '%s'\n", cdwThreadId, url );
      continue;
    }
    
    ipcode = hdrbuf[136] * 256 + hdrbuf[137];
    KmPerBin = ( (hdrbuf[146] << 8) + hdrbuf[147]) * .001;
    NMiPerBin = KmPerBin * NMILES_PER_KM;
    
    ival = (hdrbuf[20] << 24) + (hdrbuf[21] << 16) + (hdrbuf[22] << 8) +
      hdrbuf[23];
    radarlat[wx] = (double) ival / 1000.0;
    ival = (hdrbuf[24] << 24) + (hdrbuf[25] << 16) + (hdrbuf[26] << 8) +
      hdrbuf[27];
    radarlong[wx] = (double) ival / 1000.0;
    
    if ( 5 == id->m_nDebugControl || (id->m_nDebugControl && IsDebugNameDefined(id, "WX")) ){
      for ( i = 0; i < HDR_SIZE; i++ ){
        vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) hdr[%d] = %d c = %c\n",
          cdwThreadId, i, hdrbuf[i], hdrbuf[i] );
      }
      /*  get data level breakpoint values */
      D_CONTROL("WX", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X)\n data levels for wx[%d] = %s\n", cdwThreadId, wx, WxStations[wx] ));
    }
    
    for (i = 0; i < 16; i++ ){
      datalevel[i] = ( hdrbuf[ 60 + (i * 2)] << 8 ) + (hdrbuf[ 61 + (i * 2)] );
    }
    if ( datalevel[1] > 256 ){
      /* low level scale */
      dbscale = 0;
    } else {
      dbscale = 1;
    }
    
    if ( ipcode != 0xAF1F ){
      D_CONTROL("WX", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Raster type, ipcode = %x\n", cdwThreadId, ipcode ));
    } else {
      /* 16-level radial graphic products */
    }
    if ( ipcode == 0xba07 ){
      /* 16-level raster graphic product */
      LoadRasterWx( id, ptr, hdrbuf, wx, dbscale );
      if ( chunk.memory ){
        free( chunk.memory );
        chunk.memory = NULL;
      }
      continue;
    }
    msglen = (hdrbuf[8] << 24) + (hdrbuf[9] << 16) + (hdrbuf[10] << 8) +
      hdrbuf[11];
    D_CONTROL("WX", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) msglen=%d\n", cdwThreadId, msglen ));
    RadialBlockLen = (hdrbuf[132] << 24) + (hdrbuf[133] << 16) + (hdrbuf[134] << 8) +
      hdrbuf[135];
    
    Len1 = 136 + RadialBlockLen;
    TrailerLen = msglen - Len1;
    
    bufsize = msglen - HDR_SIZE;
    
    D_CONTROL("WX", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Len1=%d TrailerLen=%d msglen=%d RadialBlockLen=%d bufsize=%d\n",
        cdwThreadId, Len1, TrailerLen, msglen, RadialBlockLen, bufsize));
    /* find number of radials in the product and elevation angle */
    nbins = (hdrbuf[140] << 8) + hdrbuf[141];
   
    nradials = (hdrbuf[148] << 8) + hdrbuf[149];
    
    elev = (hdrbuf[58] << 8) + hdrbuf[59];
    
    iscale5 = 0xA002;
    
    /* assign category threshold values. Scale up values if
    indicated by comparison of ICATT(0) to ISCALE5 */
    nc = 0;
    for (n = 60; n < 91; n += 2){
      icatt[nc] = (hdrbuf[n] << 8) + hdrbuf[n + 1];
      ival0 = hdrbuf[n + 1];
      ival1 = hdrbuf[n];
      if ( ival0 % 2 == 0 ){
        icats[nc] = ival0;
      } else {
        icats[nc] = -1 * ival0;
      }
      nc++;
			if ( nc > 15 ) nc = 15;
    }
    if (icatt[0] >= iscale5 ){
      for ( nc = 0; nc < 16; nc++ ){
        icats[nc] = 5 * icats[nc];
      }
    }
    /* ADJUST '0' POSITION CATEGORY BACK TO 0 ('BACKGROUND COLOR')  */
    icats[0] = 0;
    
    nr = 0;
    nrun_marker = 0;
    
    if ( bufsize <= 0 || bufsize > 200000 ){
      vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Bad bufsize=%d, station=%s\n",
        cdwThreadId, bufsize, WxStations[wx]);
      continue;
    }

    if ( !(buf = vo_malloc(bufsize)) ){
      vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Bad malloc of bufsize=%d, station=%s\n",
        cdwThreadId, bufsize, WxStations[wx]);
      continue;
    }


    ptr += HDR_SIZE;
    // 22dec2003 mhm: bcopy( ptr, buf, bufsize );
    memcpy( buf, ptr, bufsize );
    
    while ( nr < nradials ){
      ival0 = buf[nrun_marker+1];
      ival1 = buf[nrun_marker];
      
      /* get number of runs in this radial */
      nruns = 2 * ( (buf[nrun_marker] << 8) + buf[nrun_marker+1] );
      iazmuth[nr] = (buf[nrun_marker + 2] << 8) + buf[nrun_marker+3];
      npos1 = nrun_marker + 6;
      npos2 = npos1 + nruns - 1;
      
      /* get run length from high order bits and color value
      from low-order bits of this byte, then plug into radial array */

      radialbin = 0;
      
      for ( nbyte = npos1; nbyte <= npos2; nbyte++ ){
        runlen = buf[nbyte] >> 4;
        colorval = buf[nbyte] & 0x0f;
        
        for ( rbin = radialbin; rbin < radialbin + runlen; rbin++ ){
          radbuf[nr][rbin] = colorval;
        }
        radialbin = rbin;
      }
      nrun_marker = npos2 + 1;
      nr++;
    }
    
    s_free( buf );
    
    /* create a texture with the wx data */
    GetWxTexImage(id, wx );
   
    if ( chunk.memory ){
      free( chunk.memory );
      chunk.memory = NULL;
    }
  }
    
  // cant do this in thread WxGenTextures();
  return(TRUE);
}

void ThreadWx(struct InstanceData* id)
{
  g_ThreadIsLoading = TRUE;
  ThreadLoadWxTex(id);
  g_ThreadIsLoading = FALSE;
  g_WXThreadsLoaded = TRUE;  // we have loaded threads with data-- keep from loading wx before this thread fills them
	ForceRefresh = TRUE;
}

void WxGenTexture(struct InstanceData* id, int wx)
{
  int wx_square_pixels = FALSE;

  /* bind in the Weather Wx image */  
  glBindTexture( GL_TEXTURE_2D,  id->m_gluWXTextures[wx] );

  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, WX_IMAGE_SIZE, WX_IMAGE_SIZE, 0, GL_RGBA,
    GL_UNSIGNED_BYTE, wximage[wx] );
  if ( wx_square_pixels ){
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  } else {
    /* Linear Filtering */
    /* produces a smoothed image, more accurate and realistic? */
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  }

	glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture  
}


void WxGenTextures(struct InstanceData* id)
{
  int wx;
  int wx_square_pixels = FALSE;

  NActiveRadars = 0;
  for ( wx = 0; wx < N_WX_RADARS; wx++ ){
    if ( !WxStations[wx] ){
      break;
    }
    NActiveRadars++;
  }

  if(0 == id->m_gluWXTextures[0])
    glGenTextures(NActiveRadars, &id->m_gluWXTextures[0] );
    for(wx = 0; wx < NActiveRadars; wx++){
		D_CONTROL("WX", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Generated Weather Texture %d: %d.\n", GetCurrentThreadId(), wx, id->m_gluWXTextures[wx])); 
	}
	//CkGLError();
  for ( wx = 0; wx <  NActiveRadars; wx++ ){
    /* bind in the Weather Wx image */
    WxGenTexture(id, wx);
  }
}

int drawWx( struct InstanceData* id, int wx )
{
  double deltalng, realdelta;
  double AirSize, newlat, newlng, rads, degree_long,
    xmiles, ymiles;
  GLfloat params[4];
  struct rect r1, r2;
  
  if(0 == id->m_gluWXTextures[0] || (0.0 == radarlong[wx] && 0.0 == radarlat[wx]))
	  return (SUCCEED);

  r1.left.x = id->m_sCurLayout.m_dWorldXmin;
  r1.right.x = id->m_sCurLayout.m_dWorldXmax;
  r1.left.y = id->m_sCurLayout.m_dWorldYmin;
  r1.right.y = id->m_sCurLayout.m_dWorldYmax;

  rads = radarlat[wx] * M_PI / 180.0;
  degree_long = 69.172 * cos( rads );

  xmiles = (WX_IMAGE_NMILES * MILES_PER_NMILE) / 2; /* get radius not diameter */
  ymiles = (WX_IMAGE_NMILES * MILES_PER_NMILE) / 2;

  /* calculate size of radar box */
  newlat = radarlat[wx] + (ymiles / 69.172);
  AirSize = newlat - radarlat[wx];

  r2.left.x = radarlong[wx] - AirSize;
  r2.right.x = radarlong[wx] + AirSize;
  r2.left.y = radarlat[wx] - AirSize;
  r2.right.y = radarlat[wx] + AirSize;

  // Pre-clip 
  if( !Intersects(r1,r2) )
  	  return (SUCCEED);

  /* reset the color to white so textures will appear */
  glColor3f(1.0, 1.0, 1.0 );

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  /* glortho (left, right, bottom , top, near, far ) */
  /* glOrtho(0.0f, w, 0.0f, h, 0.0f, 1.0f); */
  glOrtho(id->m_sCurLayout.m_dWorldXmin, id->m_sCurLayout.m_dWorldXmax, 
	      id->m_sCurLayout.m_dWorldYmin, id->m_sCurLayout.m_dWorldYmax,
          0.0f, 1.0f);

  RotateView(id, id->m_sCurLayout.m_dWorldXmax, id->m_sCurLayout.m_dWorldXmin, 
		id->m_sCurLayout.m_dWorldYmax, id->m_sCurLayout.m_dWorldYmin);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_TEXTURE_2D);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPEAT);
  glBindTexture( GL_TEXTURE_2D, id->m_gluWXTextures[wx] );

  /* solid with alpha masking */
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_ALPHA_TEST);


  glDepthFunc(GL_LEQUAL);

  glPushMatrix();
  glLoadIdentity();

  if ( projflag ){
    deltalng = radarlong[wx] - LngOrig;
    realdelta = deltalng * cos( radarlat[wx] * M_PI / 180.0 );
    newlng = LngOrig + realdelta;
    glTranslatef( (GLfloat) newlng,  (GLfloat)radarlat[wx], (GLfloat) trans[2] );
  } else {
    glTranslatef( (GLfloat) radarlong[wx],  (GLfloat) radarlat[wx], (GLfloat) trans[2] );
  }

  //params[0] = WxIntensity;
  //params[1] = WxIntensity;
  //params[2] = WxIntensity;
  params[0] = 1.0f;
  params[1] = 1.0f;
  params[2] = 1.0f;
  params[3] = 1.0f;

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, params);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND );
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
  //glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, params);


  glEnable(GL_TEXTURE_2D);
  glColor4f( WxIntensity, WxIntensity, WxIntensity, 1.0f);
  //glColor4f(0.0f, 0.0f, 0.0f, 1.0f);

  glBegin(GL_QUADS);
  glTexCoord2f(0.0f, 0.0f); glVertex3f((GLfloat) -AirSize, (GLfloat) -AirSize, 0.0);
  glTexCoord2f(0.0f, 1.0f); glVertex3f((GLfloat) -AirSize, (GLfloat) AirSize, 0.0);
  glTexCoord2f(1.0f, 1.0f); glVertex3f((GLfloat) AirSize, (GLfloat) AirSize, 0.0);
  glTexCoord2f(1.0f, 0.0f); glVertex3f((GLfloat) AirSize, (GLfloat) -AirSize, 0.0);

  glEnd();

  glFlush();

  glPopMatrix();

  glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture

  glDisable(GL_TEXTURE_ENV);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_DEPTH_TEST);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
  return( SUCCEED );
}

//VOID WxThread(PVOID pvoid)
//{
//	static int WxThreadCount;
//
//  vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Thread %d starting\n", GetCurrentThreadId(), WxThreadCount++ );
//  for (;;){
//    ThreadWx();
//    //LastWxThreadSecs = time(0);
//	//	ForceRefresh = TRUE;
//    SleepEx(360000, FALSE);
//  }
//
//}
