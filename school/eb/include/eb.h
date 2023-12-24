#ifndef EB_H
#define EB_H

#include "hell.h"
#include "vec2.h"
#include "vec3.h"

// Reason for this distinction is there's no need to
// duplicate mesh geometry information. The above Vertex
// type only serves as a temporary array element type
// during edgebreaking while the below is more permanent.
typedef struct {
  int positionIdx;
  int normalIdx;
  int colorIdx;
  int texelIdx;
} TriangleCorner;

typedef struct {
  TriangleCorner v[3];  // indices to the vertex array
} Triangle;


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

typedef struct {
  XmlResult pos, nml, clr, tex, tri;
  int triElemsPresent;
} Mesh;

typedef struct {
  int 
    m,  // has been encountered before
    b,  // is a boundary vertex
    heTerminalHead;  // idx to head of linked list of HEs
                     // ENDing at this vertex
} Vertex;

typedef struct {
  int   // pssst, consider making it a short later
    m, 
    s, 
    e, 
    v, 
    n, 
    p, 
    o, 
    N, 
    P,
    hasOpposite;
} HalfEdge;
#endif
