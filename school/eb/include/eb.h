#ifndef EB_H
#define EB_H

#include "vec2.h"
#include "vec3.h"
#include "vec4.h"
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
  Vec4* clr;  // TODO: suppport a color union later, or maybe decide to only supprt RGBA
  Vec2* tex;
} Vertex;

typedef struct {
  int m;
  HeLinkListNode* listOfHesEndingHere;
} VertexStatus;

typedef struct {
  int m;
  int onStack;
  Vertex v[3];  // indices to the vertex array
} Triangle;

typedef enum { C = 'C', L = 'L', E = 'E', R = 'R', S = 'S' } ClersChar;

typedef union {
  struct {
    float* xA;
    float* yA;
    float* zA;
  } pos;
} Delta;

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
  Delta delta;
} XmlResult;

typedef struct HalfEdge {
  VertexStatus
    *s,
    *e,
    *v;
  struct HalfEdge
    *n,
    *o,
    *p;
  Triangle* t;
} HalfEdge;
 

typedef struct {
  int newIsland;
  ClersChar clersChar;
  Triangle* t;  
} TraversalNode;

typedef struct {
  XmlResult pos, nml, clr, tex, tri;
  HalfEdge *heA;
  TraversalNode* traversalOrderA;
  VertexStatus *vstatA;
  int triElemsPresent;
} Mesh;

// Gives you all the half-edges and their relationships to their triangular counterparts
void getEdges( Mesh *meshP );
// Gives you the traversal order for compressing vertex attributes. 
// It also givecs you the CLERS connectivity compression codes so you only have to figure traversal out once.
void getConnectivity( Mesh *meshP );

#endif
