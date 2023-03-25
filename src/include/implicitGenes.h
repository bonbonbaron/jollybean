#ifndef IMPLICIT_GENES_
#include "jb.h"

// Implicit genes can go with exclusive genes, which must be masked by MASK_COMPONENT_TYPE.

// I tried macro'ing the hell out of this, but it can't be done in a clean way.
// So instead I have to do it manually. I think it'll be rare enough to be fine though.

#define ig_(enumType_, sizeType_) { .size = sizeof(sizeType_), .type = enumType_ }
#define impGeneList_(name_, arrayName_) \
  ImplicitGenesList igl_##name_ = {\
    .nGenes = sizeof(arrayName_) / sizeof(arrayName_[0]),\
    .listA  = arrayName_\
  };
#define igList_(name_, ...) \
  ImplicitGene name_##ImplicitGeneA[] = {\
    __VA_ARGS__\
  };\
  impGeneList_(name_, name_##ImplicitGeneA);

#define igl_(name_) &igl_##name_

igList_(renderer, 
    ig_(DST_RECT, Rect_),
    ig_(SCALE, U32));    // scaling U32 is fixed point: 16 bits as decimal places
igList_(animation, 
    ig_(SRC_RECT, Rect_), 
    ig_(RECT_OFFSET, RectOffset));

// Global table
// It's VERY IMPORTANT to make sure these line up with their enum values. This will likely be a pain.
// TODO Probably a pain worth developing a tool for.
ImplicitGenesList* iglA[] = {
  NULL,             // empty (first system ID is 1)
  NULL,             // MASTER
  igl_(renderer),   // RENDERER
  igl_(animation),  // ANIMATION
  NULL,             // TIMER
  NULL,             // COLLISION
  NULL,             // the rest... TBD
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
};

#endif  // #if IMPLICIT_GENES_
