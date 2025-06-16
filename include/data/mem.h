#ifndef MEM_H
#define MEM_H

#include "data/common.h"

typedef enum PoolId { PERMANENT, GENERAL, TEXT, IMAGE, AUDIO, TEMPORARY, N_POOLS } PoolId;

// Public functions start here.
void memClr ( const PoolId poolId );

// Allocate memory in the arena at a word-aligned address.
void* memAdd ( const size_t numBytes, const PoolId poolId );

void memRst( const PoolId poolId );

#ifndef NDEBUG
void memReport();
#else
#define memReport
#endif

#endif  // #ifndef MEM_H
