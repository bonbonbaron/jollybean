#ifndef COLL_H
#define COLL_H
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <png.h>
#include <assert.h>
#include <string.h>
#include "data.h"
#include "json.h"
#include "anim.h"
#include "fileUtils.h"
#include "xCollision.h"

#define TEST_ 1

Error coll(char *fp, char *entityName, U8 isBg, AnimJsonData *animP, U8 verbose);
// We color collidable objects' collision rectangles/grids.
// That way we know how someone should react when bumping into them.
#endif
