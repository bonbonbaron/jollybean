#include "ecs.h"

/**************************/
/***** Parent systems *****/
/**************************/
// Reactions
ENUM_KEYS_(TICK, REACT) JBActivities; 

typedef enum {
	COMPLETE,
	RUNNING
	// FAILED  // can't conceive any use for this as I'm avoiding behavior trees
} ReactionStatus;

typedef ReactionStatus (*ReactionCallback)(Message *msgP, void *paramsP);

typedef struct {
	Key trigger;
	U8  priority;
	ReactionCallback cb;  // This can return anything. I'm not so sure that it must be pinged anymore. 
  U32 condCur;  // conditions currently set to TRUE; these can be anything
  U32 condExp;  // conditions expected before calling cb
  void *paramsP;
	Message msg;  // must copy message triggered by this reaction before JB clears it from outbox
} Reaction;

// Activities output unconditional signals and/or conditional ones. Who sets the conditional ones?

typedef union {
	System s;
	Reaction r;
} XParentC;

typedef struct {
	Entity nSystems;
	GenomeGrp *genomeGroupP;
	System **sysPA;
} SParentIniSParams;

Error xParentIniS();
Error xParentIniC();
Error xIni(System **sPA, U16 nSystems, GenomeGrp *genomeGroupP);
Error xRun();
Error xParentDeliverMessage(Message *msgP);

extern System sParent;
