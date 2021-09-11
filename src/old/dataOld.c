#include "jb.h"

void vec2Ini(Vector2 *vector2, Sint8 x, Sint8 y) {
  vector2->x = x;
  vector2->y = y;
}

void vec3Ini(Vector3 *vector3, Sint8 x, Sint8 y, Sint8 z) {
  vector3->x = x;
  vector3->y = y;
  vector3->z = z;
}

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
  mz_uint32 m_state, m_n_bits, m_zhdr0, m_zhdr1, m_z_adler32, m_final, m_type, m_check_adler32, m_dist, m_counter, m_n_extra, m_table_sizes[TINFL_MAX_HUFF_TABLES];
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

  num_bits = r->m_n_bits; bit_buf = r->m_bit_buf; dist = r->m_dist; counter = r->m_counter; num_extra = r->m_n_extra; dist_from_out_buf_start = r->m_dist_from_out_buf_start;
  TINFL_CR_BEGIN

  bit_buf = num_bits = dist = counter = num_extra = r->m_zhdr0 = r->m_zhdr1 = 0; r->m_z_adler32 = r->m_check_adler32 = 1;
  if (decomp_flags & TINFL_FLAG_PARSE_ZLIB_HEADER)
  {
    TINFL_GET_BYTE(1, r->m_zhdr0); TINFL_GET_BYTE(2, r->m_zhdr1);
    counter = (((r->m_zhdr0 * 256 + r->m_zhdr1) % 31 != 0) || (r->m_zhdr1 & 32) || ((r->m_zhdr0 & 15) != 8));
    if (!(decomp_flags & TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF)) counter |= (((1U << (8U + (r->m_zhdr0 >> 4))) > 32768U) || ((out_buf_size_mask + 1) < (size_t)(1U << (8U + (r->m_zhdr0 >> 4)))));
    if (counter) { printf("2\n"); TINFL_CR_RETURN_FOREVER(36, TINFL_STATUS_FAILED); }
  }

  do
  {
    TINFL_GET_BITS(3, r->m_final, 3); r->m_type = r->m_final >> 1;
    if (r->m_type == 0)
    {
      TINFL_SKIP_BITS(5, num_bits & 7);
      for (counter = 0; counter < 4; ++counter) { if (num_bits) TINFL_GET_BITS(6, r->m_raw_header[counter], 8); else TINFL_GET_BYTE(7, r->m_raw_header[counter]); }
      if ((counter = (r->m_raw_header[0] | (r->m_raw_header[1] << 8))) != (mz_uint)(0xFFFF ^ (r->m_raw_header[2] | (r->m_raw_header[3] << 8)))) { printf("3\n"); TINFL_CR_RETURN_FOREVER(39, TINFL_STATUS_FAILED); }
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
            printf("4\n");
            TINFL_CR_RETURN_FOREVER(40, TINFL_STATUS_FAILED);
          }
        }
        n = MZ_MIN(MZ_MIN((size_t)(pOut_buf_end - pOut_buf_cur), (size_t)(pIn_buf_end - pIn_buf_cur)), counter);
        TINFL_MEMCPY(pOut_buf_cur, pIn_buf_cur, n); pIn_buf_cur += n; pOut_buf_cur += n; counter -= (mz_uint)n;
      }
    }
    else if (r->m_type == 3)
    {
      printf("5\n");
      TINFL_CR_RETURN_FOREVER(10, TINFL_STATUS_FAILED);
    }
    else
    {
      if (r->m_type == 1)
      {
        mz_uint8 *p = r->m_tables[0].m_code_size; mz_uint i;
        r->m_table_sizes[0] = 288; r->m_table_sizes[1] = 32; TINFL_MEMSET(r->m_tables[1].m_code_size, 5, 32);
        for ( i = 0; i <= 143; ++i) *p++ = 8; for ( ; i <= 255; ++i) *p++ = 9; for ( ; i <= 279; ++i) *p++ = 7; for ( ; i <= 287; ++i) *p++ = 8;
      }
      else
      {
        for (counter = 0; counter < 3; counter++) { TINFL_GET_BITS(11, r->m_table_sizes[counter], "\05\05\04"[counter]); r->m_table_sizes[counter] += s_min_table_sizes[counter]; }
        MZ_CLEAR_OBJ(r->m_tables[2].m_code_size); for (counter = 0; counter < r->m_table_sizes[2]; counter++) { mz_uint s; TINFL_GET_BITS(14, s, 3); r->m_tables[2].m_code_size[s_length_dezigzag[counter]] = (mz_uint8)s; }
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
          printf("6\n");
          TINFL_CR_RETURN_FOREVER(35, TINFL_STATUS_FAILED);
        }
        for (tree_next = -1, sym_index = 0; sym_index < r->m_table_sizes[r->m_type]; ++sym_index)
        {
          mz_uint rev_code = 0, l, cur_code, code_size = pTable->m_code_size[sym_index]; if (!code_size) continue;
          cur_code = next_code[code_size]++; for (l = code_size; l > 0; l--, cur_code >>= 1) rev_code = (rev_code << 1) | (cur_code & 1);
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
              printf("7\n");
              TINFL_CR_RETURN_FOREVER(17, TINFL_STATUS_FAILED);
            }
            num_extra = "\02\03\07"[dist - 16]; TINFL_GET_BITS(18, s, num_extra); s += "\03\03\013"[dist - 16];
            TINFL_MEMSET(r->m_len_codes + counter, (dist == 16) ? r->m_len_codes[counter - 1] : 0, s); counter += s;
          }
          if ((r->m_table_sizes[0] + r->m_table_sizes[1]) != counter)
          {
            printf("8\n");
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
          printf("9\n");
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
  r->m_n_bits = num_bits; r->m_bit_buf = bit_buf; r->m_dist = dist; r->m_counter = counter; r->m_n_extra = num_extra; r->m_dist_from_out_buf_start = dist_from_out_buf_start;
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
    r->m_check_adler32 = (s2 << 16) + s1; if ((status == TINFL_STATUS_DONE) && (decomp_flags & TINFL_FLAG_PARSE_ZLIB_HEADER) && (r->m_check_adler32 != r->m_z_adler32)) status = TINFL_STATUS_ADLER32_MISMATCH;
  }
  return status;
}

// Higher depth helper functions.
void *tinfl_decompress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len, size_t *pOut_len, int flags)
{
  tinfl_decompressor decomp; void *pBuf = NULL, *pNew_buf; size_t src_buf_ofs = 0, out_buf_capacity = 0;
  *pOut_len = 0;
  tinflIni(&decomp);
  tinfl_status status;
  for ( ; ; )
  {
    size_t src_buf_size = src_buf_len - src_buf_ofs, dst_buf_size = out_buf_capacity - *pOut_len, new_out_buf_capacity;
    status = tinfl_decompress(&decomp, (const mz_uint8*)pSrc_buf + src_buf_ofs, &src_buf_size, (mz_uint8*)pBuf, pBuf ? (mz_uint8*)pBuf + *pOut_len : NULL, &dst_buf_size,
      (flags & ~TINFL_FLAG_HAS_MORE_INPUT) | TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);
    if ((status < 0) || (status == TINFL_STATUS_NEEDS_MORE_INPUT))
    {
      printf("[in if] stat was %d before decomp returned\n", status);
      MZ_FREE(pBuf); *pOut_len = 0; return NULL;
    }
    src_buf_ofs += src_buf_size;
    *pOut_len += dst_buf_size;
    if (status == TINFL_STATUS_DONE) break;
    new_out_buf_capacity = out_buf_capacity * 2; if (new_out_buf_capacity < 128) new_out_buf_capacity = 128;
    pNew_buf = MZ_REALLOC(pBuf, new_out_buf_capacity);
    if (!pNew_buf)
    {
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

Error decompress_media(const unsigned char *src, const int src_len, void **dst, size_t *dst_len) {
  Error stat;
  long long unsigned int orig_expected_len;
  
  stat = 0;
  
  *dst = malloc((size_t) *dst_len);
  if (*dst == NULL) {
    print_no_mem("decompress_media");
    stat = 1;
  }
  else {
    orig_expected_len = *dst_len;
    *dst = tinfl_decompress_mem_to_heap(src, src_len, dst_len, TINFL_FLAG_PARSE_ZLIB_HEADER); /* dst_len should come back as actual len */
    if (orig_expected_len != *dst_len) {
      printf("[decompress_media] expected len %lld doesn't match actual len %d\n", orig_expected_len, *dst_len);
      stat = 1;
    }
    
  }
  return stat;
}

/*****************************************************/
/*****************LINKED LISTS************************/
/*****************************************************/
Error llIni(LinkedList **ll) {
  *ll = malloc(sizeof(LinkedList));
  if (*ll == NULL) {
    print_no_mem("init_linked_list");
    return 1;
  }
  (*ll)->head = NULL;
  (*ll)->tail = NULL;
  return 0;
}

void llDelNode(LinkedList *ll, ListNode *node, ListNode **next, void **data) {
  if (data != NULL) {
    *data = node->data;
  }
  if (next != NULL) {
    *next = node->next;
  }
  /* if this is somewhere inside the linked list */
  if (node->prev != NULL && node->next != NULL) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
  }
  /* if this is the head */
  else if (ll->head == node) {  
    ll->head = node->next;
    node->next->prev = NULL;
  }
  /* if this is the tail */
  else if (ll->tail == node) {
    ll->tail = node->prev;
    node->prev->next = NULL;
  }

  free(node);
}

void llDel(LinkedList *ll) {
  ListNode *node;

  if (ll != NULL && ll->head != NULL) {
    node = ll->head;
    while (node != NULL) {
      free(node->data);
      llDelNode(ll, node, NULL, NULL);
    }
    free(ll);
  }
}

Error list_insert_head(LinkedList *ll, void *data) {
  ListNode *node;
  if (ll != NULL) {
    node = malloc(sizeof(node));
    if (node == NULL) {
      print_no_mem("list_insert_head (1)");
      return 1;
    }

    if (ll->head == NULL) {
      ll->head = node;
      ll->head = ll->tail;
    }
    else {
      node->next = ll->head;
      ll->head->prev = node;
      ll->head = node;
    }

    return 0;
  }
  else {
    printf("[list_insert_head] There's no list to insert into. ");
      return 1;
  }
}

Error list_insert_tail(LinkedList *ll, void *data) {
  ListNode *node;
  if (ll != NULL) {
    node = malloc(sizeof(node));
    if (node == NULL) {
      print_no_mem("list_insert_tail (1)");
      return 1;
    }

    if (ll->tail == NULL) {
      ll->tail = node;
      ll->head = ll->tail;
    }
    else {
      node->prev = ll->tail;
      ll->tail->next = node;
      ll->tail = node;
    }

    return 0;
  }
  else {
    printf("[list_insert_tail] There's no list to insert into. ");
    return 1;
  }
}

ListNode* llFindNode(LinkedList *ll, void *data) {
  ListNode *node;
  if (ll != NULL && ll->head != NULL) {
    for (node = ll->head; node != NULL; node = node->next) {
      if (node->data == data) {
        return node;
      }
    }
  }
  return NULL;
}


/******************************/
/**********QUAD TREES**********/
/******************************/

Error qnIni(QuadNode **qn, Uint8 depth, Vector2 *tl, Vector2 *br) {
  Error stat = 0;
  *qn = malloc(sizeof(QuadNode));
  memset((*qn)->nodes, 0, 4 * sizeof(QuadNode*));  /* otherwise this'll run forever */
  if (*qn == NULL) {
    print_no_mem("init_qn(1)");
    return 1;
  }
  stat = llIni(&(*qn)->elems);
  if (!stat) {
    (*qn)->nElems = 0;
    (*qn)->depth = depth;
    memcpy(&(*qn)->tl_corner, tl, sizeof(Vector2));
    memcpy(&(*qn)->br_corner, br, sizeof(Vector2));
  }
  
  return stat;
}

Error qn_subdivide(QuadTree *qt, QuadNode *qn) {
  Error stat = 0;
  Uint8 i;
  Vector2 tl, br;
  QuadNode *qn_sub;
  Sprite *s;
  ListNode *node;

  /* Prevent more than the legal limit of subdivisions. */
  if (qn->depth >= qt->maxDepth) {
    return 0;
  }

  /* Break this quad node up into four more quad nodes. */
  for (i = 0; i < 4 && !stat; i++) {
    tl.x = qn->tl_corner.x + (((i & 0x01) * qn->br_corner.x) >> 1);  /* 1st and 3rd quads are left  */
    br.y = qn->tl_corner.y + (((i & 0xFE) * qn->br_corner.y) >> 2);  /* >= idx 2 are bottom 2 quads. Mult by 2, so div by 4. */
    stat = qnIni(&qn->nodes[i], qn->depth + 1, &tl, &br);
  }

  /* If this node had elems in it before, split them all up among its new sub-quadrants. Double checking max pop just in case. */
  if (qn->nElems > qt->max_nElems) {
    node = qn->elems->head;
    while (node != NULL) {
      llDelNode(qn->elems, node, &node, (void**)&s);  /* extract sprite from front of linked list */
      qn_sub = qn;      
      tl.x = s->rect.x;
      tl.y = s->rect.y;
      br.x = s->rect.x + s->rect.w;
      br.y = s->rect.y + s->rect.h;
      qn_query(qt, &tl, &br, &qn_sub);
      stat = qn_insert(qt, qn_sub, s);
      if (stat) {
        break;
      }
      if (qn_sub->nElems >= qt->max_nElems) {
        qn_subdivide(qt, qn_sub);
      }
    }
  }
  qn->nElems = 0;

  return stat;
}

Error qtIni(QuadTree **qt, Uint8 maxDepth, Vector2 *tree_rect_dims, Uint8 max_nElems) {
  Error stat = 0;
  static Vector2 origin = {0};

  *qt = malloc(sizeof(QuadTree));
  if (*qt == NULL) {
    print_no_mem("qtIni(1)");
    return 1;
  }
  qnIni(&(*qt)->root, 0, &origin, tree_rect_dims);
  stat = qn_subdivide(*qt, (*qt)->root);
  if (!stat) {
    (*qt)->maxDepth    = maxDepth;
    (*qt)->max_nElems = max_nElems;
  }

  return stat;
}

void qn_query(QuadTree *qt, Vector2 *tl, Vector2 *br, QuadNode **qn) {
  Uint8 idx;
  Sint32 center_x, center_y;

  /* If ListNode's pop is under limit, it hasn't needed to be split yet. So obj fits in this ListNode. */
  if ((*qn)->nElems <= qt->max_nElems) {
    return;
  }
  /* If object is completely contained by this ListNode, return this ListNode's address. */
  center_x = ((*qn)->tl_corner.x + (*qn)->br_corner.x) >> 1;
  center_y = ((*qn)->tl_corner.y + (*qn)->br_corner.y) >> 1;
  if ((tl->x < center_x && br->x >= center_x) || (tl->y < center_y && br->y >= center_y)) {
    *qn = (*qn);
    return;
  }
  /* Otherwise, dig down deeper into the appropriate ListNode. */
  idx  = ((tl->x >= center_x) ? 1 : 0);
  idx &= ((tl->y >= center_y) ? 2 : 0);
  if ((*qn)->nodes[idx] == NULL) {
    qn_query(qt, tl, br, qn);
  }
}

void qt_query(QuadTree *qt, Vector2 *tl, Vector2 *br, QuadNode **qn) {
  Error stat = 0;

  /* Don't bother searching if pos is outside quadtree. */
  if (tl->x < 0 || br->x >= qt->root->br_corner.x || tl->y < 0 || br->y >= qt->root->br_corner.y) {
    *qn = NULL;
    return;
  }

  *qn = qt->root;

  qn_query(qt, tl, br, qn);
}
/* TODO: incerement parent nodes' counts too so you know when to merge nodes back together. */

Error qn_insert(QuadTree *qt, QuadNode *qn, Sprite *s) {
  Error stat = 0;
  stat = list_insert_tail(qn->elems, s);
  if (!stat) {
    /* If this ListNode's overflwoing and you haven't reached the max depth yet, subdivide it. (Subdivide handles depth check.) */
    if ((++qn->nElems) >= qt->max_nElems) {
      stat = qn_subdivide(qt, qn);
    }
  }

  return stat;
}

void qn_remove(QuadTree *qt, QuadNode *qn, Sprite *s) {
  Error stat = 0;
  ListNode *node;

  node = llFindNode(qn->elems, s);
  if (node != NULL) {
    llDelNode(qn->elems, node, NULL, NULL);
  }
  
  qn->nElems--;
}

Error qt_insert(QuadTree *qt, Sprite *s) {
  Error stat = 0;
  Vector2 tl, br;
  QuadNode *qn = NULL;

  tl.x = s->rect.x;
  tl.y = s->rect.y;
  br.x = s->rect.x + s->rect.w;
  br.y = s->rect.y + s->rect.h;

  qt_query(qt, &tl, &br, &qn);
  if (qn != NULL) {
    stat = qn_insert(qt, qn, s);
  }

  return stat;
}

/* there should be a way to only check collisions on sprites that are AFFECTED by collisions */
Error qnMove(QuadTree *qt, QuadNode *qn, Sprite *s) {
  qn_remove(qt, qn, s);
  return qt_insert(qt, s);
}

void qnDel(QuadNode **qn) {
  Uint8 i;
  for (i = 0; i < 4; i++) {
    if ((*qn)->nodes[i] !=  NULL) {
      qnDel(&(*qn)->nodes[i]);
    }
  }
  free(*qn);
  *qn = NULL;
}

void qtDel(QuadTree **qt) {
  qnDel(&(*qt)->root);
  free(*qt);
  *qt = NULL;
}



/************************************/
/********** ARRAYS (1D & 2d) ********/
/************************************/
Error arrayNew(void **arryPP, Uint32 elemSz, Sint32 nElems) {
	Sint32 *ptr;

	if (elemSz <= 0 || nElems < 0 || arryPP == NULL) {
		return 1;  /* TODO: replace with reasonable error type */
	}
	if (nElems == 0) {
		*arryPP = NULL;
	}
	else {
		ptr = malloc((elemSz * nElems) + (2 * sizeof(Sint32)));
		if (ptr != NULL) {
			ptr[0] = elemSz;
			ptr[1] = nElems;
			*arryPP = (ptr + 2);
		}
	}
	return 0;
}

void arrayDel(void *arryP) {
	if (arryP != NULL) {
		free(arryP - 2);
	}
}

Sint32 arrayGetNElems(void *arryP) {
	Uint32 *ptr;
	if (arryP == NULL) {
		return 0;
	}
	else {
		ptr = arryP;
		return *(ptr - 1);
	}
}

inline Sint32 arrayGetElemSz(const void *arryP) {
	Uint32 *ptr;

	if (arryP == NULL) {
		return 0;
	}
	else {
		return *(ptr - 2);
	}
}

Error array2dNew(Uint32 elemSz, Sint32 xdim, Sint32 ydim, void ***arry2PPp) {
	Error stat;
	Uint32 i;

	if (arry2PPp == NULL) {
		return 1;
	}
	else if (elemSz <= 0) {
		return 1;
	}
	else if (xdim < 0 || ydim < 0) {
		return 1;
	}

	*arry2PPp = NULL;
	if (xdim == 0 || ydim == 0) {
		return 0;
	}

	stat = arrayNew(sizeof(void*), xdim, (void**) arry2PPp);
	if (!stat) {
		stat = arrayNew(elemSz, xdim * ydim, &(*arry2PPp[0]));
		if (!stat) {
			for (i = 1; i < xdim; i++) {
				(*arry2PPp)[i] = ((char*)(*arry2PPp)[0]) + (elemSz * ydim * i);
			}
		}
		else {
			arrayDel(*arry2PPp);
			*arry2PPp = NULL;
		}
	}	
	return stat;
}

void array2dDel(void **arry2PP) {
	if (arry2PP != NULL) {
		arrayDel(arry2PP[0]);
		arrayDel(arry2PP);
	}
}

Sint32 array2dGetNElems(void **arry2PP, Vector2 *dimsP) {
	Sint32 nElems;

	if (arry2PP == NULL) {
		nElems = 0; 
		if (dimsP != NULL) {
			dimsP->x = dimsP->y = 0;
		}
	}
	else {
		nElems = arrayGetNElems(arry2PP[0]);
		if (dimsP != NULL) {
			dimsP->x = arrayGetNElems(arry2PP);
			dimsP->y = (dimsP->x == 0) ? 0: (nElems / dimsP->x);
		}
	}
	return nElems;
}

Error get_a2_elemSz(void **arry2PP) {
	if (arry2PP == NULL) {
		return 1;
	}
	else {
		return getA1Sz(arry2PP[0]);
	}
}

/****** Maps ******/
inline int mapGetIdx(int flagKey, Map *mapP) {
	return bitCountLUT[mapP->idxFlags & (keyFlag - 1)];
}	

inline void* mapGet(int flagKey, void *mapP) {
	// Make sure flag key has an index before bothering to look.
	if (mapP != NULL && mapP->mapA != NULL && (flagKey & mapP->idxFlags)) {
		Uint8 idx = mapGetIdx(flagKey, mapP);
		if (idx != 0xFF)
			return mapP->mapA + (idx * arrayGetElemSz(mapP->mapA));
	}
	return null;
}

inline Error mapNew(Map **mapPP, Uint8 elemSz, Uint16 nElems) {
	Error e = jbAlloc(mapPP, sizeof(Map), 1);
	if (!e)
		e = arrayNew((*mapPP)->mapA, elemSz, nElems);
	return e;
}

inline void mapDel(Map **mapPP) {
	arrayDel(&(*mapPP)->data);
	jbFree((void**) mapPP);
}
