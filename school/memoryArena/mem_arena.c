#include "data/mem.h"
/* TODO
 * GOALS:
 *  \1. Get incrementing pointer when you allocate in arena.
 *  \2. Word-align each allocation.
 *  3. Determine how much memory you need for a scene. <-- This can easily be done by allocating a huge block and seeing how much is used.
 *  4. Differentiate between perma-memory and temporary memory (latter allows holes).
 *  5. Make teh pool extern since 
 */

typedef struct MemoryArena {
  size_t memAllocated;  // This can be done away with once we prove this out.
  size_t memRemaining;       // This can be done away with once we prove this out.
  U8* nextFreeP;
  U8* memArenaP;
} MemoryArena;

MemoryArena* maNew ( const size_t numBytes ) {
  MemoryArena* maP = jbAlloc( sizeof( MemoryArena ), 1 );
  maP->memAllocated = numBytes;
  maP->memRemaining = maP->memAllocated;
  maP->memArenaP = jbAlloc( sizeof( U8 ), numBytes );
  maP->nextFreeP = maP->memArenaP;
  return maP;
}

void maDel ( const MemoryArena** maPP ) {
  jbFree( (void**)  (*maPP )->memArenaP );
  jbFree( (void**) maPP );
}

void* maAdd ( MemoryArena* maP, size_t numBytes ) {
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
  assert( memTaken <= maP->memRemaining ); 
  maP->memRemaining -= memTaken;
  return allocatedAddress;
}

void maReset( MemoryArena* maP ) {
  maP->memRemaining = maP->memAllocated;
  maP->nextFreeP = maP->memArenaP;
}

#ifndef NDEBUG
void maReport( const MemoryArena* maP ) {
  printf("Memory arena stats:\n");
  printf( "\tmemAllocated: %16d\n", maP->memAllocated );
  printf( "\tmemRemaining: %16d\n", maP->memRemaining );
  printf( "\tnextFreeP:   0x%08x\n", (size_t) maP->nextFreeP );
  printf( "\tmemArenaP:   0x%08x\n", (size_t) maP->memArenaP );
}
#else
#define maReport
#endif

int main(int argc, char** argv) {
  MemoryArena* maP = maNew( 100 );
  maReport( maP );
  maAdd( maP, 1 );
  maReport( maP );
  maAdd( maP, 8 );
  maReport( maP );
  maDel( (const MemoryArena**) &maP );
  return 0;
}
