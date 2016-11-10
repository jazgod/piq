#ifndef XTRAFMH		/*.27*/
static char xtridd[] = "@(#)xtrafm.51.h 090112 jac Copyright Megadata Corp";
/*Copyright 1990 Megadata Corp, Bohemia NY*/
/*xtrafm.51.h 090112 jac:U_DFIXUSE U_AFIXUSE
 *xtrafm.50.h 080929 jac:U_DIVERSION
 *xtrafm.49.h 071228 jac:F_ICAO24 F_LVNLMSK F_LVNLACT F_LVNLBAT F_LVNLEST
 *xtrafm.48.h 070302 jac:U_EDT U_SPACEFIC
 *xtrafm.47.h 060624 jac:U_ICAO25 U_KLMCCIS
 *xtrafm.46.h 050418-050419 jac:d_tailno U_ICAO24
 *xtrafm.45.h 040917 jac:more U_ messge types
 *xtrafm.44.h 040512 jac:d_fltsel
 *xtrafm.43.h 031208 jac:D_ANOALT D_DNOALT
 *xtrafm.42.h 031118 jac:d_fixtag ST_TAGNOALT
 *xtrafm.41.h 030612 jac:d_nexranim d_nexrcach
 *xtrafm.40.h 030514-030517 jac:d_maptypes MAP_LARG d_ashowfind d_rrang1
 *  d_htick1 d_pptitl1 d_agtitl1 d_agytitl1 MAP_XXX now bits
 *xtrafm.39.h 021109 jac:d_dotoday
 *xtrafm.38.h 020814 jac:d_nexrad
 *xtrafm.37.h 020306 jac:default TRYPLANE
 *xtrafm.36.h 020226 jac:d_rfeta
 *xtrafm.35.h 010816-010823 jac:t_vx t_vx ST_VELOCITY ST_SPEED
 *xtrafm.34.h 010404 jac:d_remtdata
 *xtrafm.33.h 010111-010227 jac:d_exceptions d_qhrcnt NFINDT
 *xtrafm.32.h 001020 jac:ST_FIXTAG ST_TAGMASK ST_TAG1
 *xtrafm.31.h 000531-000711 jac:t_dt & t_ptk in trinfo, t_sped, d_speed d_asdeta
 *  d_pssrdominate d_ntit
 *xtrafm.30.h 000313 jac:IX_CIRC CIRC_CPLAN
 *xtrafm.29.h 990429-990623 jac:F_SMARTS d_deprpt d_matrpt
 *xtrafm.28.h 981203-981209 jac:ST_ASDPNT NTIT=1000 d_asdtrk
 *xtrafm.27.h 981013 jac:move ifndef d_passur d_stations
 *xtrafm.26.h 981001 jac:HP-UX10.20
 *xtrafm.25.h 970318-970523 jac:d_preports d_remdirec d_printer
 *  SO_ASD=> NTRL=50 NTIT=8000 f_sid f_star f_afix
 *xtrafm.24.h 961118 jac:f_acdata
 *xtrafm.23.h 960504 jac:change colormap allocation
 *xtrafm.22.h 960320 jac:on linux
 *xtrafm.21.h 960202 jac:D_UALETA
 *xtrafm.20.h 960126 jac:d_asdtsiz
 *xtrafm.19.h 950711 jac:convert fltxpndr to fltnmbdtl
 *xtrafm.18.h 11/06/93 1910 jac:d_spacing d_altcut d_althi d_altlo
 *xtrafm.17.h 09/17/93 1730 jac:LABEL_CPLAN
 *xtrafm.16.h 04/03/93 1210 jac:color change, d_colorual d_coloruax d_doual
 * d_douax d_colormenu d_dataday d_datahr d_datahr1d d_download d_help d_socket
 *xtrafm.15.h 10/07/92 0900 jac:d_pfix d_holding d_baseleg d_minpfxrng
 *xtrafm.14.h 09/19/92 1650 jac:d_fnofsiz d_qhrfsiz d_rtcfsiz d_hstfsiz
 *  d_rdrtim
 *xtrafm.13.h 09/01/92 1200 jac:fix DEPSTOL F_LIVE d_smooth d_eta d_rate d_rconf
 *xtrafm.12.h 03/06/92 1300 jac:add valid times for flight numbers
 *xtrafm.11.h 01/25/92 1710 jac:add colors for tracks to dsparms
 *xtrafm.10.h 10/25/91 1510 jac:add extnmb to p_nmtd in psparms
 *xtrafm.09.h 10/16/91 1120 jac:d_noise, add psparms
 *xtrafm.08.h 09/19/91 1440 jac:d_tclass d_manufac
 *xtrafm.07.h 06/18/91 1700 jac:d_agytitle NTIT=400
 *xtrafm.06.h 05/22/91 1620 jac:d_ljout
 *xtrafm.05.h 04/26/91 0830 jac:LUNIT=16 => 280NM,c_spnt,c_flightno,s_dbstr
 * trga_d, 7 char flight #
 *xtrafm.04.h 03/28/91 0840 jac:t_alt=array
 *xtrafm.03.h 03/26/91 1820 jac:Merge back in +aggain,aglook
 *xtrafms.02.h 03/18/91 1110 jac:For Silicon Graphics
 *xtrafm.02.h 03/05/91 0820 jac:Add pipe from xtrafm to xtrdfil
 *xtrafm.01.h 01/23/91 1300 jac:Add Flight Numbs
 *xtrafm.00.h 11/19/90 0820 jac:Header for xtrafm
 */

#define XTRAFMH		/*.19*/
#define U_HEART  0      /*.35 usr message type for heartbeat*/
#define U_TARGET 1      /*.35 usr message type for target report*/
#define U_FLIGHT 2      /*.35 usr message type for flight info*/
#define U_TIMOUT 3      /*.35 usr message type for track has timed out*/
#define U_ETA    4	/*.42 usr message type for eta report*/
#define U_DEPART 5	/*.42 usr message type for departure report*/
#define U_RWYCFG 6	/*.42 usr message type for runway configuration*/
#define U_RATE   7	/*.42 usr message type for one minute arr/dep count*/
#define U_ASDPNT 9	/*.42 usr message type for asd target report*/
#define U_ASDS   10	/*.42 usr message type for selected raw asd messages*/
#define U_TEXT 11	/*.45 usr message type for text messages*/
#define U_AFPLAN 12	/*.45 usr message type for asd flight plan*/
#define U_AHB 13	/*.45 usr message type for asd heartbeat*/
#define U_MISSED 14	/*.45 usr message type for missed approach*/
#define U_HOLD 15	/*.45 usr message type for aircraft holding*/
#define U_CUMM 16	/*.45 usr message type for cumm count since 3AM*/
#define U_HRATE 17	/*.45 for hourly rates from 5, 15 & 60 min counts*/
#define U_FIRDA 18	/*.45 for firda flight info from klm amsterdam*/
#define U_ICAO24 19	/*.46 usr message type for icao24 info*/
#define U_ICAO25 20	/*.47 usr message type for all icao24 info*/
#define U_KLMCCIS 21	/*.47 usr message type for klmccis from amsterdam*/
#define U_EDT 22	/*.48 usr message type for est. depart. time*/
#define U_SPACEFIC 23	/*.48 usr message type for spacing effic. info*/
#define U_DIVERSION 24	/*.50 usr message type for flight diversion*/
#define U_DFIXUSE 25	/*.51 usr message type for usage of a departure fix*/
#define U_AFIXUSE 26	/*.51 usr message type for usage of an arrival fix*/
#define TP_PLANE 2	/*.23 number of overlay planes*/
#define MX_PLANE 8
#define TP_COLOR 16	/*.23 number of interfering colors*/
#define IX_BCK	0	/*.23 background*/
#define IX_FOR	1	/*.23 foreground also APT*/
#define IX_UAL	2
#define IX_UAX	3
#define IX_DEP	4
#define IX_ARR	5	/*.23 any non UAL (UAX) with a flight #*/
#define IX_OTH	6
#define IX_LABEL 7
#define IX_REGN	8
/*#define IX_MARK	9	/*.23 unused*/
#define IX_CIRC	9	/*.30 range circle*/
#define IX_MAP	10
#define IX_MAP0 IX_MAP
#define IX_MAP1	11
#define IX_MAP2	12
#define IX_MAP3	13
#define IX_MAP4	14
#define IX_MAP5	15
#define IX_STRK	16	/*.23 overlay == Selected Track also FND & TRK*/
#define IX_TRK	IX_STRK	/*.23 overlay*/
#define IX_TXT	17	/*.23 overlay == Text also BOX*/
#define IX_BOX	IX_TXT	/*.23 overlay*/

char *getfltno();	/*.13 for SGI*/
#if defined(IRIS) || defined(__linux__)		/*.22*/
typedef unsigned char uchar;	/*.13 for SGI*/
#endif /*.13 IRIS*/

#define MOTIF

#define FIXNMLEN 6	/*.19 max length of a fix name*/
#define FPENTLEN 16	/*.19 max length of a flight plan entry*/
#define PNTBIT	1	/*.16 from cdelim(), change the points*/
#define DBBIT	2	/*.16 from cdelim(), change the data block*/
#define DO_SMOOTH 1	/*.12 smooth the track*/

#define FN_MVALID 4*3600 /*.13 duration of validity of manual flight # */
#define FN_HDSTRT 600	/*.13 flight # valid 10 minutes before start of trk*/
#define FN_HDEND  600	/*.13 flight # valid 10 minutes after end of trk*/

#define APDAT "apdat."	/*.12 base for apdata filename*/
#define FSUFFIX ".ff"	/*.12 suffix for flat file*/
#define SFSUFFIX ".ffs"	/*.12 suffix for output sorted flat file*/
#define SUFFIX ".tf"	/*.12 suffix for track file*/
#define CSUFFIX ".tf.Z"	/*.12 suffix for compressed track file*/
/*#define DEPSTOL 18/*.12 max flight strip time before trk start on depart=sec*/
#define DEPSTOL 1800 /*.13 max flight strip time before trk strt on depart=sec*/
#define DEPETOL 900  /*.12 max flight strip time after trk end on depart=sec*/
#define ARVSTOL 2700 /*.12 max flight strip time before trk start on arriv=sec*/
#define ARVETOL 900  /*.12 max flight strip time after trk end on arriv=sec*/
#define NOVALT  -400	/*.12 most positive invalid altitude, meters*/
#define NMTMATCH 3	/*.12 max dx & dy (pixels) to match an NMT position*/
#define NALTPNT  7	/*.12 max number of points to use to determine alt*/
#define AGYMIN  -700	/*.12 minimum altitude on altitude graphmeters)*/
#define NMTOMETER 1852	/*.12 mult dist in NM by this => dist in meters*/
#define METERTOFT 3.28083	/*.12 mult dist in meters by this => in feet*/
#define MSTOKT  1.94384	/*.12 convert meter/sec to knots*/
#define NPTEA 3         /*.12 number of points to check for arrival/dep*/
#define MDOTSQR 4	/*.12 # dots/side of a square mark*/
#define MAXMISS 1000	/*.12 max time advances without finding a point*/
#define SMOOTHLAG 5	/*.12 number of lags to use in smoothing*/
#define MAXLAG  20	/*.12 maximum lag available for smoothing*/
#define MINPSMOOTH 8	/*.12 min points on a track to smooth it*/
#define ALTCRITA 700	/*.12 too far above gnd for arriv/dep (meters)*/
#define ALTCRITB 60	/*.12 too far below gnd for arriv/dep (meters)*/
#define SZCOLOR 24	/*.11 max chars in a color name*/
#define MAXNMT	50	/*.09 max number of NMTs*/
#define NNMTNAME 10	/*.09 max number of chars in the name of an NMT site*/
#define NUM 100			/*max number of arg_lst elements*/
#define LASTIME 0x7FFFFFFF	/*last Unix calendar time*/

#define NFINDT	100		/*.33 max number of tracks to find*/
/*.33#define NFINDT	20		/*max number of tracks to find*/
#define NXPOND 4096		/*number of different mode A codes*/
#define VISFLT 10		/*number of flights simultaneously visible*/
#define FLTNCH (NDBSTR-5)	/*max number of chars in a flight number*/
#define FLTTXTNCH NDBSTR	/*width of text entry for flight # */
#define BLACK	0
#define WHITE	7
#define RED	0
#define GREEN	0
#define BLUE	0
#define LIGHTGREEN	0
#define LIGHTBLUE	0
#define LIGHTCYAN	0
#define YELLOW	0
#define AND_PUT	0
#define COPY_PUT	0
#define OR_PUT	0
#define REDOFF	~RED & WHITE	/*AND_PUT this to turn off RED*/
#define BLUOFF	~BLUE & WHITE	/*AND_PUT this to turn off BLUE*/
#define LIGHTBLUOFF ~LIGHTBLUE & WHITE	/*AND_PUT this to turn off LIGHTBLUE*/
#define NDBSTR	12	/*number of chars include NULL for track data block*/
#define LUNIT	16	/*units (in meters) of t_x & t_y*/
#define MINTPNT	3	/*minimum observed points to plot*/
#ifdef SO_ASD		/*.25 batch processing ASD vs Passur correlation*/
#define NTRL	50	/*.25 number of history points needed for batch*/
#define NTIT	8000	/*.25 number of track info entries needed for batch*/
/*#define NTIT	10000	/*.25 number of track info entries needed for batch*/
#else			/*.25 SO_ASD*/
#define NTRL	200	/*number of history points*/
#define NTIT	1000	/*.28 number of track info entries*/
/*#define NTIT	400	/*.07number of track info entries*/
#endif			/*.25 SO_ASD*/
#define NARBS	65535	/*number of arb track numbers MUST power of 2 - 1*/
#define PCHEIGHT 480	/*VGA height*/
#define PCWIDTH	 640	/*VGA width*/
#define	VSPACE	16	/*vertical spacing of character rows*/
#define VFIRST	VSPACE	/*first line to use for track info*/
#define	HSPACE	10	/*horizontal spacing of character rows*/
#define	HWIDTH	NDBSTR*HSPACE /*width of an entry*/
#define TIMOUT	30	/*timeout period for a track*/
#define ST_FLIGHT 1	/*Not Used displaying flight number*/
#define ST_MARKED 2	/*flight number mark at head of track*/
#define ST_FINDT  4	/*.07 track find mark at head of track*/
#define ST_SELECT 8	/*.16 track is selected*/
#define ST_ASDPNT 0x10	/*.28 track is from ASD*/
#define ST_FIXTAG 0x20	/*.32 bit in t_status track tag (header) in fixed loc*/
#define ST_TAG1 0x40	/*.32 bit in t_status => track tag (header) loc*/
#define ST_TAG2 0x80	/*.32 bit in t_status => track tag (header) loc*/
#define ST_TAGMASK 0xC0	/*.32 bits in t_status => track tag (header) loc*/
#define ST_SMOOTHED 0x100	/*.32 bit in t_status => already smoothed*/
#define ST_DELAYED 0x200	/*.32 bit in t_status => already delayed*/
#define ST_VELOCITY 0x400	/*.35 bit in t_status => vx & vy valid*/
#define ST_SPEED 0x800	/*.35 bit in t_status => vx is speed*/
#define ST_TAGNOALT 0x1000	/*.42 bit in t_status => no alt in track tag*/
struct trinfo {		/*info about tracks*/
   int t_tkno;		/*arbitrary track number associated with this info*/
   int t_status;	/*track status*/
   ulong t_ctim;	/*update time*/
   int t_pnt;		/*point count on this track*/
   int t_id;		/*mode A code*/
   int t_hid;		/*horizontal position of identifier*/
   int t_vid;		/*vertical position of identifier*/
   int t_ntrl;		/*history: number of points*/
   int t_inpr;		/*history: index for next point received*/
   short t_x[NTRL];	/*history: x coords of points, units = 4 meters*/
   short t_y[NTRL];	/*history: y coords of points, units = 4 meters*/
   short t_alt[NTRL];	/*altitude in 100 ft*/
   short t_dt[NTRL];	/*.31 time since preceeding point*/
   short t_vx[NTRL];	/*.35 history: x component of velocity, m/s*/
   short t_vy[NTRL];	/*.35 history: y component of velocity, m/s*/
   struct scinfo *t_psc; /*pointer to screen info*/
   struct astrk *t_ptk;	/*.31 pntr to ASD track*/
   char t_dbstr[NDBSTR]; /*NUll term string to print at head*/
};
#define t_sped t_dt	/*.31 store speed (m/s) for ASD instead of dtime*/
/*.31#define t_ptk t_pnt /*.31 temp storage in trinfo for pntr to ASD track*/
struct scinfo {		/*info about screen*/
   int s_tkno;		/*arbitrary track number associated with this info*/
   struct trinfo *s_ptr; /*pointer to track info*/
   int s_vert;		/*vertical position on screen*/
   int s_hor;		/*horizontal position on screen*/
   char s_tid[2];	/*character used to identify track*/
};

#ifndef __hpux10	/*.26*/
#if defined(OLDMOTIF) || defined(__hpux)	/*.22*/
#define XtPointer caddr_t
#endif /*OLDMOTIF*/
#endif /*.26 __hpux10 */

#ifndef RBSIZE
#define RBSIZE	1024
#endif /*RBSIZE*/
#define P_CLR	1	/*clear the screen*/
#define P_QUIT	20	/*quit*/
#define P_DBLK	21	/*plot track IDs & alts at head of tracks (toggle)*/
#define P_DLST	22	/*ID & altitude list & letter at head (toggle)*/
#define P_TRAIL	23	/*set track trail length*/
#define P_MAP	24	/*plot map (toggle on/off)*/
#define P_RNGC	25	/*plot range circles (toggle on/off)*/
#define P_STOP	26	/*Quit command, just accept only P_QUIT commands*/
#define P_GO	27	/*Quit command, resume noremal operation*/
#define P_ZIN	28	/*Quit type command, zoom in X2 */
#define P_ZOUT	29	/*Quit type command, zoom out /2 */
struct pltcmd {
   long p_typ;	/*type of plot*/
   long p_id;	/*Mode A code*/
   long p_tkno;	/*Track Number*/
};
#define VWIDTH (14*HSPACE)	/*pixel width of " %6d,%6d"*/
#define CHEIGHT VSPACE		/*pixel height of a character*/
#define CWIDTH HSPACE		/*pixel width of a character*/
#define NCOLOR		256
#define UAL_CPLAN  colorpx[IX_UAL]	/*.23 or to draw UAL arrival*/
#define UAX_CPLAN  colorpx[IX_UAX]	/*.23 or to draw UAX arrival*/
#define DEP_CPLAN  colorpx[IX_DEP]	/*.23 or to draw a departure*/
#define ARR_CPLAN  colorpx[IX_ARR]	/*.23 or to draw a non-UA arrival*/
#define OTH_CPLAN  colorpx[IX_OTH]	/*.23 or to draw any othr normal track*/
#define XTR_CPLAN  (STRK_CPLAN | TXT_CPLAN | BCK_CPLAN) /*.23 and to erase trk*/
#define STRK_CPLAN colorpx[IX_STRK]	/*.23 or to draw a sel trk = overlay*/
#define XSTR_CPLAN ~STRK_CPLAN		/*.23 and to erase a selected track*/
#define TXT_CPLAN  colorpx[IX_TXT]	/*.23 pixel value for text = overlay*/
#define BOX_CPLAN  TXT_CPLAN		/*pixel value for zoom outline*/
#define TRK_CPLAN  STRK_CPLAN		/*pixel value for track points*/
#define FND_CPLAN  STRK_CPLAN		/*pixel value for Find Track Mark*/
#define FOR_CPLAN  colorpx[IX_FOR]	/*.23 pixel value for foreground*/
#define APT_CPLAN  FOR_CPLAN		/*.23 pixel value for airports*/
#define MAP_CPLAN  colorpx[IX_MAP]	/*.23 pixel value for 1st map color*/
#define BCK_CPLAN  colorpx[IX_BCK]	/*.23 pixel value for background*/
#define LABEL_CPLAN colorpx[IX_LABEL]	/*.23 pixel value for label*/
#define REGN_CPLAN colorpx[IX_REGN]	/*.23 pixel value for region boundary*/
#define CIRC_CPLAN colorpx[IX_CIRC]	/*.30 pixel value for range circles*/
#define MARK_CPLAN colorpx[IX_MARK]	/*.23 pixel value for fix type marks*/
/*.37 #define UAL_CPLAN	0xA0	/*.16 or to draw UAL arrival*/
/*.37 #define UAX_CPLAN	0xA2	/*.16 or to draw UAX arrival*/
/*.37 #define DEP_CPLAN	0xA4	/*.16 or to draw a departure*/
/*.37 #define ARR_CPLAN	0xA6	/*.16 or to draw a non-UA arrival*/
/*.37 #define OTH_CPLAN	0xAE	/*.16 or to draw any other normal track*/
/*.37 #define XTR_CPLAN	0xD1	/*.16 and to erase normal tracks*/
/*.37 #define STRK_CPLAN	0xC0	/*.16 or to draw a selected track*/
/*.37 #define XSTR_CPLAN	0xBF	/*.16 and to erase a selected track*/
/*.37 #define TXT_CPLAN	0x90	/*.16 pixel value for text = overlay*/
/*.37 #define BOX_CPLAN	TXT_CPLAN	/*.16 pixel value for zoom outline*/
/*.37 #define TRK_CPLAN	STRK_CPLAN	/*.16 pixel value for track points*/
/*.37 #define FND_CPLAN	STRK_CPLAN	/*.16 pixel value for Find Track Mark*/
/*.16 #define BOX_CPLAN	0xB0  /*pixel value for zoom outline = overlay*/
/*.37 #define APT_CPLAN	0x87	/*pixel value for airports*/
/*.37 #define FOR_CPLAN	0x87	/*pixel value for foreground*/
/*.37 #define MAP_CPLAN	0x81	/*pixel value for 1st map color etc*/
/*.37 #define BCK_CPLAN	0x80	/*pixel value for background*/
/*.37 #define LABEL_CPLAN	0x88	/*.17 pixel value for label*/
/*.37 #define REGN_CPLAN	0x89   /*.17 pixel value for region boundaries*/
/*.37 #define CIRC_CPLAN	0x8A	/*.30 pixel value for range circles*/
/*.37 #define MARK_CPLAN	0x8B	/*.30 pixel value for fix type marks*/
/*.30#define MARK_CPLAN	0x8A	/*.17 pixel value for fix type marks*/
/*Display Types*/
#define HIST 1		/*OBSOLETE replay history in time sequence*/
#define LIVE 2		/*OBSOLETE live data*/
#define DISP_REPLAY 1		/*replay history in time sequence*/
#define DISP_LIVE   2		/*live data*/
#define DISP_BLOCK  3		/*display a block of history*/
#define DISP_CYCLE  4		/*repetative replay of history*/
#define DISP_BLOCK_GRAPH 5	/*block of history with alt graph*/

struct rdseg {		/*descriptor for some road segments*/
   int r_feat;		/*feature ID code*/
   int r_status;
   int r_nrecno;	/*number of permanent records with this feat*/
   unsigned long *r_precno;	/*pntr to array of permanent record numbs*/
   long *r_pofset;		/*pntr to array of offsets in roadxxx */
} ;
#define RD_HREC  1	/*have array of recnums for this feat*/
#define RD_HOFS  2	/*have array of offsets for this feat*/
#define RD_SELEC 4	/*this feat is selected on list1*/

#define CST_COLOR 1	/*mapmcst just colors streets*/
#define CST_RANK  2	/*mapmcst changes rank & colors streets*/

#define NRCMAX 10	/*max # of record numbers to return*/
#define NFTMAX 10	/*max number of feature codes to return*/
#define NDFTMAX 20	/*max number of feature codes in list1*/
#define NRCFMAX 1024	/*max # of record numbers matching a feat*/
#define MAXRANK 7	/*max rank to prepend to existing segment rank*/

#define MAP_ERAS 1	/*.40 bits: erased map*/
#define MAP_SUMM 2	/*.40 bits: produce summary map*/
#define MAP_COMP 4	/*.40 bits: produce complete map*/
#define MAP_OUTL 8	/*.40 bits: produce outline map*/
#define MAP_LARG 16	/*.40 bits: produce large map*/

#define D_NO 0
#define D_YES 1
#define D_EMBED D_YES	/*.11 track numbers primarily from history files*/
#define D_MANUAL 2	/*.11 track numbers primarily entered manually*/
#define D_REALTIME 3	/*.11 track numbers primarily via realtime comm*/
#define D_METERS 0	/*scale displays in meters*/
#define D_NM 1		/*scale displays in nautical miles*/
#define D_DBLOCK 1	/*d_acinfo: display all A/C info at head of track*/
#define D_DLIST 2	/*d_acinfo: display A/C info in a list*/
#define D_ANOALT 4	/*.43 d_acinfo: allow track tag without altitude*/
#define D_DNOALT 8	/*.43 d_acinfo: display track tag without altitude*/
#define D_ALLETA 2	/*.15 show all etas, not just those with flight #*/
#define D_UALETA 3	/*.21 show only ual & uax etas*/
#define MXARPT 25	/*maximum number of airports*/
#define MXAPCHAR 6	/*max number of characters in airport nomenclature*/
#define MXNTRL 10	/*maximum different trail lengths avail*/
#define MXNSPD 10	/*maximum speedup factors avail*/
#define MXNGAIN 10	/*maximum altitude gain factors avail*/
#define MXNLOOK 10	/*maximum altitude look angles avail*/
#define D_LIN 0		/*tracks display as lines*/
#define D_PNT 1		/*tracks display as points*/
#define D_NOCFILTER 1	/*leave in code 1200*/
#define D_CFILTER 2	/*filter out code 1200*/
#define SHOW_ALL 0      /*show all tracks*/
#define SHOW_ARR 1      /*show arriving tracks*/
#define SHOW_DEP 2      /*show departing tracks*/
#define SHOW_A_D 3      /*show arriving & departing tracks*/
#define SHOW_SEL 4      /*show selected tracks*/
#define SHOW_CLS 5      /*.08 show only tracks matching current track class*/
#define STATNLEN 4	/*.19 characters in an ASD airport name*/
#define ACDATALEN 7	/*.24 max length of ASD aircraft data*/
#define SIDNMLEN 8	/*.25 max length of standard departure name*/
#define STARNMLEN 8	/*.25 max length of standard arrival name*/
#define AFIXNMLEN 6	/*.25 max length of arrival fix name*/

/* Parameters to control features presented*/
struct dsparms {	/*Display Parameters*/
   int d_units;		/*units of measurement*/
   int d_rangc;		/*YES => initially show range circles*/
   int d_ntr;		/*number of different trail lengths available*/
   int d_tr[MXNTRL];	/*sequence of trail lengths to use*/
   int d_nspd;		/*number of speedup factors available*/
   int d_spd[MXNSPD];	/*sequence of speedup  factors to use*/
   int d_roads;		/*YES => provide road name lookup*/
   int d_proads;	/*YES => permit road name rank changes*/
   int d_flightno;	/*YES => provide flight numbers assignments to A/C */
   int d_disp;		/*initial display setting (hist or live)*/
   int d_acinfo;	/*initial setting for type od A/C info displayed*/
   int d_narpt;		/*number of airports available*/
   int d_mapr;		/*type of map to display*/
   int d_trktyp;	/*tracks display as lines or points*/
   int d_cfilter;	/*filter out code 1200*/
   int d_findt;		/*.07 YES => provide find track facility*/
   double d_rrangc;	/*range circle interval in meters*/
   double d_rrang1;	/*.40 range circle interval in meters MAP_LARG*/
   double d_hticks;	/*horizonal position tick mark interval in meters*/
   double d_htick1;	/*.40 horiz pos tick mark interval in meters MAP_LARG*/
   double d_vticks;	/*vertical position tick mark interval in meters*/
   int d_naggain;	/*number of altitude gain factors available*/
   int d_aggain[MXNGAIN];	/*sequence of altitude gain factors to use*/
   int d_stacks;	/*0 => no altitude stacks*/
   int d_showblk;	/*which tracks to show on a block display*/
   int d_stickblk;	/*show altitude sticks on selected trks on block disp*/
   int d_tclass;	/*.08 track class to show or store*/
   int d_classb;	/*.08 bits in c_stat for desired track class*/
   char d_arpt[MXARPT][MXAPCHAR+2];	/*airport nomenclatures*/
   char	d_notes[MAXNOTES+2];
   char d_pptitle[MAXNOTES+1];	/*title for plan position plot graph*/
   char d_pptitl1[MAXNOTES+1];	/*.40 title for plan position plot MAP_LARG*/
   char d_agtitle[MAXNOTES+1];	/*title for altitude versus x graph*/
   char d_agtitl1[MAXNOTES+1];	/*.40 title for altitude versus x MAP_LARG*/
   char d_agytitle[MAXNOTES+1];	/*title for altitude versus y graph*/
   char d_agytitl1[MAXNOTES+1];	/*.40 title for altitude versus y MAP_LARG*/
   char d_ljout[MAXNOTES+1];	/*pipeline for laser jet output (enq -o -dp)*/
   char d_manufac[MAXNOTES+1];	/*manufacturers name*/
   int d_noise;		/*.09 0 => no noise info display*/
   char d_colorarv[SZCOLOR+1];	/*.11 color for arrivals*/
   char d_colordep[SZCOLOR+1];	/*.11 color for departures*/
   char d_coloroth[SZCOLOR+1];	/*.11 color for others*/
   char d_colorsel[SZCOLOR+1];	/*.11 color for selected tracks*/
   char d_colorual[SZCOLOR+1];	/*.16 color for UAL*/
   char d_coloruax[SZCOLOR+1];	/*.16 color for UAX*/
   int d_colormenu;		/*.16 color selection menu*/
   int d_dataday;		/*.16 find available data by day for a year*/
   int d_datahr;		/*.16 find available data by time for a month*/
   int d_datahr1d;		/*.16 find available data by time for a day*/
   int d_download;		/*.16 permit a download from option menu*/
   int d_smooth;		/*.13 smooth target data*/
   int d_eta;			/*.13 display est. times of arrival*/
   int d_rate;			/*.13 arrival & departure rates*/
   int d_rconf;			/*.13 do arrival runway configuration*/
   int d_fnofsiz;		/*.14 max size of flight number file*/
   int d_qhrfsiz;		/*.14 max size of movement rate file*/
   int d_rtcfsiz;		/*.14 max size of rtcalc result file*/
   int d_hstfsiz;		/*.14 max size of rtcalc history file*/
   int d_rdrtim;		/*.14 enable exec of rdrtim when live*/
   int d_pfix;			/*.15 periodic fix interval (sec) 0=>none*/
   int d_holding;		/*.15 check for holding pattern*/
   int d_baseleg;		/*.15 check for base leg*/
   int d_minpfxrng;		/*.15 min rng from airport for pfix (m)*/
   int d_doual;			/*.16 consider UAL & UAX for color & filter*/
   int d_douax;			/*.16 consider UAX for color & filter*/
   int d_help;			/*.16 provide help buttons and windows*/
   /*.18int d_socket;		/*.16 use sockets instead of message queues*/
   int d_spacing;		/*.18 arrival spacing*/
   int d_altcut;		/*.18 perform an altitude cut on the display*/
   int d_althi;			/*.18 highest altitude (m)*/
   int d_altlo;			/*.18 lowest altitude (m)*/
   int d_asdtsiz;		/*.20 max size of asd track file*/
   int d_preports;		/*.25 print reports fron Options Menu*/
   char d_remdirec[NOMSIZ];	/*.25 NFS mounted directory with data files*/
   char d_remtdata[NOMSIZ];	/*.25 directory with target data files*/
   char d_printer[NOMSIZ];	/*.25 pipeline for printer output*/
   int d_passur;		/*.27 flag => running on a passur*/
   int d_stations;		/*.27 flag => show stations from ASD*/
   int d_asdtrk;		/*.28 flag => show ASD tracks*/
   int d_deprpt;		/*.29 print departure report from Options*/
   int d_matrpt;		/*.29 print match report from Options*/
   int d_speed;			/*.31 show speed on detail line*/
   int d_asdeta;		/*.31 show etas from ASD RT msg on etalist*/
   int d_pssrdominate;		/*.31 replace ASD track with pssr*/
   int d_ntit;			/*.31 number of tracks in tritbl & scitbl*/
   int d_exceptions;		/*.32 do exceptions display*/
   int d_qhrcnt;		/*.32 do 1/4 hour count display*/
   int d_usracfilt;		/*.33 use user defined a/c filter*/
   int d_leaders;		/*.35 put leaders at head of tracks*/
   int d_planeimg;		/*.35 put airplane image at head of tracks*/
   int d_rfeta;		/*.36 display est. time of arrival at range fix*/
   int d_nexrad;		/*.38 display nexrad weather radar*/
   int d_dotoday;		/*.39 permit processing of current data*/
   char d_maptypes[NOMSIZ]; /*.40 maps to offer:c=compl s=summ o=outl l=large*/
   int d_ashowfind;		/*.40 override filter for find track*/
   int d_nexranim;	/*.41 # weather images to animate, 0=> no animate*/
   int d_nexrcach;	/*.41 # weather images to cache, 0=> no cache*/
   int d_fixtag;	/*.42 fixed tag pos and/or no alt in tag*/
   int d_fltsel;	/*.44 select by flight number for display*/
   int d_tailno;	/*.46 show tail # from icao24 on detail line*/
   int d_spare1;
   int d_spare2;
};

#define F_EMBED  1		/*.12 f_status bit: from history info*/
#define F_MANUAL 2		/*.12 f_status bit: entered manually*/
#define F_LIVE	 4		/*.13 f_status bit: live from comm line*/
#define F_UNIMAT 8		/*.19 f_status bit: source = Unimatic*/
#define F_ASDCOR 0x10		/*.19 f_status bit: source = ASD correlation*/
#define F_ARRIVE 0x100		/*.19 f_status bit: arriving flight*/
#define F_DEPART 0x200		/*.19 f_status bit: departing flight*/
#define F_OVER	 0x400		/*.19 f_status bit: overflight*/
#define F_TMATCH 0x800		/*.19 f_status bit: matching track*/
#define F_SMARTS 0x1000		/*.29 f_status bit: source = sma-arts*/
#define F_LVNLMSK 0x6000	/*.49 f_status lvnl data*/
#define F_LVNLACT 0x2000	/*.49 lvnl actual arrival or actual departure*/
#define F_LVNLBAT 0x6000	/*.49 lvnl estimated best arrival (BAT record)*/
#define F_LVNLEST 0x4000	/*.49 lvnl estimated arrival (EST record)*/
#define F_ICAO24 0x8000		/*.49 f_status bit: source = icao24, mode s*/

struct fltnmbdtl {		/*.19 flight number vs xponder info*/
   time_t f_stim;		/*.12 start of valid use*/
   time_t f_ctim;		/*.12 estimated time of use*/
   time_t f_etim;		/*.12 end of valid use*/
   ushort f_status;		/*.12 status of this assignment*/
   short f_code;		/*.19 beacon code for the flight number*/
   char f_flid[FLTNCH+1];	/*.19 flight number airline+numb or tail # */
   char f_downline[STATNLEN+1];	/*.19 downline station*/
   char f_upline[STATNLEN+1];	/*.19 upline station*/
   char f_acdata[ACDATALEN+1];	/*.24 aircraft data (type)*/
   char f_sid[SIDNMLEN+1];	/*.25 standard departure (SID) name*/
   char f_star[STARNMLEN+1];	/*.25 standard arrival (STAR) name*/
   char f_afix[AFIXNMLEN+1];	/*.25 arrival fix name*/
};

/*format of message to rdfile: byte_count(long) type(long) data bytes*/
#define R_START    1	/*message to rdfile: start*/
#define R_STOP     2	/*message to rdfile: stop*/
#define R_SPEED    3	/*playback speedup factor*/
#define R_STIME    4	/*start time (Unix calendar time)*/
#define R_DURAT    5	/*time to play after start time (seconds)*/
#define R_REWIND   6	/*start at beginning*/
#define R_CONTINUE 7	/*message to rdfile: continue*/
#define R_FILE	   8	/*open a different file*/

/*Synchronization messages from xtrtfil to xtrafm as M_MBEAM msgs*/
#define MB_XFLAG	1       /*set m_ctime => m_stime has a message*/
#define MB_XSTRT	2       /*set m_stime => Corrected startime*/
#define MB_XDEND	3       /*set m_stime => Duration complete*/

/*A loaded record consists of struct cumrec followed by the data record*/
struct cumrec {
   long c_nbyt;			/*total record size*/
   long c_pofset;		/*offset in rec of next point to deliver*/
   time_t c_ctim;		/*will be time of previous point*/
   long c_x;			/* x coord (meters) */
   long c_y;			/* y coord (meters) */
   long c_z;			/* z coord (meters) */
   ushort c_id;			/*beacon code*/
   ushort c_stat;		/*status*/
   ushort c_spnt;		/*selected pnt (-1 if none)*/
   char c_flightno[FLTNCH + 1];	/*flight number, 1st char = NULL if none*/
};
#define C_PICK	1		/*c_stat bit: track has been selected*/
#define C_STICK	2		/*c_stat bit: track has altitude sticks*/
/*.08 c_stat bits for arrival, departure and class are from t_tstat (filter.h)*/

struct sumrec {
   time_t s_utim[2];		/*starting and ending times*/
   long s_ofset;		/*offset of record in file*/
   char *s_radr;		/*record address (from malloc)*/
};
/*Used for Track ID Selection Popup*/
#define SZTID (FLTNCH+11)	/*max size of track ID "1234 12:34:56"*/
struct trkrec {
   struct sumrec *t_psum;
   char t_tid[SZTID+1];
};

#define MAXMSTK	50	/*maximum number of members of a holding stack*/
#define NSTSTR (NDBSTR + 3)	/*code/alt id char(s_tid)*/
struct stackent {
   long s_tkno;		/*the arbitrary track number*/
   long s_alt;		/*altitude (100 ft)*/
   char s_ststr[NSTSTR];	/*flight # or xponder code & alt & id_char*/
};

struct pntl22 {		/*two 2 dim points as longs*/
   long x1;
   long y1;
   long x2;
   long y2;
};

#define m_status m_vz	/*.35 uses bits for t_status*/
struct trga_d {		/*target data*/
   ushort m_tkno;	/*track number (arbitrary) */
   ushort m_pnt;	/*point number */
   time_t m_ctim;	/*current calendar time*/
   long m_x;		/* x coord (meters) */
   long m_y;		/* y coord (meters) */
   long m_z;		/* z coord (meters) */
   ushort m_id;		/*beacon code*/
   short m_vx;		/* x velocity (m/s) */
   short m_vy;		/* y velocity (m/s) */
   short m_vz;		/* z velocity (m/s) */
   char m_flightno[FLTNCH + 1];	/*.05 flight number, 1st char = NULL if none*/
};

struct p_nmtd {		/*.09 Parameters to show positions of NMTs*/
   char name[NNMTNAME+1];	/*name of NMT site*/
   int xreal;	/*real physical position of NMT*/
   int yreal;
   int xmark;	/*position to show mark representing NMT*/
   int ymark;
   int xdisp;	/*position to show noise level from NMT*/
   int ydisp;
   int extnmb;	/*.10 external number of the NMT (used in 7618)*/
};

struct psparms {	/*.09 Position Parameters*/
   char p_apunits[80];		/*units of measurement from apdata*/
   struct pnt3 p_aporigin;	/*zero of aircraft position from apdata*/
   char	p_notes[MAXNOTES+2];
   int p_nnmt;			/*number of NMTs*/
   struct p_nmtd p_nmt[MAXNMT];	/*NMT parameters*/
   int p_spare1;
   int p_spare2;
};
#endif /*XTRAFMH	.19*/
