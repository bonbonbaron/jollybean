#include "tau/tau.h"
#include "mem.h"


TAU_MAIN()

TEST(Tau, jbAlloc) {
  U32* P = (U32*) jbAlloc( sizeof(U32), 1);
  CHECK_TRUE(P != NULL);
  jbFree((void**) &P);
  CHECK_TRUE(P == NULL);
}
