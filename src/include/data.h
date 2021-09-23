#ifndef DATA_H
#define DATA_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define N_FLAG_BYTES (32)  /* This times 8 is the number of items JB's hash map can hold. Increase as necessary. */
#define LAST_FLAG_BYTE_IDX (N_FLAG_BYTES - 1)
#define N_FLAG_BITS (8 * N_FLAG_BYTES)

typedef unsigned char U8;
typedef char S8;
typedef unsigned short U16;
typedef short S16;
typedef unsigned int U32;
typedef int S32;

#include "errors.h"

/* Basic utils */
Error jbAlloc(void **voidPP, U8 elemSz, U8 nElems);
void  jbFree(void **voidPP);
extern U8 bitCountLUT[];
extern U8 byteIdxLUT[];

/* Arrays */
Error arrayNew(void **arryPP, U32 elemSz, U32 nElems);
void arrayDel(void **arryPP);
U32 arrayGetNElems(const void *arryP);
U32 arrayGetElemSz(const void *arryP);
void arrayIniPtrs(const void *arryP, void **startP, void **endP, S32 endIdx);

/* Maps */
/* helpful macros for defining hard-coded key-value pairs in map seeds; also mitigates ugly "(void*)", null-termination, and "{..., ...}" syntax */
#define SET_(key, val) {key, (void*) &val}
#define MAP_END_ {T_NONE, NULL}
#define MAP_(...) {__VA_ARGS__, MAP_END_}
#define NUM_ARGS_(_type, ...) sizeof((_type[]){__VA_ARGS__}) / sizeof(_type)
#define HARD_CODED_MAP_(_type, ...) { \
	sizeof(_type), \
	NUM_ARGS_(KeyValPair, __VA_ARGS__), \
	NULL, /* prevents multiple copies */ \
	MAP_(__VA_ARGS__) \
}

typedef struct {
	U8 prevBitCount;
	U8 flags;
} FlagInfo;

typedef U8 Key;
typedef struct {
	Key key;
	void *valueP;
	U8 _valSz;      /* ***IMPORTANT: _valSz requires the value to be in the SAME FILE as map definition! */
} KeyValPair;

typedef struct {
	FlagInfo flagA[N_FLAG_BYTES];  /* "A" means "Array" for JB's naming standards */
	void  *mapA;  
} Map;

typedef struct {
	U32         _elemSz;
	U32         _nKeyValPairs;
	Map        *mapP;       /* defaults to NULL to prevent copies */
	KeyValPair  keyValA[];
} HardCodedMap;

Error mapNew(Map **mapPP, const U8 elemSz, const U16 nElems);
void  mapDel(Map **mapPP);
Error mapIni(Map **mapPP, HardCodedMap *hcMapP);   /* from an array of KeyValPairs */
Error mapSet(Map *mapP, const U8 key, const void *valP);
void* mapGet(const Map *mapP, const U8 key);

/* Histograms */
Error histoU8New(U32 **histoPP, const U8 *srcA, const U32 maxVal);
void histoU8Del(U32 **histoPP);

#endif
