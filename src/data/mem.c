#include "data/mem.h"

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

static void* _jbAlloc( const U32 numBytes ) {
	assert (numBytes);
	void* voidP = malloc(numBytes);
	assert( voidP != NULL);
  return voidP;
}

static void _jbFree(void **voidPP) {
	if (voidPP != NULL) {
		free(*voidPP);
		*voidPP = NULL;
	}
}

static void _memIni ( MemoryArena* memP, const size_t numBytes ) {
  size_t numActualBytes;
#if (__WORDSIZE == 32 )
  // The double ampersand prevents any branching logic for checking if addition is a multiple of word-size.
  numActualBytes = ( ( N_BYTES_PER_WORD - ( numBytes & 3 ) ) & 3 ) + numBytes;
#elif (__WORDSIZE == 64 )
  // The double ampersand prevents any branching logic for checking if addition is a multiple of word-size.
  numActualBytes = ( ( N_BYTES_PER_WORD - ( numBytes & 7 ) ) & 7 ) + numBytes;
#else
  static_assert( 0, "Jollybean supports only 32- and 64-bit systems.");
#endif
#ifndef NDEBUG
  printf("\e[91mAllocating %ld bytes.\e[0m\n", numActualBytes);
  memP->memAllocated = numActualBytes;
  memP->memRemaining = memP->memAllocated;
#endif
  memP->memArenaP = _jbAlloc( numActualBytes );
  memP->nextFreeP = memP->memArenaP;
}

// No point in clearing out the other stuff.
static void _memClr ( MemoryArena* memP ) {
  _jbFree( (void**)  &memP->memArenaP );
}

// Allocate memory in the arena at a word-aligned address.
static void* _memAdd ( MemoryArena* memP, size_t numBytes ) {
  // Keep nextFreeP word-aligned.
  void* allocatedAddress = memP->nextFreeP;
  U8* normalNextP = (U8*) memP->nextFreeP + numBytes;
#if (__WORDSIZE == 32 )
  memP->nextFreeP = ( ( N_BYTES_PER_WORD - ( (size_t) normalNextP & 3 ) ) & 3 ) + (U8*) normalNextP;
#elif (__WORDSIZE == 64 )
  // The double ampersand prevents any branching logic for checking if addition is a multiple of word-size.
  memP->nextFreeP = ( ( N_BYTES_PER_WORD - ( (size_t) normalNextP & 7 ) ) & 7 ) + (U8*) normalNextP;
#else
  static_assert( 0, "Jollybean supports only 32- and 64-bit systems.");
#endif
  size_t memTaken = (size_t) memP->nextFreeP - (size_t) allocatedAddress;
#ifndef NDEBUG
  memP->memRemaining -= memTaken;
  printf("\e[91mTaking %ld bytes. Used up %ld / %ld.\e[0m\n", memTaken, memP->memAllocated - memP->memRemaining, memP->memAllocated);
  assert( memTaken <= memP->memRemaining ); 
#endif
  return allocatedAddress;
}

static void _memRst( MemoryArena* memP ) {
#ifndef NDEBUG
  memP->memRemaining = memP->memAllocated;
#endif
  memP->nextFreeP = memP->memArenaP;
}

#ifndef NDEBUG
static void _memReport( const MemoryArena* memP, const char* arenaName ) {
  printf("%s memory arena stats:\n", arenaName);
  printf( "\tmemAllocated: %16ld\n", memP->memAllocated );
  printf( "\tmemRemaining: %16ld\n", memP->memRemaining );
  printf( "\tnextFreeP:   0x%08lx\n", (size_t) memP->nextFreeP );
  printf( "\tmemArenaP:   0x%08lx\n", (size_t) memP->memArenaP );
}
#else
#define memReport
#endif

// Public functions start here.
void memIni ( const size_t numBytes, const MemoryType memType ) {
  _memIni( &_arenaA[ memType ], numBytes );
}

void memClr ( const MemoryType memType ) {
  _memClr( &_arenaA[ memType ] );
}

// Allocate memory in the arena at a word-aligned address.
void* memAdd ( size_t numBytes, const MemoryType memType ) {
  return _memAdd( &_arenaA[ memType ], numBytes );
}

void memRst( const MemoryType memType ) {
  _memRst( &_arenaA[ memType ] );
}

#ifndef NDEBUG
void memReport( const MemoryType memType ) {
  const static char* words[] = { "Main", "Temp" };
  _memReport( &_arenaA[ memType ], words[ memType ] );
}
#else
#define memReport
#endif
