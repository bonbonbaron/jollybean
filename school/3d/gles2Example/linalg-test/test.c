#include <stdio.h>
#include <math.h>
#include "laMath.h"

int main(int argc, char** argv) {
  ESMatrix matrix, m2, proj;
  esMatrixLoadIdentity( &matrix );  // reset the matrix
  esMatrixLoadIdentity( &m2 );  // reset the matrix
  esMatrixLoadIdentity( &proj );  // reset the matrix

  // Tranformation tests
  m2.m[0][3] = 10;  // set X to 10 in world frame
  m2.m[1][3] = 5;  // set X to 10 in world frame
  m2.m[2][3] = 2;  // set X to 10 in world frame
  esTransform( &matrix, 5, 6, 7, M_PI / 3, 2, -M_PI/5);  // rotate -90 degrees around Z, then translate in that new frame
  dispMatrix( "H", &matrix );
  dispMatrix( "m", &m2 );
  esMatrixMultiply(&m2, &matrix, &m2);
  dispMatrix( "m' = H * m", &m2 );
  // Now try to invert this to make sure you've put it in the right place in the world.
  esInvert( &matrix );
  dispMatrix( "inv(H)", &matrix );
  esMatrixMultiply(&m2, &matrix, &m2);
  dispMatrix( "m = inv(H) * m'", &m2 );

  return 0;
}
