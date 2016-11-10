// servers.h
// Implements servers.c related header info

#ifndef __SERVERS_H__
#define __SERVERS_H__

#define	SERVER_CURRENT		1
#define	SERVER_NEXT			2
#define	SERVER_PRIMARY		0
#define SERVER_REFRESH		3

#define SERVER_MAXLIST		32
#define SERVER_SWITCH_PNT	3

// Orlando Values
#define	SERVER_URL_PRIMARY	 "http://www50.passur.com/fcgi/PulseTrack.fcg"
#define	SERVER_URL_SECONDARY "http://www53.passur.com/fcgi/PulseTrack.fcg"

// Bohemia Values, not used since we will now pull "ALL" server lists from insight_servers table
// above values are for backup
//#define	SERVER_URL_PRIMARY	 "http://www3.passur.com/fcgi/PulseTrack.fcg"
//#define	SERVER_URL_SECONDARY "http://www11.passur.net/fcgi/PulseTrack.fcg"

#define SERVER_POSTS		"Action=GetInsightServers&username="
#define SERVER_POSTS_SZ		128

#define TAILS_SRVR_LIST_SZ	2
#define FTRACK_SRVR_LIST_SZ 2
#define WX_SRVR_LIST_SZ		2		// Number of servers on the list
#define	ADSB_SRVR_LIST_SZ	2		// Number of servers on the list
#define	AIRASIA_SRVR_LIST_SZ	1	// Number of servers on the list
#define	ASDEX_SRVR_LIST_SZ	6		// Number of servers on the list
#define	ASD_SRVR_LIST_SZ	4		// Number of servers on the list
#define	NOISE_SRVR_LIST_SZ	1		// Number of servers on the list
#define	MLAT_SRVR_LIST_SZ	1		// Number of servers on the list
#define	PASSUR_SRVR_LIST_SZ	4		// Number of servers on the list
#define LMG_SRVR_LIST_SZ	1

extern void	setinsightServerVars( struct InstanceData* id );
extern void	noteServerSuccess( struct InstanceData* id );
extern void	noteServerFailure( struct InstanceData* id );
extern void SetCrntFtrackSrvCnt0();
extern int  GetFTrackSrvCnt();
extern char* GetFTrackSrvr(struct InstanceData* id);
extern char* GetNxtFTrackSrvr(struct InstanceData* id);
extern char* GetTailsSrvr(struct InstanceData* id);
extern char* GetNxtTailsSrvr(struct InstanceData* id);
extern char* GetWxSrvr(struct InstanceData* id);
extern char* GetNxtWxSrvr(struct InstanceData* id);
extern char* GetNxtSurfSrvr(struct InstanceData* id);
extern char *GetAsdexSrvr(struct InstanceData* id);
extern char *GetNxtAsdexSrvr(struct InstanceData* id);
extern void NoteAsdexServerSuccess(struct InstanceData* id);
extern void NoteAsdexServerFailure(struct InstanceData* id);
extern char *GetAsdSrvr(struct InstanceData* id);
extern char *GetNxtAsdSrvr(struct InstanceData* id);
extern void NoteAsdServerSuccess(struct InstanceData* id);
extern void NoteAsdServerFailure(struct InstanceData* id);
extern char *GetNoiseSrvr(struct InstanceData* id);
extern char *GetNxtNoiseSrvr(struct InstanceData* id);

extern char *GetAdsbSrvr(struct InstanceData* id);
extern char *GetNxtAdsbSrvr(struct InstanceData* id);
extern void NoteAdsbServerSuccess(struct InstanceData* id);
extern void NoteAdsbServerFailure(struct InstanceData* id);

extern char *GetAirAsiaSrvr(struct InstanceData* id);
extern char *GetNxtAirAsiaSrvr(struct InstanceData* id);
extern void NoteAirAsiaServerSuccess(struct InstanceData* id);
extern void NoteAirAsiaServerFailure(struct InstanceData* id);

extern char *GetMlatSrvr(struct InstanceData* id);
extern char *GetNxtMlatSrvr(struct InstanceData* id);
extern void NoteMlatServerSuccess(struct InstanceData* id);
extern void NoteMlatServerFailure(struct InstanceData* id);

extern char *GetLMGSrvr(struct InstanceData* id);
extern char *GetNxtLMGSrvr(struct InstanceData* id);
extern void NoteLMGServerSuccess(struct InstanceData* id);
extern void NoteLMGServerFailure(struct InstanceData* id);

extern char *GetPassurSrvr(struct InstanceData* id);
extern char *GetNxtPassurSrvr(struct InstanceData* id);
extern void NotePassurServerSuccess(struct InstanceData* id);
extern void NotePassurServerFailure(struct InstanceData* id);

#endif /* __SERVERS_H__ */
