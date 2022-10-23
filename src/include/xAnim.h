#ifndef SYS_Anim
#define SYS_Anim
#include "jb.h"

typedef struct {
} XAnimCompSrc; 

typedef struct {
  U8     timeLeft; 
  U8     currIdx;
  U8     nIndices;
  Bln    repeat;
  U8    *timeA;     
  Rect_ *srcRectA;    // This is the array of animation rectangles. We draw FROM this part of the source image.
  Rect_ *srcRectP;  // Pointer to rectangle shared across multiple systems. We draw TO this part of the dest image.
} XAnimComp;

typedef struct {
  U8     timeLeft; 
  U8     currIdx;
  U8     nIndices;
  Bln    repeat;
  U8    *timeA;     
  Rect_ *srcRectA;   
} XAnimCompData;

// Images
Error xAnimIniS(System *sP, void *sParamsP);
Error xAnimProcessMessage(System *sP, Message *msgP);

typedef struct {
  System system;
} XAnim;

extern System *sAnimP;
#endif
