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

// EdgeBreaker algorithm paper:
//      (original)   https://faculty.cc.gatech.edu/~jarek/papers/EdgeBreaker.pdf
//      (simplified) https://www.cs.cmu.edu/~alla/edgebreaker_simple.pdf
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

#define findNextBoundaryEdge( currEdge ) \
  /* Find the next edge ending at this starting vertex. */  \
  for ( heEndingAtThisVertex = currEdge->s->listOfHesEndingHere; \
        heEndingAtThisVertex; \
        heEndingAtThisVertex = heEndingAtThisVertex->next ) { \
    /* This allows pinch points to traverse an exterior boundary. */ \
    if ( !heEndingAtThisVertex->heP->o && !heEndingAtThisVertex->heP->m ) { \
      printf("found unmet oppososite 0x%08x\n", (unsigned) heEndingAtThisVertex); \
      previousBoundaryP = currEdge; \
      boundaryIterP = heEndingAtThisVertex->heP; \
      previousBoundaryP->N = boundaryIterP; \
      boundaryIterP->P = previousBoundaryP; \
      break; \
    } \
  }

// ?1 How do we distinguish interior from exterior boundaries?
#define COME_BACK_AFTER_ALL_BOUNDARIES_LABELED (-1)
// Finds boundaries in clockwise fashion ( "n" goes clockwise in holes )
void markBoundaries( Mesh* meshP ) {
  // int currBoundaryLabel = 0;
  HalfEdge *boundaryIterP, *previousBoundaryP, *heA;
  HeLinkListNode* heEndingAtThisVertex;
  assert( meshP && meshP->heA );
  heA = meshP->heA;   // convenience pointer
  int count = 0, ccount = 0;
  int foundUnmet;
  forEachInArray_( HalfEdge, he ) 
    // If this is a bounding edge and hasn't been added to a loop list yet
    if ( !heP->o && !heP->m ) {
      // printf("0heP N = 0x%08x, P = 0x%08x\n", (unsigned) heP->N, (unsigned) heP->P );
      boundaryIterP = NULL;
      // findNextBoundaryEdge( heP );
      for ( heEndingAtThisVertex = heP->s->listOfHesEndingHere; 
            heEndingAtThisVertex; 
            heEndingAtThisVertex = heEndingAtThisVertex->next ) { 
        /* This allows pinch points to traverse an exterior boundary. */ 
        if ( heEndingAtThisVertex->heP != heP ){
          if ( !heEndingAtThisVertex->heP->o && !heEndingAtThisVertex->heP->m ) { 
            ++count;
            // printf("heP found unmet oppososite 0x%08x\n", (unsigned) heEndingAtThisVertex->heP); 
            previousBoundaryP = heP; 
            boundaryIterP = heEndingAtThisVertex->heP; 
            previousBoundaryP->N = boundaryIterP; 
            boundaryIterP->P = previousBoundaryP; 
            boundaryIterP->m = 1;
            break;
          } 
        }
      }
      // Loop around the boundary
      if ( boundaryIterP ) {
        foundUnmet = 0;
        ccount = 1;
        while (boundaryIterP != heP) {
          for ( heEndingAtThisVertex = boundaryIterP->s->listOfHesEndingHere; 
              heEndingAtThisVertex; 
              heEndingAtThisVertex = heEndingAtThisVertex->next ) { 
            /* This allows pinch points to traverse an exterior boundary. */ 
            if ( !heEndingAtThisVertex->heP->o && !heEndingAtThisVertex->heP->m ) { 
              foundUnmet = 1;
              ++count;
              // printf("BOUNDARYITERP found unmet oppososite 0x%08x\n", (unsigned) heEndingAtThisVertex->heP); 
              previousBoundaryP = boundaryIterP; 
              boundaryIterP = heEndingAtThisVertex->heP; 
              previousBoundaryP->N = boundaryIterP; 
              boundaryIterP->P = previousBoundaryP; 
              boundaryIterP->m = 1;
              ++ccount;
              break;
            } 
          }
          if ( !foundUnmet ) {
            assert( 0 );
            break;
          }
          // findNextBoundaryEdge( boundaryIterP );
        }
        printf("\ttied %d edges together for current boudnary\n", ccount);
      }
      heP->m = 1;
      // printf("fheP N = 0x%08x, P = 0x%08x\n", (unsigned) heP->N, (unsigned) heP->P );
    }  // if this is a bounding edge
  endForEach_( half edge on this boundary )
  printf( " tied togerther %d boudnaries\n", count );
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

  // Determine whether vertices are on boundary or not.
  forEachInArray_( HalfEdge, he ) 
    meshP->nLoners += !heP->o; // count all the loners
    heP->e->m |= ( !heP->o );  // treat this vertex prematurely as having been "met"
  endForEach_(he)

  // Genus and mesh type are ISLAND traits, not Mesh.  So: TODO
#if DBG_HALFEDGES
  printf("%d loners out of %d HEs\n", meshP->nLoners, arrayGetNElems( heA ) );
#endif
  if ( meshP->nLoners ) {
    markBoundaries( meshP );
    if ( meshP->genus > 0 ) {
      meshP->meshType = CLERSM;
    }
    else {
      meshP->meshType = CLERS;
    }
  }
  else {
    meshP->meshType = CLERS;
  }


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
#define DBG_EDGEBREAKER (0)
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
  VertexTraversalNode* vertTravP = meshP->vertexTraversalOrderA;
  // For parallelogram prediction, it's important to start out with a seed of three vertices.
  if ( meshP->heA[0].o ) {

  }
  else { 
  }
#if DBG_EDGEBREAKER
  printf("num tris: %d; num verts: %d\n", arrayGetNElems( meshP->tri.u.triA ), arrayGetNElems( meshP->pos.u.vec3A ) );
  int nIters = 0;
  Triangle* t;
#endif
  for ( HalfEdge *heP, *hP = meshP->heA, *hEndP = hP + arrayGetNElems( meshP->heA ); 
        hP < hEndP; ++hP ) {
    if ( !hP->t->m ) {
      hP->t->g = heP = hP;
      triTravP->newIsland = 1;  // may be useful for us in vertex attribute compression
#if DBG_EDGEBREAKER
      t = hP->t;
      printf("\nnew island");  
#endif
      // I was expecting this to plow through all the triangles of a distinct mesh. E wrong?
      while ( heP ) {
#if DBG_EDGEBREAKER
        printf("\niter # % 2d ( @ tri %05d ) -> ", ++nIters, heP->t - meshP->tri.u.triA );
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
        // TODO There's some nuance i'm missing here. Supposed to find E one step sooner.
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
        assert( t->g );
#endif
        ++triTravP;  // TODO some allocation bug here, maybe because one too many
      }
    }
  }

  printf("CLERS histo: %d, %d, %d, %d, %d\n", 
      clersHisto[0],    // C
      clersHisto[1],    // L
      clersHisto[2],    // E
      clersHisto[3],    // R
      clersHisto[4] );  // S
  printf("Apparently initial bounding loop has %d edges\n",
      3 * clersHisto[2]
      + clersHisto[1]
      + clersHisto[3]
      - clersHisto[0]
      - clersHisto[4]
      );
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
//    TODO study across- and within- parallelogram predictors (isenburg, p3)
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
#if 1
  short xHistoA[2048] = { 0 };
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
  int j =0, total =0;
  forEachInArray_( short, r )
    if ( *rP != GENERATED_COORDINATE ) {
      ++xHistoA[ *rP -= xmin ];  // shift the residual value so index 0 is the starting point
      printf( "%d at %d\n", ++j, *rP);
      assert( *rP >= 0 && *rP <= 1024 );
    }
  endForEach_( histoing x )
  // And again, to print it out
#if DBG_POS_COMPRESSION
  for ( int i = 0; i < 2048; ++i ) {
    total += xHistoA[i];
    printf("%d ", xHistoA[i] );
  }
  printf("histo total: %d ", total );
#endif
#else
  processResiduals( x );
#endif
  // processResiduals( y );
  // processResiduals( z );

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
