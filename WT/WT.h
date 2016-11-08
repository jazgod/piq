#ifndef __WT_H_
#define __WT_H_

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif


typedef void (*GETDLLVERSION)(DWORD* pdwMajor, DWORD* pdwMinor);
typedef void (*INITIALIZE)(VOID* pvInitStruct);
typedef void (*STARTUP)();
typedef void (*SHUTDOWN)();
typedef void (*CALLBACK)();

#ifdef __cplusplus
}
#endif


#endif//__WT_H_
