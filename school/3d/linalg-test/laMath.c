/* 
 * The author did their translations transposed, unbeknownst to me.
 * This is probably what caused my distress over the weekend.
 * But because my rotations work as they are, it follows that the "correct" translations
 * are only breaking the code because of the perspective matrix.
 */

#define USE_ORIG (1)
///
//  Includes
//
#include "laMath.h"
#include <math.h>
#include <string.h>

#define PI 3.1415926535897932384626433832795f

void 
esScale(ESMatrix *result, GLfloat sx, GLfloat sy, GLfloat sz)
{
    result->m[0][0] *= sx;
    result->m[0][1] *= sx;
    result->m[0][2] *= sx;
    result->m[0][3] *= sx;

    result->m[1][0] *= sy;
    result->m[1][1] *= sy;
    result->m[1][2] *= sy;
    result->m[1][3] *= sy;

    result->m[2][0] *= sz;
    result->m[2][1] *= sz;
    result->m[2][2] *= sz;
    result->m[2][3] *= sz;
}

// I believe this is how I broke it last time. Because the "wrong" way seemed so impossible, I may never have fixed it.
// But I don't see any evidence that he treated matrices as transposed.
#if USE_ORIG  // this seems wrong, but wth...?
void 
esTranslate(ESMatrix *result, GLfloat tx, GLfloat ty, GLfloat tz)
{
    result->m[3][0] += (result->m[0][0] * tx + result->m[1][0] * ty + result->m[2][0] * tz);
    result->m[3][1] += (result->m[0][1] * tx + result->m[1][1] * ty + result->m[2][1] * tz);
    result->m[3][2] += (result->m[0][2] * tx + result->m[1][2] * ty + result->m[2][2] * tz);
    result->m[3][3] += (result->m[0][3] * tx + result->m[1][3] * ty + result->m[2][3] * tz);
}
#else
void 
esTranslate(ESMatrix *result, GLfloat tx, GLfloat ty, GLfloat tz)
{
  // TODO research translation component
    result->m[0][3] += (result->m[0][0] * tx + result->m[0][1] * ty + result->m[0][2] * tz);
    result->m[1][3] += (result->m[1][0] * tx + result->m[1][1] * ty + result->m[1][2] * tz);
    result->m[2][3] += (result->m[2][0] * tx + result->m[2][1] * ty + result->m[2][2] * tz);
}
#endif

void 
esRotate(ESMatrix *result, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
   GLfloat sinAngle, cosAngle;
   GLfloat mag = sqrtf(x * x + y * y + z * z);
      
   sinAngle = sinf ( angle * PI / 180.0f );
   cosAngle = cosf ( angle * PI / 180.0f );
   if ( mag > 0.0f )
   {
      GLfloat xx, yy, zz, xy, yz, zx, xs, ys, zs;
      GLfloat oneMinusCos;
      ESMatrix rotMat;
   
      x /= mag;
      y /= mag;
      z /= mag;

      xx = x * x;
      yy = y * y;
      zz = z * z;
      xy = x * y;
      yz = y * z;
      zx = z * x;
      xs = x * sinAngle;
      ys = y * sinAngle;
      zs = z * sinAngle;
      oneMinusCos = 1.0f - cosAngle;

      rotMat.m[0][0] = (oneMinusCos * xx) + cosAngle;
      rotMat.m[0][1] = (oneMinusCos * xy) - zs;
      rotMat.m[0][2] = (oneMinusCos * zx) + ys;
      rotMat.m[0][3] = 0.0F; 

      rotMat.m[1][0] = (oneMinusCos * xy) + zs;
      rotMat.m[1][1] = (oneMinusCos * yy) + cosAngle;
      rotMat.m[1][2] = (oneMinusCos * yz) - xs;
      rotMat.m[1][3] = 0.0F;

      rotMat.m[2][0] = (oneMinusCos * zx) - ys;
      rotMat.m[2][1] = (oneMinusCos * yz) + xs;
      rotMat.m[2][2] = (oneMinusCos * zz) + cosAngle;
      rotMat.m[2][3] = 0.0F; 

      rotMat.m[3][0] = 0.0F;
      rotMat.m[3][1] = 0.0F;
      rotMat.m[3][2] = 0.0F;
      rotMat.m[3][3] = 1.0F;

      esMatrixMultiply( result, &rotMat, result );
   }
}

  void 
esRotateX(ESMatrix *result, GLfloat angle)
{
  GLfloat s = sin ( angle );
  GLfloat c = cos ( angle );
  ESMatrix rx = {0};
  rx.m[0][0] =  1.0;
  rx.m[1][1] =    c;
  rx.m[1][2] =   -s;
  rx.m[2][1] =    s;
  rx.m[2][2] =    c;
  rx.m[3][3] =  1.0;
  esMatrixMultiply(result, &rx, result);
}

  void 
esRotateY(ESMatrix *result, GLfloat angle)
{
  GLfloat s = sin ( angle );
  GLfloat c = cos ( angle );
  ESMatrix ry = {0};
  ry.m[0][0] =    c;
  ry.m[0][2] =    s;
  ry.m[1][1] =  1.0;
  ry.m[2][0] =   -s;
  ry.m[2][2] =    c;
  ry.m[3][3] =  1.0;
  esMatrixMultiply(result, &ry, result);
}

  void 
esRotateZ(ESMatrix *result, GLfloat angle)
{
  GLfloat s = sin ( angle );
  GLfloat c = cos ( angle );
  ESMatrix rz = {0};
  rz.m[0][0] =    c;
  rz.m[0][1] =   -s;
  rz.m[1][0] =    s;
  rz.m[1][1] =    c;
  rz.m[2][2] =  1.0;
  rz.m[3][3] =  1.0;
  esMatrixMultiply(result, &rz, result);
}

void
esTransform( ESMatrix *result, GLfloat x, GLfloat y, GLfloat z, GLfloat w, GLfloat p, GLfloat r ) {
  ESMatrix temp;
  esMatrixLoadIdentity( &temp );
  dispMatrix( "[xform] identity: ", &temp );
  esRotateX( &temp, w );
  dispMatrix( "[xform] rot X: ", &temp );
  esRotateY( &temp, p );
  dispMatrix( "[xform] rot Y: ", &temp );
  esRotateZ( &temp, r );
  dispMatrix( "[xform] rot Z: ", &temp );
  esTranslate( &temp, x, y, z );
  dispMatrix( "[xform] trans: ", &temp );
  esMatrixMultiply( result, &temp, result );
  dispMatrix( "[xform] mult: ", &temp );
}

#if USE_ORIG
void 
esFrustum(ESMatrix *result, float left, float right, float bottom, float top, float nearZ, float farZ)
{
    float       deltaX = right - left;
    float       deltaY = top - bottom;
    float       deltaZ = farZ - nearZ;
    ESMatrix    frust;

    if ( (nearZ <= 0.0f) || (farZ <= 0.0f) ||
         (deltaX <= 0.0f) || (deltaY <= 0.0f) || (deltaZ <= 0.0f) )
         return;

    frust.m[0][0] = 2.0f * nearZ / deltaX;
    frust.m[0][1] = frust.m[0][2] = frust.m[0][3] = 0.0f;

    frust.m[1][1] = 2.0f * nearZ / deltaY;
    frust.m[1][0] = frust.m[1][2] = frust.m[1][3] = 0.0f;

    frust.m[2][0] = (right + left) / deltaX;
    frust.m[2][1] = (top + bottom) / deltaY;
    frust.m[2][2] = -(nearZ + farZ) / deltaZ;
    frust.m[2][3] = -1.0f;

    frust.m[3][2] = -2.0f * nearZ * farZ / deltaZ;
    frust.m[3][0] = frust.m[3][1] = frust.m[3][3] = 0.0f;

    esMatrixMultiply(result, result, &frust);
}

void 
esPerspective(ESMatrix *result, float fovy, float aspect, float nearZ, float farZ)
{
   GLfloat frustumW, frustumH;
   
   frustumH = tanf( fovy / 360.0f * PI ) * nearZ;
   frustumW = frustumH * aspect;

   esFrustum( result, -frustumW, frustumW, -frustumH, frustumH, nearZ, farZ );
}
#else
void
esFrustum(ESMatrix *result, float f, float f_over_aspect, float garbage1, float garbage2, float near, float far)
{
    if ( (near <= 0.0f) || (far <= 0.0f) )
         return;

    ESMatrix    frust = { 0 };
    frust.m[0][0] = f_over_aspect;
    frust.m[1][1] = f;
    frust.m[2][2] = ( far + near ) / ( near - far );
    frust.m[2][3] = (2 * far * near) / ( near - far );
    frust.m[3][2] = -1.0;
    esMatrixMultiply(result, &frust, result);
}

void 
esPerspective(ESMatrix *result, float fovy, float aspect, float nearZ, float farZ)
{
   GLfloat f, f_over_aspect;
   
   f = 1 / tanf( fovy / 360.0f * PI );  // Oh... he IS dividing by two by multiplying by PI instead of 2PI.
   f_over_aspect = f / aspect;

   esFrustum( result, f, f_over_aspect, 1.0, 1.0, nearZ, farZ );  // the 1.0s are garbage parameters
}
#endif

void 
esOrtho(ESMatrix *result, float left, float right, float bottom, float top, float nearZ, float farZ)
{
    float       deltaX = right - left;
    float       deltaY = top - bottom;
    float       deltaZ = farZ - nearZ;
    ESMatrix    ortho;

    if ( (deltaX == 0.0f) || (deltaY == 0.0f) || (deltaZ == 0.0f) )
        return;

    esMatrixLoadIdentity(&ortho);
    ortho.m[0][0] = 2.0f / deltaX;
    ortho.m[3][0] = -(right + left) / deltaX;
    ortho.m[1][1] = 2.0f / deltaY;
    ortho.m[3][1] = -(top + bottom) / deltaY;
    ortho.m[2][2] = -2.0f / deltaZ;
    ortho.m[3][2] = -(nearZ + farZ) / deltaZ;

    esMatrixMultiply(result, &ortho, result);
}


void 
esMatrixMultiply(ESMatrix *result, ESMatrix *srcA, ESMatrix *srcB)
{
    ESMatrix    tmp;
    int         i;

	for (i=0; i<4; i++)
	{
		tmp.m[i][0] =	(srcA->m[i][0] * srcB->m[0][0]) +
						(srcA->m[i][1] * srcB->m[1][0]) +
						(srcA->m[i][2] * srcB->m[2][0]) +
						(srcA->m[i][3] * srcB->m[3][0]) ;

		tmp.m[i][1] =	(srcA->m[i][0] * srcB->m[0][1]) + 
						(srcA->m[i][1] * srcB->m[1][1]) +
						(srcA->m[i][2] * srcB->m[2][1]) +
						(srcA->m[i][3] * srcB->m[3][1]) ;

		tmp.m[i][2] =	(srcA->m[i][0] * srcB->m[0][2]) + 
						(srcA->m[i][1] * srcB->m[1][2]) +
						(srcA->m[i][2] * srcB->m[2][2]) +
						(srcA->m[i][3] * srcB->m[3][2]) ;

		tmp.m[i][3] =	(srcA->m[i][0] * srcB->m[0][3]) + 
						(srcA->m[i][1] * srcB->m[1][3]) +
						(srcA->m[i][2] * srcB->m[2][3]) +
						(srcA->m[i][3] * srcB->m[3][3]) ;
	}
    memcpy(result, &tmp, sizeof(ESMatrix));
}

void esInvert( ESMatrix *result ) {
  ESMatrix tmp = {0};
  // Transpose the rotation.
  tmp.m[0][0] = result->m[0][0];
  tmp.m[0][1] = result->m[1][0];
  tmp.m[0][2] = result->m[2][0];
  tmp.m[1][0] = result->m[0][1];
  tmp.m[1][1] = result->m[1][1];
  tmp.m[1][2] = result->m[2][1];
  tmp.m[2][0] = result->m[0][2];
  tmp.m[2][1] = result->m[1][2];
  tmp.m[2][2] = result->m[2][2];
  // Then rotate and negate the translation.
  // BTW, because the translation above was done wrong, this part has to be made wrong too.
  // But even better would be to figure out how to make it done the right way. 
  // Consider copying this file over and trying to redo it from scratch.
  tmp.m[0][3] = -1 * (
    tmp.m[0][0] * result->m[0][3] + 
    tmp.m[0][1] * result->m[1][3] + 
    tmp.m[0][2] * result->m[2][3] );
  tmp.m[1][3] = -1 * (
    tmp.m[1][0] * result->m[0][3] + 
    tmp.m[1][1] * result->m[1][3] + 
    tmp.m[1][2] * result->m[2][3] );
  tmp.m[2][3] = -1 * (
    tmp.m[2][0] * result->m[0][3] + 
    tmp.m[2][1] * result->m[1][3] + 
    tmp.m[2][2] * result->m[2][3] );
  // And the last little bit.
  tmp.m[3][3] = 1.0;
  // Last, copy the result over.
  memcpy(result, &tmp, sizeof( ESMatrix ) );
}


void 
esMatrixLoadIdentity(ESMatrix *result)
{
    memset(result, 0x0, sizeof(ESMatrix));
    result->m[0][0] = 1.0f;
    result->m[1][1] = 1.0f;
    result->m[2][2] = 1.0f;
    result->m[3][3] = 1.0f;
}

void
dispMatrix( char* title, ESMatrix* matrix ) {
  printf( "%s = \n\t%6.3f, %6.3f, %6.3f, %6.3f\n\t%6.3f, %6.3f, %6.3f, %6.3f\n\t%6.3f, %6.3f, %6.3f, %6.3f\n\t%6.3f, %6.3f, %6.3f, %6.3f\n\n", 
      title,
      matrix->m[0][0], matrix->m[0][1], matrix->m[0][2], matrix->m[0][3],
      matrix->m[1][0], matrix->m[1][1], matrix->m[1][2], matrix->m[1][3],
      matrix->m[2][0], matrix->m[2][1], matrix->m[2][2], matrix->m[2][3],
      matrix->m[3][0], matrix->m[3][1], matrix->m[3][2], matrix->m[3][3]);
}
