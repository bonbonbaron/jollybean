#ifndef XMAIN_
#define XMAIN_
#include "xGo.h"

#define MASTER_ (1)
typedef enum {QUIT_ALL, SWITCH_BIOME} MasterMsgType;
typedef System* XMasterComp;  // master component is a pointer to a child system
typedef struct {} XMasterCompSrc;

typedef struct {
	U8 nXSystemsMax;
	U8 nXSystems;
	Biome *biomeP;
	System **xSysPA;  // This array contains all the systems this app will ever need.
  System *behaviorSysP;
} XMasterIniSysPrms;

typedef struct {
	System system;
	Map *sharedMMP;         // map of maps of shared "components"
	Biome *biomeP;
	Window_ *windowP;
	Renderer_ *rendererP;
} XMaster;

extern XMaster xMaster;
extern System *sMasterP;

XPostprocessCompsDefUnused_(Master);

Error xIni(System **sPA, U16 nSystems, U8 nSystemsMax, Biome *biomeP);
Error xMasterIniSys(System *sP, void *sParamsP);
XIniSubcompFuncDef_(Master);
Error xMasterIni(XMaster *xMasterSysP, System *goSysP, System **sPA, U16 nXSystems, Key nXSystemsMax, Biome *biomeP);
Error xMasterGetShare(System *sP, Map *shareMMP);
Error xMasterProcessMessage(System *sP, Message *msgP);
Error xMasterClr(System *sP);
Error xMasterRun(System *sP);
#endif
