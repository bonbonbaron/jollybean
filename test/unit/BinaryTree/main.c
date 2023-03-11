#include "data.h"
#define N_SAMPLES (256)
int main(int argc, char **argv) {
  BinaryTree *btP;
  Error e = binaryTreeNew(&btP, N_SAMPLES);
  if (!e) {
    e = binaryTreeIni(btP);
  }
  // =============================
  // Plain adding into binary tree
  // =============================
  BinaryTreeElem *parentP = &btP->idxA[0];
  for (U32 i = 0, iEnd = N_SAMPLES / 3; !e && i < iEnd; ++i) {
    binaryTreeElemSetUsed(parentP);
    e = binaryTreeAddChild(btP, LEFT_CHILD, parentP);
    if (!e && i < iEnd - 1) {
      e = binaryTreeAddChild(btP, RIGHT_CHILD, parentP++);
    }
  }
  // Review contents of binary tree.
#if 0
  printf("adding-to-tree test\n");
  for (U32 i = 0; !e && i < N_SAMPLES; ++i) {
    printf("btP->idx[%3d]: parent: %3d, left: %3d, right %3d, used: %1d\n", 
        i,
        btP->idxA[i].parentIdx,
        btP->idxA[i].childIdxA[LEFT_CHILD],
        btP->idxA[i].childIdxA[RIGHT_CHILD],
        btP->idxA[i].used);
  }
#endif
  binaryTreeDel(&btP);
 
  // ===================
  // Growing binary tree
  // ===================
  e = binaryTreeNew(&btP, N_SAMPLES);
  if (!e) {
    e = binaryTreeIni(btP);
  }
  parentP = &btP->idxA[0];
  for (U32 i = 0, iEnd = N_SAMPLES / 3; !e && i < iEnd; ++i) {
    e = binaryTreeExpand(btP, LEFT_CHILD);
    binaryTreeElemSetUsed(&btP->idxA[btP->extremityA[LEFT_CHILD]]);
    if (!e && i < iEnd - 1) {
      e = binaryTreeExpand(btP, RIGHT_CHILD);
      binaryTreeElemSetUsed(&btP->idxA[btP->extremityA[RIGHT_CHILD]]);
    }
  }
  // Review contents of binary tree.
  printf("expanding-tree test\n");
  for (U32 i = 0; !e && i < 20; ++i) {
    printf("btP->idx[%3d]: parent: %3d, left: %3d, right %3d, used: %1d\n", 
        i,
        btP->idxA[i].parentIdx,
        btP->idxA[i].childIdxA[LEFT_CHILD],
        btP->idxA[i].childIdxA[RIGHT_CHILD],
        btP->idxA[i].used);
  }
  binaryTreeDel(&btP);
  return e;
}
