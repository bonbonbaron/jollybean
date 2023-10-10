#include "strip.h"

// Stripmapped Data
void stripClr(StripDataS *sdP) {
  if (sdP) {
    inflatableClr(sdP->ss.infP);
    inflatableClr(sdP->sm.infP);
    arrayDel((void**) &sdP->ss.unpackedDataP);
    arrayDel((void**) &sdP->assembledDataA);
  }
}

// This is for when I vectorize (again) in the future for the IMPROVED stripset unpacking algorithm.
#if 0
// TODO delete everything below once bug is fixed
// Stripped data inflation
#ifdef __ARM_NEON__
__inline__ static void _unpackStrip4Bpu(U32 **srcStripPP, U32 **dstStripPP) {
  U32 *srcStripP = *srcStripPP;
  U32 *dstStripP = *dstStripPP;
  /* 6 instructions neon VS 40-58 instructions regular */
  /* Although the outer loop appears unnecessary for 1 quadword per 1Bpu strip,
     it safeguards us from changes in the number of units per strip. */
  for (int i = 0; i < N_QUADWORDS_PER_4BPU_STRIP; ++i) {  /* keeping this useless loop here for when I chagne to 128-unit strips. */
    asm volatile inline (
    "vmov.u8 q10, #" #maskByte_ "nt"   /* q10 = mask */
    "vld1.32 {d0-d1}, [%0]!nt"    /* q0 (aka d0-d1) = packed indices */
    : "+r&" (srcStripP)
    );
    for (int j = 0; j < N_4BPU_UNITS_PER_BYTE; ++j) {
      asm volatile inline (
      "vand q1, q0, q10nt"          /* q1 = unpacked indices*/
      "vst1.32 {d2-d3}, [%0]!nt"
      "vshr.u8 q0, #" #Bpu_ "nt"            /* shift q0 over 1*/
      : "+r&" (dstStripP)
      );
    }
  }
}
#endif
#endif

Error sdInflate(StripDataS *sdP) {
  if (sdP->flags & SD_SKIP_INFLATION_) {
    return SUCCESS;
  }
  Error e = inflatableIni(sdP->ss.infP);
  // Skipping assembly means no map is used.
  if (!e && !(sdP->flags & SD_SKIP_ASSEMBLY_) )  {
    e = inflatableIni(sdP->sm.infP);
  }
  return e;
}

// Unpack bits to reconstruct original data (uesd for debugging img.c)
Error sdUnpack(StripDataS *sdP) {
  if (sdP && sdP->flags & SD_SKIP_UNPACKING_) {
    return SUCCESS;
  }

  if (!sdP || (sdP->ss.infP && !sdP->ss.infP->compressedDataA)) {
    return E_BAD_ARGS;
  }

  Stripset *ssP = &sdP->ss;
  // Packed data is all whole words. Unpacked may not be.
  // Only way to tell is by looking at the number of units.
  const U32 nPackedUnitsPerWord     = N_BITS_PER_WORD / ssP->bpu;
  U32 nUnitsInExtraPackedWord = ssP->nUnits % nPackedUnitsPerWord; 

  // start copy
  Error e = arrayNew((void**) &ssP->unpackedDataP, sizeof(U8), ssP->nUnits);
  if (e) {
    return e;
  }

  U32 mask = 0;
  switch(ssP->bpu) {
    case 1:
      mask = 0x01010101;
      break;
    case 2:
      mask = 0x03030303;
      break;
    case 4:
      mask = 0x0f0f0f0f;
      break;
    default:
      return e;
  }

  const U32 offset = (sdP->ss.offset << 24) |
                     (sdP->ss.offset << 16) |
                     (sdP->ss.offset <<  8) |
                     (sdP->ss.offset      );

  U32 *packedWordP;
  // If the data was never compressed, then we're going to pull it from the "compressed" field.
  // It's just a safe place to put data that otherwise would get deleted by stripClr().
  if ( sdP->flags & SD_SKIP_INFLATION_ ) {
    packedWordP = (U32*) ssP->infP->compressedDataA;  // storing here is a trick to avoid new data fields
  }
  else {  // But if it really IS inflated, pull the inflated data.
    packedWordP = (U32*) ssP->infP->inflatedDataP;
  }
  U32 *packedWordEndP = (U32*) ((U8*) ssP->infP->inflatedDataP + (ssP->infP->inflatedLen))
                             - (nUnitsInExtraPackedWord > 0);  // stop short of partially packed word
  U32 *dstUnpackedWordP    = (U32*) ssP->unpackedDataP;

  U32 j;
  // Unpack whole words with reckless abandon (CRAZY GORILLA MODE)
  for (; packedWordP < packedWordEndP; ++packedWordP) {
    for (j = 0; j < N_BITS_PER_BYTE; j += ssP->bpu) {
      *(dstUnpackedWordP++) = ((*packedWordP >> j) & mask) + offset;
    }
  }
  // While theres >= 4 units left in packed word...
  for (j = 0; 
       nUnitsInExtraPackedWord >= sizeof(U32); 
       nUnitsInExtraPackedWord -= sizeof(U32), j += ssP->bpu) {
    *(dstUnpackedWordP++) = ((*packedWordP >> j) & mask) + offset;
  }
  // Fewer than 4 packed units remaining in last word
  // If last word has fewer than 4 units remaining, carefully extract them into < 4 output bytes.
  if (!e && nUnitsInExtraPackedWord > 0) {
    U32 lastUnpackedWord = ((*packedWordP >> j) & mask) + offset;
    memcpy((void*) dstUnpackedWordP, &lastUnpackedWord, nUnitsInExtraPackedWord);
  }

  return e;
}

Error sdAssemble(StripDataS *sdP) {
  if ( sdP && sdP->flags & SD_SKIP_ASSEMBLY_) {
    return SUCCESS;
  }

  if (!sdP || !sdP->ss.unpackedDataP || !sdP->sm.infP->inflatedDataP || sdP->assembledDataA
      || sdP->ss.nUnitsPerStrip == 0) {
    return E_BAD_ARGS;
  } 

  Error e = arrayNew((void**) &sdP->assembledDataA, sizeof(U8), sdP->sm.nIndices * sdP->ss.nUnitsPerStrip);
  
  // Piece together strips
  if (!e) {
    // If the data was never compressed, then we're going to pull it from the "compressed" field.
    // It's just a safe place to put data that otherwise would get deleted by stripClr().
#if 0
    if ( sdP->flags & SD_SKIP_INFLATION_ ) {
      packedWordP = (U32*) ssP->infP->compressedDataA;  // storing here is a trick to avoid new data fields
    }
    else {  // But if it really IS inflated, pull the inflated data.
      packedWordP = (U32*) ssP->infP->inflatedDataP;
    }
#endif
    StripmapElem *smElemP = sdP->sm.infP->inflatedDataP;
    StripmapElem *smElemEndP = smElemP + sdP->sm.nIndices;
    U8 *assembledDataP = sdP->assembledDataA;
    for (; smElemP < smElemEndP; ++smElemP, assembledDataP += sdP->ss.nUnitsPerStrip) {
      memcpy(assembledDataP,
             sdP->ss.unpackedDataP + (*smElemP * sdP->ss.nUnitsPerStrip),
             sdP->ss.nUnitsPerStrip);
    }
  }

  return e;
}

// This is the single-threaded version of inflating stripd data.
Error stripIni(StripDataS *sdP) {
  Error e = sdInflate(sdP);
  if (!e) {
    e = sdUnpack(sdP);
  }
  if (!e) {
    e = sdAssemble(sdP);
  }
  return e;
}

