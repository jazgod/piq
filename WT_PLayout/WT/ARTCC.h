#ifndef _ARTCC_H_
#define _ARTCC_H_

#ifdef __cplusplus
extern "C"
{	
#endif

void DrawARTCCs( struct InstanceData* id );
const char* ARTCCtoJSON( struct InstanceData* id );

#ifdef __cplusplus
}

void LoadARTCCs( struct InstanceData* id );
void ReleaseARTCCs( struct InstanceData* id );

#endif
#endif//_ARTCC_H_