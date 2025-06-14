#include "tau.h"
#include "data/bt.h"


TAU_MAIN()

typedef struct {
  BtElHeader header;
  U32 hp;
  U32 mp;
  U16 power;
  U16 stamina;
} BattleStat;

typedef struct Tau {
  U32 nElems;
  BattleStat *btP;
} Tau;

TEST_F_SETUP(Tau) {
  memIni( 10000, MAIN );
  tau->nElems = 100;
  tau->btP = NULL;

  tau->btP = memAdd( sizeof(BattleStat) * tau->nElems, MAIN );
  REQUIRE_TRUE(tau->btP != NULL);
  BattleStat 
    *statP = tau->btP,
    *statEndP = statP + arrayGetNElems(tau->btP);
  for (; statP < statEndP; ++statP) {
    for (Child child = LEFT_CHILD; child <= RIGHT_CHILD; ++child) {
      for (BattleStat *stat2P = statP + 1; stat2P < statEndP; ++stat2P) {
        if (_btIsAnOrphan(&tau->btP[stat2P - tau->btP].header)) {
          _btLinkNodes(&statP->header, &stat2P->header, statP - tau->btP, stat2P - tau->btP, child);
          break;
        }
      }
    }
  }
}

TEST_F_TEARDOWN(Tau) {
  memClr( MAIN );
}


TEST_F(Tau, btNew) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, btHasExpectedValues) {
}
TEST_F(Tau, btHasExpectedChildren) {
}

