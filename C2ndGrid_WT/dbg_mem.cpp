#include <windows.h>
#include <stdio.h>
#include <map>
#include <string>
using namespace std;

#define LINEBUF_SIZE 1024

typedef pair<string, unsigned int> MEMINFO;
typedef map<void*, MEMINFO> MEMMAP;
typedef MEMMAP::iterator MEMMAP_ITER;

static CRITICAL_SECTION g_csMemLock;
MEMMAP g_mMemory;

void dbg_mem_record(void* m, unsigned int l, const char* s)
{
	EnterCriticalSection(&g_csMemLock);
	g_mMemory[m] = MEMINFO(s,l);
	LeaveCriticalSection(&g_csMemLock);

	#ifdef DEBUG_CONSOLE
		OutputDebugString(s);
	#endif//DEBUG_CONSOLE
}

void dbg_mem_remove(void* m, const char* s)
{
	EnterCriticalSection(&g_csMemLock);
	MEMMAP_ITER i = g_mMemory.find(m);
	if(i != g_mMemory.end()){
		g_mMemory.erase(i);
	}
	LeaveCriticalSection(&g_csMemLock);
	
	#ifdef DEBUG_CONSOLE
		OutputDebugString(s);
	#endif//DEBUG_CONSOLE
}

extern "C"
{

extern char* strcat_alloc(char*, char*);
extern char* str_falloc(char*);
extern void* vo_create(int firstarg, ... );
extern void* vo_malloc(size_t);
extern int vo_free(void*);
extern int vp_free(void*);
extern void s_free(char*);

void dbg_mem_init()
{
	InitializeCriticalSection(&g_csMemLock);	
}

void dbg_mem_complete()
{
	char s[LINEBUF_SIZE]={0};
	DWORD dwSize = 0L;
	for(MEMMAP_ITER i = g_mMemory.begin(); i != g_mMemory.end(); i++){
		MEMINFO d = i->second;
		dwSize += d.second;
		OutputDebugString(d.first.c_str());
	}
	g_mMemory.clear();
	DeleteCriticalSection(&g_csMemLock);
	
	sprintf_s(s, LINEBUF_SIZE, "%s(%d):(0x%X) %ld allocated bytes were never freed\n", __FILE__, __LINE__, GetCurrentThreadId(), dwSize);
	OutputDebugString(s);
}


// These functions now dump output to DevStudio Output Window instead of vo_log
char* dbg_strcat_alloc(char* s1, char* s2, char* file, int line)
{
	char *ret;
	void *save = s1;
	char s[LINEBUF_SIZE]={0};
	ret = strcat_alloc(s1, s2);
	sprintf_s(s, LINEBUF_SIZE, "%s(%d):(0x%X) strcat_alloc(%x) = %x\n", file, line, GetCurrentThreadId(), save, ret );
	dbg_mem_record(ret, strlen(ret)+1, s);
	dbg_mem_remove(save, s);
	return (char*)ret;
}
char* dbg_str_falloc(char* cs, char* file, int line)
{
	void *ret;
	char s[LINEBUF_SIZE]={0};
	ret = str_falloc(cs);
	sprintf_s(s, LINEBUF_SIZE, "%s(%d):(0x%X) str_falloc(%x)\n", file, line, GetCurrentThreadId(), ret );
	dbg_mem_record(ret, strlen(cs)+1, s);
	return (char*)ret;
}
void *dbg_vo_malloc(size_t size, char* file, int line)
{
	void *ret;
	char s[LINEBUF_SIZE]={0};
	ret = vo_malloc(size);
	sprintf_s(s, LINEBUF_SIZE, "%s(%d):(0x%X) vo_malloc(%d) = %x\n", file, line, GetCurrentThreadId(), size, ret);
	dbg_mem_record(ret, size, s);
	return ret;
}
void *dbg_vo_create(size_t size, void* p, char* file, int line)
{
	void *ret;
	char s[LINEBUF_SIZE]={0};
	ret = vo_create(size, p);
	sprintf_s(s, LINEBUF_SIZE, "%s(%d):(0x%X) vo_create(%d) = %x\n", file, line, GetCurrentThreadId(), size, ret);
	dbg_mem_record(ret, size, s);
	return ret;
}
int dbg_vo_free(void *ptr, char* file, int line)
{
	char s[LINEBUF_SIZE]={0};
	sprintf_s(s, LINEBUF_SIZE, "%s(%d):(0x%X) vo_free(%x)\n", file, line, GetCurrentThreadId(), ptr);
	dbg_mem_remove(ptr, s);
	return vo_free(ptr);
}
void *dbg_calloc(size_t nmemb, size_t size, char* file, int line)
{
	void *ret;
	char s[LINEBUF_SIZE]={0};
	ret = calloc(nmemb, size);
	sprintf_s(s, LINEBUF_SIZE, "%s(%d):(0x%X) calloc(%d,%d) = %x\n", file, line, GetCurrentThreadId(), nmemb, size, ret);
	dbg_mem_record(ret, nmemb * size, s);
	return ret;
}
void *dbg_malloc(size_t size, char* file, int line)
{
	void *ret;
	char s[LINEBUF_SIZE]={0};
	ret = malloc(size);
	sprintf_s(s, LINEBUF_SIZE, "%s(%d):(0x%X) malloc(%d) = %x\n", file, line, GetCurrentThreadId(), size, ret);
	dbg_mem_record(ret, size, s);
	return ret;
}
void dbg_s_free(char *ptr, char* file, int line)
{
	char s[LINEBUF_SIZE]={0};
	sprintf_s(s, LINEBUF_SIZE, "%s(%d):(0x%X) s_free(%x)\n", file, line, GetCurrentThreadId(), ptr);
	dbg_mem_remove(ptr, s);
	free(ptr);
}

void dbg_vp_free(void *ptr, char* file, int line)
{
	char s[LINEBUF_SIZE]={0};
	sprintf_s(s, LINEBUF_SIZE, "%s(%d):(0x%X) vp_free(%x)\n", file, line, GetCurrentThreadId(), ptr);
	dbg_mem_remove(ptr, s);
	free(ptr);
}

void dbg_free(void *ptr, char* file, int line)
{
	char s[LINEBUF_SIZE]={0};
	sprintf_s(s, LINEBUF_SIZE, "%s(%d):(0x%X) free(%x)\n", file, line, GetCurrentThreadId(), ptr);
	dbg_mem_remove(ptr, s);
	free(ptr);
}
void *dbg_realloc(void *ptr, size_t size, char* file, int line)
{
	void *ret;
	void* save_ptr;
	char s[LINEBUF_SIZE]={0};
	save_ptr = ptr;
	ret = realloc(ptr, size);
	sprintf_s(s, LINEBUF_SIZE, "%s(%d):(0x%X) realloc(%x,%d) = %x\n", file, line, GetCurrentThreadId(), save_ptr, size, ret );
	dbg_mem_record(ret, size, s);
	dbg_mem_remove(save_ptr, s);
	return ret;
}
char *dbg_strdup(const char *cs, char* file, int line)
{
	void *ret;
	char s[LINEBUF_SIZE]={0};
	ret = _strdup(cs);
	sprintf_s(s, LINEBUF_SIZE, "%s(%d):(0x%X) strdup(%x) = %x\n", file, line, GetCurrentThreadId(), cs, ret );
	dbg_mem_record(ret, strlen(cs)+1, s);
	return (char*)ret;
}

} // extern "C"