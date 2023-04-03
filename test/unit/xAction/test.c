#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "xAction.h"
#include "jb.h"

// Mutations
Error do1a(Entity entity, Map *bbMP) { printf("did 1a\n"); }
Error do1b(Entity entity, Map *bbMP) { printf("did 1b\n"); }
Error do1c(Entity entity, Map *bbMP) { printf("did 1c\n"); }
Error do2a(Entity entity, Map *bbMP) { printf("did 2a\n"); }
Error do2b(Entity entity, Map *bbMP) { printf("did 2b\n"); }
Error do2c(Entity entity, Map *bbMP) { printf("did 2c\n"); }
Error do3a(Entity entity, Map *bbMP) { printf("did 3a\n"); }
Error do3b(Entity entity, Map *bbMP) { printf("did 3b\n"); }
Error do3c(Entity entity, Map *bbMP) { printf("did 3c\n"); }

typedef enum {HIGH, MEDIUM, LOW} Priority;
typedef enum {TRIG1 = 1, TRIG2, TRIG3} Trigger;

#define N_MUTATIONS_PER_ENTITY (3)

int main(int argc, char **argv) {
  // Make dem quirkz boyz. Make 'em nice 'n' juiceh.
  Quirk qA[][3] = { 
    {
      { .actionU = do1a, .priority = HIGH,   .trigger = TRIG1 },
      { .actionU = do1a, .priority = MEDIUM, .trigger = TRIG2 },
      { .actionU = do1a, .priority = LOW,    .trigger = TRIG3 },
    },
    {
      { .actionU = do2a, .priority = HIGH,   .trigger = TRIG1 },
      { .actionU = do2a, .priority = MEDIUM, .trigger = TRIG2 },
      { .actionU = do2a, .priority = LOW,    .trigger = TRIG3 },
    },
    {
      { .actionU = do3a, .priority = HIGH,   .trigger = TRIG1 },
      { .actionU = do3a, .priority = MEDIUM, .trigger = TRIG2 },
      { .actionU = do3a, .priority = LOW,    .trigger = TRIG3 },
    }
  };

  const U32 N_ENTITIES = sizeof(qA) / sizeof(qA[0]);

  Error e = xIniSys(sActionP, N_ENTITIES, NULL);
  for (Entity entity = 1; !e && entity <= N_ENTITIES; ++entity) {
    // Give this entity a component. We'll make the first quirk it.
    e = xAddEntityData(sActionP, entity, ACTION, &qA[entity][0]);
  }
  // Make the mutation maps and put them in the system's map of mutation maps.
  for (Entity entity = 1; !e && entity <= N_ENTITIES; ++entity) {
    Map *innerMP = NULL;
    e = mapNew(&innerMP, RAW_DATA, sizeof(Quirk), N_MUTATIONS_PER_ENTITY);
    for (Key quirkIdx = 0; !e && quirkIdx < N_MUTATIONS_PER_ENTITY; ++quirkIdx) {
      if (!e) {
        e = mapSet(
              innerMP, 
              qA[entity - 1][quirkIdx].trigger,
              (void*) &qA[entity - 1][quirkIdx]);
      }
    }
    // Put mutatinon into system's map of mutation maps.
    if (!e && innerMP) {
      e = xAddMutationMap(sActionP, entity, innerMP);
    }
  }

  // Wake everybody up.
  if (!e) {
    _frayActivateAll(sActionP->cF);
    e = xRun(sActionP);
  }

  xClr(sActionP);
  return e;
}
