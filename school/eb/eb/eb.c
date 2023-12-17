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
void blah( Vec3 *posA, Vec3 *normalA, Triangle *triangleA ) {
  assert( posA && normalA && triangleA );
  // Half-edge array
  HalfEdge* heA;
  Error e = arrayNew( (void**) &heA, sizeof(HalfEdge), 6 * arrayGetNElems(triangleA) );
  assert( !e );
  // Half-edge linked list
  HeLinkListNode *hellF = NULL;
  hellNew( &hellF, 3 * arrayGetNElems( triangleA ) );
  assert( hellF );
  // Vertex array
  Vertex *vertexA;
  forEachInArray_( Vertex, vertex ) 
    vertexP->heTerminalHead = -1;
  endForEach_(vertex)
  e = arrayNew( (void**) &vertexA, sizeof(Vertex), arrayGetNElems(posA) );
  assert( !e );
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
    nodeP = &hellF[vertexA[heP->s].heTerminalHead];
    /* Avoid "if this is the first elem" logic below
       by polevaulting over the incrementer.
       This is how you avoid doing the increment be-
       fore the check as traditional loops do. */
    goto foundOpposite;
    do {
      nodeP = &hellF[nodeP->next];
      foundOpposite:
      if ( heA[nodeP->heIdx].e == heP->s ) {
        heOppositeP = &heA[nodeP->heIdx];
        heOppositeP->o = heP - heA;
        heOppositeP->hasOpposite = 1;
        heP->o = nodeP->heIdx;
        heP->hasOpposite = 1;
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

