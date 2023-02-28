#ifndef SYS_CONTROL
#define SYS_CONTROL
#include "jb.h"

typedef U32 XControlComp;

// TODO maybe delete these when you have a solid design
#define CTRL_LEFT_   1 << 0
#define CTRL_RIGHT_  1 << 1
#define CTRL_UP_     1 << 2
#define CTRL_DOWN_   1 << 3
#define CTRL_A_      1 << 4
#define CTRL_B_      1 << 5
#define CTRL_START_  1 << 6
#define CTRL_SELECT_ 1 << 7

#define KD_(name_) name_##KEYDOWN_
typedef enum {KD_(LEFT_), KD_(RIGHT_), KD_(UP_), KD_(DOWN_), KD_(A_), KD_(B_), KD_(START_), KD_(SELECT_)} XControlKeyEvent;

// I know, I know... But it has to be a struct because of the macro.
typedef struct {
  System system;  
  Entity focus;   // this is the entity who has control's focus right now;
} XControl; 

typedef struct {} XControlCompSrc;

Error xControlRun(System *sP);
Error xControlProcessMessage(System *sP, Message *msgP);
Error xControlIniSys();
Error xControlClr(System *sP);
Error xControlProcessMessage(System *sP, Message *msgP);
XIniSubcompFuncDef_(Control);
XGetShareFuncDef_(Control);

extern XControl xControl;
extern System *sControlP;
#endif
