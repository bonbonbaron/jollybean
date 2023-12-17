#include "hell.h"
#include <stdio.h>
#include "fray.h"

// Velllcommme.... tooooo HELLLL!!!!!
void hellNew( HeLinkListNode** hellFP, unsigned int nElems ) {
  assert(hellFP && nElems);
  Error e = frayNew(  (void**) &(*hellFP), sizeof( HeLinkListNode ), nElems );
  assert( !e );
}

int hellIsHead (HeLinkListNode *hellF, int idx) {
  return ( hellF[idx].next == 0 || hellF[idx].tail == 0 );
}

// How are you going to distinguish between adding to head 0 and starting a new head?
unsigned int hellAdd( HeLinkListNode* hellF, int headNodeIdx, unsigned int heIdx ) {
  HeLinkListNode heNewNode = {
    .heIdx = heIdx,
    .next = 0,
    .head = headNodeIdx,
    .tail = 0
  };
  // head node of 0 means "this should be a new head"
  Error e = SUCCESS;
  // If either the user is trying to start a new list or... 
  if ( headNodeIdx < 0) {
    U32 newHeadIdx;
    e = frayAdd( (void**) hellF, (void*) &heNewNode, &newHeadIdx ); 
    hellF[newHeadIdx].head = newHeadIdx;
    assert( !e );
    return newHeadIdx;
  }
  else {
    // If you're adding to an existing linked list, update the former tail's "next" to point to new tail.
    int prevTailIdx = hellF[headNodeIdx].tail;
    e = frayAdd( (void**) hellF, (void*) &heNewNode, &hellF[headNodeIdx].tail); 
    assert( !e );
    // If the previous node belongs to our current list, append to it.
    if ( hellF[prevTailIdx].head == headNodeIdx ) {
      hellF[prevTailIdx].next = hellF[headNodeIdx].tail;
    }
    else {
      hellF[headNodeIdx].next = hellF[headNodeIdx].tail;
    }
    return headNodeIdx;
  }
}

unsigned int hellNewHead( HeLinkListNode *hellF, unsigned int heIdx ) {
  return hellAdd( hellF, -1, heIdx );
}

void dispList( HeLinkListNode* hellF, unsigned headIdx ) {
  unsigned currIdx = headIdx;
  printf("From head %d: ", headIdx);
  for (;;) {
    if ( hellF[currIdx].head == headIdx ) {
      printf("ll[%d] = %d, ", currIdx, hellF[currIdx].heIdx );
      if (hellF[currIdx].next == 0 ) {
        break;
      }
      currIdx = hellF[currIdx].next;
    }
    else {
      break;
    }
  }
  printf("\n\n");
}
