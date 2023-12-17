#ifndef HELL_H
#define HELL_H

#include "fray.h"
#include <assert.h>

// linked list for half-edges (HE).
typedef struct {
  unsigned int heIdx, next, head, tail;
} HeLinkListNode;

void hellNew( HeLinkListNode** hellFP, unsigned int nElems );
unsigned hellAdd( HeLinkListNode* hellF, int headNodeIdx, unsigned int heIdx );
unsigned int hellNewHead( HeLinkListNode *hellF, unsigned int heIdx );
void dispList( HeLinkListNode* hellF, unsigned headIdx );

#endif
