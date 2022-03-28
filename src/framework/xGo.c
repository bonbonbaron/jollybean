#include "xGo.h"

static Error _histoHiveMindTriggers(U32 **histoAP, XGoIniSeedPkg *seedPkgP, U32 *totalDistinctHiveMindsP) {
  // An  N-byte number can represent 1 << (N * 8) values.
  U32 N = 1 << (sizeof(Key) * 8);
  Error e = histoNew(histoAP, N);
  U32 *histoA;

  if (!e)
    histoA = *histoAP;

  if (!e) {
    XGoIniSeed *seedP = seedPkgP->seedA;
    XGoIniSeed *seedEndP = seedP + seedPkgP->nSeeds;
    for (; seedP < seedEndP; seedP++) {
      Quirk *quirkP = seedP->personalityP->quirkPA[0];
      Quirk *quirkEndP = quirkP + seedP->personalityP->nQuirks;
      for (; quirkP < quirkEndP; quirkP++) {
        *totalDistinctHiveMindsP += (!histoA[quirkP->trigger]);
        histoA[quirkP->trigger]++;
      }
    }
  }

  if (e && histoA)
    histoDel(histoAP);

  return e;
}

static Error _distributeHiveMinds(XGo *xGoSysP, XGoIniSeedPkg *seedPkgP) {
  if (!xGoSysP || !seedPkgP)
    return E_BAD_ARGS;

  U32 *histoA = NULL;
  U32 nDistinctHiveMinds = 0;
  // Histo the number of quirks existing for each trigger.
  Error e = _histoHiveMindTriggers(&histoA, seedPkgP, &nDistinctHiveMinds);
  if (!nDistinctHiveMinds)
    goto nevermind;
  // New hiveMind map
  if (!e)
    e = mapNew(&xGoSysP->hiveMindMP, sizeof(void*), nDistinctHiveMinds);
  // New array of entities for each trigger key's value
  if (!e) {
    U32 nElems = arrayGetNElems(histoA);
    for (U32 i = 0; !e && i < nElems; i++) 
      if (histoA[i]) {
        Entity *entityA = NULL;
        e = arrayNew((void**) &entityA, sizeof(Entity), histoA[i]);
        if (!e) {
          void *voidPtr = mapGet(xGoSysP->hiveMindMP, i);
          if (voidPtr)
            voidPtr = entityA;
        }
      }
  }
  // Form hive minds out of entities listening for same triggers.
  if (!e) {
    XGoIniSeed *seedP = seedPkgP->seedA;
    XGoIniSeed *seedEndP = seedP + seedPkgP->nSeeds;
    for (; seedP < seedEndP; seedP++) {
      Quirk *quirkP = seedP->personalityP->quirkPA[0];
      Quirk *quirkEndP = quirkP + seedP->personalityP->nQuirks;
      for (; quirkP < quirkEndP; quirkP++) {
        Entity *entityA = (Entity*) mapGet(xGoSysP->hiveMindMP, quirkP->trigger);
        // Stick entity in last empty slot for efficiency.
        if (entityA)
          entityA[--histoA[quirkP->trigger]] = seedP->entity;
      }
    }
  }

  nevermind:
  if (histoA)
    histoDel(&histoA);

  return e;
}

static Error _reactionIni(Reaction *reactionP, XGoIniSeed *seedP) {
  if (!reactionP || seedP)
    return E_BAD_ARGS;
  Error e = SUCCESS;
  if (!reactionP->treeSP->treeP)  // Behavior tree init 
    e = btNew(reactionP->treeSP->rootSrcP, &reactionP->treeSP->treeP);
  if (!e)  // Blackboard init
    e = bbNew(&reactionP->bbP, seedP->entity, seedP->bbSeedP);
  if (!e)  // BTree Status init
    e = btStatNew(&reactionP->btStatP, reactionP->treeSP->treeP->rootP);
  return e;
}

static void _reactionClr(Reaction *reactionP) {
  if (reactionP->treeSP->treeP)
    btDel(&reactionP->treeSP->treeP);
  bbDel(&reactionP->bbP);
  btStatDel(&reactionP->btStatP);
}

Error xGoIniSys(System *sP, void *sParamsP) {
  if (!sP || !sParamsP)
    return E_BAD_ARGS;
  XGo *xGoSysP = (XGo*) sP;
  XGoIniSeedPkg *seedPkgP = (XGoIniSeedPkg*) sParamsP;
  // Init seed pointers.
  XGoIniSeed *seedP = seedPkgP->seedA;
  const XGoIniSeed *seedEndP = seedP + seedPkgP->nSeeds;;
  // Behavior trees must be created before blackboards due to the latter's init dependency.
  // Make the array that'll hold the map pointers.
  Error e = arrayNew((void**) &xGoSysP->reactionMPA, sizeof(Map*), seedPkgP->nSeeds);
  if (!e) {
    for (Map **mapPP = xGoSysP->reactionMPA; !e && seedP < seedEndP; seedP++, mapPP++) {
      // Make the map that'll hold all the trigger-to-BT singleton mappings.
      e = mapNew(mapPP, sizeof(Reaction*), seedP->personalityP->nQuirks);
      if (!e) {
        Map *reactionMapP = *mapPP;
        Quirk **quirkPP = seedP->personalityP->quirkPA;
        Quirk **quirkEndPP = quirkPP + seedP->personalityP->nQuirks;
        // Map all the triggers to their respective behavior tree singletons.
        for (; !e && quirkPP < quirkEndPP; quirkPP++) {
          Quirk *quirkP = *quirkPP;
          Reaction *reactionP = &quirkP->reaction;
          e = _reactionIni(reactionP, seedP);
          // Put reaction composed of the above into reaction map
          if (!e)  
            e = mapSet(reactionMapP, quirkP->trigger, reactionP);
          // Put initial component into system; it'll populate with behavior stuff when needed.
          if (!e) { 
            e = xAddComp(sP, seedP->entity, sP->id, FALSE, NULL);
          }
        }
      }
    }
  }
  // Add all components to system.
  // (Dummy component gets filled in by XGoIniComp() through xAddComp() and added to xGo.)
  // Init hive minds.
  if (!e)
    e = _distributeHiveMinds(xGoSysP, seedPkgP);
  if (e)
    xGoClr(sP);
  return e;
}

Error xGoIniComp(System *sP, XGoComp *cP) {
  unused_(sP);
  cP->data.activePriority = 0;
  cP->data.activeBtP = NULL;  // Nobody starts out knowing what they'll do. 
  return SUCCESS;
}

void xGoClr(System *sP) {
  XGo *xGoSysP = (XGo*) sP;
  // Delete hive minds and their map container
  if (xGoSysP->hiveMindMP && xGoSysP->hiveMindMP->mapA) {
    Entity **entityPP = xGoSysP->hiveMindMP->mapA;
    Entity **entityEndPP = xGoSysP->hiveMindMP->mapA;
    for (; entityPP < entityEndPP; entityPP++)
      arrayDel((void**) entityPP);
  }
  mapDel(&xGoSysP->hiveMindMP);
  // Delete tree singletons, the maps containing them, and the array containing the maps.
  Map **mapPP = xGoSysP->reactionMPA;
  Map **mapEndPP = mapPP + arrayGetNElems(xGoSysP->reactionMPA);
  for (; mapPP < mapEndPP; mapPP++) {
    Reaction *reactionP = (Reaction*) (*mapPP)->mapA;
    Reaction *reactionEndP = reactionP + arrayGetNElems((*mapPP)->mapA);
    for (; reactionP < reactionEndP; reactionP++) 
      _reactionClr(reactionP);
    mapDel(mapPP);
  }
  arrayDel((void**) &xGoSysP->reactionMPA);
}

inline static U8 _isHigherPriority(U8 newPriority, U8 existingPriority) {
  return newPriority > existingPriority;
}

static Error _triggerIndividual(XGo *xGoSysP, Message *msgP) {
  Entity entity = msgP->topic;
  // Get entity's reaction to message's trigger.
  XGoComp *compP = (XGoComp*) xGetComp(&xGoSysP->system, entity);
  Map *compMapP = xGetCompMapP(&xGoSysP->system, entity);
  if (!compMapP)
    return E_BAD_KEY;
  Reaction *reactionP = (Reaction*) mapGet(compMapP, msgP->msg);
  if (!reactionP)
    return E_BAD_KEY;

  // Queue new action if it's higher priority than old or entity's inactive.
  if (!xCompIsActive(&xGoSysP->system, entity) || 
      _isHigherPriority(reactionP->priority, compP->data.activePriority)) {
    compP->data.activePriority = reactionP->priority;
    compP->data.activeBtP = reactionP->treeSP->treeP;
    compP->data.activeBbP = reactionP->bbP;
    compP->data.activeBtStatusP = reactionP->btStatP;
  }
  return SUCCESS;
}

static Error _triggerHiveMind(XGo *xGoSysP, Message *msgP) {
  assert(msgP);
  Error e = SUCCESS;
  Entity *eA = (Entity*) mapGet(xGoSysP->hiveMindMP, msgP->msg);
  if (!eA)
    return E_BAD_KEY;
  assert(eA);
  Entity *eP, *eEndP;
  arrayIniPtrs((void*) eA, (void**) &eP, (void**) &eEndP, -1);
  for (; !e && eP < eEndP; eP++) {
    msgP->to = *eP;
    e = _triggerIndividual(xGoSysP, msgP);
  }
  return e;
}

XGetShareFuncDefUnused_(Go);

// Entity acts on message if it's more urgent than its current activity.
XProcMsgFuncDef_(Go) {
  Error e;
  XGo *xGoSysP = (XGo*) sP;
  if (msgP->topic)
    e = _triggerIndividual(xGoSysP, msgP);
  else
    e = _triggerHiveMind(xGoSysP, msgP);
  return e;
}

Error xGoRun(Focus *fP) {
  XGo *xGoSysP = (XGo*) fP->ownerP;
  XGoComp *cP, *cEndP;
  cP = (XGoComp*) fP->compA;
  cEndP = cP + fP->firstInactiveIdx;

  // TODO: add logic for putting tree to sleep based on returned NodeStatus.
  for (; cP < cEndP; cP++) 
    btRun(cP->data.activeBtP, cP->data.activeBtStatusP, cP->data.activeBbP, xGoSysP->system.outboxP);

  return SUCCESS;
}
