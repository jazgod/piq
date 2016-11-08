#ifndef _SURFACEC_H_
#define _SURFACEC_H_

//#include "InstanceData.h"

#ifdef __cplusplus 
extern "C"{
#endif 

typedef int  (*CnSendMessageCb)(struct InstanceData* v, char*);
typedef void (*CnGetDataCb)(struct InstanceData* v, char*, struct MemoryStruct*);

void Callback2Javascript(struct InstanceData* id, char* s);

int GetConfigValueInt(struct InstanceData* id, const char* strName, int nDefault);
int GetConfigValue(struct InstanceData* id, const char* strName, char* value, int len);

#ifdef __cplusplus 
}
#endif 

#endif//_SURFACEC_H_