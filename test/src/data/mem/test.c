#include "tau/tau.h"
#include "mem.h"


TAU_MAIN()

TEST(Tau, jbAlloc) {
  U32 *P = NULL;
  Error e = jbAlloc((void**) &P, sizeof(U32), 1);
  CHECK_EQ(e, SUCCESS);
  CHECK_TRUE(P != NULL);
  jbFree((void**) &P);
  CHECK_TRUE(P == NULL);
}

TEST(Tau, jbAlloc_BadArgs) {
  U32 *P = NULL;
  Error e = jbAlloc(NULL, sizeof(U32), 1);
  CHECK_EQ(e, E_BAD_ARGS);
  CHECK_TRUE(P == NULL);
  e = jbAlloc((void**) &P, 0, 1);
  CHECK_EQ(e, E_BAD_ARGS);
  CHECK_TRUE(P == NULL);
  e = jbAlloc((void**) &P, sizeof(U32), 0);
  CHECK_EQ(e, E_BAD_ARGS);
  CHECK_TRUE(P == NULL);
}
