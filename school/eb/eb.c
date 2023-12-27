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
    hellF[newHeadIdx].tail = &hellF[ newHeadIdx ];
    assert( !e );
    return &hellF[newHeadIdx];
  }
  else {
    // If you're adding to an existing linked list, update the former tail's "next" to point to new tail.
    unsigned newTailIdx;
    HeLinkListNode* origTailP = headP->tail;
    e = frayAdd( (void**) hellF, (void*) &heNewNode, &newTailIdx); 
    assert( !e );
    // Head's tail and orig tail's next are new tail
    origTailP->next = headP->tail = &hellF[newTailIdx];
    headP->tail->head = headP;
    return headP;
  }
}

HeLinkListNode* hellNewHead( HeLinkListNode *hellF, HalfEdge* heP) {
  return hellAdd( hellF, NULL, heP );
}

void dispList( HeLinkListNode* hellF, unsigned headIdx ) {
  if ( hellF[headIdx].head == &hellF[headIdx] ) {
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
  // Vertex-met array
  e = arrayNew( (void**) &meshP->vstatA, sizeof(VertexStatus), arrayGetNElems( meshP->pos.u.vec3A ) );
  assert( !e && meshP->vstatA );
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
    hcP->s = &meshP->vstatA[ triangleP->v[0].pos - meshP->pos.u.vec3A ];
    if ( directionOfEdgeNormal > 0.0 ) {
      hcP->e = &meshP->vstatA[ triangleP->v[1].pos - meshP->pos.u.vec3A ];
      hcP->v = &meshP->vstatA[ triangleP->v[2].pos - meshP->pos.u.vec3A ];
    }
    else {
      hcP->e = &meshP->vstatA[ triangleP->v[2].pos - meshP->pos.u.vec3A ];
      hcP->v = &meshP->vstatA[ triangleP->v[1].pos - meshP->pos.u.vec3A ];
    }
    // Each position needs a "met" and "HE terminal head"
    hcP->n = hpP->p = hnP;
    hcP->p = hnP->n = hpP;
    hpP->n = hnP->p = hcP;
    hnP->s = hpP->v = hcP->e;
    hnP->e = hpP->s = hcP->v;
    hnP->v = hpP->e = hcP->s;
    hcP->t = hnP->t = hpP->t = triangleP;

    // Add this triangle's corners' terminating edges to the linked list.
    for ( int i = 0; i < 3; ++i ) {
      /* Storing the head in here over and over again may be redundant,
         but it's probably faster than asking "Has it been assigned?" 
         over and over again. */
      hcP[i].e->listOfHesEndingHere = 
        hellAdd( 
          hellF,
          hcP[i].e->listOfHesEndingHere,
          hcP + i);
    }

    nHalfEdges += 3;
  endForEach_(triangle)

  int nCorners, nLists;
  for ( int i = 0; i < arrayGetNElems( heA ); ++i ) {
    //dispList( heA[i].e->listOfHesEndingHere, i );
    nLists += ( heA[i].e->listOfHesEndingHere != NULL );
  }
  nCorners = arrayGetNElems( meshP->pos.u.vec3A );

  // printf( "%d lists, %d corners\n", nLists, nCorners );

  // Find each half-edge's opposite.
  HeLinkListNode *nodeP;
  HalfEdge* heOppositeP;
  int nOpps = 0, nLoners = 0;
  forEachInArray_( HalfEdge, he ) 
    /* Half-edge STARTing point says, "My goal is to
       find somebody who ENDs on me. */
    if ( heP->o ) {
      continue;
    }
    nodeP = heP->s->listOfHesEndingHere;
    /* Avoid "if this is the first elem" logic below
       by polevaulting over the incrementer the first iteration.
       This is how you avoid doing the increment before 
       the check as traditional loops do. */
    goto foundOpposite;
    while ( nodeP->next ) {
      nodeP = nodeP->next;
      foundOpposite:
      // if ( nodeP->heP->e == heP->s && nodeP->heP->s == heP->e ) {
      if ( nodeP->heP->s == heP->e ) {
        ++nOpps;
        heOppositeP = nodeP->heP;
        heOppositeP->o = heP;
        heP->o = heOppositeP;
        break;
      }
    }
  endForEach_(half-edge)

  // Determine whether vertices are on boundary or not.
  forEachInArray_( HalfEdge, he ) 
    if ( !heP->o ) {
      ++nLoners;
    }
    heP->e->m |= ( !heP->o );  // treat this vertex prematurely as having been "met"
  endForEach_(he)

  // printf("%d twins + %d loners = %d HEs\n", nOpps, nLoners, arrayGetNElems( heA ) );

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
#define goRight markAllAsSeen; heP = heP->n->o
#define goLeft markAllAsSeen; heP = heP->p->o
#define pushLeftNeighborToStack *(stackP++) = heP->p->o
#define popFromStack if (*stackP) heP = *(--stackP); else heP = NULL
#define markAllAsSeen markVertexAsSeen(s); markVertexAsSeen(e); markVertexAsSeen(v)
#define markVertexAsSeen(d) heP->d->m = 1
void getConnectivity( Mesh *meshP ) {
  assert( meshP && meshP->tri.u.triA && meshP->heA && ( arrayGetNElems( meshP->tri.u.triA ) > 0) );
  // Allocate a traversal stack as a fray.
  // Make a stack pointer point to it for faster than "getLastElement()".
  HalfEdge** traversalStackA;
  Error e = arrayNew( (void**) &traversalStackA, sizeof( HalfEdge* ), arrayGetNElems( meshP->tri.u.triA ) );
  assert( !e );
  HalfEdge** stackP = traversalStackA;
  // Allocate an array of traversal order.
  // Make a pointer to it for speed too.
  e = arrayNew( (void**) &meshP->traversalOrderA, sizeof( TraversalNode ), arrayGetNElems( meshP->tri.u.triA ) );
  assert( !e );
  TraversalNode* travNodeP = meshP->traversalOrderA;
  // printf("num tris: %d\n", arrayGetNElems( meshP->tri.u.triA ) );
  int nIters = 0;
  for ( HalfEdge *heP, *hP = meshP->heA, *hEndP = hP + arrayGetNElems( meshP->heA ); 
        hP < hEndP; ++hP ) {
    if ( !hP->t->m ) {
      heP = hP;
      while ( heP ) {
        // printf("\niter # %02d ( @ 0x%08x ) -> ", ++nIters, (unsigned) heP->t );
        heP->t->m = 1;
        travNodeP->t = heP->t;  
        // If v is not on boundary, we're lucky; it's an easy C.
        if ( !heP->v->m ) {
          travNodeP->clersChar = C;
          // putchar(travNodeP->clersChar);
          goRight;
        }
        // if opp vertex IS on the boundary or has been met, then we gotta figure out what kind this is.
        else if ( hasRightNeighbor ){
          if ( hasLeftNeighbor ) {
            travNodeP->clersChar = S;
            // putchar(travNodeP->clersChar);
            pushLeftNeighborToStack;
          }
          else {
            travNodeP->clersChar = L;
            // putchar(travNodeP->clersChar);
          }
          goRight;
        }
        // TODO There's some nuance i'm missing here. Supposed to find E one step sooner.
        else if ( hasLeftNeighbor ) {
          travNodeP->clersChar = R;
          // putchar(travNodeP->clersChar);
          goLeft;
        }
        else {
          travNodeP->clersChar = E;
          // putchar(travNodeP->clersChar);
          popFromStack;
        }
        ++travNodeP;  // TODO some allocation bug here, maybe because one too many
      }
    }
  }
  arrayDel( (void**) &traversalStackA );
}
