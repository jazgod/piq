#ifndef _LOOKUP3_H_
#define _LOOKUP3_H_

#ifdef WIN32
#ifndef uint8_t
typedef unsigned char uint8_t;
#endif
#ifndef uint16_t
typedef unsigned short uint16_t;
#endif
#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

uint32_t hashword(
const uint32_t *k,                /* the key, an array of uint32_t values */
size_t          length,           /* the length of the key, in uint32_ts */
uint32_t        initval);         /* the previous hash, or an arbitrary value */

void hashword2 (
const uint32_t *k,                /* the key, an array of uint32_t values */
size_t          length,           /* the length of the key, in uint32_ts */
uint32_t       *pc,               /* IN: seed OUT: primary hash value */
uint32_t       *pb);              /* IN: more seed OUT: secondary hash value */

uint32_t hashlittle( const void *key, size_t length, uint32_t initval);

void hashlittle2( 
  const void *key,       /* the key to hash */
  size_t      length,    /* length of the key */
  uint32_t   *pc,        /* IN: primary initval, OUT: primary hash */
  uint32_t   *pb);       /* IN: secondary initval, OUT: secondary hash */

uint32_t hashbig( const void *key, size_t length, uint32_t initval);

#define hashsize(n) ((uint32_t)1<<(n))
#define hashmask(n) (hashsize(n)-1)

#else
#include <stdint.h>     /* defines uint32_t etc */
#include <sys/param.h>  /* attempt to define endianness */
#endif

#endif//_LOOKUP3_H_