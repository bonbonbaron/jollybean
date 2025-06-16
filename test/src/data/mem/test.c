#include "tau.h"
#include "data/mem.h"


TAU_MAIN()

TEST(Tau, jbAlloc) {
  U32* P = (U32*) memAdd( sizeof(U32), GENERAL);
  memReport();
  memRst( GENERAL );
  memReport();
}
