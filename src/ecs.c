#include "ecs.h"

/* Go ahead and provide these functions here:
  Histogramming entitieSeeds' genes
*/

/**************/
/* Components */
/**************/
const static GetGeneFP getGeneFPA[] = {getUnaryGene, getBinaryGene, getNonbinaryGene};

/* Unary gene-selection */
void* getUnaryGene(U8 *geneHeaderP, Key *overrideKeyP) {
  UNUSED_(overrideKeyP);
  return ((UnaryGene*) geneHeaderP)->unaryP;
}

/* Binary gene-selection */
void* getBinaryGene(U8 *geneHeaderP, Key *overrideKeyP) {
  BinaryGene *bP = (BinaryGene*) geneHeaderP;
  if (overrideKeyP && *overrideKeyP)
    return arrayGetVoidElemPtr(bP->binaryA, ((*overrideKeyP & bP->mask) == bP->expVal));
  else
    return arrayGetVoidElemPtr(bP->binaryA, ((*bP->tgtP & bP->mask) == bP->expVal));
}

/* Nonbinary gene-selection */
void* getNonbinaryGene(U8 *geneHeaderP, Key *overrideKeyP) {
  NonbinaryGene *nP = (NonbinaryGene*) geneHeaderP;
  if (overrideKeyP)
    return mapGet(nP->mapP, *overrideKeyP);
  else
    return mapGet(nP->mapP, *nP->keyP);
}

void* getGene(U8 *geneHeaderP, Key *overrideKeyP) {
  /* No need for internal NULL-checks since wrapper does it here. */
  if (geneHeaderP != NULL)
    return (*getGeneFPA[*geneHeaderP])(geneHeaderP, overrideKeyP);
  return NULL;
}


/***********/
/* Systems */
/***********/
/* Only inspect every possible messaging error at load-time. Do not inspect every single message-reading. */
inline static U8 _validateActivityIdx(const System *sP, const U8 activityIdx) { 
  return (activityIdx < sP->nActivities);
}

/* Only inspect every possible messaging error at load-time. Do not inspect every single message-reading. */
inline static U8 _validateEcIdx(const System *sP, Activity *aP, const U8 cIdx) { 
  return (cIdx >= (U8) arrayGetNElems(aP->cA));
}

/* EC Getters and Setters */
/* When activating or deactivating, you have to update the system's directory to its new location. */
inline static Entity _getE(const void *cmpP) {
  return ((XHeader*) cmpP)->owner;
}


inline static void* _getCPByIndex(const System *sP, Activity *aP, U8 cIdx) {
  return (void*) (((U8*) aP->cA) + (sP->cSz * cIdx));
}

/* Returns a component's location in the jagged array of components.  */
inline static CDirEntry* _getCDirEntry(System *sP, Entity entity) {
  return (CDirEntry*) mapGet(sP->cDirectoryP, entity);
}

inline static ActDirEntry* _getActDirEntry(System *sP, Key actID) {
  return (ActDirEntry*) mapGet(sP->actDirectoryP, actID);
}

void* sGetC(System *sP, Entity entity) {
  CDirEntry *cdeP = (CDirEntry*) _getCDirEntry(sP, entity);
  if (cdeP)
    return cdeP->cP;
  return NULL;
}

Activity* sGetActivity(System *sP, Key actID) {
  ActDirEntry *adeP = _getActDirEntry(sP, actID);
  if (adeP)
    return adeP->activityP;
  return NULL;
}

Activity* sGetActivityFromE(System *sP, Entity entity) {
  CDirEntry *cdeP = (CDirEntry*) _getCDirEntry(sP, entity);
  if (cdeP) 
    return sGetActivity(sP, cdeP->activityID);
  return NULL;
}

/* Checks if the component, wherever it is in the jagged array, is before the function's stopping point in its array. */
inline static U8 _cmpIsActive(System *sP, Entity entity) {
  CDirEntry* cdeP = _getCDirEntry(sP, entity);
  Activity *aP = sGetActivityFromE(sP, entity);
  return (cdeP->cIdx < aP->firstInactiveIdx);
}

static Error  _updateCDirEntry(System *sP, Entity entity, U8 dstActID, U8 dstCIdx, void *dstCP) {
  CDirEntry *cdeP = _getCDirEntry(sP, entity);
  // This component exists in this system's directory, which means it's moving around right now.
  if (cdeP) {
    cdeP->activityID = dstActID;
    cdeP->cIdx       = dstCIdx;
    cdeP->cP         = dstCP;
  }
  // This component hasn't been added to map yet, which implies we're in initialization phase.
  else
    return E_BAD_KEY;
  return SUCCESS;
}

static Error _mvC(System *sP, void *srcCP, void *dstCP, Key dstActID, U8 dstCIdx) {
  memcpy((void*) dstCP, (const void*) srcCP, sP->cSz);  
  return _updateCDirEntry(sP, _getE(srcCP), dstActID, dstCIdx, dstCP);
}

// Used for deactivating a C within the same activity
Error sDeactivateC(System *sP, Entity entity) {
  // Directory entry for source component
  CDirEntry *srcComponentDirectoryEntryP = _getCDirEntry(sP, entity);
  if (!srcComponentDirectoryEntryP)
    return E_BAD_KEY;
  // Directory entry for source component's activity
  ActDirEntry *activityDirectoryEntryP = _getActDirEntry(sP, srcComponentDirectoryEntryP->activityID);
  if (!activityDirectoryEntryP)
    return E_BAD_KEY;
  // Pointer to destination component
  void *dstComponentP = _getCPByIndex((const System*) sP, activityDirectoryEntryP->activityP, --activityDirectoryEntryP->activityP->firstInactiveIdx);
  // Move source to placeholder
  memcpy(sP->swapPlaceholderP, (const void*) srcComponentDirectoryEntryP->cP, sP->cSz);
  // Move destination to source's original place
  Error e = _mvC(sP, dstComponentP, srcComponentDirectoryEntryP->cP, srcComponentDirectoryEntryP->activityID, srcComponentDirectoryEntryP->cIdx);
  // Move placeholder to destination
  if (!e)
    e = _mvC(sP, sP->swapPlaceholderP, dstComponentP, srcComponentDirectoryEntryP->activityID, activityDirectoryEntryP->activityP->firstInactiveIdx);
  return e;
}

// Used for activating a C within the same activity
// Used for deactivating a C within the same activity
Error sActivateC(System *sP, Entity entity) {
  // Directory entry for source component
  CDirEntry *srcComponentDirectoryEntryP = _getCDirEntry(sP, entity);
  if (!srcComponentDirectoryEntryP)
    return E_BAD_KEY;
  // Directory entry for source component's activity
  ActDirEntry *activityDirectoryEntryP = _getActDirEntry(sP, srcComponentDirectoryEntryP->activityID);
  if (!activityDirectoryEntryP)
    return E_BAD_KEY;
  // Pointer to destination component
  void *dstComponentP = _getCPByIndex((const System*) sP, activityDirectoryEntryP->activityP, activityDirectoryEntryP->activityP->firstInactiveIdx);
  // Move source to placeholder
  memcpy(sP->swapPlaceholderP, (const void*) srcComponentDirectoryEntryP->cP, sP->cSz);
  // Move destination to source's original place
  Error e = _mvC(sP, dstComponentP, srcComponentDirectoryEntryP->cP, srcComponentDirectoryEntryP->activityID, srcComponentDirectoryEntryP->cIdx);
  // Move placeholder to destination
  if (!e)
    e = _mvC(sP, sP->swapPlaceholderP, dstComponentP, srcComponentDirectoryEntryP->activityID, activityDirectoryEntryP->activityP->firstInactiveIdx++);
  return e;
}

static Error _updateActDirEntry(System *sP, Key actID, U8 dstActIdx, Activity *dstActP) {
  ActDirEntry *adeP = (ActDirEntry*) mapGet(sP->actDirectoryP, actID);
  // This component exists in this system's directory, which means it's moving around right now.
  if (adeP) {
    adeP->activityIdx = dstActIdx;
    adeP->activityP   = dstActP;
  }
  // This component hasn't been added to map yet, which implies we're in initialization phase.
  else
    return E_BAD_KEY;
  return SUCCESS;
}

static Error _mvActivity(System *sP, Activity *srcActP, U8 dstActIdx) {
  Activity *dstActP = &sP->activityA[dstActIdx];
  memcpy((void*) dstActP, (const void*) srcActP, sizeof(Activity));
  return _updateActDirEntry(sP, srcActP->id, dstActIdx, dstActP);
}

static Error _swapActivities(System *sP, Key srcActID, U8 dstActIdx) {
  ActDirEntry *srcAdeP = (ActDirEntry*) mapGet(sP->actDirectoryP, srcActID);
  if (!srcAdeP)
    return E_BAD_KEY;
  // Move source to placeholder
  Activity tmpActivity = *srcAdeP->activityP;
  // Move destination to source's original place
  Error e = _mvActivity(sP, &sP->activityA[dstActIdx], srcAdeP->activityIdx);
  // Move placeholder to destination
  if (!e)
    _mvActivity(sP, &tmpActivity, dstActIdx);
  return e;
}

Error sActivateActivity(System *sP, Key activityID) {
  return _swapActivities(sP, activityID, sP->firstInactiveActIdx++);
}

Error sDeactivateActivity(System *sP, Key activityID) {
  return _swapActivities(sP, activityID, --sP->firstInactiveActIdx);
}

/* Kind of a no-brainer that if you're transferring something, it's going to be active in its destination activity. */
inline static Error _startEcActivity(System *sP, Entity entity, Key dstActivityID) {
  Error e = SUCCESS;
  CDirEntry *cdeP = _getCDirEntry(sP, entity);
  if (cdeP != NULL)
    return E_BAD_KEY;
  if (dstActivityID >= sP->firstInactiveActIdx)
    e = sActivateActivity(sP, dstActivityID);
  if (cdeP->activityID == dstActivityID)
    e = sActivateC(sP, entity);
  else {
    /* Move component in destination out of the way then fill vacated spot with source EC. */
    Activity *dstActivityP = sGetActivity(sP, dstActivityID);
    void *firstInactiveDstCP = _getCPByIndex(sP, dstActivityP, dstActivityP->firstInactiveIdx);
    void *firstEmptyDstCP    = _getCPByIndex(sP, dstActivityP, dstActivityP->firstEmptyIdx);
    e = _mvC(sP, firstInactiveDstCP, firstEmptyDstCP, dstActivityID, dstActivityP->firstEmptyIdx++);  /* You should check if destination is already emtpy first. */
    if (!e)
      e = _mvC(sP, cdeP->cP, firstInactiveDstCP, dstActivityID, dstActivityP->firstInactiveIdx++);
  }
  return e;
}

Error sIniActivity(System *sP, Activity *aP, U32 nComps) {
  Error e = arrayNew(&aP->cA, sP->cSz, nComps);
  if (!e) 
    aP->ownerP = sP;
  return e;
}

static void _sClrActivities(System *sP) {
  if (sP->activityA != NULL)
    for (U8 i = 0, nActivities = sP->nActivities; i < nActivities; i++) 
      arrayDel((void**) &sP->activityA[i].cA);
}


Error sNewCDirectory(System *sP, U8 nElems) {
  return mapNew(&sP->cDirectoryP, sizeof(CDirEntry), nElems);
}

Error sNewActDirectory(System *sP) {
  return mapNew(&sP->actDirectoryP, sizeof(ActDirEntry), sP->nActivities);
}


// Components are initially spread out across the entity seeds, 
// so we'll initialize those one at a time later into the first activity as deactivated.
// Then the Behavior System will take care of putting those in the proper activity.
Error sIni(System *sP, U32 nComps) {
  // Sytems with system components need to initialize maps in sIniFP().
  Error e =  sNewCDirectory(sP, nComps);
  // Then allocate enough room for all components in every activity. 
  if (!e) 
    for (U8 i = 0; !e && i < sP->nActivities; i++) 
      e = sIniActivity(sP, &sP->activityA[i], nComps);
  
  // Make activity directory.
  if (!e)
    e = sNewActDirectory(sP);
  for (Key i = 1; !e && i < sP->nActivities; i++) {
    ActDirEntry ade = {i, &sP->activityA[i]};
    e = mapSet(sP->actDirectoryP, sP->activityA[i].id, &ade);
  }

  // Finally call the system's unique initializer.
  if (!e)
    e = (*sP->sIniSFP)();

  // Clean up if there are any problems
  if (e) {
    _sClrActivities(sP);
    if (sP->cDirectoryP != NULL)
      mapDel(&sP->cDirectoryP);
    if (sP->actDirectoryP != NULL)
      mapDel(&sP->actDirectoryP);
  }
  
  return e;
}

Error sAddC(System *sP, Entity entity, XHeader *xhP) {
  if (!sP || !xhP || !sP->cDirectoryP)
    return E_BAD_ARGS;

  // Skip entities who already have a component in this system.
  if (mapGet(sP->cDirectoryP, entity))
    return SUCCESS;

  // Make sure the component belongs to this system. This is only checked at load-time.
  if (xhP->type != sP->id)
    return E_SYS_CMP_MISMATCH;
  
  // Assign this component to its entity and, if necessary, prepare it for the system.
  xhP->owner = entity;
  Error e = (*sP->sIniCFP)(xhP);  // Inflatable components must be initialized before
                                    // being added to any activity.

  // If index falls within system's array of activities...
  if (_validateActivityIdx(sP, FIRST_ACTIVITY)) {
    Activity *aP = &sP->activityA[FIRST_ACTIVITY];
    // Copy component to first emtpy slot in specified activity
    void *dstCP = _getCPByIndex(sP, aP, aP->firstEmptyIdx);
    memcpy(dstCP, xhP, sP->cSz);
    // Add component's entity-lookup entry to the system's directory.
    CDirEntry cde = {
      .activityID  = FIRST_ACTIVITY,
      .cIdx        = aP->firstEmptyIdx++,  // this increments firstEmptyIdx 
      .cP          = dstCP
    };
    e = mapSet(sP->cDirectoryP, entity, &cde);
    aP->firstInactiveIdx++;  // TODO remove after developing the behavior tree
  }
  else 
    e = E_BAD_INDEX;

  return e;
}

Error sNewMailbox(Mailbox **mailboxPP, U32 nMailSlots) {
  Error e = jbAlloc((void**)mailboxPP, sizeof(Mailbox), 1);
  if (!e)
    e = arrayNew((void**) mailboxPP, sizeof(Message), nMailSlots);
  if (e) {
    arrayDel((void**) mailboxPP);
    jbFree((void**)mailboxPP);
  }
  return e;
}
    
static void _clrMailbox(Mailbox *mailboxP) {
  memset(mailboxP->msgA, 0, sizeof(Message) * arrayGetNElems(mailboxP));
}

static void _sClrMailboxes(System *sP) {
  arrayDel((void**)&sP->inbox.msgA);
  arrayDel((void**)&sP->outbox.msgA);
  sP->inbox.nMsgs = 0;
  sP->outbox.nMsgs = 0;
}

void sReset(System *sP) {
  /* TODO */
}

void sClr(System *sP) {
  _sClrMailboxes(sP);
  _sClrActivities(sP);
  mapDel(&sP->cDirectoryP);
}

// TODO:when you have some valid system-wide functions, add them here
#if 0
SysBasicFP sBasicFuncs[] = {

};
#endif

inline static void _sReadMessage(System *sP, Message *msgP) {
  switch(msgP->type) {
#if 0
    /* Message intends a system-wide response. All systems share the same pool of systemwide functions. */
    case SYSTEMWIDE_CMD:
      (sBasicFuncs[msgP->cmd])(sP);
      break;
#endif
    /* Message intends a repeating function call on a component. */
    case REPEATING_CMP_CMD:
      _startEcActivity(sP, msgP->toID, msgP->cmd);
      break;
    /* Message intends a one-time function call on a component. */
    case ONE_OFF_CMP_CMD:
      (*sP->oneOffFPA[msgP->cmd])(sP, msgP->toID, msgP->event);
      break;
    default:
      break;
  }
}

void _sReadInbox(System *sP) {
  if (sP != NULL && sP->inbox.msgA != NULL) {
    Message *msgP, *msgLastP;
    arrayIniPtrs(sP->inbox.msgA, (void**) &msgP, (void**) &msgLastP, sP->inbox.nMsgs);
    while (msgP < msgLastP)
      _sReadMessage(sP, msgP++);
  }
}

/* sWriteMessage() is called by implemented systems when specific events occur that may interest the outside world. */
void sWriteMessage(System *sP, Entity toEntity, Entity fromEntity) {
  Message *msgP = &sP->outbox.msgA[sP->outbox.nMsgs++];
  memcpy((void*) &sP->outbox.msgA[sP->outbox.nMsgs++], msgP, sizeof(Message));
}

/* This is how the entire ECS framework works. */
void sRun(System *sP) {
  _sReadInbox(sP);
  _clrMailbox(&sP->inbox);

  Activity *aP = &sP->activityA[0];
  Activity *aEndP = aP + sP->firstInactiveActIdx;

  // Run all live activities.
  for (; aP < aEndP; aP++) {
    (*aP->sFP)(aP);
    // Move dead activities out of the way.
    if (aP->firstInactiveIdx == 0)
      sDeactivateActivity(sP, aP->id);  
  }
}
