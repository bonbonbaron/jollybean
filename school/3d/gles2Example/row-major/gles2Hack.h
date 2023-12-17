#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include  <X11/Xlib.h>
#include  <X11/Xatom.h>
#include  <X11/Xutil.h>

#include "laMath.h"

/// esCreateWindow flag - RGB color buffer
#define ES_WINDOW_RGB           0
/// esCreateWindow flag - ALPHA color buffer
#define ES_WINDOW_ALPHA         1 
/// esCreateWindow flag - depth buffer
#define ES_WINDOW_DEPTH         2 
/// esCreateWindow flag - stencil buffer
#define ES_WINDOW_STENCIL       4
/// esCreateWindow flat - multi-sample buffer
#define ES_WINDOW_MULTISAMPLE   8

#define TWO_PI (M_PI * 2)

typedef struct
{
  GLuint itsTimeToExit;  // 0 when no exit, 1 when yes exit
  // Handle to a program object
  GLuint programObject;

  GLfloat aspect;
  GLbyte mouseMoved;
  GLfloat x, y, z;  // store your current angles so you don't exceed bounds
  GLfloat w, p, r;  // store your current angles so you don't exceed bounds
  GLfloat velX, velY, velZ;  
  GLfloat velW, velP, velR;
  GLint prevMouseX, prevMouseY;
  GLfloat camNormal[3];
  ESMatrix world2cam;
  ESMatrix model2world;

  // Attribute locations
  GLint  positionLoc;
  GLint  vrtNormalLoc;
  GLint  camNormalLoc;

  // Uniform locations
  GLint  mvpLoc;

  // Vertex daata
  GLfloat  *vertices;
  GLfloat  *normals;
  GLuint *indices;
  int       numIndices;

  // Rotation angle
  GLfloat   angle;

  // MVP matrix
  ESMatrix  mvpMatrix;
} UserData;

typedef struct _escontext
{
   /// Put your user data here...
   UserData    ud;

   /// Window width
   GLint       width;

   /// Window height
   GLint       height;

   GLbyte      wPressed;
   GLbyte      aPressed;
   GLbyte      sPressed;
   GLbyte      dPressed;
   /// Window handle
   EGLNativeWindowType  hWnd;

   /// EGL display
   EGLDisplay  eglDisplay;
      
   /// EGL context
   EGLContext  eglContext;

   /// EGL surface
   EGLSurface  eglSurface;
} ESContext;


