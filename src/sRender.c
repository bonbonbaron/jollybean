#include "ecs.h"

#define IMG_DEBUG 0
#define FLIP_NONE  (0)
#define FLIP_UD    (1 << 14)
#define FLIP_LR    (2 << 14)
#define FLIP_DIAG  (3 << 14)

#define NUM_ROWS_IN_TILE (7)


// I'm sitting here right now wondering about this whole approach for tile-based games. Obviously a giant rectangle on a prepared image would be fastest, but as it is, we're doing a thousand (896 visible tiles + padding tiles) for the screen. Updating with up to a thousand tiles is necessary for the start of the level. But what scares me is the prospect of having to redraw them plus offsets every single time. That's going to be way too much computation. 

// Dispmanx can't xform tiles into a resource, so you have to xform tiles here. However, leverage what it *can* do: xforming resources into elements. Leverage that by having img.py eliminate xformationally duplicate animation frames. 

typedef enum {PIXEL_FORMAT_1BPP, PIXEL_FORMAT_4BPP} PixelFormat;

typedef struct {
	U16 w, h, nTiles, tileIdxA[];  // w and h are in units of tiles
} Tilemap;

typedef struct {
	U8 bpp;
	U16 w, h;  // in pixel units; determine actual step size by pixel format
	U8 pixelData[];
} Colormap;

Error colormapNew(Colormap **cmPP) {
	return jbAlloc(cmPP, sizeof(Colormap), 1);
}

void colormapDel(Colormap **cmPP) {
	jbFree(cmPP);
}

void colormapIni(Colormap *cmSrcP, Tilemap *tmP, Colormap **cmDstPP) {
	if (tmP != NULL) {
		U16  rowIdx             = 0;
		U16 *tileIdxP           = tmP;
		register U32 nTilesInDstRow = 0;

		// *******************************
		// Create 1bpp full-image colormap
		// *******************************
		if (cmSrcP->bpp == 1) {
			register U8 dstTileRow;
		  const register U32 dstStepNextRow = (cmDstP->w * cmDstP->bpp) / 8;
		  const U32 srcStepNextRow = (cmSrcP->w * cmSrcP->bpp) / 8;
		  const U32 srcStepSevenRows = 7 * srcStepNextRow;
			U8 *cmSrcRowP;
			// For each row of every tile in destination colormap...
			for (U8 *cmDstRowP = (U8*) &cmDstP->pixelData[0]; cmDstRowP < cmDstP->pixelDataEndP; nTilesInDstRow++) {
				switch(*tileIdxP & 0xC000) {  // orientation of tile
					case FLIP_NONE:  
						cmSrcRowP  = ((U8*) &cmSrcP->pixelData[0]) + (*tileIdxP & 0x3FFF);
						*cmDstRowP = *cmSrcRowP;  cmDstRowP += dstStepNextRow;  cmSrcRowP += srcStepNextRow;
						*cmDstRowP = *cmSrcRowP;  cmDstRowP += dstStepNextRow;  cmSrcRowP += srcStepNextRow;
						*cmDstRowP = *cmSrcRowP;  cmDstRowP += dstStepNextRow;  cmSrcRowP += srcStepNextRow;
						*cmDstRowP = *cmSrcRowP;  cmDstRowP += dstStepNextRow;  cmSrcRowP += srcStepNextRow;
						*cmDstRowP = *cmSrcRowP;  cmDstRowP += dstStepNextRow;  cmSrcRowP += srcStepNextRow;
						*cmDstRowP = *cmSrcRowP;  cmDstRowP += dstStepNextRow;  cmSrcRowP += srcStepNextRow;
						*cmDstRowP = *cmSrcRowP;  cmDstRowP += dstStepNextRow;  cmSrcRowP += srcStepNextRow;
						*cmDstRowP = *cmSrcRowP;  cmDstRowP += dstStepNextRow;  
						break;
					case FLIP_UD:  
						cmSrcRowP = ((U8*) &cmSrcP->pixelData[0]) + (*tileIdxP & 0x3FFF) + srcStepSevenRows;  // start of tile's last row
						*cmDstRowP = *cmSrcRowP;  cmDstRowP += dstStepNextRow;  cmSrcRowP -= srcStepNextRow;
						*cmDstRowP = *cmSrcRowP;  cmDstRowP += dstStepNextRow;  cmSrcRowP -= srcStepNextRow;
						*cmDstRowP = *cmSrcRowP;  cmDstRowP += dstStepNextRow;  cmSrcRowP -= srcStepNextRow;
						*cmDstRowP = *cmSrcRowP;  cmDstRowP += dstStepNextRow;  cmSrcRowP -= srcStepNextRow;
						*cmDstRowP = *cmSrcRowP;  cmDstRowP += dstStepNextRow;  cmSrcRowP -= srcStepNextRow;
						*cmDstRowP = *cmSrcRowP;  cmDstRowP += dstStepNextRow;  cmSrcRowP -= srcStepNextRow;
						*cmDstRowP = *cmSrcRowP;  cmDstRowP += dstStepNextRow;  cmSrcRowP -= srcStepNextRow;
						*cmDstRowP = *cmSrcRowP;  cmDstRowP += dstStepNextRow;  
						break;
					case FLIP_LR: 
						for (U8 i = 0; i < 8; i++) {
							cmSrcRowP = ((U8*) &cmSrcP->pixelData[0]) + (*tileIdxP & 0x3FFF);
							dstTileRow = *cmSrcRowP;
							dstTileRow = ((dstTileRow & 0x0F) << 4) | ((dstTileRow & 0xF0) >> 4);  // If counting to 8 winds up being slow, bite the bullet and do loopless expansion like FLIP_NONE and FLIP_UD.
							dstTileRow = ((dstTileRow & 0x33) << 2) | ((dstTileRow & 0xCC) >> 2);
							*cmDstRowP = ((dstTileRow & 0x55) << 1) | ((dstTileRow & 0xAA) >> 1);
							cmDstRowP += dstStepNextRow;  cmSrcRowP += srcStepNextRow;
						}
						break;
					default: // case FLIP_DIAG: 
						for (U8 i = 0; i < 8; i++) {
							cmSrcRowP = ((U8*) &cmSrcP->pixelData[0]) + (*tileIdxP & 0x3FFF) + srcStepSevenRows;  // start of tile's last row
							dstTileRow = *(cmSrcOriginP + ((*tileIdxP & 0x3FFF) * N_TILE_W_BYTES) + (((NUM_ROWS_IN_TILE - rowIdx) * cmSrcP->pitch)));
							dstTileRow = ((dstTileRow & 0x0F) << 4) | ((dstTileRow & 0xF0) >> 4);
							dstTileRow = ((dstTileRow & 0x33) << 2) | ((dstTileRow & 0xCC) >> 2);
							*cmDstRowP = ((dstTileRow & 0x55) << 1) | ((dstTileRow & 0xAA) >> 1);
							cmDstRowP += dstStepNextRow;  cmSrcRowP -= srcStepNextRow;
						}
						break;
				}
				// Step to next tile 
				if (nTilesInDstRow == tmP->w) {
					nTilesInDstRow = 0;
					cmDstRowP++;
				}  // otherwise, you've already stepped to the next tile by the above logic.
			}
		}
		// *******************************
		// Create 4bpp full-image colormap   //TODO re-code this after you've debugged 1bpp case.
		// *******************************
		else 
			return E_BPP_NEITHER_1_NOR_4;
	}
	else 
		*cmDstPP = cmSrcP;
	
}

/* metadata provies */
Error load_image() {
	Error e = SUCCESS;
	return e;
}
