#ifndef _AC_TYPES_H_
#define _AC_TYPES_H_

#ifdef __cplusplus 
extern "C"{
#endif 

#define ACTYPE_LIGHT		0
#define ACTYPE_MEDIUM		1
#define ACTYPE_MEDIUM_ALT	2
#define ACTYPE_HEAVY		3

int GetAcType(struct InstanceData* id, const char* strAcType);
void LoadAcTypes(struct InstanceData* id);

#ifdef __cplusplus 
}
#endif

#endif//_AC_TYPES_H_