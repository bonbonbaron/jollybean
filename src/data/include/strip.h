#ifndef STRIP_H
#define STRIP_H

#include "array.h"
#include "inflatable.h"

// Strip Data
typedef U16 StripmapElem;  // Stripmap's inflated data is in U16 format.

typedef struct {
  U8 nUnitsPerStrip;
  U8 bpu;  // bits per unit
  U8 offset;  // amount to increment all units by (e.g. for sub-palette in atlas color palette)
  U32 nUnits;  // e.g. number of pixels in entire stripset
  Inflatable *infP;  // strip set's compressed source data
  U8 *unpackedDataP;  // destination of unpacking step in strip data inflation, if not skipped
} Stripset;

typedef struct {
  U32 nIndices;
  Inflatable *infP;  // infP->inflatedDataP contains stripmap.
} Stripmap;

// Strip Data flags
// prevents biome from double-counting an SD when determining SD array size for inflation
#define SD_IS_COUNTED_        (0x00000001)
// prevents biome from double-inserting an SD into the array for inflation
#define SD_SET_FOR_INFLATION_ (0x00000002)
/* The rest of these are self-explanatory. 
 * For example, systems like XRender want to skip the assembling step so it can assemble them
 * straight into its texture instead. */
#define SD_SKIP_INFLATION_    (0x00000004)
#define SD_SKIP_UNPACKING_    (0x00000008)
#define SD_SKIP_ASSEMBLY_     (0x000000f0)

typedef struct {
  U32 flags;     // used to indicate distinct histo inclusion, flags to skip inflation steps, etc.
  Stripmap sm;
  Stripset ss;
  U8 *assembledDataA;  // destination of strip data's assembling step, if not skipped
} StripDataS;

void stripClr(StripDataS *sdP);
Error stripIni(StripDataS *sdP);
Error sdInflate(StripDataS *sdP);
Error sdUnpack(StripDataS *sdP);
Error sdAssemble(StripDataS *sdP);

#endif  // #ifndef STRIP_H
