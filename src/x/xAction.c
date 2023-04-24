#include "xAction.h"

//#define XIniSysFuncDef_(name_) Error x##name_##IniSys(System *sP, void *sParamsP)
XIniSysFuncDef_(Action) {
  unused_(sParamsP);
  XAction *xActionP = (XAction*) sP;
  xActionP->nDistinctHivemindTriggers = 0;
  Error e = frayNew((void**) &xActionP->entityPersonalityPairF, sizeof(U32), xGetNComps(sP));
  if (!e) {
    e = arrayNew((void**) &xActionP->histoHivemindTriggerA, sizeof(U32), KEY_MAX);
  }
  return e;
}
//#define XIniSubcompFuncDef_(name_)  Error x##name_##IniSubcomp(System *sP, const Entity entity, const Key subtype, void *dataP)
XIniSubcompFuncDef_(Action) {
  if (subtype == PERSONALITY) {
    XAction *xActionP = (XAction*) sP;
    Personality *personalityP = (Personality*) dataP;
    Quirk **quirkPP = personalityP->quirkPA;
    Quirk **quirkEndPP = quirkPP + personalityP->nQuirks;
    EntityPersonalityPair epPair = {
      .entity = entity,
      .personalityP = personalityP
    };
    Error e = frayAdd(xActionP->entityPersonalityPairF, &epPair, NULL);
    if (!e) {
      for (; quirkPP < quirkEndPP; ++quirkPP) {
        xActionP->nDistinctHivemindTriggers += (!(xActionP->histoHivemindTriggerA[(*quirkPP)->trigger]++));
      }
    }
  }
  return SUCCESS;
}

XClrFuncDef_(Action) {
  XAction *xActionP = (XAction*) sP;
  hivemindDel(&xActionP->hivemindMP);
  arrayDel((void**) &xActionP->histoHivemindTriggerA);
  return SUCCESS;
}

static Error _distributeHiveminds(XAction *xActionP) {
  // Histo the number of trees existing for each trigger.
  if (!xActionP || !xActionP->histoHivemindTriggerA) {
    return E_BAD_ARGS;
  }
  Entity **hivemindEntitiesAP = NULL;
  // Allocate hivemind map
  Error e = mapNew(&xActionP->hivemindMP, ARRAY, sizeof(Entity*), xActionP->nDistinctHivemindTriggers);
  // Allocate empty hiveminds.
  if (!e) {
    U32 nElems = arrayGetNElems(xActionP->histoHivemindTriggerA);
    for (U32 trigger = 0; !e && trigger < nElems; ++trigger) {
      if (xActionP->histoHivemindTriggerA[trigger]) {
        Entity *entityA = NULL;
        e = arrayNew((void**) &entityA, sizeof(Entity), xActionP->histoHivemindTriggerA[trigger]);
        if (!e) {
          // Put the array of entities subscribed to trigger "trigger" in the hivemind map.
          hivemindEntitiesAP = (Entity**) mapGet(xActionP->hivemindMP, trigger);
          if (hivemindEntitiesAP) {
            *hivemindEntitiesAP = entityA;
          }
        }
      }
    }
  }
  // Fill the hiveminds.
  if (!e) {
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
  }

  arrayDel((void**) &xActionP->histoHivemindTriggerA);

  return e;
}

XPostprocessCompsDef_(Action) {
  return _distributeHiveminds((XAction*) sP);
}

void hivemindDel(Map **hivemindMPP) {
  if (hivemindMPP) {
    Map *hivemindMP = *hivemindMPP;
    if (hivemindMP && hivemindMP->mapA) {
      Entity **entityAP = hivemindMP->mapA;
      Entity **entityEndAP = hivemindMP->mapA + hivemindMP->population;
      for (; entityAP < entityEndAP; entityAP++) {
        arrayDel((void**) entityAP);
      }
    }
  }
  mapDel(hivemindMPP);
}

inline static U8 _isHigherPriority(U8 newPriority, U8 existingPriority) {
  return newPriority > existingPriority;
}

static Error _triggerIndividual(XAction *xActionSysP, Message *msgP) {
  System *sP = &xActionSysP->system;
  // Get entity's Action system component.
  XActionComp *cP = xGetCompPByEntity(sP, msgP->attn);
  if (!cP) {
    return E_BAD_KEY;
  }
  // Get the personality, one per entity.
  Map **activityMPP = (Map**) mapGet(sP->mutationMPMP, msgP->attn);
  if (!activityMPP || !(*activityMPP)) {
    return E_BAD_KEY;
  }
  // Get the tree that *would* be triggered if the entity is inactive or doing something less important. 
  XActionComp *cNewP = (XActionComp*) mapGet(*activityMPP, msgP->arg);

  if (cNewP) {
    // Queue new action if it's higher priority than old or entity's inactive.
    if (!_frayElemIsActive(sP->cF, msgP->attn) || 
        _isHigherPriority(cNewP->quirkP->priority, cP->quirkP->priority)) {
      *cP = *cNewP;
    }
    // Activate activity.
    xActivateComponentByEntity(sP, msgP->attn);
  }

  return SUCCESS;
}

static Error _triggerHivemind(XAction *xActionSysP, Message *msgP) {
  assert(msgP);
  Error e = SUCCESS;
  // Get the Action system's hivemind array for the given stimulus.
  Entity *eA = (Entity*) mapGet(xActionSysP->hivemindMP, msgP->arg);
  if (!eA) {
    return E_BAD_KEY;
  }
  // Init pointers.
  assert(eA);
  Entity *eP, *eEndP;
  arrayIniPtrs((void*) eA, (void**) &eP, (void**) &eEndP, -1);
  // Forward the message to each individual in the hivemind.
  for (; !e && eP < eEndP; eP++) {
    msgP->attn = *eP;
    e = _triggerIndividual(xActionSysP, msgP);
  }
  return e;
}

XGetShareFuncDefUnused_(Action);

XPostMutateFuncDefUnused_(Action);

// Entity acts on message if it's more urgent than its current activity.
XProcMsgFuncDef_(Action) {
  Error e;
  XAction *xActionSysP = (XAction*) sP;
  if (msgP->attn) {
    e = _triggerIndividual(xActionSysP, msgP);
  }
  else {
    e = _triggerHivemind(xActionSysP, msgP);
  }
  return e;
}

Error xActionRun(System *sP) {
  XActionComp *cP = sP->cF;
  XActionComp *cEndP = cP + _frayGetFirstInactiveIdx(sP->cF);
  Error e = SUCCESS;
  for (; !e && cP < cEndP; cP++) {
    e = cP->quirkP->actionU(cP->bbP);
  }
  return e;
}

X_no_mutations_(Action, 2, FLG_NO_CF_SRC_A_);
