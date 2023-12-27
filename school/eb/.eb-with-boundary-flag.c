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

// Outputs two arrays: Vertex and Half-Edge.
// Gives you all the half-edges and their relationships to their triangular counterparts
void getEdges( Mesh *meshP ) {
  assert( meshP && meshP->pos.u.vec3A && meshP->nml.u.vec3A && meshP->tri.u.triA );
  // Conditional assertions
  Vec3* posA = meshP->pos.u.vec3A;
  Vec3* normalA = meshP->nml.u.vec3A;
  Triangle* triangleA = meshP->tri.u.triA;
  HalfEdge *heA;
  // Half-edge array
  Error e = arrayNew( (void**) &meshP->heA, sizeof(HalfEdge), 3 * arrayGetNElems(triangleA) );
  assert( !e && meshP->heA );
  heA = meshP->heA;
  // Half-edge linked list
  HeLinkListNode *hellF = NULL;
  hellNew( &hellF, 3 * arrayGetNElems( triangleA ) );
  assert( hellF );
  // Vertex array
  Vertex *vertexA;
  e = arrayNew( (void**) &vertexA, sizeof(Vertex), arrayGetNElems(posA) );
  assert( !e );
  forEachInArray_( Vertex, vertex ) 
    vertexP->heTerminalHead = -1;
  endForEach_(vertex)
  // Iterate through triangles
  int nHalfEdges = 0;
  HalfEdge *hcP, *hnP, *hpP;  // current, next, and previous (corresponds to CCW in triangle, starting at gate)
  Vec3 *faceNormalP, edge0to1, edge1to2, edgeNormal;
  float directionOfEdgeNormal;
  forEachInArray_( Triangle, triangle ) 
    hcP = heA + nHalfEdges;
    hnP = heA + nHalfEdges + 1;
    hpP = heA + nHalfEdges + 2;
    // Determine which way is CCW around triangle based on comparison between face normal and edge-to-edge cross prod.
    faceNormalP = normalA + triangleP->v[0].normalIdx;
    minus(&posA[triangleP->v[1].positionIdx], &posA[triangleP->v[0].positionIdx], &edge0to1);
    minus(&posA[triangleP->v[2].positionIdx], &posA[triangleP->v[1].positionIdx], &edge1to2);
    cross( &edge0to1, &edge1to2, &edgeNormal );
    directionOfEdgeNormal = dot( &edgeNormal, faceNormalP );

    // Determine relationships of triangle's half-edges to each other.
    hcP->s = triangleP->v[0].positionIdx;
    if ( directionOfEdgeNormal > 0.0 ) {
      hcP->e = triangleP->v[1].positionIdx;
      hcP->v = triangleP->v[2].positionIdx;
    }
    else {
      hcP->e = triangleP->v[2].positionIdx;
      hcP->v = triangleP->v[1].positionIdx;
    }
    hcP->n = hpP->p = nHalfEdges + 1;
    hcP->p = hnP->n = nHalfEdges + 2;
    hpP->n = hnP->p = nHalfEdges;
    hnP->s = hpP->v = hcP->e;
    hnP->e = hpP->s = hcP->v;
    hnP->v = hpP->e = hcP->s;
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
      Vertex* vertexP = &vertexA[hcP[i].e];
      /* Storing the head in here over and over again may be redundant,
         but it's probably faster than asking "Has it been assigned?" 
         over and over again. */
      vertexP->heTerminalHead = 
        hellAdd( 
          hellF,
          vertexP->heTerminalHead,
          nHalfEdges + i);
    }

    nHalfEdges += 3;
  endForEach_(triangle)


  // Find each half-edge's opposite.
  HeLinkListNode *nodeP;
  HalfEdge* heOppositeP;
  forEachInArray_( HalfEdge, he ) 
    /* Half-edge STARTing point says, "My goal is to
       find somebody who ENDs on me. */
    if ( heP->hasOpposite ) {
      continue;
    }
    nodeP = &hellF[vertexA[heP->s].heTerminalHead];
    /* Avoid "if this is the first elem" logic below
       by polevaulting over the incrementer the first iteration.
       This is how you avoid doing the increment before 
       the check as traditional loops do. */
    goto foundOpposite;
    do {
      nodeP = &hellF[nodeP->next];
      foundOpposite:
      if ( heA[nodeP->heIdx].e == heP->s && heA[nodeP->heIdx].s == heP->e ) {
        heOppositeP = &heA[nodeP->heIdx];
        heOppositeP->o = heP - heA;
        heOppositeP->hasOpposite = 1;
        heP->o = nodeP->heIdx;
        heP->hasOpposite = 1;
#if 0
        printf("Half-edge %d goes < %d, %d >.\nIts opposite, %d, goes < %d, %d >.\n", 
          heOppositeP->o,
          heP->s, heP->e,
          heP->o,
          heOppositeP->s, heOppositeP->e );
#endif
        break;
      }
    } while ( nodeP->next != 0 );
  endForEach_(half-edge)

  // Determine whether vertices are on boundary or not.
  forEachInArray_( HalfEdge, he ) 
    vertexA[heP->e].b |= ( !heP->hasOpposite );
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
void getConnectivity( Mesh *meshP ) {
  assert( meshP && meshP->vertexA && meshP->heA && ( arrayGetNElems( meshP->vertexA ) > 0) );
  // Allocate a traversal stack as a fray.
  // Make a stack pointer point to it for faster than "getLastElement()".
  int* traversalStackF;
  Error e = frayNew( (void**) &traversalStackF, sizeof( int ), arrayGetNElems( meshP->vertexA ) );
  assert( !e );
  int* traversalStackPointer = traversalStackF;;
  // Allocate an array of traversal order.
  // Make a pointer to it for speed too.
  e = arrayNew( (void**) &meshP->traversalOrderA, sizeof( TraversalNode ), arrayGetNElems( meshP->vertexA ) );
  assert( !e );
  TraversalNode* travNodeP = meshP->traversalOrderA;
  // Convenience pointers
  Vertex* vA = meshP->vertexA;
  Vertex* hA = meshP->heA;
 
  // Start at the first HE (index 0), then start at its opposite triangle.
  HalfEdge *heP = hA;
  Vertex *vP;

  // IMPORTANT NOTE:
  // ===============
  // getEdges() doesn't address half-edges having missing neighbors.
  // It handles that indirectly by only defining half-edges on the basis of triangles in the input DAE file.
  // We only have as many half-edges as triple the triangle count.
  // So this problem, instead, will manifest indirectly as a triangle missing at least one neighbor.
  // If opposite vertex is only connected to gate counterclockwise

  // If the first gate's oppposite vertex is not on a boundary, it's a C.
  travNodeP->heIdx = heP - hA;
  vP = vA[ hA[ 0 ].v ];
  vP->m = 1;
  // If first v is not on boundary, we're lucky; it's an easy C.
  if ( !vP->b ) {
    travNodeP->clersChar = C;
  }
  // if opp vertex IS on the boundary, then we gotta figure out what kind this is.
  else if ( hA[ heP->n ].
}

