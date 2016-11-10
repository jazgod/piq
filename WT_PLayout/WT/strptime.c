#ifdef WIN32
#include < windows.h >
//#pragma comment ( lib, "Volib.lib" )-no need.
//#pragma comment ( lib, "MSVCRTD.lib" )
//08-04-03
#define strncasecmp _strnicmp
#define strcasecmp stricmp
#define usleep Sleep
//Win32:08-11-03
#define bcopy(A,B,C) memmove(B,A,C)
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vo_db.h"
#include <fcntl.h>
#include <varargs.h>
#include <sys/timeb.h>
#include <time.h>
#include <string.h>
#include "vo.h"
#include "vo_extern.h"
#include "strptime.h"


#ifndef HAVE_STRPTIME


#define asizeof(a)      (sizeof (a) / sizeof ((a)[0]))


#ifndef sun
struct dtconv {
        char    *abbrev_month_names[12];
        char    *month_names[12];
        char    *abbrev_weekday_names[7];
        char    *weekday_names[7];
        char    *time_format;
        char    *sdate_format;
        char    *dtime_format;
        char    *am_string;
        char    *pm_string;
        char    *ldate_format;
};
#endif

static struct dtconv    En_US = {
        { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
          "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" },
        { "January", "February", "March", "April",
          "May", "June", "July", "August",
          "September", "October", "November", "December" },
        { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" },
        { "Sunday", "Monday", "Tuesday", "Wednesday",
          "Thursday", "Friday", "Saturday" },
        "%H:%M:%S",
        "%m/%d/%y",
        "%a %b %e %T %Z %Y",
        "AM",
        "PM",
        "%A, %B, %e, %Y"
};

#ifdef SUNOS4
extern int      strncasecmp();
#endif


extern void StrLower( char *str );


int DtStr2Secs( str , date_fmt )
     char *str;
     char *date_fmt;
{
  time_t secs;
  char *stat_str;
  struct tm tm;

  memset( &tm, 0, sizeof(tm));

  if (!str || !date_fmt || !strlen(str) ){
    return(-1);
  }

  /* check date_fmt if specified */
  if ( date_fmt && strlen(date_fmt) ){
    /* convert SYBASE time into tm struct */

    //Win32:08-11-03
    //stat_str = strptime(str, date_fmt, &tm );
    stat_str = strptime(str, date_fmt, &tm );

    /* check fields for invalid numbers */
    if ( stat_str && !strlen( stat_str ) ){
      /* valid conversion */
      /* convert year in next century */
      if ( tm.tm_year < 70 ){
	    tm.tm_year += 100;
      }
      tm.tm_isdst = -1;
      secs = mktime( &tm );
      return( (int) secs );
    } else {
      return( -1 );
    }

  }
  return( -1 );
}




char    *
strptime(char *buf, char *fmt, struct tm *tm)
{
  char    c,
    *ptr;
  int     i, j,
    len;
  ptr = fmt;
  while (*ptr != 0) {
    if (*buf == 0)
      break;
    
    c = *ptr++;
    
    if (c != '%') {
      if (isspace(c))
        while (*buf != 0 && isspace(*buf))
          buf++;
        else if (c != *buf++)
          return 0;
        continue;
    }
    
    c = *ptr++;
    switch (c) {
    case 0:
    case '%':
      if (*buf++ != '%')
        return 0;
      break;
      
    case 'C':
      buf = strptime(buf, En_US.ldate_format, tm);
      if (buf == 0)
        return 0;
      break;
      
    case 'c':
      buf = strptime(buf, "%x %X", tm);
      if (buf == 0)
        return 0;
      break;
      
    case 'D':
      buf = strptime(buf, "%m/%d/%y", tm);
      if (buf == 0)
        return 0;
      break;
      
    case 'R':
      buf = strptime(buf, "%H:%M", tm);
      if (buf == 0)
        return 0;
      break;
      
    case 'r':
      buf = strptime(buf, "%I:%M:%S %p", tm);
      if (buf == 0)
        return 0;
      break;
      
    case 'T':
      buf = strptime(buf, "%H:%M:%S", tm);
      if (buf == 0)
        return 0;
      break;
      
    case 'X':
      buf = strptime(buf, En_US.time_format, tm);
      if (buf == 0)
        return 0;
      break;
      
    case 'x':
      buf = strptime(buf, En_US.sdate_format, tm);
      if (buf == 0)
        return 0;
      break;
      
    case 'j':
      if (!isdigit(*buf))
        return 0;
      
      for (i = 0; *buf != 0 && isdigit(*buf); buf++) {
        i *= 10;
        i += *buf - '0';
      }
      if (i > 365)
        return 0;
      
      tm->tm_yday = i;
      break;
      
    case 'M':
    case 'S':
      if (*buf == 0 || isspace(*buf))
        break;
      
      if (!isdigit(*buf))
        return 0;
      
      for (j=0,i = 0; *buf != 0 && isdigit(*buf) && j<2; j++,buf++) {
        i *= 10;
        i += *buf - '0';
      }
      if (i > 59)
        return 0;
      
      if (c == 'M')
        tm->tm_min = i;
      else
        tm->tm_sec = i;
      
      if (*buf != 0 && isspace(*buf))
        while (*ptr != 0 && !isspace(*ptr))
          ptr++;
        break;
        
    case 'H':
    case 'I':
    case 'k':
    case 'l':
      if (!isdigit(*buf))
        return 0;
      
      for (j=0,i = 0; *buf != 0 && isdigit(*buf) && j<2; j++,buf++) {
        i *= 10;
        i += *buf - '0';
      }
      if (c == 'H' || c == 'k') {
        if (i > 23)
          return 0;
      } else if (i > 11)
        return 0;
      
      tm->tm_hour = i;
      
      if (*buf != 0 && isspace(*buf))
        while (*ptr != 0 && !isspace(*ptr))
          ptr++;
        break;
        
    case 'p':
      len = strlen(En_US.am_string);
      // 7jan2004 mhm: replace with existing routine
						//  lowercase_string( buf );
						StrLower( buf );
            if (strncasecmp(buf, En_US.am_string, len) == 0) {
              if (tm->tm_hour > 12)
                return 0;
              if (tm->tm_hour == 12)
                tm->tm_hour = 0;
              buf += len;
              break;
            }
            
            len = strlen(En_US.pm_string);
            
            if (strncasecmp(buf, En_US.pm_string, len) == 0) {
              if (tm->tm_hour > 12)
                return 0;
              if (tm->tm_hour != 12)
                tm->tm_hour += 12;
              buf += len;
              break;
            }
            
            return 0;
            
    case 'A':
    case 'a':
      for (i = 0; i < asizeof(En_US.weekday_names); i++) {
        len = strlen(En_US.weekday_names[i]);
        
        // mhm: 7jan2004 lowercase_string( buf );
								StrLower( buf );
                if (strncmp(buf,
                  En_US.weekday_names[i],
                  len) == 0)
                  break;
                
                len = strlen(En_US.abbrev_weekday_names[i]);
                if (strncmp(buf,
                  En_US.abbrev_weekday_names[i],
                  len) == 0)
                  break;
      }
      if (i == asizeof(En_US.weekday_names))
        return 0;
      
      tm->tm_wday = i;
      buf += len;
      break;
      
    case 'd':
    case 'e':
      if (!isdigit(*buf))
        return 0;
      
      for (j=0,i = 0; *buf != 0 && isdigit(*buf) && j<2; j++,buf++) {
        i *= 10;
        i += *buf - '0';
      }
      if (i > 31)
        return 0;
      
      tm->tm_mday = i;
      
      if (*buf != 0 && isspace(*buf))
        while (*ptr != 0 && !isspace(*ptr))
          ptr++;
        break;
        
    case 'B':
    case 'b':
    case 'h':
      for (i = 0; i < asizeof(En_US.month_names); i++) {
        len = strlen(En_US.month_names[i]);
        
        //lowercase_string( buf );
								StrLower( buf );
                if (strncmp(buf, En_US.month_names[i],len) == 0)
                  break;
                
                len = strlen(En_US.abbrev_month_names[i]);
                if (strncmp(buf,
                  En_US.abbrev_month_names[i],
                  len) == 0)
                  break;
      }
      if (i == asizeof(En_US.month_names))
        return 0;
      
      tm->tm_mon = i;
      buf += len;
      break;
      
    case 'm':
      if (!isdigit(*buf))
        return 0;
      
      for (j=0,i = 0; *buf != 0 && isdigit(*buf) && j<2; j++,buf++) {
        i *= 10;
        i += *buf - '0';
      }
      if (i < 1 || i > 12)
        return 0;
      
      tm->tm_mon = i - 1;
      
      if (*buf != 0 && isspace(*buf))
        while (*ptr != 0 && !isspace(*ptr))
          ptr++;
        break;
        
    case 'Y':
    case 'y':
      if (*buf == 0 || isspace(*buf))
        break;
      
      if (!isdigit(*buf))
        return 0;
      
      for (j=0,i = 0; *buf != 0 && isdigit(*buf) && j<((c=='Y')?4:2); j++,buf++) {
        i *= 10;
        i += *buf - '0';
      }
      
      if (c == 'Y')
        i -= 1900;
      else if (i < 69) /*c=='y', 00-68 is for 20xx, the rest is for 19xx*/
        i += 100;
      
      if (i < 0)
        return 0;
      
      tm->tm_year = i;
      
      if (*buf != 0 && isspace(*buf))
        while (*ptr != 0 && !isspace(*ptr))
          ptr++;
        break;
    }
  }
        
  return buf;
}

#endif   /* ndef HAVE_STRPTIME */
