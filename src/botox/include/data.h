#ifndef DATA_H
#define DATA_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef unsigned char U8;
typedef char S8;
typedef unsigned short U16;
typedef short S16;
typedef unsigned int U32;
typedef int S32;
typedef U8 Key;
typedef U8 Bln;   // Boolean

// For-each macro, which allows per-element macro-processing on variadic arguments
#define N_FLAG_BYTES ((1 << (sizeof(Key) * 8)) / 8)  // This times 8 is the number of items JB's hash map can hold. Increase as necessary. 
#define LAST_FLAG_BYTE_IDX (N_FLAG_BYTES - 1)
#define N_FLAG_BITS (8 * N_FLAG_BYTES)
#define inline __attribute__((always_inline)) __inline
#define unused_(x) (void)(x)
#define enumKeys_(first, ...) typedef enum {first = 1, __VA_ARGS__} 
#define enumIndices_(...) typedef enum {__VA_ARGS__} 
#define nArgs_(type_, ...) sizeof((type_[]){__VA_ARGS__}) / sizeof(type_)
#define TRUE (1)
#define FALSE (0)
typedef enum Error {
	SUCCESS,
	E_BAD_ARGS,
	E_NO_MEMORY,
	E_BAD_INDEX,
	E_BAD_KEY,
	E_MSG_TO_ECS_TYPE_MISMATCH,
	E_MSG_TO_ID_MISMATCH,
	E_MSG_INVALID_CMD,
	E_UNEXPECTED_DCMP_SZ,
	E_UNSUPPORTED_PIXEL_FORMAT,
	E_SYS_CMP_MISMATCH,
  E_BAD_COMPONENT_TYPE,
	E_NULL_VAR,
	E_MAILBOX_FULL,
  E_MAILBOX_BAD_RECIPIENT
} Error;

// Basic utils 
Error jbAlloc(void **voidPP, U32 elemSz, U32 nElems);
void  jbFree(void **voidPP);
extern U8 bitCountLUT[];
extern U8 byteIdxLUT[];

// Arrays 
typedef struct {
	Key _enum;
	void *valP;
} EnumValPair;

typedef struct {
	U32         _elemSz;
	U32         _nEnumValPairs;
	void        *arrayP;       // defaults to NULL to prevent copies 
	EnumValPair  enumValA[];
} HardCodedArray;

// Unfortunately this doubles the sizes of arrays, but it's safe. 
#define HardCodedArray_(_type, ...) { \
	sizeof(_type), \
	nArgs__(EnumValPair, __VA_ARGS__), \
	NULL, /* prevents multiple copies */ \
	{__VA_ARGS__} \
}

Error arrayNew(void **arryPP, U32 elemSz, U32 nElems);
Error hcArrayIni(HardCodedArray *hcaP);
Error arrayIni(void **arryPP, HardCodedArray *hcaP);
void arrayDel(void **arryPP);
U32 arrayGetNElems(const void *arryP);
U32 arrayGetElemSz(const void *arryP);
void arrayIniPtrs(const void *arryP, void **startP, void **endP, S32 endIdx);
void* arrayGetVoidElemPtr(const void *arryP, S32 idx);
Error arraySetVoidElem(void *arrayP, U32 idx, const void *elemSrcP);

// Maps 
#define HardCodedMap_(_type, ...) { \
	sizeof(_type), /* elemSz */  \
	nArgs__(KeyValPair, __VA_ARGS__), /* nElems */  \
	NULL, /* prevents multiple copies */  \
	{__VA_ARGS__} /* key pair array */  \
}

// A bit's position in a bitfield is the Key to the map. The number of bits behind it is
// the index of the key's value in the array.
// If you know you're only going to put four things in the map, you'll only need one byte.
// If you're putting more than 255 things in there, you'll need to typedef Key to U16 or,
// if you're crazy, U32.
typedef struct {
	Key prevBitCount;
	Key flags;  
} FlagInfo;

typedef struct {
	Key key;
	void *valueP;
} KeyValPair;

typedef struct {
	FlagInfo flagA[N_FLAG_BYTES];  // "A" means "Array" for JB's naming standards 
	void  *mapA;  
} Map;

typedef struct {
	Key        _elemSz;
	Key        _nKeyValPairs;
	Map        *mapP;       // defaults to NULL to prevent copies 
	KeyValPair  keyValA[];
} HardCodedMap;

Error mapNew(Map **mapPP, const U8 elemSz, const Key nElems);
void  mapDel(Map **mapPP);
Error mapIni(HardCodedMap *hcMapP);   // from an array of KeyValPairs 
Error mapSet(Map *mapP, const U8 key, const void *valP);
void* mapGet(const Map *mapP, const U8 key);

// Histograms 
Error histoNew(U32 **histoPP, const U32 maxVal);
void histoDel(U32 **histoPP);

// Inflatables 
typedef struct {
	U32 compressedLen;
	U32 inflatedLen;
	void *inflatedDataP;
	U8  compressedData[];
} Inflatable;

Error inflate(Inflatable *inflatableP);

// Communcication
typedef struct {
	Key to;     // e.g. motion system
	Key attn;   // e.g. motion system's translate focus
	Key topic;  // e.g. this is for entity 42
	Key msg;    // e.g. move entity 42 with key FAST_LEFT
} Message;  

typedef struct {
	Key ownerID;
  U16 nMsgs;
  Message *msgA;
} Mailbox;

Error mailboxNew(Mailbox **mailboxPP, Key ownerID, U16 nSlots);
void mailboxClr(Mailbox *mailboxP);
void mailboxDel(Mailbox **mailboxPP);
Error mailboxWrite(Mailbox *mailboxP, Key to, Key attn, Key topic, Key msg);
Error mailboxForward(Mailbox *mailboxP, Message *msgP);
typedef Error (*inboxRead)(Mailbox *mailboxP);  // only for self

// Efficient Arrays (frays)
typedef struct {
  Key           firstInactiveIdx;    /* index of first inactive component */
  Key           firstEmptyIdx;       /* marks the first empty element's index */
  Key           pausedIdx;           /* marks the END of all the paused items in order to return firstInactiveIdx to it upon unpause */
  void         *voidA;               /* array */
} Fray;

Error frayNew(void **fPP, U32 elemSz, U32 nElems);
void  frayDel(void **frayPP);
U32 frayActivate(const void *frayP, U32 idx);
U32 frayDeactivate(const void *frayP, U32 idx);
#endif
