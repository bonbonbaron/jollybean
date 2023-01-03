#ifndef X_GO
#define X_GO
#include "biome.h"

#define GO_ (2)
typedef Activity XGoComp;
typedef struct {
	System       system;
	Map         *hivemindMP; // maps triggers to arrays of 
} XGo;

typedef U32 XGoCompSrc;  // just to make interface happy

XClrFuncDef_(Go);
XIniCompFuncDef_(Go);

extern XGo xGo;
extern System *sGoP;
#endif
