#undef NDEBUG  //force this 
#include "tau.h"
#include "data/mem.h"


TAU_MAIN()
  /*
typedef struct MemPool {
#ifndef NDEBUG
  const size_t memAllocated;  // This can be done away with once we prove this out.
  size_t memRemaining;       // This can be done away with once we prove this out.
#endif
  U8* nextFreeP;
  const U8* originP;
} MemPool;
*/

static const size_t ODD_NUMBER = 101;

static void checkMemory( const PoolId poolId, const size_t expectedDifference ) {
  MemPool* memPoolP = memGetPoolP( poolId );
  CHECK_EQ( memPoolP->memAllocated - memPoolP->memRemaining, wordAlign_(expectedDifference) );
  CHECK_EQ( (size_t) memPoolP->nextFreeP - (size_t) memPoolP->originP, wordAlign_(expectedDifference) );
}

typedef struct Tau {
  MemPool* memPoolP;
  PoolId poolId;
} Tau;

TEST_F_SETUP(Tau) {
  tau->poolId = GENERAL;
  tau->memPoolP = memGetPoolP( tau->poolId );
}

TEST_F_TEARDOWN(Tau) {
  memRstAll();
}

TEST_F(Tau, memAddAligned) {
  size_t prevMemRemaining = tau->memPoolP->memRemaining;
  size_t* P = (size_t*) memAdd( sizeof(size_t), tau->poolId);
  CHECK_EQ( prevMemRemaining - tau->memPoolP->memRemaining, wordAlign_(sizeof(size_t)) );
  checkMemory( tau->poolId, sizeof(size_t) );
}

TEST_F(Tau, memAddOddNumber) {
  size_t prevMemRemaining = tau->memPoolP->memRemaining;
  U32* P = (U32*) memAdd( ODD_NUMBER, tau->poolId);
  CHECK_EQ( (size_t) P, wordAlign_((size_t) P));
  CHECK_EQ( tau->memPoolP->memAllocated - tau->memPoolP->memRemaining, wordAlign_(ODD_NUMBER) );
}

TEST_F(Tau, memRst) {
  memAdd( sizeof(U32), tau->poolId);
  memRst( tau->poolId );
  checkMemory( tau->poolId, 0 );
}

TEST_F(Tau, memRstAll) {
  // Allocate memory in every pool
  for ( PoolId poolId = PERMANENT; poolId < N_POOLS; ++poolId ) {
    memAdd( ODD_NUMBER, poolId);
  }
  memRstAll();
  for ( PoolId poolId = PERMANENT + 1; poolId < N_POOLS; ++poolId ) {
    checkMemory( poolId, 0 );
  }
  // Ensure permanent memorypersists
  checkMemory( PERMANENT, ODD_NUMBER );
}

TEST_F(Tau, memRewind) {
  // Rewind after single add.
  memAdd( ODD_NUMBER, GENERAL );
  checkMemory( GENERAL, ODD_NUMBER );
  memRewind( GENERAL );
  checkMemory( GENERAL, 0 );
  // Rewind after double-add. It should only rewind the last one.
  memAdd( ODD_NUMBER, GENERAL );
  memAdd( ODD_NUMBER, GENERAL );
  checkMemory( GENERAL, 2 * wordAlign_(ODD_NUMBER) );
  memRewind( GENERAL );
  checkMemory( GENERAL, ODD_NUMBER );
  // Rewind after no adds. Shouldn't do anything.
  checkMemory( GENERAL, ODD_NUMBER );
}

TEST_F(Tau, memReportForFullCoverage) {
  memReport();
}
