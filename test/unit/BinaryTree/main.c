#include "data.h"
#define N_SAMPLES (256)
// SOURCE
typedef struct {
  BtElHeader header;
  U32 hp;
  U32 mp;
  U16 power;
  U16 stamina;
} BattleStat;

int main(int argc, char **argv) {
  BattleStat *btP;
  // =============================
  // Plain adding into binary tree
  // =============================
  Error e = btNew((void**) &btP, sizeof(BattleStat), N_SAMPLES);
  BattleStat 
    *statP = btP,
    *statEndP = statP + arrayGetNElems(btP);
  for (; statP < statEndP; ++statP) {
    for (Child child = LEFT_CHILD; child <= RIGHT_CHILD; ++child) {
      for (BattleStat *stat2P = statP + 1; stat2P < statEndP; ++stat2P) {
        if (btIsAnOrphan_(&stat2P->header)) {
          btLinkNodes_(btP, statP, stat2P, child);
          break;
        }
      }
    }
  }
  printf("reviewing results\n=================\n");
  for (U32 i = 0; !e && i < N_SAMPLES; ++i) {
    printf("btP->idx[%3d]: parent: %3d, left: %3d, right %3d, misc: %3d\n", 
        i,
        btP[i].header.parent,
        btP[i].header.childA[LEFT_CHILD],
        btP[i].header.childA[RIGHT_CHILD],
        btP[i].header.misc);
  }
  btDel_((void**) &btP);
  return e;
}
