#include "jb.h"

/**************************/
/***** Parent systems *****/
/**************************/
// Reactions
enumKeys_(TICK, REACT) JBActivities; 

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
	Biome *biomeP;
	System **sysPA;
} SParentIniSParams;

Error xParentIniS();
Error xParentIniC();
Error xIni(System **sPA, U16 nSystems, Biome *genomeGroupP);
Error xRun();
Error xParentDeliverMessage(Message *msgP);

extern System sParent;
