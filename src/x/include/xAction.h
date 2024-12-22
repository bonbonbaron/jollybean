#ifndef X_ACTION
#define X_ACTION
#include "x.h"

#define ACTION_ (2)
#define PERSONALITY (0x40)
#define BLACKBOARD (0x80)

void hivemindDel(Map **hivemindMPP);
void activityMPMPDel(Map **activityMPMPP);
XPostMutateFuncDef_(Action);

typedef enum { SET_TARGET = 1 } ActionMailCmdEnum;
// Think params through really carefully.
/* msgP can tell us WHOM to act on. 
 * But we need a way to write potentially mutlipel message to our mailbox.
 * You know what, though? We might considerable time by just passing in the
 * whole action component to the function rather than each painful part one by one. */
struct Activity;
typedef void (*Action)(Entity entity, struct Activity *activityP, Message* mailboxF );
#define ActionFuncDef_( name_ ) void name_( Entity entity, Activity* activityP, Message* mailboxF )
#define assertAction_ \
  assert ( entity ); \
  assert ( activityP ); \
  assert ( activityP->quirkP ); \
  assert ( activityP->quirkP->actionU ); \
  assert ( activityP->quirkP->trigger ); \
  assert ( activityP->bbMP );

#ifndef NDEBUG
// bbGet guards against casting the void pointer to the wrong pointer type in debug mode.
#define bbGet( type_, name_, key_ ) \
  /* Make sure the blackboard is a map of POINTERSS */ \
  assert( arrayGetElemSz( activityP->bbMP->mapA ) == sizeof( type_* ) ); \
  /* Blackboards are composed of void pointers.
   * So a mapGet() will return a pointer to a void pointer. */ \
  type_** name_##PP = mapGet( activityP->bbMP, key_ ); \
  assert (name_##PP); \
  type_* name_##P = *( (type_**) name_##PP );

#else
// Without debug mode on, we just hold on tight and go full blast.
#define bbGet( type_, name_, key_ ) \
  type_* name_##P = *( (type_**) mapGet( activity->bbMP, key_ ) );

#endif

typedef struct {
  Key    trigger;
  U8     priority;
  Action actionU;
} Quirk;

typedef struct {
  U32 nQuirks;
  Quirk **quirkPA;
} Personality;   // Gene-level data we expect to receive

typedef struct {
  Entity entity;
  Personality *personalityP;
} EntityPersonalityPair;

typedef struct {
  Entity entity;
  Map *bbMP;
} EntityBlackboardPair;

// Union of quirk, blackboard, and tree status.
typedef struct Activity {
  Entity      tgtEntity;     // Thing to perform action on (0 if action isn't transitive)
  Generic     amount;        // Naming this "amount" because we always assume action is quantitative.
  Quirk      *quirkP;        // trigger-action pair
  Map        *bbMP;          // blackboard 
} Activity;  

typedef Quirk* XActionMutation; 

typedef Activity XActionComp;

typedef struct {
	System                 system;
	Map                   *hivemindMP; // maps triggers to arrays of 
  Key                    nDistinctHivemindTriggers;
  U32                   *histoHivemindTriggerA;
  EntityPersonalityPair *entityPersonalityPairF;
  EntityBlackboardPair  *entityBlackboardPairF;
} XAction;
XClrFuncDef_(Action);
XIniSubcompFuncDef_(Action);
XPostprocessCompsDef_(Action);

extern System *sActionP;
#endif
