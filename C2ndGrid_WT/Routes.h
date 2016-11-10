#ifndef _ROUTES_H_
#define _ROUTES_H_

#ifdef __cplusplus 
extern "C"{
#endif 

void DrawSids( struct InstanceData* id );
void DrawStars( struct InstanceData* id );
const char* SIDtoJSON( struct InstanceData* id );
const char* STARtoJSON( struct InstanceData* id );

#ifdef __cplusplus 
}

void LoadDPs( struct InstanceData* id );
void LoadSTARs( struct InstanceData* id );

#endif 

#endif//_ROUTES_H_