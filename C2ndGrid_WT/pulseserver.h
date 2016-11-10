// pulseserver.h
// Implements pulseserver.c related header info

#ifndef __PULSESERVER_H__
#define __PULSESERVER_H__

#define	PULS_SRVR_LIST_SZ	6		// Number of servers on the list

extern char *GetPulseSrvr(struct InstanceData* id);
extern char *GetNxtPulseSrvr(struct InstanceData* id);


#endif /* __PULSESERVER_H__ */
