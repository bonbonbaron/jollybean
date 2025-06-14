#ifndef MEM_H
#define MEM_H

#include "data/common.h"

typedef enum { MAIN, TEMP } MemoryType;

// Public functions start here.
void memIni ( const size_t numBytes, const MemoryType memType);

void memClr ( const MemoryType memType );

// Allocate memory in the arena at a word-aligned address.
void* memAdd ( size_t numBytes, const MemoryType memType );

void memRst( const MemoryType memType );

#ifndef NDEBUG
void memReport( const MemoryType memType );
#else
#define memReport
#endif

#endif  // #ifndef MEM_H
