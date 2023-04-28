#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <png.h>
#include <assert.h>
#include <string.h>
#include "common.h"
#include "json.h"
#include "fileUtils.h"
#include "xCollision.h"

#define TEST_ 1

Error coll(char *fp, char *entityName, U8 isBg, Animation *animP, U8 verbose);
// We color collidable objects' collision rectangles/grids.
// That way we know how someone should react when bumping into them.
