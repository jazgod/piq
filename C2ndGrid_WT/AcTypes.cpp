#include "InstanceData.h"
#include "curl.h"
#include "json.h"
#include "AcTypes.h"

#include <hash_map>
#include <string>
using namespace std;
using namespace stdext;

extern "C"{
	extern int GetLocalFile( struct InstanceData* id, const char* localpath, const char* filename, const char* remotepath, int eMode);
	extern int IsDebugNameDefined(struct InstanceData* id, const char* strDebugName);
	extern int               vo_log_info( va_alist );
	extern int               vo_log_error( va_alist );
	extern int               vo_log_warning( va_alist );

	#define D_CONTROL(vcolname, exp) {if ( 5 == id->m_nDebugControl || (id->m_nDebugControl && IsDebugNameDefined(id, vcolname)) ) {exp;}}
	#define IS_D_CONTROL(vcolname) ( 5 == id->m_nDebugControl || (id->m_nDebugControl && IsDebugNameDefined(id, vcolname)) )
}

typedef stdext::hash_map<string, string> mapAcTypes;
typedef stdext::hash_map<string, string>::iterator mapAcTypesIter;

mapAcTypes g_mapLights;
mapAcTypes g_mapMediums;
mapAcTypes g_mapMediumAlts;
mapAcTypes g_mapHeavys;
mapAcTypes g_mapUnknowns;

void LoadAcTypeMap(struct InstanceData* id, mapAcTypes& m, struct json_object* jarr, const char* strAcGroup)
{
	const DWORD cdwThreadId = GetCurrentThreadId();
	if(jarr != NULL && json_object_is_type(jarr, json_type_array))
	{
		for(int i = 0; i < json_object_array_length(jarr); i++)
		{
			struct json_object* jelem = json_object_array_get_idx(jarr, i);
			m[string(json_object_get_string(jelem))] = string(json_object_get_string(jelem));
			D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Loading AcType %s for Group %s.\n", cdwThreadId, json_object_get_string(jelem), strAcGroup));
		}
	}else{
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to json object, json object not array!\n", cdwThreadId);
	}
}


extern "C"
{

int GetAcType(struct InstanceData* id, const char* strAcType)
{
	// Default Search is Medium, do not need to search medium because we use medium if not found as well.
	int nAcType = ACTYPE_MEDIUM;
	string sAcType(strAcType);
	const DWORD cdwThreadId = GetCurrentThreadId();
	if( strlen(strAcType) ){
		mapAcTypesIter iter = g_mapLights.find(sAcType);
		if( iter != g_mapLights.end() ){
			nAcType = ACTYPE_LIGHT;
			D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Found %s in Light Aircraft List!\n", cdwThreadId, strAcType));
		}else{
			iter = g_mapMediumAlts.find(sAcType);
			if( iter != g_mapMediumAlts.end() ){
				nAcType = ACTYPE_MEDIUM_ALT;
				D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Found %s in Medium Alternates Aircraft List!\n", cdwThreadId, strAcType));
			}else{
				iter = g_mapHeavys.find(sAcType);
				if( iter != g_mapHeavys.end() ){
					nAcType = ACTYPE_HEAVY;
					D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Found %s in Heavy Aircraft List!\n", cdwThreadId, strAcType));
				}else{
					if(IS_D_CONTROL("CAPTURE_MISSING_ACTYPES")){
						iter = g_mapMediums.find(sAcType);
						if( iter != g_mapMediums.end() ){
							D_CONTROL(__FUNCTION__, vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Found %s in Medium Aircraft List!\n", cdwThreadId, strAcType));
						}else{
							iter = g_mapUnknowns.find(sAcType);
							if( iter == g_mapUnknowns.end() ){
								g_mapUnknowns[sAcType] = sAcType;
								D_CONTROL(__FUNCTION__, vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Not Found %s using Medium Aircraft List!\n", cdwThreadId, strAcType));
								{
									string strFilename;
									struct json_object* jobj = json_object_new_object();
									struct json_object* jarr = json_object_new_array();
									mapAcTypesIter i = g_mapUnknowns.begin();
									while(i != g_mapUnknowns.end()){
										json_object_array_add(jarr, json_object_new_string(i->first.c_str()));
										i++;
									}
									json_object_object_add(jobj, "missing", jarr);
									strFilename = string("MissingAcTypes.json");
									strFilename = id->m_strMapDir + strFilename;
									if(json_object_to_file(const_cast<char*>(strFilename.c_str()), jobj)){
										vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to save missing actypes to file \"%s\"!\n", cdwThreadId, strFilename.c_str());
									}
									json_object_put(jobj);
								}
							}
						}
					}
				}
			}
		}
	}
	return nAcType;
}

void LoadAcTypes(struct InstanceData* id)
{
	const DWORD cdwThreadId = GetCurrentThreadId();
	string strFilename;
	struct json_object* jobj = NULL;
	struct json_object_iter jiter;
	strFilename = string("AcTypes.json");
	if( !GetLocalFile(id, id->m_strMapDir, strFilename.c_str(), id->m_strHttpMapPath, SM_HTTPS)  ){
		vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to find aircraft type data file(%s).\n", cdwThreadId, strFilename.c_str());
	}else{
		strFilename = id->m_strMapDir + strFilename;
		jobj = json_object_from_file(strFilename.c_str());
		if(jobj && -1 != ((int)jobj)){
			json_object_object_foreachC(jobj, jiter){
				if(!strcmp("light", jiter.key)){
					LoadAcTypeMap(id, g_mapLights, jiter.val, "light");
				}else if(!strcmp("medium", jiter.key)){
					LoadAcTypeMap(id, g_mapMediums, jiter.val, "medium");
				}else if(!strcmp("medium_alt", jiter.key)){
					LoadAcTypeMap(id, g_mapMediumAlts, jiter.val, "medium_alt");
				}else if(!strcmp("heavy", jiter.key)){
					LoadAcTypeMap(id, g_mapHeavys, jiter.val, "heavy");
				}
			}
		}
	}
	if(IS_D_CONTROL("CAPTURE_MISSING_ACTYPES")){
		strFilename = string("MissingAcTypes.json");
		strFilename = id->m_strMapDir + strFilename;
		jobj = json_object_from_file(strFilename.c_str());
		if(jobj && -1 != ((int)jobj)){
			json_object_object_foreachC(jobj, jiter){
				if(!strcmp("missing", jiter.key)){
					LoadAcTypeMap(id, g_mapUnknowns, jiter.val, "missing");
				}
			}
		}else{
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to load missing actypes from file \"%s\" (file does probably not exist)!\n", cdwThreadId, strFilename.c_str());
		}
	}
}

} // END - extern "C"


