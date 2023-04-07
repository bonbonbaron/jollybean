#ifndef SYS_A
#define SYS_A
#include "x.h"

typedef struct {
  char a;
  int b;
  short c;
  double d;
} XAComp;

typedef struct {
  short cMutation;
} XAMutation;

// Images
XIniSysFuncDef_(A);
XGetShareFuncDef_(A);
XIniSubcompFuncDef_(A);
XPostprocessCompsDef_(A);
XProcMsgFuncDef_(A);
XRunFuncDef_(A);
XClrFuncDef_(A);
XPostMutateFuncDef_(REPLACME);

typedef struct {
  System system;
} XA;

extern System *sAP;
#endif
