#ifndef SYS_A
#define SYS_A
#include "x.h"

#define XA_TYPE (1)
#define MADE_UP_SHARE_TYPE (1)

/* STRUCTURE OF AN XA COMPONENT:
 *
 * The component has four subcomponents.
 *   A and D are immutable subcomponents;
 *   B and C are mutable shares.
 *
 * Therefore, we'll store a one-to-one mapping from each entity to its A and D subcomponents internal to the system,
 * and then we'll store a one-to-many mapping from each entity to its B and C subcomponents external from the system.
 *
 * B and C will also undergo postmutations to fully test that feature of X.
 *
 * TODO wait... how did i intend to mutate only one subcomponent at a time? 
 *      Looks like the mutation function mutates the whole component at once.
 *
 */

typedef struct {
  int a;      // raw subcomponent
  short b;    // from system's mutation map
  char c;     // from system's mutation map
  double d;   // raw subcomponent
  int *intP;  // from external share map
} XAComp;

typedef struct {
  short bb;
  char cc;
} XAMutation;

// Images
XIniSysFuncDef_(A);
XGetShareFuncDef_(A);
XIniSubcompFuncDef_(A);
XPostprocessCompsDef_(A);
XProcMsgFuncDef_(A);
XRunFuncDef_(A);
XClrFuncDef_(A);
XPostMutateFuncDef_(A);

typedef struct {
  System system;
  U32 someSystemwideU32;
  Map *aMP;
  Map *dMP;
  Map *sharedIntMP;
  Entity *entityF;
} XA;

extern System *sAP;
#endif
