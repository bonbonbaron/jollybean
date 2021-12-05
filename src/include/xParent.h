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
	ReactionCallback cb;
  void *paramsP;
	Message msg;  // must consume message that triggered this reaction before JB clears a child's outbox
} Reaction;

//REACT_TO_(t1) WITH_(action) USING_PARAMS_(params) AT_PRIORITY_(5);
/* Entities need to have reactions hard-coded. This requires:
 *		callback
 *		hard-coded params
 *		priority
 */

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
