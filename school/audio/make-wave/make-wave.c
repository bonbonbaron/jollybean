#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

int main(int argc, char **argv) {
  if ( argc != 2 ) {
    printf( "Expecting a desired frequency for an argument.\n" );
    exit( 1 );
  }

  float FREQ = atof( argv[1] );
  FILE *outputP = fopen( "./outputwave", "wb" );

  /* 
   * In the context of sample rates, the sample rate divided by the actual audio frequency
   * is the number of samples it takes to complete one period.
   *
   * So if you want to make a sine wave that's 50 Hz in a 100 samples, you'll have to step
   * through 2pi in 100 / 50 = 2 steps.
   *
   * That means you need to increment by that much every time, influencing solely your input conversion.
   */

  static const int SAMPLE_RATE = 44100;  // PCM samples per second
  float INPUT_CONVERSION = ( ( FREQ / (float) SAMPLE_RATE ) * 2.0 * M_PI );
  static const int QUANTIZE_FACTOR = SHRT_MAX;
  unsigned short output;

  for (int i = 0; i < SAMPLE_RATE; ++i ) {
    output = ( sinf( i * INPUT_CONVERSION ) * QUANTIZE_FACTOR ) + SHRT_MAX;
#if 0
    printf( "%d: %d\n", i, output );
#else
    fwrite( &output, sizeof( output ), 1, outputP );
#endif
  }
  fclose(outputP);
  return 0;
}
