#include <stdio.h>
#include <math.h>

#define N (500.0f)
#define N_INV ( 1.0f / N )
#define N_OVER_2 (N / 2)
#define PI_OVER_N (M_PI / N)
#define N_OVER_2_ROUNDED ( N_OVER_2 + 0.5f )

void naive_mdct( int *xA, int *XA ) {
  for (int i = 0; i < N_OVER_2; ++i) {
    for (int j = 0; j < N; ++j) {
      XA[i] += xA[j] * cos( PI_OVER_N * ( j + N_OVER_2_ROUNDED ) * ( i + 0.5 ) );
    }
  }
}

void naive_imdct( int *XA, int *yA ) {

}

int main(int argc, char **argv) {
  int xA[N] = { 0 };
  int XA[N_OVER_2] = { 0 };
  naive_mdct( xA, XA );
  return 0;
}
