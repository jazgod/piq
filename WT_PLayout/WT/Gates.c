#include "InstanceData.h"
#include "feedstruct.h"
#include "vo.h"
#include "vo_db.h"
#include "curl.h"

#define _USE_MATH_DEFINES
#include <math.h>

#define ARPT_CODE_SIZE 5
#define BUFFER_SZ 100
#define FILENAME_SZ 128
#define FILEPATH_SZ 512
#define GATE_SIZE 6
#define TERM_SIZE 6
#define ID_SIZE 20

typedef struct POOL{
	char gate[GATE_SIZE];
	struct POOL* next;
} gates;

extern int IsDebugNameDefined(struct InstanceData* id, const char* strDebugName);
#define D_CONTROL(vcolname, exp) {if ( 5 == id->m_nDebugControl || ( id->m_nDebugControl && IsDebugNameDefined(id, vcolname) ) ) {exp;}}

extern int GetLocalFile( struct InstanceData* id, const char* localpath, const char* filename, const char* remotepath, int eMode);
extern char *StrTok(char **m,char *s,char c);
extern int               vo_log_info( va_alist );
extern int               vo_log_error( va_alist );
extern int               vo_log_warning( va_alist );
extern void DrawGLTextXY(struct InstanceData* id, double x, double y, char *mystr, GLfloat *color, GLuint fontbase, int bRotate);
extern GLfloat black[3];

VO* GetGateVO(struct InstanceData* id)
{
	static VO* GATEVO = NULL;
	if ( !GATEVO ){	
		GATEVO = (VO *) vo_create( 0, NULL );
		if( GATEVO ) {
			vo_set( GATEVO, V_NAME, "GATEVO", NULL );
			VOPropAdd( GATEVO, "id", NTBSTRINGBIND, ID_SIZE, VO_NO_ROW );
			VOPropAdd( GATEVO, "gate", NTBSTRINGBIND, GATE_SIZE, VO_NO_ROW );
			VOPropAdd( GATEVO, "lbl", INTBIND, -1, VO_NO_ROW );
			VOPropAdd( GATEVO, "lat", FLT8BIND, -1, VO_NO_ROW );
			VOPropAdd( GATEVO, "lng", FLT8BIND, -1, VO_NO_ROW );
			VOPropAdd( GATEVO, "ang", FLT8BIND, -1, VO_NO_ROW );
			// Lat and Long Gate Label Positions, offset from aircraft
			VOPropAdd( GATEVO, "latg", FLT8BIND, -1, VO_NO_ROW );
			VOPropAdd( GATEVO, "lngg", FLT8BIND, -1, VO_NO_ROW );
			
			vo_rm_rows( GATEVO, 0, GATEVO->count );

			vo_set( GATEVO, V_ORDER_COLS, "id", NULL, NULL);
		}
	}
	
	return (GATEVO);
}

VO* GetPoolVO(struct InstanceData* id)
{
	static VO* PoolVO = NULL;
	if ( !PoolVO ){	
		PoolVO = (VO *) vo_create( 0, NULL );
		if( PoolVO ) {
			vo_set( PoolVO, V_NAME, "PoolVO", NULL );
			VOPropAdd( PoolVO, "id", NTBSTRINGBIND, ID_SIZE, VO_NO_ROW );
			VOPropAdd( PoolVO, "data", VOIDBIND, -1, VO_NO_ROW );			
			vo_rm_rows( PoolVO, 0, PoolVO->count );
			vo_set( PoolVO, V_ORDER_COLS, "id", NULL, NULL);
		}
	}
	
	return (PoolVO);
}

void DrawGateLabels(struct InstanceData* id){
	VO* GateVO = NULL;
	int i = 0;
	float lat, lng;	
	char gate[GATE_SIZE] = {0};
	if(id->m_sCurLayout.m_nGATESLabel){
		GateVO = GetGateVO(id);

		for (i = 0; i < GateVO->count; i++ ){
			if(VV(GateVO, i, "lbl", int)){
				lat = VV(GateVO, i, "latg", float);
				lng = VV(GateVO, i, "lngg", float);			
				
				strncpy_s( gate, GATE_SIZE, VP(GateVO, i, "gate", char), _TRUNCATE );
				DrawGLTextXY(id, lng, lat, gate, id->m_sCurLayout.m_glfGATESColor, id->m_arFonts[id->m_sCurLayout.m_nGATESFontIndex].m_glListBase, TRUE);
			}
		}
	}
}

VO* GetGateLabelList(struct InstanceData* id, const char* arpt){
	VO* GateVO = NULL;
	VO* LabelVO = NULL;
	char name[ID_SIZE] = {0};
	int lbl = 0;
	int i = 0;
	// Create Label VO
	LabelVO = (VO *) vo_create( 0, NULL );
	if( LabelVO ) {
		vo_set( LabelVO, V_NAME, "LABELVO", NULL );
		VOPropAdd( LabelVO, "gate", NTBSTRINGBIND, GATE_SIZE, VO_NO_ROW );
		
		vo_rm_rows( LabelVO, 0, LabelVO->count );

		vo_set( LabelVO, V_ORDER_COLS, "gate", NULL, NULL);
	}
	
	GateVO = GetGateVO(id);

	for (i = 0; i < GateVO->count; i++ ){
		lbl = VV(GateVO, i, "lbl", int);
		strcpy_s(name, ID_SIZE, VP(GateVO, i, "id", char));
		if( lbl && !strncmp(arpt, name, 4) ){
			vo_alloc_rows(LabelVO, 1);
			strcpy_s( VP(LabelVO, LabelVO->count - 1, "gate", char), GATE_SIZE, VP(GateVO, i, "gate", char) );
		}
	}

	return (LabelVO);	
}

int GetAirportGateData(struct InstanceData* id, const char* arpt, const char* term, const char* gate, const char* flightid, float* lat, float* lng, float* ang){
	
	VO* PoolVO = NULL;
	VO* GateVO = NULL;
	char name[ID_SIZE] = {0};
	struct row_index *row_index;
	int row = -1;
	int nSuccess = FALSE;
	int nHaveTerm = FALSE;
	const DWORD cdwThreadId = GetCurrentThreadId();

	// Check Pool First
	PoolVO = GetPoolVO(id);
	if ( strlen( term ) && strcmp(term, "null") ){
		nHaveTerm = TRUE;
		sprintf_s(name, ID_SIZE, "%s/%s/%s", arpt, term, gate);
	}else{
		sprintf_s(name, ID_SIZE, "%s/null/%s", arpt, gate);
	}

	if ( (row_index = vo_search(PoolVO, "id", (char *)name, NULL )) ){
		struct POOL* pNext = NULL;
		struct POOL* pCur = NULL;
		int row = row_index->rownum;
		
		// Pull queue from VO
		pCur = (struct POOL*)VV(PoolVO, row, "data", long);
		pNext = pCur->next;

		// Use actual gate name
		gate = pCur->gate;
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Using POOL Gate : ARPT<%s>, FLTNUM<%s>, TERM<%s>, GATE<%s>.\n",
			cdwThreadId, arpt, flightid, term, gate );

		// Move Cur to end of queue and re-establish new head in VO
		if(pNext){
			VV(PoolVO, row, "data", void*) = pNext;
			while(pNext->next){
				pNext = pNext->next;
			}
			pNext->next = pCur;
			pCur->next = NULL;
		}
	}

	// Pull up actual Gate data from either requested Gate or Found Pool Gate Name
	GateVO = GetGateVO(id);

	if ( strlen( term ) && strcmp(term, "null") ){
		sprintf_s(name, ID_SIZE, "%s/%s/%s", arpt, term, gate);
	}else{
		sprintf_s(name, ID_SIZE, "%s/null/%s", arpt, gate);
	}

	if ( (row_index = vo_search(GateVO, "id", (char *) name, NULL )) ){
		row = row_index->rownum;
		*lat = VV(GateVO, row, "lat", float);
		*lng = VV(GateVO, row, "lng", float);
		*ang = VV(GateVO, row, "ang", float);
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Using Gate Data: ARPT<%s>, FLTNUM<%s>, TERM<%s>, GATE<%s>, LAT<%f>, LNG<%f>, ANG<%f>.\n",
			cdwThreadId, arpt, flightid, term, gate, *lat, *lng, *ang );
		nSuccess = TRUE;
	}

	// If we have a terminal value but did not find the gate, 
	// we try once more without the terminal. This is becuase 
	// sometimes we get gates with and without terminal values
	// and they are usually the same gate.
	if(!nSuccess && nHaveTerm){
		vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Gate Data Not Found, re-try with out term: ARPT<%s>, FLTNUM<%s>, TERM<%s>, GATE<%s>.\n",
			cdwThreadId, arpt, flightid, term, gate );
		nSuccess = GetAirportGateData(id, arpt, "null", gate, flightid, lat, lng, ang);
	}
	
	return nSuccess;
}

void LoadAirportGate(struct InstanceData* id, VO* GateVO, char* buffer)
{ 
	int fld = 0;
	char* ctx = NULL;
	char* tok = NULL;
	char name[ID_SIZE] = {0};
	char arpt[ARPT_CODE_SIZE] = {0};
	char term[TERM_SIZE] = {0};
	char gate[GATE_SIZE] = {0};
	char alt[GATE_SIZE] = {0};
	char pool[GATE_SIZE] = {0};
	float lat = 0.0f;
	float lng = 0.0f;
	float ang = 0.0f;
	float lat2 = 0.0f;
	float lng2 = 0.0f;
	float rad = 0.0f;
	const char delim = '\t';

	if(GateVO){	
		tok = StrTok(&ctx, buffer, delim );
		while(tok){
			switch(fld++){
			case 0: strcpy_s(arpt, ARPT_CODE_SIZE, tok);
				break;
			case 1: strcpy_s(term, TERM_SIZE, tok);
				break;
			case 2: strcpy_s(gate, GATE_SIZE, tok);
				break;
			case 3: lat = (float)atof(tok);
				break;
			case 4: lng = (float)atof(tok);
				break;
			case 5: ang = (float)atof(tok);
				break;
			case 6: strcpy_s(alt, GATE_SIZE, tok);
				break;
			case 7: strcpy_s(pool, GATE_SIZE, tok);
				break;
			}
			tok = StrTok(&ctx, NULL, delim );
		}
	
		rad = (float)(ang*(M_PI/180));
		lat2 = (float)(lat + 0.0001*sin(rad));
		lng2 = (float)(lng + 0.0001*cos(rad));

		// If we have a terminal value, gate name is terminal/gate
		if( strlen(term) ){
			vo_alloc_rows(GateVO, 1);
			sprintf_s(name, ID_SIZE, "%s/%s/%s", arpt, term, gate);
			strncpy_s( VP(GateVO, GateVO->count - 1, "id", char), ID_SIZE, name, _TRUNCATE );
			strncpy_s( VP(GateVO, GateVO->count - 1, "gate", char), GATE_SIZE, gate, _TRUNCATE );
			VV(GateVO, GateVO->count - 1, "lat", float) = lat;
			VV(GateVO, GateVO->count - 1, "lng", float) = lng;
			VV(GateVO, GateVO->count - 1, "ang", float) = ang;
			VV(GateVO, GateVO->count - 1, "lbl", int) = 0;
			VV(GateVO, GateVO->count - 1, "latg", float) = lat2;
			VV(GateVO, GateVO->count - 1, "lngg", float) = lng2;
		}
		// always Add values with just basic gate name
		vo_alloc_rows(GateVO, 1);
		sprintf_s(name, ID_SIZE, "%s/null/%s", arpt, gate);
		strncpy_s( VP(GateVO, GateVO->count - 1, "id", char), ID_SIZE, name, _TRUNCATE );
		strncpy_s( VP(GateVO, GateVO->count - 1, "gate", char), GATE_SIZE, gate, _TRUNCATE );
		VV(GateVO, GateVO->count - 1, "lat", float) = lat;
		VV(GateVO, GateVO->count - 1, "lng", float) = lng;
		VV(GateVO, GateVO->count - 1, "ang", float) = ang;
		VV(GateVO, GateVO->count - 1, "lbl", int) = 1;
		VV(GateVO, GateVO->count - 1, "latg", float) = lat2;
		VV(GateVO, GateVO->count - 1, "lngg", float) = lng2;
	
		// If we have an alternate name, add new rows with this gate identifier
		if( strlen(alt) ){
			// If we have a terminal value, gate name is terminal/alt
			if( strlen(term) ){
				vo_alloc_rows(GateVO, 1);
				sprintf_s(name, ID_SIZE, "%s/%s/%s", arpt, term, alt);
				strncpy_s( VP(GateVO, GateVO->count - 1, "id", char), ID_SIZE, name, _TRUNCATE );
				strncpy_s( VP(GateVO, GateVO->count - 1, "gate", char), GATE_SIZE, alt, _TRUNCATE );
				VV(GateVO, GateVO->count - 1, "lat", float) = lat;
				VV(GateVO, GateVO->count - 1, "lng", float) = lng;
				VV(GateVO, GateVO->count - 1, "ang", float) = ang;
				VV(GateVO, GateVO->count - 1, "lbl", int) = 0;
				VV(GateVO, GateVO->count - 1, "latg", float) = lat2;
				VV(GateVO, GateVO->count - 1, "lngg", float) = lng2;
			}

			// always Add values with just basic gate alt name
			vo_alloc_rows(GateVO, 1);
			sprintf_s(name, ID_SIZE, "%s/null/%s", arpt, alt);
			strncpy_s( VP(GateVO, GateVO->count - 1, "id", char), ID_SIZE, name, _TRUNCATE );
			strncpy_s( VP(GateVO, GateVO->count - 1, "gate", char), GATE_SIZE, alt, _TRUNCATE );
			VV(GateVO, GateVO->count - 1, "lat", float) = lat;
			VV(GateVO, GateVO->count - 1, "lng", float) = lng;
			VV(GateVO, GateVO->count - 1, "ang", float) = ang;
			VV(GateVO, GateVO->count - 1, "lbl", int) = 0;
			VV(GateVO, GateVO->count - 1, "latg", float) = lat2;
			VV(GateVO, GateVO->count - 1, "lngg", float) = lng2;
		}

		if ( strlen( pool ) ){
			struct row_index *row_index;
			VO* PoolVO = GetPoolVO(id);
			if( strlen(term) ){
				sprintf_s(name, ID_SIZE, "%s/%s/%s", arpt, term, pool);
			}else{
				sprintf_s(name, ID_SIZE, "%s/null/%s", arpt, pool);
			}
			if ( (row_index = vo_search(PoolVO, "id", (char *) name, NULL )) ){
				// Add Name to Pool Queue
				struct POOL* pNext = NULL;
				struct POOL* pNew = (struct POOL*)calloc(1, sizeof(struct POOL) );
				int row = row_index->rownum;
				strcpy_s( pNew->gate, GATE_SIZE, gate );
				pNext = (struct POOL*)VV(PoolVO, row, "data", void*);
				while(pNext->next){
					pNext = pNext->next;
				}
				pNext->next = pNew;

			}else{
				// Create Pool Queue and add to Pool VO
				struct POOL* pNew = (struct POOL*)calloc(1, sizeof(struct POOL) );
				vo_alloc_row(PoolVO, 1);
				strncpy_s( VP(PoolVO, PoolVO->count - 1, "id", char), ID_SIZE, name, _TRUNCATE );
				strcpy_s( pNew->gate, GATE_SIZE, gate );
				VV(PoolVO, PoolVO->count - 1, "data", void*) = pNew;
			}
		}
	}
}

void LoadAirportGates(struct InstanceData* id)
{
	const DWORD cdwThreadId = GetCurrentThreadId();
	char filename[FILENAME_SZ] = {0};
	char arpt[ARPT_CODE_SIZE] = {0};
	struct row_index *row_index;
	static VO* ARPTSVO = NULL;

	// Initial Arpt List VO
	if(!ARPTSVO){
		ARPTSVO = (VO *) vo_create( 0, NULL );
		vo_set( ARPTSVO, V_NAME, "ARPTSVO", NULL );
		VOPropAdd( ARPTSVO, "arpt", NTBSTRINGBIND, ARPT_CODE_SIZE, VO_NO_ROW );
		vo_rm_rows( ARPTSVO, 0, ARPTSVO->count );

		vo_set( ARPTSVO, V_ORDER_COLS, "arpt", NULL, NULL);
	}

	// Create airport icao
	strcat_s(arpt, ARPT_CODE_SIZE, "K" );
	strncat_s(arpt, ARPT_CODE_SIZE, id->m_strAsdexArpt, 3 );
	StrUpper(arpt);

	// Check if we have already loaded this arpt
	if ( !(row_index = vo_search(ARPTSVO, "arpt", (char *) arpt, NULL )) ){
		sprintf_s(filename, FILENAME_SZ, "%sGates.dat", arpt);
		if( !GetLocalFile(id, id->m_strMapDir, filename, id->m_strHttpMapPath, SM_HTTPS)  ){
			vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to find gate data file(%s).\n", cdwThreadId, filename);
		}else{
			FILE* f = NULL;
			char filepath[FILEPATH_SZ] = {0};
			char buffer[BUFFER_SZ] = {0};
			sprintf_s(filepath, FILEPATH_SZ, "%s%s", id->m_strMapDir, filename);
			if( !fopen_s(&f, filepath, "r") ){
				VO* GateVO = GetGateVO(id);
				if(GateVO){
					while( fgets(buffer, BUFFER_SZ, f) ){
						if( '#' != buffer[0] ){
							char* eol =  strchr(buffer, '\n');
							if(eol) *eol = 0;
							LoadAirportGate(id, GateVO, buffer);
						}
					}
				}
				vo_order(GateVO);
				D_CONTROL("GATES", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) GateVO='%s'\n", cdwThreadId, vo_printstr(GateVO) ) );
				fclose(f);
			}
		}
		vo_alloc_rows( ARPTSVO, 1);
		strncpy_s( VP(ARPTSVO, ARPTSVO->count - 1, "arpt", char), ARPT_CODE_SIZE, arpt , _TRUNCATE );
		vo_order(ARPTSVO);
	}	
}

