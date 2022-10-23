#include <stdio.h>

typedef unsigned char U8;

/* Most TMUs require images to be arranged in memory in T- or LT- format. These formats
 * have a sequence of Micro-tiles (M).

/* (M)s are rectangular image blocks w/ fixed size of 64 bytes' worth of pixels in raster order. 
 *   Since they're 64 bytes, their sizes are as follows:
 *      - 64bpp: 02x04 pixels
 *      - 32bpp: 04x04 pixels
 *      - 16bpp: 08x04 pixels
 *      + 08bpp: 08x08 pixels  <-- This'll be the story for nearly every atlas I use.
 *      - 04bpp: 16x08 pixels
 *      - 01bpp: 32x16 pixels
 *
 * They come in T- (T) and LT-format (L).
 *
*/

/* (T) consists of 4kB tiles of 2D images, formed from 1kB sub-tiles. 
 *     In our 8bpp style, a 4kB tile is 64x64 pixels.
 *     The 1kB sub-tiles are 32x32 pixels.
 */

int main(int argc, char **argv) {
  U8 u8A[8192] = {0};
  // first populate it
#ifdef __ARM_NEON__
  printf("arm neon is defined\n");
#else
  printf("no arm neon\n");
#endif
  for (int i = 0; i < 8192; ++i)
    u8A[i] = i;
  // then see if you can do simd on just one slice of it
  for (int i = 0; i < 8192; ++i)
    printf("%d ", u8A[i]);

  printf("\n\n");

  return 0;
}
