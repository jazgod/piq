#include "InstanceData.h"
#include "feedstruct.h"
#include "curl.h"
#include "json.h"

#include <fstream>
#include <hash_map>
#include <string>
using namespace std;
using namespace stdext;

extern "C"{
	// Must be same as in srfc.h
	#define DESCRIPTION_SIZE 35
	#define FREQUENCY_SIZE 10
	#define ID_SIZE 10
	#define SERNO_SIZE 10
	#define TAG_SIZE 10
	#define TYPE_SIZE 20
	// Must be same as in srfc.h
	typedef struct vehiclestruct {
		char Description[DESCRIPTION_SIZE];
		char Frequency[FREQUENCY_SIZE];
		char Id[ID_SIZE];
		char SerNo[SERNO_SIZE];
		char Tag[TAG_SIZE];
		char Type[TYPE_SIZE];
	} VINFO;

	extern int GetLocalFile( struct InstanceData* id, const char* localpath, const char* filename, const char* remotepath, int eMode);
	extern int IsDebugNameDefined(struct InstanceData* id, const char* strDebugName);
	extern int               vo_log_info( va_alist );
	extern int               vo_log_error( va_alist );
	extern int               vo_log_warning( va_alist );

	#define D_CONTROL(vcolname, exp) {if ( 5 == id->m_nDebugControl || (id->m_nDebugControl && IsDebugNameDefined(id, vcolname)) ) {exp;}}
	#define IS_D_CONTROL(vcolname) ( 5 == id->m_nDebugControl || (id->m_nDebugControl && IsDebugNameDefined(id, vcolname)) )
}
class Vehicle
{
public:
	// Constructors
	Vehicle( ):
		m_nTrackId(0){ };
	Vehicle(char* strData):
		m_nTrackId(0)
	{ 
		int fld = 0;
		char* ctx = NULL;
		char* tok = NULL;
		tok = strtok_s(strData, ",", &ctx);
		while(tok){
			switch(fld++){
			case 0:
				sscanf_s( tok, "%x", &m_nIcao24 );
				break;
			case 1: m_strDescription = tok;
				break;
			case 2: m_strTag = tok;
				break;
			case 3: m_strTag2 = tok;
				break;
			case 4: m_strId = tok;
				break;
			case 5: m_strSerNo = tok;
				break;
			case 6: m_strType = tok;
				break;
			case 7: m_strFrequency = tok;
				break;
			}
			tok = strtok_s(NULL, ",", &ctx);
		}
	};

	Vehicle(struct json_object* jobj):
		m_nIcao24(0),
		m_nTrackId(0)
	{
		m_strId = json_object_get_string(json_object_object_get(jobj, "flight_id"));
		m_nIcao24 = json_object_get_int(json_object_object_get(jobj, "icao24"));
		m_nTrackId = json_object_get_int(json_object_object_get(jobj, "track_id"));
	}

	Vehicle(const Vehicle& rCopy ):
		m_nIcao24(rCopy.m_nIcao24),
		m_nTrackId(rCopy.m_nTrackId),
		m_strType(rCopy.m_strType),
		m_strDescription(rCopy.m_strDescription),
		m_strTag(rCopy.m_strTag),
		m_strTag2(rCopy.m_strTag2),
		m_strId(rCopy.m_strId),
		m_strSerNo(rCopy.m_strSerNo),
		m_strFrequency(rCopy.m_strFrequency)
	{ };
	// Accessors & Mutators
	unsigned long Icao24( ) const { return m_nIcao24; }
	void Icao24( unsigned long nIcao24 ) { m_nIcao24 = nIcao24; }
	string Type( ) const { return m_strType; }
	void Type( string strType ) { m_strType = strType; }
	string Description( ) const { return m_strDescription; }
	void Description( string strDescription ) { m_strDescription = strDescription; }
	string Frequency( ) const { return m_strFrequency; }
	void Frequency( string strFrequency ) { m_strFrequency = strFrequency; }
	string Tag( ) const { return m_strTag; }
	void Tag( string strTag ) { m_strTag = strTag; }
	string Tag2( ) const { return m_strTag2; }
	void Tag2( string strTag2 ) { m_strTag2 = strTag2; }
	string Id( ) const { return m_strId; }
	void Id( string strId ) { m_strId = strId; }
	string SerNo( ) const { return m_strSerNo; }
	void SerNo( string strSerNo ) { m_strSerNo = strSerNo; }
	unsigned long TrackId( ) const { return m_nTrackId; }
	void TrackId( unsigned long nTrackId ) { m_nTrackId = nTrackId; }
	

private:
	unsigned long m_nIcao24;
	unsigned long m_nTrackId;
	string m_strType;
	string m_strDescription;
	string m_strTag;
	string m_strTag2;
	string m_strId;
	string m_strSerNo;
	string m_strFrequency;
};

typedef stdext::hash_map<unsigned int, Vehicle*> mapGroundVehicles;
typedef stdext::hash_map<unsigned int, Vehicle*>::iterator mapGroundVehiclesIter;
typedef stdext::hash_map<string, mapGroundVehicles*> mapVehiclesByAirport;
typedef stdext::hash_map<string, mapGroundVehicles*>::iterator mapVehiclesByAirportIter;

mapVehiclesByAirport g_mapAirportVehicles;

void FreeVehicleMap(struct InstanceData* id)
{
	/*if(id->m_pVehicleData){
		delete id->m_pVehicleData;
		id->m_pVehicleData = NULL;
	}*/
}

int GetVehicle(struct InstanceData* id, int icao24, Vehicle** pVehicle){
	const DWORD cdwThreadId = GetCurrentThreadId();
	int nSuccess = FALSE;
	string strAirport = string(id->m_strAsdexArpt, 0, 3);
	for(string::iterator i = strAirport.begin(); i != strAirport.end(); i++)
		(*i)=(char)toupper(*i);

	mapVehiclesByAirportIter m = g_mapAirportVehicles.end();
	if(g_mapAirportVehicles.end() != (m = g_mapAirportVehicles.find(strAirport))){ 
		mapGroundVehicles* v = m->second;	
	
		mapGroundVehiclesIter i = v->find(icao24);
		if(i != v->end()){
			*pVehicle = i->second;
			nSuccess = TRUE;
		}
	}
	return nSuccess;
}

int GetVehicle(struct InstanceData* id, int icao24, int trackid, Vehicle** pVehicle){
	const DWORD cdwThreadId = GetCurrentThreadId();
	int nSuccess = FALSE;
	Vehicle* pV = NULL;
	
	if(GetVehicle(id, icao24, &pV)){
		if(pV->TrackId()){
			if(pV->TrackId() == trackid){
				*pVehicle = pV;
				nSuccess = TRUE;
			}
		}else{
			*pVehicle = pV;
			nSuccess = TRUE;
		}
	}
	return nSuccess;
}

extern "C"
{
void AddUserDefinedVehicle(struct InstanceData* id, int icao24, char* flightid, unsigned int trackid, void* obj)
{
	struct json_object* jfobj = NULL, *jarr = NULL;
	struct json_object* jobj = (struct json_object*)obj;
	string strFilename;
	string strAirport;
	Vehicle* v = NULL;
	const DWORD cdwThreadId = GetCurrentThreadId();	

	strAirport = string(id->m_strAsdexArpt, 0, 3);
	for(string::iterator i = strAirport.begin(); i != strAirport.end(); i++){
		(*i)=(char)toupper(*i);
	}
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Adding %s with icao24(%X) to static %s user defined vehicles list.\n", cdwThreadId, flightid, icao24, strAirport.c_str());

	// Check if vehicle exists and just needs to be updated
	if(GetVehicle(id, icao24, trackid, &v)){
		if(v)
			v->Id(flightid);
	}else{
		// Adding directly to known Vehicle with new Vehicle Object
		v = new Vehicle(jobj);
		mapVehiclesByAirportIter i = g_mapAirportVehicles.end();
		// Pull out Airport Map if we have one
		if(g_mapAirportVehicles.end() != (i = g_mapAirportVehicles.find(strAirport))){ 
			mapGroundVehicles* m = i->second;
			(*m)[v->Icao24()] = v;
		}else{
			// Create a new airport map on the fly
			mapGroundVehicles* m = new mapGroundVehicles;
			(*m)[v->Icao24()] = v;
			g_mapAirportVehicles[strAirport] = m;
		}
	}	
	
	// Removed, REASON: No Longer needed, some vehicles share icao24
	// Save Vehicle to data file
	if(IS_D_CONTROL("SAVE_VEHICLE_DATA")){
		strFilename = string(id->m_strCachedDir) + string("\\-") + strAirport + string("-.json");	
		
		jfobj = json_object_from_file(strFilename.c_str());
		if(jfobj && -1 != ((int)jfobj)){
			// Add json to file object and save
			jarr = json_object_object_get(jfobj, "vehicles");
			if(jarr && json_object_is_type(jarr, json_type_array))
			{
				json_object_array_add(jarr, jobj);
				json_object_to_file((char*)strFilename.c_str(), jfobj);
			}
		}else{
			// Create new file with
			jarr = json_object_new_array();
			json_object_array_add(jarr, jobj);

			jfobj = json_object_new_object();
			json_object_object_add(jfobj, "_class", json_object_new_string("vehicle"));
			json_object_object_add(jfobj, "vehicles", jarr);
			json_object_to_file((char*)strFilename.c_str(), jfobj);
		}
		json_object_put(jfobj);
	}
}

int IsVehicle(struct InstanceData* id, int icao24, char* flightid, int trackid)
{
	int bVehicle = FALSE;
	Vehicle* v = NULL;
	if(GetVehicle(id, icao24, &v)){
		if(v->TrackId()){
			if(v->TrackId() == trackid)
				bVehicle = TRUE;
		}else{
			bVehicle = TRUE;
		}
	}
	return bVehicle;
}

int GetVehicleDescription(struct InstanceData* id, int icao24, char* bufstr, int len){
	const DWORD cdwThreadId = GetCurrentThreadId();
	int bFound = FALSE;
	Vehicle* v = NULL;
	if(GetVehicle(id, icao24, &v)){
		if(v){			
			sprintf_s(bufstr, len, "%s", v->Description().c_str());
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Matched icao24(%X) to service data for Description<%s>.\n", cdwThreadId, icao24, bufstr);
			bFound = TRUE;			
		}
	}
	return bFound;
}

int GetVehicleFrequency(struct InstanceData* id, int icao24, char* bufstr, int len){
	const DWORD cdwThreadId = GetCurrentThreadId();
	int bFound = FALSE;
	Vehicle* v = NULL;
	if(GetVehicle(id, icao24, &v)){
		if(v){
			sprintf_s(bufstr, len, "%s", v->Frequency().c_str());
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Matched icao24(%X) to service data for Frequency<%s>.\n", cdwThreadId, icao24, bufstr);
			bFound = TRUE;
		}
	}
	return bFound;
}

int GetVehicleTag(struct InstanceData* id, int icao24, char* bufstr, int len){
	const DWORD cdwThreadId = GetCurrentThreadId();
	int bFound = FALSE;
	Vehicle* v = NULL;
	if(GetVehicle(id, icao24, &v)){
		if(v){
			sprintf_s(bufstr, len, "%s", v->Tag().c_str());
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Matched icao24(%X) to service data for Tag<%s>.\n", cdwThreadId, icao24, bufstr);
			bFound = TRUE;
		}
	}
	return bFound;
}

int GetVehicleId(struct InstanceData* id, int icao24, char* bufstr, int len){
	const DWORD cdwThreadId = GetCurrentThreadId();
	int bFound = FALSE;
	Vehicle* v = NULL;
	if(GetVehicle(id, icao24, &v)){
		if(v){
			sprintf_s(bufstr, len, "%s", v->Id().c_str());
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Matched icao24(%X) to service data for Vehicle ID<%s>.\n", cdwThreadId, icao24, bufstr);
			bFound = TRUE;
		}
	}
	return bFound;
}

int GetVehicleSerNo(struct InstanceData* id, int icao24, char* bufstr, int len){
	const DWORD cdwThreadId = GetCurrentThreadId();
	int bFound = FALSE;
	Vehicle* v = NULL;
	if(GetVehicle(id, icao24, &v)){
		if(v){
			sprintf_s(bufstr, len, "%s", v->SerNo().c_str());
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Matched icao24(%X) to service data for RANGR-G<%s>.\n", cdwThreadId, icao24, bufstr);
			bFound = TRUE;
		}
	}
	return bFound;
}


int GetVehicleType(struct InstanceData* id, int icao24, char* bufstr, int len){
	const DWORD cdwThreadId = GetCurrentThreadId();
	int bFound = FALSE;
	Vehicle* v = NULL;
	if(GetVehicle(id, icao24, &v)){
		if(v){
			sprintf_s(bufstr, len, "%s", v->Type().c_str());
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Matched icao24(%X) to service data for Type<%s>.\n", cdwThreadId, icao24, bufstr);
			bFound = TRUE;
		}
	}
	return bFound;
}

int GetVehicleData(struct InstanceData* id, int icao24, int trackid, VINFO* vInfo)
{
	const DWORD cdwThreadId = GetCurrentThreadId();
	int bFound = FALSE;
	Vehicle* v = NULL;
	if(GetVehicle(id, icao24, trackid, &v)){
		if(v){
			strncpy_s(vInfo->Description, DESCRIPTION_SIZE, v->Description().c_str(), _TRUNCATE);
			strncpy_s(vInfo->Frequency, FREQUENCY_SIZE, v->Frequency().c_str(), _TRUNCATE);
			strncpy_s(vInfo->Id, ID_SIZE, v->Id().c_str(), _TRUNCATE);
			strncpy_s(vInfo->SerNo, SERNO_SIZE, v->SerNo().c_str(), _TRUNCATE);
			strncpy_s(vInfo->Tag, TAG_SIZE, v->Tag().c_str(), _TRUNCATE);
			strncpy_s(vInfo->Type, TYPE_SIZE, v->Type().c_str(), _TRUNCATE);
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Matched icao24(%X) to Vehicle data<%s,%s>.\n", cdwThreadId, icao24, vInfo->Id, vInfo->Type);
			bFound = TRUE;
		}
	}
	return bFound;
}

void LoadUserDefinedSurfaceVehicles(struct InstanceData* id, string strAirport)
{
	struct json_object* jfobj = NULL, *jarr = NULL;
	const DWORD cdwThreadId = GetCurrentThreadId();
	string strFilename = string(id->m_strCachedDir) + string("\\-") + strAirport + string("-.json");
	
	jfobj = json_object_from_file(strFilename.c_str());
	if(jfobj && -1 != ((int)jfobj)){
		// Add json to file object and save
		jarr = json_object_object_get(jfobj, "vehicles");
		if(jarr && json_object_is_type(jarr, json_type_array))
		{
			int l = json_object_array_length(jarr);
			mapVehiclesByAirportIter i = g_mapAirportVehicles.end();
			mapGroundVehicles* m = NULL;
			// Pull out Airport Map if we have one
			if(g_mapAirportVehicles.end() != (i = g_mapAirportVehicles.find(strAirport))){ 
				m = i->second;
			}else{
				m = new mapGroundVehicles;
				g_mapAirportVehicles[strAirport] = m;
			}
			for(int i = 0; i < l; i++)
			{
				struct json_object* jobj = json_object_array_get_idx(jarr, i);
				Vehicle* v = new Vehicle(jobj);
				(*m)[v->Icao24()] = v;
			}
		}
	}
}

void LoadSurfaceVehicles(struct InstanceData* id)
{
	const DWORD cdwThreadId = GetCurrentThreadId();
	string strFilename;
	string strAirport;	

	strAirport = string(id->m_strAsdexArpt, 0, 3);
	for(string::iterator i = strAirport.begin(); i != strAirport.end(); i++){
		(*i)=(char)toupper(*i);
	}
	if(g_mapAirportVehicles.end() == g_mapAirportVehicles.find(strAirport)){ 
		strFilename = strAirport + string("SurfaceVehicle.dat");
		if( !GetLocalFile(id, id->m_strMapDir, strFilename.c_str(), id->m_strHttpMapPath, SM_HTTPS)  ){
			vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to find vehicle data file(%s).\n", cdwThreadId, strFilename.c_str());
		}else{
			mapGroundVehicles* m = new mapGroundVehicles;
			char strBuffer[200] = {0};
			strFilename = id->m_strMapDir + strFilename;
			ifstream fin(strFilename.c_str());
			while(fin.getline(strBuffer, 200)){
				if(strlen(strBuffer) && strBuffer[0] != '#'){
					Vehicle* v = new Vehicle(strBuffer);
					(*m)[v->Icao24()] = v;
				}
			}
			g_mapAirportVehicles[strAirport] = m;
		}
	}

	// Removed, No Longer needed, some vehicles share icao24
	if(IS_D_CONTROL("SAVE_VEHICLE_DATA")){
		LoadUserDefinedSurfaceVehicles(id, strAirport);
	}
}

} // END - extern "C"


