#ifndef COMMON_H
#define COMMON_H
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
	E_BAD_KEY,  // 8
  E_MAP_FULL,
  E_MAP_WRONG_ELEM_TYPE,  // 10
	E_MSG_TO_ECS_TYPE_MISMATCH,
	E_MSG_TO_ID_MISMATCH,
	E_MSG_INVALID_CMD,
	E_UNEXPECTED_DCMP_SZ,
	E_UNSUPPORTED_PIXEL_FORMAT,
	E_SYS_CMP_MISMATCH,
  E_ENTITY_NOT_IN_SYSTEM,
  E_BAD_COMPONENT_TYPE,
	E_NULL_VAR,
	E_FRAY_FULL,  // 20
  E_FRAY_SEGFAULT,
  E_MAILBOX_BAD_ADDRESS,
  E_MAILBOX_BAD_RECIPIENT,
  E_MAILBOX_BAD_COMMAND,
  E_MAILBOX_BAD_ARG,
  E_MAILBOX_BAD_ATTN,
  E_BB_GENES_DONT_DO_INITIALIZATION,
  E_NULL_GENE_DATA,
  E_INVALID_GENE_CLASS,
  E_PAUSE,  // 30
  E_UNPAUSE,
  E_QUIT
} Error;

#define structMemberOffset_(structType_, memberName_) (U32) &(((structType_*) NULL)->memberName_)
#define structMemberSizeof_(structType_, memberName_) sizeof(((structType_*) NULL)->memberName_)
#define stitch_(dstP_, dstType_, dstMemberName_, srcP_) \
  /* Throw a compile-time error if stitching mismatches destination. */\
  static_assert(structMemberSizeof_(dstType_, dstMemberName_) == sizeof(*srcP_), \
      "Memcpy'ing data to a differently sized destination.");\
  memcpy((U8*) dstP_ + structMemberOffset_(dstType_, dstMemberName_), srcP_, sizeof(*srcP_))
#define memcpy_(dstP_, srcP_) \
  /* Throw a compile-time error if stitching mismatches destination. */\
  static_assert(sizeof(*(dstP_)) == sizeof(*(srcP_)), \
      "Memcpy'ing data to a differently sized destination.");\
  memcpy(dstP_, srcP_, sizeof(*(srcP_)))


#endif  // #ifndef COMMON_H
