#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "x/xAction.h"
#include "jb.h"

typedef enum {DANCE_SKILLS = 1, STAT} BbKey;
typedef enum {HIGH, MEDIUM, LOW} Priority;
typedef enum {HIT = N_XMAIL_BUILTIN_CMDS, TAKE_DAMAGE, DANCE} Trigger;

typedef enum { GOOD_GUY = 1, BAD_GUY } GuyEnum;

typedef struct Stat {
  S32 hp;
  U32 strength;
} Stat;

typedef struct DanceSkills {
  U32 spin;
  U32 bop;
  U32 hip;
  U32 hop;
} DanceSkills;


