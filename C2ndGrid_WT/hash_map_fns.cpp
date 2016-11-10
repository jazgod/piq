#include <hash_map>
#include <string>

typedef stdext::hash_map<std::string,int> mapInts;
typedef stdext::hash_map<std::string,int>::iterator mapIntsIter;


#ifdef __cplusplus 
extern "C"{
#endif//__cplusplus 
#include "InstanceData.h"

#define FLTNUM_SIZE 8

extern int IsDebugNameDefined(struct InstanceData* id, const char* strDebugName);
#define D_CONTROL(vcolname, exp) {if ( 5 == id->m_nDebugControl || (id->m_nDebugControl && IsDebugNameDefined(id, vcolname)) ) {exp;}}
#define IS_D_CONTROL(vcolname) ( 5 == id->m_nDebugControl || (id->m_nDebugControl && IsDebugNameDefined(id, vcolname)) )

extern void StripLeading0( char *src, char *des, int deslen );
extern int vo_log_info( ... );
extern char g_arFeedTypes[9][10];

void AddIndex(struct InstanceData* id, void* stlContainer, char *strFltNum, int icao24, int beacon, int nIndex, int nFeedType)
{
	char strFlight[FLTNUM_SIZE] = {0};
	char strIcao24[FLTNUM_SIZE] = {0};
	char strBeacon[FLTNUM_SIZE] = {0};
	mapInts* mi = (mapInts*)stlContainer;

	if( mi && strlen(strFltNum) )	
	{
		D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Adding index [%d] for flight (%s) to %s\n", 
			GetCurrentThreadId(), nIndex, strFltNum, g_arFeedTypes[nFeedType] ) );

		StripLeading0( strFltNum, strFlight, sizeof(strFlight) );  // always strip leading zeros for comparison
		(*mi)[strFlight] = nIndex;
	}

	if(mi && icao24)
	{
		sprintf_s(strIcao24, FLTNUM_SIZE, "%X", icao24);
		D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Adding index [%d] for flight with icao24 (%s) to %s\n", 
			GetCurrentThreadId(), nIndex, strIcao24, g_arFeedTypes[nFeedType] ) );
		(*mi)[strIcao24] = nIndex;
	}

	if(mi && beacon && beacon != 640)
	{
		sprintf_s(strBeacon, FLTNUM_SIZE, "%04o", beacon);
		D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Adding index [%d] for flight with beacon (%s) to %s\n", 
			GetCurrentThreadId(), nIndex, strBeacon, g_arFeedTypes[nFeedType] ) );
		(*mi)[strBeacon] = nIndex;
	}
}

void ClearSavedIndices(struct InstanceData* id, void* stlContainer, int nFeedType)
{
	mapInts* mi = (mapInts*)stlContainer;
	mi->clear();
	D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Clearing stored index:flightnum pairs for feed %s\n", 
			GetCurrentThreadId(), g_arFeedTypes[nFeedType] ) );
}

void* CreateIndexMap(struct InstanceData* id)
{
	mapInts* mi = new mapInts;
	return ( mi );
}

void DumpIndexMap(struct InstanceData* id, void* stlContainer)
{
	mapInts* mi = (mapInts*)stlContainer;
	mapIntsIter i;
	
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) IndexMap Dump\n", 
			GetCurrentThreadId());
	for(i = mi->begin(); i != mi->end(); i++){
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "[%d] = \"%s\"\n", 
			i->second, i->first.c_str() );
	}
}

void DestroyIndexMap(struct InstanceData* id, void* stlContainer)
{
	mapInts* mi = (mapInts*)stlContainer;
	if(IS_D_CONTROL(__FUNCTION__))
		DumpIndexMap(id, stlContainer);
	mi->clear();
	delete mi;
}

int GetIndex(struct InstanceData* id, void* stlContainer, char *strFltNum, int icao24, int beacon, int nFeedType)
{
	char strFlight[FLTNUM_SIZE] = {0};
	char strIcao24[FLTNUM_SIZE] = {0};
	char strBeacon[FLTNUM_SIZE] = {0};
	char fltnum[FLTNUM_SIZE] = {0};
	struct row_index *row_index = NULL;
	int nIndex = -1;
	mapInts* mi = (mapInts*)stlContainer;
	mapIntsIter i;

	if( mi && strlen(strFltNum) )	
	{
		StripLeading0( strFltNum, strFlight, sizeof(strFlight) );  // always strip leading zeros for comparison
		D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Searching for Flight (%s) in %s\n", 
			GetCurrentThreadId(), strFltNum, g_arFeedTypes[nFeedType] ) );

		i = mi->find(strFlight);
		if(i != mi->end()){
			nIndex = i->second;
			strcpy_s( fltnum, FLTNUM_SIZE, i->first.c_str() ); 	
			D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Found index (%d) for Flight (%s) in %s\n", 
					GetCurrentThreadId(), nIndex, strFltNum, g_arFeedTypes[nFeedType] ) );
		}else{
			D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) No index found for Flight (%s) in %s\n", 
					GetCurrentThreadId(), strFltNum, g_arFeedTypes[nFeedType] ) );
		}
	}
	if(-1 == nIndex && mi && icao24){
		sprintf_s(strIcao24, FLTNUM_SIZE, "%X", icao24);
		D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Searching for Flight with icao24(%s) in %s\n", 
			GetCurrentThreadId(), strIcao24, g_arFeedTypes[nFeedType] ) );
		i = mi->find(strIcao24);
		if(i != mi->end()){
			nIndex = i->second;
			D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Found index (%d) for Flight with icao24 (%s) in %s\n", 
					GetCurrentThreadId(), nIndex, strIcao24, g_arFeedTypes[nFeedType] ) );
		}else{
			D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) No index found for Flight with icao24 (%s) in %s\n", 
					GetCurrentThreadId(), strIcao24, g_arFeedTypes[nFeedType] ) );
		}
	}
	if(-1 == nIndex && mi && beacon && beacon != 640){
		sprintf_s(strBeacon, FLTNUM_SIZE, "%04o", beacon);
		D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Searching for Flight with beacon(%s) in %s\n", 
			GetCurrentThreadId(), strBeacon, g_arFeedTypes[nFeedType] ) );
		i = mi->find(strBeacon);
		if(i != mi->end()){
			nIndex = i->second;
			D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Found index (%d) for Flight with beacon (%s) in %s\n", 
					GetCurrentThreadId(), nIndex, strBeacon, g_arFeedTypes[nFeedType] ) );
		}else{
			D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) No index found for Flight with beacon (%s) in %s\n", 
					GetCurrentThreadId(), strBeacon, g_arFeedTypes[nFeedType] ) );
		}
	}
	return ( nIndex );
}

void RemoveIndex(struct InstanceData* id, void* stlContainer, char* strFltNum, int icao24, int beacon, int nFeedType)
{
	char strFlight[FLTNUM_SIZE] = {0};
	char strIcao24[FLTNUM_SIZE] = {0};
	char strBeacon[FLTNUM_SIZE] = {0};
	mapInts* mi = (mapInts*)stlContainer;
	mapIntsIter i;
	if ( mi && strlen(strFltNum))	
	{
		StripLeading0(strFltNum, strFlight, sizeof(strFlight) );  // always strip leading zeros for comparison		
		i = mi->find(strFlight);
		if(i != mi->end()){
			D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Removing index (%d) for flight (%s) in %s\n", 
				GetCurrentThreadId(), i->second, strFltNum, g_arFeedTypes[nFeedType] ) );
			mi->erase(i);
		}else{
			D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) No index found for Flight (%s) in %s\n", 
					GetCurrentThreadId(), strFltNum, g_arFeedTypes[nFeedType] ) );
		}
	}
	if ( mi && icao24 )
	{
		sprintf_s(strIcao24, FLTNUM_SIZE, "%X", icao24);
		i = mi->find(strIcao24);
		if(i != mi->end()){
			D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Removing index (%d) for flight with icao24 (%s) in %s\n", 
				GetCurrentThreadId(), i->second, strIcao24, g_arFeedTypes[nFeedType] ) );
			mi->erase(i);
		}else{
			D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) No index found for Flight with icao24 (%s) in %s\n", 
					GetCurrentThreadId(), strIcao24, g_arFeedTypes[nFeedType] ) );
		}
	}
	if ( mi && beacon && beacon != 640 )
	{
		sprintf_s(strBeacon, FLTNUM_SIZE, "%04o", beacon);
		i = mi->find(strBeacon);
		if(i != mi->end()){
			D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Removing index (%d) for flight with beacon (%s) in %s\n", 
				GetCurrentThreadId(), i->second, strBeacon, g_arFeedTypes[nFeedType] ) );
			mi->erase(i);
		}else{
			D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) No index found for Flight with beacon (%s) in %s\n", 
					GetCurrentThreadId(), strBeacon, g_arFeedTypes[nFeedType] ) );
		}
	}
}

#ifdef __cplusplus 
}
#endif//__cplusplus 
