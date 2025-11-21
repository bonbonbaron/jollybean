#ifndef MAP_H
#define MAP_H

#include "data/array.h"
#include "data/bitfield.h"

// Maps 
// A bit's position in a bitfield is the Key to the map. The number of high bits behind it is
// the index of the key's value in the array.

typedef struct {
	Key key;
	void *valueP;
} KeyValPair;

typedef struct {
  Key nKeyValPairs;
  KeyValPair *keyValA;
} KeyValPairArray;

typedef enum {
  RAW_DATA, 
  ARRAY,
  NONMAP_POINTER,
  MAP_POINTER,
  FUNCTION_POINTER,
  DOUBLE_POINTER,
  N_MAP_ELEM_TYPES
} MapElemType;

typedef struct {
	StaticBitfieldArray* bfaP;  // this holds the bits representing which keys have been set 
  MapElemType elemType;  // is element a pointer? Double pointer? Raw data?
  Key    population;
	void  *mapA;  
} Map;

Map* mapNew( MapElemType elemType, const U8 elemSz, const Key nElems, const PoolId poolId);
void mapSet(Map *mapP, const U8 key, const void *valP);
void* mapGet(const Map *mapP, const Key key);
void mapRem(Map *mapP, const Key key);
Key mapGetIndex(const Map *mapP, const Key key);
void mapCopyKeys(Map *dstMP, Map *srcMP);
Map* mapGetNestedMapP(Map *mapP, Key mapKey);
void* mapGetNestedMapPElem(Map *mapP, Key mapKey, Key elemKey, MapElemType expectedElemType);
U32 mapHasKey(const Map* mP, const Key key );


#endif  // #ifndef MAP_H
