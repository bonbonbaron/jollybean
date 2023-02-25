#include "xGo.h"


//#define xIniSysFuncDef_(name_) Error x##name_##IniSys(System *sP, void *sParamsP)
xIniSysFuncDef_(Go) {
  unused_(sParamsP);
  XGo *xGoP = (XGo*) sP;
  xGoP->nDistinctHivemindTriggers = 0;
  Error e = frayNew((void**) &xGoP->entityPersonalityPairF, sizeof(U32), xGetNComps(sP));
  if (!e) {
    e = histoNew(&xGoP->histoHivemindTriggerA, KEY_MAX);
  }
  return e;
}
//#define XIniSubcompFuncDef_(name_)  Error x##name_##IniSubcomp(System *sP, const Entity entity, const Key subtype, void *dataP)
XIniSubcompFuncDef_(Go) {
  if (subtype == PERSONALITY) {
    XGo *xGoP = (XGo*) sP;
    Personality *personalityP = (Personality*) dataP;
    Quirk **quirkPP = personalityP->quirkPA;
    Quirk **quirkEndPP = quirkPP + personalityP->nQuirks;
    EntityPersonalityPair epPair = {
      .entity = entity,
      .personalityP = personalityP
    };
    Error e = frayAdd(xGoP->entityPersonalityPairF, &epPair, NULL);
    if (!e) {
      for (; quirkPP < quirkEndPP; ++quirkPP) {
        xGoP->nDistinctHivemindTriggers += (!(xGoP->histoHivemindTriggerA[(*quirkPP)->trigger]++));
      }
    }
  }
  return SUCCESS;
}

XClrFuncDef_(Go) {
  XGo *xGoP = (XGo*) sP;
  hivemindDel(&xGoP->hivemindMP);
  histoDel(&xGoP->histoHivemindTriggerA);
  return SUCCESS;
}

static Error _distributeHiveminds(XGo *xGoP) {
  // Histo the number of trees existing for each trigger.
  if (!xGoP || !xGoP->histoHivemindTriggerA) {
    return E_BAD_ARGS;
  }
  Entity **hivemindEntitiesAP = NULL;
  // Allocate hivemind map
  Error e = mapNew(&xGoP->hivemindMP, sizeof(Entity*), xGoP->nDistinctHivemindTriggers);
  // Allocate empty hiveminds.
  if (!e) {
    U32 nElems = arrayGetNElems(xGoP->histoHivemindTriggerA);
    for (U32 trigger = 0; !e && trigger < nElems; ++trigger) {
      if (xGoP->histoHivemindTriggerA[trigger]) {
        Entity *entityA = NULL;
        e = arrayNew((void**) &entityA, sizeof(Entity), xGoP->histoHivemindTriggerA[trigger]);
        if (!e) {
          // Put the array of entities subscribed to trigger "trigger" in the hivemind map.
          hivemindEntitiesAP = (Entity**) mapGet(xGoP->hivemindMP, trigger);
          if (hivemindEntitiesAP) {
            *hivemindEntitiesAP = entityA;
          }
        }
      }
    }
  }
  // Fill the hiveminds.
  if (!e) {
    EntityPersonalityPair *epP = xGoP->entityPersonalityPairF;
    EntityPersonalityPair *epEndP = epP + arrayGetNElems(epP);
    for (; epP < epEndP; ++epP) {
      Quirk **quirkPP = epP->personalityP->quirkPA; 
      Quirk **quirkEndPP = quirkPP + epP->personalityP->nQuirks;
      for (; quirkPP < quirkEndPP; ++quirkPP) {
        // Get pointer to array of entities out of hivemind map.
        hivemindEntitiesAP = (Entity**) mapGet(xGoP->hivemindMP, (*quirkPP)->trigger);
        // This is how we fill arrays without storing current index of each one's next empty slot.
        if (hivemindEntitiesAP) {
          (*hivemindEntitiesAP)[--xGoP->histoHivemindTriggerA[(*quirkPP)->trigger]] = epP->entity;
        }
      }
    }
  }

  histoDel(&xGoP->histoHivemindTriggerA);

  return e;
}

XPostprocessCompsDef_(Go) {
  return _distributeHiveminds((XGo*) sP);
}

void hivemindDel(Map **hivemindMPP) {
  if (hivemindMPP) {
    Map *hivemindMP = *hivemindMPP;
    if (hivemindMP && hivemindMP->mapA) {
      Entity **entityAP = hivemindMP->mapA;
      Entity **entityEndAP = hivemindMP->mapA + arrayGetNElems(hivemindMP->mapA);
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

static Error _triggerIndividual(XGo *xGoSysP, Message *msgP) {
  System *sP = &xGoSysP->system;
  // Get entity's Go system component.
  XGoComp *cP = xGetCompPByEntity(sP, msgP->attn);
  if (!cP) {
    return E_BAD_KEY;
  }
  // Get the personality, one per entity.
  Map **activityMPP = (Map**) mapGet(sP->mutationMPMP, msgP->attn);
  if (!activityMPP || !(*activityMPP)) {
    return E_BAD_KEY;
  }
  // Get the tree that *would* be triggered if the entity is inactive or doing something less important. 
  XGoComp *cNewP = (XGoComp*) mapGet(*activityMPP, msgP->arg);

  if (cNewP) {
    // Queue new action if it's higher priority than old or entity's inactive.
    if (!frayElemIsActive(sP->cF, msgP->attn) || 
        _isHigherPriority(cNewP->quirkP->priority, cP->quirkP->priority)) {
      *cP = *cNewP;
    }
    // Activate activity.
    xActivateComponentByEntity(sP, msgP->attn);
  }

  return SUCCESS;
}

static Error _triggerHivemind(XGo *xGoSysP, Message *msgP) {
  assert(msgP);
  Error e = SUCCESS;
  // Get the Go system's hivemind array for the given stimulus.
  Entity *eA = (Entity*) mapGet(xGoSysP->hivemindMP, msgP->arg);
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
    e = _triggerIndividual(xGoSysP, msgP);
  }
  return e;
}

XGetShareFuncDefUnused_(Go);

// Entity acts on message if it's more urgent than its current activity.
XProcMsgFuncDef_(Go) {
  Error e;
  XGo *xGoSysP = (XGo*) sP;
  if (msgP->attn) {
    e = _triggerIndividual(xGoSysP, msgP);
  }
  else {
    e = _triggerHivemind(xGoSysP, msgP);
  }
  return e;
}

Error xGoRun(System *sP) {
  XGoComp *cP = sP->cF;
  XGoComp *cEndP = cP + frayGetFirstInactiveIdx(sP->cF);
  Error e = SUCCESS;
  for (; !e && cP < cEndP; cP++) {
    e = cP->quirkP->actionU(cP->entity, cP->bbMP);
  }
  return e;
}

X_(Go, 2, FLG_NO_CF_SRC_A_);
