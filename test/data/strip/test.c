#include "tau/tau.h"
#include "strip.h"

// TODO rename strip util to not clash
// TODO include strip util to pack stuff to make this easier and portable

TAU_MAIN()

typedef struct Tau {
  // Put stuff to instantiate here. 
  // This is a particularly good place for pointers since double-poitners are needed to allocate them.
} Tau;

TEST_F_SETUP(Tau) {
  // Instantiate stuff in the Tau struct here.
}

TEST_F_TEARDOWN(Tau) {
  // Free stuff in the tau struct here.
}


TEST_F(Tau, stripClr) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, sdInflate) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, sdUnpack) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, sdAssemble) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, stripIni) {
  REQUIRE_TRUE(1);
}
