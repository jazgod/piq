#include <math.h>
#include "InstanceData.h"
#include "vo_db.h"
#include "vo.h"
#include "vo_extern.h"
#include "mgl.h"
#include "srfc.h"
#include "tracking.h"
#include "hash_map_fns.h"

void MergeTracks(struct InstanceData* id, AIR* air, int nFeedType, int nCopyToFeedType, struct feedstruct *record);
PTRACK *NewTrackArr( struct InstanceData* id, int max_tracks, AIR *air );
void PrintTracks( struct InstanceData* id, AIR *air, int nFeedType );
void ReverseMergeTracks(struct InstanceData* id, AIR* airDest, AIR* airSrc, int nDestFeedType, int nSrcFeedType);
double Distance(double lat1, double lng1, double lat2, double lng2);

int DistanceCheck(struct InstanceData* id, AIR* air, struct feedstruct *record){
	if(air){
		int t = abs(record->eventtime - air->utc);
		int speed = 0;
		double d1 = 0.0f, d2 = 0.0f;
		t = max(1,t);
		speed = max(air->speed, record->speed);
		d1 = (speed * (t/3600.0)) * 1.5;
		d2 = Distance(air->CurY, air->CurX, record->lat, record->lng);
		if( d2 < d1 ) {
			D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) + FLIGHT: %s SPEED %d D1(%f) D2(%f)\n", 
					GetCurrentThreadId(), air->FltNum, air->speed, d1, d2));
			return TRUE;
		}else{
			D_CONTROL(__FUNCTION__, vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) - FLIGHT: %s SPEED %d, TDIFF(%d secs) D1(%f) D2(%f)\n", 
				GetCurrentThreadId(), air->FltNum, air->speed, t, d1, d2));
		
		}
	}
	
	return FALSE;
}


// Generic Track Reset Function, Resets passed in track array using feed tracking value
void ClearTrackedTracks( struct InstanceData* id, AIR** pairp,  int nTrackCount, int nFeedByType)
{
	AIR *pair;
	int i;
	const DWORD cdwThreadId = GetCurrentThreadId();	
	for (i = 0; pairp && i < nTrackCount; i++ ){
		if ( !(pair = pairp[i]) ){
			continue;
		}
		pair->IsTracked &= ~nFeedByType;
	}
}


// Clears ASD, PASSUR, MLAT and ASDEX Tracks Tracked By ADSB Feed
void ClearADSBTrackedTracks( struct InstanceData* id )
{
	AIR** pairp;
	const DWORD cdwThreadId = GetCurrentThreadId();	

	// Clear ASDEX Feed 
	pairp = GetASDEXAirP( id );
	ClearTrackedTracks(id, pairp, N_ASDEX_TRACK_ID, TBF_ADSB);

	// Clear MLAT Feed 
	pairp = GetMLATAirP( id );
	ClearTrackedTracks(id, pairp, N_MLAT_TRACK_ID, TBF_ADSB);

	// Clear PASSUR Feed
	pairp = GetAirP( id, 0 );
	ClearTrackedTracks(id, pairp, N_PASSUR_TRACK_ID, TBF_ADSB);

	// Clear ASD Feed
	pairp = GetASDAirP( id );
	ClearTrackedTracks(id, pairp, N_ASDI_TRACK_ID, TBF_ADSB);

	// Reset ADSB Index Map
	ClearSavedIndices(id, id->m_pADSBIndexMap, ADSB);

	D_CONTROL("ADSB_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Cleared ASDEX, PASSUR and ASD tracks tracked by ADSB\n", cdwThreadId));
}

// Clears ASD and PASSUR Tracks Tracked By MLAT Feed
void ClearMLATTrackedTracks( struct InstanceData* id )
{
	AIR** pairp;
	const DWORD cdwThreadId = GetCurrentThreadId();	
	
	// Clear PASSUR Feed
	pairp = GetAirP( id, 0 );
	ClearTrackedTracks(id, pairp, N_PASSUR_TRACK_ID, TBF_MLAT);

	// Clear ASD Feed
	pairp = GetASDAirP( id );
	ClearTrackedTracks(id, pairp, N_ASDI_TRACK_ID, TBF_MLAT);

	// Reset MLAT Index Map
	ClearSavedIndices(id, id->m_pMLATIndexMap, MLAT);

	D_CONTROL("MLAT_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Cleared PASSUR and ASD tracks tracked by MLAT\n", cdwThreadId));
}

void MarkMLATTracked(struct InstanceData* id, char* FltNum, int icao24, int beacon, int nFeedType, struct feedstruct *record)
{
	AIR **airp = NULL;	
	AIR *air = NULL;
	char newstr[FLTNUM_SIZE] = {0};
	struct row_index *row_index = NULL;
	int nIndex = GetIndex(id, id->m_pMLATIndexMap, FltNum, icao24, beacon, MLAT);
	if(-1 != nIndex)
	{
		airp = GetMLATAirP(id);
		if(airp && (air = airp[nIndex]))
		{
			if(ADSB == nFeedType){
				MergeTracks(id, air, MLAT, ADSB, NULL);
			}else if(ASDEX == nFeedType){				
				MergeTracks(id, air, MLAT, ASDEX, record);
			}
			D_CONTROL( "MLAT_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Marking %s MLAT[%d] Flight as tracked(%d) for feed %s\n", 
				GetCurrentThreadId(), air->FltNum, nIndex, air->IsTracked, g_arFeedTypes[nFeedType]) );
		}
	}else{
		D_CONTROL( "MLAT_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Ignoring Flight %s, Not found in MLAT SearchVO\n", 
				GetCurrentThreadId(), FltNum) );
	}
}


void CkNewADSBOverlap(struct InstanceData* id, AIR* airp)
{
	if ( !id->m_sCurLayout.m_bShowADSBAircraft ){
		return;
	}

	// Check for new Flight
	if(airp){// && -1 == GetIndex(id, id->m_pADSBIndexMap, airp->FltNum, airp->icao24, airp->beacon, ADSB)){
		// Add Track to Search VO
		AddIndex(id, id->m_pADSBIndexMap, airp->FltNum, airp->icao24, airp->beacon, airp->trackid, ADSB);
	}
	// Mark ASDEX track as 
	MarkASDEXTracked(id, airp->FltNum, airp->icao24, airp->beacon);

	// Mark MLAT track as 
	MarkMLATTracked(id, airp->FltNum, airp->icao24, airp->beacon, ADSB, NULL);

	// Mark PASSUR track as 
	MarkPassurTracked(id, airp->FltNum, airp->icao24, airp->beacon, ADSB, NULL);

	// Mark ASD track as 
	MarkASDTracked(id, airp->FltNum, airp->icao24, airp->beacon, ADSB, NULL);
}

void MergeField(struct InstanceData* id, char* p1, char* p2, int l)
{
	if(!strlen(p1) && strlen(p2))
		strcpy_s(p1, l, p2);
	else if(strlen(p1) && !strlen(p2))
		strcpy_s(p2, l, p1);
	
}
void MergeMLATandASDEX(struct InstanceData* id, AIR* air /*MLAT*/, AIR* air2 /*ASDEX*/)
{
	if( air->InMlatRegion || air2->InMlatRegion){
		// Mark ASDEX as being tracked by MLAT
		if(!(air2->IsTracked & TBF_MLAT)){
			air2->IsTracked |= TBF_MLAT;
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Marking %s ASDEX[%d] Flight as tracked(%d) from feed %s\n", 
				GetCurrentThreadId(), air2->FltNum, air2->trackid, air2->IsTracked, g_arFeedTypes[MLAT]);
		}
		// Make sure we are not tracked by ASDEX
		if(air->IsTracked & TBF_ASDEX){
			air->IsTracked &= ~TBF_ASDEX;
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Marking %s MLAT[%d] Flight as NOT tracked(%d) from feed %s\n", 
				GetCurrentThreadId(), air->FltNum, air->trackid, air->IsTracked, g_arFeedTypes[ASDEX]);
			// Move ASDEX Tracks to MLAT
			air->tcount = air2->tcount;
			if(air->TrackPtr)
				free(air->TrackPtr);
			air->TrackPtr = air2->TrackPtr;
			air2->tcount = 0;
			air2->TrackPtr = NULL;			
		}				
	}else{
		// Mark this track as Tracked by ASDEX
		if(!(air->IsTracked & TBF_ASDEX)){
			air->IsTracked |= TBF_ASDEX;
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Marking %s MLAT[%d] Flight as tracked(%d) from feed %s\n", 
				GetCurrentThreadId(), air->FltNum, air->trackid, air->IsTracked, g_arFeedTypes[ASDEX]);
		}
		// Make sure ASDEX is not tracked by MLAT
		if(air2->IsTracked & TBF_MLAT){
			air2->IsTracked &= ~TBF_MLAT;
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Marking %s ASDEX[%d] Flight as NOT tracked(%d) from feed %s\n", 
				GetCurrentThreadId(), air2->FltNum, air2->trackid, air2->IsTracked, g_arFeedTypes[MLAT]);
			// Copy MLAT Tracks to ASDEX
			air2->tcount = air->tcount;
			if(air2->TrackPtr)
				free(air2->TrackPtr);
			air2->TrackPtr = air->TrackPtr;
			air->tcount = 0;
			air->TrackPtr = NULL;
		}
	}
	// Merge Data
	MergeField(id, air->FltNum, air2->FltNum, FLTNUM_SIZE + 1);
	MergeField(id, air->Origin, air2->Origin, O_D_SIZE + 1);
	MergeField(id, air->Destin, air2->Destin, O_D_SIZE + 1);
	MergeField(id, air->gate,   air2->gate,   GATE_SIZE + 1);
	MergeField(id, air->runway, air2->runway, RUNWAY_SIZE + 1);
	MergeField(id, air->actype, air2->actype, ACTYPE_SIZE + 1);	
}

// PURPOSE: Check ADSB Overlap, Mark as being tracked by ASDEX
//			Check PASSUR and ASDI, Mark this tracks as being tracked.
void CkNewASDEXOverlap(struct InstanceData* id, AIR* air, struct feedstruct *record)
{
	int nIndex;
	
	if ( !id->m_sCurLayout.m_bShowASDEXAircraft ){
		return;
	}

	if ( !strcmp(air->FltNum, "UNKN") || !strcmp(air->FltNum, "ANON") ){
		return;
	}

	// Check for new Flight
	if(air){// && -1 == GetIndex(id, id->m_pASDEXIndexMap, airp->FltNum, airp->icao24, airp->beacon, ASDEX)){
		// Add Track to Search VO
		AddIndex(id, id->m_pASDEXIndexMap, air->FltNum, air->icao24, air->beacon, air->trackid, ASDEX);
	}
	// Mark MLAT track as 
	if(-1 != (nIndex = GetIndex(id, id->m_pMLATIndexMap, air->FltNum, air->icao24, air->beacon, MLAT))){
		// checking to see if last track is old enough (lkp) to drop to lower priority 
		AIR** airp = GetMLATAirP(id);
		AIR* air2 = airp[nIndex];
		if( air2 ){
			MergeMLATandASDEX(id, air2, air);
		}
	}

	// Mark PASSUR track as 
	MarkPassurTracked(id, air->FltNum, air->icao24, air->beacon, ASDEX, record);

	// Mark ASD track as 
	MarkASDTracked(id, air->FltNum, air->icao24, 0, ASDEX, record);
	
	// Check ADSB, mark this ASDEX track as "Tracked"
	if(-1 != (nIndex = GetIndex(id, id->m_pADSBIndexMap, air->FltNum, air->icao24, air->beacon, ADSB))){
		air->IsTracked |= TBF_ADSB;
		D_CONTROL( "ASDEX_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Marking %s ASDEX[%d] Flight as tracked(%d) from feed %s\n", 
				GetCurrentThreadId(), air->FltNum, nIndex, air->IsTracked, g_arFeedTypes[ADSB]) );
	}	
}

void CkNewMLATOverlap(struct InstanceData* id, AIR* air, struct feedstruct *record)
{
	int nIndex = -1;
	if ( !id->m_sCurLayout.m_bShowMLATAircraft ){
		return;
	}

	// Check for new Flight
	if(air){
		// Always Add Track to Search VO which will only add if data is missing
		AddIndex(id, id->m_pMLATIndexMap, air->FltNum, air->icao24, air->beacon, air->trackid, MLAT);
	}

	// Check ASDEX, mark this MLAT track as "Tracked"
	if(-1 != (nIndex = GetIndex(id, id->m_pASDEXIndexMap, air->FltNum, air->icao24, air->beacon, ASDEX))){
		// checking to see if last track is old enough (lkp) to drop to lower priority 
		AIR** airp = GetASDEXAirP(id);
		AIR* air2 = airp[nIndex];
		if( air2 ){
			MergeMLATandASDEX(id, air, air2);
		}
	}	
	
	// Mark PASSUR track as 
	MarkPassurTracked(id, air->FltNum, air->icao24, air->beacon, MLAT, record);

	// Mark ASD track as 
	MarkASDTracked(id, air->FltNum, air->icao24, 0, MLAT, record);
}

void ClearMLATTracked(struct InstanceData* id, char* FltNum, int icao24, int beacon, int nFeedType)
{
	AIR **airp = NULL;	
	AIR *air = NULL;
	char newstr[FLTNUM_SIZE] = {0};
	struct row_index *row_index = NULL;
	int nIndex = GetIndex(id, id->m_pMLATIndexMap, FltNum, icao24, beacon, ASDEX);
	if(-1 != nIndex)
	{
		airp = GetMLATAirP(id);
		if(airp && (air = airp[nIndex]))
		{
			if(ADSB == nFeedType){
				air->IsTracked &= ~TBF_ADSB;			
			}else if(ASDEX == nFeedType){
				air->IsTracked &= ~TBF_ASDEX;			
			}
			// No other Feed Type should have an effect

			D_CONTROL( "MLAT_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Marking %s MLAT[%d] Flight as NOT tracked(%d) from feed %s\n", 
				GetCurrentThreadId(), air->FltNum, nIndex, air->IsTracked, g_arFeedTypes[nFeedType]) );
		}
	}
}



void ClearASDEXTracked(struct InstanceData* id, char* FltNum, int icao24, int beacon, int nFeedType)
{
	AIR **airp = NULL;	
	AIR *air = NULL;
	char newstr[FLTNUM_SIZE] = {0};
	struct row_index *row_index = NULL;
	int nIndex = GetIndex(id, id->m_pASDEXIndexMap, FltNum, icao24, beacon, ASDEX);
	if(-1 != nIndex)
	{
		airp = GetASDEXAirP(id);
		if(airp && (air = airp[nIndex]))
		{
			if(ADSB == nFeedType){
				air->IsTracked &= ~TBF_ADSB;			
			}
			// No other Feed Type should have an effect

			D_CONTROL( "ASDEX_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Marking %s ASDEX[%d] Flight as NOT tracked(%d) from feed %s\n", 
				GetCurrentThreadId(), air->FltNum, nIndex, air->IsTracked, g_arFeedTypes[nFeedType]) );
		}
	}
}

// Clears ASD and PASSUR Tracks Tracked By ASDEX Feed
void ClearASDEXTrackedTracks( struct InstanceData* id )
{
	AIR** pairp;
	const DWORD cdwThreadId = GetCurrentThreadId();	
	
	// Clear MLAT Feed
	pairp = GetMLATAirP( id );
	ClearTrackedTracks(id, pairp, N_MLAT_TRACK_ID, TBF_ASDEX);

	// Clear PASSUR Feed
	pairp = GetAirP( id, 0 );
	ClearTrackedTracks(id, pairp, N_PASSUR_TRACK_ID, TBF_ASDEX);

	// Clear ASD Feed
	pairp = GetASDAirP( id );
	ClearTrackedTracks(id, pairp, N_ASDI_TRACK_ID, TBF_ASDEX);

	// Reset ADSB Index Map
	ClearSavedIndices(id, id->m_pASDEXIndexMap, ASDEX);

	D_CONTROL("ASDEX_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Cleared ASD, PASSUR and MLAT tracks tracked by ASDEX\n", cdwThreadId));
}



void MarkASDEXTracked(struct InstanceData* id, char* FltNum, int icao24, int beacon)
{
	AIR **airp = NULL;	
	AIR *air = NULL;
	char newstr[FLTNUM_SIZE] = {0};
	struct row_index *row_index = NULL;
	int nIndex = GetIndex(id, id->m_pASDEXIndexMap, FltNum, icao24, beacon, ASDEX);
	if(-1 != nIndex)
	{
		airp = GetASDEXAirP(id);
		if(airp && (air = airp[nIndex]))
		{
			if(!(air->IsTracked & TBF_ADSB)){
				MergeTracks(id, air, ASDEX, ADSB, NULL);
			}

			D_CONTROL( "ASDEX_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Marking %s ASDEX[%d] Flight as tracked(%d) from feed ADSB\n", 
				GetCurrentThreadId(), air->FltNum, nIndex, air->IsTracked) );
		}
	}else{
		D_CONTROL( "ASDEX_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Ignoring Flight %s, Not found in ADSB\n", 
				GetCurrentThreadId(), FltNum) );
	}
}


// PURPOSE: Check ASD for PASSUR Overlap, Mark as being tracked by PASSUR
//			Check ASDEX and ADSB/ Mark this tracks as being tracked.
void CkNewPassurOverlap(struct InstanceData* id, AIR* air)
{
	int nIndex = -1;
	AIR** airp = NULL;
	AIR* air2 = NULL;

	if ( !id->m_sCurLayout.m_bShowPassurAircraft ){
		return;
	}

	// Check for new Flight
	if(air){// && -1 == GetIndex(id, id->m_pPassurIndexMap, air->FltNum, air->icao24, air->beacon, PASSUR)){
		// Add Track to Search VO
		AddIndex(id, id->m_pPassurIndexMap, air->FltNum, air->icao24, air->beacon, air->trackid, PASSUR);
	}

	// Mark ASD track as 
	MarkASDTracked(id, air->FltNum, air->icao24, air->beacon, PASSUR, NULL);

	// Check MLAT, mark this PASSUR track as "Tracked"
	if(-1 != (nIndex = GetIndex(id, id->m_pMLATIndexMap, air->FltNum, air->icao24, air->beacon, MLAT))){
		// checking to see if last track is old enough (lkp) to drop to lower priority 
		airp = GetMLATAirP(id);
		air2 = airp[nIndex];
		if( air2 ){
			if( (air->utc - air2->utc) > RPT_MLAT_TIMEOUT ){
				// Remove MLAT Track, unmark PASSUR as tracked by MLAT
				ReverseMergeTracks(id, air, air2, PASSUR, MLAT);
				RemoveTrack(id, airp, nIndex, MLAT, FALSE);
				D_CONTROL( "PASSUR_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Inverse Priority, Marking %s PASSUR[%d] Flight as not tracked(%d) from feed %s\n", 
					GetCurrentThreadId(), air->FltNum, nIndex, air->IsTracked, g_arFeedTypes[MLAT]) );
			}else {
				air->IsTracked |= TBF_MLAT;
				D_CONTROL( "PASSUR_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Marking %s PASSUR[%d] Flight as tracked(%d) from feed %s\n", 
					GetCurrentThreadId(), air->FltNum, nIndex, air->IsTracked, g_arFeedTypes[MLAT]) );
			}		
		}else{
			air->IsTracked &= ~TBF_MLAT;
			RemoveIndex(id, id->m_pMLATIndexMap, air->FltNum, air->icao24, air->beacon, MLAT);
			D_CONTROL( "PASSUR_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Bad index(%d) found in MLAT search of (%s, %X, %04o) removing.\n", 
				GetCurrentThreadId(), nIndex, air->FltNum, air->icao24, air->beacon) );
		}
	}
	
	// Check ASDEX, mark this PASSUR track as "Tracked"
	if(-1 != (nIndex = GetIndex(id, id->m_pASDEXIndexMap, air->FltNum, air->icao24, air->beacon, ASDEX))){
		// checking to see if last track is old enough (lkp) to drop to lower priority 
		airp = GetASDEXAirP(id);
		air2 = airp[nIndex];
		if( air2 ){
			if( (air->utc - air2->utc) > RPT_ASDEX_TIMEOUT ){
				// Remove ASDEX Track, unmark PASSUR as tracked by ASDEX
				ReverseMergeTracks(id, air, air2, PASSUR, ASDEX);
				RemoveTrack(id, airp, nIndex, ASDEX, FALSE);
				D_CONTROL( "PASSUR_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Inverse Priority, Marking %s PASSUR[%d] Flight as not tracked(%d) from feed %s\n", 
					GetCurrentThreadId(), air->FltNum, nIndex, air->IsTracked, g_arFeedTypes[ASDEX]) );
			}else {
				air->IsTracked |= TBF_ASDEX;
				D_CONTROL( "PASSUR_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Marking %s PASSUR[%d] Flight as tracked(%d) from feed %s\n", 
					GetCurrentThreadId(), air->FltNum, nIndex, air->IsTracked, g_arFeedTypes[ASDEX]) );
			}
		}else{
			air->IsTracked &= ~TBF_ASDEX;
			RemoveIndex(id, id->m_pASDEXIndexMap, air->FltNum, air->icao24, air->beacon, ASDEX);
			D_CONTROL( "PASSUR_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Bad index(%d) found in ASDEX search of (%s, %X, %04o) removing.\n", 
				GetCurrentThreadId(), nIndex, air->FltNum, air->icao24, air->beacon) );
		}
	}	

	// Check ADSB, mark this PASSUR track as "Tracked"
	if(-1 != (nIndex = GetIndex(id, id->m_pADSBIndexMap, air->FltNum, air->icao24, air->beacon, ADSB))){
		// checking to see if last track is old enough (lkp) to drop to lower priority 
		airp = GetADSBAirP(id);
		air2 = airp[nIndex];
		if( air2 ){			
			if( (air->utc - air2->utc) > RPT_ADSB_TIMEOUT ){
				// Remove ADSB Track, unmark PASSUR as tracked by ADSB
				ReverseMergeTracks(id, air, air2, PASSUR, ADSB);
				RemoveTrack(id, airp, nIndex, ADSB, FALSE);
				D_CONTROL( "PASSUR_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Inverse Priority, Marking %s PASSUR[%d] Flight as not tracked(%d) from feed %s\n", 
					GetCurrentThreadId(), air->FltNum, nIndex, air->IsTracked, g_arFeedTypes[ADSB]) );
			}else {
				air->IsTracked |= TBF_ADSB;
				D_CONTROL( "PASSUR_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Marking %s PASSUR[%d] Flight as tracked(%d) from feed %s\n", 
					GetCurrentThreadId(), air->FltNum, nIndex, air->IsTracked, g_arFeedTypes[ADSB]) );
			}
		}else{
			air->IsTracked &= ~TBF_ADSB;
			RemoveIndex(id, id->m_pADSBIndexMap, air->FltNum, air->icao24, air->beacon, ADSB);
			D_CONTROL( "PASSUR_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Bad index(%d) found in ADSB search of (%s, %X, %04o) removing.\n", 
				GetCurrentThreadId(), nIndex, air->FltNum, air->icao24, air->beacon) );
		}
	}
}

void ClearPASSURTracked(struct InstanceData* id, char* FltNum, int icao24, int beacon, int nFeedType)
{
	AIR **airp = NULL;	
	AIR *air = NULL;
	char newstr[FLTNUM_SIZE] = {0};
	struct row_index *row_index = NULL;
	int nIndex = GetIndex(id, id->m_pPassurIndexMap, FltNum, icao24, beacon, PASSUR);
	if(-1 != nIndex)
	{
		airp = GetAirP(id, 0);
		if(airp && (air = airp[nIndex]))
		{
			switch(nFeedType){
			case ADSB:   air->IsTracked &= ~TBF_ADSB; break;
			case ASDEX:  air->IsTracked &= ~TBF_ASDEX; break;
			case MLAT:   air->IsTracked &= ~TBF_MLAT; break;
			// No other Feed Type should have an effect
			}

			D_CONTROL( "PASSUR_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Marking %s PASSUR[%d] Flight as NOT tracked(%d) from feed %s\n", 
				GetCurrentThreadId(), air->FltNum, nIndex, air->IsTracked, g_arFeedTypes[nFeedType]) );
		}
	}
}

// Clears ASD Tracks Tracked By PASSUR Feed
void ClearPASSURTrackedTracks( struct InstanceData* id )
{
	AIR** pairp;
	const DWORD cdwThreadId = GetCurrentThreadId();	
	
	// Clear ASD Feed
	pairp = GetASDAirP( id );
	ClearTrackedTracks(id, pairp, N_TRACK_ID, TBF_PASSUR);
	
	// Reset PASSUR Index Map
	ClearSavedIndices(id, id->m_pPassurIndexMap, PASSUR);

	D_CONTROL("PASSUR_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Cleared ASD tracks tracked by PASSUR\n", cdwThreadId));
}


void DumpAllPassurTrackedSettings( struct InstanceData* id, const char* pfn )
{
	AIR **pairp;
	AIR *pair;
	int i;
	const DWORD cdwThreadId = GetCurrentThreadId();

	if(id->m_sCurLayout.m_bShowPassurAircraft){
		pairp = GetAirP( id, 0 );

		for (i = 0; pairp && i < N_PASSUR_TRACK_ID; i++ ){
			if ( !(pair = pairp[i]) ){
				continue;
			}
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) %s: Flight %s, tracked(%d) for feed %s[%d]\n", cdwThreadId, pfn, pair->FltNum, pair->IsTracked, g_arFeedTypes[PASSUR], i);
		}
	}
}

void DumpTrackedPassurTracks( struct InstanceData* id, const char* pfn )
{
	AIR **pairp;
	AIR *pair;
	int i;
	const DWORD cdwThreadId = GetCurrentThreadId();	

	if(id->m_sCurLayout.m_bShowPassurAircraft){
		pairp = GetAirP( id, 0 );

		for (i = 0; pairp && i < N_PASSUR_TRACK_ID; i++ ){
			if ( !(pair = pairp[i]) ){
				continue;
			}
			if(pair->IsTracked){
				vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) %s: Flight %s, tracked(%d) for feed %s[%d]\n", cdwThreadId, pfn, pair->FltNum, pair->IsTracked, g_arFeedTypes[PASSUR], i);
			}
		}
	}
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Called from %s\n", cdwThreadId, pfn);

}


void MarkPassurTracked(struct InstanceData* id, char* FltNum, int icao24, int beacon, int nFeedType, struct feedstruct *record)
{
	AIR **airp = NULL;	
	AIR *air = NULL;
	char newstr[FLTNUM_SIZE] = {0};
	struct row_index *row_index = NULL;
	int nIndex = GetIndex(id, id->m_pPassurIndexMap, FltNum, icao24, beacon, PASSUR);
	if(-1 != nIndex)
	{
		airp = GetAirP(id, 0);
		if(airp && (air = airp[nIndex]))
		{
			if(ADSB == nFeedType){
				MergeTracks(id, air, PASSUR, ADSB, NULL);
			}else if(ASDEX == nFeedType){				
				MergeTracks(id, air, PASSUR, ASDEX, record);
			}else if(MLAT == nFeedType){				
				MergeTracks(id, air, PASSUR, MLAT, record);
			}
			D_CONTROL( "PASSUR_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Marking %s PASSUR[%d] Flight as tracked(%d) for feed %s\n", 
				GetCurrentThreadId(), air->FltNum, nIndex, air->IsTracked, g_arFeedTypes[nFeedType]) );
		}
	}else{
		D_CONTROL( "PASSUR_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Ignoring Flight %s, Not found in PASSUR SearchVO\n", 
				GetCurrentThreadId(), FltNum) );
	}
}




// PURPOSE: Check ASD for PASSUR, ASDEX and ADSB/ Mark this tracks as being tracked.
void CkNewASDOverlap(struct InstanceData* id, AIR* airp)
{
	int nIndex = -1;
	char timebuf[64];
	const DWORD cdwThreadId = GetCurrentThreadId();
	
	// Check for new Flight
	if(airp){// && -1 == GetIndex(id, id->m_pASDIndexMap, airp->FltNum, airp->icao24, airp->beacon, ASDI)){
		// Add Track to Search VO
		AddIndex(id, id->m_pASDIndexMap, airp->FltNum, airp->icao24, airp->beacon, airp->trackid, ASDI);
	}

	// Check PASSUR, mark this ASD track as "Tracked"
	if(-1 != (nIndex = GetIndex(id, id->m_pPassurIndexMap, airp->FltNum, airp->icao24, airp->beacon, PASSUR))){
		AIR** pairp = GetAirP(id, 0);
		AIR* air = pairp[nIndex];
		if(air){
			if ( !air->eta ){
				air->eta = airp->eta;
				D_CONTROL("ASD_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) adding eta %s for '%s', '%s'\n",
						cdwThreadId, VOTimeFmt(timebuf, airp->eta, "%H:%M:%S"), airp->FltNum, air->FltNum));
			}
			if ( !air->speed ){
				// no speed for passur, so give it asd speed
				air->speed = airp->speed;
				D_CONTROL("ASD_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) adding speed=%d for '%s', '%s'\n",
						cdwThreadId, airp->speed, airp->FltNum, air->FltNum));
			}
		}
		airp->IsTracked |= TBF_PASSUR;
		D_CONTROL( "ASD_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Marking %s ASD[%d] Flight as tracked(%d) from feed %s\n", 
			GetCurrentThreadId(), airp->FltNum, nIndex, airp->IsTracked, g_arFeedTypes[PASSUR]) );		
	}
	
	// Check MLAT, mark this ASD track as "Tracked"
	if(-1 != (nIndex = GetIndex(id, id->m_pMLATIndexMap, airp->FltNum, airp->icao24, 0, MLAT))){
		airp->IsTracked |= TBF_MLAT;
		D_CONTROL( "ASD_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Marking %s ASD[%d] Flight as tracked(%d) from feed %s\n", 
				GetCurrentThreadId(), airp->FltNum, nIndex, airp->IsTracked, g_arFeedTypes[MLAT]) );
	}

	// Check ASDEX, mark this ASD track as "Tracked"
	if(-1 != (nIndex = GetIndex(id, id->m_pASDEXIndexMap, airp->FltNum, airp->icao24, 0, ASDEX))){
		airp->IsTracked |= TBF_ASDEX;
		D_CONTROL( "ASD_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Marking %s ASD[%d] Flight as tracked(%d) from feed %s\n", 
				GetCurrentThreadId(), airp->FltNum, nIndex, airp->IsTracked, g_arFeedTypes[ASDEX]) );
	}

	// Check ADSB, mark this ASD track as "Tracked"
	if(-1 != (nIndex = GetIndex(id, id->m_pADSBIndexMap, airp->FltNum, airp->icao24, airp->beacon, ADSB))){
		airp->IsTracked |= TBF_ADSB;
		D_CONTROL( "ASD_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Marking %s ASD[%d] Flight as tracked(%d) from feed %s\n", 
				GetCurrentThreadId(), airp->FltNum, nIndex, airp->IsTracked, g_arFeedTypes[ADSB]) );
	}	
}

void ClearASDTracked(struct InstanceData* id, char* FltNum, int icao24, int beacon, int nFeedType)
{
	AIR **airp = NULL;	
	AIR *air = NULL;
	//char newstr[FLTNUM_SIZE] = {0};
	struct row_index *row_index = NULL;
	int nIndex = GetIndex(id, id->m_pASDIndexMap, FltNum, icao24, beacon, ASDI);
	if(-1 != nIndex)
	{
		airp = GetASDAirP(id);
		if(airp && (air = airp[nIndex]))
		{
			switch(nFeedType){
			case ADSB:   air->IsTracked &= ~TBF_ADSB; break;
			case ASDEX:  air->IsTracked &= ~TBF_ASDEX; break;
			case MLAT:   air->IsTracked &= ~TBF_MLAT; break;
			case PASSUR: air->IsTracked &= ~TBF_PASSUR; break;
			}
			D_CONTROL( "ASD_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Marking %s ASD[%d] Flight as NOT tracked(%d) for feed %s\n", 
				GetCurrentThreadId(), air->FltNum, nIndex, air->IsTracked, g_arFeedTypes[nFeedType]) );
		}
	}	
}

void DumpTrackedASDTracks( struct InstanceData* id, const char* pfn )
{
	AIR **pairp;
	AIR *pair;
	int i;
	const DWORD cdwThreadId = GetCurrentThreadId();	

	if(id->m_sCurLayout.m_bShowPassurAircraft){
		pairp = GetASDAirP( id );

		for (i = 0; pairp && i < N_TRACK_ID; i++ ){
			if ( !(pair = pairp[i]) ){
				continue;
			}
			if(pair->IsTracked){
				vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) %s: Flight %s, tracked(%d) for feed %s[%d]\n", cdwThreadId, pfn, pair->FltNum, pair->IsTracked, g_arFeedTypes[ASDI], i);
			}
		}
	}
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Called from %s\n", cdwThreadId, pfn);

}

void MergeTracks(struct InstanceData* id, AIR* air, int nFeedType, int nCopyToFeedType, struct feedstruct *record)
{
	AIR **airp2 = NULL;	
	AIR *air2 = NULL;
	int bTailCur = FALSE;
	int bTailNext = FALSE;
	int nIndex = -1;
	if( ADSB == nCopyToFeedType ){
		nIndex = GetIndex(id, id->m_pADSBIndexMap, air->FltNum, air->icao24, air->beacon, ADSB);
		if( -1 == nIndex ) return;
		air->IsTracked |= TBF_ADSB;
		airp2 = GetADSBAirP(id);
	}else if(ASDEX == nCopyToFeedType){
		if(ASDEX == nFeedType || MLAT == nFeedType)
			nIndex = GetIndex(id, id->m_pASDEXIndexMap, air->FltNum, air->icao24, air->beacon, ASDEX);
		else
			nIndex = GetIndex(id, id->m_pASDEXIndexMap, air->FltNum, air->icao24, 0, ASDEX);
		if( -1 == nIndex ) return;
		air->IsTracked |= TBF_ASDEX;
		airp2 = GetASDEXAirP(id);
	}else if(MLAT == nCopyToFeedType){
		if(ASDEX == nFeedType || MLAT == nFeedType)
			nIndex = GetIndex(id, id->m_pMLATIndexMap, air->FltNum, air->icao24, air->beacon, MLAT);
		else
			nIndex = GetIndex(id, id->m_pMLATIndexMap, air->FltNum, air->icao24, 0, MLAT);
		if( -1 == nIndex ) return;
		air->IsTracked |= TBF_MLAT;
		airp2 = GetMLATAirP(id);
	}else if(PASSUR == nCopyToFeedType){
		nIndex = GetIndex(id, id->m_pPassurIndexMap, air->FltNum, air->icao24, air->beacon, PASSUR);
		if( -1 == nIndex ) return;
		air->IsTracked |= TBF_PASSUR;
		airp2 = GetAirP(id, 0);
	}

	if(airp2 && (air2 = airp2[nIndex])){
		// We only copy if there are no existing tracks
		if(!air2->TrackPtr){
			int tcount = MIN(air->tcount, air->TrackStart);
			if( ADSB == nCopyToFeedType ){
				air2->TrackPtr = NewTrackArr(id, MAX( MIN(ADSB_TRACK_MODIFIER(id->m_sCurLayout.m_dAdsbTrailCount), ADSB_TRACK_MODIFIER(MAX_ADSB_TRACK_POINTS)) , 3 ), air2);
			}else if(ASDEX == nCopyToFeedType){
				air2->TrackPtr = NewTrackArr(id, MAX( MIN(ASDEX_TRACK_MODIFIER(id->m_sCurLayout.m_dAsdexTrailCount), ASDEX_TRACK_MODIFIER(MAX_ASDEX_TRACK_POINTS)) , 3 ), air2);
			}else if(MLAT == nCopyToFeedType){
				air2->TrackPtr = NewTrackArr(id, MAX( MIN(MLAT_TRACK_MODIFIER(id->m_sCurLayout.m_dMlatTrailCount), MLAT_TRACK_MODIFIER(MAX_MLAT_TRACK_POINTS)) , 5 ), air2);
			}else if(PASSUR == nCopyToFeedType){
				air2->TrackPtr = NewTrackArr(id, MAX( MIN(PASSUR_TRACK_MODIFIER(id->m_sCurLayout.m_nPassurTrailCount), PASSUR_TRACK_MODIFIER(MAX_PASSUR_TRACK_POINTS)) , 5 ), air2);
			}
			if(air2->max_tracks < air->max_tracks && tcount > air2->max_tracks){
				memcpy(air2->TrackPtr, air->TrackPtr + (tcount - air2->max_tracks), sizeof(PTRACK)* air2->max_tracks);
				air2->tcount = air2->max_tracks;
				air2->TrackStart = air2->max_tracks - 1;
			}else{
				memcpy(air2->TrackPtr, air->TrackPtr, sizeof(PTRACK)* tcount);
				air2->tcount = tcount;
				air2->TrackStart = air->TrackStart;
			}
			D_CONTROL( "MergeTrail", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Source %s[%d] %s tracks\n", GetCurrentThreadId(), g_arFeedTypes[nFeedType], air->trackid, air->FltNum));
			D_CONTROL( "MergeTrail1", PrintTracks( id, air, nFeedType ));
			D_CONTROL( "MergeTrail", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Dest %s[%d] %s tracks\n", GetCurrentThreadId(), g_arFeedTypes[nCopyToFeedType], nIndex, air2->FltNum));
			D_CONTROL( "MergeTrail1", PrintTracks( id, air2, nCopyToFeedType ));
		}else{
			D_CONTROL( "MergeTrail", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) NOT Copying tracks from Source %s[%d] to Dest %s[%d] for %s/%s.\n", GetCurrentThreadId(), 
				g_arFeedTypes[nFeedType], air->trackid, g_arFeedTypes[nCopyToFeedType], nIndex, air->FltNum, air2->FltNum));
		}

		// Merge Data
		if(!strlen(air2->FltNum) && strlen(air->FltNum))
			strcpy_s(air2->FltNum, FLTNUM_SIZE + 1, air->FltNum);
		else if(strlen(air2->FltNum) && !strlen(air->FltNum))
			strcpy_s(air->FltNum, FLTNUM_SIZE + 1, air2->FltNum);
		if(!strlen(air2->Origin) && strlen(air->Origin))
			strcpy_s(air2->Origin, O_D_SIZE + 1, air->Origin);
		if(!strlen(air2->Destin) && strlen(air->Destin))
			strcpy_s(air2->Destin, O_D_SIZE + 1, air->Destin);
		if(!strlen(air2->gate) && strlen(air->gate))
			strcpy_s(air2->gate, GATE_SIZE + 1, air->gate);
		if(!strlen(air2->runway) && strlen(air->runway))
			strcpy_s(air2->runway, RUNWAY_SIZE + 1, air->runway);
		if(!strlen(air2->actype) && strlen(air->actype))
			strcpy_s(air2->actype, ACTYPE_SIZE + 1, air->actype);
		// Should do all other data settings as well

	}
}

void ReverseMergeTracks(struct InstanceData* id, AIR* airDest, AIR* airSrc, int nDestFeedType, int nSrcFeedType)
{
	if(airDest && airSrc){
		// We only copy if there are no existing tracks
		if(!airDest->TrackPtr){
			int tcount = MIN(airSrc->tcount, airSrc->TrackStart);
			if( ADSB == nDestFeedType ){
				airDest->TrackPtr = NewTrackArr(id, MAX( MIN(ADSB_TRACK_MODIFIER(id->m_sCurLayout.m_dAdsbTrailCount), ADSB_TRACK_MODIFIER(MAX_ADSB_TRACK_POINTS)) , 3 ), airDest);
			}else if(ASDEX == nDestFeedType){
				airDest->TrackPtr = NewTrackArr(id, MAX( MIN(ASDEX_TRACK_MODIFIER(id->m_sCurLayout.m_dAsdexTrailCount), ASDEX_TRACK_MODIFIER(MAX_ASDEX_TRACK_POINTS)) , 3 ), airDest);
			}else if(MLAT == nDestFeedType){
				airDest->TrackPtr = NewTrackArr(id, MAX( MIN(MLAT_TRACK_MODIFIER(id->m_sCurLayout.m_dMlatTrailCount), MLAT_TRACK_MODIFIER(MAX_MLAT_TRACK_POINTS)) , 5 ), airDest);
			}else if(PASSUR == nDestFeedType){
				airDest->TrackPtr = NewTrackArr(id, MAX( MIN(PASSUR_TRACK_MODIFIER(id->m_sCurLayout.m_nPassurTrailCount), PASSUR_TRACK_MODIFIER(MAX_PASSUR_TRACK_POINTS)) , 5 ), airDest);
			}
			if(airDest->max_tracks < airSrc->max_tracks && tcount > airDest->max_tracks){
				memcpy(airDest->TrackPtr, airSrc->TrackPtr + (tcount - airDest->max_tracks), sizeof(PTRACK)* airDest->max_tracks);
				airDest->tcount = airDest->max_tracks;
				airDest->TrackStart = airDest->max_tracks - 1;
			}else{
				memcpy(airDest->TrackPtr, airSrc->TrackPtr, sizeof(PTRACK)* tcount);
				airDest->tcount = tcount;
				airDest->TrackStart = airSrc->TrackStart;
			}
			D_CONTROL( "MergeTrail", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Source %s[%d] %s tracks\n", GetCurrentThreadId(), g_arFeedTypes[nSrcFeedType], airSrc->trackid, airSrc->FltNum));
			D_CONTROL( "MergeTrail1", PrintTracks( id, airSrc, nSrcFeedType ));
			D_CONTROL( "MergeTrail", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Dest %s %s tracks\n", GetCurrentThreadId(), g_arFeedTypes[nDestFeedType], airDest->FltNum));
			D_CONTROL( "MergeTrail1", PrintTracks( id, airDest, nDestFeedType ));
		}else{
			// Determine how many to copy
			int nCopy = airSrc->tcount;
			if( nCopy > (airDest->max_tracks - airDest->tcount))
				nCopy = airDest->max_tracks - airDest->tcount;

			D_CONTROL( "MergeTrail", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) MERGE SRC: (%s,%X,%04o) TrackStart(%d), tcount(%d), max_tracks(%d)\n", GetCurrentThreadId(), 
				airSrc->FltNum, airSrc->icao24, airSrc->beacon, airSrc->TrackStart, airSrc->tcount, airSrc->max_tracks));
			D_CONTROL("MergeTrail1", PrintTracks(id, airSrc, nSrcFeedType));
			D_CONTROL( "MergeTrail", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) MERGE DEST: (%s,%X,%04o) TrackStart(%d), tcount(%d), max_tracks(%d)\n", GetCurrentThreadId(), 
				airDest->FltNum, airDest->icao24, airDest->beacon, airDest->TrackStart, airDest->tcount, airDest->max_tracks));
			D_CONTROL("MergeTrail1", PrintTracks(id, airDest, nDestFeedType));

			if ( nCopy ){
				// Move Existing Dest tracks to end
				memmove(&airDest->TrackPtr[nCopy], airDest->TrackPtr,  airDest->tcount * sizeof( PTRACK ));
				// Copy Source Tracks into Dest Array
				memcpy(airDest->TrackPtr, &airSrc->TrackPtr[airSrc->tcount - nCopy], nCopy * sizeof( PTRACK ));
				// Update Track Info
				airDest->tcount += nCopy;
				airDest->TrackStart += nCopy;
				D_CONTROL( "MergeTrail", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) MERGED DEST: (%s,%X,%04o)\n", GetCurrentThreadId(), 
					airDest->FltNum, airDest->icao24, airDest->beacon ) );
				D_CONTROL("MergeTrail1", PrintTracks(id, airDest, nDestFeedType));
			}

			/*PTRACK* pTemp = NULL;
			if( airSrc->tcount < (airSrc->max_tracks - airDest->tcount)){
				// Copy to end
				memcpy(airSrc->TrackPtr + sizeof(PTRACK)*airSrc->tcount, airDest->TrackPtr, sizeof(PTRACK)* airDest->tcount);				
				// Swap Track Arrays
				pTemp = airDest->TrackPtr;
				airDest->TrackPtr = airSrc->TrackPtr;
				airSrc->TrackPtr = pTemp;
				// Resize Destination Values
				airDest->tcount += airSrc->tcount;
			}else{
				// Copy to end
			}

			D_CONTROL( "MergeTrail", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) MERGE SRC: TrackStart(%d), tcount(%d), max_tracks(%d)\n", GetCurrentThreadId(), 
				airSrc->TrackStart, airSrc->tcount, airSrc->max_tracks));
			PrintTracks(id, airSrc, nSrcFeedType);
			D_CONTROL( "MergeTrail", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) MERGE DEST: TrackStart(%d), tcount(%d), max_tracks(%d)\n", GetCurrentThreadId(), 
				airDest->TrackStart, airDest->tcount, airDest->max_tracks));
			PrintTracks(id, airDest, nDestFeedType);
			
			D_CONTROL( "MergeTrail", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) MERGE Not Copying tracks from Source %s[%d] to Dest %s for %s/%s.\n", GetCurrentThreadId(), 
				g_arFeedTypes[nSrcFeedType], airSrc->trackid, g_arFeedTypes[nDestFeedType], airSrc->FltNum, airDest->FltNum));
				*/
		}
		// Merge Data
		if(!strlen(airDest->FltNum) && strlen(airSrc->FltNum))
			strcpy_s(airDest->FltNum, FLTNUM_SIZE + 1, airSrc->FltNum);
		if(!strlen(airDest->Origin) && strlen(airSrc->Origin))
			strcpy_s(airDest->Origin, O_D_SIZE + 1, airSrc->Origin);
		if(!strlen(airDest->Destin) && strlen(airSrc->Destin))
			strcpy_s(airDest->Destin, O_D_SIZE + 1, airSrc->Destin);
		if(!strlen(airDest->gate) && strlen(airSrc->gate))
			strcpy_s(airDest->gate, GATE_SIZE + 1, airSrc->gate);
		if(!strlen(airDest->runway) && strlen(airSrc->runway))
			strcpy_s(airDest->runway, RUNWAY_SIZE + 1, airSrc->runway);
		if(!strlen(airDest->actype) && strlen(airSrc->actype))
			strcpy_s(airDest->actype, ACTYPE_SIZE + 1, airSrc->actype);
	}
}

// Mark ASD Aircraft Tracked by Other Feed
void MarkASDTracked(struct InstanceData* id, char* FltNum, int icao24, int beacon, int nFeedType, struct feedstruct *record)
{
	AIR **airp = NULL;	
	AIR *air = NULL;
	//char newstr[FLTNUM_SIZE] = {0};
	//struct row_index *row_index = NULL;
	int nIndex = GetIndex(id, id->m_pASDIndexMap, FltNum, icao24, beacon, ASDI);
	if(-1 != nIndex)
	{
		airp = GetASDAirP(id);
		if(airp && (air = airp[nIndex]))
		{
			if(ADSB == nFeedType){
				MergeTracks(id, air, ASD, ADSB, NULL);
			}else if(ASDEX == nFeedType){
				MergeTracks(id, air, ASD, ASDEX, record);				
			}else if(MLAT == nFeedType){
				MergeTracks(id, air, ASD, MLAT, record);				
			}else if(PASSUR == nFeedType){			
				// Pulled from old PassASDOverlap to mark as tracked only if flight in PASSUR range
				if ( air->X && air->Y )
				{ 
					if( !( fabs( air->X - id->m_dLongitudes[0] ) > 3.0 ||
					       fabs( air->Y - id->m_dLatitudes[0] ) > 3.0 ) ){
						if(!(air->IsTracked & TBF_PASSUR)){
							MergeTracks(id, air, ASD, PASSUR, NULL);
						}
					}else{
						air->IsTracked &= ~TBF_PASSUR;
						D_CONTROL( "ASD_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) NOT Marking %s ASD[%d] Flight as tracked(%d) from feed %s, not in PASSUR range\n", 
						GetCurrentThreadId(), air->FltNum, nIndex, air->IsTracked, g_arFeedTypes[nFeedType]) );
					}
				}else{
					// X and Y values are not set, so mark as tracked as requested.
					air->IsTracked |= TBF_PASSUR;
				}
			}
			D_CONTROL( "ASD_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Marking %s ASD[%d] Flight as tracked(%d) from feed %s\n", 
				GetCurrentThreadId(), air->FltNum, nIndex, air->IsTracked, g_arFeedTypes[nFeedType]) );
		}
	}else{
		D_CONTROL( "ASD_TRACKS", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Ignoring Flight %s, Not found in ASD SearchVO\n", 
				GetCurrentThreadId(), FltNum) );
	}
}

