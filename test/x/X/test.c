#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "data.h"
#include "x.h"
#include "xA.h"
#include <math.h>

// TODO Looks good to the human eye, but we want the computer to do the validating for us.
typedef struct {
  Entity   entity;
  XMailCmd state;
} EntityState;

void validateEntityStates(System *sP, EntityState *entityStateA) {
  assert(sP && entityStateA);
  Key nEntities = arrayGetNElems(entityStateA);
  assert(nEntities == frayGetNElems_(sP->cF));
  for (Key stateIdx = 0; stateIdx < nEntities; ++stateIdx) {
    EntityState entityState = entityStateA[stateIdx];
    assert(entityState.entity <= nEntities);
    XAComp *cP = (XAComp*) xGetCompPByEntity(sP, entityState.entity);
    assert(cP);
    // Make sure the mappings line up.
    Key *compIdxP = (Key*) mapGet(sP->e2cIdxMP, entityState.entity);
    assert(compIdxP && *compIdxP < nEntities);
    assert(sP->cIdx2eA[*compIdxP] == entityState.entity);
    // Make sure the entity's state corresponds to the last XMailCmd run on it.
    Key frayIdx = cP - (XAComp*) sP->cF;
    assert(frayIdx == *compIdxP);
    switch(entityState.state) {
      case N_XMAIL_BUILTIN_CMDS:
        break;  // just initialized,  no actions yet
      case PAUSE:
        assert(_frayElemIsPaused(sP->cF, frayIdx));
        break;
      case ACTIVATE:
      case UNPAUSE:
      case MUTATE_AND_ACTIVATE:
        assert(_frayElemIsActive(sP->cF, frayIdx));
        break;
      case DEACTIVATE:
      case MUTATE_AND_DEACTIVATE:
        assert(!_frayElemIsActive(sP->cF, frayIdx));
        assert(!_frayElemIsPaused(sP->cF, frayIdx));
        break;
      case MUTATE:  // expect no changes here
        break;
      default:
        assert(0);  // elem has invalid state
    }
  }
}

Error _mailboxWrite(EntityState *entityStateA, System *sP, Key address, Key attn, Key cmd, Key arg) {
  assert(entityStateA && attn && attn <= arrayGetNElems(entityStateA));
  entityStateA[attn - 1].state = cmd;
  return mailboxWrite(sAP->mailboxF, address, attn, cmd, arg);
}

static void _printFrayElem(U32 idx, Entity entity, XAComp *elem) {
  printf("\tEntity %d's component cF[%d] = {%3d, %3d, %3d, %5.1f}\n", entity, idx, elem->a, elem->b, elem->c, elem->d);
}

static Error update(char *prelude, U32 nComps, System *sP, EntityState *entityStateA) {
  printf("%s\n==================\n", prelude);
  printf("numebr paused: %d\n", *_frayGetNPausedP(sP->cF));
  Error e =  xRun(sP);
  U32 i = 0;
  Entity entity;
  if (!e) {
    printf("\tSYSTEM COMPONENT FRAY LAYOUT:\n");
    printf("\t----- active section -----\n");
    for (; i < _frayGetFirstPausedIdx(sP->cF); ++i) {
      entity = xGetEntityByCompIdx(sP, i);
      _printFrayElem(i, entity, &((XAComp*) sP->cF)[i]);
    }
    printf("\t----- paused section -----\n");
    for (; i < *_frayGetFirstInactiveIdxP(sP->cF); ++i) {
      entity = xGetEntityByCompIdx(sP, i);
      _printFrayElem(i, entity, &((XAComp*) sP->cF)[i]);
    }
    printf("\t----- inactive section -----\n");
    for (; i < *_frayGetFirstEmptyIdxP(sP->cF); ++i) {
      entity = xGetEntityByCompIdx(sP, i);
      _printFrayElem(i, entity, &((XAComp*) sP->cF)[i]);
    }
    printf("\n\n");
    printf("\tSYSTEM INDEX-TO-ENTITY ARRAY LAYOUT:\n");
    for (i = 0; i < arrayGetNElems(sP->cIdx2eA); ++i) {
      entity = sP->cIdx2eA[i];
      printf("\tsP->cIdx2eA[%d] = %d\n", i, sP->cIdx2eA[i]);
    }
    printf("\n\n");
    printf("\tSYSTEM ENTITY-TO_INDEX ARRAY LAYOUT:\n");
    for (Entity entity = 1; entity <= nComps; ++entity) {
      Key *idx = (Key*) mapGet(sP->e2cIdxMP, entity);
      printf("\tsP->e2cIdxMP[Entity = %d] = %d\n", entity, *idx);
    }
    printf("\n\n");
    validateEntityStates(sP, entityStateA);
  }
  else {
    printf("xRun returned %d\n", e);
  }
  return e;
}

// TODO write mailboxWRite() wrapper
int main(int argc, char **argv) {
  static const U32 N_COMPS = 5;
  static XAComp aComp = { .a = 1, .b = 2, .c = 3, .d = 4.0 };
  static Key MADE_UP_TRIGGER = 10;
  EntityState *entityStateA = NULL;
  // Initialize system.
  Error e = xIniSys(sAP, N_COMPS, NULL);
  // Populate system's components.
  if (!e) {
    for (Entity entity = 1; !e && entity <= N_COMPS; ++entity) {
      aComp.a *= entity;
      aComp.b *= entity;
      aComp.c *= entity;
      e =  xAddEntityData(sAP, entity, sAP->id, (void*) &aComp);
    }
  }
  // Make entity state array for validation
  if (!e) {
    e = arrayNew((void**) &entityStateA, sizeof(EntityState), N_COMPS);
  }
  if (!e) {
    for (Entity entity = 1; entity <= N_COMPS; ++entity) {
      entityStateA[entity -1].entity = entity;
      entityStateA[entity -1].state = N_XMAIL_BUILTIN_CMDS;
    }
  }
  // Make mutation map for all entities, assuming they're all the same.
  Map *mutationMP;
  if (!e) {
    e = mapNew(&mutationMP, RAW_DATA, sizeof(XAMutation), 1);
  }
  if (!e) {
    XAMutation mutation = {
      .cMutation = 40
    };
    e = mapSet(mutationMP, MADE_UP_TRIGGER, &mutation);
  }
  if (!e) {
    for (Entity entity = 1; !e && entity <= N_COMPS; ++entity) {
      e = xAddMutationMap(sAP, entity, mutationMP);
    }
  }
  if (!e) {
    e = update("Initial state", N_COMPS, sAP, entityStateA);
  }
  // Mutate entities 1 and 3.
  if (!e) {
    e = _mailboxWrite(entityStateA, sAP, sAP->id, 1, MUTATE_AND_ACTIVATE, MADE_UP_TRIGGER);
  }
  if (!e) {
    e = _mailboxWrite(entityStateA, sAP, sAP->id, 3, MUTATE_AND_ACTIVATE, MADE_UP_TRIGGER);
  }
  if (!e) {
    e = update("Mutating AND activating entities 1 and 3.", N_COMPS, sAP, entityStateA);
  }
  if (!e) {
    XAComp* c1P = (XAComp*) xGetCompPByEntity(sAP, 1);
    XAComp* c2P = (XAComp*) xGetCompPByEntity(sAP, 2);
    XAComp* c3P = (XAComp*) xGetCompPByEntity(sAP, 3);
    XAComp* c4P = (XAComp*) xGetCompPByEntity(sAP, 4);
    XAComp* c5P = (XAComp*) xGetCompPByEntity(sAP, 5);
    assert(c1P != NULL);
    assert(c2P != NULL);
    assert(c3P != NULL);
    assert(c4P != NULL);
    assert(c5P != NULL);
    assert(c1P->a == 40);
    assert(c1P->b == 80);
    assert(c1P->c == 40);
    assert(fabs(c1P->d - 40.0) < 0.000001);
  }

  // Activate two arbitrary components.
  if (!e) {
    e = _mailboxWrite(entityStateA, sAP, sAP->id, 2, ACTIVATE, 0);
  }
  if (!e) {
    e = _mailboxWrite(entityStateA, sAP, sAP->id, 4, ACTIVATE, 0);
  }
  if (!e) {
    e = update("After activating 2 and 4", N_COMPS, sAP, entityStateA);
  }
  if (!e) {
    assert(xGetEntityByCompIdx(sAP, 2) == 2);
    assert(xGetEntityByCompIdx(sAP, 3) == 4);
  }

  // Pause entity 4.
  if (!e) {
    e = _mailboxWrite(entityStateA, sAP, sAP->id, 4, PAUSE, 0);
  }
  if (!e) {
    e = update("After pausing entity 4", N_COMPS, sAP, entityStateA);
  }
  if (!e) {
    assert(xGetEntityByCompIdx(sAP, 2) == 2);
    assert(xGetEntityByCompIdx(sAP, 3) == 4);
  }

  // Activate 5.
  if (!e) {
    e = _mailboxWrite(entityStateA, sAP, sAP->id, 5, ACTIVATE, 0);
  }
  if (!e) {
    e = update("After activating entity 5", N_COMPS, sAP, entityStateA);
  }

  // Deactivate 3.
  if (!e) {
    e = _mailboxWrite(entityStateA, sAP, sAP->id, 3, DEACTIVATE, 0);
  }
  if (!e) {
    e = update("Deactivating entity 3", N_COMPS, sAP, entityStateA);
  }

  xClr(sAP);
  mapDel(&mutationMP);

  return e;
}
