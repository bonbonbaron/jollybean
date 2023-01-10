#include "data.h"

#define byteIdx_(key) ((key - 1) >> 3)
#define bitFlag_(key) (1 << ((key - 1) & 0x07))

inline Error jbAlloc(void **voidPP, U32 elemSz, U32 nElems) {
	if (voidPP == NULL)
		return E_BAD_ARGS;
	*voidPP = malloc(nElems * elemSz);
	if (*voidPP == NULL)
		return E_NO_MEMORY;
	return SUCCESS;
}

inline void jbFree(void **voidPP) {
	if (voidPP != NULL) {
		free(*voidPP);
		*voidPP = NULL;
	}
}

/************************************/
/********** ARRAYS (1D & 2D) ********/
/************************************/
Error arrayNew(void **arryPP, U32 elemSz, U32 nElems) {
	if (elemSz <= 0 || nElems < 0 || arryPP == NULL) 
		return E_BAD_ARGS;  /* TODO: replace with reasonable error type */
	if (nElems == 0) 
		*arryPP = NULL;
	else {
		U32 *ptr = (U32*) malloc((elemSz * nElems) + (2 * sizeof(U32)));
		if (ptr == NULL) 
			return E_NO_MEMORY;
		ptr[0] = elemSz;
		ptr[1] = nElems;
		*arryPP = (ptr + 2);
		memset(*arryPP, 0, elemSz * nElems);
	}
	return SUCCESS;
}

void arrayDel(void **arryPP) {
	if (arryPP != NULL && *arryPP != NULL) {
		U32 *ptr = *arryPP;
		free((ptr) - 2);
		*arryPP = NULL;
	}
}

U32 arrayGetNElems(const void *arryP) {
	U32 *ptr;
	if (arryP == NULL) 
		return 0;
	else {
		ptr = (U32*) arryP;
		return *(ptr - 1);
	}
}

U32 arrayGetElemSz(const void *arryP) {
	U32 *ptr;
	if (arryP == NULL)
		return 0;
	else {
		ptr = (U32*) arryP;
		return *(ptr - 2);
	}
}

inline static void* _arrayGetElemByIdx(const void *arryP, S32 idx) {
  const U32 nElems = arrayGetNElems(arryP);
  /* If idx < 0, return void pointer past end of array. */
  if (idx < 0) 
    return (void*) (((U8*) arryP) + (nElems * arrayGetElemSz(arryP)));
  /* If idx is valid, return void pointer to indexed element. */
  else if ((U32) idx < nElems)
    return (void*) ((U8*) arryP + (idx * arrayGetElemSz(arryP)));
  /* Index is invalid. */
  else
    return NULL;  
}

/* Also provide an external copy of above function. */
void* arrayGetVoidElemPtr(const void *arryP, S32 idx) {
  const U32 nElems = arrayGetNElems(arryP);
  /* If idx < 0, return void pointer past end of array. */
  if (idx < 0) 
    return (void*) (((U8*) arryP) + (nElems * arrayGetElemSz(arryP)));
  /* If idx is valid, return void pointer to indexed element. */
  else if ((U32) idx < nElems)
    return (void*) ((U8*) arryP + (idx * arrayGetElemSz(arryP)));
  /* Index is invalid. */
  else
    return NULL;  
}

Error arraySetVoidElem(void *arrayP, U32 idx, const void *elemSrcompP) {
	if (!arrayP)
		return E_BAD_ARGS;
	U32 elemSz = arrayGetElemSz((const void*) arrayP);
	void *dstP = (U8*) arrayP + (idx * elemSz);
	memcpy(dstP, elemSrcompP, elemSz);
	return SUCCESS;
}

void arrayIniPtrs(const void *arryP, void **startP, void **endP, S32 endIdx) {
	*startP = (void*) arryP;
	*endP = _arrayGetElemByIdx(arryP, endIdx);
}

inline static U32 _fast_arrayGetElemSz(const void *arryP) {
	return *(((U32*)arryP) - 2);
}
inline static void* _fast_arrayGetElemByIdx(const void *arryP, U32 idx) {
	return (void*) ((U8*) arryP + (idx * _fast_arrayGetElemSz(arryP)));
}

/***********************/
/********* MAPS ********/
/***********************/
Error mapNew(Map **mapPP, const U8 elemSz, const Key nElems) {
	if (elemSz == 0 || nElems == 0) {
    return E_BAD_ARGS;
  }
  Error e = jbAlloc((void**) mapPP, sizeof(Map), 1);
	if (!e)
		e = arrayNew(&(*mapPP)->mapA, elemSz, nElems);
  if (!e) 
    memset((*mapPP)->flagA, 0, sizeof(FlagInfo) * N_FLAG_BYTES);
  else {
    arrayDel((*mapPP)->mapA);
    jbFree((void**)mapPP);
  }
	return e;
}

void mapDel(Map **mapPP) {
	if (mapPP != NULL && *mapPP != NULL) {
		arrayDel(&(*mapPP)->mapA);
		jbFree((void**) mapPP);
	}
}

inline static U8 _isMapValid(const Map *mapP) {
	return (mapP != NULL && mapP->mapA != NULL); 
}	

inline static U8 _isKeyValid(const Key key) {
  return (key > 0);
}

/* Map GETTING functions */
inline static U8 _countBits(Key bitfield) {
#ifdef __ARM_NEON__
  asm("vmov.8 d21[0], %0\n\t"
      "vcnt.i8 d20, d21\n\t"
      "vmov.u8 %0, d20[0]"
      : "+r" (bitfield));
	return bitfield;
#else
	register Key count = bitfield - ((bitfield >> 1) & 0x55555555);
	count = (count & 0x33333333) + ((count >> 2) & 0x33333333);
	count = (count + (count >> 4)) & 0x0f0f0f0f;
	return (count * 0x01010101) >> 24;
#endif
}
inline static FlagInfo _getFlagInfo(const Map *mapP, const Key key) {
  return mapP->flagA[byteIdx_(key)];
}

#if 0
inline static U8 _isFlagSet(const U8 flags, const Key key) {
	return flags & (1 << ((key - 1) & 0x07));
}
#endif

inline static U32 _getElemIdx(const FlagInfo f, const Key key) {
	return f.prevBitCount + _countBits(f.flags & (bitFlag_(key) - 1));
}

Error mapGetIndex(const Map *mapP, const Key key, Key *idxP) {
	const register Key keyMinus1 = key - 1;
	const register FlagInfo f = mapP->flagA[keyMinus1 >> 3];  // Divide N by 8 for byte with Nth bit.
	const register Key bitFlag = 1 << (keyMinus1 & 0x07);     // 0x07 keeps bit inside 8-bit bounds.
	if (f.flags & bitFlag) {
    *idxP = _getElemIdx(f, key);
    return SUCCESS;
 }
  return E_BAD_KEY;
}

inline static void* _getElemP(const Map *mapP, const FlagInfo f, const Key key) {
	return _fast_arrayGetElemByIdx(mapP->mapA, _getElemIdx(f, key));
}	

inline static U32 _getMapElemSz(const Map *mapP) {
  return arrayGetElemSz(mapP->mapA);
}

inline static U32 _getNBitsSet(const Map *mapP) {
  return mapP->flagA[LAST_FLAG_BYTE_IDX].prevBitCount + _countBits(mapP->flagA[LAST_FLAG_BYTE_IDX].flags);
}

void* mapGet(const Map *mapP, const Key key) {
	const register U32 keyMinus1 = key - 1;
	const register FlagInfo f = mapP->flagA[keyMinus1 >> 3];
	const register U32 bitFlag = 1 << (keyMinus1 & 0x07);
	// If the bit flag in question is set, that means a value exists for the input key.
	if (f.flags & bitFlag) {
    register U32 count = f.flags & (bitFlag - 1);  // initialize count with the bits we're counting
    // The desired array index is the number of bits set before the key'th bit.
#ifdef __ARM_NEON__
    asm("vmov.8 d21[0], %0\n\t"
        "vcnt.i8 d20, d21\n\t"
        "vmov.u8 %0, d20[0]"
        : "+r" (count));
#else
		count = count - ((count >> 1) & 0x55555555);
		count = (count & 0x33333333) + ((count >> 2) & 0x33333333);
		count = (count + (count >> 4)) & 0x0f0f0f0f;
		count = (count * 0x01010101) >> 24;
#endif
		return _fast_arrayGetElemByIdx(mapP->mapA, count + f.prevBitCount);
	}
	return NULL;
}

/* Map SETTING functions */
/* If any bits exist to the left of the key's bit, array elements exist in target spot. */
inline static U8 _idxIsPopulated(const U32 nBitsSet, U32 idx) {
  return (idx < nBitsSet);
}

static Error preMapSet(const Map *mapP, const Key key, void **elemPP, void **nextElemPP, U32 *nBytesTMoveP) {
  *nBytesTMoveP = 0;
  if (_isMapValid(mapP) && _isKeyValid(key)) {
    FlagInfo f;
    f = _getFlagInfo(mapP, key);
    *elemPP = _getElemP(mapP, f, key);
	  if (*elemPP) {  /* Side-stepping mapGet() to avoid NULL pointers and double-calling _isMapValid() */
      U32 nBitsSet = _getNBitsSet(mapP);
			U32 keyElemIdx = _getElemIdx(f, key);
      /* If something's already in the target index, move everything over one. */
      if (_idxIsPopulated(nBitsSet, keyElemIdx)) {
        U32 mapElemSz = _getMapElemSz(mapP);
        *nBytesTMoveP = (nBitsSet - keyElemIdx) * mapElemSz;
        *nextElemPP = (U8*) *elemPP + mapElemSz;
      }
      return SUCCESS;
    } else {
      return E_BAD_KEY;
    }
  } else 
    return E_BAD_ARGS;
}

Error mapSet(Map *mapP, const Key key, const void *valP) {
	void *elemP, *nextElemP;
  U32 nBytesToMove;
  Error e = preMapSet(mapP, key, &elemP, &nextElemP, &nBytesToMove);
  if (!e) {
    if (nBytesToMove) 
      memcpy(nextElemP, (const void*) elemP, nBytesToMove);
		/* Write value to map element. */
		memcpy(elemP, valP, _getMapElemSz(mapP));
		/* Set flag. */
		Key byteIdx = byteIdx_(key);
		mapP->flagA[byteIdx].flags |= bitFlag_(key);  /* flagNum & 0x07 gives you # of bits in the Nth byte */
		/* Increment all prevBitCounts in bytes above affected one. */
#ifdef __ARM_NEON__
#else
		while (++byteIdx < N_FLAG_BYTES) 
		  ++mapP->flagA[byteIdx].prevBitCount;
#endif
	}
	return e;
}

Error mapRem(Map *mapP, const Key key) {
	void *elemP, *nextElemP;
  U32 nBytesToMove;
  Error e = preMapSet(mapP, key, &elemP, &nextElemP, &nBytesToMove);
  if (!e) {
    if (nBytesToMove) 
      memcpy(elemP, (const void*) nextElemP, nBytesToMove);
		/* Unset flag. */
		U8 byteIdx = byteIdx_(key);
		mapP->flagA[byteIdx].flags &= ~bitFlag_(key);  /* key's bit position byteIdx'th byte */
		/* Increment all prevBitCounts in bytes above affected one. */
		while (++byteIdx < N_FLAG_BYTES) 
			--mapP->flagA[byteIdx].prevBitCount;
	}
	return e;
}

Error mapGetNestedMapP(Map *mapP, Key mapKey, Map **mapPP) {
  if (!mapP || !mapKey)
    return E_BAD_ARGS;

  Map **_mapPP = (Map**) mapGet(mapP, mapKey);

  if (_mapPP && *_mapPP)
    *mapPP = *_mapPP;
  else
    return E_BAD_KEY;

  return SUCCESS;
}

Error mapGetNestedMapPElem(Map *mapP, Key mapKey, Key elemKey, void **returnedItemPP) {
  if (!elemKey || !returnedItemPP)
    return E_BAD_ARGS;

  Map *nestedMapP = NULL;
  Error e = mapGetNestedMapP(mapP, mapKey, &nestedMapP);

  if (!e) {
    void **valPP = mapGet(nestedMapP, elemKey);
    if (valPP)
      *returnedItemPP = *valPP;
    if (!*returnedItemPP)
      return E_BAD_KEY;
  }

  return SUCCESS;
}

// Binary Trees
// We're letting the user define the search function, because every object is different and callbacks are slow.

/************************************/
/************ HISTOGRAMS ************/
/************************************/

Error histoNew(U32 **histoPP, const U32 maxVal) {
	if (histoPP == NULL)
		return E_BAD_ARGS;
	return arrayNew((void**) histoPP, sizeof(U32), maxVal);
}	

void histoDel(U32 **histoPP) {
	arrayDel((void**) histoPP);
}

/*************************************/
/******** TINFL DECOMPRESSION ********/
/*************************************/

/* tinfl.c v1.11 - public domain inflate with zlib header parsing/adler32 checking (inflate-only subset of miniz.c)
   See "unlicense" statement at the end of this file.
   Rich Geldreich <richgel99@gmail.com>, last updated May 20, 2011
   Implements RFC 1950: http://www.ietf.org/rfc/rfc1950.txt and RFC 1951: http://www.ietf.org/rfc/rfc1951.txt

   The entire decompressor coroutine is implemented in tinfl_decompress(). The other functions are optional high-depth helpers.
*/
#ifndef TINFL_HEADER_INCLUDED
#define TINFL_HEADER_INCLUDED

typedef unsigned char mz_uint8;
typedef signed short mz_int16;
typedef unsigned short mz_uint16;
typedef unsigned int mz_uint32;
typedef unsigned int mz_uint;
typedef unsigned long long mz_uint64;

#if defined(_M_IX86) || defined(_M_X64)
// Set MINIZ_USE_UNALIGNED_LOADS_AND_STORES to 1 if integer loads and stores to unaligned addresses are acceptable on the target platform (slightly faster).
#define MINIZ_USE_UNALIGNED_LOADS_AND_STORES 1
// Set MINIZ_LITTLE_ENDIAN to 1 if the processor is little endian.
#define MINIZ_LITTLE_ENDIAN 1
#endif

#if defined(_WIN64) || defined(__MINGW64__) || defined(_LP64) || defined(__LP64__)
// Set MINIZ_HAS_64BIT_REGISTERS to 1 if the processor has 64-bit general purpose registers (enables 64-bit bitbuffer in inflator)
#define MINIZ_HAS_64BIT_REGISTERS 1
#endif

// Works around MSVC's spammy "warning C4127: conditional expression is constant" message.
#ifdef _MSC_VER
  #define MZ_MACRO_END while (0, 0)
#else
  #define MZ_MACRO_END while (0)
#endif

// Decompression flags used by tinfl_decompress().
// TINFL_FLAG_PARSE_ZLIB_HEADER: If set, the input has a valid zlib header and ends with an adler32 checksum (it's a valid zlib stream). Otherwise, the input is a raw deflate stream.
// TINFL_FLAG_HAS_MORE_INPUT: If set, there are more input bytes available beyond the end of the supplied input buffer. If clear, the input buffer contains all remaining input.
// TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF: If set, the output buffer is large enough to hold the entire decompressed stream. If clear, the output buffer is at least the size of the dictionary (typically 32KB).
// TINFL_FLAG_COMPUTE_ADLER32: Force adler-32 checksum computation of the decompressed bytes.
enum
{
  TINFL_FLAG_PARSE_ZLIB_HEADER = 1,
  TINFL_FLAG_HAS_MORE_INPUT = 2,
  TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF = 4,
  TINFL_FLAG_COMPUTE_ADLER32 = 8
};

// High depth decompression functions:
// tinfl_decompress_mem_to_heap() decompresses a block in memory to a heap block allocated via malloc().
// On entry:
//  pSrc_buf, src_buf_len: Pointer and size of the Deflate or zlib source data to decompress.
// On return:
//  Function returns a pointer to the decompressed data, or NULL on failure.
//  *pOut_len will be set to the decompressed data's size, which could be larger than src_buf_len on uncompressible data.
//  The caller must free() the returned block when it's no longer needed.
void *tinfl_decompress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags);

// tinfl_decompress_mem_to_mem() decompresses a block in memory to another block in memory.
// Returns TINFL_DECOMPRESS_MEM_TO_MEM_FAILED on failure, or the number of bytes written on success.
#define TINFL_DECOMPRESS_MEM_TO_MEM_FAILED ((size_t)(-1))
size_t tinfl_decompress_mem_to_mem(void *pOut_buf, size_t out_buf_len, const void *pSrc_buf, size_t src_buf_len, int flags);

// tinfl_decompress_mem_to_callback() decompresses a block in memory to an internal 32KB buffer, and a user provided callback function will be called to flush the buffer.
// Returns 1 on success or 0 on failure.
typedef int (*tinfl_put_buf_func_ptr)(const void* pBuf, int len, void *pUser);
int tinfl_decompress_mem_to_callback(const void *pIn_buf, size_t *pIn_buf_size, tinfl_put_buf_func_ptr pPut_buf_func, void *pPut_buf_user, int flags);

struct tinfl_decompressor_tag; typedef struct tinfl_decompressor_tag tinfl_decompressor;

// Max size of LZ dictionary.
#define TINFL_LZ_DICT_SIZE 32768

// Return status.
typedef enum
{
  TINFL_STATUS_BAD_PARAM = -3,
  TINFL_STATUS_ADLER32_MISMATCH = -2,
  TINFL_STATUS_FAILED = -1,
  TINFL_STATUS_DONE = 0,
  TINFL_STATUS_NEEDS_MORE_INPUT = 1,
  TINFL_STATUS_HAS_MORE_OUTPUT = 2
} tinfl_status;

// Initializes the decompressor to its initial state.
#define tinflIni(r) do { (r)->m_state = 0; } MZ_MACRO_END
#define tinfl_get_adler32(r) (r)->m_check_adler32

// Main low-depth decompressor coroutine function. This is the only function actually needed for decompression. All the other functions are just high-depth helpers for improved usability.
// This is a universal API, i.e. it can be used as a building block to build any desired higher depth decompression API. In the limit case, it can be called once per every byte input or output.
tinfl_status tinfl_decompress(tinfl_decompressor *r, const mz_uint8 *pIn_buf_next, size_t *pIn_buf_size, mz_uint8 *pOut_buf_start, mz_uint8 *pOut_buf_next, size_t *pOut_buf_size, const mz_uint32 decomp_flags);

// Internal/private bits follow.
enum
{
  TINFL_MAX_HUFF_TABLES = 3, TINFL_MAX_HUFF_SYMBOLS_0 = 288, TINFL_MAX_HUFF_SYMBOLS_1 = 32, TINFL_MAX_HUFF_SYMBOLS_2 = 19,
  TINFL_FAST_LOOKUP_BITS = 10, TINFL_FAST_LOOKUP_SIZE = 1 << TINFL_FAST_LOOKUP_BITS
};

typedef struct
{
  mz_uint8 m_code_size[TINFL_MAX_HUFF_SYMBOLS_0];
  mz_int16 m_look_up[TINFL_FAST_LOOKUP_SIZE], m_tree[TINFL_MAX_HUFF_SYMBOLS_0 * 2];
} tinfl_huff_table;

#if MINIZ_HAS_64BIT_REGISTERS
  #define TINFL_USE_64BIT_BITBUF 1
#endif

#if TINFL_USE_64BIT_BITBUF
  typedef mz_uint64 tinfl_bit_buf_t;
  #define TINFL_BITBUF_SIZE (64)
#else
  typedef mz_uint32 tinfl_bit_buf_t;
  #define TINFL_BITBUF_SIZE (32)
#endif

struct tinfl_decompressor_tag
{
  mz_uint32 m_state, m_n_bits, m_zhdr0, m_zhdr1, m_z_adler32, m_final, m_type, m_check_adler32, m_dist, m_counter, m_num_extra, m_table_sizes[TINFL_MAX_HUFF_TABLES];
  tinfl_bit_buf_t m_bit_buf;
  size_t m_dist_from_out_buf_start;
  tinfl_huff_table m_tables[TINFL_MAX_HUFF_TABLES];
  mz_uint8 m_raw_header[4], m_len_codes[TINFL_MAX_HUFF_SYMBOLS_0 + TINFL_MAX_HUFF_SYMBOLS_1 + 137];
};

#endif // #ifdef TINFL_HEADER_INCLUDED

// ------------------- End of Header: Implementation follows. (If you only want the header, define MINIZ_HEADER_FILE_ONLY.)

#ifndef TINFL_HEADER_FILE_ONLY

#include <string.h>

// MZ_MALLOC, etc. are only used by the optional high-depth helper functions.
#ifdef MINIZ_NO_MALLOC
  #define MZ_MALLOC(x) NULL
  #define MZ_FREE(x) x, ((void)0)
  #define MZ_REALLOC(p, x) NULL
#else
  #define MZ_MALLOC(x) malloc(x)
  #define MZ_FREE(x) free(x)
  #define MZ_REALLOC(p, x) realloc(p, x)
#endif

#define MZ_MAX(a,b) (((a)>(b))?(a):(b))
#define MZ_MIN(a,b) (((a)<(b))?(a):(b))
#define MZ_CLEAR_OBJ(obj) memset(&(obj), 0, sizeof(obj))

#if MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN
  #define MZ_READ_LE16(p) *((const mz_uint16 *)(p))
  #define MZ_READ_LE32(p) *((const mz_uint32 *)(p))
#else
  #define MZ_READ_LE16(p) ((mz_uint32)(((const mz_uint8 *)(p))[0]) | ((mz_uint32)(((const mz_uint8 *)(p))[1]) << 8U))
  #define MZ_READ_LE32(p) ((mz_uint32)(((const mz_uint8 *)(p))[0]) | ((mz_uint32)(((const mz_uint8 *)(p))[1]) << 8U) | ((mz_uint32)(((const mz_uint8 *)(p))[2]) << 16U) | ((mz_uint32)(((const mz_uint8 *)(p))[3]) << 24U))
#endif

#define TINFL_MEMCPY(d, s, l) memcpy(d, s, l)
#define TINFL_MEMSET(p, c, l) memset(p, c, l)

#define TINFL_CR_BEGIN switch(r->m_state) { case 0:
#define TINFL_CR_RETURN(state_index, result) do { status = result; r->m_state = state_index; goto common_exit; case state_index:; } MZ_MACRO_END
#define TINFL_CR_RETURN_FOREVER(state_index, result) do { for ( ; ; ) { TINFL_CR_RETURN(state_index, result); } } MZ_MACRO_END
#define TINFL_CR_FINISH }

// TODO: If the caller has indicated that there's no more input, and we attempt to read beyond the input buf, then something is wrong with the input because the inflator never
// reads ahead more than it needs to. Currently TINFL_GET_BYTE() pads the end of the stream with 0's in this scenario.
#define TINFL_GET_BYTE(state_index, c) do { \
  if (pIn_buf_cur >= pIn_buf_end) { \
    for ( ; ; ) { \
      if (decomp_flags & TINFL_FLAG_HAS_MORE_INPUT) { \
        TINFL_CR_RETURN(state_index, TINFL_STATUS_NEEDS_MORE_INPUT); \
        if (pIn_buf_cur < pIn_buf_end) { \
          c = *pIn_buf_cur++; \
          break; \
        } \
      } else { \
        c = 0; \
        break; \
      } \
    } \
  } else c = *pIn_buf_cur++; } MZ_MACRO_END

#define TINFL_NEED_BITS(state_index, n) do { mz_uint c; TINFL_GET_BYTE(state_index, c); bit_buf |= (((tinfl_bit_buf_t)c) << num_bits); num_bits += 8; } while (num_bits < (mz_uint)(n))
#define TINFL_SKIP_BITS(state_index, n) do { if (num_bits < (mz_uint)(n)) { TINFL_NEED_BITS(state_index, n); } bit_buf >>= (n); num_bits -= (n); } MZ_MACRO_END
#define TINFL_GET_BITS(state_index, b, n) do { if (num_bits < (mz_uint)(n)) { TINFL_NEED_BITS(state_index, n); } b = bit_buf & ((1 << (n)) - 1); bit_buf >>= (n); num_bits -= (n); } MZ_MACRO_END

// TINFL_HUFF_BITBUF_FILL() is only used rarely, when the number of bytes remaining in the input buffer falls below 2.
// It reads just enough bytes from the input stream that are needed to decode the next Huffman code (and absolutely no more). It works by trying to fully decode a
// Huffman code by using whatever bits are currently present in the bit buffer. If this fails, it reads another byte, and tries again until it succeeds or until the
// bit buffer contains >=15 bits (deflate's max. Huffman code size).
#define TINFL_HUFF_BITBUF_FILL(state_index, pHuff) \
  do { \
    temp = (pHuff)->m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]; \
    if (temp >= 0) { \
      code_len = temp >> 9; \
      if ((code_len) && (num_bits >= code_len)) \
      break; \
    } else if (num_bits > TINFL_FAST_LOOKUP_BITS) { \
       code_len = TINFL_FAST_LOOKUP_BITS; \
       do { \
          temp = (pHuff)->m_tree[~temp + ((bit_buf >> code_len++) & 1)]; \
       } while ((temp < 0) && (num_bits >= (code_len + 1))); if (temp >= 0) break; \
    } TINFL_GET_BYTE(state_index, c); bit_buf |= (((tinfl_bit_buf_t)c) << num_bits); num_bits += 8; \
  } while (num_bits < 15);

// TINFL_HUFF_DECODE() decodes the next Huffman coded symbol. It's more complex than you would initially expect because the zlib API expects the decompressor to never read
// beyond the final byte of the deflate stream. (In other words, when this macro wants to read another byte from the input, it REALLY needs another byte in order to fully
// decode the next Huffman code.) Handling this properly is particularly important on raw deflate (non-zlib) streams, which aren't followed by a byte aligned adler-32.
// The slow path is only executed at the very end of the input buffer.
#define TINFL_HUFF_DECODE(state_index, sym, pHuff) do { \
  int temp; mz_uint code_len, c; \
  if (num_bits < 15) { \
    if ((pIn_buf_end - pIn_buf_cur) < 2) { \
       TINFL_HUFF_BITBUF_FILL(state_index, pHuff); \
    } else { \
       bit_buf |= (((tinfl_bit_buf_t)pIn_buf_cur[0]) << num_bits) | (((tinfl_bit_buf_t)pIn_buf_cur[1]) << (num_bits + 8)); pIn_buf_cur += 2; num_bits += 16; \
    } \
  } \
  if ((temp = (pHuff)->m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0) \
    code_len = temp >> 9, temp &= 511; \
  else { \
    code_len = TINFL_FAST_LOOKUP_BITS; do { temp = (pHuff)->m_tree[~temp + ((bit_buf >> code_len++) & 1)]; } while (temp < 0); \
  } sym = temp; bit_buf >>= code_len; num_bits -= code_len; } MZ_MACRO_END

tinfl_status tinfl_decompress(tinfl_decompressor *r, const mz_uint8 *pIn_buf_next, size_t *pIn_buf_size, mz_uint8 *pOut_buf_start, mz_uint8 *pOut_buf_next, size_t *pOut_buf_size, const mz_uint32 decomp_flags)
{
  static const int s_length_base[31] = { 3,4,5,6,7,8,9,10,11,13, 15,17,19,23,27,31,35,43,51,59, 67,83,99,115,131,163,195,227,258,0,0 };
  static const int s_length_extra[31]= { 0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0,0,0 };
  static const int s_dist_base[32] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193, 257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,0,0};
  static const int s_dist_extra[32] = { 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};
  static const mz_uint8 s_length_dezigzag[19] = { 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };
  static const int s_min_table_sizes[3] = { 257, 1, 4 };

  tinfl_status status = TINFL_STATUS_FAILED; mz_uint32 num_bits, dist, counter, num_extra; tinfl_bit_buf_t bit_buf;
  const mz_uint8 *pIn_buf_cur = pIn_buf_next, *const pIn_buf_end = pIn_buf_next + *pIn_buf_size;
  mz_uint8 *pOut_buf_cur = pOut_buf_next, *const pOut_buf_end = pOut_buf_next + *pOut_buf_size;
  size_t out_buf_size_mask = (decomp_flags & TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF) ? (size_t)-1 : ((pOut_buf_next - pOut_buf_start) + *pOut_buf_size) - 1, dist_from_out_buf_start;

  // Ensure the output buffer's size is a power of 2, unless the output buffer is large enough to hold the entire output file (in which case it doesn't matter).
  if (((out_buf_size_mask + 1) & out_buf_size_mask) || (pOut_buf_next < pOut_buf_start)) { *pIn_buf_size = *pOut_buf_size = 0; return TINFL_STATUS_BAD_PARAM; }

  num_bits = r->m_n_bits; bit_buf = r->m_bit_buf; dist = r->m_dist; counter = r->m_counter; num_extra = r->m_num_extra; dist_from_out_buf_start = r->m_dist_from_out_buf_start;
  TINFL_CR_BEGIN

  bit_buf = num_bits = dist = counter = num_extra = r->m_zhdr0 = r->m_zhdr1 = 0; r->m_z_adler32 = r->m_check_adler32 = 1;
  if (decomp_flags & TINFL_FLAG_PARSE_ZLIB_HEADER)
  {
    TINFL_GET_BYTE(1, r->m_zhdr0); TINFL_GET_BYTE(2, r->m_zhdr1);
    counter = (((r->m_zhdr0 * 256 + r->m_zhdr1) % 31 != 0) || (r->m_zhdr1 & 32) || ((r->m_zhdr0 & 15) != 8));
    if (!(decomp_flags & TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF)) counter |= (((1U << (8U + (r->m_zhdr0 >> 4))) > 32768U) || ((out_buf_size_mask + 1) < (size_t)(1U << (8U + (r->m_zhdr0 >> 4)))));
    if (counter) { TINFL_CR_RETURN_FOREVER(36, TINFL_STATUS_FAILED); }
  }

  do
  {
    TINFL_GET_BITS(3, r->m_final, 3); r->m_type = r->m_final >> 1;
    if (r->m_type == 0)
    {
      TINFL_SKIP_BITS(5, num_bits & 7);
      for (counter = 0; counter < 4; ++counter) { if (num_bits) TINFL_GET_BITS(6, r->m_raw_header[counter], 8); else TINFL_GET_BYTE(7, r->m_raw_header[counter]); }
      if ((counter = (r->m_raw_header[0] | (r->m_raw_header[1] << 8))) != (mz_uint)(0xFFFF ^ (r->m_raw_header[2] | (r->m_raw_header[3] << 8)))) { TINFL_CR_RETURN_FOREVER(39, TINFL_STATUS_FAILED); }
      while ((counter) && (num_bits))
      {
        TINFL_GET_BITS(51, dist, 8);
        while (pOut_buf_cur >= pOut_buf_end) { TINFL_CR_RETURN(52, TINFL_STATUS_HAS_MORE_OUTPUT); }
        *pOut_buf_cur++ = (mz_uint8)dist;
        counter--;
      }
      while (counter)
      {
        size_t n; while (pOut_buf_cur >= pOut_buf_end) { TINFL_CR_RETURN(9, TINFL_STATUS_HAS_MORE_OUTPUT); }
        while (pIn_buf_cur >= pIn_buf_end)
        {
          if (decomp_flags & TINFL_FLAG_HAS_MORE_INPUT)
          {
            TINFL_CR_RETURN(38, TINFL_STATUS_NEEDS_MORE_INPUT);
          }
          else
          {
            TINFL_CR_RETURN_FOREVER(40, TINFL_STATUS_FAILED);
          }
        }
        n = MZ_MIN(MZ_MIN((size_t)(pOut_buf_end - pOut_buf_cur), (size_t)(pIn_buf_end - pIn_buf_cur)), counter);
        TINFL_MEMCPY(pOut_buf_cur, pIn_buf_cur, n); pIn_buf_cur += n; pOut_buf_cur += n; counter -= (mz_uint)n;
      }
    }
    else if (r->m_type == 3)
    {
      TINFL_CR_RETURN_FOREVER(10, TINFL_STATUS_FAILED);
    }
    else
    {
      if (r->m_type == 1)
      {
        mz_uint8 *p = r->m_tables[0].m_code_size; mz_uint i;
        r->m_table_sizes[0] = 288; r->m_table_sizes[1] = 32; TINFL_MEMSET(r->m_tables[1].m_code_size, 5, 32);
        for ( i = 0; i <= 143; ++i) *p++ = 8; 
				for ( ; i <= 255; ++i) *p++ = 9; 
				for ( ; i <= 279; ++i) *p++ = 7; 
				for ( ; i <= 287; ++i) *p++ = 8;
      }
      else
      {
        for (counter = 0; counter < 3; counter++) { TINFL_GET_BITS(11, r->m_table_sizes[counter], "\05\05\04"[counter]); r->m_table_sizes[counter] += s_min_table_sizes[counter]; }
        MZ_CLEAR_OBJ(r->m_tables[2].m_code_size); 
				for (counter = 0; counter < r->m_table_sizes[2]; counter++) { mz_uint s; TINFL_GET_BITS(14, s, 3); r->m_tables[2].m_code_size[s_length_dezigzag[counter]] = (mz_uint8)s; }
        r->m_table_sizes[2] = 19;
      }
      for ( ; (int)r->m_type >= 0; r->m_type--)
      {
        int tree_next, tree_cur; tinfl_huff_table *pTable;
        mz_uint i, j, used_syms, total, sym_index, next_code[17], total_syms[16]; pTable = &r->m_tables[r->m_type]; MZ_CLEAR_OBJ(total_syms); MZ_CLEAR_OBJ(pTable->m_look_up); MZ_CLEAR_OBJ(pTable->m_tree);
        for (i = 0; i < r->m_table_sizes[r->m_type]; ++i) total_syms[pTable->m_code_size[i]]++;
        used_syms = 0, total = 0; next_code[0] = next_code[1] = 0;
        for (i = 1; i <= 15; ++i) { used_syms += total_syms[i]; next_code[i + 1] = (total = ((total + total_syms[i]) << 1)); }
        if ((65536 != total) && (used_syms > 1))
        {
          TINFL_CR_RETURN_FOREVER(35, TINFL_STATUS_FAILED);
        }
        for (tree_next = -1, sym_index = 0; sym_index < r->m_table_sizes[r->m_type]; ++sym_index)
        {
          mz_uint rev_code = 0, l, cur_code, code_size = pTable->m_code_size[sym_index]; if (!code_size) continue;
          cur_code = next_code[code_size]++; 
					for (l = code_size; l > 0; l--, cur_code >>= 1) rev_code = (rev_code << 1) | (cur_code & 1);
          if (code_size <= TINFL_FAST_LOOKUP_BITS) { mz_int16 k = (mz_int16)((code_size << 9) | sym_index); while (rev_code < TINFL_FAST_LOOKUP_SIZE) { pTable->m_look_up[rev_code] = k; rev_code += (1 << code_size); } continue; }
          if (0 == (tree_cur = pTable->m_look_up[rev_code & (TINFL_FAST_LOOKUP_SIZE - 1)])) { pTable->m_look_up[rev_code & (TINFL_FAST_LOOKUP_SIZE - 1)] = (mz_int16)tree_next; tree_cur = tree_next; tree_next -= 2; }
          rev_code >>= (TINFL_FAST_LOOKUP_BITS - 1);
          for (j = code_size; j > (TINFL_FAST_LOOKUP_BITS + 1); j--)
          {
            tree_cur -= ((rev_code >>= 1) & 1);
            if (!pTable->m_tree[-tree_cur - 1]) { pTable->m_tree[-tree_cur - 1] = (mz_int16)tree_next; tree_cur = tree_next; tree_next -= 2; } else tree_cur = pTable->m_tree[-tree_cur - 1];
          }
          tree_cur -= ((rev_code >>= 1) & 1); pTable->m_tree[-tree_cur - 1] = (mz_int16)sym_index;
        }
        if (r->m_type == 2)
        {
          for (counter = 0; counter < (r->m_table_sizes[0] + r->m_table_sizes[1]); )
          {
            mz_uint s; TINFL_HUFF_DECODE(16, dist, &r->m_tables[2]); if (dist < 16) { r->m_len_codes[counter++] = (mz_uint8)dist; continue; }
            if ((dist == 16) && (!counter))
            {
              TINFL_CR_RETURN_FOREVER(17, TINFL_STATUS_FAILED);
            }
            num_extra = "\02\03\07"[dist - 16]; TINFL_GET_BITS(18, s, num_extra); s += "\03\03\013"[dist - 16];
            TINFL_MEMSET(r->m_len_codes + counter, (dist == 16) ? r->m_len_codes[counter - 1] : 0, s); counter += s;
          }
          if ((r->m_table_sizes[0] + r->m_table_sizes[1]) != counter)
          {
            TINFL_CR_RETURN_FOREVER(21, TINFL_STATUS_FAILED);
          }
          TINFL_MEMCPY(r->m_tables[0].m_code_size, r->m_len_codes, r->m_table_sizes[0]); TINFL_MEMCPY(r->m_tables[1].m_code_size, r->m_len_codes + r->m_table_sizes[0], r->m_table_sizes[1]);
        }
      }
      for ( ; ; )
      {
        mz_uint8 *pSrc;
        for ( ; ; )
        {
          if (((pIn_buf_end - pIn_buf_cur) < 4) || ((pOut_buf_end - pOut_buf_cur) < 2))
          {
            TINFL_HUFF_DECODE(23, counter, &r->m_tables[0]);
            if (counter >= 256)
              break;
            while (pOut_buf_cur >= pOut_buf_end) { TINFL_CR_RETURN(24, TINFL_STATUS_HAS_MORE_OUTPUT); }
            *pOut_buf_cur++ = (mz_uint8)counter;
          }
          else
          {
            int sym2; mz_uint code_len;
#if TINFL_USE_64BIT_BITBUF
            if (num_bits < 30) { bit_buf |= (((tinfl_bit_buf_t)MZ_READ_LE32(pIn_buf_cur)) << num_bits); pIn_buf_cur += 4; num_bits += 32; }
#else
            if (num_bits < 15) { bit_buf |= (((tinfl_bit_buf_t)MZ_READ_LE16(pIn_buf_cur)) << num_bits); pIn_buf_cur += 2; num_bits += 16; }
#endif
            if ((sym2 = r->m_tables[0].m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0)
              code_len = sym2 >> 9;
            else
            {
              code_len = TINFL_FAST_LOOKUP_BITS; do { sym2 = r->m_tables[0].m_tree[~sym2 + ((bit_buf >> code_len++) & 1)]; } while (sym2 < 0);
            }
            counter = sym2; bit_buf >>= code_len; num_bits -= code_len;
            if (counter & 256)
              break;

#if !TINFL_USE_64BIT_BITBUF
            if (num_bits < 15) { bit_buf |= (((tinfl_bit_buf_t)MZ_READ_LE16(pIn_buf_cur)) << num_bits); pIn_buf_cur += 2; num_bits += 16; }
#endif
            if ((sym2 = r->m_tables[0].m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0)
              code_len = sym2 >> 9;
            else
            {
              code_len = TINFL_FAST_LOOKUP_BITS; do { sym2 = r->m_tables[0].m_tree[~sym2 + ((bit_buf >> code_len++) & 1)]; } while (sym2 < 0);
            }
            bit_buf >>= code_len; num_bits -= code_len;

            pOut_buf_cur[0] = (mz_uint8)counter;
            if (sym2 & 256)
            {
              pOut_buf_cur++;
              counter = sym2;
              break;
            }
            pOut_buf_cur[1] = (mz_uint8)sym2;
            pOut_buf_cur += 2;
          }
        }
        if ((counter &= 511) == 256) break;

        num_extra = s_length_extra[counter - 257]; counter = s_length_base[counter - 257];
        if (num_extra) { mz_uint extra_bits; TINFL_GET_BITS(25, extra_bits, num_extra); counter += extra_bits; }

        TINFL_HUFF_DECODE(26, dist, &r->m_tables[1]);
        num_extra = s_dist_extra[dist]; dist = s_dist_base[dist];
        if (num_extra) { mz_uint extra_bits; TINFL_GET_BITS(27, extra_bits, num_extra); dist += extra_bits; }

        dist_from_out_buf_start = pOut_buf_cur - pOut_buf_start;
        if ((dist > dist_from_out_buf_start) && (decomp_flags & TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF))
        {
          TINFL_CR_RETURN_FOREVER(37, TINFL_STATUS_FAILED);
        }

        pSrc = pOut_buf_start + ((dist_from_out_buf_start - dist) & out_buf_size_mask);

        if ((MZ_MAX(pOut_buf_cur, pSrc) + counter) > pOut_buf_end)
        {
          while (counter--)
          {
            while (pOut_buf_cur >= pOut_buf_end) { TINFL_CR_RETURN(53, TINFL_STATUS_HAS_MORE_OUTPUT); }
            *pOut_buf_cur++ = pOut_buf_start[(dist_from_out_buf_start++ - dist) & out_buf_size_mask];
          }
          continue;
        }
#if MINIZ_USE_UNALIGNED_LOADS_AND_STORES
        else if ((counter >= 9) && (counter <= dist))
        {
          const mz_uint8 *pSrc_end = pSrc + (counter & ~7);
          do
          {
            ((mz_uint32 *)pOut_buf_cur)[0] = ((const mz_uint32 *)pSrc)[0];
            ((mz_uint32 *)pOut_buf_cur)[1] = ((const mz_uint32 *)pSrc)[1];
            pOut_buf_cur += 8;
          } while ((pSrc += 8) < pSrc_end);
          if ((counter &= 7) < 3)
          {
            if (counter)
            {
              pOut_buf_cur[0] = pSrc[0];
              if (counter > 1)
                pOut_buf_cur[1] = pSrc[1];
              pOut_buf_cur += counter;
            }
            continue;
          }
        }
#endif
        do
        {
          pOut_buf_cur[0] = pSrc[0];
          pOut_buf_cur[1] = pSrc[1];
          pOut_buf_cur[2] = pSrc[2];
          pOut_buf_cur += 3; pSrc += 3;
        } while ((int)(counter -= 3) > 2);
        if ((int)counter > 0)
        {
          pOut_buf_cur[0] = pSrc[0];
          if ((int)counter > 1)
            pOut_buf_cur[1] = pSrc[1];
          pOut_buf_cur += counter;
        }
      }
    }
  } while (!(r->m_final & 1));
  if (decomp_flags & TINFL_FLAG_PARSE_ZLIB_HEADER)
  {
    TINFL_SKIP_BITS(32, num_bits & 7); for (counter = 0; counter < 4; ++counter) { mz_uint s; if (num_bits) TINFL_GET_BITS(41, s, 8); else TINFL_GET_BYTE(42, s); r->m_z_adler32 = (r->m_z_adler32 << 8) | s; }
  }
  TINFL_CR_RETURN_FOREVER(34, TINFL_STATUS_DONE);
  TINFL_CR_FINISH

common_exit:
  r->m_n_bits = num_bits; r->m_bit_buf = bit_buf; r->m_dist = dist; r->m_counter = counter; r->m_num_extra = num_extra; r->m_dist_from_out_buf_start = dist_from_out_buf_start;
  *pIn_buf_size = pIn_buf_cur - pIn_buf_next; *pOut_buf_size = pOut_buf_cur - pOut_buf_next;
  if ((decomp_flags & (TINFL_FLAG_PARSE_ZLIB_HEADER | TINFL_FLAG_COMPUTE_ADLER32)) && (status >= 0))
  {
    const mz_uint8 *ptr = pOut_buf_next; size_t buf_len = *pOut_buf_size;
    mz_uint32 i, s1 = r->m_check_adler32 & 0xffff, s2 = r->m_check_adler32 >> 16; size_t block_len = buf_len % 5552;
    while (buf_len)
    {
      for (i = 0; i + 7 < block_len; i += 8, ptr += 8)
      {
        s1 += ptr[0], s2 += s1; s1 += ptr[1], s2 += s1; s1 += ptr[2], s2 += s1; s1 += ptr[3], s2 += s1;
        s1 += ptr[4], s2 += s1; s1 += ptr[5], s2 += s1; s1 += ptr[6], s2 += s1; s1 += ptr[7], s2 += s1;
      }
      for ( ; i < block_len; ++i) s1 += *ptr++, s2 += s1;
      s1 %= 65521U, s2 %= 65521U; buf_len -= block_len; block_len = 5552;
    }
    r->m_check_adler32 = (s2 << 16) + s1; 
		if ((status == TINFL_STATUS_DONE) && (decomp_flags & TINFL_FLAG_PARSE_ZLIB_HEADER) && (r->m_check_adler32 != r->m_z_adler32)) {
	 	 status = TINFL_STATUS_ADLER32_MISMATCH;
		}
  }
  return status;
}

// Higher depth helper functions.
void *tinfl_decompress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags) {
  tinfl_decompressor decomp; void *pBuf = NULL, *pNew_buf; size_t src_buf_ofs = 0, out_buf_capacity = 0;
  *pOut_len = 0;
  tinflIni(&decomp);
  tinfl_status status;
  for (;;) {
    size_t src_buf_size = src_buf_len - src_buf_ofs, dst_buf_size = out_buf_capacity - *pOut_len, new_out_buf_capacity;
    status = tinfl_decompress(&decomp, (const mz_uint8*)pSrc_buf + src_buf_ofs, &src_buf_size, (mz_uint8*)pBuf, pBuf ? (mz_uint8*)pBuf + *pOut_len : NULL, &dst_buf_size,
      (flags & ~TINFL_FLAG_HAS_MORE_INPUT) | TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);

    if ((status < 0) || (status == TINFL_STATUS_NEEDS_MORE_INPUT)) {
      MZ_FREE(pBuf); *pOut_len = 0; return NULL;
		}

    src_buf_ofs += src_buf_size;
    *pOut_len += dst_buf_size;

    if (status == TINFL_STATUS_DONE) 
			break;

    new_out_buf_capacity = out_buf_capacity * 2; if (new_out_buf_capacity < 128) new_out_buf_capacity = 128;
    pNew_buf = MZ_REALLOC(pBuf, new_out_buf_capacity);

    if (!pNew_buf) {
      MZ_FREE(pBuf); *pOut_len = 0; return NULL;
		}

    pBuf = pNew_buf; out_buf_capacity = new_out_buf_capacity;
  }

  return pBuf;
}



#endif // #ifndef TINFL_HEADER_FILE_ONLY

/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or
  distribute this software, either in source code form or as a compiled
  binary, for any purpose, commercial or non-commercial, and by any
  means.

  In jurisdictions that recognize copyright laws, the author or authors
  of this software dedicate any and all copyright interest in the
  software to the public domain. We make this dedication for the benefit
  of the public at large and to the detriment of our heirs and
  successors. We intend this dedication to be an overt act of
  relinquishment in perpetuity of all present and future rights to this
  software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.

  For more information, please refer to <http://unlicense.org/>
*/

Error botoxInflate(Inflatable *inflatableP) {
	Error e = SUCCESS;
	long long unsigned int expectedInflatedLen;
	if (inflatableP != NULL && inflatableP->inflatedDataP == NULL) {

		e = jbAlloc(&inflatableP->inflatedDataP, inflatableP->inflatedLen, 1);
		if (!e) {
			expectedInflatedLen = inflatableP->inflatedLen;
			inflatableP->inflatedDataP = tinfl_decompress_mem_to_heap(
                                     (const void*) inflatableP->compressedDataA, 
                                     (size_t) inflatableP->compressedLen, 
                                     &inflatableP->inflatedLen,
                                     TINFL_FLAG_PARSE_ZLIB_HEADER); 
			if (inflatableP->inflatedLen != expectedInflatedLen) {
				e = E_UNEXPECTED_DCMP_SZ;
				jbFree(&inflatableP->inflatedDataP);
			}
		}
	}
  return e;
}

void botoxDeflate(Inflatable **inflatablePP) {
  if (inflatablePP && *inflatablePP && (*inflatablePP)->inflatedDataP)
    jbFree(&(*inflatablePP)->inflatedDataP);
}


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
Error frayNew(void **fPP, U32 elemSz, U32 nElems) {
	if (elemSz <= 0 || nElems <= 0 || fPP == NULL) 
		return E_BAD_ARGS;  
	else {
    // Add 1 more element for swaps. 
		U32 *ptr = (U32*) malloc((elemSz * nElems) + ((N_PREFRAY_ELEMS + 1) * sizeof(U32)));
		if (ptr == NULL) 
			return E_NO_MEMORY;
		ptr[N_PREFRAY_ELEMS - OFFSET_INACTIVE]   = 0;       
		ptr[N_PREFRAY_ELEMS - OFFSET_N_PAUSED]   = 0;  
		ptr[N_PREFRAY_ELEMS - OFFSET_1ST_EMPTY]  = 0;       
		ptr[N_PREFRAY_ELEMS - OFFSET_ELEM_SZ]    = elemSz;
		ptr[N_PREFRAY_ELEMS - OFFSET_N_ELEMS]    = nElems;
		*fPP = (ptr + N_PREFRAY_ELEMS);
		memset(*fPP, 0, elemSz * nElems);
  }
  return SUCCESS;
}

void frayDel(void **frayPP) {
	if (frayPP != NULL && *frayPP != NULL) {
		U32 *ptr = *frayPP;
		free((ptr) - N_PREFRAY_ELEMS);
		*frayPP = NULL;
	}
}

void frayClr(void *fP) {
  memset(fP, 0, frayGetElemSz_(fP) * arrayGetNElems(fP));
  *frayGetFirstEmptyIdxP(fP) = 0;
}

// Pointers beat values. We usually inc/decrement it after using it. Avoids double-queries.
inline static U32 _frayGetFirstInactiveIdx(const void *frayP) {
  return *(((U32*) frayP - OFFSET_INACTIVE));
}

inline static U32* _frayGetFirstInactiveIdxP(const void *frayP) {
  return ((U32*) frayP - OFFSET_INACTIVE);
}

inline static U32* _frayGetNPausedP(const void *frayP) {
  return ((U32*) frayP - OFFSET_N_PAUSED);
}

inline static U32* _frayGetFirstEmptyIdxP(const void *frayP) {
  return ((U32*) frayP - OFFSET_1ST_EMPTY);
}

// Non-static versions of the above for global use
U32 frayGetFirstInactiveIdx(const void *frayP) {
  return _frayGetFirstInactiveIdx(frayP);
}

U32* frayGetFirstInactiveIdxP(const void *frayP) {
  return _frayGetFirstInactiveIdxP(frayP);
}

U32* frayGetNPausedP(const void *frayP) {
  return _frayGetNPausedP(frayP);
}

inline static U32 _frayGetNPaused(const void *frayP) {
  return *((U32*) frayP - OFFSET_N_PAUSED);
}

U32 frayGetNPaused(const void *frayP) {
  return _frayGetNPaused(frayP);
}

U32 frayGetFirstPausedIdx(const void *frayP) {
  return _frayGetFirstInactiveIdx(frayP) - _frayGetNPaused(frayP);
}

U32* frayGetFirstEmptyIdxP(const void *frayP) {
  return _frayGetFirstEmptyIdxP(frayP);
}

/* Checks if the component, wherever it is in the jagged array, is before the function's stopping point in its array. */
inline static U8 _frayElemIsActive(const void *frayP, U32 idx) {
  return idx < _frayGetFirstInactiveIdx(frayP);
}

U8 frayElemIsActive(const void *frayP, U32 idx) {
  return _frayElemIsActive(frayP, idx);
}

inline static U8 _frayHasRoom(const void *frayP) {
  return (*_frayGetFirstEmptyIdxP(frayP) < frayGetNElems_(frayP));
}

// Returns index of added element
Error frayAdd(const void *frayP, void *elemP, U32 *elemNewIdxP) {
  if (!_frayHasRoom(frayP))
    return E_FRAY_FULL;
  U32 *firstEmptyIdxP = _frayGetFirstEmptyIdxP(frayP);
  void *dstP = frayGetElemByIdx_(frayP, (*firstEmptyIdxP)++);
  memcpy(dstP, elemP, frayGetElemSz_(frayP));
  if (elemNewIdxP)
    *elemNewIdxP = *firstEmptyIdxP - 1;
  return SUCCESS;
}

static void _fraySwap(const void *frayP, U32 oldIdx, U32 newIdx) {
  // Get source, destination, and placeholder
  register void *elem1P       = frayGetElemByIdx_(frayP, oldIdx);
  register void *placeholderP = frayGetElemByIdx_(frayP, frayGetNElems_(frayP));
  register void *elem2P       = frayGetElemByIdx_(frayP, newIdx); 
  // Swap with the first inactive.
  register U32   elemSz       = frayGetElemSz_(frayP);
  memcpy(placeholderP, elem1P,       elemSz);
  memcpy(elem1P,       elem2P,       elemSz);
  memcpy(elem2P,       placeholderP, elemSz);
}

static U8 _frayElemIsPaused(const void *frayP, U32 idx) {
  U32 firstInactiveIdx = frayGetFirstInactiveIdx(frayP);
  return idx < firstInactiveIdx &&
         idx > firstInactiveIdx - _frayGetNPaused(frayP);
}

// Pausing *active* elements moves them to the first paused position.
// Pausing *inactive* elements moves them to the last paused position.
U32 frayPause(const void *frayP, U32 idx) {
  U32 *nPausedP = _frayGetNPausedP(frayP);
  if (!_frayElemIsPaused(frayP, idx) && *nPausedP < arrayGetNElems(frayP)) {
    U32 *firstInactiveIdxP = _frayGetFirstInactiveIdxP(frayP);
    ++(*nPausedP);
    U32 newIdx;
    if (idx < *firstInactiveIdxP) 
      newIdx = *firstInactiveIdxP - *nPausedP;
    else 
      newIdx = (*firstInactiveIdxP)++;
    _fraySwap(frayP, idx, newIdx);
    return newIdx;
  }
  return idx;
}

// Unlike pausing, unpausing can only send elements in one direction: leftward.
U32 frayUnpause(const void *frayP, U32 idx) {
  register U32  *nPausedP = _frayGetNPausedP(frayP);
  U32 firstInactiveIdx =  _frayGetFirstInactiveIdx(frayP);
  if (_frayElemIsPaused(frayP, idx) && *nPausedP < firstInactiveIdx) {
    U32 newIdx = firstInactiveIdx - (*nPausedP)--;
    _fraySwap(frayP, idx, newIdx);
    return newIdx;
  }
  return idx;
}

void frayPauseAll(const void *frayP) {
  *_frayGetNPausedP(frayP) = _frayGetFirstInactiveIdx(frayP);
}

void frayUnpauseAll(const void *frayP) {
  *_frayGetNPausedP(frayP) = 0;
}

// Returns new index of activated element 
U32 frayActivate(const void *frayP, U32 idx) {
  if (!_frayElemIsActive(frayP, idx)) {
    U32 *firstInactiveIdxP = _frayGetFirstInactiveIdxP(frayP);
    U32 newIdx;
    U32 nPaused = _frayGetNPaused(frayP);
    if (!nPaused)  // With no paused elements, we can blissfully single-swap.
      newIdx = (*firstInactiveIdxP)++;            // swap with first inactive (from left side)
    else {  // Otherwise, we must double-swap to preserve intermediate paused elems' contiguity.
      newIdx = *firstInactiveIdxP;                // swap with first inactive (from right side)
      _fraySwap(frayP, idx, newIdx);
      idx = newIdx;                  
      newIdx = (*firstInactiveIdxP)++ - nPaused;  // swap 1st deact. elem with 1st paused
    }
    _fraySwap(frayP, idx, newIdx);
    // Return index of activated element's new position.
    return newIdx;
  }
  return idx;
}

// Returns new index of deactivated element 
U32 frayDeactivate(const void *frayP, U32 idx) {
  if (_frayElemIsActive(frayP, idx)) {
    U32 *firstInactiveIdxP = _frayGetFirstInactiveIdxP(frayP);
    U32 newIdx;
    U32 nPaused = _frayGetNPaused(frayP);
    if (!nPaused)  // With no paused elements, we can blissfully single-swap.
      newIdx = --(*firstInactiveIdxP);            // swap with last active 
    else {  // Otherwise, we must double-swap to preserve intermediate paused elems' contiguity.
      newIdx = *firstInactiveIdxP - nPaused - 1;  // swap with last active
      _fraySwap(frayP, idx, newIdx);
      idx = newIdx;                  
      newIdx = --(*firstInactiveIdxP);  // swap last active with last paused
    }
    _fraySwap(frayP, idx, newIdx);
    // Return index of activated element's new position.
    return newIdx;
  }
  return idx;
}

// Messaging
// There is no corresponding mailboxRead() function because that's specific to each implementer.
Error mailboxWrite(Message *mailboxP, Key address, Key attn, Key cmd, Key arg) {
  Message message = {
    .address = address,
    .attn = attn,
    .cmd = cmd,
    .arg = arg 
  };
  return frayAdd((void*) mailboxP, &message, NULL);
}

Error mailboxForward(Message *mailboxP, Message *msgP) {
  return frayAdd((void*) mailboxP, msgP, NULL);
}

// TODO hard-code the used unpacker to step through it
// Strip Inflation
defineUnpackStripFunction_(1, 0x01);
defineUnpackStripFunction_(2, 0x03);
//defineUnpackStripFunction_(4, 0x0f);  // TODO uncomment
defineUnpackRemainderUnitsFunction_(1, 0x01);
defineUnpackRemainderUnitsFunction_(2, 0x03);
//defineUnpackRemainderUnitsFunction_(4, 0x0f);  // TODO uncomment

// Flips already-unpacked (8bpu) strips that need flipping. 
// Jollybean's image compressor deletes strips that're mirror-images of another.
void flipUnpackedStrips(StripSetS *stripSetP, void *outputDataP) {
  U16 *flipEndP = stripSetP->flipSet.flipIdxA + stripSetP->flipSet.nFlips;
  U32 *dstLeftWordP, *dstRightWordP;
  // 4 *unpacked* Units per U32 out of 64 Units means there are 16 U32s.
  for (U16 *flipIdxP = stripSetP->flipSet.flipIdxA; flipIdxP < flipEndP; ++flipIdxP) {
    dstLeftWordP = (U32*) outputDataP + flipIdxTo8BpuStripPtr_(*flipIdxP);
#ifdef __ARM_NEON__
    // Flip all the bytes in a 32-byte region with a divide-and-conquer approach.
    // 4 quad-words per strip
    dstRightWordP = dstLeftWordP + 12;  // 12 is for starting on words 13-16 ("word indices" 12-15)
    for (int i = 0; i < 2; ++i) {
      asm volatile (
      "vld1.32 {d0-d1}, [%0]\n\t"
      "vld1.32 {d2-d3}, [%1]\n\t"
      "vrev64.8 q0, q0\n\t" // reverses order of 8 bytes in each half-quad-word
      "vrev64.8 q1, q1\n\t"
      "vswp d0, d1\n\t" // swaps the half quad-words
      "vswp d2, d3\n\t"
      "vswp q0, q1\n\t" 
      "vst1.32 {d0-d1}, [%1]\n\t" // swaps entire quad-words
      "vst1.32 {d2-d3}, [%0]!\n\t"  // %0 increments forward; %1 backwards (next line)
      "sub %1, #16\n\t"
      : "+r" (dstLeftWordP), "+r" (dstRightWordP)
      );
    }
#else
    dstRightWordP = dstLeftWordP + 15;
    // Flip all the bytes in a 32-byte region with a divide-and-conquer approach.
    for (; dstLeftWordP < dstRightWordP; ++dstLeftWordP, --dstRightWordP) {
      // Left 4 Units
      *dstLeftWordP  = ((*dstLeftWordP & 0xFFFF0000) >> 16) | ((*dstLeftWordP & 0x0000FFFF) << 16);
      *dstLeftWordP  = ((*dstLeftWordP & 0xFF00FF00) >>  8) | ((*dstLeftWordP & 0x00FF00FF) <<  8);
      // Right 4 Units
      *dstRightWordP = ((*dstRightWordP & 0xFFFF0000) >> 16) | ((*dstRightWordP & 0x0000FFFF) << 16);
      *dstRightWordP = ((*dstRightWordP & 0xFF00FF00) >>  8) | ((*dstRightWordP & 0x00FF00FF) <<  8);  // Put right 4 Units into left 4 Units
      swap_(*dstLeftWordP, *dstRightWordP);
    }
#endif
  }
}

defineInflateStripsWithBpu_(1);
defineInflateStripsWithBpu_(2);
//defineInflateStripsWithBpu_(4);

// TODO delete everything below once bug is fixed
// Stripped data inflation
#ifdef __ARM_NEON__
__inline__ static void _unpackStrip4Bpu(U32 **srcStripPP, U32 **dstStripPP) {
  U32 *srcStripP = *srcStripPP;
  U32 *dstStripP = *dstStripPP;
  /* 6 instructions neon VS 40-58 instructions regular */
  /* Although the outer loop appears unnecessary for 1 quadword per 1Bpu strip,
     it safeguards us from changes in the number of units per strip. */
  for (int i = 0; i < N_QUADWORDS_PER_4BPU_STRIP; ++i) {  /* keeping this useless loop here for when I chagne to 128-unit strips. */
    asm volatile inline (
    "vmov.u8 q10, #" #maskByte_ "nt"   /* q10 = mask */
    "vld1.32 {d0-d1}, [%0]!nt"    /* q0 (aka d0-d1) = packed indices */
    : "+r&" (srcStripP)
    );
    for (int j = 0; j < N_4BPU_UNITS_PER_BYTE; ++j) {
      asm volatile inline (
      "vand q1, q0, q10nt"          /* q1 = unpacked indices*/
      "vst1.32 {d2-d3}, [%0]!nt"
      "vshr.u8 q0, #" #Bpu_ "nt"            /* shift q0 over 1*/
      : "+r&" (dstStripP)
      );
    }
  }
}
#else
//static int k = 0;
__inline__ static void _unpackStrip4Bpu(U32 **srcStripPP, U32 **dstStripPP) {
  U32 *srcStripP = *srcStripPP;
  U32 *dstStripP = *dstStripPP;
  /* Although the first loop line appears unnecessary for 1 word per 1Bpu strip,
     it safeguards us from changes in the number of units per strip. */
  for (int i = 0; i < N_WORDS_PER_4BPU_STRIP; ++i) {
    for (int j = 0; j < N_BITS_PER_BYTE; j += SHIFT_INCREMENT_4BPU) {
      *dstStripP++ =  (*srcStripP >> j) & 0x0f0f0f0f;
    }
  }
  //printf("\tstrip #: %d\n", k++);
}
#endif

__inline__ static void _unpackRemainderUnits4Bpu(U8 *byteA, U8 *outputByteP, U32 nRemainderUnits) {
  U8 *byteP = byteA;
  U8 *byteEndP = byteP + countWholeBytesFor4BpuUnits_(nRemainderUnits);
  /* Handle all the whole bytes of units. */
  //printf("# whole bytes of remainder data: %d\n", byteEndP - byteP);
  while (byteP < byteEndP)
    for (U8 i = 0; i < N_BITS_PER_BYTE; i += SHIFT_INCREMENT_4BPU)
      *outputByteP++ =  (*(byteP++) >> i) & 0x0f;
  /* Handle the last, partial byte of data. */
  U8 iEnd = countUnitsInPartialByte4BPU_(nRemainderUnits);
  //printf("# partial bytes of remainder data: %d\n", iEnd);
  for (U8 i = 0; i < iEnd; i += SHIFT_INCREMENT_4BPU)
    *outputByteP++ =  (*(byteP++) >> i) & 0x0f;
}

void inflateStripsWithBpu4 (StripSetS *stripSetP, StripMapS *stripMapP, U32 *dstStripP) {
  U32 *srcStripP; 
  U32 *dstStripOriginP = dstStripP; /* keep track of beginning as pointer gets incremented */ 
  /* Count remainder of pixels to process after all the whole strips. */ 
  //printf("stripSetP->nUnits: %d\n", stripSetP->nUnits);
  //printf("# bytes: %d\n", stripSetP->nUnits / 2);
  U32 nWholeStrips = countWholeStrips_(stripSetP->nUnits); 
  U32 nRemainderUnits = countRemainderUnits_(stripSetP->nUnits); 
  /* Mapped stripsets need to be both unpacked and indexed. They may need strips to be flipped too. */ 
  /* First read all mapped strips into the target colormap. */
  // Things to check:
  //    1) size of strip map's inflated data
  //    2) size of strip set's inflated data
  //    3) counting of whole strips
  //    4) counting of remainder units
  //    5) integrity of stripIdxTo4BpuStripPtr_(*stripMapElemP)
  if (stripMapP) {
    U16 *mapEndP = ((U16*) stripMapP->stripMapInfP->inflatedDataP) + nWholeStrips;
    //printf("nWholeStrips: %d\n", nWholeStrips);
    //printf("nRemainderUnits: %d\n", nRemainderUnits);
    for (U16 *stripMapElemP = (U16*) stripMapP->stripMapInfP->inflatedDataP; stripMapElemP < mapEndP; stripMapElemP++) {
      srcStripP = ((U32*) stripSetP->stripSetInfP->inflatedDataP + stripIdxTo4BpuStripPtr_(*stripMapElemP));  
      _unpackStrip4Bpu(&srcStripP, &dstStripP);
    }
    srcStripP = (U32*) stripSetP->stripSetInfP->inflatedDataP + stripSetP->nStrips - 1;
    //printf("doing partial strip now\n");
    _unpackRemainderUnits4Bpu((U8*) srcStripP, (U8*) dstStripP, nRemainderUnits);
    /* Then flip whatever strips need flipping. Remember data's already expanded to U8s! */
    if (stripSetP->flipSet.nFlips) {
      //printf("doing flips\n");
      flipUnpackedStrips(stripSetP, dstStripOriginP);
    }
  } 
  /* Unmapped stripsets are already ordered, so they only need to be unpacked. */
  else {
    U32 *srcEndP = stripSetP->stripSetInfP->inflatedDataP + stripIdxTo1BpuStripPtr_(nWholeStrips);
    for (U32 *srcStripP = stripSetP->stripSetInfP->inflatedDataP; srcStripP < srcEndP; srcStripP++) 
      _unpackStrip4Bpu(&srcStripP, &dstStripP);
    _unpackRemainderUnits4Bpu((U8*) srcStripP, (U8*) dstStripP, nRemainderUnits);
  }
}
