#ifndef _TRACKING_H_
#define _TRACKING_H_

/**
 *==============================================================================
 *
 * Filename: TRACKING.h
 *
 *==============================================================================
 *
 *  Copyright 2013  PASSUR Aerospace Inc. All Rights Reserved
 *
 *==============================================================================
 *
 * Description:
 *    Support Functions to Manage Priority Tracks
 *
 *==============================================================================
 * 
 * Revision History:
 *
 * MCT 1/29/13 Created
 *==============================================================================
 */

// Reverse Priority Tracking Timeouts
#define RPT_ADSB_TIMEOUT	30
#define RPT_ASDEX_TIMEOUT	30
#define RPT_MLAT_TIMEOUT	5
#define RPT_PASSUR_TIMEOUT	30
#define RPT_ASDI_TIMEOUT	30

// ADSB Tracking Functions
void CkNewADSBOverlap(struct InstanceData* id, AIR* airp);
void ClearADSBTrackedTracks( struct InstanceData* id );

// ASDE-X Tracking Functions
void CkNewASDEXOverlap(struct InstanceData* id, AIR* airp, struct feedstruct *record);
void ClearASDEXTracked(struct InstanceData* id, char* FltNum, int icao24, int beacon, int nFeedType);
void ClearASDEXTrackedTracks( struct InstanceData* id );
void MarkASDEXTracked(struct InstanceData* id, char* FltNum, int icao24, int beacon );

// MLAT Tracking Functions
void CkNewMLATOverlap(struct InstanceData* id, AIR* airp, struct feedstruct *record);
void ClearMLATTracked(struct InstanceData* id, char* FltNum, int icao24, int beacon, int nFeedType);
void ClearMLATTrackedTracks( struct InstanceData* id );
void MarkMLATTracked(struct InstanceData* id, char* FltNum, int icao24, int beacon, int nFeedType, struct feedstruct *record );

// PASSUR Tracking Functions
void CkNewPassurOverlap(struct InstanceData* id, AIR* air);
void ClearPASSURTracked(struct InstanceData* id, char* FltNum, int icao24, int beacon, int nFeedType);
void ClearPASSURTrackedTracks( struct InstanceData* id );
void MarkPassurTracked(struct InstanceData* id, char* FltNum, int icao24, int beacon, int nFeedType, struct feedstruct *record);

// ASD(I) Tracking Functions
void CkNewASDOverlap(struct InstanceData* id, AIR* airp);
void ClearASDTracked(struct InstanceData* id, char* FltNum, int icao24, int beacon, int nFeedType);
void MarkASDTracked(struct InstanceData* id, char* FltNum, int icao24, int beacon, int nFeedType, struct feedstruct *record);

#endif//_TRACKING_H_