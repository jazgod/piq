#ifndef _DBG_MEM_H_
#define _DBG_MEM_H_
/**
 *==============================================================================
 *
 * Filename:         dbg_mem.h
 *
 *==============================================================================
 *
 *  Copyright 2012  PASSUR Aerospace Inc. All Rights Reserved
 *
 *==============================================================================
 *
 * Description:
 *    Memory allocation/dealloction debug tracking routines
 * Notes:
 *    1) Only use in single instance executions, although thread safe, it is not 
 *    designed for multiple instances (all data is stored in globals)
 *    2) Logging is done using the debug output window (so must be run in debugger
 *    to see results.
 *
 *==============================================================================
 * 
 * Revision History:
 *
 * MCT 6/27/12 Created
 *==============================================================================
 */

#ifdef __cplusplus
extern "C"
{	
#endif

// Set DEBUG_MEMLEAK=1 for strcat_alloc, str_falloc, strdup and free
#define DBG_STRING_ALLOC	1 
// Set DEBUG_MEMLEAK=2 for vo_create and vo_free
#define DBG_VO_CREATE		2
// Set DEBUG_MEMLEAK=4 for vo_malloc and s_free
#define DBG_VO_MALLOC		4
// Set DEBUG_MEMLEAK=8 for vo_calloc and free
#define DBG_CALLOC			8
// Set DEBUG_MEMLEAK=16 for vo_malloc and free
#define DBG_MALLOC			16
// Set DEBUG_MEMLEAK=32 for vo_realloc and free
#define DBG_REALLOC			32
// Set DEBUG_MEMLEAK=63 for all (WARNING, final output is very slow, as we have a lot that we purposefully do not deallocate)
#define DBG_MEMLEAK_ALL		(DBG_STRING_ALLOC|DBG_VO_CREATE|DBG_VO_MALLOC|DBG_CALLOC|DBG_MALLOC|DBG_REALLOC)

// Must define DEBUG_MEMLEAK=# preprocessor def to get replacement functions.
// i.e. For WT Properties add DEBUG_MEMLEAK#DBG_STRING_ALLOC
#ifdef DEBUG_MEMLEAK

void dbg_mem_init();
void dbg_mem_complete();

char* dbg_strcat_alloc(char* s1, char* s2, char* file, int line);
char* dbg_str_falloc(char* cs, char* file, int line);
char *dbg_strdup(const char *cs, char* file, int line);

void *dbg_vo_create(size_t size, void* p, char* file, int line);
int dbg_vo_free(void *ptr, char* file, int line);

void *dbg_vo_malloc(size_t size, char* file, int line);
void dbg_s_free(char *ptr, char* file, int line);
void dbg_vp_free(void *ptr, char* file, int line);

void *dbg_calloc(size_t nmemb, size_t size, char* file, int line);
void *dbg_malloc(size_t size, char* file, int line);
void *dbg_realloc(void *ptr, size_t size, char* file, int line);
void dbg_free(void *ptr, char* file, int line);

#if DBG_STRING_ALLOC  &  DEBUG_MEMLEAK 
#define strcat_alloc(s1, s2) dbg_strcat_alloc(s1, s2, __FILE__, __LINE__)
#define str_falloc(s) dbg_str_falloc(s, __FILE__, __LINE__)
#define strdup(s) dbg_strdup(s, __FILE__, __LINE__)
#endif

#if DBG_VO_CREATE & DEBUG_MEMLEAK 
#define vo_create(size, ptr) dbg_vo_create(size, ptr, __FILE__, __LINE__)
#define vo_free(ptr) dbg_vo_free(ptr, __FILE__, __LINE__)
#endif

#if DBG_VO_MALLOC & DEBUG_MEMLEAK 
#define vo_malloc(size) dbg_vo_malloc(size, __FILE__, __LINE__)
#define s_free(ptr) dbg_s_free(ptr, __FILE__, __LINE__)
#define vp_free(ptr) dbg_vp_free(ptr, __FILE__, __LINE__)
#endif

#if DBG_CALLOC & DEBUG_MEMLEAK 
#define calloc(nmemb, size) dbg_calloc(nmemb, size, __FILE__, __LINE__)
#endif

#if DBG_MALLOC & DEBUG_MEMLEAK 
#define malloc(size) dbg_malloc(size, __FILE__, __LINE__)
#endif

#if DBG_REALLOC & DEBUG_MEMLEAK 
#define realloc(ptr, size) dbg_realloc(ptr, size, __FILE__, __LINE__)
#endif 

#if DBG_CALLOC & DEBUG_MEMLEAK  || DBG_MALLOC & DEBUG_MEMLEAK  || DBG_REALLOC & DEBUG_MEMLEAK || DBG_STRING_ALLOC &  DEBUG_MEMLEAK 
#define free(ptr) dbg_free(ptr, __FILE__, __LINE__)
#endif


#else
#define dbg_mem_init() 
#define dbg_mem_complete()
#endif//DEBUG_MEMLEAK

#ifdef __cplusplus
}
#endif

#endif//_DBG_MEM_H_ 
