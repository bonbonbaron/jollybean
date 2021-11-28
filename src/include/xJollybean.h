#include "xRender.h"
#include "xControl.h"
#include "SDL.h"
#include "genes.h"
#include "ecs.h"
#include <assert.h>

/***** Reactions *****/
typedef enum {
	COMPLETE,
	RUNNING
	// FAILED  // can't conceive any use for this as I'm avoiding behavior trees
} ReactionStatus;

typedef ReactionStatus (*ReactionCallback)(Message *msgP, void *paramsP);

typedef struct {
	Key trigger;
	U8  priority;
	ReactionCallback cb;
  void *paramsP;
	Message msg;  // must consume message that triggered this reaction before JB clears a child's outbox
} Reaction;

/***** Jollybean *****/
typedef struct {
	union {
		System s;
		Reaction r;
	} item;
} XJollybeanC;

ENUM_KEYS_(JB_RUN_CHILDREN, JB_REACT) JBActivities; 
Error xJollybeanExe(Activity *aP);
Error xJollybeanIniC(XHeader *xhP);
Error xJollybeanIniS();
Error xJollybeanReact(Activity *aP);
Error xJollybeanRunChildren(Activity *aP);
Error xJollybeanBehave(Activity *aP);
extern System sJollybean;
