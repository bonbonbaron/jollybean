#include "tau/tau.h"
#include "strip.h"
#include "sd.h"

typedef struct Tau {
  U8 *raw1bppA;
  U8 *raw2bppA;
  U8 *raw4bppA;
  StripDataS *sd1bppP;
  StripDataS *sd2bppP;
  StripDataS *sd4bppP;
} Tau;

U8 rawData4bpp[] = {
  1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
  3, 15, 12, 2, 0, 10, 9, 10, 1, 98,
  1, 15, 13, 2, 0, 11, 9, 9, 1, 0,
  6, 15, 12, 3, 0, 12, 8, 9, 1, 99,
  5, 15, 12, 2, 0, 10, 8, 9, 1, 3,
  1, 15, 12, 2, 1, 11, 8, 9, 1, 0,
  1, 17, 12, 2, 0, 10, 8, 9, 1, 0,
  1, 15, 12, 2, 0, 11, 8, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  1, 15, 12, 2, 0, 30, 8, 10, 1, 0,
  1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
  1, 15, 12, 2, 0, 10, 8, 9, 2, 0,
  255, 35, 12, 2, 0, 10, 8, 9, 2, 0,
  5, 15, 12, 2, 0, 10, 8, 9, 1, 3,
  1, 15, 12, 2, 1, 11, 8, 9, 1, 0,
  1, 17, 12, 2, 0, 10, 8, 9, 1, 0,
  1, 15, 12, 2, 0, 11, 8, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
  3, 15, 12, 2, 0, 10, 9, 10, 1, 98,
  1, 15, 13, 2, 0, 11, 9, 9, 1, 0,
  6, 15, 12, 3, 0, 12, 8, 9, 1, 99,
  5, 15, 12, 2, 0, 10, 8, 9, 1, 3,
  1, 15, 12, 2, 1, 11, 8, 9, 1, 0,
  1, 17, 12, 2, 0, 10, 8, 9, 1, 0,
  1, 15, 12, 2, 0, 11, 8, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  1, 15, 12, 2, 0, 30, 8, 10, 1, 0,
  1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
  1, 15, 12, 2, 0, 10, 8, 9, 2, 0,
  255, 35, 12, 2, 0, 10, 8, 9, 2, 0,
  5, 15, 12, 2, 0, 10, 8, 9, 1, 3,
  1, 15, 12, 2, 1, 11, 8, 9, 1, 0,
  1, 17, 12, 2, 0, 10, 8, 9, 1, 0,
  1, 15, 12, 2, 0, 11, 8, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
  3, 15, 12, 2, 0, 10, 9, 10, 1, 98,
  1, 15, 13, 2, 0, 11, 9, 9, 1, 0,
  6, 15, 12, 3, 0, 12, 8, 9, 1, 99,
  5, 15, 12, 2, 0, 10, 8, 9, 1, 3,
  1, 15, 12, 2, 1, 11, 8, 9, 1, 0,
  1, 17, 12, 2, 0, 10, 8, 9, 1, 0,
  1, 15, 12, 2, 0, 11, 8, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  1, 15, 12, 2, 0, 30, 8, 10, 1, 0,
  1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
  1, 15, 12, 2, 0, 10, 8, 9, 2, 0,
  255, 35, 12, 2, 0, 10, 8, 9, 2, 0,
  5, 15, 12, 2, 0, 10, 8, 9, 1, 3,
  1, 15, 12, 2, 1, 11, 8, 9, 1, 0,
  1, 17, 12, 2, 0, 10, 8, 9, 1, 0,
  1, 15, 12, 2, 0, 11, 8, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  32, 15, 12, 2, 0, 10, 8, 9, 1, 0,
  2, 16, 12, 2, 0, 50, 8, 9, 1, 0,
  1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
  3, 15, 12, 2, 0, 10, 9, 10, 1, 98,
  1, 15, 13, 2, 0, 11, 9, 9, 1, 0,
  6, 15, 12, 3, 0, 12, 8, 9, 1, 99,
  5, 15, 12, 2, 0, 10, 8, 9, 1, 3,
  1, 15, 12, 2, 1, 11, 8, 9, 1, 0,
  1, 17, 12, 2, 0, 10, 8, 9, 1, 0,
  1, 15, 12, 2, 0, 11, 8, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  1, 15, 12, 2, 0, 30, 8, 10, 1, 0,
  1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
  1, 15, 12, 2, 0, 10, 8, 9, 2, 0,
  255, 35, 12, 2, 0, 10, 8, 9, 2, 0,
  5, 15, 12, 2, 0, 10, 8, 9, 1, 3,
  1, 15, 12, 2, 1, 11, 8, 9, 1, 0,
  1, 17, 12, 2, 0, 10, 8, 9, 1, 0,
  1, 15, 12, 2, 0, 11, 8, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  1, 15, 12, 2, 0, 30, 8, 10, 1, 0,
  1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
  1, 15, 12, 2, 0, 10, 8, 9, 2, 0,
  255, 35, 12, 2, 0, 10, 8, 9, 2, 0,
  5, 15, 12, 2, 0, 10, 8, 9, 1, 3,
  1, 15, 12, 2, 1, 11, 8, 9, 1, 0,
  1, 17, 12, 2, 0, 10, 8, 9, 1, 0,
  1, 15, 12, 2, 0, 11, 8, 9, 1, 0,
  9, 16, 12, 2, 0, 40, 10, 9, 1, 0,
  1, 15, 12, 2, 0, 30, 8, 10, 1, 0,
  1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
  1, 15, 12, 2, 0, 10, 8, 9, 2, 0,
  255, 35, 12, 2, 0, 10, 8, 9, 2, 0
};

