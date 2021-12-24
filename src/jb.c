#include "jb.h"

static System *sPA[] = {
	&sRender
}; 

NodeCb_(cb1) {
  printf("node 1\n");
  U32 *hp = (U32*) mapGet(bbP->agentBbP, 1);
  U32 *mp = (U32*) mapGet(bbP->agentBbP, 2);
  printf("hp is %d; mp is %d.\n", *hp, *mp);
  return COMPLETE;
}

NodeCb_(cb2) {
  printf("node 2\n");
  return COMPLETE;
}

NodeCb_(cb3) {
  printf("node 3\n");
  return COMPLETE;
}

NodeCb_(cb4) {
  printf("node 4\n");
  return COMPLETE;
}

NodeCb_(cb5) {
  printf("node 5\n");
  return COMPLETE;
}

LeafNode_(a, cb1);
LeafNode_(b, cb2);
LeafNode_(c, cb3);
LeafNode_(d, cb4);
SelectorNode_(sel, &a, &b, &c, &d);
LeafNode_(e, cb5);
SequenceNode_(root, &sel, &e);

U32 hp = 200;
U32 mp = 300;
#define BBSeed_(name_, ...) \
  KeyValPair name_##BbKvPair[] = {__VA_ARGS__};\
  BBSeed name_##BBSeed = {\
    .nKeyValPairs = sizeof(name_##BbKvPair) / sizeof(KeyValPair), \
    .keyValPairA = name_##BbKvPair\
  };

BBSeed_(mb, {1, &hp}, {2, &mp});

int main() {
  BTree *treeP;
  Blackboard *bbP;
  Error e = btNew(&root, 0, &treeP);

  if (!e)
    e = bbNew(&bbP, treeP->rootP, &mbBBSeed);

  if (!e)
    btRun(treeP, bbP);

  if (!e)
    e = xIni(sPA, sizeof(sPA) / sizeof(sPA[0]), &grp1);

	while (!e) {
		sRun(&sParent);
  }

  btDel(&treeP);
  bbDel(&bbP);
  return e;
}


// TODO:
//  \1) test BB by making a tree that uses it.
//  2) test sending a system a message
