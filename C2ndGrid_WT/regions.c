#include "InstanceData.h"
#include "vo.h"
#include "vo_db.h"
#include "mgl.h"
#include "srfc.h"
#include "curl.h"
#include "SurfaceC.h"
#include "regions.h"
#include <process.h>

#define LINEBUF_SIZE 1024 
#define REGION_JSON_SIZE 250
#define ACTION_SIZE 8
#define ARRDEP_SIZE	2
#define UNIQUEID_SIZE 60

#ifdef PERFORMANCE_TESTING
#define TIME_TRACE_DBG(msg)\
	{\
	SYSTEMTIME t;\
	char s[LINEBUF_SIZE]={0};\
	GetSystemTime(&t);\
	sprintf_s(s, LINEBUF_SIZE, "%s (%d): (0x%X) TRACE-%s (%2d:%2d %2d.%4d)\n", __FILE__, __LINE__, GetCurrentThreadId(), msg, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);\
	OutputDebugString(s);\
	}
#else
#define TIME_TRACE_DBG(msg) 
#endif

const char* RegionJSON(struct InstanceData* id, VO *ROIDataVO, int i);

void ClearRegionData(struct InstanceData* id){
	VO* Empty = NULL;
	// Also clean up Temp JSON strings
	if ( id->m_pGRegionDelTemp )
	{
		free( id->m_pGRegionDelTemp );
		id->m_pGRegionDelTemp = NULL;
	}
	if ( id->m_pGRegionHistTemp )
	{
		free( id->m_pGRegionHistTemp );
		id->m_pGRegionHistTemp = NULL;
	}

	// Release Main Historical VO
	if ( id->m_pROIHistVO )
	{
		Empty = id->m_pROIHistVO;
		id->m_pROIHistVO = NULL;
		vo_rm_rows(Empty, 0, Empty->count);
		vo_free(Empty);
	}

	// Release Delete VO (records in the delete stage)
	if ( id->m_pROIDelVO )
	{
		Empty = id->m_pROIDelVO;
		id->m_pROIDelVO = NULL;
		vo_rm_rows(Empty, 0, Empty->count);
		vo_free(Empty);
	}

	// Release synchronized VO's
	EnterCriticalSection(&id->m_csROIHist);
	if ( id->m_pROINewActiveVO )
	{
		Empty = id->m_pROINewActiveVO;
		id->m_pROINewActiveVO = NULL;
		vo_rm_rows(Empty, 0, Empty->count);
		vo_free(Empty);
	}

	if ( id->m_pROINewHistVO )
	{
		Empty = id->m_pROINewHistVO;
		id->m_pROINewHistVO = NULL;
		vo_rm_rows(Empty, 0, Empty->count);
		vo_free(Empty);
	}
	LeaveCriticalSection(&id->m_csROIHist);
}

void* CreateRegionVO(struct InstanceData* id, const char* strVoName)
{
	VO* ROIVO = NULL;
	
	ROIVO = (VO *) vo_create( 0, NULL );
	if( ROIVO ) {
		vo_set( ROIVO, V_NAME, strVoName, NULL );
		VOPropAdd( ROIVO, "uniqueid", NTBSTRINGBIND, UNIQUEID_SIZE, VO_NO_ROW );
		VOPropAdd( ROIVO, "Region", NTBSTRINGBIND, REGION_SIZE, VO_NO_ROW );
		VOPropAdd( ROIVO, "flightid", NTBSTRINGBIND, FLTNUM_SIZE, VO_NO_ROW );
		VOPropAdd( ROIVO, "arpt", NTBSTRINGBIND, ARPT_CODE_SIZE, VO_NO_ROW );
		VOPropAdd( ROIVO, "starttime", INTBIND, -1, VO_NO_ROW );
		VOPropAdd( ROIVO, "endtime", INTBIND, -1, VO_NO_ROW );
		VOPropAdd( ROIVO, "lasttime", INTBIND, -1, VO_NO_ROW );  // track when last track point found in order to delete dead tracks
		VOPropAdd( ROIVO, "elapsed", INTBIND, -1, VO_NO_ROW );
		VOPropAdd( ROIVO, "arrdep", NTBSTRINGBIND, ARRDEP_SIZE, VO_NO_ROW );
		VOPropAdd( ROIVO, "Active", INTBIND, -1, VO_NO_ROW );
		VOPropAdd( ROIVO, "fix", NTBSTRINGBIND, DEP_FIX_SIZE, VO_NO_ROW );
		VOPropAdd( ROIVO, "action", NTBSTRINGBIND, ACTION_SIZE, VO_NO_ROW ); // Used to track when we insert/update or delete rows.
		VOPropAdd( ROIVO, "json", NTBSTRINGBIND, REGION_JSON_SIZE, VO_NO_ROW ); // Used to track when we insert/update or delete rows.
		vo_rm_rows( ROIVO, 0, ROIVO->count );

		vo_set( ROIVO, V_ORDER_COLS, "uniqueid", "flightid", "Region", NULL, NULL);
	}
	
	return (ROIVO);
}

// Function, Remove substring 
void ExtractRegionString(struct InstanceData* id, char** json, const char* extract)
{
	int n1 = 0;
	int n2 = 0;
	int n3 = 0;
	char* s = NULL;
	char e[REGION_JSON_SIZE] = {0};
	sprintf_s(e, REGION_JSON_SIZE, ",\n%s", extract);
	// Get Length of substring to extract
	if(extract)
		n1 = strlen(e);
	if(*json){
		// Get length of parent data for strcat_s
		n2 = strlen(*json);
		// Search for substring with starting comma (mid-string case)
		s = strstr(*json, e);
		if(!s){
			// Search for substring with ending comma (handle start of string case)
			sprintf_s(e, REGION_JSON_SIZE, "%s,\n", extract);
			s = strstr(*json, e);

			if(!s){
				// Search for substring with-out comma (only record case)
				s = strstr(*json, extract);
				n1 -= 2;
			}
		}

		if(s && n1 == n2){
			// free string
			free(*json);
			*json = NULL;
		}else if(s){
			// check length of string after substring
			n3 = strlen(s + n1);
			*s = 0;
			// Only do this if not at end of string
			if( n3 > 0 ){
				s += n1;
				strcat_s(*json, n2 + 1, s);
			}
		}
	}
}

void GetHistRegions(struct InstanceData* id, int bForceGet)
{
	struct MemoryStruct chunk;
	char tmpstr[ 25000 ];
	const DWORD cdwThreadId = GetCurrentThreadId();
	char strROIArpt[ 5 ] = {0};	
	VO* ROIActive = NULL;
	VO* ROIVO = NULL;
	time_t plottime = id->m_tPlotTime;	
	

	sprintf_s(strROIArpt, 5, "K%c%c%c", id->m_strAsdexArpt[0], id->m_strAsdexArpt[1], id->m_strAsdexArpt[2]);
	StrUpper( strROIArpt );

	if(bForceGet || !strlen(id->m_strROIArpt) || strcmp(id->m_strROIArpt, strROIArpt)){
		ClearRegionData(id);

		// Create Needed VO's if the don't exist
		if ( !id->m_pROIHistVO ){
			id->m_pROIHistVO = (VO*) CreateRegionVO(id, "ROIHistVO");
		}
		ROIVO = id->m_pROIHistVO;

		if ( !id->m_pROIDelVO ){
			id->m_pROIDelVO = (VO*) CreateRegionVO(id, "ROIDelVO");
		}		

		strcpy_s(id->m_strROIArpt, 5, strROIArpt);
		
		clear_chunk_memory(&chunk);

		if ( id->m_nDelaySecs ){
			int delay = id->m_nDelaySecs;
			sprintf_s(tmpstr, _countof(tmpstr), "Action=RegionHist&arpt=%s&secs=%d&delay=%d", 
				id->m_strROIArpt, id->m_nHistRegionSecs, delay );
		}else{
			sprintf_s(tmpstr, _countof(tmpstr), "Action=RegionHist&arpt=%s&secs=%d", 
				id->m_strROIArpt, id->m_nHistRegionSecs );
		}
	
		D_CONTROL("CURL", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlFormPostResults - id->m_strHttpDBservername <%s> tmpstr <%s>.\n", 
			GetCurrentThreadId(), id->m_strHttpDBservername[id->m_nDBServerIndex], tmpstr ));

		if(!GetCurlFormPostResults( id, &chunk, id->m_strHttpDBservername[id->m_nDBServerIndex], tmpstr, SM_HTTPS, TRUE )){
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) failed to load historial region data from <%s>.\n", 
				GetCurrentThreadId(), id->m_strROIArpt);
		}else{
			char linebuf[256] = {0};
			char* membuf = NULL;
			char* eolstr = NULL;
			int len = 0;

			if ( !plottime ){
				plottime = time(NULL) - id->m_nDelaySecs;
			}
	
			if ( chunk.memory && strcmp(chunk.memory, "EOD") ){
				membuf = chunk.memory;
				D_CONTROL("HIST_REGION_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Historical Region Data for %s.\n\n%s\n", 
					GetCurrentThreadId(), id->m_strROIArpt, membuf ) );
			}else{
				vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) No Historical Region Data for %s returned from server.\n", 
					GetCurrentThreadId(), id->m_strROIArpt );
			}

			while ( membuf && strlen(membuf) ){
				char tmptimebuf[64] = {0};
				char tmproi[100] = {0};
				char tmpend[100] = {0};
				char arptcode[ARPT_CODE_SIZE] = {0};
				char* roiname = NULL;
				char flightid[FLTNUM_SIZE] = {0};
				char dfix[DEP_FIX_SIZE] = {0};
				char arrdep[ARRDEP_SIZE] = {0};
				time_t starttime = 0;
				time_t endtime = 0;
				int elapsed = 0;
				int matches = 0;

				if (  (eolstr = strchr( membuf, '\n' ) ) ){
					len = eolstr - membuf;
				}

				if (len > 255){
					break;  // some problem with the data
				}
				strncpy_s( linebuf, _countof(linebuf), membuf, len );
				linebuf[len] = '\0';

				if ( !strcasecmp(linebuf, "EOD") ){
					break; // EOD
				}

				membuf = eolstr;
				membuf++; /* skip over eol char */

				matches = sscanf_s(linebuf, "%s\t%s\t%s\t%d\t%s\t%d\t%s\t%s",
					arptcode, _countof(arptcode), flightid, _countof(flightid), tmproi, _countof(tmproi), &starttime, 
 					tmpend, _countof(tmpend), &elapsed, arrdep, _countof(arrdep), dfix ,_countof(dfix) );
				D_CONTROL("ROI_HIST", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) %s\n", GetCurrentThreadId(), linebuf));

				if( 8 == matches ){
					char uniqueid[UNIQUEID_SIZE] = {0};
					// Data preparation/Conversion
					roiname = curl_unescape(tmproi, strlen(tmproi));
					sprintf_s( uniqueid, _countof( uniqueid ), "%s%s%d", flightid, roiname, starttime );
					CleanString(uniqueid);
					if(strcmp(tmpend, "NULL")) endtime = atoi(tmpend);
					if(!strcmp(dfix, "NULL")) memset(dfix, 0, sizeof(char) * DEP_FIX_SIZE);

					// Skip if startime is somehow prior to expected plottime
					if( id->m_nDelaySecs && ( starttime > plottime ) ){
						vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Skipped Historical ROI (starttime after plottime): %s, %s, %s, %d, %s\n", 
							cdwThreadId, roiname, 
							flightid, arptcode,
							starttime, tmpend);
						continue;
					}

					// For Replay, make ended times that are greater than current plottime set to null
					// and re-set elapsed time.
					if( id->m_nDelaySecs && ( endtime > plottime ) ){
						endtime = 0;
						elapsed = plottime - starttime;
					}
					
					// Check endtime here, if no endtime then we have an active record otherwise we have a finished record
					if(endtime){
						// Endtime exists so this is a finshed record, add to Historical VO
						char tmpstart[64] = {0};
						char tmpend[64] = {0};
						const char* json = NULL;
						vo_alloc_rows( ROIVO, 1);

						VV(ROIVO, ROIVO->count - 1, "endtime", int) = endtime;
						VV(ROIVO, ROIVO->count - 1, "lasttime", int) = endtime;
						strncpy_s( VP(ROIVO, ROIVO->count - 1, "uniqueid", char), UNIQUEID_SIZE, uniqueid , _TRUNCATE );
						strncpy_s( VP(ROIVO, ROIVO->count - 1, "Region", char), REGION_SIZE, roiname , _TRUNCATE );
						strncpy_s( VP(ROIVO, ROIVO->count - 1, "flightid", char), FLTNUM_SIZE, flightid, _TRUNCATE );
						strncpy_s( VP(ROIVO, ROIVO->count - 1, "arpt", char), ARPT_CODE_SIZE, arptcode, _TRUNCATE );
						strncpy_s( VP(ROIVO, ROIVO->count - 1, "action", char), ACTION_SIZE, "insert", _TRUNCATE );
						if(strlen(dfix))
							strncpy_s( VP(ROIVO, ROIVO->count - 1, "fix", char), DEP_FIX_SIZE, dfix, _TRUNCATE );
						VV(ROIVO, ROIVO->count - 1, "starttime", int) = starttime;
						VV(ROIVO, ROIVO->count - 1, "elapsed", int) = elapsed;
						strncpy_s( VP(ROIVO, ROIVO->count - 1, "arrdep", char), ARRDEP_SIZE, arrdep, _TRUNCATE );
						VOTimeFmt(tmpstart, VV(ROIVO, ROIVO->count - 1, "starttime", int), "%Y-%m-%d %H:%M:%S" );
						VOTimeFmt(tmpend, endtime, "%Y-%m-%d %H:%M:%S" );
						json = RegionJSON(id, ROIVO, ROIVO->count - 1);

						// Copy json into temp history string
						if(id->m_pGRegionHistTemp){
							id->m_pGRegionHistTemp = strcat_alloc(id->m_pGRegionHistTemp, ",\n");
							id->m_pGRegionHistTemp = strcat_alloc(id->m_pGRegionHistTemp, json);
						}else{
							id->m_pGRegionHistTemp = str_falloc(json);
						}
#ifdef TEST_EXTRACT
						{
							static int debug = 1;
							static const char* save1 = NULL;
							static const char* save2 = NULL;
							if(debug)
							{
								int n = strlen(id->m_pGRegionHistTemp) + 1;
								if(1 == debug){
									// Remove Only record in string
									ExtractRegionString(id, &id->m_pGRegionHistTemp, json);
									id->m_pGRegionHistTemp = str_falloc(json);
									save1 = json;
								}else if(2 == debug){
									// Remove End Record of two
									ExtractRegionString(id, &id->m_pGRegionHistTemp, json);
									strcat_s(id->m_pGRegionHistTemp, n, ",\n");
									strcat_s(id->m_pGRegionHistTemp, n, json);
									save2 = json;
								}else{
									debug = -1;
									// Remove Middle Record of set
									ExtractRegionString(id, &id->m_pGRegionHistTemp, save2);
									strcat_s(id->m_pGRegionHistTemp, n, ",\n");
									strcat_s(id->m_pGRegionHistTemp, n, save2);

									// Remove Front Record of set
									ExtractRegionString(id, &id->m_pGRegionHistTemp, save1);
									strcat_s(id->m_pGRegionHistTemp, n, ",\n");
									strcat_s(id->m_pGRegionHistTemp, n, save1);									
								}
								debug++;
							}
						}
#endif//TEST_EXTRACT
						D_CONTROL("HIST_REGION_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Historical ROI: %s, %s, %s, %s, %s\n", 
							cdwThreadId, VP(ROIVO, ROIVO->count - 1, "Region", char), 
							VP(ROIVO, ROIVO->count - 1, "flightid", char), VP(ROIVO, ROIVO->count - 1, "arpt", char),
							tmpstart, tmpend) );
					}else{
						// Endtime does not exist so add to Active Record VO
						char uniqueid2[UNIQUEID_SIZE] = {0};
						if(!ROIActive){
							ROIActive = (VO*) CreateRegionVO(id, "ROIHistActVO");
						}
						vo_alloc_rows( ROIActive, 1);
						
						// Use only flightid and roiname for uniqueid for CalcRegions Algorithm
						sprintf_s( uniqueid2, _countof( uniqueid2 ), "%s%s", flightid, roiname );
						CleanString(uniqueid2);
						strncpy_s( VP(ROIActive, ROIActive->count - 1, "uniqueid", char), UNIQUEID_SIZE, uniqueid2 , _TRUNCATE );
						strncpy_s( VP(ROIActive, ROIActive->count - 1, "flightid", char), FLTNUM_SIZE, flightid, _TRUNCATE );
						strncpy_s( VP(ROIActive, ROIActive->count - 1, "Region", char), REGION_SIZE, roiname , _TRUNCATE );
						strncpy_s( VP(ROIActive, ROIActive->count - 1, "arpt", char), ARPT_CODE_SIZE, arptcode, _TRUNCATE );
						strncpy_s( VP(ROIActive, ROIActive->count - 1, "action", char), ACTION_SIZE, "insert", _TRUNCATE );
						if(strlen(dfix))
							strncpy_s( VP(ROIActive, ROIActive->count - 1, "fix", char), DEP_FIX_SIZE, dfix, _TRUNCATE );
						VV(ROIActive, ROIActive->count - 1, "starttime", int) = starttime;
						VV(ROIActive, ROIActive->count - 1, "elapsed", int) = elapsed;
						strncpy_s( VP(ROIActive, ROIActive->count - 1, "arrdep", char), ARRDEP_SIZE, arrdep, _TRUNCATE );
						VV(ROIActive, ROIActive->count - 1, "lasttime", int) = plottime;					
						VV(ROIActive, ROIActive->count - 1, "Active", int ) = 1;

						// Don't need to build JSON string, being passed to CalcRegions

						D_CONTROL("HIST_REGION_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Historical(Active) ROI: %s, %s, %s, %s\n", 
							cdwThreadId, VP(ROIActive, ROIActive->count - 1, "Region", char), 
							VP(ROIActive, ROIActive->count - 1, "flightid", char), VP(ROIActive, ROIActive->count - 1, "arpt", char),
							VOTimeFmt(tmptimebuf, VV(ROIActive, ROIActive->count - 1, "starttime", int), "%Y-%m-%d %H:%M:%S" )) );
					}

					curl_free(roiname);
					roiname = NULL;
				}
			}
		}

		clear_chunk_memory(&chunk);

		D_CONTROL("HIST_REGION_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) HistRegionVO='%s'\n\n", cdwThreadId, vo_printstr(ROIVO) ) );
		D_CONTROL("HIST_REGION_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) HistActiveVO='%s'\n", cdwThreadId, vo_printstr(ROIActive) ) );
	}	

	if(ROIActive){
		EnterCriticalSection(&id->m_csROIHist);
		id->m_pROINewActiveVO = ROIActive;
		LeaveCriticalSection(&id->m_csROIHist);
	}	
}

void ProcessHistRegions(struct InstanceData* id)
{
	char uniqueid[UNIQUEID_SIZE] = {0};
	char tmptimebuf1[64] = {0};
	char tmptimebuf2[64] = {0};
	char tmptimebuf3[64] = {0};
	char tmptimebuf4[64] = {0};
	char arpt[ARPT_CODE_SIZE] = {0};
	char region[REGION_SIZE] = {0};
	char flightid[FLTNUM_SIZE] = {0};
	char dfix[DEP_FIX_SIZE] = {0};
	char arrdep[ARRDEP_SIZE] = {0};
	time_t starttime = 0;
	time_t endtime = 0;
	int elapsed = 0;
	const char* json = NULL;
	const DWORD cdwThreadId = GetCurrentThreadId();
				
	int i = 0;
	VO* New = NULL;
	VO* ROIVO = NULL;
	if ( !id->m_pROIHistVO ){
		id->m_pROIHistVO = (VO*) CreateRegionVO(id, "ROIHistVO");
	}
	ROIVO = id->m_pROIHistVO;	
	
	// Process New Historical Regions to add to Historical List	
	if(id->m_pROINewHistVO){
		EnterCriticalSection(&id->m_csROIHist);
		New = id->m_pROINewHistVO;
		id->m_pROINewHistVO = NULL;
		LeaveCriticalSection(&id->m_csROIHist);

		for (i = 0; i < New->count; i++ ){
			strcpy_s( arpt, ARPT_CODE_SIZE, VP(New, New->count - 1, "arpt", char));
			// Only load data if for this airport (in case the airport changes)
			if( !strcmp(arpt, id->m_sCurLayout.m_strPassurArpt) || !strcmp(arpt, id->m_strROIArpt) ){
				vo_alloc_rows( ROIVO, 1);
				
				sprintf_s( uniqueid, _countof( uniqueid ), "%s%s%d", VP(New, i, "flightid", char), VP(New, i, "Region", char), VV(New, i, "starttime", int) );
				CleanString(uniqueid);
						
				strncpy_s( VP(ROIVO, ROIVO->count - 1, "uniqueid", char), UNIQUEID_SIZE, uniqueid, _TRUNCATE );
				strncpy_s( VP(ROIVO, ROIVO->count - 1, "Region", char), REGION_SIZE, VP(New, i, "Region", char), _TRUNCATE );
				strncpy_s( VP(ROIVO, ROIVO->count - 1, "flightid", char), FLTNUM_SIZE, VP(New, i, "flightid", char), _TRUNCATE );
				strncpy_s( VP(ROIVO, ROIVO->count - 1, "arpt", char), ARPT_CODE_SIZE, VP(New, i, "arpt", char), _TRUNCATE );
				strncpy_s( VP(ROIVO, ROIVO->count - 1, "fix", char), DEP_FIX_SIZE, VP(New, i, "fix", char), _TRUNCATE );
				strncpy_s( VP(ROIVO, ROIVO->count - 1, "arrdep", char), ARRDEP_SIZE, VP(New, i, "arrdep", char), _TRUNCATE );
				strncpy_s( VP(ROIVO, ROIVO->count - 1, "action", char), ACTION_SIZE, VP(New, i, "action", char), _TRUNCATE );
				VV(ROIVO, ROIVO->count - 1, "endtime", int) = VV(New, i, "endtime", int);
				VV(ROIVO, ROIVO->count - 1, "lasttime", int) = VV(New, i, "lasttime", int);
				VV(ROIVO, ROIVO->count - 1, "starttime", int) = VV(New, i, "starttime", int);
				VV(ROIVO, ROIVO->count - 1, "elapsed", int) = VV(New, i, "elapsed", int);	
				json = RegionJSON(id, ROIVO, ROIVO->count - 1);

				// Copy json into temp history string
				if(id->m_pGRegionHistTemp){
					id->m_pGRegionHistTemp = strcat_alloc(id->m_pGRegionHistTemp, ",\n");
					id->m_pGRegionHistTemp = strcat_alloc(id->m_pGRegionHistTemp, json);
				}else{
					id->m_pGRegionHistTemp = str_falloc(json);
				}

				D_CONTROL("HIST_REGION_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Active to Historical ROI: %s:%s:%s,  ST:%s,  ET:%s, LT:%s\n", 
						cdwThreadId, 
						VP(ROIVO, ROIVO->count - 1, "arpt", char), 
						VP(ROIVO, ROIVO->count - 1, "Region", char), 
						VP(ROIVO, ROIVO->count - 1, "flightid", char), 
						VOTimeFmt(tmptimebuf1, VV(ROIVO, ROIVO->count - 1, "starttime", int), "%Y-%m-%d %H:%M:%S" ),
						VOTimeFmt(tmptimebuf2, VV(ROIVO, ROIVO->count - 1, "endtime", int), "%Y-%m-%d %H:%M:%S" ),
						VOTimeFmt(tmptimebuf3, VV(ROIVO, ROIVO->count - 1, "lasttime", int), "%Y-%m-%d %H:%M:%S" )) );
			}else{
				vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Active to Historical ROI, dumping: %s:%s:%s, Reason: Incorrect Airport <%s>\n", 
						cdwThreadId, 
						VP(New, i, "arpt", char), 
						VP(New, i, "Region", char), 
						VP(New, i, "flightid", char), 
						arpt);
			}
		}
		vo_rm_rows( New, 0, New->count );
		vo_free(New);
	}

	// Process All Historical Regions for delete.
	if ( ROIVO )
	{
		for (i = 0; i < ROIVO->count; i++ ){
			endtime = VV(ROIVO, i, "endtime", int);		
			if((id->m_tPlotTime - endtime) > id->m_nMaxRegionSecs ){
				VO* DelVO = NULL;
				if ( !id->m_pROIDelVO ){
					id->m_pROIDelVO = (VO*) CreateRegionVO(id, "ROIDelVO");
				}	
				DelVO = id->m_pROIDelVO;
				// Remove JSON from History Temp
				ExtractRegionString(id, &id->m_pGRegionHistTemp, VP(ROIVO, i, "json", char));

				// Move to Delete VO
				vo_alloc_rows( DelVO, 1);
				strncpy_s( VP(DelVO, DelVO->count - 1, "uniqueid", char), UNIQUEID_SIZE, VP(ROIVO, i, "uniqueid", char), _TRUNCATE );
				strncpy_s( VP(DelVO, DelVO->count - 1, "Region", char), REGION_SIZE, VP(ROIVO, i, "Region", char), _TRUNCATE );
				strncpy_s( VP(DelVO, DelVO->count - 1, "flightid", char), FLTNUM_SIZE, VP(ROIVO, i, "flightid", char), _TRUNCATE );
				strncpy_s( VP(DelVO, DelVO->count - 1, "arpt", char), ARPT_CODE_SIZE, VP(ROIVO, i, "arpt", char), _TRUNCATE );
				strncpy_s( VP(DelVO, DelVO->count - 1, "fix", char), DEP_FIX_SIZE, VP(ROIVO, i, "fix", char), _TRUNCATE );
				strncpy_s( VP(DelVO, DelVO->count - 1, "arrdep", char), ARRDEP_SIZE, VP(ROIVO, i, "arrdep", char), _TRUNCATE );
				strncpy_s( VP(DelVO, DelVO->count - 1, "action", char), ACTION_SIZE, "delete", _TRUNCATE );
				VV(DelVO, DelVO->count - 1, "endtime", int) = VV(ROIVO, i, "endtime", int);
				VV(DelVO, DelVO->count - 1, "lasttime", int) = VV(ROIVO, i, "lasttime", int);
				VV(DelVO, DelVO->count - 1, "starttime", int) = VV(ROIVO, i, "starttime", int);
				VV(DelVO, DelVO->count - 1, "elapsed", int) = VV(ROIVO, i, "elapsed", int);	
				json = RegionJSON(id, DelVO, DelVO->count - 1);

				if(id->m_pGRegionDelTemp){
					id->m_pGRegionDelTemp = strcat_alloc(id->m_pGRegionDelTemp, ",\n");
					id->m_pGRegionDelTemp = strcat_alloc(id->m_pGRegionDelTemp, json);
				}else{
					id->m_pGRegionDelTemp = str_falloc(json);
				}
				
				D_CONTROL("HIST_REGION_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Moving Historical ROI Record to Delete: %s:%s:%s, ST:%s, ET:%s, LT:%s, PT:%s\n", 
							cdwThreadId, 
							VP(ROIVO, i, "arpt", char),
							VP(ROIVO, i, "Region", char), 
							VP(ROIVO, i, "flightid", char), 
							VOTimeFmt(tmptimebuf1, VV(ROIVO, i, "starttime", int), "%Y-%m-%d %H:%M:%S" ),
							VOTimeFmt(tmptimebuf2, VV(ROIVO, i, "endtime", int), "%Y-%m-%d %H:%M:%S" ),
							VOTimeFmt(tmptimebuf3, VV(ROIVO, i, "lasttime", int), "%Y-%m-%d %H:%M:%S" ),
							VOTimeFmt(tmptimebuf4, id->m_tPlotTime, "%Y-%m-%d %H:%M:%S" )) );
				vo_rm_row( ROIVO, i );
				i--;    
			}
		}
	}
}

void ProcessDelRegions(struct InstanceData* id)
{	
	int i = 0;	
	time_t endtime = 0;
	char tmptimebuf1[64] = {0};
	char tmptimebuf2[64] = {0};
	char tmptimebuf3[64] = {0};
	char tmptimebuf4[64] = {0};
	VO* ROIVO = id->m_pROIDelVO;
	const DWORD cdwThreadId = GetCurrentThreadId();

	if( ROIVO )
	{
		// Process All Historical Regions for delete.
		for (i = 0; i < ROIVO->count; i++ ){
			endtime = VV(ROIVO, i, "endtime", int);		
			if((id->m_tPlotTime - endtime) > (id->m_nMaxRegionSecs + id->m_nRegionTimeoutDelay) ){
				// Delete from JSON string
				ExtractRegionString(id, &id->m_pGRegionDelTemp, VP(ROIVO, i, "json", char));
				D_CONTROL("HIST_REGION_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Deleting Historical ROI Record: %s:%s:%s, ST:%s, ET:%s, LT:%s, PT:%s\n", 
							cdwThreadId, 
							VP(ROIVO, i, "arpt", char),
							VP(ROIVO, i, "Region", char), 
							VP(ROIVO, i, "flightid", char), 
							VOTimeFmt(tmptimebuf1, VV(ROIVO, i, "starttime", int), "%Y-%m-%d %H:%M:%S" ),
							VOTimeFmt(tmptimebuf2, VV(ROIVO, i, "endtime", int), "%Y-%m-%d %H:%M:%S" ),
							VOTimeFmt(tmptimebuf3, VV(ROIVO, i, "lasttime", int), "%Y-%m-%d %H:%M:%S" ),
							VOTimeFmt(tmptimebuf4, id->m_tPlotTime, "%Y-%m-%d %H:%M:%S" )) );
				vo_rm_row( ROIVO, i );
				i--;    
			}
		}
	}
}

// PURPOSE: Helper function to remove spaces and non-alphanumerics
void CleanString(char* strData)
{
	char* p = _strdup(strData);
	if(p){
		char* p1 = strData;
		char* p2 = p;
		memset(strData, 0, strlen(p));
		while(*p2){
			if(isalnum(*p2)){
				*p1 = *p2;
				p1++;
			}
			p2++;
		}
		free(p);
	}
}

const char* RegionJSON(struct InstanceData* id, VO *ROIDataVO, int i)
{
	int startsecs, endsecs, elapsed, lasttime;
	char stime[256], etime[256], elapsedtime[256];
	char action[ACTION_SIZE] = {0};
	char region_json[REGION_JSON_SIZE] = {0};
	const DWORD cdwThreadId = GetCurrentThreadId();

	// Extract VO data needed for processing 
	strcpy_s( action, ACTION_SIZE, VP(ROIDataVO, i, "action", char) );
	startsecs = VV( ROIDataVO, i, "starttime", int);
	endsecs = VV( ROIDataVO, i, "endtime", int);
	lasttime = VV( ROIDataVO, i, "lasttime", int );
	elapsed = VV( ROIDataVO, i, "elapsed", int );

	// Create substring of time format
	if(id->m_nROITimeDisplayLocal)
	{			
		VOTimeFmt(stime, startsecs, "%H:%M:%S" );
		if ( endsecs ){
			VOTimeFmt(etime, endsecs, "%H:%M:%S" );
		} else {
			etime[0] = '\0';
		}		
	}else{
		VOGMTime(stime, startsecs, "%H:%M:%S" );
		if ( endsecs ){
			VOGMTime(etime, endsecs, "%H:%M:%S" );
		} else {
			etime[0] = '\0';
		}		
	}

	// Create substring for elapsed time
	if ( elapsed > 3600 ){
		_snprintf_s(elapsedtime, sizeof(elapsedtime), _TRUNCATE, "%02d:%02d:%02d", elapsed / 3600, (elapsed % 3600) / 60 , elapsed % 60 );
	} else {
		_snprintf_s(elapsedtime, sizeof(elapsedtime), _TRUNCATE, "%02d:%02d", elapsed / 60 , elapsed % 60 );
	}

	// Create JSON string 
	_snprintf_s( region_json, REGION_JSON_SIZE, _TRUNCATE,
		"{\"uniqueid\":\"%s\", \"action\":\"%s\", \"region\":\"%s\", \"flightid\":\"%s\", \"start\":\"%s\", \"end\":\"%s\", \"elapsed\":\"%s\", "
		"\"arrdep\":\"%s\", \"fix\":\"%s\", \"active\":\"N\"}",
		VP( ROIDataVO, i, "uniqueid", char ), action, VP( ROIDataVO, i, "Region", char ), VP( ROIDataVO, i, "flightid", char ), stime, etime, elapsedtime, 
		VP( ROIDataVO, i, "arrdep", char ), VP( ROIDataVO, i, "fix", char ) );

	// Move JSON into VO
	strncpy_s(VP( ROIDataVO, i, "json", char ), REGION_JSON_SIZE, region_json, _TRUNCATE);
	return (VP( ROIDataVO, i, "json", char ));
}

// Runs thru VO's and resets the JSON string data (becuase of time format change)
void RebuildJSON(struct InstanceData* id)
{
	int i = 0;
	const char* json = NULL;
	VO* ROIVO = id->m_pROIDelVO;

	// Clean up Temp JSON strings
	if ( id->m_pGRegionDelTemp )
	{
		free( id->m_pGRegionDelTemp );
		id->m_pGRegionDelTemp = NULL;
	}
	if ( id->m_pGRegionHistTemp )
	{
		free( id->m_pGRegionHistTemp );
		id->m_pGRegionHistTemp = NULL;
	}

	// Process Del VO
	if( ROIVO ){
		for (i = 0; i < ROIVO->count; i++ ){
			json = RegionJSON(id, ROIVO, i);
			// Copy json into temp history string
			if(id->m_pGRegionDelTemp){
				id->m_pGRegionDelTemp = strcat_alloc(id->m_pGRegionDelTemp, ",\n");
				id->m_pGRegionDelTemp = strcat_alloc(id->m_pGRegionDelTemp, json);
			}else{
				id->m_pGRegionDelTemp = str_falloc(json);
			}
		}
	}

	// Process Historical VO
	ROIVO = id->m_pROIHistVO;
	if( ROIVO ) {
		for (i = 0; i < ROIVO->count; i++ ){
			json = RegionJSON(id, ROIVO, i);
			// Copy json into temp history string
			if(id->m_pGRegionHistTemp){
				id->m_pGRegionHistTemp = strcat_alloc(id->m_pGRegionHistTemp, ",\n");
				id->m_pGRegionHistTemp = strcat_alloc(id->m_pGRegionHistTemp, json);
			}else{
				id->m_pGRegionHistTemp = str_falloc(json);
			}
		}
	}
}

void BuildHistRegionsJSON(struct InstanceData* id)
{
	char *RegStr = NULL;
	char *RegStr2 = NULL;
	char tmpstr[4096] = {0};
	//char stime[256], etime[256], elapsedtime[256];	
	//int startsecs, endsecs, elapsed, lasttime;
	VO *ROIDataVO = NULL;
	const DWORD cdwThreadId = GetCurrentThreadId();

	if(id->m_pGRegionDelTemp){
		RegStr2 = str_falloc( "\t, \"deleted\": [\n" );
		RegStr2 = strcat_alloc( RegStr2, id->m_pGRegionDelTemp );
		RegStr2 = strcat_alloc( RegStr2, "\n\t]\n}\n" );
	}

	if(id->m_pGRegionHistTemp){
		RegStr = str_falloc( "\t, \"historical\": [\n" );
		RegStr = strcat_alloc( RegStr, id->m_pGRegionHistTemp );
		RegStr = strcat_alloc( RegStr, "\n\t]\n}\n" );
	}

	if( id->m_nROITimeDisplayLocal == id->m_sUserSettings.TimeDisplayLocal ){
		if(RegStr){
			EnterCriticalSection(&id->m_csROIHist);
			if( id->m_pGRegionsHistorical ) {					
				free(id->m_pGRegionsHistorical);
				id->m_pGRegionsHistorical = NULL;
			}		
			id->m_pGRegionsHistorical = RegStr;
			LeaveCriticalSection(&id->m_csROIHist);
			D_CONTROL("HIST_REGION_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X): Current Regions <%d:%s>\n", cdwThreadId, strlen(id->m_pGRegionsHistorical), id->m_pGRegionsHistorical));
		}
		if(RegStr2){
			EnterCriticalSection(&id->m_csROIHist);
			if(id->m_pGRegionsDelete){					
				free(id->m_pGRegionsDelete);
				id->m_pGRegionsDelete = NULL;
			}
			id->m_pGRegionsDelete = RegStr2;
			LeaveCriticalSection(&id->m_csROIHist);
			D_CONTROL("HIST_REGION_DATA", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X): Current Deletable Regions <%d:%s>\n", cdwThreadId, strlen(id->m_pGRegionsDelete), id->m_pGRegionsDelete));
		}
	}else{
		// Needs to be redone on next loop
		// Created json strings are dumped
		free(RegStr);
		free(RegStr2);

		id->m_nROITimeDisplayLocal = id->m_sUserSettings.TimeDisplayLocal;
		RebuildJSON(id);
	}
	TIME_TRACE_DBG("\t\tBuildHistRegionsJSON(3)");

}

// This thread is to load the VO data from large downloaded files and is only needed to be done once
unsigned _stdcall HistoricalROIDataThread(PVOID pvoid)
{
	struct InstanceData* id = (struct InstanceData*)pvoid;
	int bReplay = FALSE;
	char strConfigValue[100] = {0};	
	const DWORD cdwThreadId = GetCurrentThreadId();
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X):[%d] Thread Created\n", cdwThreadId, id->m_nControlId);

	//// Seconds in past for which to retreive old region entries
	if(GetConfigValue(id, "HIST_REGION_SECS", strConfigValue, 100)){
		id->m_nHistRegionSecs = atoi(strConfigValue);
	}

	if(GetConfigValue(id, "REGION_TIMEOUT_DELAY", strConfigValue, 100)){
		id->m_nRegionTimeoutDelay = atoi(strConfigValue);
	}

	do
	{
		TIME_TRACE_DBG("START - HistoricalROIDataThread");
		if ( !bReplay && REPLAY_STATUS_PLAYING == id->m_bReplayStatus ){
			bReplay = TRUE;
			ClearRegionData(id);
			GetHistRegions(id, TRUE);
		}else if ( bReplay && REPLAY_STATUS_STOPPED == id->m_bReplayStatus ){
			bReplay = FALSE;
			ClearRegionData(id);
			GetHistRegions(id, TRUE);
		}

		if ( id->m_sCurLayout.m_bShowASDEXAircraft ){
			if ( REPLAY_STATUS_PAUSED != id->m_bReplayStatus ){
				TIME_TRACE_DBG("\tSTART - GetHistRegions");
				GetHistRegions(id, FALSE);
				TIME_TRACE_DBG("\tEND - GetHistRegions");

				TIME_TRACE_DBG("\tSTART - ProcessDelRegions");
				ProcessDelRegions(id);
				TIME_TRACE_DBG("\tEND - ProcessDelRegions");

				TIME_TRACE_DBG("\tSTART - ProcessHistRegions");
				ProcessHistRegions(id);
				TIME_TRACE_DBG("\tEND - ProcessHistRegions");

				TIME_TRACE_DBG("\tSTART - BuildHistRegionsJSON");
				BuildHistRegionsJSON(id);
				TIME_TRACE_DBG("\tEND - BuildHistRegionsJSON");
			}
		}else{
			ClearRegionData(id);
		}
		TIME_TRACE_DBG("END - HistoricalROIDataThread");
	}while(WAIT_TIMEOUT == WaitForSingleObject(id->m_evShutdown, id->m_dwBkThread));

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X):[%d] Thread Terminated\n", cdwThreadId, id->m_nControlId);
	_endthreadex(0);
	return 0;
}


