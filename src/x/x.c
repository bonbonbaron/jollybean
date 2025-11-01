#include "x/x.h"
#include "data/share.h"

static Map* _sysMP =  NULL;  // used strictly when distributing genes
static Map* _inboxMP =  NULL;  // used for inter-system communication

inline static Entity _getEntityByCompIdx(System *sP, Key compIdx) {
  return sP->cIdx2eA[compIdx];
}

void* xGetCompPByEntity(System *sP, Entity entity) {
  if (!sP || !entity) {
    return NULL;
  }
  Key *elemIdxP = _getCompIdxPByEntity(sP, entity);
  if (!elemIdxP) {
    return NULL;
  }
	return (void*) ((U8*) sP->cF + *elemIdxP * arrayGetElemSz(sP->cF));
}

Entity xGetEntityByCompIdx(System *sP, Key compIdx) {
  return _getEntityByCompIdx(sP, compIdx);
}

Key xGetCompIdxByEntity( System *sP, Entity entity ) {
  assert( sP && entity );
#ifndef NDEBUG
  return *( _getCompIdxPByEntity( sP, entity ) );
#else
  Key* idxP = _getCompIdxPByEntity( sP, entity );
  assert( idxP );
  return *idxP;
#endif
}


void __xSwap(System *sP, S32 origIdx, S32 newIdx) {
  assert (sP && newIdx >= 0 && origIdx >= 0);
  if (origIdx != newIdx) {
    Entity entity1 = _getEntityByCompIdx(sP, origIdx);
    Entity entity2 = _getEntityByCompIdx(sP, newIdx);
    Key *origIdxP  = (Key*) mapGet(sP->e2cIdxMP, entity1);
    Key *newIdxP   = (Key*) mapGet(sP->e2cIdxMP, entity2);
    assert(origIdxP && newIdxP);
    swap_(sP->cIdx2eA[newIdx], sP->cIdx2eA[*origIdxP]);
    swap_(*origIdxP, *newIdxP);
  }
}

static void _xSwap(System *sP, FrayChanges *changesP) {
  // If nothing changed, just return success; e.g. activating an element already in index 0
  if (changesP->newIdx != changesP->origIdx) {
    // Existence of paused elements requires an additional, intermediate swap.
    if (changesP->newIdx >= 0) {
      if (changesP->intermediateIdx == 0) {
        __xSwap(sP, changesP->origIdx, changesP->newIdx);
      }
      else {
        __xSwap(sP, changesP->origIdx, changesP->intermediateIdx);
        __xSwap(sP, changesP->intermediateIdx, changesP->newIdx);
      }
    }
  }
}

void xActivateComponentByEntity(System *sP, Entity entity) {
  assert (sP && entity);
  // We use a pointer instead of a new one to swap its actual location later.
  FrayChanges changes;
  Key *compOrigIdxP = _getCompIdxPByEntity(sP, entity);
  assert(compOrigIdxP);
  frayChangesIni_(changes, *compOrigIdxP);
  frayActivate(sP->cF, *compOrigIdxP, &changes);
  _xSwap(sP, &changes);
  sP->postActivate( sP, &changes );
}

void xDeactivateComponentByEntity(System *sP, Entity entity) {
  assert (sP && entity);
  FrayChanges changes;
  Key *compOrigIdxP = _getCompIdxPByEntity(sP, entity);
  assert (compOrigIdxP);
  frayChangesIni_(changes, *compOrigIdxP);
  frayDeactivate(sP->cF, *compOrigIdxP, &changes);
  _xSwap(sP, &changes);
  sP->postDeactivate( sP, &changes );
}

void xPauseComponentByEntity(System *sP, Entity entity) {
  assert (sP && entity);
  FrayChanges changes;
  Key *compOrigIdxP = _getCompIdxPByEntity(sP, entity);
  assert (compOrigIdxP);
  frayChangesIni_(changes, *compOrigIdxP);
  frayPause(sP->cF, *compOrigIdxP, &changes);
  _xSwap(sP, &changes);
}

void xUnpauseComponentByEntity(System *sP, Entity entity) {
  assert (sP && entity);
  FrayChanges changes;
  Key *compOrigIdxP = _getCompIdxPByEntity(sP, entity);
  assert (compOrigIdxP);
  frayChangesIni_(changes, *compOrigIdxP);
  frayUnpause(sP->cF, *compOrigIdxP, &changes);
  _xSwap(sP, &changes);
}

U32 xGetNComps(System *sP) {
  assert(sP && sP->cF);
	return arrayGetNElems(sP->cF);
}

// This adds a mutation map for an entity to the system and returns a poitner to it.
static Map* xNewMutationMap( const System* sP, const Entity entity, const Key nElems ) {
  assert( sP );
  assert( entity );
  assert( nElems );
  Map* mP = mapNew( RAW_DATA, sP->mutationSz, nElems, GENERAL );
  assert( mP );
  assert( sP->mutationMPMP );
  // Add new map to the system's nested maps of mutations before returning it.
  mapSet( sP->mutationMPMP, entity, &mP );
  return mP;
}

// First makes a new mutation map that gets added to mutation map nest, then fills it with mutations
void xMakeMutationMap( const System* sP, const Entity entity, const GeneHdr *geneP ) {
  assert( sP );
  assert( entity );
  assert( geneP );
  assert( geneP->class == MUTABLE );
  assert( mapGet( sP->mutationMPMP, entity ) == NULL );

  MutableGene* mutableGeneP = (MutableGene*) geneP;
  Map* entitysMutationMP = xNewMutationMap( sP, entity, mutableGeneP->n );

  Mutation* mutationP = mutableGeneP->mutationA;
  Mutation* mutationEndP = mutationP + mutableGeneP->n;
  for ( ; mutationP < mutationEndP; ++mutationP ) {
    assert( mutationP->mutationBodyP );
    assert( mutationP->key );
    mapSet(entitysMutationMP, mutationP->key, mutationP->mutationBodyP);
  }
}

static void _xIniSystem(System *sP, U32 nComps) {
  // Sytems with special parts need to initialize maps in sIniU().
  sP->cF = frayNew(sP->compSz, nComps, GENERAL );
  sP->cIdx2eA = arrayNew(sizeof(Entity), nComps, GENERAL );
  sP->e2cIdxMP = mapNew( RAW_DATA, sizeof(Key), nComps, GENERAL );
  sP->deactivateQueueF = frayNew(sizeof(Entity), nComps, GENERAL );
  sP->pauseQueueF = frayNew( sizeof(Entity), nComps, GENERAL );
  assert( _sysMP );
  assert( _inboxMP );
  mapSet( _sysMP, sP->id, &sP );
  mapSet( _inboxMP, sP->id, &sP->mailboxF );
  if (!(sP->flags & FLG_NO_MUTATIONS_) && sP->mutationSz) {
    sP->mutationMPMP = mapNew( MAP_POINTER, sizeof(Map*), nComps, GENERAL );
  }
  // Only allocate one mailbox; it serves as input and output.
  // TODO make this smarter than a raw constant
  // Also, give it ample room to handle multiple messages per entity.
#define MAILBOX_MULTIPLY_NUM_SLOTS (3)
  sP->mailboxF = mailboxNew( nComps * MAILBOX_MULTIPLY_NUM_SLOTS, GENERAL );
  // Finally, call the system's unique initializer.
  (*sP->iniSys)(sP);  // fail-assert if this bombs
}

static void _xIniSystems( const System* sPA[], const GeneHisto* geneHisto, const Key nSystems ) {
  assert( sPA );
  assert( geneHisto->nExclusivesA );
  assert( nSystems );
  for ( Key i = 0; i < nSystems; ++i ) {
    if ( geneHisto[i].nExclusivesA[i] ) {
      _xIniSystem( (System*) sPA[i], geneHisto[i].nExclusivesA[i] );
    }
  }
}

void xMutateComponent(System *sP, Entity entity, Key newCompKey) {
  // Make sure entity, system, and the key to the new component are all valid parameters.
  assert (sP &&  entity &&  newCompKey);
  // Make sure the system was set up for mutations in the first place.
  if (!(sP->flags & FLG_NO_MUTATIONS_)) {
    // Get the nested map of mutations for this particular entity.
    Map *mutationMP = mapGetNestedMapP(sP->mutationMPMP, entity);
    // Get a pointer to the entity's component.
    void* cP = xGetCompPByEntity(sP, entity);
    if (cP) {
      // Get a pointer to the mutation.
      void *tmpP = mapGet(mutationMP, newCompKey);
      if (tmpP) {
        // Mutate the only part of the component that should change.
        memcpy((U8*) cP + sP->mutationOffset, tmpP, arrayGetElemSz(mutationMP->mapA));
        return sP->postMutate(sP, cP);
      }
      // It's a design decision to not error out when mutation doesn't exist. 
    }
  }
}

static void _xReadInbox(System *sP) {
  assert (sP &&  sP->mailboxF);
  Message *msgP = sP->mailboxF;
  Message *msgEndP = msgP + *_frayGetFirstEmptyIdxP(sP->mailboxF);
  for (; msgP < msgEndP; msgP++) {
    assert (msgP->address == sP->id);
    switch(msgP->cmd) {
      case MUTATE_AND_ACTIVATE: 
        xMutateComponent(sP, msgP->attn, msgP->arg);
      case ACTIVATE:
        xActivateComponentByEntity(sP, msgP->attn);
        break;
      case MUTATE_AND_DEACTIVATE: 
        xMutateComponent(sP, msgP->attn, msgP->arg);
      case DEACTIVATE:
        xDeactivateComponentByEntity(sP, msgP->attn);
        break;
      case PAUSE:
        xPauseComponentByEntity(sP, msgP->attn);
        break;
      case UNPAUSE:
        xUnpauseComponentByEntity(sP, msgP->attn);
        break;
      case MUTATE:
        xMutateComponent(sP, msgP->attn, msgP->arg);
        break;
      default:
        sP->processMessage(sP, msgP);  // some systems require more specific treatment
        break;
    }
  }
  _frayClr(sP->mailboxF);
}

static void _pauseQueue(System *sP) {
  assert (sP);
  U32 nPausedEntities = *_frayGetFirstEmptyIdxP(sP->pauseQueueF);
  if (nPausedEntities) {
    Entity *entityP = sP->pauseQueueF;
    Entity *entityEndP = entityP + nPausedEntities;
    for (; entityP < entityEndP; ++entityP) {
      xPauseComponentByEntity(sP, *entityP);
    }
    _frayClr(sP->pauseQueueF);
  }
}

static void _deactivateQueue(System *sP) {
  assert (sP);
  U32 nDeactivatedEntities = *_frayGetFirstEmptyIdxP(sP->deactivateQueueF);
  if (nDeactivatedEntities) {
    Entity *entityP = sP->deactivateQueueF;
    Entity *entityEndP = entityP + nDeactivatedEntities;
    for (; entityP < entityEndP; ++entityP) {
      xDeactivateComponentByEntity(sP, *entityP);
    }
    _frayClr(sP->deactivateQueueF);
  }
}

void xAddEntity( const System* sP, const Entity entity ) {
  assert( sP );
  assert( entity );
  assert( sP->e2cIdxMP );
  assert( sP->cIdx2eA );
  assert( mapGet( sP->e2cIdxMP, entity ) ==  NULL ); // prevents double-adding entity

  U32 cIdx = 0;
  // Add empty component to fray. Get its index too so you know which belongs to this entity.
  frayAddEmpty( sP->cF, &cIdx );
  assert( cIdx < arrayGetNElems( sP->cF ) );
  assert( cIdx < KEY_MAX );
  sP->cIdx2eA[ cIdx ] = entity;
  mapSet( sP->e2cIdxMP, entity, (Key*) &cIdx );
}
  

Entity xGetEntityByVoidComponentPtr(System *sP, void *componentP) {
  assert (sP && componentP);
  Entity compIdx = ((void*) componentP - (void*) sP->cF) / sP->compSz;
  return xGetEntityByCompIdx(sP, compIdx);
}

Bln xIsEntityActive( System  *sP, Entity entity ) {
  Key* idxP = _getCompIdxPByEntity(sP, entity);
  assert( idxP );
  return _frayElemIsActive( sP->cF, *idxP );
}

void xQueuePause(System *sP, void *componentP) {
  Entity entity = xGetEntityByVoidComponentPtr(sP, componentP);
  frayAdd(sP->pauseQueueF, &entity, NULL);
}

void xQueueDeactivate(System *sP, void *componentP) {
  Entity entity = xGetEntityByVoidComponentPtr(sP, componentP);
  frayAdd(sP->deactivateQueueF, &entity, NULL);
}

/* This is how the entire ECS framework works. */
void xRun(System *sP) {
  _xReadInbox(sP);
  sP->run(sP);
  _deactivateQueue(sP);
  _pauseQueue(sP);
}

// We don't need to share systems.
// We don't need to share inboxes either. Only systems need each other's inboxes.

static System* _getSystem( const SystemId sysId ) {
  assert( _sysMP );
  System** sPP = mapGet( _sysMP, sysId );
  assert( sPP );
  assert( *sPP );
  return *sPP;
}

Message* xGetInbox( const SystemId sysId ) {
  assert( _sysMP );
  return (Message*) mapGet( _sysMP, sysId );
}


static StripDataS** _sdPF;

// Inflate a whole array of strip data.
static void _inflateMedia() {
  if ( _sdPF ) {
#if MULTITHREADED
    multithread_(sdInflate, (void*) _sdPF);
    multithread_(sdUnpack, (void*) _sdPF);
    multithread_(sdAssemble, (void*) _sdPF);
#else 
    StripDataS** sdPP = _sdPF;
    StripDataS** sdEndPP = sdPP + frayGetNElems_(_sdPF);
    for ( ;  sdPP < sdEndPP; ++sdPP ) {  // TODO make this more pro bruh
      stripIni(*sdPP, TEMPORARY);
    }
#endif
  }
}

void xRegisterForInflation( StripDataS* sdP ) {
  // We use a static array in order to allow all systems to put stuff into it without passing it everywhere.
  assert( _sdPF );  
  assert( sdP );  
  // Defer inflation to optimize icache and for multithreaded inflation.
  if (!(sdP->flags & SD_SET_FOR_INFLATION_)) {
    sdP->flags |= SD_SET_FOR_INFLATION_;
    frayAdd(_sdPF, &sdP, NULL);  // this asserts frayP != NULL, so no need to do it above.
  }
}

static void _distributeGene( Entity entity, GeneHdr* geneHdrP ) {
  assert(geneHdrP);
  assert(entity);

  GeneHdr** currGeneHdrPP;
  GeneHdr** geneHdrEndPP;
  System *sP;
  switch (geneHdrP->class) {
    case INTERCOMPOSITE:  // recurses
      InterCompositeGene* compGeneP = (InterCompositeGene*) geneHdrP;
      currGeneHdrPP = compGeneP->geneHdrPA;
      geneHdrEndPP = currGeneHdrPP + compGeneP->hdr.u.n;
      for (; currGeneHdrPP < geneHdrEndPP; ++currGeneHdrPP) {
        assert(currGeneHdrPP);
        _distributeGene(entity, *currGeneHdrPP );
      }
      break;
    // This takes care of giving the same genome a different position in various scenes.
    // TODO make rendering system handle positioning.
    //
    // Now the question is... how do we reliably fuse the pivots with the additives and substitutes?
    // A pivot data structure should tell us how many fields are missing in a composite.
    // But that leaves out hwo many can be added.This is where fixed-size arrays becomes unwieldy.
    // Eh... I'll figure thsi out later.
    case DERIVATIVE:
      DerivativeGene* appGeneP = (DerivativeGene*) geneHdrP;
      currGeneHdrPP = appGeneP->tweakPA;
      geneHdrEndPP = currGeneHdrPP + appGeneP->hdr.u.n;
      for (; currGeneHdrPP < geneHdrEndPP; ++currGeneHdrPP) {
        assert(currGeneHdrPP);
        _distributeGene(entity, *currGeneHdrPP );
      }
      break;
    case DERIVATIVE_VECTOR:
      DerivativeVectorGene* appGeneP = (DerivativeVectorGene*) geneHdrP;
      currGeneHdrPP = appGeneP->tweakPA;
      geneHdrEndPP = currGeneHdrPP + appGeneP->hdr.u.n;
      for (; currGeneHdrPP < geneHdrEndPP; ++currGeneHdrPP) {
        assert(currGeneHdrPP);
        _distributeGene(entity, *currGeneHdrPP );
      }
      break;
    // case MEDIA:  <-- Systems registering MEDIA genes instead of direct distribution allows for keeping them in intercomposites.
    case INTRACOMPOSITE:
    case IMMUTABLE:
    case MUTABLE:
      sP = _getSystem( geneHdrP->u.type ); 
      assert(sP);
      xAddEntity( sP, entity );
      sP->consumeGene(sP, entity, geneHdrP);
      break;
    default:
      assert(FALSE); // gene has an incompatible gene class
      break;
  }
}

static void _distributeGenes( const RootGene* rootP ) {
  assert( rootP );
  assert( rootP->hdr.class == ROOT );

  _sdPF = NULL;
  if ( rootP->histo.nDistinctMedia ) {
    _sdPF = frayNew( sizeof(StripDataS*), rootP->histo.nDistinctMedia, TEMPORARY);  
  }

  GeneHdr** genomePP = rootP->genomePA;
  GeneHdr** genomeEndPP = genomePP + rootP->hdr.u.n;
  for (Entity entity = 0; genomePP < genomeEndPP; ++genomePP) { // entity = 0 -> preincrement is slightly faster lol
    _distributeGene( ++entity, *genomePP );
  }

  _inflateMedia();  
}

static void _xMakeComponents( const System* sPA[], const Key nSystems ) {
  assert( sPA );
  assert( nSystems );
  for ( Key i = 0; i < nSystems; ++i ) {
    sPA[i]->makeComponents( (System*) sPA[i] );
  }
}

void xIni( const System* sPA[],  const Key nSystems, const RootGene* rootP ) {
  // Reset memory
  memRstAll();
  // init permanent system memory
  if ( !_sysMP && !_inboxMP ) {
    _sysMP = mapNew( NONMAP_POINTER, sizeof(System*), N_SYSTEM_TYPES, PERMANENT );
    _inboxMP = mapNew( NONMAP_POINTER, sizeof(Message*), N_SYSTEM_TYPES, PERMANENT );
  }

  _xIniSystems( sPA, &rootP->histo, nSystems );
  _distributeGenes( rootP );
  _xMakeComponents( sPA, nSystems );
}

