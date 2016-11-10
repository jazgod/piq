#ifndef RADARH	/*A.30*/
static char rdridd[] = "@(#)radar.50.h 090215 jac Copyright Megadata Corp";
/* Copyright 1990 Megadata Corp, Bohemia NY (unpublished work)*/
/*radar.50.h 090215 jac:add M_TEXTD
 *radar.49.h 081227-090103 jac:adjust llfixinfo
 *radar.48.h 081209-081215 jac:a_lldfixes a_llafixes
 *radar.47.h 080717 jac:shapefiles
 *radar.46.h 070302 jac:add M_RTCSPACEFIC
 *radar.45.h 060622 jac:add M_KLMCCIS
 *radar.44.h 060602-060602 rjd:MSQ_SC MS_RAWMSG; fix some comments
 *radar.43.h 060105-060105 jac:a_nrepb a_ntrtrak
 *radar.42.h 050402-050406 jac:M_ICAO24
 *radar.41.h 050205-050304 jac:a_m12loc M_REALTIMET
 *radar.40.h 040917 jac:M_FIRDA
 *radar.39.h 040624-040705 jac:merge in radarC.07.h, MXTYP 20<-10 a_asbeac
 *  FLTNCHA asbeacon A_NASBEACON NAUXI
 *radarA.38.h 040514 jac:A_DEPART a_airstat[8]
 *radarA.37.h 031229 rjd:STAG256 conditionalize MAXSTG and RBSIZE
 *radarA.36.h 031119-031120 jac:a_airlin3 ... a_airlin7
 *radarA.35.h 030801 rjd:add ModeS definitions (modify per radarA.30S.h from
 *  radarA.30.h):MS_NIRO MS_NIF MSSENSI MSSENSR MSSENSO MSINTDLY MSACTHR MSDII
 *  MSPURGTIM MSCIDCHGLIM MSCFSIZ MSCALTTOL MSCCORPCT MSCCORPCTC MSCCORUNIQ
 *  MSCDECPCT MSCECPCTC MSDECCT MSCECSEC MSQ_SA MSQ_SB MS_DRIVER MS_RESTART
 *  MS_IDMSG MS_ALTMSG a_mssensiro a_msintdly a_msacthr a_msdpurgif
 *  a_msidchglim a_msdii a_msdspare0 a_mscfsiz a_mscpurgif a_mscalttol
 *  a_msccorpct a_msccorpctc a_msccoruniq a_mscdecpct a_mscecpctc a_mscdecsec
 *  a_mscspare0
 *radarA.34.h 030502-030531 jac:MAXTZ -> 16 a_larg a_nlarg
 *radarA.33.h 030421-030422 jac:add M_ICAO24SEND M_ICAO24LIVE M_ICAO24STAT
 *radarA.32.h 020430 jac:add a_mbcolt MAXSTG & RBSIZE unchanged
 *radarA.31.h 020214 jac:add a_gacodes MAXGACODES
 *radarA.30.h 010404 jac:add a_remdirec a_remtdata
 *radarA.29.h 001017 jac:add s_cplsmis s_angsmis h_cplsmis h_angsmis
 *radarA.28.h 000911 jac:add a_asfnfsiz a_astkfsiz a_mxhfsiz
 *radarA.27.h 000324 jac:add a_partner MAXPARTNER
 *radarA.26.h 990925 jac:add lat/lon points MXVERT=20
 *radarA.25.h 990831 jac:more upper case stations
 *radarA.24.h 990729 jac:more mbstat & hstat
 *radarA.23.h 990324-990701 jac:NTRTRAK=1000->2000 more sigtbl & hstat
 *radarA.22.h 981229 jac:more hstat
 *radarA.21.h 981211 jac:multi radar locs. NRADAR
 *radarA.20.h 980602 jac:MAXTKNO TKNMBTIM MXARG M_SENDMBTM
 *radarA.19.h 980509-980602 jac:s_mbutc cutime
 *radarA.18.h 971124-971125 jac:a_bndry a_nbndry a_origin M_SENDPASS M_SPARE3
 *radarA.17.h 970108-970523 jac:M_AFLTLIVE M_AFLTSEND M_PASSIT SPACE_REGION=2048
 *  M_SNDRGN
 *radarA.16.h 960126 jac:M_ASDDEL M_ASDTRK
 *radarA.15.h 950913 jac:a_airlinx a_uaxfnrng NAIRLCH a_airfnmb a_nrpchn s_rcnts
 *radarA.14.h 950712 jac:uaxcarr
 *radarA.13.h 09/19/94 1410 jac:M_ASDPNT
 *radarA.12.h 06/15/94 1040 jac:MAXOUTS=3->5 a_delay
 *radarA.11.h 02/15/94 1650 jac:on HP A_UNIREQ A_SPAREB A_SPAREC
 *radarA.10.h 02/07/94 1700 jac:NPRP9L=6 MXTYP 10<-3 MAXPRP
 *radarA.09.h 11/03/93 1250 jac:M_FLTNC M_RTCSPACING
 *radarA.08.h 08/23/93 0950 jac:r_nvert r_bndy MINCYCL87 a_asd
 *radarA.07.h 05/03/93 0950 jac:elim extra M_..., M_RTCTCLS M_SPARE[12]
 *  a_pluses a_exes a_nomark
 *radarA.06.h 11/20/92 1320 jac:a_msq1-3 a_portno a_server M_WWVLIVE M_WWV
 *  M_SAR + much ASR9
 *radarA.05.h 08/13/92 1050 jac:a_rwyinfo NTRTRAK=600->1000 NREPB=4000->10000
 *radarA.04.h 05/29/92 1130 jac:a_nrwyn a_rwynam a_nafixes a_afixes a_minaltfin
 *  a_maxaltfin a_minaltbas a_maxaltbas a_mintrnbas a_maxtrnbas a_station
 *  a_airline
 *  M_RTCETA M_RTCRATE M_RTFLTNO M_RTCETAS M_FLTNO M_FLTLIVE M_FLTSEND
 *  M_FLTSTATUS
 *radarA.03.h 03/27/92 1140 jac:ASR9 types: r_87typ r_87difx s_87typ s_87difx
 * MAXPRP87 MINCYCL87 NPRP9L r_ndup s_ndup T_SRB
 *radarA.02.h 03/19/92 1250 jac:More replies & 200NM range (i486-33)
 *  NTRTRAK=300->600 MAXTWD=2230->16000 MAXOUTS=1->3
 *radar.17.h 01/21/92 1650 jac:M_HIST M_RUNL S_LCKING SL87RLCK SLMISS MBMISS
 * NMBULKSAV NTIDX NSIDX MINCHK
 *radar.16.h 11/21/91 0820 jac:r_87difl
 *radar.15.h 11/20/91 0740 jac:a_debug[],a_hctime
 *radar.14.h 07/05/91 1750 jac:a_mbint,a_timout
 *radar.13.h 05/14/91 1750 jac:a_tz
 *radar.12.h 05/01/91 1830 jac:Flat histo, M_TARGA
 *radar.11.h 04/12/91 1110 jac:Add ifndef SENSOR, creset
 *radar.10.h 03/04/91 0750 jac:Add regions & rangcent, change fixes to integer
 *radar.09.h 02/11/91 1140 jac:Add a_maxtwd => like radarA but sizes, Add M_LSL
 *	antenna control
 *radar.08.h 02/06/91 1350 jac:Increase MAXFIXS 128
 *radar.07.h 12/27/90 1540 jac:Add fixes & nrangc
 *radar.06.h 10/06/90 1700 jac:Add M_VERSIONS to M_PSEF
 *radar.05.h 10/03/90 1400 jac:Add a_iwdwth
 *radar.04.h 08/15/90 1200 jac:Add mode A override
 *radar.03.h 06/25/90 1330 jac:At Sarasota, very unstable radar IWDWTH=13->29
 *	PRDWW=255->1023 FPTOLS=4295->42950
 *	ASR9 at Orlando s_87idx,s_87nt,s_87ns,s_87difm,s_87difl r_87idx,r_87dif
 *radar.02.h 05/31/90 1550 jac:After 1st delivery
 *	6/12/90 TRAN_DELAY from 7120 to 7382 to align BRK & REP airports
 *radar.01.h 12/14/89 1300 jac:use all double
 *radar.00.h 10/10/89 0750 jac:Header for aircraft traffic monitor
 * The abbreviation asec is attoseconds = 10E-18 sec.
 * The least significant half of precision times is 33.76 asec.
 * Only the high order 16 bits are significant.  Thus the times are
 * stored with a precision of 2.2125 picoseconds.
 */

#ifdef STAG256	/*A.37*/
static char rdr256[] = "@(#)radarA.xx.h compiled for 256-step stagger"; /*A.37*/
#endif	/*A.37 STAG256*/

#define MINMATCH 4	/*C.00 minimum number of interrogations in match*/
#define MAXMODE 2	/*C.00 number of modes to try excluding MB*/
#define MBMODE MAXMODE	/*C.00 mode index for main beam*/
#define SLSTOL 2	/*C.06 tolerance on pri match*/
#define USEC1P5 10	/*C.00 1.5 usec lsb=145ns*/
#define USEC2P5 17	/*C.00 2.5 usec lsb=145ns*/
struct dlymode  {	/*C.03*/
   int m_code;		/*code for the mode*/
   int m_delay;		/*P2 to P3 time for the mode, .145usec*/
};			/*C.03*/

#define RADARH	/*A.30*/
/*A.23 #ifdef __hpux /*A.10*/
#if defined(__hpux) || defined (__sun__)	/*A.23*/
typedef unsigned char uchar;	/*A.10*/
typedef unsigned long ulong;	/*A.10*/
#endif /*A.10 __hpux*/
#ifdef __sun__	/*A.23*/
typedef unsigned short ushort;	/*A.23*/
#endif /*A.23 __sun__*/

#ifdef WIN32 
typedef unsigned long ulong;
typedef unsigned short ushort;
#endif

struct sigtbl *mbmatch();
struct sigtbl *tbrhnd();
ulong cpprc();
double cgr_alt();
struct sigtbl *pbrhnd();
double utc145();	/*A.19 convert internal time (145ns) to UTC (unix)*/

#define MXARG 63	/*A.19 max arguments for a command to communA*/
#define MAXTKNO 65000	/*A.19 1 + highest possible arbitrary track number*/
#define TKNMBTIM 65001	/*A.19 track number to hold precision MB time*/
#define NAIRLCH 3       /*A.15 three chars in airline part of flight id*/
#define R_ARRIVE 0	/*A.08 runway end index for the arrival boundary*/
#define R_DEPART 1	/*A.08 runway end index for the departure boundary*/
#define ASR9TCYC 256	/*A.06 number of ASR9 cycles per rotation*/
/*.39#define ASR9CTOL 7	/*A.06 tolerance on raw number of ASR9 cycles per rot*/
#define ASR9CTOL 9	/*.39 tolerance on raw number of ASR9 cycles per rot*/
#define ASR9ACTL 3	/*A.06 tol on fitted number of ASR9 cycles per rot*/
#define MAXS95LEN 5	/*A.06 max length of short stagger addon to ASR9*/
#define S95SOFS MAXSTG-13 /*A.06 PSSR offset in prplst to put short stag DTs*/
#define S95LNIDX MAXSTG-3 /*A.06 PSSR offset in p_prp to pass short stag len*/
#define BIGVAL	10000	/*A.06 local value larger than largest short stag DT*/
#define NSTOL	8		/*A.06 ASR9 -NSTOL <= ns <= nt + NSTOL*/
#define S95MAX	S5MAX	/*A.06 max diff between same PRI due to 5 step stagger*/
#define NS95DIF	24	/*A.06 max number of 5 step stagger difs for an ASR9*/
#define MINPMAT	4	/*A.06 minimum prp matches to align nonunique ASR9*/
#define MAXPLOOK MINPMAT+2 /*A.06 max prps to examine to align nonunique ASR9*/
#define MAXPMISS 3	/*A.06 max sequential misses to align nonunique ASR9*/
#define ASR9CLK 5.3276	/*A.06 ASR9 clock period in units of 0.145 usec.
		16/3 is 5.3333 for a fractional difference of 0.0011*/
#define MAXASR9PROG 4	/*A.06 max ASR9 stagger progression*/
#define MINMCYC 150	/*A.06 min meas cycles to constrain ns/nt in cpprc()*/
#define MINTCYC 50	/*A.06 min MB cycles to constrain ns/nt in cpprc()*/
#define NOMSIZ	128	/*A.06 smallish buffer*/
#define TYP87_76  1	/*.A03 ASR9 r_87typ s_87typ, 7step + 6step*/
#define TYP87_77  2	/*.A03 ASR9 r_87typ s_87typ, 7step + 7step*/
#define TYP87_87  3	/*.A03 ASR9 r_87typ s_87typ, 8step + 7step*/
#define TYP87_88  4	/*.A03 ASR9 r_87typ s_87typ, 8step + 8step*/
#define SL87RLCK  3*SL87MISS	/*.17 ASR9 attempt relock after misses*/
#define SLMISS	MAXSTG-3	/*.17 ASR9 s_prpm[SLMISS] is sequential misses*/
#define MBMISS	5	/*.17 gap in main beams to process MB, was 2*/
#define NMBULKSAV 5	/*.17 instances of info to save at unlock TESTING*/
#define NTIDX	MAXSTG-2	/*.17 ASR9 s_prpm[NTIDX] is total cycle count*/
#define NSIDX	MAXSTG-1	/*.17 ASR9 s_prpm[NSIDX] is short cycle count*/
#define	MINCHK 3	/*.17 ASR9 min windows from edge to check branches*/
#define NMBDEBUG 3	/*.15 number of independent classes of debug*/
#define MAXNSREF 200	/*.15 max # SLS reference times to store*/
#define HCTIME	600	/*.15 time interval between history statistic reports*/
#define MAXOUTS	5	/*A.12 number of output comm lines*/
/*#define MAXOUTS 3	/*A.02 number of output comm lines*/
/*#define MAXOUTS 1	/*.14 number of output comm lines*/
#define GDLCKCNT 10	/*.14 # locked rots before disable MB ints*/
#define MXSEQERR 20	/*.14 # sequential driver errors before reset*/
#define BITSDIF	 3	/*max # mode A bits different for match to track*/
/*A.17#define SPACE_REGION 256 /*.10 #integers for regions*/
#define SPACE_REGION 2048 /*A.17 #integers for regions*/
#define MAXTZ 16	/*.34 max number of characters in Unix timezone*/
/*.34#define MAXTZ 8	/*max number of characters in Unix timezone*/
#define MAXFIXS 128	/*max number of navigation fixes*/
#define MAXLLFIX 512	/*A.26 max number of lat/lon fixes*/
#define DOTSQR 4	/*A.26 half # dots to make a square*/
#define MAXMAPR	 6	/*max number of characters in map region suffix*/
#define MAXNOTES 128	/*max number of characters in notes*/
#define PEREXTRA 70	/*addition TOA added to perimeter 0.145usec*/
/*.A03#define MAXPRP87 37931 /*special max for sum of two prps for stag87*/
#define	MAXPRP87 44827	/*.A03 max for sum of two prps for stag87 6500usec*/
#define	MAXCYCL87 172414	/*max cycle length for stag87, 25msec*/
#define	MINCYCL87 115700	/*.A08 minimum ASR9 cycle length (actual)*/
/*A.08#define	MINCYCL87 103448	/*.A03 minimum ASR9 cycle length 15ms*/
#define FRCSTP87 429	/*ASR9 stop search at frac instability 100E-9*/
#define FRCOK87 2147	/*ASR9 accept pattern at frac instability 500E-9*/
#define NPRP9  8	/*.A03 ASR9 Highest ASR9 stagger length*/
/*.A10 #define NPRP9L 7	/*.A03 ASR9 Lowest ASR9 stagger length*/
#define NPRP9L 6	/*.A10 ASR9 Lowest ASR9 stagger length*/
#define NSHFCC	7	/*scan per match within 1/2^NSHFCC of a circle*/

#define TRAN_DELAY 7200		/*Attempt to straighten paths over ISP-24*/
/*#define TRAN_DELAY 7308	/*Nominal less 100ns short in reply delay 6/18*/
/*#define TRAN_DELAY 7382	/*Attempt to align BRK & REP airports 6/12*/
/*#define TRAN_DELAY 7120		/*shorten by 5 clocks*/
/*#define TRAN_DELAY 7338	/* 3us transponder delay + 20.3 usec reply
		* duration + 34 clock F2 decode & time latch delay 
		* delay - 26 clock P3 decode & time latch delay in meters
		* the 1/2 window delay has already been adjusted
		* This is best estimate of hardware as of 5/10/90*/
/*#define TRAN_DELAY 7295		/* shorten by 2 taps*/
/*#define TRAN_DELAY 7382	/* 3us transponder delay + 169 tap F1 decode
				 * delay - 20 tap P3 decode delay in meters
				 * the 1/2 window delay has been adjusted
				 * already*/
#define CODEBITS 07777	/*valid mode A & mode C bits*/
#define SAMALT	100	/*same altitude crit for no recalc of position, meters*/
#define MBCTIM	68965520	/*accumulation period for MB hits, lsb=0.145us*/
#define NPERIM	200	/*number of points around perim to calc maxtoa*/
#define	MAXRATOA 4800	/*max toa to consider ring around, meters*/
#define	MAXDRTOA   50	/*tolerance to match toa for ring around, meters*/
#define MAXDSTOA  100	/*tolerance to match toa for side lobes, meters*/
#define	MAXDMTOA  300	/*tolerance to match toa for multipath, meters*/
#define MAXDPHI 0.1	/*dif raw & extrap phi to delete meas pos., radians*/
#define TKDGRAD	3	/*x-y pos timeout after this # rots of locked radar*/
#define TKTOROT	6	/*track timeout after this # rots of locked radar*/
#define TKTOSEC	20	/*track timeout after this time if no locked radar*/
#define TOAMIN	87	/*min TOA above ambiguous sol for recon (meters)*/
			/*0.145usec = 43.5meters*/
/*#define IWDWTH	13	/*interrogation window width lsb=0.145usec*/
#define IWDWTH	29	/*.03 interrogation window width lsb=0.145usec*/
#define MAXTWD 16000	/*A.01 max reply wind width (352km) lsb=0.145usec*/
/*#define MAXTWD	2230	/*maximum reply window width lsb=0.145usec*/
#define MAXTGAP	30	/*time gap at which to terminate a track (sec)*/
#define MAXNTGAP 4	/*maximum amount time can move backward (sec)*/
#define NMBTIM 256	/*number of main beam time entries*/
#define NRDBF 75	/*number of radar data buffer entries*/
#define NSIGTB	50	/*number of signature table entries*/
/*#define CDFSIZ 1000000 /*OBSOLETE size of disk circ buffer for target data*/
#define MAXMSG	95	/*max number of msg queues for C & D queues*/
#define DT	30	/*1 sd error in toa (meters), 30m => 0.1usec*/
#define DPHI	0.00436	/*1 sd error in daz (radians), 0.00436 => .25deg*/
/*DACC = 0.2 holds with 100m/s R=1000m S-turn(180-180) 4sec update*/
/*DACC = 0.1 breaks badly with 100m/s R=1000m 4sec update*/
/*DACC = 0.16 breaks with 100m/s R=1000m S-turn(180-180) 4sec update*/
/*DACC = 0.16 holds with 100m/s R=1000m 180 turn 4sec update*/
#define DACC	0.25	/*1 sd error in accell per second, meter/sec^3 */
#define DALT	15.	/*1 sd error in altitude (meters) (15m = 50 ft)*/
#define AVAR1	10.	/*Initial Variance in alt rate, meter^2/sec^2 */
#define AVAR2	0.25	/*Variance in alt accell per sec^2, meter^2/sec^6 */
#define MINDT	6	/*minimum-1 dt entries left to start a sequence*/
#define MINSEQ	6	/*minimum dt hits to declare a sequence*/
/*#A.03define MINSTG	6 /*minimum matching interrogs to declare a stagger*/
#define MINSTG	7	/*A.03 minimum matching interrogs to declare a stagger*/
/*.A10 #define MAXPRP	34483	/*maximum PRP=5000us, lsb=.145usec*/
#define MAXPRP MAXPRP87 /*.A10 maximum PRP=6500us, lsb=.145usec*/
#define MINPRP	6897	/*minimum prp=1000us, 0.145usec*/
#define MAXPRP40 25517	/*maximum PRP from recomb=3700, lsb=.145usec*/
#define MINPRP40 12069	/*minimum prp for recomb=1750us, 0.145usec*/
#define S5MAX	100	/*max 5 step value, 0.145usec*/
#define MAXCYCL	855172	/*maximum stagger cycle length=124ms, lsb=.145us*/
/*#define MAXCYCL 172414 /*maximum stagger cycle length=25ms, lsb=.145us*/
#define PTOL	5	/*match tolerance is +- 5 least counts*/
#define FITOLS	214748	/*instability tolerance at startup = 50 ppm*/
#define FTTOLS	42950	/*instability tolerance to declare pred. = 10 ppm*/
/*#define FPTOLS 4295	/*instability tolerance to stay predictable = 1 ppm*/
#define FPTOLS	42950	/*instability tolerance to stay predictable = 10 ppm*/
#define PRDCLR	2	/*required clearance at window ends for predictable*/
#define PRDEWW	20	/*extra window width (hits) for predictable*/
#define MNWW2	2	/*minimum 1/2 window width on number of
			 * mbeam hits for consistant scan period*/
#define RPERTOL	0.2	/*tolerance to match desired rotation period, sec*/
#define PRPTOL	2.0	/*tolerance to match desired prp, usec*/
#define MINSP	 20689655 /*minimum scan period = 3sec, lsb=0.145usec*/
/*#define MAXSP	110344827 /*maximum scan period =16sec, lsb=0.145usec*/
#define MAXSP	89655172 /*maximum scan period =13sec, lsb=0.145usec*/
/*#define PRDWW	255	/*wide window width for predictable, lsb=0.145usec*/
#define PRDWW	1023	/*wide window width for predictable, lsb=0.145usec*/
#define PRDFWW	6	/*fine window width for predictable, lsb=0.145usec*/
#define MGAP	0xFFFE	/*burst sep time gap for predictable, lsb=0.145usec*/
#define PMISCR	3	/*consectutive misses on predictable => to tentative*/
#define TENTTO	414000000 /*time out for tentative radar (60sec), lsb=0.145us*/
#define SLWIND	PTOL	/*window=> tent burst = sidelobe, lsb=0.145usec*/
#define MPWIND	75	/*window=> tent burst = multipath, lsb=0.145usec*/
#define MINHB 3		/*min number of hits in middle toa to try*/
#define MTOL 128	/*tolerance to match prps in tgrcorr*/
#define RGAP 6		/*number of missed replies to separate bursts*/
#define MINREPLY 6	/*minimum number of replies to declare a target*/
/*#define FRCCLN 90	/*fraction of scan period to time out replies*/
/*A.06#define FRCCLN	40	/*fraction of scan period to time out replies*/
#define FRCCLN	60	/*A.06 fraction of scan period to time out replies*/
/*#define MAXRTOA 4140	/* OBSOLETE  maximum TOA, lsb=0.145usec 30km*/
#define MAXSPED 310.	/*max assumed speed when starting a track, meters/sec*/
#define MAXSPED2 96100.	/*maximum speed squared for 1pt track (m/s)^2 */
#define MAXDEFECT 36.	/*max defect (miss^2/variance) when extending a trk*/
#define MAXTRAP 30.0e6	/*maximum extrap dist squared for >1pt (meters^2)*/
#define MAXTRAP1 8.0e6	/*maximum extrap dist squared for 1pt (meters^2)*/
#define TOATRN	7200	/*toa (meters) to use maxima, reduce if smaller*/
#define MINQFRAC 0.1	/*minimum frac of maxima to use for small toas*/
#define NOALT	-600.	/*altitude retured if altitude is not available*/
#define ATOMETER 30.48	/*altitude in 100ft to meters*/
#define NDTIM	NMBTIM	/*number of dtime entries*/
#ifndef STAG256	/*A.37*/
#define MAXSTG	43	/*desired+1, max stagger is 40 (8*5) steps*/
#else	/*A.37 STAG256*/
#define MAXSTG	260	/*A.37 desired+4, max stagger is 256 for lhr/asr11*/
#endif	/*A.37 STAG256*/
/*.10 #define MXTYP 3	/*number of different prp patterns a radar might have*/
/*.39#define MXTYP 10	/*.10 number of dif. prp patterns a radar might have*/
#define MXTYP	20	/*.39 number of dif. prp patterns a radar might have*/
#define NSENSE	4	/*number of receiver sensitivity values*/
#define MAXPASSWD 14	/*max char in password required at commun startup*/
/*#define MAXSTG	13	/*desired+1, max stagger is 12 steps*/
#define SZBUF 16384	/*size of buffer for LCA load data*/
			/*LCA3042 takes 30824 bits => 15412 chars for 4 LCA*/
#define SSIZ	6	/*size of state vector*/
#define S	SSIZ
#define ASIZ	3	/*size of altitude state vector*/
#define A	ASIZ
#define TMASK	0x80000000L /*mask for msb of a 32 bit long*/
#define TMASK2	0x40000000L /*mask for msb-1 of a 32 bit long*/
#define TMASK16	0x8000L /*mask for msb of a 16 bit short*/
#define NINDX	6	/*mode voting interlace length, 6=2*3 */
#define MODEO	0	/*code for "other" interrogation mode*/
#define MODEA	1	/*code for mode A interrogation*/
#define MODEC	2	/*code for mode C interrogation*/
#define MODEU	3	/*code for "unknown" interrogation mode*/
#define NREPB	10000	/*A.05 number of replies plus interrogs in a segment*/
/*#define NREPB	4000	/*number of replies plus interrogations in a segment*/
/*A.05 tried NRPCHN 12000 but startup with 250 targets took > 4 minutes*/
#define NRPCHN	8000	/*A.01 number of reply chain entries*/
/*#define NRPCHN	2000	/*number of reply chain entries*/
#define NTRTRAK 2000	/*A.23 number of target track entries*/
/*.23#define NTRTRAK 1000 /*A.05 number of target track entries*/
/*#define NTRTRAK 600	/*A.01 number of target track entries*/
/*#define NTRTRAK 300	/*number of target track entries*/
#define MAXTARG 32	/*DO NOT CHANGE max number of targets per segment*/
#define NPOT	3	/*number of potential matches per track*/
#define RCA	1	/*bit in rcbits for chain A non-NULL*/
#define RCB	2	/*bit in rcbits for chain B non-NULL*/
#define RCC	4	/*bit in rcbits for chain C non-NULL*/
#define RCAB	3
#define RCAC	5
#define RCBC	6
#define RCABC	7
#define NMODE 2		/*number of interrog modes of interest*/
#define NRBIT 12	/*number of bits in a reply code*/
#define PIOV	  0.0174532925199	/* PI/180 */
#define PIOV2	  1.5707963267949	/* PI/2 */
#ifdef PI	/*A.17*/
#undef PI	/*A.17*/
#endif /*.17 PI*/
#define PI	  3.1415926535898
#define THREPIOV2 4.7123889803849	/* 3*PI/2 */
#define TUPI	  6.2831853071796
#define TUNITU 0.145	/*time unit in usec*/
#define TUNITS 0.000000145	/*time unit in sec*/
#define SECTTU 6896552		/*# of time units in one second*/
#define IREFRAC 0.9990186	/*speed of L band/300 meters/usec
				  * C(L band) = 299.7925/1.00029 m/usec*/
#ifndef RBSIZE
#ifndef STAG256	/*A.37*/
#define RBSIZE 1024	/*size of standard "buffer" for radar stuff*/
#else	/*A.37 STAG256*/
#define RBSIZE 2048	/*A.37 size of standard "buffer" for radar stuff*/
#endif	/*A.37 STAG256*/
#endif /*RBSIZE*/
#define DB_MAGIC 0x2d3c4b5a	/*magic number for disk circ buffer*/
#define CNT_C	3		/*Control C == ETX*/
#define ETX	3		/*Control C == ETX*/
#define MAINSENS  0		/*Set Main Beam Sensitivity D to A Converter*/
#define SLSSENS   1		/*Set SLS Sensitivity D to A Converter*/
#define REPLYSENS 2		/*Set Reply Sensitivity D to A Converter*/
#define ASFNFSIZ1 12000000	/*A.28 default size of asfltfile (a_asfnfsiz)*/
#define ASTKFSIZ1 10000000	/*A.28 default size of asdtkfil (a_astkfsiz)*/
#define MXHFSIZ1  100000	/*A.28 default max size of history file*/

/*A.35 Mode S aparms-element dimensions */
#define MS_NIRO	3		/*A.35 int/reply/omni item */
#define MS_NIF	2		/*A.35 inactivity/frequency item */
/*A.35 Mode S aparms-element defaults */
#define MSSENSI 70		/*A.35 interrogation sensitivity*/
#define MSSENSR 90		/*A.35 directional reply sensitivity*/
#define MSSENSO 160		/*A.35 omni reply sensitivity*/
#define MSINTDLY 6		/*A.35 interrog delay to first data cell*/
#define MSACTHR 3		/*A.35 all-call target validation threshold*/
#define MSDII   0		/*A.35 local radar ii*/
#define MSPURGTIM 120		/*A.35 purge seconds default*/
#define MSCIDCHGLIM 5		/*A.35 allowable id changes per minute*/
#define MSCFSIZ  50000000	/*A.35 default correl output file size*/
#define MSCALTTOL 100		/*A.35 correl alt tolerance*/
#define MSCCORPCT 90		/*A.35 correl agree percentage*/
#define MSCCORPCTC 10		/*A.35 min count for correl agree percentage*/
#define MSCCORUNIQ 8		/*A.35 correl number of unique alts*/
#define MSCDECPCT 50		/*A.35 decorrel agree percent*/
#define MSCDECPCTC 20		/*A.35 min count for decorrel agree pctg*/
#define MSCDECCT  9		/*A.35 decorrel successive disagreements*/
#define MSCDECSEC 30		/*A.35 continuous seconds of disagreement*/

struct dtim {
   ushort d_dtim;	/*delta time, lsb=0.145usec*/
   ushort d_mbndx;	/*index of corresponding main beam time*/
}; 

#define SHCYC	0x100	/*ASR9 or into d_pidx => after junction in short cycle*/
struct ditim {
   ushort d_dtim;	/*delta time, lsb=0.145usec*/
   ushort d_mbndx;	/*index of corresponding main beam time*/
   ushort d_idx;	/*expected interrogation index*/
   ushort d_pidx;	/*prp index*/
}; 

struct mbtim {
   ulong m_tim;		/*interrogation time, lsb=0.145usec*/ 
   ushort m_mod;	/*interrogation mode*/
};
#define MBTUSED	0x8000	/*in m_mod => this interrog used in a sequence*/

#define R_TENT	1	/*bits in r_typ*/
#define R_PRED	2
#define R_INTF	4
#define R_PRPC	8	/*potential prp pattern change*/
#define R_CCOMPL 0x100	/*prp cycle is complete*/

struct rdbuf {		/*radar data buffer, xfer data from interrogation
			 * recognizers to burst handlers*/
   ulong r_itim;	/*time of reference prp, lsb=0.145usec*/
   ulong r_ctim;	/*beam center time, lsb=0.145usec*/
   ulong r_cycm;	/*most sig half of cycle length, lsb=0.145usec
			 * cycle length is sum of all prps */
   ulong r_cycl;	/*least sig half of cycle length, lsb=33.76asec*/
   ushort r_typ;	/*radar type*/
   short r_nhit;	/*number of central hits*/
   short r_slhit;	/*number of sidelobe hits*/
   struct sigtbl *r_ste; /*corresponding signature table pntr or NULL*/
   short r_nprp;	/*stagger length*/
   ushort r_prplst[MAXSTG]; /*prps, lsb=0.145usec*/
   ushort r_87dif;	/*ASR9 diff between 8 step & 7 step cycle duration*/
   ushort r_87idx;	/*ASR9 index of 1st of pair to combine*/
   char r_mode[NINDX];	/*interrogation modes, [0] for r_itim, [1] for next*/
   short r_nmod;	/*repeat length of interrogation modes*/
   ushort r_87difl;	/*.16 ASR9 8 step & 7 step cycle difference 2.23psec*/
   ushort r_87typ;	/*.A03 ASR9 type, 7/6=>1, 7/7=>2, 8/7=>3, 8/8=>4*/
   short r_ndup;	/*.A03 largest # copies - 1 of a prp*/
   short r_87difx;	/*.A03 for 8/8 step, delta first different prp .145us*/
#define r_95len r_87difx	/*A.06 length of short stagger addon to ASR9*/
};
struct mbstat {		/*main beam hit statistics*/
   time_t m_uctim;	/*current unix calendar time*/
   ulong m_stime;	/*accumulation start time, lsb=0.145usec*/
   ulong m_ctime;	/*current time, lsb=0.145usec*/
   int m_chcnt;		/*cummulative correlated hit count*/
   int m_hcnt;		/*cummulative hit count*/
   int m_altitud;	/*altitude in hundreds of feet*/
   int m_ovltim;	/*cummulative overload time, lsb=9.5msec*/
   double m_utctim;	/*A.19 UTC from GPS 1PPS (like unix time)*/
   int m_ppscnt;	/*A.24 cummulative 1PPS */
   int m_appscnt;	/*A.24 cummulative accepted 1PPS */
   int m_rppscnt;	/*A.24 cummulative rejected 1PPS */
   int m_cslscnt;	/*C.02 cummulative (10sec) correlated sls count*/
   int m_slscnt;	/*C.02 cummulative (10sec) sls count*/
   int m_crpycnt;	/*C.02 cummulative (10sec) correlated reply count*/
   int m_rpycnt;	/*C.02 cummulative (10sec) reply count*/
};
struct cutime {		/*A.19 time setting history*/
   double ct_gpst;	/*A.19 UTC from GPS*/
   ulong ct_itim;	/*A.19 internal time 145ns*/
   time_t ct_utim;	/*A.19 unix time at this internal time*/
   int ct_status;	/*A.19 state of this entry*/
};

#define CUTHIST 5	/*A.19 history of assoc UTC with 145ns clock*/
#define CT_UNK	 1	/*A.19 unix time unknown at this internal time*/
#define CT_XTRAP 2	/*A.19 unix time extrapolated for this internal time*/
#define CT_CONSI 4	/*A.19 unix time consistant for this internal time*/
#define MHST 6		/*amount of history, current plus 5 prev*/
#define NSCLS 4		/*number of radar classes*/
#define S_TENT	1	/*bits in s_cls = tentative*/
#define S_PRED	2	/*predictable radar*/
#define S_INTF	4
#define S_LCKD	8	/*locked radar*/
#define S_RECOG	0x10	/*recog tentative radar, ok scan per but prp unstable*/
#define S_EXPD	0x20	/*expanded display desired*/
#define S_MLCK	0x40	/*manual lock request*/
#define S_ALCK	0x80	/*auto lock request*/
#define S_CCOMPL 0x100	/*prp cycle is complete*/
#define S_SIGNIF 0x8000	/*a significant change has been made*/
#define S_LOCKED 0x4000 /*predictable radar which is locked*/
#define S_LCKDLY 0x2000 /*ASR9 to permit addit rotation before try relock*/
#define S_LCKING 0x1000	/*.17 ASR9 lockon not yet verified*/
#define QMASK	S_MLCK|S_EXPD

struct sigtbl {		/*signature table, info on recognized radars*/
   ushort s_cls;	/*radar classification info*/
   short s_ste;		/*signature table entry number*/
   char	s_htab;		/*horizontal position on display*/
   char	s_vtab;		/*vertical position on display*/
   ulong s_mbcnt;	/*number of mbeam bursts received*/
   ulong s_mbmis;	/*number of consecutive mbeam bursts missed*/
   short s_mbnhit;	/*number of hits in main beam*/
   ulong s_mbcent;	/*main beam center time, lsb=0.145usec*/
   ulong s_sper;	/*scan period, lsb=0.145usec*/
   char s_mode[NINDX];	/*interrogation modes, [0] for s_mbitim, [1] for next*/
   short s_nmod;	/*repeat length of interrogation modes*/
   ulong s_frc;		/*fractional instability, msb = 0.5 */
   ushort s_rcnts;	/*A.15 short cumm total reply count (name change only)*/
   ushort s_crcnt;	/*cummulative correlated reply count*/
   ushort s_trcnt;	/*cummulative target report count*/
   ushort s_racnt;	/*cummulative ring around target reports*/
   ushort s_fbcnt;	/*cummulative false bracket target reports*/
   ushort s_slcnt;	/*cummulative sidelobe target reports*/
   ushort s_upcnt;	/*cummulative track update count*/
   ushort s_cplhit;	/*cummulative hits contributing to phase lock*/
   short s_nprp;	/*stagger length*/
   ulong s_cycm;	/*most sig half of cycle length, lsb=0.145usec
			 * cycle length is sum of all prps */
   ulong s_cycl;	/*least sig half of cycle length, lsb=33.76asec*/
   ushort s_prpm[MAXSTG]; /*msh prps, lsb=0.145usec*/
   ushort s_prpl[MAXSTG]; /*lsh prps, lsb=2.213psec*/
   ushort s_87idx;	/*ASR9 index of 1st of pair to combine*/
   ushort s_87nt;	/*ASR9 total number of stag cycles, ref->ref*/
   ushort s_87ns;	/*ASR9 number of short stag cycles, ref->ref*/
   ulong s_87difm;	/*ASR9 diff bet 8 step & 7 step cycle durat .145us*/
   ulong s_87difl;	/*ASR9 diff bet 8 step & 7 step cycle durat 33.76asec*/
   ulong s_itim[MHST];	/*burst reference time history, lsb=0.145usec*/
   short s_nhit[MHST];	/*number of hits in burst history*/
   ulong s_cent[MHST];	/*burst center time history, lsb=0.145usec*/
   ulong s_mbitim;	/*main beam reference time, lsb=0.145usec*/
   short s_ict;		/*number of interrogations, reference to reference*/
   ushort s_thit[MHST];	/*number of hits in scan & history*/
   ulong s_pmbst;	/*earliest next main beam start time, lsb=0.145usec*/
   ulong s_pmben;	/*latest next main beam end time, lsb=0.145usec*/
   char s_name[4];	/*locked radar mnemonic*/
   double s_dist;	/*projected distance from active to passive site*/
   double s_sdist;	/*slant distance from active to passive site*/
   double s_azim;	/*angle wrt North of active from passive, radians*/
   double s_xr;		/*x position of radar wrt passive*/
   double s_yr;		/*y position of radar wrt passive*/
   double s_zr;		/*z position of radar wrt passive*/
   double s_zpasiv;	/*altitude of passive site*/
   double s_altalt;	/*altitude of ref altimeter (m)*/
   double s_refalt;	/*altitude from ref altimeter, offset removed (m)*/
   ushort s_87typ;	/*.A03 ASR9 type, 7/6=>1, 7/7=>2, 8/7=>3, 8/8=>4*/
   short s_ndup;	/*.A03 largest # copies - 1 of a prp*/
   short s_95len;	/*A.23 length of short stagger addon to ASR9*/
/*A.23 #define s_95len s_87difx	/*A.06 length of short stagger addon to ASR9*/
   ulong s_rcnt;	/*A.15 long cummulative total reply count*/
   double s_mbutc;	/*A.19 floating UTC at mb center (like unix time)*/
   ulong s_pltm0;	/*A.23 time of 1st contrib to s_cplhit in rot*/
   ulong s_pltm1;	/*A.23 time of last contrib to s_cplhit in rot*/
   short s_cplmis;	/*A.23 misses during accumulation of s_cplhit*/
   short s_87difx;	/*A.23 for 8/8 step, delta first different prp .145us*/
   short s_cplsmis;	/*.29 max sequential misses in a rotation*/
   short s_angsmis;	/*.29 angle of max sequential misses degree*/
};

struct psigtbl {	/*partial signature table, info on recognized radars*/
   ushort s_cls;	/*radar classification info*/
   short s_ste;		/*signature table entry number*/
   char	s_htab;		/*horizontal position on display*/
   char	s_vtab;		/*vertical position on display*/
   ulong s_mbcnt;	/*number of mbeam bursts received*/
   ulong s_mbmis;	/*number of consecutive mbeam bursts missed*/
   short s_mbnhit;	/*number of hits in main beam*/
   ulong s_mbcent;	/*main beam center time, lsb=0.145usec*/
   ulong s_sper;	/*scan period, lsb=0.145usec*/
   char s_mode[NINDX];	/*interrogation modes, [0] for s_mbitim, [1] for next*/
   short s_nmod;	/*repeat length of interrogation modes*/
   ulong s_frc;		/*fractional instability, msb = 0.5 */
   ushort s_rcnts;	/*A.15 short cumm total reply count (name change only)*/
   ushort s_crcnt;	/*cummulative correlated reply count*/
   ushort s_trcnt;	/*cummulative target report count*/
   ushort s_racnt;	/*cummulative ring around target reports*/
   ushort s_fbcnt;	/*cummulative false bracket target reports*/
   ushort s_slcnt;	/*cummulative sidelobe target reports*/
   ushort s_upcnt;	/*cummulative track update count*/
   ushort s_cplhit;	/*cummulative hits contributing to phase lock*/
};			/*MUST DUPLICATE regular sigtbl to here*/

struct cov2 {			/* 2 by 2 covariance matrix (symmetrical)*/
   double p_11;
   double p_12;
   double p_22;
};

struct cov4 {			/* 4 by 4 covariance matrix (symmetrical)*/
   double p_11;
   double p_12;
   double p_13;
   double p_14;
   double p_22;
   double p_23;
   double p_24;
   double p_33;
   double p_34;
   double p_44;
};

#define SDINFO	1		/*d_stat bit on => correlated target report*/
#define SDPOS	2		/*d_stat bit on => has a valid position*/
#define SDASIGN	4		/*d_stat bit on => assigned to a track*/
#define SDFPOS	8		/*d_stat bit on => has a valid fixed-up pos*/
struct scandb {
   ulong d_stat;		/*status, &1 => has info, &2 => assigned*/
   int d_hit;			/*total replies contributing*/
   int d_toa;			/*uncorrected toa, lsb=1meter*/
   ulong d_daz;			/*differential azimuth, bams*/
   ulong d_time;		/*observation time, lsb=0.145usec*/
   int d_runl;			/*max run length*/
   ushort d_mode[NMODE];	/*mode A & mode C replies*/
   ushort d_mask[NMODE];	/*bit by bit confidence, on => confident*/
   double d_x;			/*meters*/
   double d_y;			/*meters*/
   double d_z;			/*meters*/
   struct cov2 d_cxy;		/*meters^2 */ 
};

#define TKUPDAT 1		/*t_stat => track updated*/
#define TKPNT	2		/*t_stat => track has one position meas*/
#define TKTRK	4		/*t_stat => track has position tracking*/
#define TKMODEA 0		/*index for mode A interrog in t_mode & t_mask*/
#define TKMODEC 1		/*index for mode C interrog in t_mode & t_mask*/
struct trtrak {			/*target track*/
   ulong t_stat;		/*status*/
   ulong t_astat;		/*altitude status*/
   ushort t_ser;		/*serial number of this track entry*/
   ushort t_tkno;		/*arbitrary track # for this track entry*/
   ushort t_fcnt;		/*number of times this track found*/
   ushort t_acnt;		/*number of times valid altitude found*/
   struct trtrak *t_nxt;	/*link to next entry (free or occupied)*/
   ulong t_time;		/*observation time, lsb=0.145usec*/
   ulong t_ptim;		/*position observation time, lsb=0.145usec*/
   ulong t_atim;		/*altitude observation time, lsb=0.145usec*/
   ushort t_mode[NMODE];	/*mode A & mode C replies*/
   ushort t_mask[NMODE];	/*bit by bit confidence, on => confident*/
   double t_x;			/*meters*/
   double t_y;			/*meters*/
   double t_vx;			/*meters/sec*/
   double t_vy;			/*meters/sec*/
   double t_ap;			/*accel parallel to velocity, m^2/sec */
   double t_at;			/*accel transverse to velocity, m^2/sec */
   double t_cova[SSIZ*SSIZ];	/* 6 by 6 X-Y covariance matrix*/
   double t_def[NPOT];		/*defect (miss^2/variance) for a target report*/
   double t_astate[ASIZ];	/*altitude state vector*/
   double t_acov[ASIZ*ASIZ];	/* 3 by 3 altitude covariance matrix*/
   struct scandb *t_sd[NPOT];	/*pointer to a target report*/
   struct scandb t_m0;		/*most recent measurement*/
   struct scandb t_m1;		/*next most recent measurement*/
   struct scandb t_m2;		/*3rd newest measurement*/
   ushort t_acode;		/* possible mode A override*/
   ushort t_aocnt;		/* # sequential mode A override votes*/

};

struct mat2 {			/* 2 by 2 matrix */
   double p_11;
   double p_12;
   double p_21;
   double p_22;
};

struct mat4 {			/* 4 by 4 matrix */
   double p_11;
   double p_12;
   double p_13;
   double p_14;
   double p_21;
   double p_22;
   double p_23;
   double p_24;
   double p_31;
   double p_32;
   double p_33;
   double p_34;
   double p_41;
   double p_42;
   double p_43;
   double p_44;
};

struct mat24 {			/* 2 wide by 4 high matrix */
   double p_11;
   double p_12;
   double p_21;
   double p_22;
   double p_31;
   double p_32;
   double p_41;
   double p_42;
};

/*stuff for message queues*/
#define MSQ_C		'C'	/*message queue c */
#define MSQ_D		'D'	/*message queue d */
#define MSQ_E		'E'	/*.05 (spare) message queue e */
/*A.35 Mode S message queue keys*/
#define MSQ_SA	('S'<<8|'A')	/*A.35 write queue to Mode S data process*/
#define MSQ_SB	('S'<<8|'B')	/*A.35 write queue to alt correlator process*/
#define MSQ_SC	('S'<<8|'C')	/*.44  write queue to adsb aux process*/

/*A.12 the following message types are used by com_outA:
 * M_CNT_C M_COMPRES M_DFILE M_FLTLIVE M_FLTNO M_FLTSEND M_FLTSTATUS M_FLUSH
 * M_INIT M_INTERACTIVE M_MBEAM M_QUIT M_RADAR MR_FULL MR_PART M_RADARS
 * M_REALTIME M_RUNL M_SEND M_STATUS M_TARGET M_TEXT M_TIME M_WWV M_WWVLIVE
 */

#define M_ICAO24	('I'<<8|'A')	/*.42 icao24 code vs xponder*/
#define M_RADAR		('M'<<8|'A')	/*radar report*/
#define MR_FULL		('M'<<8|'f')	/*full signature table entry*/
#define MR_PART		('M'<<8|'p')	/*partial signature table entry*/
#define M_MBEAM		('M'<<8|'B')	/*radar report*/
#define M_CNT_C		('M'<<8|'C')	/*input control C*/
#define M_RTCTCLS	('M'<<8|'D')	/*A.07 track classification*/
#define M_ECHO		('M'<<8|'E')	/*input echo command*/
#define M_FLUSH		('M'<<8|'F')	/*input flush command*/
#define M_SENDPASS	('M'<<8|'G')	/*A.18 pass on tagged radar data*/
/*A.18 #define M_SPARE1	('M'<<8|'G')	/*A.07 a spare command*/
#define M_FIRDA		('M'<<8|'H')	/*.40 klm firda flight data*/
/*.40 #define M_SPARE2	('M'<<8|'H')	/*A.07 a spare command*/
#define M_INTERACTIVE	('M'<<8|'I')	/*input interactive command*/
#define M_AFLTLIVE	('M'<<8|'J')	/*A.17 send ASD flight numb live*/
#define M_AFLTSEND	('M'<<8|'K')	/*A.17 send stored ASD flight numb*/
#define M_PASSIT	('M'<<8|'L')	/*A.17 pass on a message to comm line*/
#define M_TIME		('M'<<8|'M')	/*input time command*/
#define M_SEND		('M'<<8|'N')	/*input send track command*/
#define M_SENDRGN	('M'<<8|'O')	/*A.17 restrict target deliv region*/
#define M_PRINT		('M'<<8|'P')	/*info to be printed*/
#define M_QUIT		('M'<<8|'Q')	/*quit command*/
#define M_REALTIME	('M'<<8|'R')	/*input realtime command*/
#define M_STATUS	('M'<<8|'S')	/*input status command*/
#define M_TARGET	('M'<<8|'T')	/*target report*/
#define M_SPARE3	('M'<<8|'U')	/*A.18 a spare command*/
#define M_SENDMBTM	('M'<<8|'V')	/*A.19 send MB time in target points*/
#define M_TEXT		('M'<<8|'X')	/*text for output*/
#define M_INIT		('M'<<8|'Z')	/*reinitialize*/
#define M_PLOT		('M'<<8|'a')	/*plot from the dfile*/
#define M_RADARS	('M'<<8|'b')	/*input radars command*/
#define M_ASDPNT	('M'<<8|'c')	/*A.13 measured point from ASD*/
#define M_ASDDEL	('M'<<8|'d')	/*A.16 delete an ASD flight*/
#define M_ASDTRK	('M'<<8|'e')	/*A.16 info about an ASD flight*/
#define M_DFILE		('M'<<8|'f')	/*send dfile pointers*/
#define M_REALTIMET	('M'<<8|'g')	/*.41 send realtime with time included*/
#define M_COMPRESS	('M'<<8|'h')	/*.14 target data report compression*/
#define M_HIST		('M'<<8|'i')	/*.17 extract restarts from hstfile*/
#define M_RUNL		('M'<<8|'j')	/*.17 send run length & hits*/
#define M_RTCETA	('M'<<8|'k')	/*A.04 ETA message from rtcalcs*/
#define M_KLMCCIS	('M'<<8|'l')	/*.45 KLM CCIS message*/
#define M_RTCRATE	('M'<<8|'m')	/*A.04 Rate message from rtcalcs*/
#define M_RTFLTNO	('M'<<8|'n')	/*A.04 Flight Numb vs beacon msg*/
#define M_RTCETAS	('M'<<8|'o')	/*A.04 Summary ETA msg from rtcalcs*/
#define M_FLTNO		('M'<<8|'p')	/*A.04 a flight nmb info report*/
#define M_RTCSPACEFIC	('M'<<8|'q')	/*.46 spacing efficiency info /minute*/
#define M_FLTLIVE	('M'<<8|'r')	/*A.04 send flight numb info live*/
#define M_FLTSEND	('M'<<8|'s')	/*A.04 send stored flight numb info*/
#define M_TARGA		('M'<<8|'t')	/*.12 augmented targ report(flt #)*/
#define M_FLTSTATUS	('M'<<8|'t')	/*A.04 send status of flight numb info*/
#define M_RTCNTQH	('M'<<8|'u')	/*A.04 quarter hour cnts from rtcalcs*/
#define M_SETARPT	('M'<<8|'v')	/*A.06 set the airport*/
#define M_WWVLIVE	('M'<<8|'w')	/*A.06 send raw WWV time reports*/
#define M_WWV		('M'<<8|'x')	/*A.06 raw WWV time report*/
#define M_FLTNC		('M'<<8|'y')	/*A.09 a flight nmb string report*/
#define M_RTCSPACING	('M'<<8|'z')	/*A.09 arrival spacing measurement*/
#define M_ICAO24SEND	('M'<<8|'1')	/*A.33 send stored icao24 code info*/
#define M_ICAO24LIVE	('M'<<8|'2')	/*A.33 send icao24 code info live*/
#define M_ICAO24STAT	('M'<<8|'3')	/*A.33 send status of icao24 code info*/
#define M_TEXTD		('M'<<8|'4')	/*.50 text without special meaning*/
#define A_UNIREQ	0x41414141	/*A.11 AAAA req for data to Unimatic*/
#define A_SPAREA	0x41414142	/*A.11 AAAB spare*/
#define A_SPAREB	0x41414143	/*A.11 AAAC spare*/
#define A_DEPART	1		/*A.38 departures for this airline*/
#define A_NAIRLINE	8		/*A.38 number of airlines*/
#define A_NASBEACON	50	/*.39 number preassigned beacon + flt id*/

/*A.35 used from psdata to Mode S data process (MSQ_SA)*/
#define MS_DRIVER	('S'<<8|'a')	/*A.35 message from driver*/
#define MS_RESTART	('S'<<8|'b')	/*A.35 psdata restart*/
/*A.35 note: psdata also sends M_RADAR msgs re locked radar to MSQ_SA*/
/*A.35 from Mode S data process to alt correlator process (MSQ_SB)*/
#define MS_IDMSG	('S'<<8|'A')	/*A.35 id event report*/
#define MS_ALTMSG	('S'<<8|'B')	/*A.35 altitude event(s) report*/
/*A.35 note: psdata sends M_TARGET msgs to MSQ_SB*/
/*.44 from Mode S data process to aux (adsb/RA/etc) process (MSQ_SC)*/
#define MS_RAWMSG	('S'<<8|'1')	/*.44 contains filtered raw replies*/
#define MS_RAWMAX	2048		/*.44 MS_RAWMSG buffer size*/

struct m_buf {		/*message for message queue */
   long mtype;		/*message type*/
   long msize;		/*message size excluding mtype*/
   char mtext[RBSIZE];	/*body of message*/
};
struct m_rdbuf {	/*radar message for message queue */
   long mtype;		/*message type*/
   long msize;		/*message size excluding mtype*/
   long mstype;		/*message subtype*/
   char mtext[RBSIZE];	/*body of message*/
};
struct trg_d {		/*target data*/
   ushort m_tkno;	/*track number (arbitrary) */
   ushort m_pnt;	/*point number */
   time_t m_ctim;	/*current calendar time*/
   ushort m_id;		/*beacon code*/
   long m_x;		/* x coord (meters) */
   long m_y;		/* y coord (meters) */
   long m_z;		/* z coord (meters) */
   short m_vx;		/* x velocity (m/s) */
   short m_vy;		/* y velocity (m/s) */
   short m_vz;		/* z velocity (m/s) */
};
/*.12 struct trga_d defined in xtrafm.h where FLTNCH is defined */

struct dbihdr {		/*part of circular buffer header on disk*/
   long i_magic;	/*magic number*/
   long i_size;		/*total size of file*/
   long i_cin;		/*current in pointer*/
};
struct dbohdr {		/*part of circular buffer header on disk*/
   long o_cout;		/*current out pointer*/
   long o_cinp;		/*in pointer the last time out pointer checked*/
};
struct dbhdr {		/*circular buffer header on disk*/
   struct dbihdr i;	/*written only by buffer source*/
   struct dbohdr o;	/*written only by buffer sink*/
};
#define	d_magic	i.i_magic
#define	d_size	i.i_size
#define	d_cin	i.i_cin
#define	d_cout	o.o_cout
#define	d_cinp	o.o_cinp

#define MAXGACODES 10	/*.31 max number of beacon codes for ga a/c*/
#define MAXPARTNER 32	/*.27 max number of airline partners (above airlinx)*/
#define MXVERT 20	/*.26 max number of vertices on airport boundary*/
/*.26#define MXVERT 10	/*max number of vertices on airport boundary*/
#define MXRUN 40	/*max number of runways on airport*/
#define MXRWYNAM 5	/*A.04 max number of chars in a runways name*/
#define MXFIXNAM 9	/*A.04 max number of chars in a fix name*/
#define MXSTNNAM 5	/*A.04 max number of chars in a station name*/
#define MAXLINE 512	/*max number of characters on a parameter line*/
#define MAXUAXCAR 10	/*A.14 max number of UAX carriers*/
#define MXBND 10	/*A.18 max number of distinct boundary/origin sets*/
#define NRADAR 4	/*A.21 max number of different radar locations*/
#define MXSTATN 4	/*A.25 max number of different station names*/

struct pnt2 {		/*a two dimensional point*/
   double x;
   double y;
};
struct pnt3 {		/*a three dimensional point*/
   double x;
   double y;
   double z;
};
struct pnt22 {		/*two - two dimensional point*/
   double x1;
   double y1;
   double x2;
   double y2;
};
struct pnti2 {		/*a two dimensional point stored as integers*/
   int x;
   int y;
};
#define AF_ARRIV 1	/*A.04 arrival type fix (f_fixtyp)*/
#define AF_RANGE 3	/*A.05 fixed range type fix (f_fixtyp)*/
#ifndef SENSOR
struct fixinfo {	/*A.04 arrival & final fixes*/
   double f_cosfa;	/*cos of max angle from airport bet fix & A/C */
   double f_cosah;	/*cos of max angle bet dir to airport & flight path*/
   int f_x;		/*position meters*/
   int f_y;
   int f_minalt;	/*minimum altitude (100ft)*/
   int f_maxalt;	/*maximum altitude (100ft)*/
   int f_fixtyp;	/*type of fix*/
   char f_name[MXFIXNAM+1];	/*name of fix*/
};
struct rwyinfo {	/*A.05 runway info (one for each end)*/
   double r_angra;	/*max angle bet rwy axis & A/C pos (radians)*/
   double r_cosrh;	/*cos of max angle bet rwy axis & flight path*/
   int r_mxtran;	/*max transverse of point from rwy axis*/
   int r_ffdist;	/*distance from final fix to rwy*/
   int r_minalt;	/*minimum altitude at final fix (100ft)*/
   int r_maxalt;	/*maximum altitude at final fix (100ft)*/
   char r_name[MXRWYNAM+1];	/*name of runway end*/
   int r_nvert;		/*A.08 # of vertices for arrivals or departures*/
   struct pnt2 r_bndry[MXVERT];	/*A.08 arriv or depart boundary*/
};
struct markinfo {	/*A.07 arbitrary marks with labels*/
   int f_x;		/*position meters*/
   int f_y;
   char f_name[MXFIXNAM+1];	/*label*/
};
struct llmarkinfo {	/*A.26 arbitrary marks with labels*/
   int f_lat;		/*A.26 latitude in sec of arc*/
   int f_lon;		/*A.26 longitude in sec of arc*/
   int f_rad;		/*A.26 radius in meters*/
   char f_name[MXFIXNAM+1];	/*A.26 label*/
}; /*A.26*/
struct llfixinfo {	/*.48 nav fixes for miles in trail calcs.*/
   double f_lat;	/*.48 latitude in degrees*/
   double f_lon;	/*.48 longitude in degrees*/
   double f_direc;	/*.48 direction of airway in deg wrt true North*/
   double f_uxa;	/*.48 unit vector along airway (x in lon direction)*/
   double f_uya;	/*.48 unit vector along airway (y in lat direction)*/
   int f_adist;		/*.49 dist. along airway for measurement, meters*/
   int f_mtdist;	/*.49 max dist from airway in meters*/
   int f_x;		/*.48 x,y wrt a_lonlat in meters*/
   int f_y;		/*.48 x,y wrt a_lonlat in meters*/
   int f_lalt;		/*.48 lowest altitude (100 ft)*/
   int f_halt;		/*.48 highest altitude (100 ft)*/
   int f_lspeed;	/*.48 lowest speed in meters/sec */
   int f_hspeed;	/*.48 highest speed in meters/sec */
   int f_mx;		/*.49 x,y wrt a_lonlat to make meas. in meters*/
   int f_my;		/*.49 x,y wrt a_lonlat to make meas. in meters*/
   char f_name[MXFIXNAM+1];	/*.48 fix name*/
}; /*A.26*/
#define FLTNCHA 7	/*.39 should match FLTNCH in xtrafm.h*/
#define NAUXI 3		/*.39 auxiliary info H=helo L=local D=depart*/
struct asbeacon {	/*.39 preassigned beacon + flt id*/
   ushort as_nasign;	/*.39 number of sequential beacon + flt id*/
   ushort as_beacon;	/*.39 beacon at start of sequence*/
   char as_flid[FLTNCHA+1];	/*.39 preassignd flight id*/
   char as_auxi[NAUXI+1];	/*.39 auxiliary info H=helo L=local D=depart*/
}; /*.39*/
#endif /*SENSOR*/
struct m12loc {		/*.41 Motorola Oncore M12 GPS*/
   double c_lat;	/*.41 latitude of GPS receiver, degrees from rx*/
   double c_lon;	/*.41 latitude of GPS receiver, degrees from rx*/
   double c_hgt;	/*.41 latitude of GPS receiver, meters from rx*/
   int c_flag;		/*.41 GPS is Motorola M12*/
};			/*.41*/
struct uaxcarr {		/*A.14 UAX carriers*/
   int c_fnlo;			/*A.14 lowest flight number to take*/
   int c_fnhi;			/*A.14 highest flight number to take*/
   int c_fnadd;			/*A.14 flight number addon*/
   char c_nam[MXSTNNAM+1];	/*A.14 acronym for the airline, (GLA)*/
};				/*A.14*/
struct uaxfnrng {		/*A.15 range of Unimatic flt # => UAX*/
   int r_fnlo;			/*A.15 lowest flight number to include*/
   int r_fnhi;			/*A.15 highest flight number to include*/
};
struct aparms {			/*airport & radar parameters*/
   char a_units[80];		/*units of measurement*/
   struct pnt3 a_acorigin;	/*zero of aircraft position*/
   int a_nradar;		/*.21 number of active radar positions*/
   int a_iradar;		/*.21 selected active radar position*/
   struct pnt3 a_radar[NRADAR];		/*.21 position of active radar*/
   struct pnt3 a_pssr;		/*position of PSSR*/
   int a_nouts;			/*number of outer surface vertices*/
   struct pnt2 a_outs[MXVERT];	/*vertices of outer surface*/
   double a_altouts;		/*altitude of outer surface*/
   int a_nins;			/*number of inner surface vertices*/
   struct pnt2 a_ins[MXVERT];	/*vertices of inner surface*/
   double a_altins;		/*altitude of inner surface*/
   int a_nrun;			/*number of runways*/
   struct pnt22 a_run[MXRUN];	/*runway endpoints*/
   double a_altrun;		/*altitude of runways*/
   double a_rperiod;		/*radar rotation period (sec)*/
   int a_nprp[MXTYP];		/*number of prps in a stagger cycle*/
   int a_actvprp[MXTYP];	/*.21 index of active position for this prp*/
   double a_prp[MXTYP][MAXSTG];	/*radar pulse rep periods (usec) */
   long a_dfile;		/*maximum size of data file (creation only)*/
   int a_sendonly;		/*B&K Real-Time Noie Monitoring System*/ 
   int a_sense[NSENSE];		/*values for receiver sensitivities*/
   double a_altalt;		/*altitude of altimeter for corrections*/
   char a_passwd[MAXPASSWD+2];	/*password required at commun startup*/
   struct pnt2 a_lonlat;	/*longitude & latitude of origin*/
   char a_mapr[MAXMAPR+2];	/*map region suffix*/
   char a_notes[MAXNOTES+2];	/*notes for documentation*/
   int a_iwdwth;		/*interrogation window width, 3-29 */
   int a_nfixes;		/*.07 number of navigation fixes*/
#ifndef SENSOR
   struct pnti2 a_fixes[MAXFIXS]; /*.10 navigation fixes*/
#endif /*SENSOR*/
   int a_nrangc;		/*.07 number of range circles*/
   double a_rrangc;		/*.07 radius of inner of range circle*/
   int a_maxtwd;		/*.09 max target window .145us (9000=>192km)*/
   int a_nregions;		/*.10 number of regions*/
#ifndef SENSOR
   int a_region[SPACE_REGION];	/*.10 sequences => regions*/
#endif /*SENSOR*/
   struct pnt2 a_rangcent;	/*.10 center of range circles*/
   int a_nlclrwy;		/*.10 number of local runways*/
   int a_crestrot;		/*.11 # rots to wait before check reset*/
   int a_cresttarg;		/*.11 # targets to acquire before check reset*/
   int a_crestrun;		/*.11 min frac good runs to not reset*/
   int a_crestalt;		/*.11 min frac good alts to not reset*/
   char a_tz[MAXTZ];		/*.13 timezone of PASSUR*/
   int a_mbint;			/*.14 1=> disable MB ints while locked*/
   int a_timout;		/*.14 safety net alarm period*/
   int a_hctime;		/*.15 time between history statistic reports*/
   int a_debug[NMBDEBUG];	/*.15 debug state (independent classes)*/
   int a_spare1;
   int a_spare2;
   int a_spare3;
   int a_spare4;
   int a_spare5;
   int a_maxrngfin;		/*A.04 max range to be on final (m)*/
   int a_minaltfin;		/*A.04 min altitude on final (100ft)*/
   int a_maxaltfin;		/*A.04 max altitude on final (100ft)*/
   int a_maxrngbas;		/*A.04 max range to be on base (m)*/
   int a_minaltbas;		/*A.04 min altitude on base (100ft)*/
   int a_maxaltbas;		/*A.04 max altitude on base (100ft)*/
   int a_mintrnbas;		/*A.04 min transverse dist on base (m)*/
   int a_maxtrnbas;		/*A.04 max transverse dist on base (m)*/
   int a_nafixes;		/*A.04 number of arrival & other fixes*/
   int a_nrwyn;			/*A.05 number of runways with addit info*/
   char a_station[MXSTATN][MXSTNNAM+1];	/*A.25 airport names, upper case, SFO*/
   char a_airline[MXSTNNAM+1];	/*A.04 acronym for the airline, (ual)*/
#ifndef SENSOR
   struct fixinfo a_afixes[MAXFIXS];	/*A.04 arrival & other fixes*/
   struct rwyinfo a_rwyinfo[MXRUN][2];	/*A.05 info about rwy end & final fix*/
   int a_msq1;			/*A.06 message queue for this arpt to xtrafm*/
   int a_msq2;			/*A.06 message queue for this arpt to rtcalcs*/
   int a_msq3;			/*A.06 message queue for this arpt to exporter*/
   int a_portno;		/*A.06 well known port for passur server*/
   char a_server[MAXLINE+1];	/*A.06 name of server for this arpt on network*/
#endif /*SENSOR*/
   int a_npluses;		/*A.07 number of plus marks*/
   int a_nexes;			/*A.07 number of X marks*/
   int a_nnomark;		/*A.07 number of labels without marks*/
#ifndef SENSOR
   struct markinfo a_pluses[MAXFIXS]; /*A.07 plus marks*/
   struct markinfo a_exes[MAXFIXS];   /*A.07 X marks*/
   struct markinfo a_nomark[MAXFIXS]; /*A.07 labels without marks*/
#endif /*SENSOR*/
   int a_asd;			/*.08 handling ASD data*/
   int a_nxregions;		/*A.09 number of exclusion regions*/
   int a_nuaxcarr;		/*A.14 number of UAX carriers*/
   struct uaxcarr a_uaxcarr[MAXUAXCAR];	/*A.14 UAX carriers*/
#ifndef SENSOR
   int a_xregion[SPACE_REGION];	/*A.09 sequences => exclusion regions*/
#endif /*SENSOR*/
   int a_delay;			/*A.12 additional delay of 1090 (meters)*/
   char a_airlinx[MXSTNNAM+1];	/*A.15 acronym for airline express, (UAX)*/
   struct uaxfnrng a_uaxfnrng;	/*A.15 range of Unimatic flt # => UAX*/
   char a_airfnmb[MXSTNNAM+1];	/*A.15 acronym for other airline, (FNB)*/
   int a_nrpchn;		/*A.15 # entries in reply chain for trgcorr*/
   int a_mbhkill;		/*A.17 kill if # MB/10sec exceeds this*/
   int a_slskill;		/*A.17 kill if # SLS/10sec exceeds this*/
   int a_rpykill;		/*A.17 kill if # RPY/10sec exceeds this*/
   int a_nbndry[MXBND];		/*A.18 number of boundary vertices*/
   struct pnt3 a_origin[MXBND];	/*A.18 zero of aircraft position*/
   struct pnt2 a_bndry[MXBND][MXVERT];	/*A.18 vertices of boundary*/
   int a_nllpluses;		/*A.26 number of lat/lon plus marks*/
   int a_nllexes;		/*A.26 number of lat/lon X marks*/
   int a_nllnomark;		/*A.26 number of lat/lon labels without marks*/
   int a_nllcircle;		/*A.26 number of lat/lon circles*/
   int a_npartner;		/*A.27 number of partner airlines*/
   int a_asfnfsiz;		/*A.28 size of asfltfile (default ASFNFSIZ1)*/
   int a_astkfsiz;		/*A.28 size of asdtkfil (default ASTKFSIZ1)*/
   int a_mxhfsiz;		/*A.28 max size of history file*/
#ifndef SENSOR /*A.26*/
   struct llmarkinfo a_llpluses[MAXLLFIX]; /*A.26 lat/lon plus marks*/
   struct llmarkinfo a_llexes[MAXLLFIX];   /*A.26 lat/lon X marks*/
   struct llmarkinfo a_llnomark[MAXLLFIX]; /*A.26 lat/lon labels without marks*/
   struct llmarkinfo a_llcircle[MAXLLFIX]; /*A.26 lat/lon circles*/
   char a_partner[MAXPARTNER][MXSTNNAM+1]; /*A.27 acronym for partner airline*/
   char a_remdirec[NOMSIZ];	/*A.30 remote non-target data directory*/
   char a_remtdata[NOMSIZ];	/*A.30 remote target data directory*/
#endif /*A.26 SENSOR*/
   int a_gacodes[MAXGACODES+1];	/*.31 beacon codes for ga a/c*/
   ulong a_mbcolt;	/*.32 time to write mb to file, ticks, 0=>forever*/
   int a_nlarg;			/*.34 number of large surface vertices*/
   char a_mapl[MAXMAPR+2];	/*.34 large map region suffix*/
#ifndef SENSOR /*A.34*/
   struct pnt2 a_larg[MXVERT];	/*.34 vertices of large surface*/
#endif /*A.34 SENSOR*/
/*A.35 Mode-S parameters */
   int a_mssensiro[MS_NIRO];	/*A.35 h/w sensitivities (int/rep/omni)*/
   int a_msintdly;		/*A.35 h/w interrogation delay param*/
   int a_msacthr;		/*A.35 all-call targ qualif threshold*/
   int a_msdpurgif[MS_NIF];	/*A.35 data purge secs (inactivity/freq)*/
   int a_msidchglim;		/*A.35 permissible id changes per minute*/
   int a_msdii;			/*A.35 radar ii*/
   int a_msdspare0;		/*A.35 data app spare*/
   int a_mscfsiz;		/*A.35 correl output file size*/
   int a_mscpurgif[MS_NIF];	/*A.35 correl purge secs (inactivity/freq)*/
   int a_mscalttol;		/*A.35 correl altitude tolerance(ft)*/
   int a_msccorpct;		/*A.35 min pct agreement for correlation*/
   int a_msccorpctc;		/*A.35 min ct for testing correl pct*/
   int a_msccoruniq;		/*A.35 min unique alts for correlation*/
   int a_mscdecpct;		/*A.35 pct agreement thresh for decorrelation*/
   int a_mscdecpctc;		/*A.35 min ct for testing decorrel pctg*/
   int a_mscdecct;		/*A.35 successive disagrees for decorrelation*/
   int a_mscdecsec;		/*A.35 secs of contin disagree for decorrel*/
   int a_mscspare0;		/*A.35 correl app spare*/
#ifndef SENSOR /*A.38*/
   char a_airlin3[MXSTNNAM+1];	/*A.36 acronym for 3rd airline*/
   char a_airlin4[MXSTNNAM+1];	/*A.36 acronym for 4th airline*/
   char a_airlin5[MXSTNNAM+1];	/*A.36 acronym for 5th airline*/
   char a_airlin6[MXSTNNAM+1];	/*A.36 acronym for 6th airline*/
   char a_airlin7[MXSTNNAM+1];	/*A.36 acronym for 7th airline*/
   int a_airstat[A_NAIRLINE];	/*A.38 status of each airline, e=0 x=1 3=3*/
   struct asbeacon a_asbeac[A_NASBEACON]; /*.39 preassigned beacon + flt id*/
#endif /*A.38 SENSOR*/
   struct m12loc a_m12loc;	/*.41 Motorola Oncore M12 GPS*/
   int a_nrepb;		/*.43 number of longs in repbuf*/
   int a_ntrtrak;	/*.43 number of target track entries for trgtrak*/
   int a_nlldfixes;	 /*.48 # of departure fixes for miles in trl*/
   int a_nllafixes;	 /*.48 # of arrival fixes for miles in trl*/
#ifndef SENSOR /*47*/
   char *a_shapefiles;	/*.47 names of shapefiles*/
   struct llfixinfo a_lldfixes[MAXFIXS]; /*.48 depart fixes for miles in trl*/
   struct llfixinfo a_llafixes[MAXFIXS]; /*.48 arrival fixes for miles in trl*/
#endif /*47 SENSOR*/
};
/*.14 message types for compressed target data transmissions*/
#define T_SRA	1	/*.14 compressed send reports info*/
#define T_RTA	2	/*.14 compressed realtime target reports*/
#define T_SRB	3	/*A.01 compressed send reports info with runlength*/

struct plsdif {			/*.15 ASR9 combined prp time intervals*/
   long d_tim;			/*.15 interpulse time difference*/
   char d_strt;			/*.15 start pulse index*/
   char d_end;			/*.15 end pulse index*/
   char d_nmb;			/*.15 number of intervals*/
};

struct hstat {		/*.15 history statistics*/
   time_t h_stime;	/*accumulation start time, seconds*/
   int h_chcnt;		/*cummulative correlated hit count*/
   int h_hcnt;		/*cummulative hit count*/
   int h_ovltim;	/*cummulative overload time, lsb=9.5msec*/
   int h_rcnt;		/*cummulative total reply count*/
   int h_crcnt;		/*cummulative correlated reply count*/
   int h_trcnt;		/*cummulative target report count*/
   int h_upcnt;		/*cummulative track update count*/
   int h_mbnhit;	/*number of hits in main beam*/
   int h_cplhit;	/*cummulative hits contributing to phase lock*/
   int h_nprp;		/*stagger length*/
   int h_mbcnt;		/*number of mbeam bursts received*/
   int h_wfrc;		/*worst fractional instability, msb = 0.5 */
   int h_plcnt;		/*mbeam burst count at lock on, 0 if not locked*/
   int h_rbocnt;	/*.22 count of reply buffer overflo*/
   int h_rfcnt;		/*.22 count of reply chain unavailable*/
   int h_tfcnt;		/*.22 count of no free tracks*/
   int h_nfcnt;		/*.22 count of no free track numbers*/
   int h_ntrk;		/*.22 current number of tracks*/
   int h_cplmis;	/*A.23 number of missed pris over span of s_cplhit*/
   int h_scplmis;	/*A.23 cumm # missed pris*/
   int h_nmismes;	/*A.23 # meas of cumm # missed pris*/
   int h_avcplmis;	/*A.23 avg # missed pris over span of s_cplhit*/
   int h_ppscnt;	/*A.24 cummulative 1PPS */
   int h_appscnt;	/*A.24 cummulative accepted 1PPS */
   int h_rppscnt;	/*A.24 cummulative rejected 1PPS */
   int h_cplsmis;	/*.29 max sequential misses*/
   int h_angsmis;	/*.29 angle of misses, degrees*/
   int h_cslscnt;	/*C.02 cummulative (10sec) correlated sls count*/
   int h_slscnt;	/*C.02 cummulative (10sec) sls count*/
   int h_crpycnt;	/*C.02 cummulative (10sec) correlated reply count*/
   int h_rpycnt;	/*C.02 cummulative (10sec) reply count*/
};
#endif /*A.30 RADARH */
