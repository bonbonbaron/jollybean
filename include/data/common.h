#ifndef COMMON_H
#define COMMON_H
//#include <bits/pthreadtypes.h>
// #include <stdlib.h>  // see if getting rid of malloc precludes this
#ifndef NDEBUG
#include <stdio.h>
#endif
#include <string.h>
#if 0
#define NDEBUG
#endif
#include <assert.h>

typedef unsigned char U8;
typedef signed char S8;
typedef unsigned short U16;
typedef signed short S16;
typedef unsigned int U32;
typedef signed int S32;
typedef U8 Key;
typedef U8 Bln;   // Boolean

typedef union Generic {
  U8 u8;
  S8 s8;
  U16 u16;
  S16 s16;
  U32 u32;
  S32 s32;  // e.g. hurt target enemy with X amount of damage
  struct {
    U8 u8a;   // this'll be good for having *up to* four  byte-sized pieces of generic data
    U8 u8b;
    U8 u8c;
    U8 u8d;
  } u8quad;
  struct {
    S8 s8a;   // this'll be good for having *up to* four  byte-sized pieces of generic data
    S8 s8b;
    S8 s8c;
    S8 s8d;
  } s8quad;
  struct {
    U16 u16a;  // These'll be useful for telling someone to go *to* a location.
    U16 u16b;  // (and for other generic pairwise data, of course)
  } u16pair;
  struct {
    S16 s16a;  // These'll be useful for telling someone to go *to* a location.
    S16 s16b;  // (and for other generic pairwise data, of course)
  } s16pair;
} Generic;

#define N_BITS_PER_BYTE (8)
#define N_BITS_PER_WORD (N_BITS_PER_BYTE * sizeof(size_t))
#define N_BYTES_PER_WORD (sizeof(size_t))

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

#define structMemberOffset_(structType_, memberName_) (size_t) &(((structType_*) NULL)->memberName_)
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
