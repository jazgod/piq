#ifndef _VEHICLE_H_
#define _VEHICLE_H_

#ifdef __cplusplus 
extern "C"{
#endif 

void AddUserDefinedVehicle(struct InstanceData* id, int icao24, char* flightid, unsigned int trackid, void* obj);
int IsVehicle(struct InstanceData* id, int icao24, char* flightid, int trackid);
int GetVehicleData(struct InstanceData* id, int icao24, int trackid, VINFO* vInfo);
void LoadSurfaceVehicles(struct InstanceData* id );

#ifdef __cplusplus 
}
#endif

#endif//_VEHICLE_H_