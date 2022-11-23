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

// We color collidable objects' collision rectangles/grids.
// That way we know how someone should react when bumping into them.
