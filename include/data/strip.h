#ifndef STRIP_H
#define STRIP_H

#include "data/array.h"
#include "data/inflatable.h"

// Strip Data
typedef U16 StripmapElem;  // Stripmap's inflated data is in U16 format.

typedef struct {
  U8 nUnitsPerStrip;
  U8 bpu;  // bits per unit
  U8 offset;  // amount to increment all units by (e.g. for sub-palette in atlas color palette)
  U32 nUnits;  // e.g. number of pixels in entire stripset
  Inflatable *infP;  // strip set's compressed source data
  U8 *unpackedDataA;  // destination of unpacking step in strip data inflation, if not skipped
} Stripset;

typedef struct {
  U32 nIndices;
  Inflatable *infP;  // infP->inflatedDataP contains stripmap.
} Stripmap;

// Strip Data flags
// prevents gene.c from double-inserting an SD into the array for inflation
#define SD_SET_FOR_INFLATION_ (0x01)
/* The rest of these are self-explanatory. 
 * For example, systems like XRender want to skip the assembling step so it can assemble them
 * straight into its texture instead. */
#define SD_SKIP_INFLATION_    (0x02)
#define SD_SKIP_UNPACKING_    (0x04)
#define SD_SKIP_ASSEMBLY_     (0x08)

typedef struct {
  // 2-byte gene hdr is expected to go here for each media gene; separating this struct from it helps
  // us to make a pointer to it more easily though.
  U8 flags;     // used to indicate distinct histo inclusion, flags to skip inflation steps, etc.
  Stripmap sm;
  Stripset ss;
  U8 *assembledDataA;  // destination of strip data's assembling step, if not skipped
} StripDataS;

void stripIni(StripDataS *sdP, const PoolId poolId);
U8* ssGetInput( StripDataS* sdP );
U8* ssGetOutput( StripDataS* sdP );
void sdInflate(StripDataS *sdP, const PoolId poolId);
void sdUnpack(StripDataS *sdP, const PoolId poolId);
void sdAssemble(StripDataS *sdP, const PoolId poolId);

#endif  // #ifndef STRIP_H
