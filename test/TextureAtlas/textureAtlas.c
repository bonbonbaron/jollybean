#include "xRender.h"
#include "blehColormap.h"
#include "heckColormap.h"
#include "redColormap.h"


/* Coding Kaizen:
 *  1) Code only a little bit at a time and test it.
 */

int main(int argc, char **argv) {
  Colormap *colormapPA[] = {
    &blehColormap,
    &heckColormap,
    &redColormap
  };

  for (U32 i = 0; i < sizeof(colormapPA) / sizeof(colormapPA[0]); ++i) {
    printf("HI!\n");
  }
  

  return 0;
}
