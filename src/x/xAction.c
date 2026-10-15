#include "x/xAction.h"
#include "data/share.h"

XPostMutateFuncDefUnused_(Action);
XPostActivateFuncDefUnused_(Action);
XPostDeactivateFuncDefUnused_(Action);

//#define XIniSysFuncDef_(name_) Error x##name_##IniSys(System *sP)
XIniSysFuncDef_(Action) {
  XAction *xActionP = (XAction*) sP;
  xActionP->nDistinctHivemindTriggers = 0;
  xActionP->entityBlackboardPairF = frayNew( sizeof( EntityBlackboardPair ), xGetNComps(sP), TEMPORARY );
  xActionP->histoHivemindTriggerA = arrayNew( sizeof(U32), KEY_MAX, TEMPORARY );
  memset( xActionP->histoHivemindTriggerA, 0, sizeof(U32) * KEY_MAX );
}

// Action genes:
//   MUTABLE:   the entity's personality; each mutation is a Quirk* keyed by its trigger.
//   IMMUTABLE: the entity's blackboard (body is a Map* of void pointers).
XConsumeGeneFuncDef_(Action) {
  XAction* xP = (XAction*) sP;
  assert( sP );
  assert( geneP );
  switch ( geneP->class ) {
    case MUTABLE: {
      xMakeMutationMap( sP, entity, geneP );
      // Histo the triggers so hiveminds can be sized in makeComponents().
      const MutableGene* mutGeneP = (const MutableGene*) geneP;
      const Mutation* mutationP = mutGeneP->mutationA;
      const Mutation* mutationEndP = mutationP + mutGeneP->n;
      for ( ; mutationP < mutationEndP; ++mutationP ) {
        U32 *histoElemP = &xP->histoHivemindTriggerA[ mutationP->key ];
        if ( !*histoElemP ) {
          ++xP->nDistinctHivemindTriggers;
        }
        ++( *histoElemP );
      }
      break;
    }
    case IMMUTABLE: {
      const ImmutableGene* immGeneP = (const ImmutableGene*) geneP;
      assert( immGeneP->bodyP );
      EntityBlackboardPair ebbPair = {
        .entity = entity,
        .bbMP = (Map*) immGeneP->bodyP
      };
      frayAdd( xP->entityBlackboardPairF, &ebbPair, NULL );
      break;
    }
    case INTRACOMPOSITE:
      xConsumeGeneDefault( sP, entity, geneP );
      break;
    default:
      assert( FALSE );
      break;
  }
}

static void _distributeHiveminds(XAction *xActionP) {
  // Histo the number of trees existing for each trigger.
  assert (xActionP && xActionP->histoHivemindTriggerA);
  // Allocate hivemind map
  xActionP->hivemindMP = mapNew( ARRAY, sizeof(Entity*), xActionP->nDistinctHivemindTriggers, GENERAL);
  // Allocate empty hiveminds.
  U32 nElems = arrayGetNElems(xActionP->histoHivemindTriggerA);
  for (U32 trigger = 0; trigger < nElems; ++trigger) {
    if (xActionP->histoHivemindTriggerA[trigger]) {
      Entity *entityA = arrayNew( sizeof(Entity), xActionP->histoHivemindTriggerA[trigger], GENERAL );
      mapSet(xActionP->hivemindMP, trigger, &entityA);
    }
  }
  // Fill the hiveminds. Counting histo elements back down fills each array without tracking indices.
  System *sP = &xActionP->system;
  if ( !sP->mutationMPMP ) {
    return;
  }
  Entity *entityP = sP->cIdx2eA;
  Entity *entityEndP = entityP + *_frayGetFirstEmptyIdxP( sP->cF );
  Entity **hivemindEntitiesAP;
  for ( ; entityP < entityEndP; ++entityP ) {
    if ( !mapHasKey( sP->mutationMPMP, *entityP ) ) {
      continue;
    }
    Map *quirkMP = mapGetNestedMapP( sP->mutationMPMP, *entityP );
    for ( U32 trigger = 0; trigger < nElems; ++trigger ) {
      if ( xActionP->histoHivemindTriggerA[ trigger ] && mapHasKey( quirkMP, trigger ) ) {
        hivemindEntitiesAP = (Entity**) mapGet( xActionP->hivemindMP, trigger );
        assert( hivemindEntitiesAP && *hivemindEntitiesAP );
        (*hivemindEntitiesAP)[ --xActionP->histoHivemindTriggerA[ trigger ] ] = *entityP;
      }
    }
  }
}

XMakeComponentsDef_(Action) {
  // XAction doesn't need system mailboxes since actions will grab other systems' mailboxes internally.
  // That way you don't have to include an ugly mailbox argument across all your actions.
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
  Map *activityMP = mapGetNestedMapP(sP->mutationMPMP, msgP->attn);
  assert( activityMP );
  // Get the tree that *would* be triggered if the entity is inactive or doing something less important. 
  Quirk** requestedQuirkPP = (Quirk**) mapGet(activityMP, msgP->cmd);
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
    cP->quirkP->actionU( xGetEntityByVoidComponentPtr( sP, cP ),  (Activity*) cP );
    if ( cP->complete ) {
      xQueueDeactivate( sP, cP );
    }
  }
}

X_(Action, ACTION, quirkP, 0);
