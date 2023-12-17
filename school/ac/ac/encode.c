
/*
   encode.c

   Throughout this file, the left variable contains the value of a and width contains b−a. The
   value of k in the definition of approx is contained in bits, while kink contains the location
   of the kink in the graph of the approximation to integer division.
   The denom variable contains the denominator to be use next time AC_Encode is called.
   It is not used directly, other than in the assertions at the beginning of AC_Encode.
   */
/*
 * encode.c
 * By Barney Stratford
 * Version 12, 13th January 2005
 */

#include <assert.h>
#include <stdio.h>
#include "ac.h"
#define BITS 16

static FILE *file = NULL;
static long left, kink, bits;
static long denom;
extern const int ac_shift[];

static void write_byte (long byte);

void AC_InitialiseEncoder (FILE *compressed)
{
#ifdef DEBUG
  assert (compressed != NULL);
  assert (file == NULL);
#endif
  /* Initialise state */
  file = compressed;
  left = 0;
  kink = 0;
  bits = BITS + 8;
  denom = 1;
}
void AC_Encode (register long low, register long high, long new_denom)
{
  register long width;
#ifdef DEBUG
  assert (1 <= new_denom);
  assert (new_denom <= MAX_DENOM);
  assert (0 <= low);
  assert (low < high);
  assert (high <= denom);
  assert (file != NULL);
#endif
  /* Shrink the interval */
  low <<= bits;
  high <<= bits;
  if (low < kink) low <<= 1;
  else low += kink;
  if (high < kink) high <<= 1;
  else high += kink;
  left += low;
  width = high - low;

  /* If necessary, write out some bytes */
  while (width <= 1L << BITS)
  {
    write_byte (left >> BITS);
    left = (left & ((1L << BITS) - 1)) << 8;
    width <<= 8;
  }
  /* Adjust the denominator */
  if (new_denom <= 256)
    bits = BITS + ac_shift[new_denom] - ac_shift[width >> BITS];
  else
    bits = BITS - 8 + ac_shift[new_denom >> 8] - ac_shift[width >> BITS];
  if (new_denom << bits > width) bits--;
  kink = width - (new_denom << bits);
  denom = new_denom;
}

void AC_FinaliseEncoder (void)
{
#ifdef DEBUG
  assert (file != NULL);
#endif
  /* Write out any remaining data in the buffer */
  write_byte (left >> BITS);
  write_byte (256);
  write_byte (0);
  write_byte (0);
  file = NULL;
}

static void write_byte (long byte)
{
  static int buffer = 0, carry = 0;
  if (byte < 255)
  {
    putc (buffer, file);
    while (carry)
    {
      putc (255, file);
      carry--;
    }
    buffer = byte;
  }
  else if (byte > 255)
  {
    putc (buffer + 1, file);
    while (carry)
    {
      putc (0, file);
      carry--;
    }
    buffer = byte - 256;
  }
  else carry++;
}


