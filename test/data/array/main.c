#include "tau/tau.h"

TAU_MAIN()

struct ABC {
  int a, b, c;
} ABC;

TEST_F_SETUP(ABC) {
  tau->a = 1;
  tau->b = 2;
  tau->c = 3;
}

TEST_F_TEARDOWN(ABC) {
  tau->a = 0;
  tau->b = 0;
  tau->c = 0;
}

TEST_F(ABC, checkVals) {
  REQUIRE_EQ(tau->a, 1);
  REQUIRE_EQ(tau->b, 2);
  REQUIRE_EQ(tau->c, 3);
}
