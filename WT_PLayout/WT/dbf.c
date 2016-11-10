#include < windows.h >
#include <fcntl.h>
#include "unzip.h"
#include "tailor.h"
#include "gd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <errno.h>	/*declares extern errno*/
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
// #include <GL\glaux.h>		// Header File For The Glaux Library
#include "vo.h"
#include "vo_extern.h"
#include "SSI.h"
#include "ExecUtil.h"
#include "vo_db.h"
#include "TexFont.h"
#include "shapefil.h"
#include "Tracks.h"
#include "nexrad.h"
#include "mgl.h"
#include "resource.h"           // About box resource identifiers.
#include "gfx.h"
#include "fnmatch.h"
#include "InstanceData.h"
#include "Curl.h"
#include "dbg_mem.h"

//Win32:08-19-03
//#pragma comment ( lib, "SDL.lib" )
//#pragma comment ( lib, "SDLmain.lib" )
//#pragma comment ( lib, "GLU32.lib" )
//#pragma comment ( lib, "GLUT32.lib" )
//#pragma comment ( lib, "OPENGL32.lib" )
extern char *GetNextField( char *recstr, char *tmpstr, int tmpsize );
extern int GetLocalFile( struct InstanceData* id, const char* localpath, const char* filename, const char* remotepath, int eMode);
extern int HavePermission(struct InstanceData* id, const char* strPerm);
extern int IsDebugNameDefined(struct InstanceData* id, const char* strDebugName);
extern ARPT *FindArptRow( struct InstanceData* id, char *arpt, int match_type );

#define D_CONTROL(vcolname, exp) {if ( 5 == id->m_nDebugControl || (id->m_nDebugControl && IsDebugNameDefined(id, vcolname)) ) {exp;}}
#define IS_D_CONTROL(vcolname) ( 5 == id->m_nDebugControl || (id->m_nDebugControl && IsDebugNameDefined(id, vcolname)) )


FIX* g_pFixes;				// Array of all Fixes
FIX* g_pFixesLow;			// Array of Low Fixes
FIX* g_pFixesHigh;			// Array of High Fixes
FIX* g_pFixesTerm;			// Array of Terminal Fixes
FIX* g_pFixesPref;			// Array of Preferred Fixes
int g_nFixCount;			// Count of Fixes
int g_nFixCountLow;			// Count of Low Fixes
int g_nFixCountHigh;		// Count of High Fixes
int g_nFixCountTerm;		// Count of Terminal Fixes
int g_bFixesLoaded;			// Fixes loaded flag
int g_nFixCountPref;		// Count of Preferred Fixes


int append_lg_buffers(char** dest, int* dest_size, char* src, int src_size)
{
	int result = TRUE;
	char* save_buffer;
	save_buffer = *dest;

	if (NULL == *dest) {
		*dest = malloc(src_size);
		if (NULL == *dest) return FALSE;
		memcpy(*dest, src, src_size);
		*dest_size = src_size;
		return result;
	}

	*dest = realloc(*dest, *dest_size + src_size);
	if (NULL != *dest) {
	  memcpy(*dest + *dest_size, src, src_size);
      *dest_size += src_size;
	} else {
		*dest = save_buffer;
		result = FALSE;
	}
	return result;
}
int compareFix( const void *arg1, const void *arg2 )
{
  FIX *Fix1, *Fix2;

  //char *str1, *str2;
  /* Compare all of both strings: */
  //str1 = *arg1;
  //str2 = *arg2;

  Fix1 = (FIX *) arg1;
  Fix2 = (FIX *) arg2;

  return _strcmpi( Fix1->fixname, Fix2->fixname );
}

void LoadAirAsiaFixFile( struct InstanceData* id, char *pFilename, FIX** ppFixes, int *pCount )
{
	FILE *in = NULL;
	char filepath[256] = {0};
	char linebuf[80] = {0};
	char val[15] = {0};
	FIX *Fixes = NULL;
	char *s = NULL;
	int rcnt = 0;
	int lcnt = 0;
	int degs, minutes, secs;
	float fracs, sign;
	int bytes1, bytes2;
	const DWORD cdwThreadId = GetCurrentThreadId();

	sprintf_s(filepath, _countof(filepath), "%s%s", id->m_strMapDir, pFilename);
	if ( fopen_s(&in, filepath, "r") ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not open file %s.\n", cdwThreadId, filepath);	
		return;
	}

	// Allocate Buffer for Incoming Records
	Fixes = calloc(15000, sizeof(FIX));

	// Read Records and load fix data
	while ( !feof(in) ){
		memset(linebuf, 0, sizeof linebuf);
		if ( !fgets(linebuf, sizeof linebuf, in) ){
			/* ran out of data */
			break;
		}else{
			// skip over headers
			if(++lcnt < 3)
				continue;
		}
		if ( !strncmp( linebuf, "EOD", 3 ) ){
			break; // End of File
		}

		// Get ID
		s = GetNextField(linebuf, Fixes[rcnt].fixname, 7);

		// Get Lat
		s = GetNextField(s, val, 15);
		sign = 1.0f;
		if( 'S' == val[0] || 's' == val[0] ) sign = -1.0f;
		degs = (10 * (val[1] - '0') ) + val[2] - '0';
		minutes = (10 * (val[4] - '0') ) + val[5] - '0';
		secs = (10 * (val[7] - '0') ) + val[8] - '0';
		fracs = (100.0f * (val[10] - '0') );
		if ( isdigit(val[11]) ){
			fracs += (10.0f * (val[11] - '0'));
		}
		if ( isdigit(val[12] ) ){
			fracs += val[12] - '0';
		}
		fracs /= 1000.0;
		Fixes[rcnt].lat = sign * ((float) degs + ( minutes / 60.0f ) + (secs / 3600.0f) + (fracs / 3600.0f));

		// Get Lng
		s = GetNextField(s, val, 15);
		sign = 1.0f;
		if( 'W' == val[0] || 'W' == val[0] ) sign = -1.0f;
		degs = (100 * (val[1] - '0') ) + (10 * (val[2] - '0') ) + val[3] - '0';
		minutes = (10 * (val[5] - '0') ) + val[6] - '0';
		secs = (10 * (val[8] - '0') ) + val[9] - '0';
		fracs = (100.0f * (val[11] - '0') );
		if ( isdigit(val[12]) ){
			fracs += 10.0f * (val[12] - '0');
		} if ( isdigit(val[13]) ){
			fracs += val[13] - '0';
		}
		fracs /= 1000.0;
		Fixes[rcnt].lng = sign * ((float) degs + ( minutes / 60.0f ) + (secs / 3600.0f) + (fracs / 3600.0f));

		rcnt++;
	}

	if( IS_D_CONTROL( "AAFIXES" ) ){
		int i;
		for (i = 0; i < rcnt; i++){
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) FIX[%d]='%s',lat=%g lng=%g\n",
				cdwThreadId, i, Fixes[i].fixname, Fixes[i].lat, Fixes[i].lng );
		}
	}
	// Append to end of existing fix array
	bytes1 = *pCount * sizeof(FIX);
	bytes2 = rcnt * sizeof(FIX);
	if(append_lg_buffers((char**)ppFixes, &bytes1, (char*)Fixes, bytes2))
	{	
		*pCount += rcnt;
	} 

	free(Fixes);
}

FIX *LoadFixFile( struct InstanceData* id, char *filename, int *fixcountp )
{
  DBFHandle	hDBF;
  int		i, iRecord, reccount, fieldcount, goodflag, len;
  char	*pszFilename = NULL;
  int		nWidth, nDecimals;
  int		bHeader = 0;
  int		bRaw = 0;
  int		bMultiLine = 0;
  char	szTitle[12];
  char	szFormat[32];
  char namebuf[39];
  char icaobuf[12];
  char latbuf[11];
  char longbuf[11];
  int compareFix( const void *arg1, const void *arg2 );
  FIX *Fixes;
  int fixcount = 0;
  const DWORD cdwThreadId = GetCurrentThreadId();


  
  EnterCriticalSection(&id->m_csDBFSync);
  hDBF = DBFOpen(filename, "rb" );
  if( hDBF == NULL )
  {
    vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not open %s\n", cdwThreadId, filename );
	LeaveCriticalSection(&id->m_csDBFSync);
    return( 0 );
  }
  
  reccount = DBFGetRecordCount(hDBF);
  if( reccount <= 0 )
  {
    vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Bad file format for %s\n", cdwThreadId, filename );
	DBFClose( hDBF );
	LeaveCriticalSection(&id->m_csDBFSync);
	return( 0 );
  }

  fieldcount = DBFGetFieldCount(hDBF);
  
  if( fieldcount <= 0 )
  {
    vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Bad file format for %s\n", cdwThreadId, filename );
	DBFClose( hDBF );
	LeaveCriticalSection(&id->m_csDBFSync);
	return( 0 );
  } 
  
  for( i = 0; i < fieldcount; i++ )
  {
    DBFFieldType	eType;
    const char	 	*pszTypeName;
    
    eType = DBFGetFieldInfo( hDBF, i, szTitle, &nWidth, &nDecimals );
    if( eType == FTString )
      pszTypeName = "String";
    else if( eType == FTInteger )
      pszTypeName = "Integer";
    else if( eType == FTDouble )
      pszTypeName = "Double";
    else if( eType == FTInvalid )
      pszTypeName = "Invalid";
    
    //vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Field %d: Type=%s, Title=`%s', Width=%d, Decimals=%d\n",
    //  cdwThreadId, i, pszTypeName, szTitle, nWidth, nDecimals );
  }
  
  Fixes = calloc(sizeof(FIX), reccount );
	*fixcountp = reccount;
  

  for( iRecord = 0; iRecord < reccount; iRecord++ ){
    
    goodflag = 1;
    for( i = 0; i < fieldcount; i++ ){
      DBFFieldType	eType;
      
      eType = DBFGetFieldInfo( hDBF, i, szTitle, &nWidth, &nDecimals );
      
      if ( !strcmp( szTitle, "CTRY" ) ){
        _snprintf_s(szFormat, _countof(szFormat), _TRUNCATE, "%%-%ds", nWidth );
        _snprintf_s(namebuf, _countof(namebuf), _TRUNCATE, szFormat, DBFReadStringAttribute( hDBF, iRecord, i ) );
        STrimRight( namebuf );
        
        len = strlen( namebuf );
        if ( !strcmp( filename, "data/maps/WPTT_P.dbf" ) && 
          ( strcmp(namebuf, "US") && strcmp(namebuf, "CA") ) ){
          // only expect terminal fixes from US and Canada to avoid duplicates such as PORGY, that is
          // in both High and Terminal fixes
          goodflag = 0;
        }
        
      } else if ( !strcmp( szTitle, "WPT_IDENT" ) ){ 
        _snprintf_s( szFormat, _countof(szFormat), _TRUNCATE, "%%-%ds", nWidth );
        _snprintf_s(icaobuf, _countof(icaobuf), _TRUNCATE, szFormat, DBFReadStringAttribute( hDBF, iRecord, i ) );
        STrimRight( icaobuf );
        
        len = strlen( icaobuf );
        
       
        //if ( strlen(icaobuf) == 4 && icaobuf[0] == 'K' ){
        //  /* do not use US Fixes in this DB since the FAA from LoadFixes is better */
        //  goodflag = 0;
        //}
        
        strncpy_s(Fixes[fixcount].fixname, 7, icaobuf, _TRUNCATE);
      } else if ( !strcmp( szTitle, "WGS_DLAT" ) ){
        _snprintf_s( szFormat, _countof(szFormat), _TRUNCATE, "%%-%ds", nWidth );
        _snprintf_s( latbuf, _countof(latbuf), _TRUNCATE, szFormat, 
          DBFReadStringAttribute( hDBF, iRecord, i ) );
        Fixes[fixcount].lat = (float) atof( latbuf );
      } else if ( !strcmp( szTitle, "WGS_DLONG" ) ){
        _snprintf_s( szFormat, _countof(szFormat), _TRUNCATE, "%%-%ds", nWidth );
        _snprintf_s( longbuf, _countof(longbuf), _TRUNCATE, szFormat, 
          DBFReadStringAttribute( hDBF, iRecord, i ) );
        Fixes[fixcount].lng = (float) atof( longbuf );
      }
      
    }
    
    
    if (goodflag){ 
      fixcount++;
    }
  }
  
  
  DBFClose( hDBF );
  LeaveCriticalSection(&id->m_csDBFSync);
 
	*fixcountp = fixcount;
  return( Fixes );
}

void GetPreferredFixes( struct InstanceData* id )
{
	char *str;
	static char **result;
	FIX key;
	FIX *curfix;
	FIX *FixesPreferred = NULL;
	int i, prefcount, fixcountP;
	const DWORD cdwThreadId = GetCurrentThreadId();

	static char *PrefFixNames[] = {"CAMRN", "LENDY", "VALRE", "PROUD", "RBV", "HOLEY",
		"LIZZI", "SINCA", "RMG", "DIRTY", "SHANE", "TEDDY", "BENKY", "WYNDE", "HALIE",
		"BIRLE", "GROVE", "BYP", "CAPTI", "WEIRS", "JUMBO", "CADES", "SICUM", "SPS", "UKW", 
		"JEN", "CCC", "IGN", "SCUPP", "PVD", "GDM", "ORW", "WOONS", "VRB", "PBI", "CURSO", 
		"CARNU", "MILSY", "WORPP", "DEEDS", "FOWEE", "CYY", "JUNUR", "DARTS", "SXC", "SINCA", 
		"RMG", "FLCON", "LGC", "POLAR", "GEMNI", "MIZAR", "SPICA", "ROUTER", "PCMAN", "JORAY",
		"SEATE", "JUULI", "BARRY", "SSCOT", "BAGGS", "GATTZ", "ROBEY", "ACKME", "PLEBS", "CHECK",
		"CQY", "PENNS", "SLT", "HELON", "JENNO", "BOTON", 0};

	i = 0;
	while ( (str = PrefFixNames[i++] ) );
	fixcountP = i;
	FixesPreferred = calloc(sizeof(FIX), fixcountP );

	i = 0;
	prefcount = 0;
	while ( (str = PrefFixNames[i++] ) ){
		// find in low fixes

		strncpy_s( key.fixname, 7, str, _TRUNCATE);
		StrUpper(key.fixname);

		if ( (result = (char **) bsearch( 
			(char *) &key, (char *) g_pFixesLow, g_nFixCountLow, sizeof( FIX ), (int (*)(const void*, const void*)) compareFix )) || 
			(result = (char **) bsearch( (char *) &key, (char *) g_pFixesHigh, g_nFixCountHigh,
			sizeof( FIX ), (int (*)(const void*, const void*)) compareFix ) ) ||
			(result = (char **) bsearch( (char *) &key, (char *) g_pFixesTerm, g_nFixCountTerm,
			sizeof( FIX ), (int (*)(const void*, const void*)) compareFix )) )
		{

			curfix = (FIX *) result;

			// may be multiple matching fixes, so chose closest one to given lng , lat
			// add fix to preferred list
			if ( prefcount < fixcountP ){
				strcpy_s( FixesPreferred[ prefcount ].fixname, 7, curfix->fixname );
				FixesPreferred[ prefcount ].lng = curfix->lng;
				FixesPreferred[ prefcount ].lat = curfix->lat;
				prefcount++;
			} else {
				vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) bad fix count\n", cdwThreadId);
			}
		} else {
			vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) cound not find fix '%s'\n", cdwThreadId, str );
		}
	}
	g_pFixesPref = FixesPreferred;
	g_nFixCountPref = prefcount;
}



void LoadInternationalFixes(struct InstanceData* id)

{
	char *filename;
	char dbfilename[256];
	const DWORD cdwThreadId = GetCurrentThreadId();
	int bAirAsia = HavePermission(id, "AIRASIA");

	if( HavePermission(id, "AIRASIA") )
	{
		LoadAirAsiaFixFile(id, "aafh.txt", &g_pFixesHigh, &g_nFixCountHigh);
		LoadAirAsiaFixFile(id, "aaft.txt", &g_pFixesTerm, &g_nFixCountTerm);
		LoadAirAsiaFixFile(id, "aafl.txt", &g_pFixesLow, &g_nFixCountLow);
	}
	else
	{
		// if all fixes are used then:
		//TotFixes = 23431 /* H */ + 46699 /* T */ + 24334 /* L */;

		// we only use terminal fixes from US and Canada, so we only need 80222
		//TotFixes = 80222;

		filename = "WPTH_P.dbf";
		sprintf_s(dbfilename, _countof(dbfilename), "%s%s", id->m_strMapDir, filename);
		g_pFixesHigh = LoadFixFile( id, dbfilename, &g_nFixCountHigh );

		filename = "WPTT_P.dbf";
		sprintf_s(dbfilename, _countof(dbfilename), "%s%s", id->m_strMapDir, filename);
		g_pFixesTerm = LoadFixFile( id, dbfilename, &g_nFixCountTerm );

		filename = "WPTL_P.dbf";
		sprintf_s(dbfilename, _countof(dbfilename), "%s%s", id->m_strMapDir, filename);
		g_pFixesLow = LoadFixFile( id, dbfilename , &g_nFixCountLow );
	}

	qsort( (void *)g_pFixesHigh, (size_t) g_nFixCountHigh, sizeof(FIX), compareFix );
	if( IS_D_CONTROL( "HIGHFIXES" ) ){
		int i;
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) HIGH FIXES\n", cdwThreadId );
		for (i = 0; i < g_nFixCountHigh; i++){
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) FIX[%d]='%s',lat=%g lng=%g\n",
				cdwThreadId, i, g_pFixesHigh[i].fixname, g_pFixesHigh[i].lat, g_pFixesHigh[i].lng );
		}
	}
	qsort( (void *)g_pFixesLow, (size_t) g_nFixCountLow, sizeof(FIX), compareFix );
	if( IS_D_CONTROL( "LOWFIXES" ) ){
		int i;
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) LOW FIXES\n", cdwThreadId );
		for (i = 0; i < g_nFixCountLow; i++){
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) FIX[%d]='%s',lat=%g lng=%g\n",
				cdwThreadId, i, g_pFixesLow[i].fixname, g_pFixesLow[i].lat, g_pFixesLow[i].lng );
		}
	}
	qsort( (void *)g_pFixesTerm, (size_t) g_nFixCountTerm, sizeof(FIX), compareFix );
	if( IS_D_CONTROL( "TERMFIXES" ) ){
		int i;
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) TERM FIXES\n", cdwThreadId );
		for (i = 0; i < g_nFixCountTerm; i++){
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) FIX[%d]='%s',lat=%g lng=%g\n",
				cdwThreadId, i, g_pFixesTerm[i].fixname, g_pFixesTerm[i].lat, g_pFixesTerm[i].lng );
		}
	}

	// Second Fix array for all fixes, used in Flight Plan display
	g_nFixCount =  g_nFixCountHigh + g_nFixCountTerm + g_nFixCountLow;
	g_pFixes = (FIX*) calloc(sizeof(FIX), g_nFixCount );
	memcpy(g_pFixes , g_pFixesHigh, sizeof(FIX) * g_nFixCountHigh);
	memcpy((FIX*)g_pFixes + g_nFixCountHigh, g_pFixesLow, sizeof(FIX) * g_nFixCountLow);
	memcpy((FIX*)g_pFixes + (g_nFixCountHigh + g_nFixCountLow), g_pFixesTerm, sizeof(FIX) * g_nFixCountTerm);
	qsort( (void *)g_pFixes, (size_t) g_nFixCount, sizeof(FIX), compareFix );
}

void LoadFixes( struct InstanceData* id )
{
	int i;
	const DWORD cdwThreadId = GetCurrentThreadId();
	HANDLE hMutex = 0;

	// Create a System wide Mutex, with ownership to serialize loading (everybody instance shares Fixes)
	hMutex = CreateMutex(NULL, TRUE, "WEBTRACKER_FIX_MUTEX");
	// Check if Mutex was created by other process/thread and exit
	if(!hMutex || ERROR_ALREADY_EXISTS == GetLastError())
		return;

	// For AirAsia Load PANASONIC Provided Fixes
	if(HavePermission(id, "AIRASIA"))
	{
		if(!GetLocalFile(id, id->m_strMapDir, "aafh.txt", id->m_strHttpMapPath, SM_HTTPS))
		{
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to open local WPTH_P.dbf", cdwThreadId);	
		}
		if(!GetLocalFile(id, id->m_strMapDir, "aafl.txt", id->m_strHttpMapPath, SM_HTTPS))
		{
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to open local WPTL_P.dbf", cdwThreadId);	
		}
		if(!GetLocalFile(id, id->m_strMapDir, "aaft.txt", id->m_strHttpMapPath, SM_HTTPS))
		{
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to open local WPTT_P.dbf", cdwThreadId);	
		}
	}
	else
	{
		if(!GetLocalFile(id, id->m_strMapDir, "WPTH_P.dbf", id->m_strHttpMapPath, SM_HTTPS))
		{
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to open local WPTH_P.dbf", cdwThreadId);	
		}
		if(!GetLocalFile(id, id->m_strMapDir, "WPTL_P.dbf", id->m_strHttpMapPath, SM_HTTPS))
		{
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to open local WPTL_P.dbf", cdwThreadId);	
		}
		if(!GetLocalFile(id, id->m_strMapDir, "WPTT_P.dbf", id->m_strHttpMapPath, SM_HTTPS))
		{
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to open local WPTT_P.dbf", cdwThreadId);	
		}
	}
	
	LoadInternationalFixes(id);
	GetPreferredFixes(id);

	if ( 5 == id->m_nDebugControl || (id->m_nDebugControl && IsDebugNameDefined(id, "FIXES" )) ){
		for (i = 0; i < g_nFixCount; i++ ){
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) %s %1.4f %1.4f\n", 
				cdwThreadId, g_pFixes[i].fixname, g_pFixes[i].lng, g_pFixes[i].lat );
		}
	}

	g_bFixesLoaded = TRUE;
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
}

int LoadInternationalArpts( struct InstanceData* id )

{
  DBFHandle	hDBF;
  ARPT *Arpt;
  int		i, iRecord, reccount, fieldcount, goodcount, goodflag, len;
  char	*pszFilename = NULL;
  int		nWidth, nDecimals;
  int		bHeader = 0;
  int		bRaw = 0;
  int		bMultiLine = 0;
  char	szTitle[12];
  char *filename;
  char dbfilename[MAX_PATH_SZ];
  char	szFormat[32];
  char namebuf[39];
  char icaobuf[12];
  char latbuf[11];
  char longbuf[11];
  char *faa_arpt;
  int GoodAirports;
  VO *LoadAirports( struct InstanceData* id );
  VO *AirportsVO;
  int misscount = 0;
  int USAirports;
  ARPT *Arpts;
  int ArptCount;
  const DWORD cdwThreadId = GetCurrentThreadId();
  
  if(!id->m_bArptLoaded){
    //Get a local copy of ARPT.dbf and then call LoadInternationalAirport
    if(!GetLocalFile(id, id->m_strMapDir, "ARPT.dbf", id->m_strHttpMapPath, SM_HTTPS))
    {
	  vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to open local ARPT.dbf.", cdwThreadId);	
    }
  			
    /* this file will not work with the shapefile since it has been sorted by ICAO code */
    filename = "ARPT.dbf";
    sprintf_s(dbfilename, MAX_PATH_SZ, "%s%s", id->m_strMapDir, filename);
 
	EnterCriticalSection(&id->m_csDBFSync);
    hDBF = DBFOpen(dbfilename, "rb" );
    if( hDBF == NULL )
    {
      vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not open %s\n", cdwThreadId, filename );
      return( 0 );
    }
  
  
    if( DBFGetFieldCount(hDBF) == 0 )
    {
      vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Bad file format for %s\n", cdwThreadId, filename );
    }
 
  
    for( i = 0; i < DBFGetFieldCount(hDBF); i++ )
    {
      DBFFieldType	eType;
      const char	 	*pszTypeName;
    
      eType = DBFGetFieldInfo( hDBF, i, szTitle, &nWidth, &nDecimals );
      if( eType == FTString )
        pszTypeName = "String";
      else if( eType == FTInteger )
        pszTypeName = "Integer";
      else if( eType == FTDouble )
        pszTypeName = "Double";
      else if( eType == FTInvalid )
        pszTypeName = "Invalid";
    
    //vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Field %d: Type=%s, Title=`%s', Width=%d, Decimals=%d\n",
    //  cdwThreadId, i, pszTypeName, szTitle, nWidth, nDecimals );
    }
  
    reccount = DBFGetRecordCount(hDBF);
    fieldcount = DBFGetFieldCount(hDBF);

    USAirports = 13519;
    // USAirports = 20000;
    GoodAirports = 5083 + USAirports; /* number of 4 letter icao codes worldwide, use only these, 
                                ignore helo airports for now
                                also add 310 for airports from FAA database not in International DB */

    Arpts = calloc(sizeof(ARPT), GoodAirports );
    goodcount = 0; // count of arpts actually used (4 letter icao only), skip helos and other crap
    for( iRecord = 0; iRecord < reccount; iRecord++ )
    {
      
      goodflag = 1;
      for( i = 0; i < fieldcount; i++ )
      {
        DBFFieldType	eType;
      
        eType = DBFGetFieldInfo( hDBF, i, szTitle, &nWidth, &nDecimals );
      
        if ( !strcmp( szTitle, "NAME" ) ){
          _snprintf_s( szFormat, _countof(szFormat), _TRUNCATE, "%%-%ds", nWidth );
          _snprintf_s( namebuf, _countof(namebuf), _TRUNCATE, szFormat, 
            DBFReadStringAttribute( hDBF, iRecord, i ) );
          STrimRight( namebuf );
       
          len = strlen( namebuf );

        // for now, we don't need airport name
        //str = malloc( len + 1 );
        //strcpy(str, namebuf);
        //str[len] = '\0';
        //Arpts[goodcount].name = str;
        
        } else if ( !strcmp( szTitle, "ICAO" ) ){ 
          _snprintf_s( szFormat, _countof(szFormat), _TRUNCATE, "%%-%ds", nWidth );
          _snprintf_s( icaobuf, _countof(icaobuf), _TRUNCATE, szFormat, 
            DBFReadStringAttribute( hDBF, iRecord, i ) );
			  	icaobuf[ sizeof(icaobuf) - 1 ] = '\0';
          STrimRight( icaobuf );
        
          len = strlen( icaobuf );
        
          if (len != 4 ){
            if ( len == 2 && !strcmp( icaobuf, "KZ" ) ){
              /* no icao designation for this airport, so use HOST_IDENT */
              eType = DBFGetFieldInfo( hDBF, 4, szTitle, &nWidth, &nDecimals );
              sprintf_s( szFormat, _countof(szFormat), "%%-%ds", nWidth );
              _snprintf_s(icaobuf, _countof(icaobuf), _TRUNCATE, szFormat, 
                DBFReadStringAttribute( hDBF, iRecord, 4 ) );
              STrimRight( icaobuf );
            } else {
              // bad airport, so skip it
              goodflag = 0;
              break;
            }
          }
          if ( strlen(icaobuf) == 4 && icaobuf[0] == 'K' ){
            /* do not use US airports in this DB since the FAA from LoadAirports is better */
            goodflag = 0;
          }
          strncpy_s(Arpts[goodcount].icao, 5, icaobuf, _TRUNCATE);
        } else if ( !strcmp( szTitle, "WGS_DLAT" ) ){
          sprintf_s( szFormat, _countof(szFormat), "%%-%ds", nWidth );
          _snprintf_s(latbuf, _countof(latbuf), _TRUNCATE, szFormat, 
            DBFReadStringAttribute( hDBF, iRecord, i ) );
          Arpts[goodcount].lat = (float) atof( latbuf );
        } else if ( !strcmp( szTitle, "WGS_DLONG" ) ){
          sprintf_s( szFormat, _countof(szFormat), "%%-%ds", nWidth );
          _snprintf_s(longbuf, _countof(longbuf), _TRUNCATE, szFormat, 
            DBFReadStringAttribute( hDBF, iRecord, i ) );
          Arpts[goodcount].lng = (float) atof( longbuf );
        }

      }
      //if ( !strcmp(icaobuf, "K3C8") ){
      //  vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "found it\n");
      //}

      if (goodflag){
        goodcount++;
        if ( goodcount >= GoodAirports ){
          vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) ran out of room last = '%s'\n", cdwThreadId, namebuf );
          goodcount--;
        }
      }
    }
  
    ArptCount = goodcount;

    DBFClose( hDBF );
	LeaveCriticalSection(&id->m_csDBFSync);
 
    qsort( (void *)Arpts, (size_t) ArptCount, sizeof(ARPT), compareArpt );
 

    /* load in FAA airports for US airports and find out how many are missing from other DB */
 
    AirportsVO = LoadAirports(id);  // load airports just once
  
    for (i = 0; AirportsVO && i < AirportsVO->count; i++ ){
      faa_arpt = VP(AirportsVO, i, "code", char );
      if ( !strcmp(faa_arpt, "ATL") ){
        vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "found it\n");
      }
      if ( !strcmp(faa_arpt, "ANC") ){
        /* special case, ignore since PANC is in database for same airport */
        continue;
      }

      /* need exact match test so that ATL does not get match with PATL, which is in alaska */
      if ( (Arpt = FindArptRow( id, faa_arpt, MATCH_EXACT )) <= 0 ){
        //vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Missing us airport '%s'\n", cdwThreadId, faa_arpt);
        misscount++;
        strncpy_s(Arpts[goodcount].icao, 5, faa_arpt, _TRUNCATE);
      
        // don't need name right now Arpts[goodcount].name = str;
      
        Arpts[goodcount].lng = (float) VV(AirportsVO, i, "lng", double );
        Arpts[goodcount].lat = (float) VV(AirportsVO, i, "lat", double );
      
        goodcount++;
        if ( goodcount >= GoodAirports ){
          vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) ran out of room last = '%s', i=%d, AirportsVO->count=%d\n", 
            cdwThreadId, namebuf, i, AirportsVO->count );
          break;
        }
      }
    }
  
    vo_free( AirportsVO );

    ArptCount = goodcount;
    qsort( (void *)Arpts, (size_t) ArptCount, sizeof(ARPT), compareArpt );

    //for (i = 0; i < ArptCount; i++ ){
    //  vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) %s %g %g\n", cdwThreadId, Arpts[i].icao, Arpts[i].lng, Arpts[i].lat );
    //}
	id->m_pArpts = Arpts;
	id->m_nArptCount = ArptCount;
	id->m_bArptLoaded = TRUE;
  }

  return( 0 );
}


int LoadInternationalArptsDump(struct InstanceData* id)

{
  DBFHandle	hDBF;
  int		i, iRecord, *panWidth;
  char	*pszFilename = NULL;
  int		nWidth, nDecimals;
  int		bHeader = 0;
  int		bRaw = 0;
  int		bMultiLine = 0;
  char	szTitle[12];
  char *filename;
  char	szFormat[32];
  const DWORD cdwThreadId = GetCurrentThreadId();
  
  filename = "data/maps/ARPT_ICAO_SORT.dbf";
  EnterCriticalSection(&id->m_csDBFSync);
  hDBF = DBFOpen(filename, "rb" );
  if( hDBF == NULL )
  {
    vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not open %s\n", cdwThreadId, filename );
    return( 0 );
  }
  
  
  if( DBFGetFieldCount(hDBF) == 0 )
  {
    vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Bad file format for %s\n", cdwThreadId, filename );
  }
  panWidth = (int *) malloc( DBFGetFieldCount( hDBF ) * sizeof(int) );
  /* -------------------------------------------------------------------- */
  /*	Dump header definitions.					*/
  /* -------------------------------------------------------------------- */
  
  for( i = 0; i < DBFGetFieldCount(hDBF); i++ )
  {
    DBFFieldType	eType;
    const char	 	*pszTypeName;
    
    eType = DBFGetFieldInfo( hDBF, i, szTitle, &nWidth, &nDecimals );
    if( eType == FTString )
      pszTypeName = "String";
    else if( eType == FTInteger )
      pszTypeName = "Integer";
    else if( eType == FTDouble )
      pszTypeName = "Double";
    else if( eType == FTInvalid )
      pszTypeName = "Invalid";
    
    printf( "Field %d: Type=%s, Title=`%s', Width=%d, Decimals=%d\n",
      i, pszTypeName, szTitle, nWidth, nDecimals );
  }
  
  
  for( iRecord = 0; iRecord < DBFGetRecordCount(hDBF); iRecord++ )
  {
      
    for( i = 0; i < DBFGetFieldCount(hDBF); i++ )
    {
      DBFFieldType	eType;
      
      eType = DBFGetFieldInfo( hDBF, i, szTitle, &nWidth, &nDecimals );
      
      if( bMultiLine )
      {
        printf( "%s: ", szTitle );
      }
      
      /* -------------------------------------------------------------------- */
      /*      Print the record according to the type and formatting           */
      /*      information implicit in the DBF field description.              */
      /* -------------------------------------------------------------------- */
      if( !bRaw )
      {
        if( DBFIsAttributeNULL( hDBF, iRecord, i ) )
        {
          if( eType == FTString )
            sprintf_s( szFormat, _countof(szFormat), "%%-%ds", nWidth );
          else
            sprintf_s( szFormat, _countof(szFormat), "%%%ds", nWidth );
          
          printf( szFormat, "(NULL)" );
        }
        else
        {
          switch( eType )
          {
          case FTString:
            sprintf_s( szFormat, _countof(szFormat), "%%-%ds", nWidth );
            printf( szFormat, 
              DBFReadStringAttribute( hDBF, iRecord, i ) );
            break;
            
          case FTInteger:
            sprintf_s( szFormat, _countof(szFormat), "%%%dd", nWidth );
            printf( szFormat, 
              DBFReadIntegerAttribute( hDBF, iRecord, i ) );
            break;
            
          case FTDouble:
            sprintf_s( szFormat, _countof(szFormat), "%%%d.%dlf", nWidth, nDecimals );
            printf( szFormat, 
              DBFReadDoubleAttribute( hDBF, iRecord, i ) );
            break;
            
          default:
            break;
          }
        }
      }
      
      /* -------------------------------------------------------------------- */
      /*      Just dump in raw form (as formatted in the file).               */
      /* -------------------------------------------------------------------- */
      else
      {
        sprintf_s( szFormat, _countof(szFormat), "%%-%ds", nWidth );
        printf( szFormat, 
          DBFReadStringAttribute( hDBF, iRecord, i ) );
      }
      
      /* -------------------------------------------------------------------- */
      /*      Write out any extra spaces required to pad out the field        */
      /*      width.                                                          */
      /* -------------------------------------------------------------------- */
      if( !bMultiLine )
      {
        sprintf_s( szFormat, _countof(szFormat), "%%%ds", panWidth[i] - nWidth + 1 );
        printf( szFormat, "" );
      }
      
      if( bMultiLine )
        printf( "\n" );
      
      fflush( stdout );
    }
    printf( "\n" );
  }
  
  
  DBFClose( hDBF );
  LeaveCriticalSection(&id->m_csDBFSync);
  
  return( 0 );
}

VO* LoadDBFFile(struct InstanceData* id, const char* file)
{
	VO* DBFVO = NULL;
	DBFHandle	hDBF;
	DBFFieldType	eType;
	int i, j, c = 0, r = 0;
	int	nWidth, nDecimals;
	char szTitle[12];
	const DWORD cdwThreadId = GetCurrentThreadId();
	
	EnterCriticalSection(&id->m_csDBFSync);
	hDBF = DBFOpen(file, "rb");

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Loading %s\n", cdwThreadId, file);
	if (hDBF == NULL) {
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not open %s\n", cdwThreadId, file);
	} else {
		c = DBFGetFieldCount(hDBF);
		r = DBFGetRecordCount(hDBF);
		if (r > 0) {

			// Create VO, Add Fields from DBF
			DBFVO = (VO *)vo_create(0, NULL);
			vo_set(DBFVO, V_NAME, "A", NULL);
			for (i = 0; i < c; i++) {			
				const char	 	*pszTypeName;

				eType = DBFGetFieldInfo(hDBF, i, szTitle, &nWidth, &nDecimals);
				if (eType == FTString) {
					pszTypeName = "String";				
					VOPropAdd(DBFVO, szTitle, NTBSTRINGBIND, nWidth + 1, VO_NO_ROW);
				} else if (eType == FTInteger) {
					pszTypeName = "Integer";
					VOPropAdd(DBFVO, szTitle, INTBIND, -1, VO_NO_ROW);
				} else if (eType == FTDouble) {
					pszTypeName = "Double";
					VOPropAdd(DBFVO, szTitle, FLT8BIND, -1, VO_NO_ROW);
				} else if (eType == FTInvalid) {
					pszTypeName = "Invalid";
					vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Field %d: Type=%s, Title=`%s', Width=%d, Decimals=%d\n",
						cdwThreadId, i, pszTypeName, szTitle, nWidth, nDecimals);
					continue;
				}
				//if (!strcmp("STATUS", szTitle))
				//	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) FILE:%s Field %d: Type=%s, Title=`%s', Width=%d, Decimals=%d\n", cdwThreadId, file, i, pszTypeName, szTitle, nWidth, nDecimals);
				D_CONTROL("DBF", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Field %d: Type=%s, Title=`%s', Width=%d, Decimals=%d\n", cdwThreadId, i, pszTypeName, szTitle, nWidth, nDecimals));
			}
		
			// Load DBF from Records
			vo_alloc_rows(DBFVO, r);
			for (i = 0; i < r; i++) {
				for (j = 0; j < c; j++) {
					int v = 0;
					if (DBFIsAttributeNULL(hDBF, i, j)) {
						vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) NULL Attribute for ROW=%d COL=%d'\n\n", cdwThreadId, i, j);
						continue;
					} else {
						const char* s = NULL;
						eType = DBFGetFieldInfo(hDBF, j, szTitle, &nWidth, &nDecimals);

						switch (eType) {
						case FTString:
							s = DBFReadStringAttribute(hDBF, i, j);
							strcpy_s(VP(DBFVO, i, szTitle, char), nWidth + 1, s);
							break;

						case FTInteger:
							v = DBFReadIntegerAttribute(hDBF, i, j);
							//if (!strcmp("STATUS", szTitle))
							//	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Record %d: Status=%d\n", cdwThreadId, i, v);
							VV(DBFVO, i, szTitle, int) = v;
							break;

						case FTDouble:
							VV(DBFVO, i, szTitle, double) = DBFReadDoubleAttribute(hDBF, i, j);
							break;

						default:
							break;
						}
					}
				}
			}
			// Log Completed VO
			D_CONTROL("DBF", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) %s='%s'\n\n", cdwThreadId, vo_printstr(DBFVO)));
		} 

		DBFClose(hDBF);		
	}
	LeaveCriticalSection(&id->m_csDBFSync);
	D_CONTROL("DBF", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) %s Load Complete\n", cdwThreadId, file));
	return (DBFVO);
}