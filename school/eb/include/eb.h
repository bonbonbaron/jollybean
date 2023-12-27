#ifndef EB_H
#define EB_H

#include "vec2.h"
#include "vec3.h"
#include "fray.h"
#include <assert.h>

struct HalfEdge;  // This tells HeLinkListNode "I promise this type will exist later in this file."

// linked list for half-edges (HE).
typedef struct HeLinkListNode {
  struct HeLinkListNode* head, *tail, *next;
  struct HalfEdge* heP;
} HeLinkListNode;

void hellNew( HeLinkListNode** hellFP, unsigned int nElems );
HeLinkListNode* hellAdd( HeLinkListNode* hellF, HeLinkListNode* headP, struct HalfEdge* heP);
HeLinkListNode* hellNewHead( HeLinkListNode *hellF, struct HalfEdge* heP );
void dispList( HeLinkListNode* hellF, unsigned headIdx );

// Reason for this distinction is there's no need to
// duplicate mesh geometry information. The above Vertex
// type only serves as a temporary array element type
// during edgebreaking while the below is more permanent.
typedef struct {
  Vec3* pos;
  Vec3* nml;
  Vec3* clr;
  Vec2* tex;
  int m;  // has been encountered before
  HeLinkListNode* heTerminalHead;  // idx to head of linked list of HEs
} Vertex;

typedef struct {
  int m;
  Vertex v[3];  // indices to the vertex array
} Triangle;

typedef enum { C = 'C', L = 'L', E = 'E', R = 'R', S = 'S' } ClersChar;

typedef struct {
  int count;         // number of space-delimited elements in valString
  int stride;
  char* valString;
  union {
    int scalar; 
    float* scalarA;
    Vec2* vec2A;
    Vec3* vec3A;
    Triangle* triA;
  } u;
  union {
    float scalar;
    Vec2 vec2;
    Vec3 vec3;
    Triangle* triA;
  } min;
  union {
    float scalar;
    Vec2 vec2;
    Vec3 vec3;
    Triangle* triA;
  } max;
} XmlResult;

#if 0
typedef struct {
  int
    s,   // start vertex of this half-edge
    e,   // end vertex of this half-edge
    v,   // opposite vertex from this half-edge
    n,   // next counter-clockwise half-edge in this triangle
    p,   // previous counter-clockewise half-edge in this triangle
    o,   // half-edge coincident this one, but with opposite start and end
    t,   // index of triangle this half-edge belongs to
    // N,  // i dont think we need these anymore
    // P, // i dont think we need these anymore
    hasOpposite;
} HalfEdge;
#else
typedef struct HalfEdge {
  Vertex
    *s,
    *e,
    *v;
  struct HalfEdge
    *n,
    *o,
    *p;
  Triangle* t;
} HalfEdge;
#endif


typedef struct {
  ClersChar clersChar;
  int heIdx;  // TODO consider replacing this with a pointer like you're doing with everything else
} TraversalNode;

typedef struct {
  XmlResult pos, nml, clr, tex, tri;
  HalfEdge *heA;
  TraversalNode* traversalOrderA;
  int triElemsPresent;
} Mesh;

// Gives you all the half-edges and their relationships to their triangular counterparts
void getEdges( Mesh *meshP );
// Gives you the traversal order for compressing vertex attributes. 
// It also givecs you the CLERS connectivity compression codes so you only have to figure traversal out once.
void getConnectivity( Mesh *meshP );

#endif
