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
#define KEY_MIN (1)
#define KEY_MAX ((1 << (sizeof(Key) * 8)) - 1)

#define swap_(a, b) {a^=b; b^=a; a^=b;}

typedef enum Error {
	SUCCESS,
	E_BAD_ARGS,
	E_NO_MEMORY,
  E_FILE_IO,
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
	E_FRAY_FULL,
  E_MAILBOX_BAD_RECIPIENT,
  E_BB_GENES_DONT_DO_INITIALIZATION,
  E_NULL_GENE_DATA,
  E_INVALID_GENE_CLASS
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
// A bit's position in a bitfield is the Key to the map. The number of bits behind it is
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

typedef struct {
	FlagInfo flagA[N_FLAG_BYTES];  // "A" means "Array" for JB's naming standards 
	void  *mapA;  
} Map;

Error mapNew(Map **mapPP, const U8 elemSz, const Key nElems);
void  mapDel(Map **mapPP);
Error mapSet(Map *mapP, const U8 key, const void *valP);
void* mapGet(const Map *mapP, const U8 key);
Error mapGetIndex(const Map *mapP, const Key key, Key *idxP);
Error mapGetNestedMapP(Map *mapP, Key mapKey, Map **mapPP);
Error mapGetNestedMapPElem(Map *mapP, Key mapKey, Key elemKey, void **returnedItemPP);

// Binary trees
#define binaryTreeNew_ arrayNew 
#define binaryTreeDel_ arrayDel 
#define getLeftChildIdx_(parentIdx_) ((parentIdx_ << 1) + 1)
#define getRightChildIdx_(parentIdx_) ((parentIdx_ << 1) + 2)
#define getParentIdx_(childIdx_) ((childIdx_ - 1 - !(childIdx_ & 1)) >> 1)

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

Error botoxInflate(Inflatable *inflatableP);
void botoxDeflate(Inflatable **inflatablePP);

// Efficient Arrays (frays)
Error frayNew(void **fPP, U32 elemSz, U32 nElems);
void  frayDel(void **frayPP);
Error frayAdd(const void *frayP, void *elemP, U32 *elemNewIdxP);
void  frayClr(void *fP);
U32   frayActivate(const void *frayP, U32 idx);
U32   frayDeactivate(const void *frayP, U32 idx);
U32   frayPause(const void *frayP, U32 idx);
U32   frayUnpause(const void *frayP, U32 idx);
U8    frayElemIsActive(const void *frayP, U32 idx);
U32   frayGetFirstInactiveIdx(const void *frayP);
U32*  frayGetFirstInactiveIdxP(const void *frayP);
U32*  frayGetFirstEmptyIdxP(const void *frayP);
U32   frayGetNPaused(const void *frayP);
U32   frayGetFirstPausedIdx(const void *frayP);

// Communcication
typedef enum {
  ACTIVATE,
  DEACTIVATE,
  PAUSE,
  UNPAUSE,
  SWITCH, 
  SWITCH_AND_ACTIVATE, 
  SWITCH_AND_DEACTIVATE, 
  N_MAIL_BUILTIN_CMDS
} MailCmd;

typedef struct {
	Key address;   // e.g. motion system
	Key attn;      // e.g. specific component in motion system 
	Key cmd;       // e.g. move, [de]activate, or do nothing ( 
	Key arg;       // e.g. change component value to MOVE_UP
} Message;  

Error mailboxWrite(Message *mailboxF, Key address, Key attn, Key cmd, Key arg);
Error mailboxForward(Message *mailboxF, Message *msgP);
typedef Error (*inboxRead)(Message *mailboxF);  // only for self

// Strip inflation
typedef struct {
  U16 nFlips;
  U16 *flipIdxA;
} FlipSetS;

// Strip set's inflated data is in U32 format.
typedef struct {
  U8 nUnitsPerStrip;
  U16 nStrips;
  U32 nUnits;
  FlipSetS flipSet;
  Inflatable *stripSetInfP;  // strip set's compressed source data
} StripSetS;

// StripMapS's inflated data is in U16 format.
typedef S16 StripmapIdx;
typedef struct {
  U32 nIndices;
  Inflatable *stripMapInfP;
} StripMapS;

// N_UNITS_PER_STRIP drives all the other quantities. TODO add compile-time assertion for being a multiple of 32.
#define N_UNITS_PER_STRIP (32) 
#define RSHIFT_TO_DIV_BY_NPPS (5)   /* NPPS = Number of Units of Data Per Strip */
#define N_COLORS_SUPPORTED_MAX_ (16)
/* Words per strip math goes like this:
    # units     # bits    byte     word      # words
    -------- *  ------ * ------ * ------- =  -------
     strip       unit    # bits   # bytes     strip
*/
#define N_WORDS_PER_1BPU_STRIP ((N_UNITS_PER_STRIP * 1) >> 5)  
#define N_WORDS_PER_2BPU_STRIP ((N_UNITS_PER_STRIP * 2) >> 5)  
#define N_WORDS_PER_4BPU_STRIP ((N_UNITS_PER_STRIP * 4) >> 5)  
#define N_WORDS_PER_8BPU_STRIP ((N_UNITS_PER_STRIP * 8) >> 5)    // for flips
#define N_QUADWORDS_PER_1BPU_STRIP ((N_WORDS_PER_1BPU_STRIP + (N_WORDS_PER_1BPU_STRIP >> 1))>> 2)  // make sure 0.5 rounds up to 1
#define N_QUADWORDS_PER_2BPU_STRIP ((N_WORDS_PER_2BPU_STRIP + (N_WORDS_PER_2BPU_STRIP >> 1))>> 2)
#define N_QUADWORDS_PER_4BPU_STRIP ((N_WORDS_PER_4BPU_STRIP + (N_WORDS_PER_4BPU_STRIP >> 1))>> 2)
// Number of units per byte
#define N_1BPU_UNITS_PER_BYTE (8)
#define N_2BPU_UNITS_PER_BYTE (4)
#define N_4BPU_UNITS_PER_BYTE (2)
// Amount of shifting to do per masking iteration.
#define N_BITS_PER_BYTE (8)
#define SHIFT_INCREMENT_1BPU (1)
#define SHIFT_INCREMENT_2BPU (2)
#define SHIFT_INCREMENT_4BPU (4)
// Masks used for grabbing 4 units in parallel. (That's why I stagger their order in img.c.)
#define MASK_1BPU  (0x01010101)
#define MASK_2BPU  (0x03030303)
#define MASK_4BPU  (0x0f0f0f0f)
#define QUADWORD_MASK_1BPU  (0x01)
#define QUADWORD_MASK_2BPU  (0x03)
#define QUADWORD_MASK_4BPU  (0x0f)
// Macros for converting strip index to strip pointer. WARNING: ASSUMES STRIPIDX IS A POINTER TO U32!
#define stripIdxTo1BpuStripPtr_(stripIdx_) (stripIdx_ << (3 * N_WORDS_PER_1BPU_STRIP))
#define stripIdxTo2BpuStripPtr_(stripIdx_) (stripIdx_ << (3 * N_WORDS_PER_2BPU_STRIP))
#define stripIdxTo4BpuStripPtr_(stripIdx_) (stripIdx_ << (3 * N_WORDS_PER_4BPU_STRIP))
// Macro for converting flip index to strip pointer.
#define flipIdxTo8BpuStripPtr_(flipIdx_) (flipIdx_ << (3 * N_WORDS_PER_8BPU_STRIP))
// Macros for counting remainder units that take up whole bytes.
#define countWholeBytesFor1BpuUnits_(nUnits_) (nUnits_ >> 3)
#define countWholeBytesFor2BpuUnits_(nUnits_) (nUnits_ >> 2)
#define countWholeBytesFor4BpuUnits_(nUnits_) (nUnits_ >> 1)
// Macros for counting remainder units in last, partial byte.
#define countUnitsInPartialByte1BPU_(nUnits_) (nUnits_ & (N_1BPU_UNITS_PER_BYTE - 1))
#define countUnitsInPartialByte2BPU_(nUnits_) (nUnits_ & (N_2BPU_UNITS_PER_BYTE - 1))
#define countUnitsInPartialByte4BPU_(nUnits_) (nUnits_ & (N_4BPU_UNITS_PER_BYTE - 1))
// Macro for counting number of whole strips
#define countWholeStrips_(nUnits_) (nUnits_ >> RSHIFT_TO_DIV_BY_NPPS);
// Macro for counting number of remainder units
#define countRemainderUnits_(nUnits_) (nUnits_ & (N_UNITS_PER_STRIP - 1))
// Assembler instructions need the "inline" keyword.
#ifdef inline
#undef inline
#endif

// Stripped data inflation
#ifdef __ARM_NEON__
#define defineUnpackStripFunction_(Bpu_, maskByte_) \
__inline__ static void _unpackStrip##Bpu_##Bpu(U32 **srcStripPP, U32 **dstStripPP) {\
  U32 *srcStripP = *srcStripPP;\
  U32 *dstStripP = *dstStripPP;\
  /* 6 instructions neon VS 40-58 instructions regular */\
  /* Although the outer loop appears unnecessary for 1 quadword per 1Bpu strip,
     it safeguards us from changes in the number of units per strip. */\
  for (int i = 0; i < N_QUADWORDS_PER_##Bpu_##BPU_STRIP; ++i) {  /* keeping this useless loop here for when I chagne to 128-unit strips. */\
    asm volatile inline (\
    "vmov.u8 q10, #" #maskByte_ "\n\t"   /* q10 = mask */\
    "vld1.32 {d0-d1}, [%0]!\n\t"    /* q0 (aka d0-d1) = packed indices */\
    : "+r&" (srcStripP)\
    );\
    for (int j = 0; j < N_##Bpu_##BPU_UNITS_PER_BYTE; ++j) {\
      asm volatile inline (\
      "vand q1, q0, q10\n\t"          /* q1 = unpacked indices*/\
      "vst1.32 {d2-d3}, [%0]!\n\t"\
      "vshr.u8 q0, #" #Bpu_ "\n\t"            /* shift q0 over 1*/\
      : "+r&" (dstStripP)\
      );\
    }\
  }\
}
#else
#define defineUnpackStripFunction_(Bpu_, maskWord_) \
__inline__ static void _unpackStrip##Bpu_##Bpu(U32 **srcStripPP, U32 **dstStripPP) {\
  U32 *srcStripP = *srcStripPP;\
  U32 *dstStripP = *dstStripPP;\
  /* Although the first loop line appears unnecessary for 1 word per 1Bpu strip,
     it safeguards us from changes in the number of units per strip. */\
  for (int i = 0; i < N_WORDS_PER_##Bpu_##BPU_STRIP; ++i) \
    for (int j = 0; j < N_BITS_PER_BYTE; j += SHIFT_INCREMENT_##Bpu_##BPU)\
      *dstStripP++ =  (*(srcStripP++) >> j) & maskWord_;\
}
#endif

#define defineUnpackRemainderUnitsFunction_(Bpu_, maskByte_) \
__inline__ static void _unpackRemainderUnits##Bpu_(U8 *byteA, U8 *outputByteP, U32 nRemainderUnits) {\
  U8 *byteP = byteA;\
  U8 *byteEndP = byteP + countWholeBytesFor##Bpu_##BpuUnits_(nRemainderUnits);\
  /* Handle all the whole bytes of units. */\
  while (byteP < byteEndP)\
    for (U8 i = 0; i < N_BITS_PER_BYTE; i += SHIFT_INCREMENT_##Bpu_##BPU)\
      *outputByteP++ =  (*(byteP++) >> i) & maskByte_;\
  /* Handle the last, partial byte of data. */\
  U8 iEnd = countUnitsInPartialByte##Bpu_##BPU_(nRemainderUnits);\
  for (U8 i = 0; i < iEnd; i += SHIFT_INCREMENT_##Bpu_##BPU)\
    *outputByteP++ =  (*(byteP++) >> i) & maskByte_;\
}

void flipUnpackedStrips(StripSetS *stripSetP, void *outputDataP);

#define defineInflateStripsWithBpu_(Bpu_)\
  declareInflateStripsWithBpu_(Bpu_) {\
    U32 *srcStripP; \
    U32 *dstStripOriginP = dstStripP; /* keep track of beginning as pointer gets incremented */ \
    /* Count remainder of pixels to process after all the whole strips. */ \
    U32 nWholeStrips = countWholeStrips_(stripSetP->nUnits); \
    U32 nRemainderUnits = countRemainderUnits_(stripSetP->nUnits); \
    /* Mapped stripsets need to be both unpacked and indexed. They may need strips to be flipped too. */ \
    /* First read all mapped strips into the target colormap. */\
    if (stripMapP) {\
      U16 *mapEndP = ((U16*) stripMapP->stripMapInfP->inflatedDataP) + nWholeStrips;\
      for (U16 *ssIdxP = (U16*) stripMapP->stripMapInfP->inflatedDataP; ssIdxP < mapEndP; ssIdxP++) {\
        srcStripP = stripSetP->stripSetInfP->inflatedDataP + stripIdxTo##Bpu_##BpuStripPtr_(*ssIdxP);  \
        _unpackStrip##Bpu_##Bpu(&srcStripP, &dstStripP);\
      }\
      srcStripP = stripSetP->stripSetInfP->inflatedDataP + stripSetP->nStrips - 1;\
      _unpackRemainderUnits##Bpu_((U8*) srcStripP, (U8*) dstStripP, nRemainderUnits);\
      /* Then flip whatever strips need flipping. Remember data's already expanded to U8s! */\
      if (stripSetP->flipSet.nFlips) \
        flipUnpackedStrips(stripSetP, dstStripOriginP);\
    } \
    /* Unmapped stripsets are already ordered, so they only need to be unpacked. */\
    else {\
      U32 *srcEndP = stripSetP->stripSetInfP->inflatedDataP + stripIdxTo1BpuStripPtr_(nWholeStrips);\
      for (U32 *srcStripP = stripSetP->stripSetInfP->inflatedDataP; srcStripP < srcEndP; srcStripP++) \
        _unpackStrip##Bpu_##Bpu(&srcStripP, &dstStripP);\
      _unpackRemainderUnits##Bpu_((U8*) srcStripP, (U8*) dstStripP, nRemainderUnits);\
    }\
  }

#define declareInflateStripsWithBpu_(Bpu_) void inflateStripsWithBpu##Bpu_ (StripSetS *stripSetP, StripMapS *stripMapP, U32 *dstStripP)

declareInflateStripsWithBpu_(1);
declareInflateStripsWithBpu_(2);
declareInflateStripsWithBpu_(4);
#endif
