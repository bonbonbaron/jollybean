#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define N (5000)
#define N_INV ( 1.0f / N )
#define N_OVER_2 (N / 2)
#define PI_OVER_N (M_PI / N)
#define N_OVER_2_ROUNDED ( N_OVER_2 + 0.5f )
#define SAMPLE_RATE (44100)
void naive_mdct( unsigned short *xA, unsigned int *XA ) {
  for (int i = 0; i < N_OVER_2; ++i) {
    for (int j = 0; j < N; ++j) {
      XA[i] += xA[j] * cos( PI_OVER_N * ( j + N_OVER_2_ROUNDED ) * ( i + 0.5 ) );
    }
  }
}

void naive_imdct( int *XA, int *yA ) {

}

int main(int argc, char **argv) {
  if ( argc <= 1 ) {
    printf("C'mon man\n");
    exit(1);
  }
  char* filename = argv[1];
  FILE *fP = fopen( filename, "rb" );
  // Determine whether this is a wave file or a raw wave first.
  static const char RIFF[5] = "RIFF";
  static const char DATA[5] = "data";
  // Find data
  char dataString[5];  // 5 because it stores a terminating '\0' character at the end
  fgets( dataString, 5, fP );  
  if ( !strcmp( dataString, RIFF ) ) {
    while ( ( dataString[0] = fgetc( fP ) ) != EOF ) {
      if ( dataString[0] == 'd' ) {
        ungetc( dataString[0], fP );
        fgets( dataString, 5, fP );  
        if ( !strcmp( dataString, DATA ) ) {
          break;
        }
      }
    }
  }
  // This is NOT a wave file. Treat it like a raw binary wave.
  else {
    fseek( fP, 0, SEEK_SET );
  }
    
  unsigned short xA[N] = { 0 };
  size_t totalItems = 0;
  size_t nItems = 0;
  unsigned int XA[N_OVER_2] = { 0 };
  do {
    nItems = fread( &xA, sizeof( unsigned short ), N, fP );
    totalItems += nItems;
    naive_mdct( xA, XA );  // xA is in the frequency domain, so for now, let's see what adding everything into it does.
    // printf(" total: %d\n", totalItems);
  } while ( nItems == N );

  // Display output
  // I see why the frequency resolution sucks at small window sizes now.
#define NYQUIST_FREQUENCY ( SAMPLE_RATE / 2 )
#define BANDWIDTH ( NYQUIST_FREQUENCY / N )
  // Middle C is ~261Hz, which is why i'm expecting to see it dominate the frequency domain output.
  // But I'm not seeing that. Could be a formatting issue.
  for ( int i = 0; i < N_OVER_2; ++i ) {
    printf( "%5d - %5d Hz: %12d\n", BANDWIDTH * i, BANDWIDTH * ( i + 1 ), XA[i] );
  }

  return 0;
}
