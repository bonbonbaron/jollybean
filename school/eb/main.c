#include "xmlParser.h"
#undef FALSE
#undef TRUE
#include "eb.h"

#include <zlib.h>
#include <string.h>

// TODO Plan how you're going to refer to the materials associated with each triangle set.
// TODO Determine an optimal order to store triangles in to cache-boost indexing.
// TODO handle links to other blender files
// TODO handle textures
// NOTE Decompression will put all vertex attributes together into a single array.
static const char NAMESPACE_ALIAS[] = "mb";
static const xmlChar* GEOMETRY_DATA_XPATH = ( xmlChar * ) "/mb:COLLADA/mb:library_geometries/mb:geometry";  // gets each individual geometry
static const xmlChar* VERTEX_DATA_XPATH = ( xmlChar * ) "mb:mesh/mb:source";  // vertex attribute arrays
static const xmlChar* TRIANGLE_DATA_XPATH = ( xmlChar * ) "mb:mesh/mb:triangles";  // triangle data... duh


// Inflatables 
typedef struct Inflatable {
  U32 compressedLen;
  U32 inflatedLen;
  void *inflatedDataP;
  U8  *compressedDataA;
} Inflatable;

void inflatableNew(void *voidA, Inflatable **inflatablePP) {
  assert (voidA && inflatablePP);

  U32 szDataOrig = arrayGetNElems(voidA) * arrayGetElemSz(voidA);
  unsigned long szDataCompressed  = (szDataOrig * 1.1) + 12;
  U8 *dataOrigP = (U8*) voidA;
  U8 *dataCompressed = (U8*)malloc( szDataCompressed );
  int z_result = compress(dataCompressed, &szDataCompressed, dataOrigP, szDataOrig);    

  switch(z_result) {
    case Z_OK:
      break;
    case Z_MEM_ERROR:
    case Z_BUF_ERROR:
      printf( "out of memory\n" );
      exit( 1 );
      break;
    default:
      assert( z_result == 0 );
      break;
  }

  *inflatablePP = malloc( sizeof(Inflatable) * 1);
  assert( *inflatablePP );
  (*inflatablePP)->compressedLen   = szDataCompressed;
  (*inflatablePP)->inflatedLen     = szDataOrig;
  (*inflatablePP)->inflatedDataP   = NULL;
  (*inflatablePP)->compressedDataA = dataCompressed;
}

// The reason data.c doesn't own this function is because inflatables are permanent in-game.
void inflatableDel(Inflatable **inflatablePP) {
  if (inflatablePP && *inflatablePP) {
    if ((*inflatablePP)->compressedDataA) {
      free((*inflatablePP)->compressedDataA);
      (*inflatablePP)->compressedDataA = NULL;
    }
    if ((*inflatablePP)->inflatedDataP) {
      free((*inflatablePP)->inflatedDataP);
      (*inflatablePP)->inflatedDataP = NULL;
    }
    jbFree((void**) inflatablePP);
  }
}


// This assumes elements are separated by whitespace only.


void extractVec2Array( XmlResult *resultP ) {
  assert( resultP );
  assert( resultP->count );
  assert( resultP->valString );

  resultP->min.vec2.s = FLT_MAX;
  resultP->min.vec2.t = FLT_MAX;
  resultP->max.vec2.s = FLT_MIN;
  resultP->max.vec2.t = FLT_MIN;

  arrayNew( (void**) &resultP->u.vec2A, sizeof( Vec2 ), resultP->count / 2 );  // TODO fix w/o division
  assert( resultP->u.vec2A );

  // Extract string into array here.
  char* cPtr = resultP->valString;
  char* cEndPtr = cPtr + strlen( resultP->valString );
  Vec2* vec2P = resultP->u.vec2A;
  while ( cPtr < cEndPtr ) {
    /*****/
    /* X */
    /*****/
    vec2P->s = atof( cPtr );
    // Skip past the current number
    while ( *cPtr != ' ' && *cPtr != '\0' ) {
      ++cPtr;
    }
    // Skip past the current space to the next number
    while ( *cPtr == ' ' && *cPtr != '\0' ) {
      ++cPtr;
    }
    /*****/
    /* Y */
    /*****/
    vec2P->t = atof( cPtr );
    // Skip past the current number
    while ( *cPtr != ' ' && *cPtr != '\0' ) {
      ++cPtr;
    }
    // Skip past the current space to the next number
    while ( *cPtr == ' ' && *cPtr != '\0' ) {
      ++cPtr;
    }

    // Check min
    if ( vec2P->s < resultP->min.vec2.s ) {
      resultP->min.vec2.s = vec2P->s;
    }
    if ( vec2P->t < resultP->min.vec2.t ) {
      resultP->min.vec2.t = vec2P->t;
    }
    // Check max
    if ( vec2P->s > resultP->max.vec2.s ) {
      resultP->max.vec2.s = vec2P->s;
    }
    if ( vec2P->t > resultP->max.vec2.t ) {
      resultP->max.vec2.t = vec2P->t;
    }
    ++vec2P;
  }
  // xmlFree( mesh.pos.valueString );  // TODO be sure to do this later
}

void extractVec3Array( XmlResult *resultP ) {
  assert( resultP );
  assert( resultP->count );
  assert( resultP->valString );

  resultP->min.vec3.x = FLT_MAX;
  resultP->min.vec3.y = FLT_MAX;
  resultP->min.vec3.z = FLT_MAX;
  resultP->max.vec3.x = FLT_MIN;
  resultP->max.vec3.y = FLT_MIN;
  resultP->max.vec3.z = FLT_MIN;

  arrayNew( (void**) &resultP->u.vec3A, sizeof( Vec3 ), resultP->count / 3 );   // TODO fix without division
  assert( resultP->u.vec3A );

  // Extract string into array here.
  char* cPtr = resultP->valString;
  char* cEndPtr = cPtr + strlen( resultP->valString );
  Vec3* vec3P = resultP->u.vec3A;
  while ( cPtr < cEndPtr ) {
    /*****/
    /* X */
    /*****/
    vec3P->x = atof( cPtr );
    // Skip past the current number
    while ( *cPtr != ' ' && *cPtr != '\0' ) {
      ++cPtr;
    }
    // Skip past the current space to the next number
    while ( *cPtr == ' ' && *cPtr != '\0' ) {
      ++cPtr;
    }
    /*****/
    /* Y */
    /*****/
    vec3P->y = atof( cPtr );
    // Skip past the current number
    while ( *cPtr != ' ' && *cPtr != '\0' ) {
      ++cPtr;
    }
    // Skip past the current space to the next number
    while ( *cPtr == ' ' && *cPtr != '\0' ) {
      ++cPtr;
    }
    /*****/
    /* Z */
    /*****/
    vec3P->z = atof( cPtr );
    // Skip past the current number
    while ( *cPtr != ' ' && *cPtr != '\0' ) {
      ++cPtr;
    }
    // Skip past the current space to the next number
    while ( *cPtr == ' ' && *cPtr != '\0' ) {
      ++cPtr;
    }

    // Check min
    if ( vec3P->x < resultP->min.vec3.x ) {
      resultP->min.vec3.x = vec3P->x;
    }
    if ( vec3P->y < resultP->min.vec3.y ) {
      resultP->min.vec3.y = vec3P->y;
    }
    if ( vec3P->z < resultP->min.vec3.z ) {
      resultP->min.vec3.z = vec3P->z;
    }
    // Check max
    if ( vec3P->x > resultP->max.vec3.x ) {
      resultP->max.vec3.x = vec3P->x;
    }
    if ( vec3P->y > resultP->max.vec3.y ) {
      resultP->max.vec3.y = vec3P->y;
    }
    if ( vec3P->z > resultP->max.vec3.z ) {
      resultP->max.vec3.z = vec3P->z;
    }
    ++vec3P;
  }
}

void extractVec4Array( XmlResult *resultP ) {
  assert( resultP );
  assert( resultP->count );
  assert( resultP->valString );

  resultP->min.vec4.r = FLT_MAX;
  resultP->min.vec4.g = FLT_MAX;
  resultP->min.vec4.b = FLT_MAX;
  resultP->min.vec4.a = FLT_MAX;
  resultP->max.vec4.r = FLT_MIN;
  resultP->max.vec4.g = FLT_MIN;
  resultP->max.vec4.b = FLT_MIN;
  resultP->max.vec4.a = FLT_MIN;

  arrayNew( (void**) &resultP->u.vec4A, sizeof( Vec4 ), resultP->count );
  assert( resultP->u.vec4A );

  // Extract string into arrag here.
  char* cPtr = resultP->valString;
  char* cEndPtr = cPtr + strlen( resultP->valString );
  Vec4* vec4P = resultP->u.vec4A;
  while ( cPtr < cEndPtr ) {
    /*****/
    /* R */
    /*****/
    vec4P->r = atof( cPtr );
    // Skip past the current number
    while ( *cPtr != ' ' && *cPtr != '\0' ) {
      ++cPtr;
    }
    // Skip past the current space to the next number
    while ( *cPtr == ' ' && *cPtr != '\0' ) {
      ++cPtr;
    }
    /*****/
    /* G */
    /*****/
    vec4P->g = atof( cPtr );
    // Skip past the current number
    while ( *cPtr != ' ' && *cPtr != '\0' ) {
      ++cPtr;
    }
    // Skip past the current space to the next number
    while ( *cPtr == ' ' && *cPtr != '\0' ) {
      ++cPtr;
    }
    /*****/
    /* B */
    /*****/
    vec4P->b = atof( cPtr );
    // Skip past the current number
    while ( *cPtr != ' ' && *cPtr != '\0' ) {
      ++cPtr;
    }
    // Skip past the current space to the next number
    while ( *cPtr == ' ' && *cPtr != '\0' ) {
      ++cPtr;
    }

    /*****/
    /* A */
    /*****/
    vec4P->a = atof( cPtr );
    // Skip past the current number
    while ( *cPtr != ' ' && *cPtr != '\0' ) {
      ++cPtr;
    }
    // Skip past the current space to the next number
    while ( *cPtr == ' ' && *cPtr != '\0' ) {
      ++cPtr;
    }

    // Check min
    if ( vec4P->r < resultP->min.vec4.r ) {
      resultP->min.vec4.r = vec4P->r;
    }
    if ( vec4P->g < resultP->min.vec4.g ) {
      resultP->min.vec4.g = vec4P->g;
    }
    if ( vec4P->b < resultP->min.vec4.b ) {
      resultP->min.vec4.b = vec4P->b;
    }
    if ( vec4P->a < resultP->min.vec4.a ) {
      resultP->min.vec4.a = vec4P->a;
    }
    // Check max
    if ( vec4P->r > resultP->max.vec4.r ) {
      resultP->max.vec4.r = vec4P->r;
    }
    if ( vec4P->g > resultP->max.vec4.g ) {
      resultP->max.vec4.g = vec4P->g;
    }
    if ( vec4P->b > resultP->max.vec4.b ) {
      resultP->max.vec4.b = vec4P->b;
    }
    if ( vec4P->a > resultP->max.vec4.a ) {
      resultP->max.vec4.a = vec4P->a;
    }
    ++vec4P;
  }
}

/* To separate concerns, I'm only going to have this know about XML.
 * Then anybody else can come in and extract an array of something. 
 * It'll return a simple data structure containiing a string and count. */

inline static int getStride(xmlDocPtr docP, xmlNodePtr nodeP, xmlXPathContextPtr context) {
  context->node = nodeP->next->next->children->next;  // first next is a newline for some reason
  //printf("curr node name: %s\n", nodeP->name);
  //printf("next node name: %s\n", context->node->name);
  xmlXPathObjectPtr r = xmlGetNodes ( docP, context, (xmlChar*) "@stride" );
  //printf("\n\n\nhey: %s\n\n\n", r->nodesetval->nodeTab[0]->children->content);
  return atoi( (char*) r->nodesetval->nodeTab[0]->children->content);
}

/* It also needs to be able to return a node. That way we can extract any
 * node we want and then ask it to return any property from it we want. */
/* Rather than factoring this code out, we hard-code it to our specific use-case  
   to avoid re-iterating over any linked lists. */
 
void getVertexAttributes( Mesh* meshP, xmlXPathContextPtr xpathContext, xmlXPathObjectPtr xpathResultP, xmlDocPtr docP ) {
  assert( meshP && xpathResultP && xpathContext && docP );
  // Don't actually change the context. Just copy it.
  xmlXPathContextPtr context = xpathContext;
  XmlResult* resultP;
  xmlNodePtr propNodeP;
  forEachNode_( xpathResultP, top ) {
    resultP = NULL;  // reset the result pointer so we don't falsely populate the count later on.
    // For each property of this node...
    forEachChild_( topNodePP, floatArray ) {
      // Then keep the xml objects alive until you're finished compressing each mesh.
      // If this is an identifier node, determine which mesh property it is.
      if ( !(strcmp( (char*) floatArrayChildNodeP->name, "float_array" ) ) ) {
        // For each property of the current <float_array>...
        forEachProperty_( &floatArrayChildNodeP, floatArray ) {
          propNodeP = floatArrayPropertyNodeP->children;
          // Determine which float array this is.
          if( strstr( (char*) propNodeP->content, "mesh-positions" ) > 0 ) {
            resultP = &meshP->pos;
          }  
          else if( strstr( (char*) propNodeP->content, "mesh-normals" ) > 0 ) {
            resultP = &meshP->nml;
          } 
          else if( strstr( (char*) propNodeP->content, "mesh-colors" ) > 0 ) {
            resultP = &meshP->clr;
          }
          else if( strstr( (char*) propNodeP->content, "mesh-map" ) > 0 ) {
            resultP = &meshP->tex;
          }
          // Get the count 
          else if( !strcmp( (char*) floatArrayPropertyNodeP->name, "count" ) ) {
            resultP->count = atoi( (char*) floatArrayPropertyNodeP->children->content );
            break;  // no more properties required
          }
        }
        // Extract array.
        if ( resultP ) {
          resultP->stride = getStride(docP, floatArrayChildNodeP, context);
          resultP->valString = (char*) floatArrayChildNodeP->children->content;
          switch( resultP->stride ) {
            case 2:
              extractVec2Array( resultP );
              break;
            case 3:
              extractVec3Array( resultP );
              break;
            case 4:
              extractVec4Array( resultP );
              break;
            case 1:   // TODO support if you ever need to with new extractScalarArray() function
            default:
              printf("Stride length of %d is unsupported. Stopping now...\n", resultP->stride);
              exit(1);
              break;
          }
          goto skipToNextSource;
        }
      }
    }
skipToNextSource:
    continue;
  }  
}  

// =========
// TRIANGLES
// =========

typedef enum { POSITION = 1, NORMAL = 2, COLOR = 4, TEXTURE = 8 } TriElem;
void getTriangles( Mesh* meshP, xmlXPathContextPtr context, xmlXPathObjectPtr triangleXpathResult, xmlDocPtr docP ) {
  assert( meshP && triangleXpathResult && context && docP );
  // Don't actually change the context. Just copy it.
  context->node = triangleXpathResult->nodesetval->nodeTab[0];
  xmlXPathObjectPtr triCountNodeP, semanticNodesetP, vertexArrayNode;
  // TODO get elements needed for CURRENT set of triangles
  // TODO get number of triangles in CURRENT set of triangles
  // TODO pray this is only for the current geometry... right??
  // Count total number of triangles. Could be separate triangle sub-arrays for different materials.
  forEachNode_( triangleXpathResult, _triangle ) {
    // First we get the number of triangles that use the current material.
    context->node = *_triangleNodePP;
    triCountNodeP = xmlGetNodes ( docP, context, (xmlChar*) "@count" );
    meshP->tri.count += atoi( (char*) triCountNodeP->nodesetval->nodeTab[0]->children[0].content );
  }

  // printf("total number of triangles: %d\n", meshP->tri.count );

  // Then allocate your triangle array.
  arrayNew( (void**) &meshP->tri.u.triA, sizeof(Triangle), meshP->tri.count );
  assert( meshP->tri.u.triA );

  // Next, figure out what kind of triangle data we're dealing with.
  meshP->triElemsPresent = 0;
  int dstTriIdx = 0;
  forEachNode_( triangleXpathResult, triangle ) {
    context->node = *triangleNodePP;

    void* arrayPtrs[4];
    int 
      elementSzs[4],
      dstOffsets[4], 
      numAttrsPerCorner = 0;  // stores where to put each triangle corner element & how many
    semanticNodesetP = xmlGetNodes ( docP, context, (xmlChar*) "./mb:input/@semantic" );
    static Triangle dummyTriangle;
    if ( semanticNodesetP ) {
      forEachNode_( semanticNodesetP, semantic ) {
        if (!strcmp( (char*) (*semanticNodePP)->children->content, "VERTEX" ) ) {
          meshP->triElemsPresent |= POSITION;
          arrayPtrs[numAttrsPerCorner] = meshP->pos.u.vec3A;
          elementSzs[numAttrsPerCorner] = sizeof( Vec3 );
          dstOffsets[numAttrsPerCorner++] = (U8*) &dummyTriangle.v[0].pos - (U8*) &dummyTriangle.v[0];
        }
        else if (!strcmp( (char*) (*semanticNodePP)->children->content, "NORMAL" ) ) {
          meshP->triElemsPresent |= NORMAL;
          arrayPtrs[numAttrsPerCorner] = meshP->nml.u.vec3A;
          elementSzs[numAttrsPerCorner] = sizeof( Vec3 );
          dstOffsets[numAttrsPerCorner++] = (U8*) &dummyTriangle.v[0].nml - (U8*) &dummyTriangle.v[0];
        }
        else if (!strcmp( (char*) (*semanticNodePP)->children->content, "COLOR" ) ) {
          meshP->triElemsPresent |= COLOR;
          arrayPtrs[numAttrsPerCorner] = meshP->clr.u.vec4A;
          elementSzs[numAttrsPerCorner] = sizeof( Vec4 );
          dstOffsets[numAttrsPerCorner++] = (U8*) &dummyTriangle.v[0].clr - (U8*) &dummyTriangle.v[0];
        }
        else if (!strcmp( (char*) (*semanticNodePP)->children->content, "TEXCOORD" ) ) {
          meshP->triElemsPresent |= TEXTURE;
          arrayPtrs[numAttrsPerCorner] = meshP->tex.u.vec2A;
          elementSzs[numAttrsPerCorner] = sizeof( Vec2 );
          dstOffsets[numAttrsPerCorner++] = (U8*) &dummyTriangle.v[0].tex - (U8*) &dummyTriangle.v[0];
        }
      }
    }

    // Finally, grab all the data.
    vertexArrayNode = xmlGetNodes ( docP, context, (xmlChar*) "./mb:p" );
    int offsetIdx, cornerIdx;
    void** triElemPP;
    // For each <p> array in current <triangles> node (there should only be one, but idk what idk)...
    // Get all offsets
    forEachNode_( vertexArrayNode, vertexElem ) {
      meshP->tri.valString = (char*) ((*vertexElemNodePP)->children->content);
#define NUM_CORNERS_PER_TRIANGLE (3)
      // For all the input triangles in current set of triangles...
      char* cPtr = meshP->tri.valString;
      char* cEndPtr = cPtr + strlen( meshP->tri.valString );
      for ( ; cPtr < cEndPtr; ++dstTriIdx ) {
        // For each corner of the current triangle...
        for ( cornerIdx = 0; cornerIdx < 3; ++cornerIdx ) {
          // For each attribute of each corner (holy nested loops Batman)...
          for ( offsetIdx = 0; offsetIdx < numAttrsPerCorner; ++offsetIdx ) {
            triElemPP = (void**) ((U8*) &meshP->tri.u.triA[ dstTriIdx ].v[ cornerIdx ] + dstOffsets[ offsetIdx ] );
            *triElemPP = (U8*) arrayPtrs[ offsetIdx ] + elementSzs[ offsetIdx ] * atoi( cPtr );
            // Skip past the current number
            while ( *cPtr != ' ' && *cPtr != '\0' ) {
              ++cPtr;
            }
            // Skip past the current space to the next number
            while ( *cPtr == ' ' && *cPtr != '\0' ) {
              ++cPtr;
            }
          }  // for each attribute of each corner
        }  // for each corner 
#if 0
        Triangle* triP = &meshP->tri.u.triA[ dstTriIdx ];
        for (int i = 0; i < 3; ++i) {
          printf("%04d: tri.v[%d].pos = { %f, %f, %f }\n", dstTriIdx, i, triP->v[i].pos->x, triP->v[i].pos->y, triP->v[i].pos->z );
          printf("%04d: tri.v[%d].nml = { %f, %f, %f }\n", dstTriIdx, i, triP->v[i].nml->x, triP->v[i].nml->y, triP->v[i].nml->z );
          if ( triP->v[i].clr ) {
            printf("%04d: tri.v[%d].clr = { %f, %f, %f }\n", dstTriIdx, i, triP->v[i].clr->x, triP->v[i].clr->y, triP->v[i].clr->z );
          }
          if ( triP->v[i].tex ) {
            printf("%04d: tri.v[%d].tex = { %f, %f }\n", dstTriIdx, i, triP->v[i].tex->s, triP->v[i].tex->t );
          }
        }
#endif
      }  // while character pointer is still traversing through array string
    }  // for each <p> node in current <triangles>
  }  // for each <triangles> node in this geometry
}  // getTriangles()


void pack(U16* array, int bits, U8** result) {
  U32 buffer = 0;
  int buffer_bits = 0;
  int result_index = 0;

  U32 nInputElems = arrayGetNElems( array );
  arrayNew( (void**) result, sizeof( U8 ), ( nInputElems * bits + 7 ) / 8); 
  for(int idx = 0; idx < nInputElems; idx++) {
    buffer |= (array[idx] << buffer_bits);
    buffer_bits += bits;

    while(buffer_bits >= 8) {
      (*result)[result_index++] = buffer & 0xFF;
      buffer >>= 8;
      buffer_bits -= 8;
    }
  }

  if(buffer_bits > 0) {
    (*result)[result_index++] = buffer;
  }
}

void clrMesh( Mesh* meshP ) {
  arrayDel( (void**) &meshP->pos.u.vec3A );
  arrayDel( (void**) &meshP->pos.quantized.pos.xA );
  arrayDel( (void**) &meshP->pos.quantized.pos.yA );
  arrayDel( (void**) &meshP->pos.quantized.pos.zA );
  arrayDel( (void**) &meshP->pos.residual.pos.xA );
  arrayDel( (void**) &meshP->pos.residual.pos.yA );
  arrayDel( (void**) &meshP->pos.residual.pos.zA );
  arrayDel( (void**) &meshP->nml.u.vec3A );
  arrayDel( (void**) &meshP->clr.u.vec4A );
  arrayDel( (void**) &meshP->tex.u.vec2A );
  arrayDel( (void**) &meshP->tri.u.triA );
  arrayDel( (void**) &meshP->heA );
  arrayDel( (void**) &meshP->traversalOrderA );
  arrayDel( (void**) &meshP->vstatA );
  memset( meshP, 0, sizeof( Mesh ) );
}

int main ( int argc, char **argv ) {
  Mesh mesh = { 0 };
  // Get the document name we want to parse.
  if ( argc <= 1 ) {
    printf ( "Usage: %s docname\n", argv[0] );
    return ( 1 );
  }
  char* docname = argv[1];

  // Parse document and get its namespace context we're obligated to use for everything.
  xmlDocPtr docP = xmlParse ( docname );
  xmlXPathContextPtr xpathContext = xmlGetNamespaceContext( docP, NAMESPACE_ALIAS );
  if ( xpathContext == NULL ) {
    printf( "null xpathContext\n" );
    return 1;
  }

  // Execute XPath searches.
  // First, find all the geometry nodes.
  xmlXPathObjectPtr geometryXpathResult = xmlGetNodes ( docP, xpathContext, GEOMETRY_DATA_XPATH );
  // Second, iterate through the geometries to extract their information.
  forEachNode_( geometryXpathResult, geometry ) {
    // Confine the current XPath searches to only the current geometry node.
    xpathContext->node = *geometryNodePP;
    xmlXPathObjectPtr vertexXpathResult   = xmlGetNodes ( docP, xpathContext, VERTEX_DATA_XPATH );
    xmlXPathObjectPtr triangleXpathResult = xmlGetNodes ( docP, xpathContext, TRIANGLE_DATA_XPATH );

    // Extract all data from XML.
    getVertexAttributes( &mesh, xpathContext, vertexXpathResult, docP );
    getTriangles( &mesh, xpathContext, triangleXpathResult, docP );
    getEdges( &mesh );
    getConnectivity( &mesh );
    // Compress them
    compressPositions( &mesh );

#if 0
    // Raw quantization
    U16* qPosA = NULL;
    arrayNew( (void**) &qPosA, sizeof(U16), mesh.pos.count * 3 );
    assert( qPosA );
    // TODO macro-out 1024 so we tweak the number of bits and all its dependencies with one single parameter.
    const float convX = 1024.0 / fabs( mesh.pos.max.vec3.x - mesh.pos.min.vec3.x );
    const float convY = 1024.0 / fabs( mesh.pos.max.vec3.y - mesh.pos.min.vec3.y );
    const float convZ = 1024.0 / fabs( mesh.pos.max.vec3.z - mesh.pos.min.vec3.z );
    for (int i = 0; i < mesh.pos.count; ++i) {
      qPosA[ 3 * i     ] = (int) ( (mesh.pos.u.vec3A[i].x - mesh.pos.min.vec3.x ) * convX);
      qPosA[ 3 * i + 1 ] = (int) ( (mesh.pos.u.vec3A[i].y - mesh.pos.min.vec3.y ) * convY);
      qPosA[ 3 * i + 2 ] = (int) ( (mesh.pos.u.vec3A[i].z - mesh.pos.min.vec3.z ) * convZ);
    }
    printf( "\n\nQuantized from %dB to %dB.\n", mesh.pos.count * sizeof(Vec3), arrayGetElemSz( qPosA ) * arrayGetNElems( qPosA ) );

    // Pack quantized bits together
    U8* packedQPosA =  NULL;
    pack( qPosA, 10, &packedQPosA );
    printf( "Packed from %dB to %dB.\n", arrayGetElemSz( qPosA ) * arrayGetNElems( qPosA ), arrayGetElemSz( packedQPosA ) * arrayGetNElems( packedQPosA ) );
    assert( packedQPosA );

    // Compress quantization results (might make it worse)
    Inflatable *infP = NULL;
    inflatableNew( packedQPosA, &infP );
    printf( "Compressed from %dB to %dB.\n", arrayGetNElems( packedQPosA ) * arrayGetElemSz( packedQPosA ), infP->compressedLen );
    arrayDel( (void**) &qPosA );
    arrayDel( (void**) &packedQPosA );
    inflatableDel( &infP );
#endif

    // Free everything.
    xmlFreeXpathResult( &vertexXpathResult );
    xmlFreeXpathResult( &triangleXpathResult );
    clrMesh( &mesh );
  }  // for each geometry in this  mesh
  xmlClean( &docP, &xpathContext );

  return 0;
}
