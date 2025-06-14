#include "data/mem.h"

typedef enum { MAIN_MEMARENA, TEMP_MEMARENA } MemoryType;

// Public functions start here.
void maIni ( const MemoryType memType, const size_t numBytes );

void maClr ( const MemoryType memType );

// Allocate memory in the arena at a word-aligned address.
void* maAdd ( const MemoryType memType, size_t numBytes );

void maRst( const MemoryType memType );

#ifndef NDEBUG
void maReport( const MemoryType memType );
#else
#define maReport
#endif

