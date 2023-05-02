#include "tau/tau.h"
#include "xA.h"

TAU_MAIN()

typedef struct Tau {
  Error e;
  U32 nEntities;
  XA *xP;
} Tau;

TEST_F_SETUP(Tau) {
  tau->xP = NULL;
  tau->nEntities = 100;
  tau->e = xIniSys(sAP, tau->nEntities, NULL);
  REQUIRE_EQ(tau->e, SUCCESS);
  REQUIRE_EQ(sAP->compSz, sizeof(XAComp));
  REQUIRE_EQ(frayGetNElems_((void**) sAP->cF), tau->nEntities);
  // TODO make a loop here adding sub-componets 'a' and 'd' to xAP.
  //for (;;) {
  //}
}

TEST_F_TEARDOWN(Tau) {
  xClr(&tau->xP->system);
}
//--------------------------------------------

TEST_F(Tau, xGetCompPByEntity) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xGetEntityByCompIdx) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xActivateComponentByEntity) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xDeactivateComponentByEntity) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xPauseComponentByEntity) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xUnpauseComponentByEntity) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xActivateComponentByIdx) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xDeactivateComponentByIdx) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xGetNComps) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xAddMutationMap) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xAddComp) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xAddEntityData) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xIniSys) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xClr) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xMutateComponent) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xGetEntityByVoidComponentPtr) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xQueuePause) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xQueueDeactivate) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xRun) {
  REQUIRE_TRUE(1);
}
