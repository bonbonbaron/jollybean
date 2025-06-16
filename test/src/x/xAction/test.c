#include "test.h"

// Blackboard items
static DanceSkills goodGuyDanceSkills = {
  .spin = 3,
  .bop = 5,
  .hop = 12
};

static DanceSkills badGuyDanceSkills = {
  .spin = 4,
  .bop = 8,
  .hop = 2
};

static Stat goodguyStats = {
  .hp = 100,
  .strength = 40
};

static Stat badguyStats = {
  .hp = 50,
  .strength = 12
};

// Pointers are needed so blackboards store the pointers, not copies of the raw data.
DanceSkills* ggDanceSkillsP = &goodGuyDanceSkills;
DanceSkills* bgDanceSkillsP = &badGuyDanceSkills;
Stat*        ggStatsP       = &goodguyStats;
Stat*        bgStatsP       = &badguyStats;

ActionFuncDef_( takeDamage ) {
  assertAction_;
  bbGet( Stat, stat, STAT );
  U32 oldhp = statP->hp;
  statP->hp -= activityP->amount.u32;
  printf("%d takes %d damage. HP falls from %d to %d. ", entity, activityP->amount, oldhp, statP->hp );
  if ( statP->hp <= 0 ) {
    printf(" He's dead!");
  }
  printf("\n");
  activityP->complete = 1;
}

ActionFuncDef_( dance ) {
  assertAction_;
  bbGet( DanceSkills, danceSkills, DANCE_SKILLS );
  printf( "Entity %d spins with %d and bops with %d!\n", entity, danceSkillsP->spin, danceSkillsP->bop );
}

ActionFuncDef_( hit ) {
  assertAction_;
  // bad linle:  ) == sizeof( Stat*
  bbGet( Stat, stats, STAT );
  printf("%d hits %d with %d strength.\n", entity, activityP->tgtEntity, statsP->strength );
  Generic g = { .u32 = statsP->strength };
  mailboxWrite( mailboxF, ACTION, activityP->tgtEntity, TAKE_DAMAGE, 0, &g );
  activityP->complete = 1;
}

Quirk q1 = { .actionU = hit, .priority = HIGH, .trigger = HIT };
Quirk q2 = { .actionU = takeDamage, .priority = HIGH, .trigger = TAKE_DAMAGE };
Quirk q3 = { .actionU = dance, .priority = MEDIUM, .trigger = DANCE };

Quirk* quirkPA[] = { &q1, &q2, &q3 };

int main(int argc, char **argv) {
  /* Prove that multiple entities can share quirks. */
  // PERSONALITY ( one personality that's shared by both entities )
  const U32  N_MUTATIONS_PER_ENTITY = sizeof( quirkPA ) / sizeof( quirkPA[0] );

  // Make only one personality that everybody can use.
  Personality personality = {
    .quirkPA = quirkPA,
    .nQuirks = N_MUTATIONS_PER_ENTITY
  };

  // BLACKBOARDS ( one per entity )
  Map* goodguyBbMP = mapNew( NONMAP_POINTER, sizeof( void* ), 2, GENERAL );
  Map* badguyBbMP = mapNew( NONMAP_POINTER, sizeof( void* ), 2, GENERAL );
  mapSet( goodguyBbMP, DANCE_SKILLS, &ggDanceSkillsP );
  mapSet( goodguyBbMP, STAT, &ggStatsP );
  mapSet( badguyBbMP, DANCE_SKILLS, &bgDanceSkillsP );
  mapSet( badguyBbMP, STAT, &bgStatsP );

  // An array of map pointers
  Map* bbMPA[] = { goodguyBbMP, badguyBbMP };

  // Array of arrays of pointers
  const U32 N_ENTITIES = 2;

  xIniSys(sActionP, N_ENTITIES, NULL);
  // Add the subcomponents ( personalities and blackboards ).
  for (Entity entity = 1; entity <= N_ENTITIES; ++entity) {
    // Give this entity a component. We'll make the first quirk it.
    xAddEntityData(sActionP, entity, ACTION | PERSONALITY, &personality);
    xAddEntityData(sActionP, entity, ACTION | BLACKBOARD, bbMPA[ entity - 1 ]);  // bend some rules
  }
  // Make the mutation maps and put them in the system's map of mutation maps.
  for (Entity entity = 1; entity <= N_ENTITIES; ++entity) {
    Map *innerMP = mapNew(NONMAP_POINTER, sizeof(Quirk*), N_MUTATIONS_PER_ENTITY, GENERAL);
    for (Key quirkIdx = 0; quirkIdx < N_MUTATIONS_PER_ENTITY; ++quirkIdx) {
      mapSet(
          innerMP, 
          personality.quirkPA[quirkIdx]->trigger,
          (void*) &personality.quirkPA[quirkIdx]);
    }
    // Put mutation map into system's map of mutation maps.
    xAddMutationMap(sActionP, entity, innerMP);
  }

  sActionP->postprocessComps( sActionP );

  mailboxWrite( sActionP->mailboxF, ACTION, 1, HIT, 2, NULL );
  xRun(sActionP); // Run once for entity 1 to hit entity 0.
  xRun(sActionP); // Run again for entity 2 to experience the pain. lol
  mailboxWrite( sActionP->mailboxF, ACTION, 1, HIT, 2, NULL );
  mailboxWrite( sActionP->mailboxF, ACTION, 1, DANCE, 0, NULL );
  xRun(sActionP); // Run once more for 1 to deliver final blow to 2.
  xRun(sActionP); // Run one last time for 2 to die.

  // Then test hiveminds by letting everybody, even the dead guy, dance.
  mailboxWrite( sActionP->mailboxF, ACTION, 0, DANCE, 0, NULL );
  xRun(sActionP); 

  return 0;
}
