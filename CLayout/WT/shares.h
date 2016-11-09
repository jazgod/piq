#ifndef _SHARES_H_
#define _SHARES_H_
/**
*==============================================================================
*
* Filename:         Shares.h
*
*==============================================================================
*
*  Copyright 2016  PASSUR Aerospace Inc. All Rights Reserved
*
*==============================================================================
*
* Description:
*    Message Shares Header
*
*==============================================================================
*
* Revision History:
*
* MCT 16/08/09 Created
*==============================================================================
*/

#include "InstanceData.h"

#define SD_SHARE  0
#define SD_EXPIRE 1
#define SD_UPDATE 2
#define SD_OTHER  3

#define SDT_TEST				0
#define SDT_FREE_FORM_TEXT		1
#define SDT_PERSISTANT_TARGET	2

#define UUID_SIZE	40

void CreateShare(struct InstanceData* id, int gid, int type, int stime, int etime, const char* data, char* uuid);
void ExpireShare(struct InstanceData* id, const char* uuid);
void LoadSharedMessages(struct InstanceData* id);
void SetFreeFormTextInfo(struct InstanceData* id, void* air_ptr, const char* uuid, int stime, int etime, int priv, const char* freeform);
void UpdateShare(struct InstanceData* id, const char* uuid, int gid, int etime, const char* data);

#endif//_SHARES_H_