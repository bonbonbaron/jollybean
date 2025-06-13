#include "data/array.h"

/* TODO
 * GOALS:
 *  1. Get incrementing pointer when you allocate in arena.
 *  2. Word-align each allocation.
 *  3. Determine how much memory you need for a scene.
 *  4. Differentiate between perma-memory and temporary memory (latter allows holes).
 */

typedef struct MemoryArena {
  size_t memAllocated;
  U8* nextFreeP;
  U8* memArenaP;
} MemoryArena;

MemoryArena* maNew ( const size_t numBytes ) {
  MemoryArena* maP = jbAlloc( sizeof( MemoryArena ), 1 );
  maP->memAllocated = numBytes;
  maP->memArenaP = jbAlloc( sizeof( U8 ), numBytes );
  maP->nextFreeP = maP->memArenaP;
  return maP;
}

void maDel ( const MemoryArena** maPP ) {
  jbFree( (void**)  (*maPP )->memArenaP );
  jbFree( (void**) maPP );
}

void maAdd ( MemoryArena* maP, 
int main(int argc, char** argv) {
  MemoryArena* maP = maNew( 100 );
  maDel( (const MemoryArena**) &maP );
  return 0;
}
