#include "tau/tau.h"
#include "xMotion.h"


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


TEST_F(Tau, xMotionIniSys) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xMotionIniSubcomp) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xMotionGetShare) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xMotionPostprocessComps) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xMotionPostMutate) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xMotionProcessMessage) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xMotionRun) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xMotionClr) {
  REQUIRE_TRUE(1);
}
