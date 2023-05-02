#ifndef SYS_A
#define SYS_A
#include "x.h"

typedef struct {
  int a;
  short b;
  char c;
  double d;
  int *intP;
} XAComp;

typedef struct {
  short bb;
  char cc;
} XAMutation;

// Images
XIniSysFuncDef_(A);
XGetShareFuncDef_(A);
XIniSubcompFuncDef_(A);
XPostprocessCompsDef_(A);
XProcMsgFuncDef_(A);
XRunFuncDef_(A);
XClrFuncDef_(A);
XPostMutateFuncDef_(A);

typedef struct {
  System system;
  U32 someSystemwideU32;
  Map *aMP;
  Map *dMP;
  Map *sharedIntMP;
  Entity *entityF;
} XA;

extern System *sAP;
#endif
