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

// Keys  (hopefully obsolete later)
typedef enum {UPDATE_RECT = N_XMAIL_BUILTIN_CMDS, ANIMATE} MessageCommand;

// Gene enums
typedef enum {EXCLUSIVE_GENE, MEDIA_GENE, SHARED_GENE, COMPOSITE_GENE} GeneClass;
typedef enum {MASTER = 1, ACTION, RENDER, ANIMATION, TIMER, COLLISION, N_SYSTEM_TYPES} ExclusiveGeneType; 
typedef enum {GUI_GENE_TYPE = 1, SRC_RECT, RECT_OFFSET, DST_RECT, SCALE, N_SHARED_GENE_TYPES} SharedGeneType;
typedef enum {BODY = N_SHARED_GENE_TYPES, GRAYBODY, PALETTE} CompositeGeneType;
#endif
