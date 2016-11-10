#ifndef _SSLTHREADSUPPORT_H_
#define _SSLTHREADSUPPORT_H_

#ifdef __cplusplus 
extern "C"{
#endif 

void openssl_thread_setup( );
void openssl_thread_cleanup( );

#ifdef __cplusplus 
}
#endif 

#endif//_SSLTHREADSUPPORT_H_