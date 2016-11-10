static char *scid = "@(#)mkgmtime.00.c 03/10/92 jac";
/* Copyright 1989 Megadata Corp, Bohemia NY (unpublished work)*/
/*mkgmtime.00.c 03/10/92 1340 jac:make unix time from GMT (UTC or CUT)
 *mktime.02.c 03/09/92 1800 jac:adjust for timezone before DST decision
 *  else switch for EST occurs at 7AM
 *mktime.01.c 03/02/92 1450 jac:eliminate print LY when adjust for leap year
 *mktime.00.c 12/28/89 1230 jac:Part of ANSI C
 * Converts local time to calendar time.
 * Local time is in the form of struct tm.
 * Calendar time is the number of seconds since Jan 1 1970.
 * This ignores leap seconds.
 */
/*#define TESTING		/*yields a main prog to test this*/

#include <sys/types.h>
#include <time.h>

#define daypyear(x) ((x%4) ? 365 : 366)
static int daypmon[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

long mkgmtime( tp )
struct tm *tp;
{
int year, month, day, hour, minute, second;
long ctim = 0;	/*calendar time*/

year = tp->tm_year + 1900;	/*from years since 1900*/
if(year < 1970) return(-1);	/*cant represent*/
while( --year >= 1970 )		/*add up days per year up to this year*/
   ctim += daypyear(year);
year = tp->tm_year + 1900;	/*from years since 1900*/
month = tp->tm_mon;		/*January is month 0 */
if(month < 0 || month > 11)	/*invalid*/
   return(-1);
if(month > 1 && daypyear(year) == 366) {	/*after Feb in a leap year*/
   /*.01printf(" LY ");*/
   ctim += 1;			/*an extra day*/
}
while( --month >= 0 )		/*add up days per month up to this month*/
   ctim += daypmon[month];
month = tp->tm_mon;		/*January is month 0 */
day = tp->tm_mday;		/*day of month, starts at 1 */
if(day < 1 || day > daypmon[month]) {
   if(day == 29 && daypyear(year) == 366) ;	/*ok*/
   else return(-1);				/*invalid*/
}
ctim += day - 1;		/*add on days up to today*/
ctim *= 24;			/*conv to hours*/
hour = tp->tm_hour;
if(hour < 0 || hour > 23)	/*invalid*/
   return(-1);
ctim += hour;			/*add on hours up to now*/
ctim *= 60;			/*conv to minutes*/
minute = tp->tm_min;
if(minute < 0 || minute > 59)	/*invalid*/
   return(-1);
ctim += minute;			/*add on minutes up to now*/
ctim *= 60;			/*conv to seconds*/
second = tp->tm_sec;
if(second < 0 || second > 59)	/*invalid*/
   return(-1);
ctim += second;			/*add on seconds up to now*/
return(ctim);
}

#ifdef TESTING
main()
{
struct tm ltime;
struct tm tmTemp;
long ctim;

while( scanf("%d %d %d %d %d %d",
   &ltime.tm_year, &ltime.tm_mon, &ltime.tm_mday,
   &ltime.tm_hour, &ltime.tm_min, &ltime.tm_sec) ) {
   printf("%d %02d %02d %02d %02d %02d  ",
   ltime.tm_year, ltime.tm_mon, ltime.tm_mday,
   ltime.tm_hour, ltime.tm_min, ltime.tm_sec);
   ctim = mkgmtime( &ltime );
   printf("SEC=%ld  ", ctim );
   localtime_s( &tmTemp, &ctim );	/*convert back to struct tm */
   printf("%d/%02d/%02d %02d:%02d:%02d  ",
   tmTemp.tm_year, tmTemp.tm_mon, tmTemp.tm_mday,
   tmTemp.tm_hour, tmTemp.tm_min, tmTemp.tm_sec);
   gmtime_s( &tmTemp, &ctim );		/*convert back to struct tm */
   printf("%d/%02d/%02d %02d:%02d:%02dUTC\n",
   tmTemp.tm_year, tmTemp.tm_mon, tmTemp.tm_mday,
   tmTemp.tm_hour, tmTemp.tm_min, tmTemp.tm_sec);
}
}
#endif /*TESTING*/
