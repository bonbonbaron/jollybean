#ifndef EB_H
#define EB_H

#include "vec2.h"
#include "vec3.h"
#include "vec4.h"
#include "fray.h"
#include <assert.h>
#include <math.h>
#include <limits.h>

struct HalfEdge;  // This tells HeLinkListNode "I promise this type will exist later in this file."

// linked list for half-edges (HE).
typedef struct HeLinkListNode {
  struct HeLinkListNode* head, *tail, *next;
  struct HalfEdge* heP;
} HeLinkListNode;

// void hellNew( HeLinkListNode** hellFP, unsigned int nElems );
// HeLinkListNode* hellAdd( HeLinkListNode* hellF, HeLinkListNode* headP, struct HalfEdge* heP);
// HeLinkListNode* hellNewHead( HeLinkListNode *hellF, struct HalfEdge* heP );
// oid dispList( HeLinkListNode* hellF, unsigned headIdx );

// Reason for this distinction is there's no need to
// duplicate mesh geometry information. The above Vertex
// type only serves as a temporary array element type
// during edgebreaking while the below is more permanent.
typedef struct {
  Vec3* pos;
  Vec3* nml;
  Vec4* clr;  // TODO: suppport a color union later, or maybe decide to only supprt RGBA
  Vec2* tex;
} Vertex;

typedef struct {
  int m;
  int posIdx;  // index of this vertex's position; will come in handy when you quantize arrays later
  int nGatesPointingAtMe;
  HeLinkListNode* listOfHesEndingHere;
} VertexStatus;

typedef struct {
  int m;
  Vec3 normal;
  struct HalfEdge* g;  // gate used to access this triangle
  Vertex v[3];  // indices to the vertex array
} Triangle;

typedef enum { C, L, R, G, F } ClrgfChar;

typedef union {
  struct {
    short* xA;
    short* yA;
    short* zA;
  } pos;
} Quantized;


typedef union {
  struct {
    short* xA;
    short* yA;
    short* zA;
  } pos;
} Residual;

typedef struct {
  int count;         // number of space-delimited elements in valString
  int stride;
  char* valString;
  union {
    int scalar; 
    float* scalarA;
    Vec2* vec2A;
    Vec3* vec3A;
    Vec4* vec4A;
    Triangle* triA;
  } u;
  union {
    float scalar;
    Vec2 vec2;
    Vec3 vec3;
    Vec4 vec4;
    Triangle* triA;
  } min;
  union {
    float scalar;
    Vec2 vec2;
    Vec3 vec3;
    Vec4 vec4;
    Triangle* triA;
  } max;
  Residual residual;
  Quantized quantized;
} XmlResult;

typedef enum { UNMET, MET, DISCARDED } MetStatus;
typedef struct HalfEdge {
  VertexStatus
    *s,
    *e,
    *v;
  struct HalfEdge
    *n,
    *o,
    *p,
    *N,  // next half-edge on bounding loop (if this is a bounding half-edge; NULL otherwise)
    *P;  // prev half-edge on bounding loop (if this is a bounding half-edge; NULL otherwise)
  int boundaryId;   // if this Half-edge sits on a boundary,  tell us which boundary it is.  // TODO needed?
  struct HalfEdge* nextStartingGate;  // we loop through boundaries; then we start over for simple meshes
  MetStatus m; // indicates whether this half-edge has been met yet
  Triangle* t;
} HalfEdge;

typedef struct {
  int newIsland;
  ClrgfChar clrgfChar;
  HalfEdge* g;   // the gate used for the current triangle
} TriangleTraversalNode;

typedef struct {
  int jumped;
  HalfEdge* g;   // the gate used for the current triangle
} VertexTraversalNode;


/* Euler's characteristic of mesh:
 * (notes from https://faculty.cc.gatech.edu/~jarek/papers/EBholes.pdf)
 *  
 *    x(s) = |T(s)| - |E(s)| + |V(s)| = 2 - 2*g(s) - b(s)
 *
 *      where...
 *        s is the mesh
 *        T(s) are the triangles in s
 *        E(s) is the set of edges in s
 *        V(s) is the set of vertices in s
 *        g(s) is the genus of s
 *        b(s) is the number of boundary curves in s
 *
 *  But allowing islands in my meshes is complicating things.
 *
 *  I can know |E(s)|, |V(s)|, and b(s) overall from counting. T(s) is given.
 *  If I forbid islands, then g(s) is a simple matter of arithmetic. 
 *  Or... If i wait to count some things till I handle the islands, I'll be fine.
 *  I can store the individual counts in a sub-mesh linked list.
 */

typedef struct Mesh {
  XmlResult pos, nml, clr, tex, tri;
  HalfEdge *heA;
  HalfEdge *initialGate;  // TODO if you have a boundary, find the first bounding half-edge and start there
  TriangleTraversalNode* triangleTraversalOrderA;
  VertexTraversalNode* vertexTraversalOrderA;
  VertexStatus *vstatA;
  int triElemsPresent;
  int eulerCharacteristic;
  int genus;  // a hole may be considered an outer boundary if only one exists.
  int nBoundingLoops;  // we can't know this for a mesh with disparate parts,
  struct Mesh *islandListNodeP;
} Mesh;

// Gives you all the half-edges and their relationships to their triangular counterparts
void getEdges( Mesh *meshP );
// Gives you the traversal order for compressing vertex attributes. 
// It also givecs you the CLRGF connectivity compression codes so you only have to figure traversal out once.
void getConnectivity( Mesh *meshP );
// Compress positions coordinate-wise using parallelogram rule
void compressPositions( Mesh* meshP );

#endif
