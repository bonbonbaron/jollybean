#include "eb.h"

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

// EdgeBreaker algorithm papers:
// =============================
// 1999 (A: original)                 https://faculty.cc.gatech.edu/~jarek/papers/EdgeBreaker.pdf
// 2002 (B: corner-table, A++)        https://www.cs.cmu.edu/~alla/edgebreaker_simple.pdf
// 2004 (C: arbitrary topology, B++)) https://faculty.cc.gatech.edu/~jarek/papers/EBholes.pdf
// 2007 (D: CLRGF w/o ES, B++)        https://www.scitepress.org/papers/2007/20743/20743.pdf
//    (D) elimates the need for preprocessing step in decompression and is O(n).
//
//      Jollybean follows the edgebreaker algorithm laid out in (TODO D or C?).
//
//
//      TODO
//
//      corner table
//      label boundaries
//      store boundary lengths
//      separate islands into sub-meshs structures
//      initialize all loops except the first one as 1, since you don't know which island they're on
//      make list of visited vertices in getConnectivity(). 
//      if you're dealing with the first boundary of a new island and it has a bounding loop,
//          relabel it as 2. Then when it hits another boundary that doesn't match it, 
//          relink the next and previous bouding loop nodes. Then increment the current submesh's
//          bounding loop count. TODO decide whether tracking geometry stats is a worthwhile idea,
//          and whether Euler's characteristic is useful here.
//
//
//
#define DBG_HALFEDGES (1)
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

#if DBG_HALFEDGES
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
#endif

void dispBoundaryLink( Vec3* v1, Vec3 *v2 ) {
  printf( "{ %f, %f, %f } -> { %f, %f, %f }\n", 
      v1->x,
      v1->y,
      v1->z,
      v2->x,
      v2->y,
      v2->z );
}

// ?1 How do we distinguish interior from exterior boundaries?
#define DBG_BOUNDARY_MARKER (1)
#define UNMET_BOUNDARY (0)
#define INTERNAL_BOUNDARY (1)
#define EXTERNAL_BOUNDARY (2)
// Finds boundaries in clockwise fashion ( "n" goes clockwise in holes )
void markBoundaries( Mesh* meshP ) {
  // int currBoundaryLabel = 0;
  HalfEdge *boundaryIterP, *heA, *prevBoundaryStartP = NULL;
  HeLinkListNode* heEndingAtThisVertex;
  assert( meshP && meshP->heA );
  heA = meshP->heA;   // convenience pointer
#if DBG_BOUNDARY_MARKER
  int count = 0;
#endif
  int metLabel;
  meshP->initialGate = meshP->heA;  // just in case
  forEachInArray_( HalfEdge, he ) 
    // If this is a bounding edge and hasn't been added to a loop list yet
    if ( !heP->o && heP->m == UNMET_BOUNDARY ) {
      ++meshP->nLoners;
      if ( !meshP->initialGate ) {
        meshP->initialGate = heP;
        metLabel = EXTERNAL_BOUNDARY;  // in multi-island meshes, only the first mesh's ext. B will be 2.
      }                                // Subsequent islands will know to mark first B as 2 later.
      else {
        metLabel = INTERNAL_BOUNDARY;
      }
      boundaryIterP = heP;
      // Find all the bounding edges that form a loop with this one.
      do {
        for ( heEndingAtThisVertex = boundaryIterP->s->listOfHesEndingHere; 
              heEndingAtThisVertex; 
              heEndingAtThisVertex = heEndingAtThisVertex->next ) { 
          /* This allows pinch points to traverse an exterior boundary. */ 
          if ( heEndingAtThisVertex->heP != boundaryIterP ){
            if ( !heEndingAtThisVertex->heP->o && !heEndingAtThisVertex->heP->m ) { 
#if DBG_BOUNDARY_MARKER
              ++count;
#endif
              heEndingAtThisVertex->heP->P = boundaryIterP;
              boundaryIterP->N = heEndingAtThisVertex->heP;
              boundaryIterP = heEndingAtThisVertex->heP; 
              boundaryIterP->m = metLabel;
#if DBG_BOUNDARY_MARKER
              dispBoundaryLink( 
                  &meshP->pos.u.vec3A[boundaryIterP->P->s->posIdx], 
                  &meshP->pos.u.vec3A[boundaryIterP->P->e->posIdx] );
#endif
              break;
            } 
          }
        }
      } while ( boundaryIterP != heP );
      ++meshP->nBoundingLoops;
      if ( prevBoundaryStartP ) {
        prevBoundaryStartP->nextStartingGate = heP;
      }
      assert( heP->P );
      prevBoundaryStartP = heP;
    }  // if this is an UNMET bounding edge
  endForEach_( half edge on this boundary )
#if DBG_BOUNDARY_MARKER
  printf( " tied togerther %d boudnaries\n", count );
#endif
  if ( meshP->nBoundingLoops > 1 ) {
    meshP->type = CLERSM;
  }
  else {
    meshP->type = CLERS;
  }
}

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
  Vec3 edge0to1, edge1to2;
  float directionOfEdgeNormal;
  unsigned posIdx;
  forEachInArray_( Triangle, triangle ) 
    hcP = heA + nHalfEdges;
    hnP = heA + nHalfEdges + 1;
    hpP = heA + nHalfEdges + 2;
    // Determine which way is CCW around triangle based on comparison between face normal and edge-to-edge cross prod.
    minus(triangleP->v[1].pos, triangleP->v[0].pos, &edge0to1);
    minus(triangleP->v[2].pos, triangleP->v[1].pos, &edge1to2);
    cross( &edge0to1, &edge1to2, &triangleP->normal );
    directionOfEdgeNormal = dot( &triangleP->normal, triangleP->v[0].nml );
    // float inv_magnitude = 1.0 / ( sqrt(   // TODO do i  need this for creases in parallelo predicting?

    // Determine relationships of triangle's half-edges to each other.
    hcP->s = &meshP->vstatA[ posIdx = (triangleP->v[0].pos - meshP->pos.u.vec3A) ];
    hcP->s->posIdx = posIdx;
    if ( directionOfEdgeNormal > 0.0 ) {
      hcP->e = &meshP->vstatA[ posIdx = (triangleP->v[1].pos - meshP->pos.u.vec3A) ];
      hcP->e->posIdx = posIdx;
      hcP->v = &meshP->vstatA[ posIdx = (triangleP->v[2].pos - meshP->pos.u.vec3A) ];
      hcP->v->posIdx = posIdx;
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

#if DBG_HALFEDGES
  int nLists = 0;
  for ( int i = 0; i < arrayGetNElems( heA ); ++i ) {
    // dispList( heA[i].e->listOfHesEndingHere, i );
    nLists += ( heA[i].e->listOfHesEndingHere != NULL );
  }
  printf( "%d lists, %d corners\n", nLists, arrayGetNElems( meshP->pos.u.vec3A ) );
#endif

  // Find each half-edge's opposite.
  HeLinkListNode *nodeP;
  HalfEdge* heOppositeP;
  forEachInArray_( HalfEdge, he ) 
    /* Half-edge STARTing point says, "My goal is to
       find somebody who ENDs on me. */
    // Skip this half-edge it it already has an opposite.
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
        heOppositeP = nodeP->heP;
        heOppositeP->o = heP;
        heP->o = heOppositeP;
        break;
      }
    }
  endForEach_(half-edge)

  // Genus and mesh type are ISLAND traits, not Mesh.  So: TODO
#if DBG_HALFEDGES
  printf("%d loners out of %d HEs\n", meshP->nLoners, arrayGetNElems( heA ) );
#endif
  markBoundaries( meshP );
  assert( meshP->initialGate );


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

// TODO keep track of most common letter
// Gives you the traversal order for compressing vertex attributes. 
// It also givecs you the CLERS connectivity compression codes so you only have to figure traversal out once.
// Then I can reuse this traversal-order array for normals, texels, colors, and connectivity (CLERS).
// That way, I can avoid having to reset the "met" flags after the first traversal.
#define hasRightNeighbor heP->n->o && !heP->n->o->t->m
#define hasLeftNeighbor  heP->p->o && !heP->p->o->t->m
#define goRight markAllAsSeen(heP); heP = heP->n->o
#define goLeft markAllAsSeen(heP); heP = heP->p->o
#define pushLeftNeighborToStack markAllAsSeen(heP->p->o); heP->p->o->t->onStack = 1; *(++stackP) = heP->p->o
#define popFromStack \
  markAllAsSeen(heP); \
  if ( stackP > traversalStackA ) { \
    heP = *(stackP--); \
    heP->t->onStack = 0; \
  } \
  else { \
    heP = NULL; \
  }
#define markAllAsSeen(h) \
  markTriangleAsSeen(h); markVertexAsSeen(h, s); markVertexAsSeen(h, e); markVertexAsSeen(h, v)
#define markVertexAsSeen(h, d) h->d->m = 1
#define markTriangleAsSeen(h) h->t->m = 1
#define DBG_EDGEBREAKER (1)
// EdgeBreaker: https://faculty.cc.gatech.edu/~jarek/papers/EdgeBreaker.pdf
void getConnectivity( Mesh *meshP ) {
  assert( meshP && meshP->tri.u.triA && meshP->heA && ( arrayGetNElems( meshP->tri.u.triA ) > 0) );
  // Allocate a traversal stack as a fray.
  // Make a stack pointer point to it for faster than "getLastElement()".
  HalfEdge** traversalStackA;
  Error e = arrayNew( (void**) &traversalStackA, sizeof( HalfEdge* ), arrayGetNElems( meshP->tri.u.triA ) );
  assert( !e );
  HalfEdge** stackP = traversalStackA;
  // Allocate an array of traversal order of triangles.
  // Make a pointer to it for speed too.
  e = arrayNew( (void**) &meshP->triangleTraversalOrderA, sizeof( TriangleTraversalNode ), arrayGetNElems( meshP->tri.u.triA ) );
  assert( !e );
  TriangleTraversalNode* triTravP = meshP->triangleTraversalOrderA;
  int clersHisto[5] = {0};
  // Allocate an array of traversal order of unmet vertices.
  // Make a pointer to it for speed too.
  e = arrayNew( (void**) &meshP->vertexTraversalOrderA, sizeof( VertexTraversalNode ), arrayGetNElems( meshP->pos.u.vec3A ) + 1 );
  assert( !e );
#if DBG_EDGEBREAKER
  int nTrianglesRemaining = arrayGetNElems( meshP->tri.u.triA );
  printf("mesh is %s type\n", ( meshP->type == CLERS ) ? "CLERS": "CLERSM" );
  printf("num tris: %d; num verts: %d\n", arrayGetNElems( meshP->tri.u.triA ), arrayGetNElems( meshP->pos.u.vec3A ) );
  int nIters = 0;
  Triangle* t = NULL;
#endif
  HalfEdge *heP, *hP = meshP->initialGate, *hEndP = meshP->heA + arrayGetNElems( meshP->heA );
  goto skipNewIslandLogic;
  while ( hP < hEndP ) {
    // For the first iteration, jump past the new island logic. Assume first triangle is unmet.
    if ( !hP->t->m ) {
#if DBG_EDGEBREAKER
      --nTrianglesRemaining;
      assert( nTrianglesRemaining >= 0 );
      t = hP->t;
      printf("\nnew island");  
#endif
skipNewIslandLogic:
      triTravP->newIsland = 1;  // may be useful for us in vertex attribute compression
      hP->t->g = heP = hP;
      while ( heP ) {
#if DBG_EDGEBREAKER
        printf("\niter # %d ( @ tri %05d ) -> ", ++nIters, heP->t - meshP->tri.u.triA );
#endif
        assert( !heP->t->onStack );
        triTravP->g = heP;  
        // If v is not on boundary, we're lucky; it's an easy C.
        if ( !heP->v->m ) {
          triTravP->clersChar = C;
          ++clersHisto[0];
          goRight;
        }
        // if opp vertex IS on the boundary or has been met, then we gotta figure out what kind this is.
        else if ( hasRightNeighbor ){
          if ( hasLeftNeighbor ) {
#if DBG_EDGEBREAKER
            printf( "\e[1;32mS (pushing %05d) ", heP->p->o->t - meshP->tri.u.triA );
#endif
            // TODO this is where you'll apply a condition to switch between S and M
            triTravP->clersChar = S;
            ++clersHisto[4];
            pushLeftNeighborToStack;
          }
          else {
            triTravP->clersChar = L;
            ++clersHisto[1];
          }
          goRight;
        }
        else if ( hasLeftNeighbor ) {
          triTravP->clersChar = R;
          ++clersHisto[3];
          goLeft;
        }
        else {
#if DBG_EDGEBREAKER
          printf( "\e[1;31mE (pulling %05d) ", *stackP ? ( (*stackP)->t - meshP->tri.u.triA ) : 0 );
#endif
          triTravP->clersChar = E;
          ++clersHisto[2];
          popFromStack;
        }
#if DBG_EDGEBREAKER
        if ( triTravP->clersChar != E && triTravP->clersChar != S ) {
          putchar(triTravP->clersChar);
        }
        printf( "\e[0m" );
        // Only gets checked on subsequent islands past the first one.
        if ( t ) {
          assert( t->g );  // newIsland protects against accessing a null triangle
        }
#endif
        ++triTravP;
      }  // while we're covering all the triangles we can, after starting from the initial gate
    }   // if we haven't met this half-edge's triangle yet
    if ( hP->nextStartingGate ) {
      printf("going to next bounding loop\n");
      exit(0);
      hP = hP->nextStartingGate;
    }
    else {
      // printf("simple iter\n");
      ++hP;
    }
  }  // while we're still iterating over the whole mesh's half-edges

  assert( nTrianglesRemaining == 0 );

  printf("CLERS histo: %d, %d, %d, %d, %d\n", 
      clersHisto[0],    // C
      clersHisto[1],    // L
      clersHisto[2],    // E
      clersHisto[3],    // R
      clersHisto[4] );  // S
  assert( ( triTravP - meshP->triangleTraversalOrderA ) == arrayGetNElems( meshP->tri.u.triA ) );
  arrayDel( (void**) &traversalStackA );
}

/* ============================== */
/* Predictive-Delta quantization  */
/* ============================== */
//
// For residual coding:
// ====================
//    During compression (process coordinates separately):
//    X3' = X1 + (X2 - X1)
//    residual = X3' - X3
//    (?2) How do I determine the necessary bitlength needed for quantizing?
//    Or is VLE really that great?
//    qr = quantize( residual, 10 )
//
//    Then, as each coordinate's residual array, histogram out of 1024 with a grand total.
//    Then you'll have your probabilties needed for the arithmetic encoding.
//    But compare this output with dead-simple quantization first before you commit to it.
//    And compare even that against quantization of differences (how low can they go?).
//
//    During inflation:
//    Arithmetic-decode each array of coordinate residuals
//    X3' = X1 + (X2 - X1) 
//    X3 = X3' - residual
//
//    
// Now I want to try residual. For this I need to first determine the way positions are connected to each other. In order to do that,

#define processResiduals( coord ) \
  /* Quantize coordinates */  \
  short xHistoA[1024] = { 0 }; \
  short* r##coord##A = meshP->pos.residual.pos.xA;  /* convenience pointer */ \
  const float convx = 1024.0 / fabs( meshP->pos.max.vec3.x - meshP->pos.min.vec3.x ); \
  short* q##coord##A = meshP->pos.quantized.pos.xA;  /* convenience pointer */ \
  short xmin = SHRT_MAX, xmax = SHRT_MIN; \
  for (int i = 0; i < nPositions; ++i) { \
    q##coord##A[i] = (short) ( (meshP->pos.u.vec3A[i].x - meshP->pos.min.vec3.x ) * convx); \
  } \
  /* Compute residual between parallelogram prediction and actual g.v value */ \
  short* r##coord##P = meshP->pos.residual.pos.coord##A;  /* convenience pointer */ \
  forEachInArray_( TriangleTraversalNode, trav )  \
    *r##coord##P = q##coord##A[ travP->g->v->posIdx ]   /* actual value */ \
     - (  q##coord##A[ travP->g->s->posIdx ]  \
        + q##coord##A[ travP->g->e->posIdx ]  \
        - q##coord##A[ travP->g->o->v->posIdx ] ); /* predicted */ \
    if ( *r##coord##P < coord##min ) { \
      coord##min = *r##coord##P; \
    } \
    if ( *r##coord##P > coord##max ) { \
      coord##max = *r##coord##P; \
    } \
    printf( "%d ", *r##coord##P ); \
    ++r##coord##P; \
  endForEach_( traversal node ) \
  printf( "\n\n"); \
  /* Histogram the values */ \
  forEachInArray_( short, r##coord ) \
    ++coord##HistoA[ *r##coord##P - coord##min ]; \
  endForEach_( histoing coord ) \
  /* And again, to print it out */ \
  for ( int i = 0; i < 1024; ++i ) { \
    printf("%d ", coord##HistoA[i] ); \
  } \
  printf( "\n\n");

#define DBG_POS_COMPRESSION (1)
// Parallelogram Predictor: 
//   (original) https://www.graphicsinterface.org/wp-content/uploads/gi1998-4.pdf
//   (improved) https://www.cs.unc.edu/~isenburg/papers/ia-cpmgpp-02.pdf
//   Touma and Gotsmas apply a crease angle enhancement.
// Local Quantization (for mobile)   http://cg.postech.ac.kr/papers/mesh_comp_mobile_conference.pdf
//
//
//
void compressPositions( Mesh* meshP ) {
  // Check arguments
  assert( meshP && meshP->triangleTraversalOrderA );
  assert( meshP->tri.u.triA && ( arrayGetNElems( meshP->tri.u.triA ) > 0 ) );
  assert( arrayGetNElems( meshP->triangleTraversalOrderA ) == arrayGetNElems( meshP->tri.u.triA ) );
  // Allocate
  const int nPositions = arrayGetNElems( meshP->pos.u.vec3A );
  Error e = arrayNew( (void**) &meshP->pos.quantized.pos.xA, sizeof( short ), nPositions );
  assert( !e );
  e = arrayNew( (void**) &meshP->pos.quantized.pos.yA, sizeof( short ), nPositions );
  assert( !e );
  e = arrayNew( (void**) &meshP->pos.quantized.pos.zA, sizeof( short ), nPositions );
  assert( !e );
  // Allocate arrays of residuals
  e = arrayNew( (void**) &meshP->pos.residual.pos.xA, sizeof( short ), nPositions );
  assert( !e );
  e = arrayNew( (void**) &meshP->pos.residual.pos.yA, sizeof( short ), nPositions );
  assert( !e );
  e = arrayNew( (void**) &meshP->pos.residual.pos.zA, sizeof( short ), nPositions );
  assert( !e );
  // Allocate frays of potentially generated points if necessary.
  if ( meshP->nLoners) {
    e = arrayNew( (void**) &meshP->pos.generated.pos.xA, sizeof( short ), meshP->nLoners );
    assert( !e );
    e = arrayNew( (void**) &meshP->pos.generated.pos.yA, sizeof( short ), meshP->nLoners );
    assert( !e );
    e = arrayNew( (void**) &meshP->pos.generated.pos.zA, sizeof( short ), meshP->nLoners );
    assert( !e );
  }
  // =============
  // X-Coordinates
  // =============
  TriangleTraversalNode* travA = meshP->triangleTraversalOrderA;  // convenience pointer
  short xHistoA[1024] = { 0 };
  const float convx = 1024.0 / fabs( meshP->pos.max.vec3.x - meshP->pos.min.vec3.x );
  short* qxA = meshP->pos.quantized.pos.xA;  // convenience pointer
  short xmin = SHRT_MAX, xmax = SHRT_MIN;
  short generatedCoord;
  int i;
  /* Quantize */
  for (i = 0; i < nPositions; ++i) {
    qxA[i] = (short) ( (meshP->pos.u.vec3A[i].x - meshP->pos.min.vec3.x ) * convx);
    // printf("quantx[%d] = %d, orig = %f\n", i, qxA[i], meshP->pos.u.vec3A[i].x );
    assert( qxA[i] >= 0 && qxA[i] <= 1024 );
  }
#if DBG_POS_COMPRESSION
  printf("%d elems were quantized out of %d\n", i, arrayGetNElems( qxA ) );
#endif
  short* rA = meshP->pos.residual.pos.xA;  // convenience pointer
  short* rP = rA;  // convenience pointer
  short* gxP = meshP->pos.generated.pos.xA;
  // The first three values will be raw.
  short* qxP = qxA;
  *(rP++) = *(qxP++);
  *(rP++) = *(qxP++);
  *(rP++) = *(qxP++);
  // Compute residuals by subtracting the parallelogram prediction from the actual value.
  // How about, to speed this up, we add a traversal order of vertices too?
  forEachInArray_( TriangleTraversalNode, trav ) 
    // Check to see if this gate is missing an opposite half-edge.
    // If it is, then we need to generate a  point for it on the fly.
    // The way we'll go about doing that is  just reverse-computing the
    // backwards g.o.v. 
    if ( !travP->g->o ) {
      // printf("on a loner\n");
      *(rP++) = GENERATED_COORDINATE; /* prevents 0-1024 check from bombing us */ 
      *(gxP++) = 
            qxA[ travP->g->s->posIdx ] 
          + qxA[ travP->g->e->posIdx ] 
          - qxA[ travP->g->v->posIdx ]; /* perfect prediction so residual isn't needed */
    }
    else {
      if ( travP->g->v->m ) {
        *rP = qxA[ travP->g->v->posIdx ]   /* actual value */
         - (  qxA[ travP->g->s->posIdx ] 
            + qxA[ travP->g->e->posIdx ] 
            - qxA[ travP->g->o->v->posIdx ] ); /* predicted */
        if ( *rP < xmin ) {
          xmin = *rP;
        }
        if ( *rP > xmax ) {
          xmax = *rP;
        }
        ++rP;
        travP->g->v->m = 0;  // toggle the flag so we don't have to reset them all. now 0 = we've met.
      }
    }
    // printf( "rP is %d / %d \n", rP - rA, arrayGetNElems( rA ) );
  endForEach_( traversal node )
#if DBG_POS_COMPRESSION
  printf( "\n%d residuals populated out of %d ( %d loners )\n\n", rP - rA, arrayGetNElems( rA ), meshP->nLoners);
  printf( "min = %d, max = %d\n", xmin, xmax );
#endif
  // Histogram the values
  forEachInArray_( short, r )
    if ( *rP != GENERATED_COORDINATE ) {
      ++xHistoA[ *rP -= xmin ];  // shift the residual value so index 0 is the starting point
      assert( *rP >= 0 && *rP <= 1024 );
    }
  endForEach_( histoing x )
  // And again, to print it out

  // TODO macro the above out after you validate it
#if 0
  // Y-Coordinates
  const float convY = 1024.0 / fabs( meshP->pos.max.vec3.y - meshP->pos.min.vec3.y );
  // Z-Coordinates
  const float convZ = 1024.0 / fabs( meshP->pos.max.vec3.z - meshP->pos.min.vec3.z );
#endif
  /*
   * How do we apply crease-enhanced prediction?
   * And also, if we apply crease-enhanced prediction, does that play any part in decompression?
   */
}


/* Normal vector compression
 *
 *   1) spherical compression (with even distribution toward poles): 
 *    https://people.tamu.edu/~gpetrova//SPS1.pdf
 *
 *
 *    I'm thinking of (1) in some way, shape, or form:
 *      1a) with rotating frames, if there's some trick I can use post-quantization
 *      1b) without rotating frames; just a simple difference detection between prior 2 or 3 normals
 *      
 *      Either way, I want to try arithmetic coding for the above. Pi Zero lacks vectorization anyway.
 *
 *
 */

// TODO bresenham-based approach to timed constant rotation when less than one per frame, or even more

/* TODO i'm thinking coarse lossy decompression of normals is fine, but then I want fine rotations.
        I can do this with shorts. I can perhaps even do it with all three unit vectors at the same time.
        If I have a timer, I can do it for sure.
        Goddamn I'm good.
*/

/* TODO I want to use the spherical approach, but it looks like I need to come up with a 
        fast way to rotate the unit vectors (both for input and output). 

offline:  happens neither when the game engine nor pipeline is running; neither in game engine nor pipeline memory
pipetime: done during compression, not stored in game engine memory
gametime:  done during game engine, stored in game engine memory

        The first thing that comes to mind is a combination of fixed point arithmetic and
        lookup tables. Lookups can be done for the sin and (implicitly) cos values. 

        Yes, this is important, because ALL my transformations CPU-side will be done this way.

        n.x = sin(lat)cos(lon)   ( use gametime sin LUT for both sin and cos )
        n.y = sin(lat)sin(lon)   ( use gametime sin LUT for both sin and cos )
        n.z = cos(lat)           ( use gametime sin LUT for both sin and cos ) 
              ^
              |
              +--- because lat is 0 at the unit sphere's north pole

        The above will actually be its own LUT.

        PIPETIME LATLON:
        lat = acos(z)           ( use pipetime acos LUT for quantized coordinates )
        lon = atan2(y, x)       ( use pipetime atan2 LUT for quantized coordinates )

        sine LUT (whatever granularity I tell it to)
        LATGRAN = latitude  granularity =  pi / ( latGranularity - 1 )
        HALF_LATGRAN = 0.5 * LATGRAN
        LONGRAN = longitude granularity =  2*pi / lonGranularity 
        INV_LATGRAN = 1 / LATGRAN
        INV_LONGRAN = 1 / LONGRAN
        (I can experiment with making the above unsigned shorts as well.)
        NLAT = desired number of latitudinal  increments
        NLON = desired number of longitudinal increments

        lat* = j * LATGRAN, where 0 <= j <= (NLAT - 1)
        lon* = k * LONGRAN, where 0 <= k <= (NLON - 1)  <-- not a typo since it's exclusive of 2pi

        simple algebraic rearranging of the above gets you the below:

        j = round(lat * INV_LATGRAN)
        k = round(lon * INV_LONGRAN) % (NLON) ( because each latitude has different NLON(j) )
        E (angle accuracy) is an input

        If we kept NLON uniform from top to bottom, then there'd be huge clusters of points 
            toward the poles.
        That's why we choose NLON adaptively, making j the only independent variable.

        So the total number of possible points on the unit sphere will be:
          
          sum( NLON(j) from 0 <= j <= NLAT - 1 )  <-- not a typo

        Every unit vector will map onto a rectangular patch on the unit sphere's surface, which gives:

          (lat, lon) -> (lat*, lon*)

        Lat of 0 points straight up, btw.  So for top half of sphere, 0 <= lat < pi/2.
        Hence the signage below.
        On top half of the sphere, the furthest point in each patch from its center are bottom two corners.
        Its coords are ( lat* + (pi/(2*(NLAT - 1))),  lon* + pi/(NLON(j)) ).

        You use the above to find out the minimum NLON(j) that'll satisfy the above inequality.
        But don't worry about that for now.

        Instead, you'll just calculate NLON(j) with this (done offline, btw):

        NLON(j) = ceil( 
                                pi 
          -----------------------------------------------
           ( cos(E) - cos(lat*)cos(lat* + HALF_LATGRAN) ) 
           ( ------------------------------------------ )
           (     sin(lat*)sin(lat* + HALF_LATGRAN )     )
        
        )

        You loop through and keep computing the total number of points based on (NLAT + sum(NLON(j))).

        The above loop will give you the smallest number of points possible to give the desired accuracy.

        For reference, the offline-generated N(j) LUT is only 54 bytes for E = 1.2 degrees.
        It's 25kB for E = 0.0045 degrees. Hot damn, that's awesome.

        We can generate a LUT for pre-computed unit vectors n.{x,y,z} based on lat* and lon*, 
          which will only have the total number of points (which is only a few thousand at most).
          This is especially great if we can concatenate the lat* and lon* values in single bytes.

        OFFLINE (1b)
  `     ===========
        TODO

        PIPETIME (1b)
        ===========================================================
          convert n{x,y,z} to latlon (raw floats or quantized shorts???)
          compute j
          look up NLON(j) in NLON_J_LUT
          compute k
          compute quantized lat*
          compute quantized lon*
          compute lat*lon*s residuals in clrgf order
          arithmetic-or-huffman encode residuals (which is better?")

        GAMETIME (1b)
        ===========================================================
          arithmetic-or-huffman decode residuals
          delta-correct all latlongs
          compute n.{x,y,z}  <-- offline, figure out a way to make this 16-bit quantized per coordinate
                                 as 16 bits is PLENTY in terms of rotation

*/

void compressNormals( Mesh* meshP ) {
  // Check arguments
  assert( meshP && meshP->triangleTraversalOrderA );
  assert( meshP->tri.u.triA && ( arrayGetNElems( meshP->tri.u.triA ) > 0 ) );
  assert( arrayGetNElems( meshP->triangleTraversalOrderA ) == arrayGetNElems( meshP->tri.u.triA ) );
  // Allocate a buncha crap
  /*
  const int nTriangleCorners = 3 * arrayGetNElems( meshP->tri.u.triA );
  Error e = arrayNew( (void**) &meshP->nml.quantized.nml.xA, sizeof( short ), nPositions );
  assert( !e );
  e = arrayNew( (void**) &meshP->nml.quantized.nml.yA, sizeof( short ), nPositions );
  assert( !e );
  e = arrayNew( (void**) &meshP->nml.quantized.nml.zA, sizeof( short ), nPositions );
  assert( !e );
  // Allocate arrays of residuals
  e = arrayNew( (void**) &meshP->nml.residual.nml.xA, sizeof( short ), nPositions );
  assert( !e );
  e = arrayNew( (void**) &meshP->nml.residual.nml.yA, sizeof( short ), nPositions );
  assert( !e );
  e = arrayNew( (void**) &meshP->nml.residual.nml.zA, sizeof( short ), nPositions );
  assert( !e );
  // Allocate frays of potentially generated points if necessary.
  if ( meshP->nLoners) {
    e = arrayNew( (void**) &meshP->nml.generated.nml.xA, sizeof( short ), meshP->nLoners );
    assert( !e );
    e = arrayNew( (void**) &meshP->nml.generated.nml.yA, sizeof( short ), meshP->nLoners );
    assert( !e );
    e = arrayNew( (void**) &meshP->nml.generated.nml.zA, sizeof( short ), meshP->nLoners );
    assert( !e );
  }
  */
}
