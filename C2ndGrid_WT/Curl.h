#ifndef _CURL_H_
#define _CURL_H_

#include <curl/curl.h>
#include "InstanceData.h"

#ifdef __cplusplus 
extern "C"{
#endif 

enum SecureMode { SM_HTTP = 0, SM_HTTPS = 1 };

struct MemoryStruct {
  char *memory;
  size_t size;
};

struct curlPrxyOpt
{
	char		*name;				// curl option name
	CURLoption	 option;			// curl option parameter
	int			 type;				// curl option value type
};


struct curlCachedProxyOpt
{
	CURLoption option;
	int		   type;
	union
	{
		char*		str;
		int			val;
		CURLoption	opt;
	};
	struct curlCachedProxyOpt* next;
};

CURL* GetCurlHandle(struct InstanceData* id, enum SecureMode eMode);
int GetCurlPage( struct InstanceData* id, struct MemoryStruct *chunk, char *url, enum SecureMode eMode, int maxtries, unsigned long timeout);
int GetCurlPage_r(struct InstanceData* id, CURL* curl_handle, struct MemoryStruct *chunk, char *url, enum SecureMode eMode, int maxtries, unsigned long timeout);
int GetCurlFormGetResults( struct InstanceData* id, struct MemoryStruct *chunk,  char *url, char *postfields, enum SecureMode eMode, int logflag );
int GetCurlFormGetResults_r( struct InstanceData* id, CURL* curl_handle, struct MemoryStruct *chunk, char *url, char *postfields, enum SecureMode eMode, int logflag );
int GetCurlFormGetResultsHdrs_r( struct InstanceData* id, CURL* curl_handle, struct MemoryStruct *chunk, struct MemoryStruct *hdrs, char *url, char *postfields, enum SecureMode eMode, int logflag );
int GetCurlFormPostResults( struct InstanceData* id, struct MemoryStruct *chunk, char *url, char *postfields, enum SecureMode eMode, int logflag );
int IsProxyRequired(struct InstanceData* id);
int loadCurlProxyOpts (struct InstanceData* id, int* pnProxyRequired, int* pnProxyAuthenticates, int* pnProxySet, int* pnProxyUserPwdSet);
int saveCurlProxyOpts (struct InstanceData* id, int bEncrypt);
void setCurlTimeout(CURL* curl_handle, long timeout);

void clear_chunk_memory(struct MemoryStruct* chunk);
void free_chunk_memory(struct MemoryStruct* chunk);

#ifdef __cplusplus 
}
#endif 

#endif//_CURL_H_