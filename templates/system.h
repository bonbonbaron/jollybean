#ifndef SYS_REPLACEME
#define SYS_REPLACEME
#include "x.h"
#include "jb.h"

typedef struct {
} XREPLACEMEComp;

typedef struct {
} XREPLACEMEMutation;

// Images
XIniSysFuncDef_(REPLACEME);
XGetShareFuncDef_(REPLACEME);
XIniSubcompFuncDef_(REPLACEME);
XPostprocessCompsDef_(REPLACEME);
XProcMsgFuncDef_(REPLACEME);
XRunFuncDef_(REPLACEME);
XClrFuncDef_(REPLACEME);
XPostMutateFuncDef_(REPLACEME);

typedef struct {
  System system;  // This MUST stay the first element in the struct!!
} XREPLACEME;

extern System *sREPLACEMEP;
#endif
