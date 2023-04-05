#ifndef DATA_H
#define DATA_H
//#include <bits/pthreadtypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef unsigned char U8;
typedef signed char S8;
typedef unsigned short U16;
typedef signed short S16;
typedef unsigned int U32;
typedef signed int S32;
typedef U8 Key;
typedef U8 Bln;   // Boolean

#define N_BITS_PER_BYTE (8)
#define N_BITS_PER_WORD (N_BITS_PER_BYTE * 4)

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
#define KEY_MIN (1)
#define KEY_MAX ((1 << (sizeof(Key) * 8)) - 1)
#define arrayNElems_(x_) (sizeof(x_) / sizeof(x_[0]))

#define swap_(a, b) {a^=b; b^=a; a^=b;}

typedef enum Error {
	SUCCESS,
	E_BAD_ARGS,
	E_NO_MEMORY,
  E_SEGFAULT,
  E_INFLATION_FAILED,
  E_FILE_IO,
  E_BAD_DATA,
	E_BAD_INDEX,
	E_BAD_KEY,
  E_MAP_FULL,
  E_MAP_WRONG_ELEM_TYPE,
	E_MSG_TO_ECS_TYPE_MISMATCH,
	E_MSG_TO_ID_MISMATCH,
	E_MSG_INVALID_CMD,
	E_UNEXPECTED_DCMP_SZ,
	E_UNSUPPORTED_PIXEL_FORMAT,
	E_SYS_CMP_MISMATCH,
  E_ENTITY_NOT_IN_SYSTEM,
  E_BAD_COMPONENT_TYPE,
	E_NULL_VAR,
	E_FRAY_FULL,
  E_MAILBOX_BAD_RECIPIENT,
  E_MAILBOX_BAD_COMMAND,
  E_MAILBOX_BAD_ARG,
  E_MAILBOX_BAD_ATTN,
  E_BB_GENES_DONT_DO_INITIALIZATION,
  E_NULL_GENE_DATA,
  E_INVALID_GENE_CLASS,
  E_PAUSE,
  E_UNPAUSE,
  E_QUIT
} Error;

// Basic utils 
Error jbAlloc(void **voidPP, U32 elemSz, U32 nElems);
void  jbFree(void **voidPP);
extern U8 bitCountLUT[];
extern U8 byteIdxLUT[];

// Arrays 
typedef struct {
	U32         elemSz;
	U32         nElems;
	void        *arrayP;       // defaults to NULL to prevent copies 
} HardCodedArray;

// Unfortunately this doubles the sizes of arrays, but it's safe. 
#define HardCodedArray_(type_, nElems_, array_) { \
  .elemSz_ = sizeof(type_),\
  .nElems = nArgs_(type_, array_),\
	array_, /* prevents multiple copies */ \
}

Error arrayNew(void **arryPP, U32 elemSz, U32 nElems);
Error array2dNew(void **arryPP, U32 w, U32 h, U32 elemSz, U32 nElems);
Error hcArrayIni(HardCodedArray *hcaP);
Error arrayIni(void **arryPP, HardCodedArray *hcaP);
void arrayDel(void **arryPP);
U32 arrayGetNElems(const void *arryP);
U32 arrayGetElemSz(const void *arryP);
void arrayIniPtrs(const void *arryP, void **startP, void **endP, S32 endIdx);
void* arrayGetVoidElemPtr(const void *arryP, S32 idx);
Error arraySetVoidElem(void *arrayP, U32 idx, const void *elemSrcP);

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

Error mapNew(Map **mapPP, MapElemType elemType, const U8 elemSz, const Key nElems);
void  mapDel(Map **mapPP);
Error mapSet(Map *mapP, const U8 key, const void *valP);
void* mapGet(const Map *mapP, const U8 key);
Error mapGetIndex(const Map *mapP, const Key key, Key *idxP);
void  mapSetFlag(Map *mapP, const Key key);
Error mapCopyKeys(Map *dstMP, Map *srcMP);
Error mapGetNestedMapP(Map *mapP, Key mapKey, Map **mapPP);
Error mapGetNestedMapPElem(Map *mapP, Key mapKey, Key elemKey, MapElemType expectedElemType, void **returnedItemPP);
void  mapOfNestedMapsDel(Map **outerMapPP);

/* Binary trees
 * ============
 * Binary trees in JB are implemented by tacking headers onto the elements of otherwise normal arrays.
 *   Using headers prevents single-child-per-level trees from growing to > 2^256 bytes.
 * Every node's 4-byte header is initialized to 0x33333333.
 *   This takes advantage of the impossibility of identical, initialized children.
 *   It uses that to say "This node is an orphan!" AKA "This node has no parents or children yet!"
 *
 */

typedef enum { LEFT_CHILD, RIGHT_CHILD } Child;

// Orphan-hood (having neither parents nor children) 
//   is determined when both children are identical non-zeros. 
// The rest is just because I'm a dork.
#define ORPHAN_BYTE_ (0x33)
#define ORPHAN_ (\
    /* ORPHAN_BYTE_ << 24 | */ \
    ORPHAN_BYTE_ << 16 | \
    ORPHAN_BYTE_ <<  8 | \
    ORPHAN_BYTE_)
#define btIsAnOrphan_(btP_, idx_) ((*((U32*) &btP_[(idx_)].header) & 0x00ffffff) == ORPHAN_)
#define btHasChild_(btElHeaderP_, child_) btElHeaderP_->childA[child_]  /* if nonzero, non-orphan has childn*/
#define btEraseHeader_(btElHeaderP_) memset(btElHeaderP_, 0, sizeof(BtElHeader))
#define btLinkNodes_(btP_, parentP_, childP_, child_) \
  (parentP_)->header.childA[child_] = (childP_) - btP_;\
  btEraseHeader_(&(childP_)->header);\
  (childP_)->header.parent = (parentP_) - btP_;

typedef struct {
  Key            /* these all need to be the same type */
    childA[2],   // children of this element
    parent;      // parent of this element
} BtElHeader;

Error btNew(void **btAP, U32 elemSz, U32 nElems);
#define btDel_ arrayDel

// Histograms 
Error histoNew(U32 **histoPP, const U32 maxVal);
void histoDel(U32 **histoPP);

// Inflatables 
typedef struct {
	U32 compressedLen;
	U32 inflatedLen;
	void *inflatedDataP;
	U8  *compressedDataA;
} Inflatable;

Error inflatableIni(Inflatable *inflatableP);
void inflatableClr(Inflatable *inflatableP);

// Efficient Arrays (frays)
#define N_PREFRAY_ELEMS (5)
#define OFFSET_INACTIVE     (5)  /* ptr[0] */
#define OFFSET_N_PAUSED     (4)  /* ptr[1] */
#define OFFSET_1ST_EMPTY    (3)  /* ptr[2] */
#define OFFSET_ELEM_SZ      (2)  /* ptr[3] */
#define OFFSET_N_ELEMS      (1)  /* ptr[4] */
#define frayGetNElems_ arrayGetNElems
#define frayGetElemSz_ arrayGetElemSz
#define frayGetElemByIdx_ _fast_arrayGetElemByIdx

Error frayNew(void **fPP, U32 elemSz, U32 nElems);
void  frayDel(void **frayPP);
Error frayAdd(const void *frayP, void *elemP, U32 *elemNewIdxP);
U32   frayActivate(const void *frayP, U32 idx);
U32   frayDeactivate(const void *frayP, U32 idx);
U32   frayPause(const void *frayP, U32 idx);
U32   frayUnpause(const void *frayP, U32 idx);
U8    frayElemIsActive(const void *frayP, U32 idx);
void  frayActivateAll(const void *frayP);
void  frayUnpauseAll(const void *frayP);
void  frayDeactivateAll(const void *frayP);

// Pointers beat values. We usually inc/decrement it after using it. Avoids double-queries.
inline U32 _frayGetFirstInactiveIdx(const void *frayP) {
  return *(((U32*) frayP - OFFSET_INACTIVE));
}

inline U32* _frayGetFirstInactiveIdxP(const void *frayP) {
  return ((U32*) frayP - OFFSET_INACTIVE);
}

inline U32* _frayGetNPausedP(const void *frayP) {
  return ((U32*) frayP - OFFSET_N_PAUSED);
}

inline U32 _frayGetFirstPausedIdx(const void *frayP) {
  return *_frayGetFirstInactiveIdxP(frayP) - *_frayGetNPausedP(frayP);
}

inline U32* _frayGetFirstEmptyIdxP(const void *frayP) {
  return ((U32*) frayP - OFFSET_1ST_EMPTY);
}

inline void _frayClr(void *fP) {
  memset(((U32*) fP) - N_PREFRAY_ELEMS, 0, sizeof(U32) * 3);
  /*  The above has this effect:
      *_frayGetFirstEmptyIdxP(fP) = 0;
      *_frayGetNPausedP(fP) = 0;
      *_frayGetFirstInactiveIdxP(fP) = 0;
  */
}

inline U8 _frayElemIsActive(const void *frayP, U32 idx) {
  return idx < _frayGetFirstPausedIdx(frayP);
}

inline U8 _frayElemIsPaused(const void *frayP, U32 idx) {
  return idx >= _frayGetFirstPausedIdx(frayP) && idx < _frayGetFirstInactiveIdx(frayP);
}

inline U8 _frayHasRoom(const void *frayP) {
  return (*_frayGetFirstEmptyIdxP(frayP) < frayGetNElems_(frayP));
}

inline void _frayPauseAll(const void *frayP) {
  *_frayGetNPausedP(frayP) = _frayGetFirstInactiveIdx(frayP);
}

inline void _frayUnpauseAll(const void *frayP) {
  *_frayGetNPausedP(frayP) = 0;
}

inline void _frayActivateAll(const void *frayP) {
  U32 *firstInactiveIdxP = _frayGetFirstInactiveIdxP(frayP);
  if (*_frayGetNPausedP(frayP)) {
    _frayUnpauseAll(frayP);
  }
  *firstInactiveIdxP = *_frayGetFirstEmptyIdxP(frayP);
}

inline void _frayDeactivateAll(const void *frayP) {
  if (*_frayGetNPausedP(frayP)) {
    _frayUnpauseAll(frayP);
  }
  *_frayGetFirstInactiveIdxP(frayP) = 0;
}

// Mailboxes
typedef struct {
	Key address;   // e.g. motion system
	Key attn;      // e.g. specific component in motion system 
	Key cmd;       // e.g. move, [de]activate, or do nothing ( 
	Key arg;       // e.g. change component value to MOVE_UP
} Message;  

Error mailboxWrite(Message *mailboxF, Key address, Key attn, Key cmd, Key arg);
Error mailboxForward(Message *mailboxF, Message *msgP);
typedef Error (*inboxRead)(Message *mailboxF);  // only for self

// Strip Data
typedef U16 StripmapElem;  // Stripmap's inflated data is in U16 format.

typedef struct {
  U8 nUnitsPerStrip;
  U8 bpu;  // bits per unit
  U8 offset;  // amount to increment all units by (e.g. for sub-palette in atlas color palette)
  U32 nUnits;  // e.g. number of pixels in entire stripset
  Inflatable *infP;  // strip set's compressed source data
  U8 *unpackedDataP;  // destination of unpacking step in strip data inflation, if not skipped
} Stripset;

typedef struct {
  U32 nIndices;
  Inflatable *infP;  // infP->inflatedDataP contains stripmap.
} Stripmap;

// Strip Data flags
// prevents biome from double-counting an SD when determining SD array size for inflation
#define SD_IS_COUNTED_        (0x00000001)
// prevents biome from double-inserting an SD into the array for inflation
#define SD_SET_FOR_INFLATION_ (0x00000002)
/* The rest of these are self-explanatory. 
 * For example, systems like XRender want to skip the assembling step so it can assemble them
 * straight into its texture instead. */
#define SD_SKIP_INFLATION_    (0x00000004)
#define SD_SKIP_UNPACKING_    (0x00000008)
#define SD_SKIP_ASSEMBLY_     (0x000000f0)

typedef struct {
  U32 flags;     // used to indicate distinct histo inclusion, flags to skip inflation steps, etc.
  Stripmap sm;
  Stripset ss;
  U8 *assembledDataA;  // destination of strip data's assembling step, if not skipped
} StripDataS;

void stripClr(StripDataS *sdP);
Error stripIni(StripDataS *sdP);
Error sdInflate(StripDataS *sdP);
Error sdUnpack(StripDataS *sdP);
Error sdAssemble(StripDataS *sdP);
Error stripIni(StripDataS *sdP);

// Stitching
#define structMemberOffset_(structType_, memberName_) (U32) &(((structType_*) NULL)->memberName_)
#define structMemberSizeof_(structType_, memberName_) sizeof(((structType_*) NULL)->memberName_)

#endif  // #ifndef DATA_H
