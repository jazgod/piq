/**
 *==============================================================================
 *
 * Filename:         InstanceData.c
 *
 *==============================================================================
 *
 *  Copyright 2011  PASSUR Aerospace Inc. All Rights Reserved
 *
 *==============================================================================
 *
 * Description:
 *    Shared Resource Data Structure Support Functions
 *
 *==============================================================================
 * 
 * Revision History:
 *
 * MCT 7/22/11 Created
 *==============================================================================
 */
#include <string.h>
#include "InstanceData.h"
#include "clist.h"
#include "vo.h"
#include "mgl.h"
#include "srfc.h"
#include "hash_map_fns.h"

void FreePolyList( polylist *flist );
typedef struct mapdatastruct{
	char shapeFilename[256];	// Unique name for each shape file data
	polylist *elemPolyList;		// Loaded Polygon list (may be NULL for Line-only shape files)
	GLuint elemListPolygon;		// Opengl call-list id for Polygons
	GLuint elemListLine;		// Opengl call-list id for Lines
	GLuint elemListPolygonClosed;	// Opengl call-list id for Polygons with Status Closed
	GLuint elemListLineClosed;		// Opengl call-list id for Lines with Status Closed
	HGLRC	   hRC;				// OpenGL Resource Context, for forced re-loading	
	VO* DBFVO;					// VO holding dbf shapefile data
	int    nStatus;				// Short-circuit status, 0 - Ok, 1 - Error
};


void InitializeInstanceData(struct InstanceData* id)
{
	// This handles null/0 values for entire structure
	memset(id, 0, sizeof(struct InstanceData));

	// Only need to initialize non NULL/0 values in this function
	// or call other initialization functions below this comment

	// Handles
	
	// Synch
	InitializeCriticalSection(&id->m_csSync);
	InitializeCriticalSection(&id->m_csASDSync);
	InitializeCriticalSection(&id->m_csPassurSync);
	InitializeCriticalSection(&id->m_csADSBSync);
	InitializeCriticalSection(&id->m_csAirAsiaSync);
	InitializeCriticalSection(&id->m_csMLATSync);
	InitializeCriticalSection(&id->m_csLMGSync);
	InitializeCriticalSection(&id->m_csASDEXSync);
	InitializeCriticalSection(&id->m_csPlanned);
	InitializeCriticalSection(&id->m_csEta);
	InitializeCriticalSection(&id->m_csArptRangeRing);
	InitializeCriticalSection(&id->m_csROI);
	InitializeCriticalSection(&id->m_csROIHist);
	InitializeCriticalSection(&id->m_csBlock);
	InitializeCriticalSection(&id->m_csNoiseSync);
	InitializeCriticalSection(&id->m_csAirportsVO);	
	InitializeCriticalSection(&id->m_csAirlineCodesVO);	
	InitializeCriticalSection(&id->m_csDBFSync);
	
	// Events
	id->m_evAirDataReset  = CreateEvent(NULL, TRUE, FALSE, NULL);
	id->m_evAirDrawReset  = CreateEvent(NULL, TRUE, FALSE, NULL);
	id->m_evPlannedReset  = CreateEvent(NULL, TRUE, FALSE, NULL);
	id->m_evRecVideoReset = CreateEvent(NULL, TRUE, FALSE, NULL);

	// Integers
	id->m_sCurLayout.m_dNoiseTrailCount = 1;
	id->m_sCurLayout.m_dAdsbTrailCount = 1;
	id->m_sCurLayout.m_nAirAsiaTrailCount = 5;
	id->m_sCurLayout.m_nAsdiTrailCount = 5;
	id->m_sCurLayout.m_nPassurTrailCount = 5;
	id->m_sCurLayout.m_dAsdexTrailCount = 1;
	id->m_sCurLayout.m_nAircraftSize = 12;
	id->m_nReplaySpeed = 5;
	id->m_nSelectedAirIndex = -1;
	id->m_nPassurRequestNum = 30;
	id->m_nASDEXRequestNum = 30;
	id->m_nADSBRequestNum = 30;
	id->m_nMLATRequestNum = 30;
	id->m_sCurLayout.m_nLayerEnableFlag = LEF_ALL;
	id->m_sCurLayout.m_nTagBorderEnableFlag = TB_ALL;
	id->m_nAdsbArpt = -1;
	id->m_nRegionAlertEnabled = TRUE;
	id->m_nRegionOTOYellow = 30 /* mins */ * 60 /* secs */;		// Default Region Out to Off Med Alert Threshold
	id->m_nRegionOTORed = 46 /* mins */ * 60 /* secs */;		// Default Region Out to Off High Alert Threshold
	id->m_nRegionOTIYellow = 16 /* mins */ * 60 /* secs */;		// Default Region On to In Med Alert Threshold
	id->m_nRegionOTIRed = 31 /* mins */ * 60 /* secs */;		// Default Region On to In High Alert Threshold
	id->m_nTarmacDelayEnabled = TRUE;
	id->m_nTarmacDelayOTOYellow = 30 /* mins */ * 60 /* secs */;	// Default Tarmac Delay Out to Off Med Alert Threshold
	id->m_nTarmacDelayOTORed = 46 /* mins */ * 60 /* secs */;		// Default Tarmac Delay Out to Off High Alert Threshold
	id->m_nTarmacDelayOTIYellow = 16 /* mins */ * 60 /* secs */;	// Default Tarmac Delay On to In Med Alert Threshold
	id->m_nTarmacDelayOTIRed = 31 /* mins */ * 60 /* secs */;		// Default Tarmac Delay On to In High Alert Threshold
	id->avilib.m_lRecDurSec = 600;  /* 10 minutes * 60 seconds */   // Default record duration in seconds (UI option)                      
	id->m_nMLATArpt = -1;
	id->m_nMaxTagFilters = 200;

	// Needs to be 4.5 hours and value is in seconds
	// ROI grid tool cannot handle 4.5 hours of data 
	id->m_nMaxRegionSecs = (int)(4.5 /*hours*/ * 60 /* mins */ * 60 /* secs */);
	//id->m_nMaxRegionSecs = (int)(1.0 /*hours*/ * 60 /* mins */ * 60 /* secs */);
	id->m_nMaxRegionTimeout = (int)(15 /* mins */ * 60 /* secs */);
	id->m_nHistRegionSecs = (int)(4.5 /*hours*/ * 60 /* mins */ * 60 /* secs */);
	//id->m_nHistRegionSecs = (int)(1.0 /*hours*/ * 60 /* mins */ * 60 /* secs */);
	id->m_nRegionTimeoutDelay = 120;

	// Flags
	id->m_bResetTracks = TRUE;
	id->m_sCurLayout.m_bShowWorldMap = TRUE;
	id->m_sCurLayout.m_bShowWorldBorders = TRUE;
	id->m_sCurLayout.m_bShowSmallTags = TRUE;
	id->m_bShowFlightID = TRUE; 
	id->m_sCurLayout.m_bShowAllAircraft = TRUE;
	id->m_sCurLayout.m_bShowASDAircraft = TRUE;
	id->m_sCurLayout.m_bShowPassurAircraft = TRUE; 
	id->m_bShowBorder = TRUE;
	id->m_sCurLayout.m_bShowTrails = TRUE;
	id->m_bFastUpdates = TRUE;
	id->m_bBuildRangeRings = TRUE;
	id->m_bASDFirstLoad = TRUE;
	
	// Time Values
	id->m_LastASDFillTags = 0;
	id->m_LastASDEXFillTags = 0;
	id->m_LastPassurFillTags = 0;
	
	// DWORDS
	id->m_dwAirDataLastBeat = (DWORD)-1;
	id->m_sCurLayout.m_dRingSize = 1.0;
	
	// DOUBLES
	
	// OpenGL
	
	// Strings or char*
	strcpy_s(id->m_strAirportMap, ARPTNAME_SZ, "NONE");
	strcpy_s(id->m_strPrevAirportMap, ARPTNAME_SZ, "NONE");
	strcpy_s(id->m_strHttpImagePath, FOLDERNAME_SZ, "surf/images");
	strcpy_s(id->m_strHttpMapPath, FOLDERNAME_SZ, "surf/maps");
	strcpy_s(id->m_strHttpFontPath, FOLDERNAME_SZ, "surf/fonts");
	strcpy_s(id->m_strHttpSoundPath, FOLDERNAME_SZ, "surf/sounds");	
	
	// Struct Pointers
	id->m_pASDIndexMap = CreateIndexMap(id);
	id->m_pPassurIndexMap = CreateIndexMap(id);
	id->m_pADSBIndexMap = CreateIndexMap(id);
	id->m_pASDEXIndexMap = CreateIndexMap(id);	
	id->m_pMLATIndexMap = CreateIndexMap(id);
}

void ReleaseInstanceData(struct InstanceData* id)
{
	int i = 0, j = 0, k =0;
	AIR ***Airs = NULL, **airp = NULL, *air = NULL;
	MAPLL *png = NULL, *png2 = NULL;
	struct clist_struct *mlist = NULL;
	struct mapdatastruct* mdata = NULL;
	CUSTOM_MAP_DATA* curr = NULL;
	CUSTOM_MAP_DATA* prev = NULL;
	// Structs
	
	// Handles
	DeleteCriticalSection(&id->m_csROIHist);
	DeleteCriticalSection(&id->m_csROI);
	DeleteCriticalSection(&id->m_csEta);	
	DeleteCriticalSection(&id->m_csADSBSync);
	DeleteCriticalSection(&id->m_csAirAsiaSync);
	DeleteCriticalSection(&id->m_csMLATSync);
	DeleteCriticalSection(&id->m_csASDEXSync);
	DeleteCriticalSection(&id->m_csLMGSync);
	DeleteCriticalSection(&id->m_csPassurSync);
	DeleteCriticalSection(&id->m_csASDSync);
	DeleteCriticalSection(&id->m_csSync);
	DeleteCriticalSection(&id->m_csNoiseSync);
	DeleteCriticalSection(&id->m_csAirportsVO);	
	DeleteCriticalSection(&id->m_csDBFSync);

	// Curl Handles
	if (id->m_pCurlHandle) curl_easy_cleanup(id->m_pCurlHandle);
	if (id->m_pCurlHandleASDEX) curl_easy_cleanup(id->m_pCurlHandleASDEX);
	id->m_pCurlHandle = NULL;
	id->m_pCurlHandleASDEX = NULL;

	// Allocated Pointers
	free(id->m_pActiveFilterPriorityIndices);
	id->m_pActiveFilterPriorityIndices = NULL;
	free(id->m_pASDfilebuf); 
	id->m_pASDfilebuf = NULL;
	free(id->m_pADSBFileBuf);
	id->m_pADSBFileBuf = NULL;
	free(id->m_pASDEXFileBuf);
	id->m_pASDEXFileBuf = NULL;
	free(id->m_pNoiseFileBuf);
	id->m_pNoiseFileBuf = NULL;
	free(id->m_pGTailBuf);
	id->m_pGTailBuf = NULL;
	free(id->m_pGRegions);
	id->m_pGRegions = NULL;
	free(id->m_pMLATFileBuf);
	id->m_pMLATFileBuf = NULL;
	

	for(i = 0; i < MAX_PASSURS; i++){
		if(id->m_pPassurfilebufs[i]){
			free(id->m_pPassurfilebufs[i]);
			id->m_pPassurfilebufs[i] = NULL;
		}
	}
	free(id->m_pstrPerms);
	id->m_pstrPerms = NULL;
	free(id->m_pstrLayouts);
	id->m_pstrLayouts = NULL;
	free(id->m_pFilters);
	id->m_pFilters = NULL;
	free(id->m_pTagFilters);
	id->m_pTagFilters = NULL;


	// Release Map structures
	if(id->m_sMapList){
		mlist = id->m_sMapList;
		if(clist_head(mlist)) {
			do {
				mdata = (struct mapdatastruct*)mlist->data;
				FreePolyList(mdata->elemPolyList);
				vo_free(mdata->DBFVO);
				free(mdata);
			} while(clist_next(mlist));
		}
		clist_free(id->m_sMapList);
		id->m_sMapList = NULL;
	}

	// Release Noise Air structs
	airp =  (AIR**)id->m_pNoiseAir;
	if(airp){
		for(j = 0; j < N_NOISE_TRACK_ID; j++){
			air = airp[j];
			if(air){
				if(air->TrackPtr) free(air->TrackPtr);
				if(air->FltRoute) free(air->FltRoute);
				if(air->FltRouteOrig) free(air->FltRouteOrig);
				free( air );
				airp[j] = NULL;
			}
		}
		free(airp);
		id->m_pNoiseAir = NULL;
	}

	// Release PASSUR Air structs
	Airs = id->m_pPASSURAirs;
	if(Airs){
		for(i = 0; i < 3; i++){
			if(Airs[i]){
				airp = Airs[i];
				if(airp){
					for(j = 0; j < N_PASSUR_TRACK_ID; j++){
						air = airp[j];
						if(air){
							if(air->TrackPtr) free(air->TrackPtr);
							if(air->FltRoute) free(air->FltRoute);
							if(air->FltRouteOrig) free(air->FltRouteOrig);
							free( air );
							airp[j] = NULL;
						}
					}
					free(airp);
					Airs[i] = NULL;
				}
			}
		}
	}
	free(id->m_pPASSURAirs);
	id->m_pPASSURAirs = NULL;

	// Release ASDEX Air Structs
	airp =  (AIR**)id->m_pASDEXAir;
	if(airp){
		for(j = 0; j < N_TRACK_ID; j++){
			air = airp[j];
			if(air){
				if(air->TrackPtr) free(air->TrackPtr);
				if(air->FltRoute) free(air->FltRoute);
				if(air->FltRouteOrig) free(air->FltRouteOrig);
				free( air );
				airp[j] = NULL;
			}
		}
		free(airp);
		id->m_pASDEXAir = NULL;
	}

	// Release ADSB Air Structs
	airp =  (AIR**)id->m_pADSBAir;
	if(airp){
		for(j = 0; j < N_ADSB_TRACK_ID; j++){
			air = airp[j];
			if(air){
				if(air->TrackPtr) free(air->TrackPtr);
				if(air->FltRoute) free(air->FltRoute);
				if(air->FltRouteOrig) free(air->FltRouteOrig);
				free( air );
				airp[j] = NULL;
			}
		}
		free(airp);
		id->m_pADSBAir = NULL;
	}

	// Release MLAT Air Structs
	airp =  (AIR**)id->m_pMLATAir;
	if(airp){
		for(j = 0; j < N_MLAT_TRACK_ID; j++){
			air = airp[j];
			if(air){
				if(air->TrackPtr) free(air->TrackPtr);
				if(air->FltRoute) free(air->FltRoute);
				if(air->FltRouteOrig) free(air->FltRouteOrig);
				free( air );
				airp[j] = NULL;
			}
		}
		free(airp);
		id->m_pMLATAir = NULL;
	}

	// Release ASDI Air Structs
	airp =  (AIR**)id->m_pASDAir;
	if(airp){
		for(j = 0; j < N_TRACK_ID; j++){
			air = airp[j];
			if(air){
				if(air->TrackPtr) free(air->TrackPtr);
				if(air->FltRoute) free(air->FltRoute);
				if(air->FltRouteOrig) free(air->FltRouteOrig);
				free( air );
				airp[j] = NULL;
			}
		}
		free(airp);
		id->m_pASDEXAir = NULL;
	}

	// Release Gate Air Structs
	airp =  (AIR**)id->m_pGateAir;
	if(airp){
		for(j = 0; j < N_TRACK_ID; j++){
			air = airp[j];
			if(air){
				if(air->TrackPtr) free(air->TrackPtr);
				if(air->FltRoute) free(air->FltRoute);
				if(air->FltRouteOrig) free(air->FltRouteOrig);
				free( air );
				airp[j] = NULL;
			}
		}
		free(airp);
		id->m_pGateAir = NULL;
	}

	// Release Region Info
	for(i = 0; i < id->m_nRegions; i++)
	{ 
		PREGION* reg = id->m_pRegions[i];
		free(reg->vertx);
		free(reg->verty);
		free(reg);
		reg = NULL;		
	}
	free(id->m_pRegions);
	id->m_pRegions = NULL;

	// Release MAPLL
	for ( png = id->m_pPngLL; png; png = png->np )
	{
		if(png2){
			free(png2);
			png2 = NULL;
		}
		png2 = png;
	}
	if(png2){
		free(png2);
		png2 = NULL;
	}

	// Free Custom Map Data
	curr = id->m_pCustomMapData;
	while(curr)
	{
		prev = curr;
		curr = curr->np;
		free(prev);
	}

	DestroyIndexMap(id, id->m_pASDIndexMap );
	DestroyIndexMap(id, id->m_pPassurIndexMap );
	DestroyIndexMap(id, id->m_pADSBIndexMap );
	DestroyIndexMap(id, id->m_pASDEXIndexMap );
	DestroyIndexMap(id, id->m_pMLATIndexMap );

	vo_free(id->m_pADSBBeaconVO);
	vo_free(id->m_pMLATBeaconVO);
	vo_free(id->m_pASDEXBeaconVO);
	vo_free(id->m_pRangeRingVO);
	vo_free(id->m_pTempRangeRingVO);
	vo_free(id->m_pETAVO);
	vo_free(id->m_pROIDataVO);
	vo_free(id->m_pROIVO);
	vo_free(id->m_pPlannedDepVO);
	vo_free(id->m_pPlannedArrVO);
	vo_free(id->m_pAirportsVO);
	vo_free(id->m_pTDVO);
	vo_free(id->m_pGateInVO);
	vo_free(id->m_pGateOutVO);

	// Unset pointers that do not need to be freed
	id->m_pSelectedAircraft = NULL;
}
