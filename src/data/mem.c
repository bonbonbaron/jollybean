#include "data/mem.h"

#if (__WORDSIZE == 32 )
#define MODULO_ (3)
#elif (__WORDSIZE == 64 )
#define MODULO_ (7)
#else
static_assert( 0, "Jollybean supports only 32- and 64-bit systems.");
#endif

#define wordAlign_(x_) ( ( ( N_BYTES_PER_WORD - ( x_ & MODULO_ ) ) & MODULO_ ) + x_ )

#ifndef NDEBUG
static char* _poolNames[] = {
  "Permament",
  "General",
  "Text",
  "Image",
  "Audio",
  "Temporary"
};
#endif

// TODO optimize these later
// TODO make these easily configurable
#define PRM_SZ wordAlign_(1000000)
#define GEN_SZ wordAlign_(1000000)
#define TXT_SZ wordAlign_(1000000)
#define IMG_SZ wordAlign_(1000000)
#define AUD_SZ wordAlign_(1000000)
#define TMP_SZ wordAlign_(1000000)

// Keep the definition of memory arena here since nobody else will need it.
typedef struct MemPool {
#ifndef NDEBUG
  const size_t memAllocated;  // This can be done away with once we prove this out.
  size_t memRemaining;       // This can be done away with once we prove this out.
#endif
  U8* nextFreeP;
  const U8* originP;
} MemPool;

// The double ampersand prevents any branching logic for checking if addition is a multiple of word-size.
// numActualBytes =  + numBytes;
// // The double ampersand prevents any branching logic for checking if addition is a multiple of word-size.
// numActualBytes =  + numBytes;
static unsigned char _data[
  PRM_SZ +
  GEN_SZ +
  TXT_SZ +
  IMG_SZ +
  AUD_SZ +
  TMP_SZ];

#define PRM_ORIGIN_IDX (0)
#define GEN_ORIGIN_IDX (PRM_ORIGIN_IDX + PRM_SZ)
#define TXT_ORIGIN_IDX (GEN_ORIGIN_IDX + GEN_SZ)
#define IMG_ORIGIN_IDX (TXT_ORIGIN_IDX + TXT_SZ)
#define AUD_ORIGIN_IDX (IMG_ORIGIN_IDX + IMG_SZ)
#define TMP_ORIGIN_IDX (AUD_ORIGIN_IDX + AUD_SZ)

  typedef struct Mem {
    const unsigned char* dataP;
    MemPool poolA[N_POOLS];
  } Mem;

static Mem _mem = {
  .dataP = _data,
  .poolA = {
    // Permanent pool
    {
#ifndef NDEBUG
      .memAllocated = PRM_SZ,
      .memRemaining = PRM_SZ,
#endif
      .nextFreeP = &_data[PRM_ORIGIN_IDX],
      .originP = &_data[PRM_ORIGIN_IDX]
    },
    // General pool
    {
#ifndef NDEBUG
      .memAllocated = GEN_SZ,
      .memRemaining = GEN_SZ,
#endif
      .nextFreeP = &_data[GEN_ORIGIN_IDX],
      .originP = &_data[GEN_ORIGIN_IDX]
    },
    // Text pool
    {
#ifndef NDEBUG
      .memAllocated = TXT_SZ,
      .memRemaining = TXT_SZ,
#endif
      .nextFreeP = &_data[TXT_ORIGIN_IDX],
      .originP = &_data[TXT_ORIGIN_IDX]
    },
    // Image pool
    {
#ifndef NDEBUG
      .memAllocated = IMG_SZ,
      .memRemaining = IMG_SZ,
#endif
      .nextFreeP = &_data[IMG_ORIGIN_IDX],
      .originP = &_data[IMG_ORIGIN_IDX]
    },
    // Audio pool
    {
#ifndef NDEBUG
      .memAllocated = AUD_SZ,
      .memRemaining = AUD_SZ,
#endif
      .nextFreeP = &_data[AUD_ORIGIN_IDX],
      .originP = &_data[AUD_ORIGIN_IDX]
    },
    // Temporary pool
    {
#ifndef NDEBUG
      .memAllocated = TMP_SZ,
      .memRemaining = TMP_SZ,
#endif
      .nextFreeP = &_data[TMP_ORIGIN_IDX],
      .originP = &_data[TMP_ORIGIN_IDX]
    }
  }
};  // static Mem _mem.

// Allocate memory in the arena at a word-aligned address.
void* memAdd ( size_t numBytes, const PoolId poolId ) {
  // Keep nextFreeP word-aligned.
  MemPool* poolP = &_mem.poolA[poolId];
  void* allocatedAddress = poolP->nextFreeP;
  poolP->nextFreeP += wordAlign_( numBytes );
#ifndef NDEBUG
  size_t memTaken = (size_t) poolP->nextFreeP - (size_t) allocatedAddress;
  poolP->memRemaining -= memTaken;
  // printf("\e[91mTaking %ld bytes. Used up %ld / %ld.\e[0m\n", memTaken, poolP->memAllocated - poolP->memRemaining, poolP->memAllocated);
  assert( memTaken <= poolP->memRemaining ); 
#endif
  return allocatedAddress;
}

void memRst( const PoolId poolId ) {
  MemPool* poolP = &_mem.poolA[poolId];
#ifndef NDEBUG
  poolP->memRemaining = poolP->memAllocated;
#endif
  poolP->nextFreeP = (U8*) poolP->originP;
}

#ifndef NDEBUG
void memReport() {
  for ( PoolId i = PERMANENT; i < N_POOLS; ++i ) {
    printf("\e[91m%s\e[0m memory pool stats:\n", _poolNames[i]);
    printf( "\tmemAllocated: %16ld\n", _mem.poolA[i].memAllocated );
    printf( "\tmemRemaining: %16ld\n", _mem.poolA[i].memRemaining);
    printf( "\tnextFreeP:   0x%08lx\n", (size_t) _mem.poolA[i].nextFreeP );
    printf( "\tmemArenaP:   0x%08lx\n", (size_t) _mem.poolA[i].originP);
  }
}
#else
#define memReport
#endif

