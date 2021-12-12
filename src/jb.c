#include "jb.h"

#if 0
static System *sPA[] = {
	&sRender
}; 
#endif

//typedef NodeStat(*NodeCB)(struct Node *rootP, struct Node *currNodeP, U8 event, Map *bbP);  
NodeFunc_(cb1) {
  printf("node 1\n");
  return FAILED;
}

NodeFunc_(cb2) {
  printf("node 2\n");
  return COMPLETE;
}

NodeFunc_(cb3) {
  printf("node 3\n");
  return COMPLETE;
}

NodeFunc_(cb4) {
  printf("node 4\n");
  return COMPLETE;
}

CbNode_(a, cb1);
CbNode_(b, cb2);
CbNode_(c, cb3);
CbNode_(d, cb4);
SelectorNode_(seq, &a, &b, &c, &d);

int main() {
#if 0
	Error e = xIni(sPA, sizeof(sPA) / sizeof(sPA[0]), &grp1);
	while (!e)
		sRun(&sParent);
	return e;
#else
  Node *nodeP;
  Error e = btNew(&seq, 5, &nodeP);
  if (!e) {
    NodeStat stat = btRun(nodeP, 0, NULL);
    printf("stat is %d\n", stat);
  }
  else 
    printf("btNew failed.\n");
  return e;
#endif
}
