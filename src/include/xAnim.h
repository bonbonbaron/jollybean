#ifndef SYS_Anim
#define SYS_Anim
#include "botox.h"
#include "yoyoInterface.h"

typedef struct {
} XAnimCompSrc; 

typedef struct {
  U8     timeLeft; 
  U8     currIdx;
  U8     nIndices;
  Bln    repeat;
  U8    *timeA;     
  Rect_ *srcRectA;   
  Rect_ *shareRectP;
} XAnimComp;

typedef struct {
  U8     timeLeft; 
  U8     currIdx;
  U8     nIndices;
  Bln    repeat;
  U8    *timeA;     
  Rect_ *srcRectA;   
} XAnimCompData;
//
// Images
Error xAnimIniS(System *sP, void *sParamsP);
Error xAnimProcessMessage(System *sP, Message *msgP);

typedef struct {
  System system;
} XAnim;

extern System *sAnimP;
#endif
