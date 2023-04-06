#ifndef SYS_REPLACEME
#define SYS_REPLACEME
#include "x.h"

typedef struct {
} XREPLACEMEComp;

// Images
XIniSysFuncDef_(REPLACEME);
XGetShareFuncDef_(REPLACEME);
XIniSubcompFuncDef_(REPLACEME);
XPostprocessCompsDef_(REPLACEME);
XProcMsgFuncDef_(REPLACEME);
XRunFuncDef_(REPLACEME);
XClrFuncDef_(REPLACEME);
XPostMutateFuncDef_(REPLACME);

typedef struct {
  System system;
} XREPLACEME;

extern System *sREPLACEMEP;
#endif
