#ifndef XML_PARSER_H
#define XML_PARSER_H
#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/xpath.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>

// For each node in this set of result nodes matching an xpath search...
// The below table is an *array*, so we can increment like this.
#define forEachNode_( xpathResultP_, id_ ) \
xmlNodePtr* id_##NodePP = xpathResultP_->nodesetval->nodeTab;   \
xmlNodePtr* node##id_##EndPP = id_##NodePP + xpathResultP_->nodesetval->nodeNr; \
for ( ; id_##NodePP < node##id_##EndPP; ++id_##NodePP )

#define forEachProperty_( nodePP_, id_ )  \
for ( xmlAttrPtr id_##PropertyNodeP = (*nodePP_)->properties; id_##PropertyNodeP; id_##PropertyNodeP = (id_##PropertyNodeP)->next )

#define forEachChild_( nodePP_, id_ )  \
for ( xmlNodePtr id_##ChildNodeP = (*nodePP_)->children; id_##ChildNodeP; id_##ChildNodeP = id_##ChildNodeP->next )


// #include "array.h"

/* To separate concerns, I'm only going to have this know about XML.
 * Then anybody else can come in and extract an array of something. 
 * It'll return a simple data structure containiing a string and count. */

/* It also needs to be able to return a node. That way we can extract any
 * node we want and then ask it to return any property from it we want. */
// extern XMLPUBFUN const xmlChar * XMLCALL xmlXPathNsLookup (xmlXPathContextPtr ctxt, const xmlChar *prefix);  // TODO how do we fix this annoying compiler warning?

// These functions are listed in the order you should call them.
xmlDocPtr          xmlParse ( char *docname );
xmlXPathContextPtr xmlGetNamespaceContext( xmlDocPtr doc, const char* nsAlias );
xmlXPathObjectPtr  xmlGetNodes ( xmlDocPtr doc, xmlXPathContextPtr context, const xmlChar * xpath );

void               xmlFreeXpathResult( xmlXPathObjectPtr* xpathResultPP );
void               xmlClean( xmlDocPtr* xmlDocPP, xmlXPathContextPtr* contextPP );
#endif
