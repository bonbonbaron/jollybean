#ifndef X_SYSTEM_
#define X_SYSTEM_
#include "jb.h"

/**************************/
/***** System systems *****/
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

typedef System XSystemC;

typedef struct {
	Entity nSystems;
	Biome *biomeP;
	System **sysPA;
} SSystemIniSParams;

Error xSystemIniS();
Error xSystemIniC();
Error xIni(System **sPA, U16 nSystems, Biome *genomeGroupP);
Error xRun();
Error xSysDeliverMessage(Message *msgP);

extern System sSystem;
#endif
