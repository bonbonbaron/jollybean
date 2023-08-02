#include "tau/tau.h"
#include "xA.h"

TAU_MAIN()

# define requireSuccess_ REQUIRE_EQ(tau->e, SUCCESS)
typedef struct Tau {
  Error e;
  U32 nEntities;
  XA *xP;
} Tau;

TEST_F_SETUP(Tau) {
  tau->xP = NULL;
  tau->nEntities = 100;
  tau->e = xIniSys(sAP, tau->nEntities, NULL);
  requireSuccess_;
  REQUIRE_EQ(sAP->compSz, sizeof(XAComp));
  REQUIRE_EQ(frayGetNElems_((void**) sAP->cF), tau->nEntities);
  // Give all entities subcomponents.
  // Error    xIniSubcomp(System *sP, const Entity entity, const void *cmpP);
  for (Entity entity = 1; entity <= tau->nEntities; ++entity) {
    XAComp xAComp = {0};
    int a = entity * 1;
    short b = entity * 2;
    char c = entity * 3;
    double d = entity * 4.0;
    // Error    xAddEntityData(System *sP, Entity entity, Key compType, void *entityDataP);
    tau->e = xAddEntityData(&tau->xP->system, entity, 0x40 & XA_TYPE, 
    tau->e = xAddComp(&tau->xP->system, entity, &xAComp);
    requireSuccess_;
    //tau->xP->
    // TODO you're gonna have to make a loop making a map of mutations for b and c as well.
  }
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
