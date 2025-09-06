#include "tau.h"
#include "xGeneric.h"
#include "share.h"

TAU_MAIN()

extern XGeneric xGeneric;

typedef struct Tau {
  XGeneric *xP;
} Tau;

TEST_F_SETUP(Tau) {
  tau->xP = &xGeneric;
  shareIni( 2 );  
}

TEST_F_TEARDOWN(Tau) {
  memRst( GENERAL );
}
