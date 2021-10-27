#include "sRender.h"

// Compressed images are already in memory in JB. 
// JB just reconstructs them from strips of staggered pixels. 
// Packed bits are staggered to allow JB to unpack 4 color-mapped pixels' bits at a time.

void imageClr(Image *imgP) {
	if (imgP != NULL && imgP->pixelA != NULL)
		jbFree((void**) &imgP->pixelA);
}

Error imageGen(Image *imgP) {
	Error e;
	register U32 dstFlip;
	register U32 srcStrip;

	if (imgP != NULL) 
		e = jbAlloc((void**) &imgP->pixelA, sizeof(U8), imgP->w * imgP->h);
	else
		e = E_BAD_ARGS;

	// Expand strip set into image.
	if (!e) {
		U32 *dstStripP = (U32*) imgP->pixelA;
		U16 *mapEndP = imgP->stripMapP->idxA + imgP->stripMapP->nIndices;
		switch (imgP->bpp) {
			//TODO: case 1:
			//TODO: case 2:
			case 4: 
				// First read all the strips into the target image.
				for (U16 *ssIdxP = imgP->stripMapP->idxA; ssIdxP < mapEndP; ssIdxP++) {
					srcStrip = imgP->stripSetP->stripA[*ssIdxP];
					*dstStripP++ =  srcStrip & 0x0F0F0F0F;
					*dstStripP++ = (srcStrip & 0xF0F0F0F0) >> 4;
				}
				
				// Then flip whatever strips need flipping. Remember data's already expanded to U8s!
				U16 *flipEndP = imgP->stripSetP->flipSetP->flipIdxA + imgP->stripSetP->flipSetP->nFlips;
				for (U16 *flipIdxP = imgP->stripSetP->flipSetP->flipIdxA; flipIdxP < flipEndP; flipIdxP++) {
					U32 *dstWordP = ((U32*) imgP->pixelA + *flipIdxP); 
					dstFlip = *dstWordP;
					dstFlip   = ((dstFlip & 0xFFFF0000) >> 16) | ((dstFlip & 0x0000FFFF) << 16);
					*dstWordP = ((dstFlip & 0xFF00FF00) >>  8) | ((dstFlip & 0x00FF00FF) <<  8);
				}

				break;

			default:  
				e = E_UNSUPPORTED_PIXEL_FORMAT;
				break;
		}
	}
	else 
		imageClr(imgP);

	return e;
}
