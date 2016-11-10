#ifndef _GATES_H_
#define _GATES_H_

#ifdef __cplusplus 
extern "C"{
#endif 

void DrawGateLabels( struct InstanceData* id );
int GetAirportGateData( struct InstanceData* id, const char* arpt, const char* term, const char* gate, const char* flightid, float* lat, float* lng, float* ang );
VO* GetGateLabelList(struct InstanceData* id, const char* arpt);
void LoadAirportGates( struct InstanceData* id );

#ifdef __cplusplus 
}
#endif

#endif//_GATES_H_