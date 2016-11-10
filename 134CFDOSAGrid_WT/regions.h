#ifndef _REGIONS_H_
#define _REGIONS_H_

#ifdef __cplusplus 
extern "C"{
#endif 

void* CreateRegionVO(struct InstanceData* id, const char* strVoName);
unsigned _stdcall HistoricalROIDataThread(PVOID pvoid);
void CleanString(char* strData);

#ifdef __cplusplus 
}
#endif 

#endif//_REGIONS_H_