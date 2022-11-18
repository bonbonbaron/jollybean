#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <png.h>
#include <assert.h>
#include <string.h>
#include "data.h"
#include "json.h"
#include "fileUtils.h"
#include "xCollision.h"

#define TEST_ 1

const char LOCAL_TROVE_ANIM_DIR[] = "/.jb/src/Collision/";
const char LOCAL_TROVE_GENE_DIR[] = "/.jb/src/Gene/";
const char COLORCOLLMAPPING_FILENAME = "ColorCollDirectory";
// We color collidable objects' collision rectangles/grids.
// That way we know how someone should react when bumping into them.

typedef struct {
  char collType[28];  // Jollybean only supports 256 types of collisions.
  U32 color;
} SavedColorCollisionMapping;

typedef struct {
  U8 nSavedColorCollisionMappings;
  ColorCollisionMapping colorCollMappingA[];
}
