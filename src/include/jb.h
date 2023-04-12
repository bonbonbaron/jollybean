#ifndef JB_
#define JB_

// Core
#include "data.h"
#include "x.h"
#include "animKeyring.h"
// Core jb objects
#include "interface.h"
// Child systems
#include "xMaster.h"
#include "xRender.h"
#include "xAnim.h"
#if 0  // add back in later after fixing mess
#include "xTimer.h"
#include "xMotionTrans.h"
#include "xCollision.h"
#endif

// Keys  (hopefully obsolete later)
typedef enum {UPDATE_RECT = N_XMAIL_BUILTIN_CMDS, ANIMATE} MessageCommand;
typedef enum {EXCLUSIVE_GENE, MEDIA_GENE, SHARED_GENE, BB_GENE, COMPOSITE_GENE, IMPLICIT_GENE} GeneClass;
typedef enum {MASTER = 1, ACTION, RENDER, ANIMATION, TIMER, COLLISION, N_SYSTEM_TYPES} ExclusiveGeneType; 
// Make sure you don't get rid of the 3! 2 belongs to Master.
typedef enum {GUI_GENE_TYPE = 1, SRC_RECT, RECT_OFFSET, DST_RECT, SCALE, N_SHARED_GENE_TYPES} SharedGeneType;
typedef enum {BODY = N_SHARED_GENE_TYPES, GRAYBODY, PALETTE, ANIMATION} CompositeGeneType;
#endif
