#include "x/xAction.h"

//#define XIniSysFuncDef_(name_) Error x##name_##IniSys(System *sP, void *sParamsP)
XIniSysFuncDef_(Action) {
  unused_(sParamsP);
  XAction *xActionP = (XAction*) sP;
  xActionP->nDistinctHivemindTriggers = 0;
  xActionP->entityPersonalityPairF = frayNew( sizeof( EntityPersonalityPair ), xGetNComps(sP) );
  xActionP->entityBlackboardPairF = frayNew( sizeof( EntityBlackboardPair ), xGetNComps(sP) );
  xActionP->histoHivemindTriggerA = arrayNew( sizeof(U32), KEY_MAX );
  memset( xActionP->histoHivemindTriggerA, 0, sizeof(U32) * KEY_MAX );
}

//#define XIniSubcompFuncDef_(name_)  Error x##name_##IniSubcomp(System *sP, const Entity entity, const Key subtype, void *dataP)
XIniSubcompFuncDef_(Action) {
  XAction *xActionP = (XAction*) sP;
  if (subtype == PERSONALITY) {
    Personality *personalityP = (Personality*) dataP;
    Quirk **quirkPP = personalityP->quirkPA;
    Quirk **quirkEndPP = quirkPP + personalityP->nQuirks;
    EntityPersonalityPair epPair = {
      .entity = entity,
      .personalityP = personalityP
    };
    frayAdd(xActionP->entityPersonalityPairF, &epPair, NULL);
    // For each quirk in the personality, increment the number of distinct triggers
    // whenever you find one we've never encountered before.
    for (; quirkPP < quirkEndPP; ++quirkPP) {
      U32 *histoElemP = &xActionP->histoHivemindTriggerA[ (*quirkPP)->trigger ];
      if ( *histoElemP  ) {
        ++xActionP->nDistinctHivemindTriggers;
      }
      ++( *histoElemP );
    }
  }
  else if ( subtype == BLACKBOARD  ) {
    EntityBlackboardPair ebbPair = {
      .entity = entity,
      .bbMP = (Map*) dataP
    };
    frayAdd( xActionP->entityBlackboardPairF, &ebbPair, NULL );
  }
}

XClrFuncDef_(Action) {
  XAction *xActionP = (XAction*) sP;
  XActionComp* cP = sP->cF;
  XActionComp* cEndP = cP + xGetNComps( sP );
  for ( ; cP < cEndP; ++cP ) {
    mapDel( &cP->bbMP );
  }
  hivemindDel(&xActionP->hivemindMP);
  arrayDel((void**) &xActionP->histoHivemindTriggerA);
  frayDel((void**) &xActionP->entityPersonalityPairF );
  frayDel((void**) &xActionP->entityBlackboardPairF );
}

static void _distributeHiveminds(XAction *xActionP) {
  // Histo the number of trees existing for each trigger.
  assert (xActionP && xActionP->histoHivemindTriggerA);
  Entity **hivemindEntitiesAP = NULL;
  // Allocate hivemind map
  xActionP->hivemindMP = mapNew( ARRAY, sizeof(Entity*), xActionP->nDistinctHivemindTriggers);
  // Allocate empty hiveminds.
  U32 nElems = arrayGetNElems(xActionP->histoHivemindTriggerA);
  for (U32 trigger = 0; trigger < nElems; ++trigger) {
    if (xActionP->histoHivemindTriggerA[trigger]) {
      Entity *entityA = arrayNew( sizeof(Entity), xActionP->histoHivemindTriggerA[trigger] );
      mapSet(xActionP->hivemindMP, trigger, &entityA);
    }
  }
  // Fill the hiveminds.
  EntityPersonalityPair *epP = xActionP->entityPersonalityPairF;
  EntityPersonalityPair *epEndP = epP + arrayGetNElems(epP);
  for (; epP < epEndP; ++epP) {
    Quirk **quirkPP = epP->personalityP->quirkPA; 
    Quirk **quirkEndPP = quirkPP + epP->personalityP->nQuirks;
    for (; quirkPP < quirkEndPP; ++quirkPP) {
      // Get pointer to array of entities out of hivemind map.
      hivemindEntitiesAP = (Entity**) mapGet(xActionP->hivemindMP, (*quirkPP)->trigger);
      // This is how we fill arrays without storing current index of each one's next empty slot.
      if (hivemindEntitiesAP) {
        (*hivemindEntitiesAP)[--xActionP->histoHivemindTriggerA[(*quirkPP)->trigger]] = epP->entity;
      }
    }
  }

  arrayDel((void**) &xActionP->histoHivemindTriggerA);
}

XPostprocessCompsDef_(Action) {
  _distributeHiveminds((XAction*) sP);
  // Everybody should have empty components right now.
  // What we need to do is populate the blackboard pointers.
  // Quirks will be populated when we tell the system to do something
  //   via a message. The postmutate function knows to copy the Quirk
  //   into the quirk pointer part of the Action component.
  XActionComp* cP;

  // Give components pointers to their blackboards.
  XAction* xP = (XAction*) sP;
  EntityBlackboardPair* ebbPairP = xP->entityBlackboardPairF;
  EntityBlackboardPair* ebbPairEndP = ebbPairP + *_frayGetFirstEmptyIdxP( xP->entityBlackboardPairF );
  for ( ; ebbPairP < ebbPairEndP; ++ebbPairP ) {
    cP = (XActionComp*) xGetCompPByEntity( sP, ebbPairP->entity );
    assert( cP );
    cP->bbMP = ebbPairP->bbMP;
  }
}

void hivemindDel(Map **hivemindMPP) {
  if (hivemindMPP) {
    Map *hivemindMP = *hivemindMPP;
    if (hivemindMP && hivemindMP->mapA) {
      Entity **entityAP = hivemindMP->mapA;
      Entity **entityEndAP = entityAP + hivemindMP->population;
      for (; entityAP < entityEndAP; entityAP++) {
        arrayDel((void**) entityAP);
      }
    }
  }
  mapDel(hivemindMPP);
}

inline static U8 _isHigherPriority(U8 newPriority, U8 existingPriority) {
  return newPriority < existingPriority;  // lower numbers are higher priority lol
}

static void _triggerIndividual(XAction *xActionSysP, Message *msgP) {
  assert( msgP->attn );
  System *sP = &xActionSysP->system;
  // Get entity's Action system component.
  XActionComp* cP = (XActionComp*) xGetCompPByEntity( sP, msgP->attn );
  assert (cP);
  // Get the personality, one per entity.
  Map **activityMPMP = (Map**) mapGet(sP->mutationMPMP, msgP->attn);
  // Get the tree that *would* be triggered if the entity is inactive or doing something less important. 
  Quirk** requestedQuirkPP = (Quirk**) mapGet(*activityMPMP, msgP->cmd);
  assert( requestedQuirkPP && *requestedQuirkPP );
  // Queue new action if it's higher priority than old or entity's inactive.
  if (!xIsEntityActive(sP, msgP->attn) || !cP->quirkP ||
      _isHigherPriority((*requestedQuirkPP)->priority, cP->quirkP->priority)) {
    xMutateComponent( sP, msgP->attn, msgP->cmd );
    // Set the target of this entity's action, if any.
    cP->tgtEntity = msgP->arg;
    // Set the quantity for the action, if applicable.
    cP->amount = msgP->attachment;  // better use S32 for actions
  }
  // Activate activity.
  xActivateComponentByEntity(sP, msgP->attn);
}

static void _triggerHivemind(XAction *xActionSysP, Message *msgP) {
  assert(msgP);
  // Get the Action system's hivemind array for the given stimulus.
  Entity **eAP = (Entity**) mapGet(xActionSysP->hivemindMP, msgP->cmd);
  assert( eAP && *eAP );
  // Init pointers.
  Entity *eP, *eEndP;
  arrayIniPtrs( (void*) *eAP, (void**) &eP, (void**) &eEndP, -1 );
  // Forward the message to each individual in the hivemind.
  for (; eP < eEndP; eP++) {
    msgP->attn = *eP;
    _triggerIndividual(xActionSysP, msgP);
  }
}

XGetShareFuncDefUnused_(Action);

XPostMutateFuncDefUnused_(Action);

XPostActivateFuncDefUnused_(Action);
XPostDeactivateFuncDefUnused_(Action);

// Entity acts on message if it's more urgent than its current activity.
XProcMsgFuncDef_(Action) {
  XAction *xActionSysP = (XAction*) sP;
  if (msgP->attn) {
    _triggerIndividual(xActionSysP, msgP);
  }
  else {
    _triggerHivemind(xActionSysP, msgP);
  }
}

void xActionRun(System *sP) {
  static_assert( sizeof( Activity ) == sizeof( XActionComp ), "Action system's component is supposed to be identical to an Activity! WHO BROKE THIS???" );
  XActionComp *cP = sP->cF;
  XActionComp *cEndP = cP + _frayGetFirstInactiveIdx(sP->cF);
  for (; cP < cEndP; cP++) {
    cP->quirkP->actionU( xGetEntityByVoidComponentPtr( sP, cP ),  (Activity*) cP, sP->mailboxF );
    if ( cP->complete ) {
      xQueueDeactivate( sP, cP );
    }
  }
}

X_(Action, 2, quirkP, FLG_NO_CF_SRC_A_);
