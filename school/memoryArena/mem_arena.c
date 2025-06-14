#include "mem_arena.h"
/* TODO
 * GOALS:
 *  \1. Get incrementing pointer when you allocate in arena.
 *  \2. Word-align each allocation.
 *  3. Determine how much memory you need for a scene. <-- This can easily be done by allocating a huge block and seeing how much is used.
 *  \4. Differentiate between perma-memory and temporary memory (latter allows holes).
 *  \5. Make the pool extern since there'll only ever be two of them. 
 *  6. Determine how to most easily implement the two memory allocations across all your code.
 */

// Keep the definition of memory arena here since nobody else will need it.
typedef struct MemoryArena {
#ifndef NDEBUG
  size_t memAllocated;  // This can be done away with once we prove this out.
  size_t memRemaining;       // This can be done away with once we prove this out.
#endif
  U8* nextFreeP;
  U8* memArenaP;
} MemoryArena;

static MemoryArena _arenaA[2];

static void _maIni ( MemoryArena* maP, const size_t numBytes ) {
#ifndef NDEBUG
  maP->memAllocated = numBytes;
  maP->memRemaining = maP->memAllocated;
#endif
  maP->memArenaP = jbAlloc( sizeof( U8 ), numBytes );
  maP->nextFreeP = maP->memArenaP;
}

// No point in clearing out the other stuff.
static void _maClr ( MemoryArena* maP ) {
  jbFree( (void**)  &maP->memArenaP );
}

// Allocate memory in the arena at a word-aligned address.
static void* _maAdd ( MemoryArena* maP, size_t numBytes ) {
  // Keep nextFreeP word-aligned.
  void* allocatedAddress = maP->nextFreeP;
  U8* normalNextP = (U8*) maP->nextFreeP + numBytes;
#if (__WORDSIZE == 32 )
  maP->nextFreeP = ( ( N_BYTES_PER_WORD - ( (size_t) normalNextP & 3 ) ) & 3 ) + (U8*) normalNextP;
#elif (__WORDSIZE == 64 )
  // The double ampersand prevents any branching logic for checking if addition is a multiple of word-size.
  maP->nextFreeP = ( ( N_BYTES_PER_WORD - ( (size_t) normalNextP & 7 ) ) & 7 ) + (U8*) normalNextP;
#else
  static_assert( 0, "Jollybean supports only 32- and 64-bit systems.");
#endif
  size_t memTaken = (size_t) maP->nextFreeP - (size_t) allocatedAddress;
#ifndef NDEBUG
  assert( memTaken <= maP->memRemaining ); 
  maP->memRemaining -= memTaken;
#endif
  return allocatedAddress;
}

static void _maRst( MemoryArena* maP ) {
#ifndef NDEBUG
  maP->memRemaining = maP->memAllocated;
#endif
  maP->nextFreeP = maP->memArenaP;
}

#ifndef NDEBUG
static void _maReport( const MemoryArena* maP, const char* arenaName ) {
  printf("%s memory arena stats:\n", arenaName);
  printf( "\tmemAllocated: %16d\n", maP->memAllocated );
  printf( "\tmemRemaining: %16d\n", maP->memRemaining );
  printf( "\tnextFreeP:   0x%08x\n", (size_t) maP->nextFreeP );
  printf( "\tmemArenaP:   0x%08x\n", (size_t) maP->memArenaP );
}
#else
#define maReport
#endif

// Public functions start here.
void maIni ( const MemoryType memType, const size_t numBytes ) {
  _maIni( &_arenaA[ memType ], numBytes );
}

void maClr ( const MemoryType memType ) {
  _maClr( &_arenaA[ memType ] );
}

// Allocate memory in the arena at a word-aligned address.
void* maAdd ( const MemoryType memType, size_t numBytes ) {
  return _maAdd( &_arenaA[ memType ], numBytes );
}

void maRst( const MemoryType memType ) {
  _maRst( &_arenaA[ memType ] );
}

#ifndef NDEBUG
void maReport( const MemoryType memType ) {
  const static char* words[] = { "Main", "Temp" };
  _maReport( &_arenaA[ memType ], words[ memType ] );
}
#else
#define maReport
#endif

// This is just a test. Get rid of it in the main implementation.
int main(int argc, char** argv) {
  // Main memory
  maIni( MAIN_MEMARENA, 100 );
  maReport( MAIN_MEMARENA );
  maAdd( MAIN_MEMARENA, 1 );
  maReport( MAIN_MEMARENA );
  maAdd( MAIN_MEMARENA, 8 );
  maReport( MAIN_MEMARENA );
  maRst( MAIN_MEMARENA );
  maReport( MAIN_MEMARENA );
  maClr( MAIN_MEMARENA );
  // Temporary memory
  maIni( TEMP_MEMARENA, 100 );
  maReport( TEMP_MEMARENA );
  maAdd( TEMP_MEMARENA, 1 );
  maReport( TEMP_MEMARENA );
  maAdd( TEMP_MEMARENA, 8 );
  maReport( TEMP_MEMARENA );
  maRst( TEMP_MEMARENA );
  maReport( TEMP_MEMARENA );
  maClr( TEMP_MEMARENA );
  return 0;
}
