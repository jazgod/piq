#ifndef _FEEDSTRUCT_H_
#define _FEEDSTRUCT_H_

#ifdef __cplusplus 
extern "C"{
#endif 

#ifndef MXAPCHAR
#define MXAPCHAR 16
#endif//MXAPCHAR

#ifndef MAX_ROUTE_SIZE
#define MAX_ROUTE_SIZE 256
#endif//MAX_ROUTE_SIZE

#ifndef DEP_FIX_SIZE
#define DEP_FIX_SIZE 6
#endif//DEP_FIX_SIZE

#ifndef FLTNUM_SIZE
#define FLTNUM_SIZE 8
#endif//FLTNUM_SIZE

#ifndef ACTYPE_SIZE
#define ACTYPE_SIZE 6
#endif//ACTYPE_SIZE

#ifndef O_D_SIZE
#define O_D_SIZE 5
#endif//O_D_SIZE

#ifndef N_TAIL_SIZE
#define N_TAIL_SIZE 9
#endif//N_TAIL_SIZE

#ifndef RUNWAY_SIZE
#define RUNWAY_SIZE 30
#endif//RUNWAY_SIZE

#ifndef SRC_RADAR_SIZE
#define SRC_RADAR_SIZE 5
#endif//SRC_RADAR_SIZE

#ifndef CID_SIZE
#define CID_SIZE 4
#endif//CID_SIZE


struct feedstruct {
  int rectype;
  char airport[ MXAPCHAR + 1];
  time_t eventtime;
  time_t eta;
  time_t etd;
  unsigned beacon;
  int TrackID;
  int feedx;
  int feedy;
  int pointnum;
  int prevlng;  /* from asd only */
  int prevlat;  /* from asd only */
  time_t prevtime;
  char FltNum[ FLTNUM_SIZE + 1 ];
  char origin[ O_D_SIZE + 1 ];
  char destin[ O_D_SIZE + 1 ];
  int altitude;
  char actype[ ACTYPE_SIZE + 1];
  int speed;
  int velocity_x;
  int velocity_y;
  int latsecs;
  int longsecs;
  char qualifier;
  char src_radar[ SRC_RADAR_SIZE + 1];
  char cid[ CID_SIZE + 1];
  char route[ MAX_ROUTE_SIZE + 1];
  char *waypoints;
  char departure_fix[ DEP_FIX_SIZE + 1];
  char afix[ DEP_FIX_SIZE + 1 ];
  char tail[ N_TAIL_SIZE + 1];
  char runway[ RUNWAY_SIZE + 1 ];
  int icao24; // 24 bit mode s number
  int vehicle;
  float lat;
  float lng;
  float heading;
  int etatype;
  int etadist;
  int fob;
  int evnoddc; 
};

#ifdef __cplusplus 
}
#endif 

#endif//_FEEDSTRUCT_H_