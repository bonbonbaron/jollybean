#ifndef JB_H
#define JB_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define N_FLAG_BYTES (1)  /* This times 8 is the number of items JB's hash map can hold. Increase as necessary. */
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

/* Maps */
typedef struct {
	U8 prevBitCount;
	U8 flags;
} FlagInfo;

typedef struct {
	FlagInfo flagA[N_FLAG_BYTES];  /* "A" means "Array" for JB's naming standards */
	void  *mapA;  
} Map;

Error mapNew(Map **mapPP, const U8 elemSz, const U16 nElems);
void  mapDel(Map **mapPP);
Error mapSet(Map *mapP, const U8 key, const void *valP);
void* mapGet(const Map *mapP, const U8 key);

/* Histograms */
Error histoU8New(U32 **histoPP, const U8 *srcA, const U32 maxVal);
void histoU8Del(U32 **histoPP);

#endif
/* how sys starts up:
 * T-TO-C MAPPING
 *		B: entities' behaviors
 *		S: System
 *		C: Cmp
 *		Si: System index
 *		Ci: Cmp index
 *		E: Entity
 *		J: Jagged array mapping triggers to Cs
 *		T: Trigger
 *			"histo T and C"
 *			/0)  make histo() function
 *			1)  histo B's Ts  and alias them in an array the same size as the histo. 
 *			2)  histo Es' components, C in each E's G via their C- headers (for checking system compatibility),
 *			    and alias them in array same size as histo for Si.
 *			"make S array"
 *			3)  allocate array of S based on # nonzeros in (2)
 *			4)  create S based on order of nonzeros and tell those S'es what their aliases are
 *			"make T jagged array"
 *			5)  allocate the base of a jagged array, J with the highest alias assigned in (1) 
 *			6)  rn each element of (5), allocate the count from the histo                  
 *			7)  for each E, go through its G and make J[1.alias][counter++] = {Si, Ci}. 
 *			"make E-to-S:C mapping"
 *			8)  Collision S spits out a msg saying E1 collided with a type. So we go to the E array, index E1, and look at its trigger array. It may not respond to collisions; we don't know yet. A flag could say so, and the number of 1s up to the bit can indicate where in that E's behavior array the response to the T is. This limits an E to 32 Cs. But if the game has more than 32 Cs, you have to have an array of integers. All you have to do is mask out the bits beyond the desired one and use a LUT.. 
 *
 *
 *			This is going to be such an incredible game engine. Here are 2 reasons why:
 *			* Writing behaviors is easy: I just need to give the sys key, function enum, parameter key, and desired output when it's done for entire sets of behaviors. That's programming by tiny data instead of huge, 64-bit machine instructions. 
 *			* Writing entities is going to be easy too, because all I have to do there is write an array of pointers to component maps. 
 *
 *			In terms of development, having pre-existing components to borrow from helps a ton. 
