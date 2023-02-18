#ifndef X_GO
#define X_GO
#include "biome.h"

#define GO_ (2)
typedef Activity XGoComp;
typedef struct {
	System       system;
	Map         *hivemindMP; // maps triggers to arrays of 
} XGo;

XClrFuncDef_(Go);
XIniCompElemFuncDef_(Go);
XPostprocessCompsDef_(Go);

extern XGo xGo;
extern System *sGoP;
#endif
