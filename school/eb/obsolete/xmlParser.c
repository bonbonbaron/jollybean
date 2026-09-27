#include "xmlParser.h"

/* To separate concerns, I'm only going to have this know about XML.
 * Then anybody else can come in and extract an array of something. 
 * It'll return a simple data structure containiing a string and count. */

/* It also needs to be able to return a node. That way we can extract any
 * node we want and then ask it to return any property from it we want. */

/* Procedure for using this API is the following:
 *
 *  1) Parse your XML file
 *  2) Find a set of nodes
 *  3) Search that set of nodes for particular values or properties you want
 */

// This'll also register the namespace.
xmlXPathContextPtr xmlGetNamespaceContext( xmlDocPtr doc, const char* nsAlias ) {
  xmlXPathContextPtr context;
  xmlXPathObjectPtr result;
  context = xmlXPathNewContext ( doc );
  if ( context == NULL ) {
    printf ( "Error in xmlXPathNewContext\n" );
    return NULL;
  }
  // First get the namespace if it exists.
  result = xmlXPathEvalExpression ( BAD_CAST "namespace-uri(/*)", context );
  // I'm noticing it's actually getvertexting the namespace in the stringval.
  if ( result != NULL && result->stringval != NULL ) {
    // printf( "aliasing namespace %s with %s\n", result->stringval, nsAlias );
    int registryResult = xmlXPathRegisterNs ( context, BAD_CAST nsAlias, BAD_CAST result->stringval );
    if ( registryResult == 0 ) {
    } else {
      printf( "Registering namespace failed.\n");
    }
    xmlXPathFreeObject ( result );
  }
  return context;
}

//  Parse XML file.
xmlDocPtr xmlParse ( char *docname ) {
  xmlDocPtr doc = xmlParseFile ( docname );
  if ( !doc ) {
    fprintf ( stderr, "Failed to parse %s.\n", docname );
    return NULL;
  }
  return doc;
}

// Find everything matching the given xpath for the specified namespace.
xmlXPathObjectPtr xmlGetNodes ( xmlDocPtr doc, xmlXPathContextPtr context, const xmlChar * xpath ) {
  xmlXPathObjectPtr matchingNodes = xmlXPathEvalExpression ( xpath, context );
  if ( matchingNodes == NULL ) {
    fprintf ( stderr, "Error in xmlXPathEvalExpression\n" );
    return NULL;
  }
  if ( xmlXPathNodeSetIsEmpty ( matchingNodes->nodesetval ) ) {
    xmlXPathFreeObject ( matchingNodes );
    fprintf ( stderr, "Couldn't find any nodes matching the following XPath:\n\t%s\n\n", xpath );
    return NULL;
  }
  return matchingNodes;
}

void xmlFreeXpathResult( xmlXPathObjectPtr* xpathResultPP ) {
  if (xpathResultPP ) { 
    xmlXPathFreeObject ( *xpathResultPP );
    *xpathResultPP  = NULL;
  }
}


void xmlClean( xmlDocPtr* xmlDocPP, xmlXPathContextPtr* contextPP ) {
  if ( contextPP ) {
    xmlXPathFreeContext ( *contextPP );
    *contextPP = NULL;
  }
  if ( xmlDocPP ) {
    xmlFreeDoc( *xmlDocPP );
    *xmlDocPP = NULL;
  }
  xmlCleanupParser ();
}
