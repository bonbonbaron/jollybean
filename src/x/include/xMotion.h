#ifndef SYS_Motion
#define SYS_Motion
#include "x.h"
#include "jb.h"
#include "interface.h"

typedef struct {
  S8 x;
  S8 y;
} Velocity;

typedef Velocity XMotionMutation;

typedef struct {
  Velocity velocity;
  Rect_* dstRectP;
} XMotionComp;

// Images
XIniSysFuncDef_(Motion);
XGetShareFuncDef_(Motion);
XIniSubcompFuncDef_(Motion);
XPostprocessCompsDef_(Motion);
XProcMsgFuncDef_(Motion);
XRunFuncDef_(Motion);
XClrFuncDef_(Motion);
XPostMutateFuncDef_(Motion);

typedef struct {
  System system;  // This MUST stay the first element in the struct!!
  Map *dstRectMP;  // this points to master's shared map of destination rectangles; we don't delete it since we don't own it.
} XMotion;

extern System *sMotionP;
#endif
