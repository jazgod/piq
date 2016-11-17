#include "shares.h"
#include "vo.h"
#include "mgl.h"
#include "srfc.h"
#include "Curl.h"
#include "servers.h"
#include "json.h"
#include "hash_map_fns.h"
#include <Rpc.h>
#pragma comment(lib, "Rpcrt4.lib")

int vo_log_info(va_alist);
int vo_log_error(va_alist);
int vo_log_warning(va_alist);


char* json_unescape_str(const char* str);
void ProcessFreeFormText(struct InstanceData* id, const char* data, const char* uuid, int stime, int etime);
void ProcessPersistantTarget(struct InstanceData* id, const char* data);
void ProcessSharedMessageData(struct InstanceData* id, const char* data);
void ProcessTestObject(struct InstanceData* id, const char* data);
void UUIDGen(struct InstanceData* id, char* uuid);
int ValidateJSONResult(struct InstanceData* id, const char* data);

void CreateShare(struct InstanceData* id, int gid, int type, int stime, int etime, const char* data, char* uuid)
{
	// Sample:"https://www54.passur.com/fcgi/Surf.fcg?Action=Share&prop=1&op=0&gid=0&rtype=0&etime=$EXP&data=%7B%20%22string%22%3A%20%22STRING%22%2C%20%22boolean%22%3A%20true%2C%20%22integer%22%3A%20347%2C%20%22double%22%3A%203%2E141590%20%7D"

	char tmpstr[8192];
	char *edata = NULL;
	struct MemoryStruct m;
	int op = SD_SHARE;
	int prop = 1;
	const DWORD cdwThreadId = GetCurrentThreadId();

	// Build Action String
	UUIDGen(id, uuid);
	edata = curl_escape(data, strlen(data));
	_snprintf_s(tmpstr, _countof(tmpstr), _TRUNCATE, "Action=Share&op=%d&prop=%d&usrname=%s&gid=%d&uuid=%s&rtype=%d&stime=%d&etime=%d&data=%s", op, prop, id->m_strUserName, gid, uuid, type, stime, etime, edata);
	curl_free(edata);

	clear_chunk_memory(&m);

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlFormPostResults - id->m_strHttpDBservername <%s> tmpstr <%s>.\n",
		cdwThreadId, id->m_strHttpDBservername[id->m_nDBServerIndex], tmpstr);

	if (!GetCurlFormPostResults(id, &m, id->m_strHttpDBservername[id->m_nDBServerIndex], tmpstr, SM_HTTPS, TRUE)) {
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to send new share\n", cdwThreadId);
		GetNxtSurfSrvr(id);
	}

	if (m.memory) {
		ValidateJSONResult(id, m.memory);
		free_chunk_memory(&m);
	}
}

void ExpireShare(struct InstanceData* id, const char* uuid)
{
	// Sample:"https://www54.passur.com/fcgi/Surf.fcg?Action=Share&prop=1&op=1&uuid=$UUID"
	char tmpstr[8192];
	struct MemoryStruct m;
	char *edata = NULL;
	int op = SD_EXPIRE;
	int prop = 1;
	const DWORD cdwThreadId = GetCurrentThreadId();

	_snprintf_s(tmpstr, _countof(tmpstr), _TRUNCATE, "Action=Share&op=%d&prop=%d&usrname=%s&uuid=%s", op, prop, id->m_strUserName, uuid);

	clear_chunk_memory(&m);

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlFormPostResults - id->m_strHttpDBservername <%s> tmpstr <%s>.\n",
		cdwThreadId, id->m_strHttpDBservername[id->m_nDBServerIndex], tmpstr);

	if (!GetCurlFormPostResults(id, &m, id->m_strHttpDBservername[id->m_nDBServerIndex], tmpstr, SM_HTTPS, TRUE)) {
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to send expired share msg\n", cdwThreadId);
		GetNxtSurfSrvr(id);
	}

	if (m.memory) {
		ValidateJSONResult(id, m.memory);
		free_chunk_memory(&m);
	}
}

char* json_unescape_str(const char* str){
	char* ustr = NULL;
	int l = 0;
	int i = 0;
	int p = 0;
	if (str && (l = strlen(str))){
		ustr = calloc(l + 1, sizeof(char));
		for (; i < l; i++){
			if ('\\' == str[i]){

				switch (str[++i]){
				case 'b': ustr[p++] = '\b';
					break;
				case 'n': ustr[p++] = '\n';
					break;
				case 'r': ustr[p++] = '\r';
					break;
				case 't': ustr[p++] = '\t';
					break;
				case '\\':
				case '/':
				case '"': ustr[p++] = str[i];
					break;
				}
			}
			else if ('\"' == str[i]){
				ustr[p++] = '"';
			}
			else {
				ustr[p++] = str[i];
			}
		}
	}
	return ustr;
}

void LoadSharedMessages(struct InstanceData* id)
{
	char tmpstr[8192];
	char *data = NULL;
	struct MemoryStruct m;
	const DWORD cdwThreadId = GetCurrentThreadId();

	clear_chunk_memory(&m);
	_snprintf_s(tmpstr, _countof(tmpstr), _TRUNCATE, "Action=GetShared&gid=%d", id->m_nGroupID);

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlFormPostResults - id->m_strHttpDBservername <%s> tmpstr <%s>.\n",
		cdwThreadId, id->m_strHttpDBservername[id->m_nDBServerIndex], tmpstr);

	if (!GetCurlFormPostResults(id, &m, id->m_strHttpDBservername[id->m_nDBServerIndex], tmpstr, SM_HTTPS, TRUE)){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to retrieve actions\n", cdwThreadId);
		GetNxtSurfSrvr(id);
	}
	if (m.memory){
		data = curl_unescape(m.memory, strlen(m.memory));
		free_chunk_memory(&m);

		ProcessSharedMessageData(id, data);

		curl_free(data);
	}
}

void ProcessFreeFormText(struct InstanceData* id, const char* data, const char* uuid, int stime, int etime)
{
	enum json_tokener_error jerror;
	struct json_object* jobj = NULL;
	const DWORD cdwThreadId = GetCurrentThreadId();

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Processing Free Form Text Share json_object: %s\n", GetCurrentThreadId(), data);

	if (data && strlen(data)){
		jobj = json_tokener_parse_verbose(data, &jerror);
		if (json_tokener_success == jerror){
			if (json_object_is_type(jobj, json_type_object)){
				struct json_object* jfield = NULL;
				int track_id = 0, icao24 = 0, beacon = 0, priv = 0, i = 0;
				char fltnum[FLTNUM_SIZE + 1] = { 0 };
				const char *freeform = 0;
				jfield = json_object_object_get(jobj, "flight_id");
				if (jfield && json_type_string == json_object_get_type(jfield))
					strcpy_s(fltnum, FLTNUM_SIZE + 1, json_object_get_string(jfield));
				jfield = json_object_object_get(jobj, "icao24");
				if (jfield && json_type_int == json_object_get_type(jfield))
					icao24 = json_object_get_int(jfield);
				jfield = json_object_object_get(jobj, "beacon");
				if (jfield && json_type_int == json_object_get_type(jfield))
					beacon = json_object_get_int(jfield);
				jfield = json_object_object_get(jobj, "track_id");
				if (jfield && json_type_int == json_object_get_type(jfield))
					track_id = json_object_get_int(jfield);
				jfield = json_object_object_get(jobj, "freeform");
				if (jfield && json_type_string == json_object_get_type(jfield))
					freeform = json_object_get_string(jfield);
				jfield = json_object_object_get(jobj, "private");
				if (jfield && json_type_boolean == json_object_get_type(jfield))
					priv = json_object_get_boolean(jfield);

				// Now Modify/Create FFINFOS for Air structs.
				for (i = 0; i < 5; i++) {
					AIR** airp = NULL;
					AIR* air = NULL;
					int idx = 0;
					switch (i) {
					case 0: 
					{	// TFM/ASDI
						if ((idx = GetIndex(id, id->m_pASDIndexMap, fltnum, icao24, beacon, ASD)) != -1) {
							airp = GetASDAirP(id);
							air = airp[idx];
						}
					}
					break;
					case 1:
					{	// PASSUR
						if ((idx = GetIndex(id, id->m_pPassurIndexMap, fltnum, icao24, beacon, PASSUR)) != -1) {
							airp = GetAirP(id, 0);
							air = airp[idx];
						}
					}
					break;
					case 2:
					{	// MLAT
						if ((idx = GetIndex(id, id->m_pMLATIndexMap, fltnum, icao24, beacon, MLAT)) != -1) {
							airp = GetMLATAirP(id);
							air = airp[idx];
						}
					}
					break;
					case 3:
					{	// ASDEX
						if ((idx = GetIndex(id, id->m_pASDEXIndexMap, fltnum, icao24, beacon, ASDEX)) != -1) {
							airp = GetASDEXAirP(id);
							air = airp[idx];
						}
					}
					break;
					case 4:
					{	// ADSB
						if ((idx = GetIndex(id, id->m_pADSBIndexMap, fltnum, icao24, beacon, ADSB)) != -1) {
							airp = GetADSBAirP(id);
							air = airp[idx];
						}
					}
					break;
					}
					if (air) {
						SetFreeFormTextInfo(id, air, uuid, stime, etime, priv, freeform);
					}
				}



			} else {
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Invalid initial json object, object is of type: %s\n", GetCurrentThreadId(), json_type_to_name(json_object_get_type(jobj)));
			}
			json_object_put(jobj);
		} else {
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to parse json data string: %s\n", GetCurrentThreadId(), data);
		}
	} else {
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) No data string from message\n", GetCurrentThreadId());
	}
}

void ProcessPersistantTarget(struct InstanceData* id, const char* data)
{
}

void ProcessSharedMessageData(struct InstanceData* id, const char* data)
{
	enum json_tokener_error jerror;
	struct json_object* jarr = NULL;
	const DWORD cdwThreadId = GetCurrentThreadId();

	if (data && strlen(data)){
		jarr = json_tokener_parse_verbose(data, &jerror);
		if (json_tokener_success == jerror){
			if (json_object_is_type(jarr, json_type_array)){
				struct json_object* jobj = NULL;
				struct json_object* jclass = NULL;
				int i = 0;
				int c = json_object_array_length(jarr);
				for (; i < c; i++){
					jobj = json_object_array_get_idx(jarr, i);
					jclass = json_object_object_get(jobj, "_class");
					if (jclass && !strcmp("Share", json_object_get_string(jclass))){
						const char* uuid = json_object_get_string(json_object_object_get(jobj, "uuid"));
						int gid = json_object_get_int(json_object_object_get(jobj, "gid"));
						int type = json_object_get_int(json_object_object_get(jobj, "type"));
						int stime = json_object_get_int(json_object_object_get(jobj, "stime"));
						int etime = json_object_get_int(json_object_object_get(jobj, "etime"));
						char* data = json_unescape_str(json_object_get_string(json_object_object_get(jobj, "data")));
						switch (type)
						{
						case SDT_TEST:	ProcessTestObject(id, data); break;
						case SDT_FREE_FORM_TEXT: ProcessFreeFormText(id, data, uuid, stime, etime); break;
						case SDT_PERSISTANT_TARGET: ProcessPersistantTarget(id, data); break;
						default:
							vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unknown share message type (%d) for json_object: %s\n", GetCurrentThreadId(), type, json_object_to_json_string(jobj));
						}
						free(data);
					}
					else{
						vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unexpected json object : %s\n", GetCurrentThreadId(), json_object_to_json_string(jobj));
					}
				}
			}
			else {
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unexpected json object type from parse data: %s\n", GetCurrentThreadId(), data);
			}
			json_object_put(jarr);
		}
		else {
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Error parsing expected json data: %s\n", GetCurrentThreadId(), data);
		}
	}
}

void ProcessTestObject(struct InstanceData* id, const char* data)
{
	enum json_tokener_error jerror;
	struct json_object* jobj = NULL;
	const DWORD cdwThreadId = GetCurrentThreadId();

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Processing Test Share json_object: %s\n", GetCurrentThreadId(), data);

	if (data && strlen(data)){
		jobj = json_tokener_parse_verbose(data, &jerror);
		if (json_tokener_success == jerror){
			if (json_object_is_type(jobj, json_type_object)){
				struct json_object_iter jiter;
				vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Processing Test json_object:\n", GetCurrentThreadId());
				json_object_object_foreachC(jobj, jiter){
					switch (json_object_get_type(jiter.val))
					{
					case json_type_boolean:
						vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) \t%s:%s(json_type_boolean)\n", GetCurrentThreadId(), jiter.key, (json_object_get_boolean(jiter.val) ? "true" : "false"));
						break;
					case json_type_double:
						vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) \t%s:%f(json_type_double)\n", GetCurrentThreadId(), jiter.key, json_object_get_double(jiter.val));
						break;
					case json_type_int:
						vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) \t%s:%d(json_type_int)\n", GetCurrentThreadId(), jiter.key, json_object_get_int(jiter.val));
						break;
					case json_type_object:
						break;
					case json_type_array:
						break;
					case json_type_string:
						vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) \t%s:%s(json_type_string)\n", GetCurrentThreadId(), jiter.key, json_object_get_string(jiter.val));
						break;
					}
				}
			}
		}
	}
}

void SetFreeFormTextInfo(struct InstanceData* id, void* air_ptr, const char* uuid, int stime, int etime, int priv, const char* freeform)
{
	AIR* air = (AIR*)air_ptr;
	int del = FALSE;
	if (etime <= (int)time(0))
		del = TRUE;

	if (air) {
		if (del) {
			if (air->FFTInfoPtr) {
				if (air->FFTInfoPtr->text) {
					free(air->FFTInfoPtr->text);
					air->FFTInfoPtr->text = NULL;
				}
				if (air->FFTInfoPtr->uuid) {
					free(air->FFTInfoPtr->uuid);
					air->FFTInfoPtr->uuid = NULL;
				}
				air->FFTInfoPtr = NULL;
			}
		} else {
			if (air->FFTInfoPtr) {
				if (air->FFTInfoPtr->text)
					free(air->FFTInfoPtr->text);
				if (air->FFTInfoPtr->uuid)
					free(air->FFTInfoPtr->uuid);
			} else {
				air->FFTInfoPtr = calloc(1, sizeof(FFTINFO));
			}
			air->FFTInfoPtr->stime = stime;
			air->FFTInfoPtr->etime = etime;
			air->FFTInfoPtr->priv = priv;
			air->FFTInfoPtr->text = _strdup(freeform);
			air->FFTInfoPtr->uuid = _strdup(uuid);
		}
	}
}

void UpdateShare(struct InstanceData* id, const char* uuid, int gid, int etime, const char* data)
{
	char tmpstr[8192];
	char *edata = NULL;
	struct MemoryStruct m;
	int op = SD_UPDATE;
	int prop = 1;
	const DWORD cdwThreadId = GetCurrentThreadId();

	// Build Action String
	edata = curl_escape(data, strlen(data));
	_snprintf_s(tmpstr, _countof(tmpstr), _TRUNCATE, "Action=Share&op=%d&prop=%d&usrname=%s&gid=%d&uuid=%s&etime=%d&data=%s", op, prop, id->m_strUserName, gid, uuid, etime, edata);
	curl_free(edata);

	clear_chunk_memory(&m);

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURL / GetCurlFormPostResults - id->m_strHttpDBservername <%s> tmpstr <%s>.\n",
		cdwThreadId, id->m_strHttpDBservername[id->m_nDBServerIndex], tmpstr);

	if (!GetCurlFormPostResults(id, &m, id->m_strHttpDBservername[id->m_nDBServerIndex], tmpstr, SM_HTTPS, TRUE)) {
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Failed to send new share\n", cdwThreadId);
		GetNxtSurfSrvr(id);
	}
	if (m.memory) {
		ValidateJSONResult(id, m.memory);
		free_chunk_memory(&m);
	}
}

void UUIDGen(struct InstanceData* id, char* struuid)
{
	UUID uuid;
	ZeroMemory(&uuid, sizeof(UUID));

	// Create uuid or load from a string by UuidFromString() function
	UuidCreate(&uuid);

	// If you want to convert uuid to string, use UuidToString() function
	CHAR* szUuid = NULL;
	UuidToStringA(&uuid, (RPC_CSTR*)&szUuid);
	if (szUuid != NULL)
	{
		strcpy_s(struuid, UUID_SIZE, szUuid);
		RpcStringFreeA((RPC_CSTR*)&szUuid);
		szUuid = NULL;
	}
}

int ValidateJSONResult(struct InstanceData* id, const char* data)
{
	int bValid = FALSE;
	struct json_object* jobj = NULL;
	enum json_tokener_error jerror;
	char* edata = NULL;
	const DWORD cdwThreadId = GetCurrentThreadId();

	if (data) {
		edata = curl_unescape(data, strlen(data));
		jobj = json_tokener_parse_verbose(edata, &jerror);
		if (jobj && json_tokener_success == jerror) {
			if (json_object_is_type(jobj, json_type_object)) {
				struct json_object* jmem = NULL;
				jmem = json_object_object_get(jobj, "_class");
				if (!jmem && strcmp("Result", json_object_get_string(jmem)))
					vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Invalid json object class returned, not a Result class: <%s>.\n", cdwThreadId, edata);
				else {
					jmem = json_object_object_get(jobj, "success");
					if (!jmem || !json_object_get_boolean(jmem)) {
						jmem = json_object_object_get(jobj, "description");
						vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Operation failed: <%s>.\n", cdwThreadId, json_object_get_string(jmem));
					} else {
						vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Share Operation Succeeded\n", cdwThreadId);
						bValid = TRUE;
					}
				}
			} else {
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Invalid json entity returned, expected <json_type_object>, received<%s> : <%s>.\n", cdwThreadId, json_type_to_name(json_object_get_type(jobj)), edata);
			}
			json_object_put(jobj);
		} else {
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to parse JSON result data: <%s>.\n", cdwThreadId, edata);
		}
	} else {
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) No data to validate.\n", cdwThreadId);
	}
	return bValid;
}