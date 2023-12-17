/* 
decode.c

All the variables in the decoder are analogous to those in the encoder, with the exception of
left, which now contains be − ac. The fractional part of e is stored in the incoming encoded
data-stream.
The value variable is used only to facilitate the assertion-checking that occurs at the
beginning of AC_Decode.
*/

/*
 * decode.c
 * By Barney Stratford
 * Version 12, 13th January 2005
 */

#include <stdio.h>
#include <assert.h>
#include "ac.h"
#define BITS 16

static FILE *file = NULL;
static long left, kink, bits;
static long denom, value;
extern const int ac_shift[];
static long read_byte (void);

void AC_InitialiseDecoder (FILE *compressed)
{
  assert (compressed != NULL);
  assert (file == NULL);

  /* Initialise state */
  file = compressed;
  read_byte ();
  left = (read_byte () << 16) + (read_byte () << 8) + read_byte ();
  kink = 0;
  bits = BITS + 8;
  denom = 1;
  value = 0;
}

long AC_Decode (register long low, register long high, long new_denom)
{
  register long width;
  assert (1 <= new_denom);
  assert (new_denom <= MAX_DENOM);
  assert (0 <= low);
  assert (low <= value);
  assert (value < high);
  assert (high <= denom);
  assert (file != NULL);

  /* Shrink the interval */
  low <<= bits;
  high <<= bits;
  if (low < kink) low <<= 1;
  else low += kink;
  if (high < kink) high <<= 1;
  else high += kink;
  left -= low;
  width = high - low;
  /* If necessary, read in some bytes */
  while (width <= 1L << BITS)
  {
    left = (left << 8) + read_byte ();
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

  /* Find the next symbol */
  if (left < kink << 1) return (value = (left >> 1) >> bits);
  else return (value = (left - kink) >> bits);
}
void AC_FinaliseDecoder (void)
{
  assert (file != NULL);
  file = NULL;
}
static long read_byte (void)
{
  int byte = getc (file);
  return (byte == EOF ? 0 : byte);
}


