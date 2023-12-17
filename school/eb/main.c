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


void extractVec3Array( Vec3** tgtArray, Vec3* minVecP, Vec3* maxVecP, int count, char* arrayString ) {
  assert( tgtArray );
  assert( count );
  assert( arrayString );

  minVecP->x = FLT_MAX;
  minVecP->y = FLT_MAX;
  minVecP->z = FLT_MAX;
  maxVecP->x = FLT_MIN;
  maxVecP->y = FLT_MIN;
  maxVecP->z = FLT_MIN;

  *tgtArray = malloc( sizeof( Vec3 ) * count );
  assert( *tgtArray );

  // Extract string into array here.
  for (int j = 0, sIdx = 0; sIdx >= 0 && j < count; ++j) {
    sscanf( &arrayString[sIdx], "%f %f %f", &(*tgtArray)[j].x, &(*tgtArray)[j].y, &(*tgtArray)[j].z );
    sIdx = _getNextNumberIdx( arrayString, sIdx, 3 );

    // Check min
    if ( (*tgtArray)[j].x < minVecP->x ) {
      minVecP->x = (*tgtArray)[j].x;
    }
    if ( (*tgtArray)[j].y < minVecP->y ) {
      minVecP->y = (*tgtArray)[j].y;
    }
    if ( (*tgtArray)[j].z < minVecP->z ) {
      minVecP->z = (*tgtArray)[j].z;
    }
    // Check max
    if ( (*tgtArray)[j].x > maxVecP->x ) {
      maxVecP->x = (*tgtArray)[j].x;
    }
    if ( (*tgtArray)[j].y > maxVecP->y ) {
      maxVecP->y = (*tgtArray)[j].y;
    }
    if ( (*tgtArray)[j].z > maxVecP->z ) {
      maxVecP->z = (*tgtArray)[j].z;
    }
  }
  // xmlFree( mesh.posResult.valueString );  // TODO be sure to do this later
}

void extractVec2Array( Vec2** tgtArray, Vec2* minVecP, Vec2* maxVecP, int count, char* arrayString ) {
  assert( tgtArray );
  assert( count );
  assert( arrayString );

  minVecP->s = FLT_MAX;
  minVecP->t = FLT_MAX;
  maxVecP->s = FLT_MIN;
  maxVecP->t = FLT_MIN;

  *tgtArray = malloc( sizeof( Vec2 ) * count );
  assert( *tgtArray );

  // Extract string into array here.
  for (int j = 0, sIdx = 0; sIdx >= 0 && j < count; ++j) {
    sscanf( &arrayString[sIdx], "%f %f", &(*tgtArray)[j].s, &(*tgtArray)[j].t );
    sIdx = _getNextNumberIdx( arrayString, sIdx, 2 );

    // Check min
    if ( (*tgtArray)[j].s < minVecP->s ) {
      minVecP->s = (*tgtArray)[j].s;
    }
    if ( (*tgtArray)[j].t < minVecP->t ) {
      minVecP->t = (*tgtArray)[j].t;
    }
    // Check max
    if ( (*tgtArray)[j].s > maxVecP->s ) {
      maxVecP->s = (*tgtArray)[j].s;
    }
    if ( (*tgtArray)[j].t > maxVecP->t ) {
      maxVecP->t = (*tgtArray)[j].t;
    }
  }
  // xmlFree( mesh.posResult.valueString );  // TODO be sure to do this later
}

typedef struct {
  int count;         // number of space-delimited elements in this set
  union {
    int num; 
    float* floatA;
    Vec2* vec2A;
    Vec3* vec3A;
  } u;
  union {
    int color;
    Vec2 texel;
    Vec3 position, normal;
  } min;
  union {
    int color;
    Vec2 texel;
    Vec3 position, normal;
  } max;
} XmlResult;


typedef struct {
  XmlResult posResult, nmlResult, clrResult, texResult, triResult;
} Mesh;


/* To separate concerns, I'm only going to have this know about XML.
 * Then anybody else can come in and extract an array of something. 
 * It'll return a simple data structure containiing a string and count. */

/* It also needs to be able to return a node. That way we can extract any
 * node we want and then ask it to return any property from it we want. */
/* Rather than factoring this code out, we hard-code it to our specific use-case  
   to avoid re-iterating over any linked lists. */
void getVertexAttributes( Mesh* meshP, xmlXPathObjectPtr xpathResultP ) {
  assert( meshP && xpathResultP );
  XmlResult* resultP;
  xmlNodePtr propChildP;
  char* posArrayString = NULL;
  char* nmlArrayString = NULL;
  char* texArrayString = NULL;
  char* clrArrayString = NULL;
  int numElemsPerUnit = 1;
  forEachNode_( xpathResultP, top ) {
    resultP = NULL;  // reset the result pointer so we don't falsely populate the count later on.
                     // For each property of this node...
    forEachProperty_( topNodePP, parent ) {
      if ( parentPropertyNodeP->type == XML_ATTRIBUTE_NODE ) {
        // TODO rather than clone strings, just point at their values.
        //      Then keep the xml objects alive until you're finished compressing each mesh.
        printf("prop node name: %s\n", parentPropertyNodeP->name);
        // If this is an identifier node, determine which mesh property it is.
        if ( !(strcmp( (char*) parentPropertyNodeP->name, "id" ) ) ) {
          propChildP = parentPropertyNodeP->children;
          if ( propChildP && propChildP->type == XML_TEXT_NODE ) {
            // ---------
            // POSITIONS
            // ---------
            if( strstr( (char*) propChildP->content, "positions-array" ) > 0 ) {
              numElemsPerUnit = 3;
              resultP = &meshP->posResult;
              // Extract the positions.
              forEachChild_( topNodePP, curr ) {
                if (currChildNodeP->type == XML_TEXT_NODE) {
                  posArrayString = (char*) currChildNodeP->content;
                  printf("\tpositions child node val: %s\n\n", currChildNodeP->content );
                  break;
                }
              }
            }
            // -------
            // NORMALS
            // -------
            else if( strstr( (char*) propChildP->content, "normals-array" ) > 0 ) {
              numElemsPerUnit = 3;
              resultP = &meshP->nmlResult;
              // Extract the normals.
              forEachChild_( topNodePP, curr ) {
                if (currChildNodeP->type == XML_TEXT_NODE) {
                  nmlArrayString = (char*) currChildNodeP->content;
                  printf("\tnormals child node val: %s\n\n", currChildNodeP->content );
                  break;
                }
              }
            }
            // ======
            // COLORS
            // ======
            // TODO can there be multiple color sources that split up for varying color formats too?
            // For now, only support one set of colors per geometry.
            else if( strstr( (char*) propChildP->content, "mesh-colors" ) > 0 ) {
              numElemsPerUnit = 3;  // TODO programmatically determine number of color channels
              resultP = &meshP->texResult;
              // Extract the normals.
              forEachChild_( topNodePP, curr ) {
                if (currChildNodeP->type == XML_TEXT_NODE) {
                  clrArrayString = (char*) currChildNodeP->content;
                  printf("\ttexels child node val: %s\n\n", currChildNodeP->content );
                  break;
                }
              }
            }
            // ======
            // TEXELS
            // ======
            else if( strstr( (char*) propChildP->content, "mesh-map" ) > 0 ) {
              numElemsPerUnit = 2;
              resultP = &meshP->texResult;
              // Extract the normals.
              forEachChild_( topNodePP, curr ) {
                if (currChildNodeP->type == XML_TEXT_NODE) {
                  texArrayString = (char*) currChildNodeP->content;
                  printf("\ttexels child node val: %s\n\n", currChildNodeP->content );
                  break;
                }
              }
            }
          }
        }
        // Getting the count (number of elements) for the current attribute assumes we got the ID first.
        else if ( !(strcmp( (char*) parentPropertyNodeP->name, "count") ) ) {
          propChildP = parentPropertyNodeP->children;
          if ( propChildP && propChildP->type == XML_TEXT_NODE ) {
            if ( resultP ) {
              printf("\tCHILD node val: %s\n\n", propChildP->content );
              resultP->count = atoi( (char*) propChildP->content ) / numElemsPerUnit;  // e.g. vec3s have 3 elements per vector 
            }
          }
        }
      }  // if parent property is an attribute node
    }  // for each property

    // Extract position array of vectors
    if ( posArrayString && meshP->posResult.count ) {
      extractVec3Array( &meshP->posResult.u.vec3A, &meshP->posResult.min.position, &meshP->posResult.max.position, meshP->posResult.count, posArrayString );
    }
    // Extract normal array of vectors
    if ( nmlArrayString && meshP->nmlResult.count ) {
      extractVec3Array( &meshP->nmlResult.u.vec3A, &meshP->nmlResult.min.normal, &meshP->nmlResult.max.normal, meshP->nmlResult.count, nmlArrayString );
    }
    // Extract color array of vectors
    if ( clrArrayString && meshP->clrResult.count ) {
      extractVec3Array( &meshP->clrResult.u.vec3A, &meshP->clrResult.min.normal, &meshP->clrResult.max.normal, meshP->clrResult.count, clrArrayString );
    }
    // Extract texel array of vectors
    if ( texArrayString && meshP->texResult.count ) {
      extractVec2Array( &meshP->texResult.u.vec2A, &meshP->texResult.min.texel, &meshP->texResult.max.texel, meshP->texResult.count, texArrayString );
    }
  }
}

// =========
// TRIANGLES
// =========
void getTriangles( Mesh* meshP, xmlXPathObjectPtr xpathResultP ) {
  assert( meshP && xpathResultP );
  char* triArrayString;
  xmlNodePtr propChildP;
  int numElemsPerUnit;  // TODO determine this programmatically
  forEachNode_( xpathResultP, parent ) {
    forEachProperty_( parentNodePP, parent ) {
      if ( parentPropertyNodeP->type == XML_ATTRIBUTE_NODE ) {
        // TODO rather than clone strings, just point at their values.
        //      Then keep the xml objects alive until you're finished compressing each mesh.
        printf("prop node name: %s\n", parentPropertyNodeP->name);
        // If this is an identifier node, determine which mesh property it is.
        if ( !(strcmp( (char*) parentPropertyNodeP->name, "id" ) ) ) {
          propChildP = parentPropertyNodeP->children;
        }
      }
    }
    if( strstr( (char*) propChildP->content, "mesh-vertices" ) > 0 ) {
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
  static const char NAMESPACE_ALIAS[] = "mb";
  // TODO you're going to have to iterate through each <geometry> of <library_geometries>.
  /* You'll perform relative XPath queries in each <geometry> like this:
       // Reuse context with the new node.
       xpathContext->node = parent_node;

       // Perform subsequent xpath query.
       result = xmlXPathEvalExpression(BAD_CAST "xpath_relative_to_parent", xpathContext);
       if(result == NULL) {
         printf("Error: unable to evaluate xpath expression\n");
         return;
       }
  */
  static const xmlChar* GEOMETRY_DATA_XPATH = ( xmlChar * ) "/mb:COLLADA/mb:library_geometries/mb:geometry";
  static const xmlChar* VERTEX_DATA_XPATH = ( xmlChar * ) "/mb:mesh/mb:source/mb:float_array";
  static const xmlChar* TRIANGLE_DATA_XPATH = ( xmlChar * ) "/mb:mesh/mb:triangles";

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
    xpathContext->node = geometryXpathResult->nodesetval->nodeTab[0];
    xmlXPathObjectPtr vertexXpathResult   = xmlGetNodes ( docP, xpathContext, VERTEX_DATA_XPATH );
    xmlXPathObjectPtr triangleXpathResult = xmlGetNodes ( docP, xpathContext, TRIANGLE_DATA_XPATH );

    if ( !vertexXpathResult || !triangleXpathResult ) {
      printf( "Incomplete 3D mesh information. Quitting...\n");
      return 1;
    }

    getVertexAttributes( &mesh, vertexXpathResult );

    // Quantize
    U16* qPosA = NULL;
    arrayNew( (void**) &qPosA, sizeof(U16), mesh.posResult.count * 3 );
    assert( qPosA );
    // TODO macro-out 1024 so we tweak the number of bits and all its dependencies with one single parameter.
    const float convX = 1024.0 / fabs( mesh.posResult.max.position.x - mesh.posResult.min.position.x );
    const float convY = 1024.0 / fabs( mesh.posResult.max.position.y - mesh.posResult.min.position.y );
    const float convZ = 1024.0 / fabs( mesh.posResult.max.position.z - mesh.posResult.min.position.z );
    for (int i = 0; i < mesh.posResult.count; ++i) {
      qPosA[ 3 * i     ] = (int) ( (mesh.posResult.u.vec3A[i].x - mesh.posResult.min.position.x ) * convX);
      qPosA[ 3 * i + 1 ] = (int) ( (mesh.posResult.u.vec3A[i].y - mesh.posResult.min.position.y ) * convY);
      qPosA[ 3 * i + 2 ] = (int) ( (mesh.posResult.u.vec3A[i].z - mesh.posResult.min.position.z ) * convZ);
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
