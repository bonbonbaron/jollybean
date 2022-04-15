#ifndef SYS_Audio
#define SYS_Audio
#include "botox.h"

typedef struct {
} XAudioCompSrc; 

typedef struct {
} XAudioComp;

// Images
Error xAudioIniS(System *sP, void *sParamsP);
Error xAudioProcessMessage(System *sP, Message *msgP);

typedef struct {
  System system;
} XAudio;

extern System *sAudioP;
#endif
