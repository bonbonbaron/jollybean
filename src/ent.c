#include "jb.h"

//TODO: design functions for entity:
//	if you go with an array of genes, then you need sizeof(). But you also need a type to tell which system should take it.
//
//	typedef struct {
//		U8 geneType;
//		U8 geneSize;
//	} GeneHeader;
//
//	typedef struct {
//		void *geneSrcP;  // this has to implement a GeneHeader.
//	} Gene;
//
//	This way you can have an easily initializable array of Gene pointers. Of course, then you need a null-terminator. But before we blindly build an array of these, we need to decide how they'll interplay with personalities.
//	
//	The reason we're separating these is so that genes are reusable with multiple personalities and vice versa. 
//	genes = {
//		(void*) &velWalkNormal,
//		(void*) &imgOldMan,
//		(void*) &soundFart,
//		(void*) &
//		...
//
//	Here's a problem: Those functions in the personality still don't know their arguments. So instead of genes, you should focus on argument structs. For multiple possible arguments to the same function, I'm not sure what to do yet.
