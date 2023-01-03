#ifndef SYS_Anim
#define SYS_Anim
#include "jb.h"

typedef struct {
  U32 x, y, w, h;  // these are the source rect coordinates
  U32 duration;
} AnimFrame;

typedef struct {
  U8  nFrames;
  Bln flip;
  Bln repeat;       
  Bln pingPong;       
  AnimFrame *frameA;
} AnimStrip;

typedef struct {
} XAnimCompSrc; 

typedef Animation XAnimComp;

// Images
Error xAnimIniS(System *sP, void *sParamsP);
Error xAnimProcessMessage(System *sP, Message *msgP);

typedef struct {
  System system;
} XAnim;

extern System *sAnimP;
#endif
