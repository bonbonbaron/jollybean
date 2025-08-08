#ifndef JB_H
#define JB_H

#include "x/x.h"

// Keys  (hopefully obsolete later)
typedef enum {UPDATE_RECT = N_XMAIL_BUILTIN_CMDS, ANIMATE, MSG_SOFT_COLLISIONS_DETECTED, MSG_MOVE_UP_A_LAYER, MSG_MOVE_DOWN_A_LAYER, MSG_MOVE_TO_LAYER} MessageCommand;
typedef enum {MASTER = 1, ACTION, RENDER, MOTION, ANIMATION, TIMER, COLLISION, N_SYSTEM_TYPES} ExclusiveGeneType; 
typedef enum {GUI_GENE_TYPE = 1, SRC_RECT, RECT_OFFSET, DST_RECT, SCALE, Z_HEIGHT, BLOB, N_SHARED_GENE_TYPES} SharedGeneType;
typedef enum {BODY = N_SHARED_GENE_TYPES, GRAYBODY, PALETTE} CompositeGeneType;
#endif
