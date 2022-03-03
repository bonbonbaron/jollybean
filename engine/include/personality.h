#ifndef PERSONALITY_
#define PERSONALITY_
#include "jb.h"

// Personality is a hard-coded map from events to tree pointers. However, these tree pointers come from singletons. That means a function needs to see if the singleton is initialized, and if it isn't, initialize it. Then after it's init'd, store its non-null tree pointer in the runtime map with the trigger. A specialized function needs to do this for Personality.

typedef struct {
	U8 key;
	BTreeSingleton *treeSgtonP;
} TriggerPair;

// in a hard-coded ARRAY, store the key-bTreeSgton pairs as a PersonalitySeed (you gotta typedef it to keep key together)
// for each member in the above array... 
	// check if bTreeSgton.treeP is NULL
	// if it is, call btNew() on it with its root source
	// e = mapSet(bTreeSgton.event, (void*) bTreeSgton.treeP);

#endif
