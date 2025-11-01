#ifndef MEM_H
#define MEM_H

#include "data/common.h"

#if (__WORDSIZE == 32 )
#define MODULO_ (3)
#elif (__WORDSIZE == 64 )
#define MODULO_ (7)
#else
static_assert( 0, "Jollybean supports only 32- and 64-bit systems.");
#endif

#define wordAlign_(x_) ( ( ( N_BYTES_PER_WORD - ( x_ & MODULO_ ) ) & MODULO_ ) + x_ )

// TODO optimize these later
// TODO make these easily configurable
#define PRM_SZ wordAlign_(1000000)
#define GEN_SZ wordAlign_(1000000)
#define TXT_SZ wordAlign_(1000000)
#define IMG_SZ wordAlign_(1000000)
#define AUD_SZ wordAlign_(1000000)
#define TMP_SZ wordAlign_(1000000)

typedef enum PoolId { PERMANENT, GENERAL, TEXT, GRAPHIC, AUDIO, TEMPORARY, N_POOLS } PoolId;

// Keep the definition of memory arena here since nobody else will need it.
typedef struct MemPool {
#ifndef NDEBUG
  const size_t memAllocated;  // This can be done away with once we prove this out.
  size_t memRemaining;       // This can be done away with once we prove this out.
#endif
  U8* nextFreeP;
  const U8* originP;
} MemPool;

// Public functions start here.
void memClr ( const PoolId poolId );

// Allocate memory in the arena at a word-aligned address.
void* memAdd ( const size_t numBytes, const PoolId poolId );

void memRstAll();  // resets all but permanent
void memRst( const PoolId poolId );
void memRewind( const PoolId poolId );

#ifndef NDEBUG
MemPool* memGetPoolP( const PoolId poolId );
void memReport();

#else
#define memReport
#endif

#endif  // #ifndef MEM_H
