#include "eb.h"

// Velllcommme.... tooooo HELLLL!!!!!
void hellNew( HeLinkListNode** hellFP, unsigned int nElems ) {
  assert(hellFP && nElems);
  Error e = frayNew(  (void**) &(*hellFP), sizeof( HeLinkListNode ), nElems );
  assert( !e );
}

int hellIsHead (HeLinkListNode *hellF, int idx) {
  return ( !hellF[idx].next || !hellF[idx].tail );
}

// How are you going to distinguish between adding to head 0 and starting a new head?
HeLinkListNode* hellAdd( HeLinkListNode* hellF, HeLinkListNode* headP, HalfEdge* heP) {
  HeLinkListNode heNewNode = {
    .heP = heP,
    .next = 0,
    .head = headP,
    .tail = 0
  };
  // head node of 0 means "this should be a new head"
  Error e = SUCCESS;
  // If either the user is trying to start a new list or... 
  if ( !headP ) {
    U32 newHeadIdx;
    e = frayAdd( (void**) hellF, (void*) &heNewNode, &newHeadIdx ); 
    hellF[newHeadIdx].head = &hellF[ newHeadIdx ];
    assert( !e );
    return &hellF[newHeadIdx];
  }
  else {
    // If you're adding to an existing linked list, update the former tail's "next" to point to new tail.
    HeLinkListNode* prevTail = headP->tail;
    unsigned currTailIdx;
    e = frayAdd( (void**) hellF, (void*) &heNewNode, &currTailIdx); 
    assert( !e );
    headP->tail = &hellF[currTailIdx];
    // If the previous node belongs to our current list, append to it. (WHY TF DID I WRITE THIS?)
    if ( prevTail->head == headP ) {
      prevTail->next = headP->tail;
    }
    else {
      headP->next = headP->tail;
    }
    return headP;
  }
}

HeLinkListNode* hellNewHead( HeLinkListNode *hellF, HalfEdge* heP) {
  return hellAdd( hellF, NULL, heP );
}

void dispList( HeLinkListNode* hellF, unsigned headIdx ) {
  HeLinkListNode* currNodeP = &hellF[ headIdx ];
  printf("From head %d: ", headIdx);
  for (;; currNodeP = currNodeP->next ) {
    if ( currNodeP->head == ( hellF + headIdx ) ) {
      printf("ll[%d] = 0x%08x, ", currNodeP - hellF, (unsigned int) currNodeP->heP );
      if (currNodeP->next == NULL) {
        break;
      }
    }
    else {
      break;
    }
  }
  printf("\n\n");
}

#define forEachInArray_( type_, pointerPrefix_ ) \
  if (1) {  /* this allows redeclaring the below variables across multiple macro uses */ \
    type_ *pointerPrefix_##P = pointerPrefix_##A;\
    type_ *pointerPrefix_##EndP = pointerPrefix_##P + arrayGetNElems( pointerPrefix_##A );\
    for ( ; pointerPrefix_##P < pointerPrefix_##EndP; ++pointerPrefix_##P ) {

#define forEachInFray_( type_, pointerPrefix_ ) \
  if (1) {  /* this allows redeclaring the below variables across multiple macro uses */ \
    type_ *pointerPrefix_##P = pointerPrefix_##F;\
    type_ *pointerPrefix_##EndP = pointerPrefix_##P + frayGetFirstEmpty_( pointerPrefix_##F );\
    for ( ; pointerPrefix_##P < pointerPrefix_##EndP; ++pointerPrefix_##P ) {

#define endForEach_( loopName_ ) }}

// Outputs two arrays: Vertex and Half-Edge.
// Gives you all the half-edges and their relationships to their triangular counterparts
void getEdges( Mesh *meshP ) {
  assert( meshP && meshP->pos.u.vec3A && meshP->nml.u.vec3A && meshP->tri.u.triA );
  // Conditional assertions
  Triangle* triangleA = meshP->tri.u.triA;  // convenience pointer
  HalfEdge *heA;  // convenience pointer
  // Half-edge array
  Error e = arrayNew( (void**) &meshP->heA, sizeof(HalfEdge), 3 * arrayGetNElems(triangleA) );
  assert( !e && meshP->heA );
  heA = meshP->heA;
  // Half-edge linked list
  HeLinkListNode *hellF = NULL;
  hellNew( &hellF, 3 * arrayGetNElems( triangleA ) );
  assert( hellF );
  // Iterate through triangles
  int nHalfEdges = 0;
  HalfEdge *hcP, *hnP, *hpP;  // current, next, and previous (corresponds to CCW in triangle, starting at gate)
  Vec3 edge0to1, edge1to2, edgeNormal;
  float directionOfEdgeNormal;
  forEachInArray_( Triangle, triangle ) 
    hcP = heA + nHalfEdges;
    hnP = heA + nHalfEdges + 1;
    hpP = heA + nHalfEdges + 2;
    // Determine which way is CCW around triangle based on comparison between face normal and edge-to-edge cross prod.
    minus(triangleP->v[1].pos, triangleP->v[0].pos, &edge0to1);
    minus(triangleP->v[2].pos, triangleP->v[1].pos, &edge1to2);
    cross( &edge0to1, &edge1to2, &edgeNormal );
    directionOfEdgeNormal = dot( &edgeNormal, triangleP->v[0].nml );

    // Determine relationships of triangle's half-edges to each other.
    hcP->s = &triangleP->v[0];
    if ( directionOfEdgeNormal > 0.0 ) {
      hcP->e = &triangleP->v[1];
      hcP->v = &triangleP->v[2];
    }
    else {
      hcP->e = &triangleP->v[2];
      hcP->v = &triangleP->v[1];
    }
    hcP->n = hpP->p = hnP;
    hcP->p = hnP->n = hpP;
    hpP->n = hnP->p = hcP;
    hnP->s = hpP->v = hcP->e;
    hnP->e = hpP->s = hcP->v;
    hnP->v = hpP->e = hcP->s;
    hcP->t = hnP->t = hpP->t = triangleP;
#if 0
    printf("tri %d, hcP: s = %d, e = %d, n = %d, p = %d, v = %d\n",
        triangleP - triangleA,
        hcP->s,
        hcP->e,
        hcP->n,
        hcP->p,
        hcP->v );
    printf("tri %d, hnP: s = %d, e = %d, n = %d, p = %d, v = %d\n",
        triangleP - triangleA,
        hnP->s,
        hnP->e,
        hnP->n,
        hnP->p,
        hnP->v );
    printf("tri %d, hpP: s = %d, e = %d, n = %d, p = %d, v = %d\n\n",
        triangleP - triangleA,
        hpP->s,
        hpP->e,
        hpP->n,
        hpP->p,
        hpP->v );
#endif

    // Add this triangle's corners' terminating edges to the linked list.
    for ( int i = 0; i < 3; ++i ) {
      /* Storing the head in here over and over again may be redundant,
         but it's probably faster than asking "Has it been assigned?" 
         over and over again. */
      hcP[i].e->heTerminalHead = 
        hellAdd( 
          hellF,
          hcP[i].e->heTerminalHead,
          hcP + i);
    }

    nHalfEdges += 3;
  endForEach_(triangle)


  // Find each half-edge's opposite.
  HeLinkListNode *nodeP;
  HalfEdge* heOppositeP;
  forEachInArray_( HalfEdge, he ) 
    /* Half-edge STARTing point says, "My goal is to
       find somebody who ENDs on me. */
    if ( heP->o ) {
      continue;
    }
    nodeP = heP->s->heTerminalHead;
    /* Avoid "if this is the first elem" logic below
       by polevaulting over the incrementer the first iteration.
       This is how you avoid doing the increment before 
       the check as traditional loops do. */
    goto foundOpposite;
    do {
      nodeP = nodeP->next;
      foundOpposite:
      if ( nodeP->heP->e == heP->s && nodeP->heP->s == heP->e ) {
        heOppositeP = nodeP->heP;
        heOppositeP->o = heP;
        heP->o = heOppositeP;
#if 0
        printf("Half-edge %d goes < %d, %d >.\nIts opposite, %d, goes < %d, %d >.\n", 
          heOppositeP->o,
          heP->s, heP->e,
          heP->o,
          heOppositeP->s, heOppositeP->e );
#endif
        break;
      }
    } while ( nodeP->next );
  endForEach_(half-edge)

  // Determine whether vertices are on boundary or not.
  forEachInArray_( HalfEdge, he ) 
    heP->e->m |= ( !heP->o );  // treat this vertex prematurely as having been "met"
  endForEach_(he)

  // free hell
  frayDel( (void**) &hellF );
}


/*
 * while curr index is not zero,
 *   mark HE as "met"
 *   if he.v is a STRANGER,
 *      mark he.v as met.
 *      go to RIGHT triangle ( he.n.o ).
 *   elif you've met the RIGHT triangle,
 *      AND if you've met the LEFT triangle (you've met BOTH neighbors),
 *          pop (?1) (in recursion, this is simply a return out of this nested call)
 *      else go to the LEFT triangle
 *   elif you've met the LEFT triangle,
 *      go to the RIGHT triangle
 *   else (you've met NEITHER),
 *      push the LEFT triangle onto a to-do stack
 *      go to the RIGHT triangle
 *      
 */

// Gives you the traversal order for compressing vertex attributes. 
// It also givecs you the CLERS connectivity compression codes so you only have to figure traversal out once.
// Then I can reuse this traversal-order array for normals, texels, colors, and connectivity (CLERS).
// That way, I can avoid having to reset the "met" flags after the first traversal.
#define hasRightNeighbor heP->n->o && !heP->n->o->t->m
#define hasLeftNeighbor  heP->p->o && !heP->p->o->t->m
void getConnectivity( Mesh *meshP ) {
  assert( meshP && meshP->tri.u.triA && meshP->heA && ( arrayGetNElems( meshP->tri.u.triA ) > 0) );
  // Allocate a traversal stack as a fray.
  // Make a stack pointer point to it for faster than "getLastElement()".
  int* traversalStackF;
  Error e = frayNew( (void**) &traversalStackF, sizeof( int ), arrayGetNElems( meshP->tri.u.triA ) );
  assert( !e );
  int* traversalStackPointer = traversalStackF;
  // Allocate an array of traversal order.
  // Make a pointer to it for speed too.
  e = arrayNew( (void**) &meshP->traversalOrderA, sizeof( TraversalNode ), arrayGetNElems( meshP->tri.u.triA ) );
  assert( !e );
  TraversalNode* travNodeP = meshP->traversalOrderA;
  // Convenience pointers
  HalfEdge* hA = meshP->heA;  // convenience pointer to half-edge array
  HalfEdge* heP = hA;

  // IMPORTANT NOTE:
  // ===============
  // getEdges() doesn't address half-edges having missing neighbors.
  // It handles that indirectly by only defining half-edges on the basis of triangles in the input DAE file.
  // We only have as many half-edges as triple the triangle count.
  // So this problem, instead, will manifest indirectly as a triangle missing at least one neighbor.

  // If the first gate's oppposite vertex is not on a boundary, it's a C.
  travNodeP->heIdx = heP - hA;
  
  // If first v is not on boundary, we're lucky; it's an easy C.
  if ( !heP->v->m ) {
    travNodeP->clersChar = C;
  }
  // if opp vertex IS on the boundary or has been met, then we gotta figure out what kind this is.
  else if ( hasRightNeighbor ){
  }
  else if ( hasLeftNeighbor ){
  }
  heP->v->m = 1;
  heP->t->m = 1;
}

/* QUESTIONS
 *
 * Am I going to store the order of traversal for positions or triangles? 
 * ANd can either of those two things be used for nromals, texels, and colors?
 * The lengths for all the above vectors may differ. So we need to rely on triangle definitions.
 *
 */
