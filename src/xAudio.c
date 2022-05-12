#include "xAudio.h"

// TODO research MIDI format to learn how/if you can compress it the way you do images.
//======================================================
// Initialize Audio's system.
//======================================================
Error xAudioIniSys(System *sP, void *sParamsP) {
	unused_(sParamsP);
  unused_(sP);
  return SUCCESS;
}

//======================================================
// Initialize xAudio's components, which are Images.
//======================================================
Error xAudioIniComp(System *sP, void *compDataP, void *compDataSrcP) {
	if (!sP || !compDataP || !compDataSrcP)
		return E_BAD_ARGS;

  Error e = SUCCESS;
	XAudio *xAudioSysP = (XAudio*) sP;
	XAudioComp *cP = (XAudioComp*) compDataP;
  XAudioCompSrc *imgP = (XAudioCompSrc*) compDataSrcP;

	return e;
}

Error xAudioProcessMessage(System *sP, Message *msgP) {
	unused_(sP);
	unused_(msgP);
	return SUCCESS;
}

XClrFuncDef_(Audio) {
  unused_(sP);
  return SUCCESS;
}

XGetShareFuncDef_(Audio) {
  XAudio *xAudioSysP = (XAudio*) sP;
  Error e = SUCCESS;
  return e;
}

//======================================================
// Audio run function
//======================================================
Error xAudioRun(System *sP) {
	Error e = SUCCESS;

	XAudioComp *cP = (XAudioComp*) sP->cF;
	XAudioComp *cEndP = cP + frayGetFirstInactiveIdx(sP->cF);

  for (; cP < cEndP; cP++) {
    // do something on each element here
  }

	return e;
}

//======================================================
// System definition
//======================================================
#define FLAGS_HERE (0)
X_(Audio, 1, FLAGS_HERE);
