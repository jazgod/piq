
#ifndef DEFS_H
#define DEFS_H

/* common typedefs */
typedef char    Bool;
typedef unsigned char Byte;
typedef unsigned short Word;
typedef unsigned long DWord;

/* common constants */
#ifndef NULL
#define NULL        0		/* NULL pointer */
#endif

#ifndef TRUE
#define TRUE        1		/* TRUE value */
#endif

#ifndef FALSE
#define FALSE       0		/* FALSE value */
#endif

#ifndef PI
#define PI           3.14159265358979323846	/* PI number */
#endif

#ifndef DBL_MAX
#define DBL_MAX      1.7976931348623157E+308	/* Maximum double value */
#endif

/* useful indices */
#define LO          0
#define HI          1

/* useful macros */
#define FOREVER     for ( ; ; )
#define Max(A,B)    ((A>B) ? A : B)
#define Min(A,B)    ((A<B) ? A : B)
#define Swap(A,B)   (A)^=(B), (B)^=(A), (A)^=(B);
#define Signum(A)   ((A>0) ? 1 : ((A==0) ? 0 : -1))

#endif

