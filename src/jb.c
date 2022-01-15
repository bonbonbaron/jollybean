#include "jb.h"

static System *sPA[] = {
	&sRender
}; 

int main() {
	//BTreeSingleton ts1, ts2, ts3, ts4;
	BTree *treeP;
  Blackboard *bbP;

  Error e = btNew(&root, 0, &treeP);

  if (!e)
    e = bbNew(&bbP, treeP->rootP, &mbBBSeed);

  if (!e)
    btRun(treeP, bbP);

  if (!e)
    e = xIni(sPA, sizeof(sPA) / sizeof(sPA[0]), &grp1);

	while (!e) {
		sRun(&sParent);
  }

  btDel(&treeP);
  bbDel(&bbP);
  return e;
}

// TODO:
// =====
//  \1) test BB by making a tree that uses it.
//  2) move behavior tree stuff to their own directory
//  3) load entity's behavior trees at scene start
//  4) send message to system; at least just start an activity
//  5) receive message from a system
//  6) send check to a system
//  7) be triggered by a positive check in a system
	// In "real life" I'm going to init all the ones used by each entity. What're all the trees used by each entity?
	// Those in the map's array. Like this:
	/*
		typedef struct {
			U8         _elemSz;
			Key        _nKeyValPairs;
			Map        *mapP;       // defaults to NULL to prevent copies 
			KeyValPair  keyValA[];
		} HardCodedMap;
	*/
	// A tree has a source node set and an actual tree implementation. These should be bundled together as singletons.
	// If you want to direclty map to a tree itself, then you have to separate the grove map from the grove map seed.
	// 
	//	grove seed array: event key ==> tree singleton
	// 	grove map:			  event key ==> tree 
	// 
	// So when I code an entity's behaviors, it'll be like this:
	//	HardCodedArray bobGroveHCA = {
		//	{EVENT_COLL_FIRE,     &CollFireTreeSgton}
		//	{EVENT_COLL_ICE,      &CollIceIceSgton}
		//	{EVENT_COLL_SPIKE,    &CollSpikeTreeSgton}
		//	{EVENT_COLL_DOOR,     &CollDoorTreeSgton}
		//	{EVENT_COLL_CTRL_UP,  &WalkUpTreeSgton}
	//	};
	//
	//	Because I like to easily write things with only one type declarator, I'll keep all the grove definitions in one file.
	//
	// NEXT PROBLEM: initializing BBs
	// ==============================
	//	1) providing multiple tree stat arrays and condiiton maps
	//	2) checking whether BBs implement all keys required by trees
	//	3) initialize
	//
	//	(1)> Change bbNew() definition to have HardCodedArray parameter. 
	//			 Iterate through grove hca, performing the counts for each one for both condition map and tree stat array map.
	//			 Change bbDel() to account for this.
	//	(2)> There's no clean, jolly way to do this. You're going to have to return an error from callbacks finding unset keys.
	//
