#ifndef _FEEDS_H_
#define _FEEDS_H_

#ifdef __cplusplus 
extern "C"{
#endif//__cplusplus 

/**
 *==============================================================================
 *
 * Filename: hash_map_fns.h
 *
 *==============================================================================
 *
 *  Copyright 2013  PASSUR Aerospace Inc. All Rights Reserved
 *
 *==============================================================================
 *
 * Description:
 *    Support Functions to Manage Feed map to indice functions using cpp stl hash_map class
 *
 *==============================================================================
 * 
 * Revision History:
 *
 * MCT 1/29/13 Created
 *==============================================================================
 */
void  AddIndex(struct InstanceData* id, void* stlContainer, char *strFltNum, int icao24, int beacon, int nIndex, int nFeedType);
void  ClearSavedIndices(struct InstanceData* id, void* stlContainer, int nFeedType);
void* CreateIndexMap(struct InstanceData* id);
void  DestroyIndexMap(struct InstanceData* id, void* stlContainer);
int   GetIndex(struct InstanceData* id, void* stlContainer, char *strFltNum, int icao24, int beacon, int nFeedType);
void  RemoveIndex(struct InstanceData* id, void* stlContainer, char* strFltNum, int icao24, int beacon, int nFeedType);

#ifdef __cplusplus 
}
#endif//__cplusplus  

#endif//_FEEDS_H_