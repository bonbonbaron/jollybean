#include "read-png.h"

#define DEBUG_IMG_ (0)

unsigned char readPng(char *imgPathA, Image *imgP, int verbose) {
  if (!imgPathA || !imgP) {
    return 1;
  }

#if DEBUG_IMG_
  FILE *fP = fopen(imgPathA, "rb");
  if (fP && verbose) {
    fseek(fP, 0, SEEK_END);
    printf("PNG: original size is %d bytes.\n", ftell(fP));
    fclose(fP);
  }
#endif

  png_image *pngP = NULL;
  // Allocate PNG image info
  Error e = jbAlloc((void**) &pngP, sizeof(png_image), 1);
  // Set up PNG reader.
  if (!e) {
    memset(pngP, 0, sizeof(png_image));  // bombed, seeing if this worked
    pngP->version = PNG_IMAGE_VERSION;
    int e = png_image_begin_read_from_file(pngP, imgPathA);
    e = !e ? 1 : 0;
    // Handle errors if any
    if (pngP->warning_or_error) {
      printf("libpng error message: %s\n", pngP->message);
      printf("related to file %s.\n", imgPathA);
    }
    else if (e) {  // in this context, !e is an error (libpng way of doing it)
      printf("png_image_begin_read() errored with e = %d.\n", e);
      if (pngP->message[0]) {
        printf("%s\n", pngP->message);
      }
    }
  }
  // No errors. Perform actual read.
  if (!e) {
    imgP->pixelSz = PNG_IMAGE_PIXEL_SIZE(pngP->format);
    imgP->pitch = PNG_IMAGE_ROW_STRIDE(*pngP);
    unsigned bufferSz = PNG_IMAGE_BUFFER_SIZE(*pngP, imgP->pitch);  // number of bytes whole src image
    imgP->w = imgP->pitch / imgP->pixelSz;
    imgP->h = bufferSz / imgP->pitch;

    if (verbose) {
      printf("src img pitch: %d\n", imgP->pitch);
      printf("src img size in bytes: %d\n", bufferSz);
      printf("src img pixel size: %d\n", imgP->pixelSz);
    }

    e = arrayNew((void**) &imgP->dataP, sizeof(Color), imgP->w * imgP->h);
    assert(bufferSz == imgP->w * imgP->h * sizeof(Color));
    // In PNGs that're already colormaps, we go ahead and populate the color palette here.
    if (!e) {
      // Actual read happens here. row_stride param being 0 forces libpng to calculate the pitch for you.
      e = png_image_finish_read(pngP, NULL, imgP->dataP, 0, NULL);
      e = !e ? 1 : 0;
      // mallocSanityCheck("after finishing reeading PNG 2");
    }
#if DEBUG_IMG_
    if (!e & verbose) {
      Color *pixelP = (Color*) imgP->dataP;
      Color *pixelEndP = (Color*) imgP->dataP + arrayGetNElems(imgP->dataP);
      assert ((pixelEndP - pixelP) == (bufferSz / imgP->pixelSz));
      assert ((pixelEndP - pixelP) == (imgP->w * imgP->h));
      for (; pixelP < pixelEndP; ++pixelP) {
        printf("{%d, %d, %d}, ", pixelP->r, pixelP->g, pixelP->b);
      }
    }
#endif
  }

  if (pngP->warning_or_error) {
    printf("Closing warning or error: %s\n", pngP->message);
  }


  if (!pngP->warning_or_error && verbose) {
    printf("PNG image load success: %d x %d \n", pngP->width, pngP->height);
  }

  jbFree((void**) &pngP);

  return e;  // libpng errors are weird.
}

