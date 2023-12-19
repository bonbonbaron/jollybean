#include "xmlParser.h"
#undef FALSE
#undef TRUE
#include "eb.h"

#include <math.h>
#include <limits.h>
#include <zlib.h>

// TODO Plan how you're going to refer to the materials associated with each triangle set.
// TODO Determine an optimal order to store triangles in to cache-boost indexing.
// TODO handle multiple <geometry> tags, where each is a standalone object in the scene.
// TODO handle links to other blender files
// TODO handle textures
// TODO combine geometries with triangles separated for multiple textures
// TODO handle triangles with varying numbers of attributes
// NOTE Decompression will put all vertex attributes together into a single array.
static const char NAMESPACE_ALIAS[] = "mb";
static const xmlChar* GEOMETRY_DATA_XPATH = ( xmlChar * ) "/mb:COLLADA/mb:library_geometries/mb:geometry";
static const xmlChar* VERTEX_DATA_XPATH = ( xmlChar * ) "mb:mesh/mb:source";
static const xmlChar* TRIANGLE_DATA_XPATH = ( xmlChar * ) "mb:mesh/mb:triangles";

typedef struct {
  int count;         // number of space-delimited elements in this set
  int stride;
  char* valString;
  union {
    int scalar; 
    float* scalarA;
    Vec2* vec2A;
    Vec3* vec3A;
  } u;
  union {
    float scalar;
    Vec2 vec2;
    Vec3 vec3;
  } min;
  union {
    float scalar;
    Vec2 vec2;
    Vec3 vec3;
  } max;
} XmlResult;

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

// This assumes elements are separated by whitespace only.
static int _getNextNumberIdx( char* string, unsigned startIdx, int numToSkip ) {
  assert( startIdx >= 0 );

  if ( startIdx >= strlen( string ) ) {
    return -1;
  }

  char* cPtr = &string[ startIdx ];

  // Skip past three numbers
  for ( int i = 0; i < numToSkip; ++i ) {
    // Skip past the current number
    while ( *cPtr != ' ' && *cPtr != '\0' ) {
      ++cPtr;
    }
    // Skip past the current space to the next number
    while ( *cPtr == ' ' && *cPtr != '\0' ) {
      ++cPtr;
    }
  }

  return cPtr - string;
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

  resultP->u.vec3A = malloc( sizeof( Vec3 ) * resultP->count );
  assert( resultP->u.vec3A );

  // Extract string into array here.
  for (int j = 0, sIdx = 0; sIdx >= 0 && j < resultP->count; ++j) {
    sscanf( &resultP->valString[sIdx], "%f %f %f", &resultP->u.vec3A[j].x, &resultP->u.vec3A[j].y, &resultP->u.vec3A[j].z );
    sIdx = _getNextNumberIdx( resultP->valString, sIdx, 3 );

    // Check min
    if ( resultP->u.vec3A[j].x < resultP->min.vec3.x ) {
      resultP->min.vec3.x = resultP->u.vec3A[j].x;
    }
    if ( resultP->u.vec3A[j].y < resultP->min.vec3.y ) {
      resultP->min.vec3.y = resultP->u.vec3A[j].y;
    }
    if ( resultP->u.vec3A[j].z < resultP->min.vec3.z ) {
      resultP->min.vec3.z = resultP->u.vec3A[j].z;
    }
    // Check max
    if ( resultP->u.vec3A[j].x > resultP->max.vec3.x ) {
      resultP->max.vec3.x = resultP->u.vec3A[j].x;
    }
    if ( resultP->u.vec3A[j].y > resultP->max.vec3.y ) {
      resultP->max.vec3.y = resultP->u.vec3A[j].y;
    }
    if ( resultP->u.vec3A[j].z > resultP->max.vec3.z ) {
      resultP->max.vec3.z = resultP->u.vec3A[j].z;
    }
  }
  // xmlFree( mesh.posResult.valueString );  // TODO be sure to do this later
}

void extractVec2Array( XmlResult *resultP ) {
  assert( resultP );
  assert( resultP->count );
  assert( resultP->valString );

  resultP->min.vec2.s = FLT_MAX;
  resultP->min.vec2.t = FLT_MAX;
  resultP->max.vec2.s = FLT_MIN;
  resultP->max.vec2.t = FLT_MIN;

  resultP->u.vec2A = malloc( sizeof( Vec2 ) * resultP->count );
  assert( resultP->u.vec2A );

  // Extract string into array here.
  for (int j = 0, sIdx = 0; sIdx >= 0 && j < resultP->count; ++j) {
    sscanf( &resultP->valString[sIdx], "%f %f", &resultP->u.vec2A[j].s, &resultP->u.vec2A[j].t );
    sIdx = _getNextNumberIdx( resultP->valString, sIdx, 2 );

    // Check min
    if ( resultP->u.vec2A[j].s < resultP->min.vec2.s ) {
      resultP->min.vec2.s = resultP->u.vec2A[j].s;
    }
    if ( resultP->u.vec2A[j].t < resultP->min.vec2.t ) {
      resultP->min.vec2.t = resultP->u.vec2A[j].t;
    }
    // Check max
    if ( resultP->u.vec2A[j].s > resultP->max.vec2.s ) {
      resultP->max.vec2.s = resultP->u.vec2A[j].s;
    }
    if ( resultP->u.vec2A[j].t > resultP->max.vec2.t ) {
      resultP->max.vec2.t = resultP->u.vec2A[j].t;
    }
  }
  // xmlFree( mesh.posResult.valueString );  // TODO be sure to do this later
}


typedef struct {
  XmlResult posResult, nmlResult, clrResult, texResult, triResult;
} Mesh;


/* To separate concerns, I'm only going to have this know about XML.
 * Then anybody else can come in and extract an array of something. 
 * It'll return a simple data structure containiing a string and count. */

static int getStride(xmlDocPtr docP, xmlNodePtr nodeP, xmlXPathContextPtr context) {
  context->node = nodeP->next->next->children->next;  // first next is a newline for some reason
  //printf("curr node name: %s\n", nodeP->name);
  //printf("next node name: %s\n", context->node->name);
  xmlXPathObjectPtr r = xmlGetNodes ( docP, context, (char*) "@stride" );
  //printf("\n\n\nhey: %s\n\n\n", r->nodesetval->nodeTab[0]->children->content);
  return atoi( (char*) r->nodesetval->nodeTab[0]->children->content);
}

/* It also needs to be able to return a node. That way we can extract any
 * node we want and then ask it to return any property from it we want. */
/* Rather than factoring this code out, we hard-code it to our specific use-case  
   to avoid re-iterating over any linked lists. */
 
void getVertexAttributes( Mesh* meshP, xmlXPathContextPtr xpathContext, xmlXPathObjectPtr xpathResultP, xmlDocPtr docP ) {
  assert( meshP && xpathResultP );
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
            resultP = &meshP->posResult;
          }  
          else if( strstr( (char*) propNodeP->content, "mesh-normals" ) > 0 ) {
            resultP = &meshP->nmlResult;
          } 
          else if( strstr( (char*) propNodeP->content, "mesh-colors" ) > 0 ) {
            resultP = &meshP->texResult;
          }
          else if( strstr( (char*) propNodeP->content, "mesh-map" ) > 0 ) {
            resultP = &meshP->texResult;
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
            case 1:   // TODO support if you ever need to with new extractScalarArray() function
            case 4:   // TODO support if you ever need to with new extractVec4Array() function
            default:
              printf("Stride length of %d is unsupported. Stopping now...\n", resultP->stride);
              exit(1);
              break;
          }
          // TODO go ahead and extract the value here.
          printf("val: %s\n", resultP->valString);
          goto skipToNextSource;
        }
      }
    }
skipToNextSource:
    continue;
  }  
}  

#if 0
// ======
// COLORS
// ======
// For now, only support one set of colors per geometry.
}
}
#endif

// =========
// TRIANGLES
// =========
void getTriangles( Mesh* meshP, xmlXPathObjectPtr xpathResultP ) {
  assert( meshP && xpathResultP );
  char* triArrayString;
  xmlNodePtr propNodeP;
  int numElemsPerUnit;  // TODO determine this programmatically
  forEachNode_( xpathResultP, parent ) {
    forEachProperty_( parentNodePP, parent ) {
      if ( parentPropertyNodeP->type == XML_ATTRIBUTE_NODE ) {
        // TODO rather than clone strings, just point at their values.
        //      Then keep the xml objects alive until you're finished compressing each mesh.
        printf("prop node name: %s\n", parentPropertyNodeP->name);
        // If this is an identifier node, determine which mesh property it is.
        if ( !(strcmp( (char*) parentPropertyNodeP->name, "id" ) ) ) {
          propNodeP = parentPropertyNodeP->children;
        }
      }
    }
    if( strstr( (char*) propNodeP->content, "mesh-vertices" ) > 0 ) {
      // These may have anywhere from 2-4 elements per triangle corner.
      // So determine the type of triangle by looking at all the input elements' "semantic" properties.
      numElemsPerUnit = 2;  // TODO determine this programmatically
                            // Extract the triangles.
      forEachChild_( parentNodePP, curr ) {
        if (currChildNodeP->type == XML_TEXT_NODE) {
          triArrayString = (char*) currChildNodeP->content;
          printf("\ttexels child node val: %s\n\n", currChildNodeP->content );
          break;
        }
      }
    }
  }
}

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

  // Any remaining bits.
  if(buffer_bits > 0) {
    (*result)[result_index++] = buffer;
  }
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
    getVertexAttributes( &mesh, xpathContext, vertexXpathResult, docP );

    // Quantize
    U16* qPosA = NULL;
    arrayNew( (void**) &qPosA, sizeof(U16), mesh.posResult.count * 3 );
    assert( qPosA );
    // TODO macro-out 1024 so we tweak the number of bits and all its dependencies with one single parameter.
    const float convX = 1024.0 / fabs( mesh.posResult.max.vec3.x - mesh.posResult.min.vec3.x );
    const float convY = 1024.0 / fabs( mesh.posResult.max.vec3.y - mesh.posResult.min.vec3.y );
    const float convZ = 1024.0 / fabs( mesh.posResult.max.vec3.z - mesh.posResult.min.vec3.z );
    for (int i = 0; i < mesh.posResult.count; ++i) {
      qPosA[ 3 * i     ] = (int) ( (mesh.posResult.u.vec3A[i].x - mesh.posResult.min.vec3.x ) * convX);
      qPosA[ 3 * i + 1 ] = (int) ( (mesh.posResult.u.vec3A[i].y - mesh.posResult.min.vec3.y ) * convY);
      qPosA[ 3 * i + 2 ] = (int) ( (mesh.posResult.u.vec3A[i].z - mesh.posResult.min.vec3.z ) * convZ);
    }
    printf( "\n\nQuantized from %dB to %dB.\n", mesh.posResult.count * sizeof(Vec3), arrayGetElemSz( qPosA ) * arrayGetNElems( qPosA ) );

    // Pack quantized bits together
    U8* packedQPosA =  NULL;
    pack( qPosA, 10, &packedQPosA );
    printf( "Packed from %dB to %dB.\n", arrayGetElemSz( qPosA ) * arrayGetNElems( qPosA ), arrayGetElemSz( packedQPosA ) * arrayGetNElems( packedQPosA ) );
    assert( packedQPosA );

    // Compress quantization results (might make it worse)
    Inflatable *infP = NULL;
    inflatableNew( packedQPosA, &infP );
    printf( "Compressed from %dB to %dB.\n", arrayGetNElems( packedQPosA ) * arrayGetElemSz( packedQPosA ), infP->compressedLen );

    // Now I want to try delta. For this I need to first determine the way positions are connected to each other. In order to do that,

    // Free everything.
    xmlFreeXpathResult( &vertexXpathResult );
    xmlFreeXpathResult( &triangleXpathResult );

  }
  xmlClean( &docP, &xpathContext );

  return 0;
}
