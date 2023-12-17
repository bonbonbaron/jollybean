#ifndef LAMATH_H
#define LAMATH_H

#include <GLES2/gl2.h>
#include <math.h>
#include <stdio.h>


typedef struct
{
    GLfloat   m[4][4];
} ESMatrix;

typedef struct
{
    GLfloat   m[4];
} ESVector;


void esScale(ESMatrix *result, GLfloat sx, GLfloat sy, GLfloat sz);
void esTranslate(ESMatrix *result, GLfloat tx, GLfloat ty, GLfloat tz);
void esRotateX(ESMatrix *result, GLfloat angle);
void esRotateY(ESMatrix *result, GLfloat angle);
void esRotateZ(ESMatrix *result, GLfloat angle);
void esTransform( ESMatrix *result, GLfloat x, GLfloat y, GLfloat z, GLfloat roll, GLfloat pitch, GLfloat yaw );
void esInvert( ESMatrix *result);
void dispMatrix(char* title, ESMatrix *m);
void esFrustum(ESMatrix *result, float left, float right, float bottom, float top, float nearZ, float farZ);
// void esPerspective(ESMatrix *result, float fovy, float aspect, float nearZ, float farZ);
void esPerspective(ESMatrix *result, float aspect, float fov, float near, float far);
void esOrtho(ESMatrix *result, float left, float right, float bottom, float top, float nearZ, float farZ);
void esMatrixMultiply(ESMatrix *result, ESMatrix *srcA, ESMatrix *srcB);
void esMatrixLoadIdentity(ESMatrix *result);
void dispMatrix( char* title, ESMatrix* matrix );

#endif // ESUTIL_H
