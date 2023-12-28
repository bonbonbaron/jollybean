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
  Vec4* clr;  // TODO: suppport a color union later, or maybe decide to only supprt RGBA
  Vec2* tex;
} Vertex;

typedef struct {
  int m;
  int posIdx;  // index of this vertex's position; will come in handy when you quantize arrays later
  HeLinkListNode* listOfHesEndingHere;
} VertexStatus;

typedef struct {
  int m;
  int onStack;
  Vec3 normal;
  struct HalfEdge* g;  // gate used to access this triangle
  Vertex v[3];  // indices to the vertex array
} Triangle;

typedef enum { C = 'C', L = 'L', E = 'E', R = 'R', S = 'S', M = 'M' } ClersChar;
typedef enum { CLERS, CLERSM, CLERSM_WITH_HANDLE } MeshType;  // CLERSM changes the code length of LERS from to 4 bits

typedef union {
  struct {
    short* xA;
    short* yA;
    short* zA;
  } pos;
} Quantized;

#define GENERATED_COORDINATE (0xffff)

typedef union {
  struct {
    short* xA;
    short* yA;
    short* zA;
  } pos;
} Residual;

typedef union {
  struct {
    short* xA;
    short* yA;
    short* zA;
  } pos;
} Generated;

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
  Generated generated;
} XmlResult;

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
  int m; // indicates whether this half-edge has been met yet, and whether it's exterior or interior
  Triangle* t;
} HalfEdge;

typedef struct {
  int newIsland;
  ClersChar clersChar;
  HalfEdge* g;   // the gate used for the current triangle
} TriangleTraversalNode;

typedef struct {
  int jumped;
  HalfEdge* g;   // the gate used for the current triangle
} VertexTraversalNode;


typedef struct {
  XmlResult pos, nml, clr, tex, tri;
  HalfEdge *heA;
  HalfEdge *initialGate;  // TODO if you have a boundary, find the first bounding half-edge and start there
  TriangleTraversalNode* triangleTraversalOrderA;
  VertexTraversalNode* vertexTraversalOrderA;
  VertexStatus *vstatA;
  int triElemsPresent;
  int nLoners;
  int nBoundingLoops;
  int genus;  // a hole may be considered an outer boundary if only one exists.
  MeshType meshType;
} Mesh;

// Gives you all the half-edges and their relationships to their triangular counterparts
void getEdges( Mesh *meshP );
// Gives you the traversal order for compressing vertex attributes. 
// It also givecs you the CLERS connectivity compression codes so you only have to figure traversal out once.
void getConnectivity( Mesh *meshP );
// Compress positions coordinate-wise using parallelogram rule
void compressPositions( Mesh* meshP );

#endif
