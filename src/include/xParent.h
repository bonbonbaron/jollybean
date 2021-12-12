#include "ecs.h"

/**************************/
/***** Parent systems *****/
/**************************/
// Reactions
ENUM_KEYS_(TICK, REACT) JBActivities; 

typedef struct {
  Key sysID;
  Key activityID;
  Key key;
} XCmd;

typedef struct {
  U8 nCmds;
  XCmd *xCmdA;
} XCmdSet;

typedef struct {
  U8 currRoot;
  Map *rootMP;
} EntityState;
typedef struct {
  Key id;
  U8 type;
  Entity otherEntity;
} Event;
  
// Activities output unconditional signals and/or conditional ones. Who sets the conditional ones?

typedef System XParentC;

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
