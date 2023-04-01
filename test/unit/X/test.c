#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "x.h"
#include "xA.h"

static void _printFrayElem(U32 idx, Entity entity, XAComp *elem) {
  printf("\tEntity %d's component cF[%d] = {%3d, %3d, %3d}\n", entity, idx, elem->a, elem->b, elem->c);
}

static Error reportAndRun(char *prelude, U32 nComps, System *sP) {
  printf("%s\n==================\n", prelude);
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
  }
  return e;
}

int main(int argc, char **argv) {
  static const U32 N_COMPS = 5;
  static XAComp aComp = { .a = 1, .b = 2, .c = 3 };
  // Initialize system.
  Error e = xIniSys(sAP, N_COMPS, NULL);
  // Populate system's components.
  if (!e) {
    for (U32 i = 1; !e && i <= N_COMPS; ++i) {
      aComp.a *= i;
      aComp.b *= i;
      aComp.c *= i;
      e = xAddComp(sAP, i, &aComp);
    }
  }

  // Activate two arbitrary components.
  if (!e) {
    e = xActivateComponentByEntity(sAP, 2);
  }
  if (!e) {
    e = xActivateComponentByEntity(sAP, 4);
  }
  if (!e) {
    e = reportAndRun("After activating 2 and 4", N_COMPS, sAP);
  }
  if (!e) {
    assert(xGetEntityByCompIdx(sAP, 0) == 2);
    assert(xGetEntityByCompIdx(sAP, 1) == 4);
  }

  // Pause entity 4.
  if (!e) {
    e = xPauseComponentByEntity(sAP, 4);
  }
  if (!e) {
    e = reportAndRun("After pausing entity 4", N_COMPS, sAP);
  }
  if (!e) {
    assert(xGetEntityByCompIdx(sAP, 0) == 2);
    assert(xGetEntityByCompIdx(sAP, 1) == 4);
  }

  // Activate 5.
  if (!e) {
    e = xActivateComponentByEntity(sAP, 5);
  }
  if (!e) {
    e = reportAndRun("After activating entity 5", N_COMPS, sAP);
  }

  return e;
}
