#ifndef MAP_H
#define MAP_H

#include "array.h"

#define byteIdx_(key) ((key - 1) >> 3)
#define bitFlag_(key) (1 << ((key - 1) & 0x07))

// Maps 
// A bit's position in a bitfield is the Key to the map. The number of high bits behind it is
// the index of the key's value in the array.
// If you know you're only going to put four things in the map, you'll only need one byte.
// If you're putting more than 255 things in there, you'll need to typedef Key to U16 or,
// if you're crazy, U32. TODO: Implement a map with offset to prevent overly large maps.
typedef struct {
	Key prevBitCount;
	Key flags;  
} FlagInfo;

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
	FlagInfo flagA[N_FLAG_BYTES];  // "A" means "Array" for JB's naming standards 
  MapElemType elemType;  // is element a pointer? Double pointer? Raw data?
  Key    population;
	void  *mapA;  
} Map;

Map* mapNew( MapElemType elemType, const U8 elemSz, const Key nElems);
void  mapDel(Map **mapPP);
void mapSet(Map *mapP, const U8 key, const void *valP);
void* mapGet(const Map *mapP, const U8 key);
void mapRem(Map *mapP, const Key key);
Key mapGetIndex(const Map *mapP, const Key key);
void  mapSetFlag(Map *mapP, const Key key);
void mapCopyKeys(Map *dstMP, Map *srcMP);
Map* mapGetNestedMapP(Map *mapP, Key mapKey);
void* mapGetNestedMapPElem(Map *mapP, Key mapKey, Key elemKey, MapElemType expectedElemType);
void  mapOfNestedMapsDel(Map **outerMapPP);
void  mapOfNestedMapsClr(Map **outerMapPP);


#endif  // #ifndef MAP_H
