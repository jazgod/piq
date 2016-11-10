#ifndef __STRUCTS_H__
#define __STRUCTS_H__

/**
 *==============================================================================
 *
 * Filename:         structs.h
 *
 *==============================================================================
 *
 *  Copyright 2006  Megadata All Rights Reserved
 *
 *==============================================================================
 *
 * Description:
 *    Separate and define common structs used across all source
 *
 *==============================================================================
 * 
 * Revision History:
 *
 * 01-03-2012   M.Trujillo, Initial file creation
 * 03-15-2012   D.Migliorisi, Added the RegionOutlineColor struct
 *==============================================================================
 */

// LAYOUT Identifiers
#define LAYOUT_PASSUR_SIZE 5
#define LAYOUT_NAME_SIZE 128
#define CHAR24  24
#define TAG_LENGTH	256

// Layer Enable Flags
#define LEF_NONE				0x0000
#define LEF_AIRPORT_APRON		0x0001
#define LEF_AIRPORT_PARKING		0x0002
#define LEF_BUILDINGS			0x0004
#define LEF_CONSTRUCTION		0x0008
#define LEF_DEICING				0x0010
#define LEF_FREQUENCY			0x0020
#define LEF_MARKINGS			0x0040
#define LEF_ROADS				0x0080
#define LEF_RUNWAY				0x0100
#define LEF_SHOULDER			0x0200
#define LEF_TAXIWAY				0x0400
#define LEF_CUSTOM				0x0800
#define LEF_CLOSED				0x1000
#define LEF_ALL					0xFFFF

#define TB_NONE					0x00
#define TB_PASSUR				0x01
#define TB_ASDI					0x02
#define TB_ASDEX				0x04
#define TB_ADSB					0x08
#define TB_ALERTS				0x10
#define TB_MLAT					0x20
#define TB_AIRASIA				0x40
#define TB_GATE					0x80
#define TB_ALL					0xFF

// ROI color values
#define ROI_REGION_NAME_LEN 32
#define ROI_COLOR_LEN 7

typedef struct _region_outline_color {
    char region[ROI_REGION_NAME_LEN + 1];  // region name "RWY 16L 32R"
    char color[ROI_COLOR_LEN + 1];         // region outline color "#001122"
    GLfloat glColor[3];
} REGION_OUTLINE_COLOR;

typedef struct layoutstruct {
	char m_strLayoutName[ LAYOUT_NAME_SIZE ];	// Layout Name
	char m_strOwner[ CHAR24 ];	// Owner Name
	double m_dWorldXmin, m_dWorldXmax, m_dWorldYmin, m_dWorldYmax;// World in lat/lng coordinates	
	double m_dWorldXSize;		// World in lat/lng dimension
	double m_dWorldYSize;		// World in lat/lng dimension
	int IsActive;
	int m_bShowWeather;			// Show Weather
	int ShowArptNames;
	int m_bShowRunways;			// Show Runways
	int m_bShowSmallTags;		// Show Small Tags
	int m_bShowLargeTags;		// Show Large Tags	
	int ShowHighways;
	int ShowFlightID;
	char m_strPassurArpt[ LAYOUT_PASSUR_SIZE ];// 3 or 4 letter iata code, sometimes KJFK for international
	int m_bShowAircraftRings;	// Show Aircraft Rings
	double m_dRingSize;			// Aircraft Range Ring Radius	
	int m_bShowAirportRings;	// Show Airport Rings
	int m_nAirportRingSize;		// Airport Ring Radius
	int m_nAirportRingCount;	// Airport Ring Count
	char RangeRingNum[ CHAR24 ];
	char RangeRingDistance[ CHAR24 ];
	int m_bShowFAAStatus;		// Show FAA Status
	int m_bShowRoi;      		// Show Region of Interest	
	int m_bShowFltPlan;			// Show Flight Plan
	char m_strTagLine1[ TAG_LENGTH ];
	char m_strTagLine2[ TAG_LENGTH ];
	char m_strTagLine3[ TAG_LENGTH ];
	char m_strTagLine4[ TAG_LENGTH ];
	char m_strTagLine5[ TAG_LENGTH ];
	char m_strTagVLine1[ TAG_LENGTH ];
	char m_strTagVLine2[ TAG_LENGTH ];
	char m_strTagVLine3[ TAG_LENGTH ];
	int m_bShowAllAircraft;		// Show All Aircraft 
	int m_bShowADSBAircraft;	// Show ADSB Aircraft
	int m_bShowAirAsiaAircraft;	// Show AirAsia Aircraft
	int m_bShowASDAircraft;		// Show ASD Aircraft
	int m_bShowASDEXAircraft;	// Show ASDE-X Aircraft
	int m_bShowPassurAircraft;	// Show PASSUR Aircraft
	int m_bShowMLATAircraft;	// Show MLAT Aircraft
	int m_bShowWorldMap;		// Shows World Map
	int m_bShowWorldBorders;	// Shows Land/State Boundaries
	int ShowTerrain;
	int TerrainColor;
	int ShowAvitatOverlay;
	int ShowAirCellOverlay;
	int ShowAirCellRangeRings;
	int m_bShowCompression;
	int m_bShowTrails;			// Show Trails
	int m_bShowTrailTimes;		// Show Trail Times
	int m_nPassurTrailCount;	// Number of track points to plot for PASSUR Tracks
	double m_dAdsbTrailCount;	// Number of track points to plot for ADSB Tracks
	double m_dMlatTrailCount;	// Number of track points to plot for MLAT Tracks
	int m_nAsdiTrailCount;		// Number of track points to plot for ASDI Tracks
	double m_dAsdexTrailCount;	// Number of track points to plot for ASDE-X Tracks	
	double m_dNoiseTrailCount;		// Number of track points to plot for NOISE Tracks
	int m_nAirAsiaTrailCount;	// Number of track points to plot for AirAsia Tracks
	int m_bShowFixesH;			// Show High Fixes
	int m_bShowFixesL;			// Show Low Fixes
	int m_bShowFixesT;			// Show Terminal Fixes	
	int m_nLayerEnableFlag;		// Layer Enable Bit Flag (uses LEF_macros)
	int m_nTagBorderEnableFlag;	// Large Tag Border Enable Bit Flag (uses TB_macros)
	int m_nAircraftSize;		// Aircraft Icon Size
	int m_bShowNoiseAircraft;	// Show Noise Aircraft Data
	
	// Layout Colors
	GLfloat m_glfUSColor[3];
	GLfloat m_glfUSFillColor[3];
	GLfloat m_glfWaterColor[3];			
	GLfloat m_glfWaterFillColor[3];		
	GLfloat m_glfBuildingColor[3];
	GLfloat m_glfBuildingFillColor[3];
	GLfloat m_glfTaxiColor[3];
	GLfloat m_glfTaxiFillColor[3];
	GLfloat m_glfRunwayColor[3];
	GLfloat m_glfRunwayFillColor[3];
	GLfloat m_glfApronColor[3];
	GLfloat m_glfApronFillColor[3];
	GLfloat m_glfParkingColor[3];
	GLfloat m_glfParkingFillColor[3];
	GLfloat m_glfMarkingsColor[3];
	GLfloat m_glfMarkingsFillColor[3];
	GLfloat m_glfShoulderColor[3];
	GLfloat m_glfShoulderFillColor[3];
	GLfloat m_glfDeicingFillColor[3];
	GLfloat m_glfDeicingOutlineColor[3];
	GLfloat m_glfClosedFillColor[3];
	GLfloat m_glfClosedOutlineColor[3];
	GLfloat m_glfConstructionFillColor[3];
	GLfloat m_glfConstructionOutlineColor[3];
	GLfloat m_glfFrequencyFillColor[3];
	GLfloat m_glfFrequencyOutlineColor[3];
	GLfloat m_glfRoadFillColor[3];
	GLfloat m_glfRoadOutlineColor[3];
	GLfloat m_glfFixTermFillColor[3];
	GLfloat m_glfFixHighFillColor[3];
	GLfloat m_glfFixLowFillColor[3];
	GLfloat m_glfCustomFillColor[3];
	GLfloat m_glfCustomOutlineColor[3];
	
	// Tag Color Settings
	int m_nLargeTagTransparencyPASSUR;	// Background Transparency value for PASSUR Tags
	int m_nLargeTagTransparencyASDI;	// Background Transparency value for ASDI Tags
	int m_nLargeTagTransparencyASDEX;	// Background Transparency value for ASDE-X Tags	
	int m_nLargeTagTransparencyADSB;	// Background Transparency value for ADS-B Tags	
	int m_nLargeTagTransparencyMLAT;	// Background Transparency value for M-LAT Tags	
	int m_nLargeTagTransparencyAlerts;	// Background Transparency value for Alert Tags	
	int m_nLargeTagTransparencyAirAsia;	// Background Transparency value for AirAsia Tags
	int m_nLargeTagTransparencyGate;	// Background Transparency value for Gated Aircraft Tags
	GLfloat m_glfLargeTagTextColorPASSUR[3];
	GLfloat m_glfLargeTagBkColorPASSUR[3];
	GLfloat m_glfLargeTagTextColorASDI[3];
	GLfloat m_glfLargeTagBkColorASDI[3];
	GLfloat m_glfLargeTagTextColorASDEX[3];
	GLfloat m_glfLargeTagBkColorASDEX[3];
	GLfloat m_glfLargeTagTextColorADSB[3];
	GLfloat m_glfLargeTagBkColorADSB[3];
	GLfloat m_glfLargeTagTextColorMLAT[3];
	GLfloat m_glfLargeTagBkColorMLAT[3];
	GLfloat m_glfLargeTagTextColorAlerts[3];
	GLfloat m_glfLargeTagBkColorAlerts[3];
	GLfloat m_glfLargeTagTextColorAirAsia[3];
	GLfloat m_glfLargeTagBkColorAirAsia[3];
	GLfloat m_glfLargeTagTextColorGate[3];
	GLfloat m_glfLargeTagBkColorGate[3];

	int m_nARTCCLabel;
	int m_nGATESLabel;
	int m_nROISLabel;
	int m_nSIDSLabel;
	int m_nSTARSLabel;
	
	//font size index
	int m_nARTCCFontIndex;
	int m_nGATESFontIndex;
	int m_nFixesHFontIndex;			// High Fixes
	int m_nFixesLFontIndex;			// Low Fixes
	int m_nFixesTFontIndex;			// Terminal Fixes	
	int m_nROISFontIndex;
	int m_nSIDSFontIndex;
	int m_nSTARSFontIndex;

	GLfloat m_glfARTCCColor[3];
	GLfloat m_glfROISColor[3];
	GLfloat m_glfSIDSColor[3];
	GLfloat m_glfSTARSColor[3];
	GLfloat m_glfGATESColor[3];


	// Variable Settings
	char* m_pstrRangeRingArpts;
	char* m_pstrARTCC; //JSON String of ARTCC Regions (and Sectors of Regions)
	char* m_pstrSIDS; // String of SIDS to Display
	char* m_pstrSTARS; //String of STARS to Display

} LAYOUTS;

typedef struct pregions {
	char name[33];
	float *vertx;
	float *verty;
	int nvert;
} PREGION;


#endif//__STRUCTS_H__
