/* Goals:
 * X) Rearrange code to be more easily hackable
 * X) Control the camera motion
 * X) Make object levitate and rotate like a magical object
 * *) Make color brightness based on collinearity between normal and camera
 * *) Try textures out.
 * *) Make depth darken object overall
 * *) Bring in a 3D model and navigate around it; pipeline it baby.
 * *) Compile shader offline and use binary
 * *) Import shader's binary as an array of chars and use directly
 */

/* NOTES: 
 * 
 * I need to be able to turn away from the cube. It's not letting me do that. How come?
 *
 * I need to be able to translate sideways, but I can't. how come? 
 * 
 * It's warping badly when I rotate instead of looking normal. Why?
 *
 * That's enough mysteries for now; more to come.
 */
#include "gles2Hack.h"
#include <assert.h>
#include "read-png.h"
#define DBG (1)

/*
 * save model matrix
 * combine proj-view matrix together
 * make uniform for light position
 */

static float  i = 0.0;
static float di = 0.02;
static float  j = 0.0;
static float dj = 0.02;

static float  x = 0.0;
static float  y = 0.0;
static float  z = 0.0;

static float  w = 0.0;
static float  p = 0.0;
static float  r = 0.0;

void checkError( char* title ) {
  GLenum err = glGetError();
  if ( err == GL_NO_ERROR ) {
    return;
  }
  printf( "Error at %s: ", title );
  switch( err ) {
    case GL_INVALID_VALUE:
      printf( "Invalid value.\n" );
      exit( 1 );
      break;
    case GL_INVALID_ENUM:
      printf( "Invalid enum.\n" );
      exit( 1 );
      break;
    case GL_INVALID_OPERATION:
      printf( "Invalid operation.\n");
      exit( 1 );
      break;
    case GL_OUT_OF_MEMORY:
      printf( "Out of memory.\n");
      exit( 1 );
      break;
  }
}


// TEMPORARY: get rid of this when you become sophisticated enough to load your own models.
int esGenCube ( UserData *ud )
{
   const static int numVertices = 24;
   const static int numIndices = 36;

   // Generate the indices
   // Vertex and index buffers
   glGenBuffers( 2, &ud->buffers[0] );
   checkError( "Generating two buffers" );
   glBindBuffer ( GL_ARRAY_BUFFER, ud->buffers[0] );
   checkError( "Binding vertex buffer" );
   glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, ud->buffers[1] );
   checkError( "Binding index buffer" );
   glBufferData ( GL_ARRAY_BUFFER, numVertices * N_FLOATS_PER_VERTEX * sizeof( GLfloat ), cubeVerts, GL_STATIC_DRAW );
   checkError( "Filling up vertex buffer" );
   glBufferData ( GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof( GLushort ), cubeIndices, GL_STATIC_DRAW );
   checkError( "Filling up index buffer" );

   return numIndices;
}

// X11 related local variables
static Display *x_display = NULL;

GLint getCompilerStatus( GLuint shader, const char* shaderType ) {
  GLint compiled;
  // Check the compile status
  glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );  

  if ( !compiled ) 
  {
    GLint infoLen = 0;
    glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );

    if ( infoLen > 1 )
    {
      char* infoLog = malloc (sizeof(char) * infoLen );
      glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
      printf( "Error compiling %s:\n%s\n", shaderType, infoLog );
      free ( infoLog );
    }

    glDeleteShader ( shader );
    return 1;  // bad
  }
  return 0;  // good 
}

GLint getLinkerStatus( GLuint programObject ) {
  GLint linked;
  // Check the link status
  glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

  if ( !linked ) 
  {
    GLint infoLen = 0;

    glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );

    if ( infoLen > 1 )
    {
      char* infoLog = malloc (sizeof(char) * infoLen );

      glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
      printf( "Error linking program:\n%s\n", infoLog );

      free ( infoLog );
    }

    glDeleteProgram ( programObject );
    return 1;  // bad
  }
  return 0;  // good
}

GLuint compileShader( GLuint shaderType, const char* srcString) {
  GLuint shader = glCreateShader ( shaderType );
  if ( shader == 0 ) {
    return 0;
  }
  glShaderSource ( shader, 1, &srcString, NULL );
  glCompileShader ( shader );
  if (getCompilerStatus(shader, shaderType == GL_VERTEX_SHADER ? "Vertex shader" : "Fragment shader" )) {
    return 0;
  }
  return shader;
}

///
// Initialize the shader and program object
//
int Init ( ESContext *esContext ) {
  UserData *ud = &esContext->ud;
  // Initialize kinematics.
  ud->aspect = (GLfloat) esContext->width / (GLfloat) esContext->height;
  ud->velX = ud->velY = ud->velZ = 0;
  ud->velR = ud->velP = ud->velW = 0.0;
  ud->x = ud->y = ud->z = 0;
  ud->w = ud->p = ud->r = M_PI / 4;
  ud->prevMouseX = 0;
  ud->prevMouseY = 0;
  // World to camera frame; initialize its orientation to the correct direction.
  GLchar vShaderStr[] = 
    #include "vert-shader.vert"
  GLchar fShaderStr[] =  
    #include "frag-shader.vert"
  // Compile shaders
  GLuint vShader = compileShader( GL_VERTEX_SHADER, vShaderStr );  
  checkError( "Compiling vertex shader" );
  GLuint fShader = compileShader( GL_FRAGMENT_SHADER, fShaderStr );  
  checkError( "Compiling fragment shader" );
  if ( !vShaderStr || !fShaderStr ) {
    return 0;
  }

  // Link the object files into an executable (program)
  ud->programObject = glCreateProgram ( );
  checkError( "Creating program" );
  if ( ud->programObject == 0 ) {
    return 0;
  }
  glAttachShader ( ud->programObject, vShader );
  glAttachShader ( ud->programObject, fShader );
  glLinkProgram ( ud->programObject );
  if (getLinkerStatus( ud->programObject ) ) {
    return 0;
  }

  // Grab the memory locations of the shader's inputs and outputs.
  // Attributes
  ud->positionLoc  = glGetAttribLocation ( ud->programObject, "a_position" );  
  checkError( "Getting position location" );
  ud->colorLoc  = glGetAttribLocation ( ud->programObject, "a_color" );  
  checkError( "Getting color location" );
  ud->normalLoc  = glGetAttribLocation ( ud->programObject, "a_normal" );  
  checkError( "Getting normal location" );
  ud->texelLoc  = glGetAttribLocation ( ud->programObject, "a_texelCoord" );  
  checkError( "Getting normal location" );
  // Uniforms
  ud->mLoc = glGetUniformLocation( ud->programObject, "u_mMatrix" );  
  checkError( "Getting M matrix location" );
  ud->vpLoc = glGetUniformLocation( ud->programObject, "u_vpMatrix" );  
  checkError( "Getting VP matrix location" );
  ud->camNormalLoc  = glGetUniformLocation ( ud->programObject, "u_camNormal" );  
  checkError( "Getting camera normal location" );

  // Scream if any locations are bad-looking.
  if (
      ud->positionLoc < 0 || 
      ud->mLoc < 0 || 
      ud->vpLoc < 0 || 
      ud->colorLoc < 0 ||
      ud->normalLoc < 0 ||
      ud->camNormalLoc < 0 ||
      ud->texelLoc < 0
     ) {
    printf("programObject is %d\n", ud->programObject);
    printf("ATTRIBUTES:\n-----------\n");
    printf("positionLoc is %d\n", ud->positionLoc);
    printf("colorLoc is %d\n", ud->colorLoc);
    printf("normalLoc is %d\n", ud->normalLoc);
    printf("texelLoc is %d\n", ud->texelLoc);
    printf("\nUNIFORMS:\n-----------\n");
    printf("mLoc is %d\n", ud->mLoc);
    printf("vpLoc is %d\n", ud->vpLoc);
    printf("camNormalLoc is %d\n", ud->camNormalLoc);
    return 0;
  }


  // Generate the vertex data
  ud->numIndices = esGenCube( ud );
  // Cool. Now the stuff we only need to send once, we don't send every frame.
  glUseProgram ( ud->programObject );
  // Enable vertex positions.
  checkError(" using program ");
  glEnableVertexAttribArray ( ud->positionLoc );
  checkError("enabling position loc");
  glEnableVertexAttribArray ( ud->colorLoc );
  checkError("enabling color loc");
  glEnableVertexAttribArray ( ud->normalLoc );
  checkError("enabling normal loc");
  glEnableVertexAttribArray ( ud->texelLoc );
  checkError("enabling texel loc");
  glVertexAttribPointer ( ud->positionLoc, 3, GL_FLOAT, GL_FALSE, N_FLOATS_PER_VERTEX * sizeof(float), (const void*) (0) );
  checkError("show where positions are in AOS");
  glVertexAttribPointer ( ud->normalLoc, 3, GL_FLOAT, GL_FALSE, N_FLOATS_PER_VERTEX * sizeof(float), (const void*) (3 * sizeof( GL_FLOAT )) );
  checkError("show where normals are in AOS");
  glVertexAttribPointer ( ud->colorLoc, 3, GL_FLOAT, GL_FALSE, N_FLOATS_PER_VERTEX * sizeof(float), (const void*) (6 * sizeof( GL_FLOAT )) );
  checkError("show where colors are in AOS");
  glVertexAttribPointer ( ud->texelLoc, 2, GL_FLOAT, GL_FALSE, N_FLOATS_PER_VERTEX * sizeof(float), (const void*) (9 * sizeof( GL_FLOAT )) );
  checkError("show where texel coordinates are in AOS");

#if 1
  glClearColor ( 1.0f, 1.0f, 1.0f, 1.0f );
#else
  glClearColor ( 0.0f, 0.0f, 0.0f, 1.0f );
#endif

  // Generate texture
  // First, load the image we want.
  Image img = {0};
  Error e = readPng("./ddr.png", &img, 1);
  glGenTextures(1, &ud->texture);
  checkError("generating texture");
  glBindTexture(GL_TEXTURE_2D, ud->texture);
  checkError("binding texture");
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.w, img.h, 0, GL_RGB, GL_UNSIGNED_BYTE, img.dataP);
  glGenerateMipmap(GL_TEXTURE_2D);

  // Delete shaders
  glDeleteShader(vShader);
  glDeleteShader(fShader);
  return GL_TRUE;
}

///
// Handle input
//
#define SENSITIVITY (0.0001)
void Key ( ESContext *esContext) {
  UserData* ud = &esContext->ud;
  esContext->ud.mouseMoved = 0;
  XEvent xev;
  KeySym key;
  GLboolean userinterrupt = GL_FALSE;
  char text;
  static int prevX = 0;
  // Reset the local, incremental kinematics before we repopulate them below.
  // Pump all messages from X server. Keypresses are directed to keyfunc (if defined)
  while ( XPending ( x_display ) ) {
    XNextEvent( x_display, &xev );
    switch (xev.type) {
      case MotionNotify:
        esContext->ud.mouseMoved = 1;
        esContext->ud.velP = ( xev.xmotion.x - 0.5 * esContext->width ) * SENSITIVITY;  // Pitch axis is vertical in camera frame. Think P as in uP
        esContext->ud.velR = ( xev.xmotion.y - 0.5 * esContext->height ) * SENSITIVITY;  // Roll axis is horizontal in camera frame. Think R as in Right.
        ud->prevMouseX = xev.xmotion.x;                     // We don't use yaw in camera frame.
        ud->prevMouseY = xev.xmotion.y;

        break;
      case KeyRelease:
        if (XLookupString(&xev.xkey,&text,1,&key,0)==1)
        {
          // I know this is messy, but I don't want to register another function right now.
          switch (text) {
            case 'w':
              esContext->ud.velZ = 0;
              break;
            case 's':
              esContext->ud.velZ = 0;
              break;
            case 'a':
              esContext->ud.velX = 0;
              break;
            case 'd':
              esContext->ud.velX = 0;
              break;
            default:
              break;
          }
        }
        break;
      case KeyPress:
        if (XLookupString(&xev.xkey,&text,1,&key,0)==1)
        {
#if DBG
          printf("pressed %c\n", text);
#endif
          switch ( text )
          {
            case 'w':
              esContext->ud.velZ = 0.25;
              break;
            case 's':
              esContext->ud.velZ = -0.25;
              break;
            case 'a':
              esContext->ud.velX = 0.25;
              break;
            case 'd':
              esContext->ud.velX = -0.25;
              break;
            case 'r':
              x = y = z = w = p = r = 0.0;
              break;
            case 033:  // ASCII Escape Key
              esContext->ud.itsTimeToExit = GL_TRUE;
              break;
          }
        }
        break;
    }
    if ( xev.type == DestroyNotify ) {
      esContext->ud.itsTimeToExit = GL_TRUE;
    }
  }
}


///
// Update MVP matrix based on time
//

void Update ( UserData *ud, float deltaTime )
{
  // Model matrix: a levitating, spinning cube
  esMatrixLoadIdentity( &ud->mMatrix  );
  esTranslate( &ud->mMatrix, 0, sin( 3 * i ), -5.0 );  // Translate 
  esRotateX( &ud->mMatrix, i += di );       // post-multiply to get from 1 to 2
  esRotateY( &ud->mMatrix, j += dj );       // again from 2 to 3
  // View-Perspective matrix
  esMatrixLoadIdentity( &ud->vpMatrix );
  esRotateY( &ud->vpMatrix, p += ud->velP ); // Turn camera up and down
  esRotateX( &ud->vpMatrix, r += ud->velR ); // Turn camera right and left
  esTranslate( &ud->vpMatrix, x += ud->velX, 0.0, (z += ud->velZ) );  // Translate 
  // dispMatrix( "vp Matrix", &ud->vpMatrix );
  esPerspective( &ud->vpMatrix, 60.0f, ud->aspect, 1.0f, 20.0f );  // post-multiply since this is done on current digidigi
}

// Draw a triangle using the shader pair created in Init()
void Draw ( ESContext *esContext )
{
  UserData *ud = &esContext->ud;
  glViewport ( 0, 0, esContext->width, esContext->height );
  glClear ( GL_COLOR_BUFFER_BIT );
  // TODO see whether this is necessary every time.
  glUseProgram ( ud->programObject );
#if 1
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
#endif
  // Do the matrices
  glUniformMatrix4fv( ud->vpLoc, 1, GL_FALSE, (GLfloat*) &ud->vpMatrix.m[0][0] );
  checkError("uniform VP matrix");
  glUniformMatrix4fv( ud->mLoc, 1, GL_FALSE, (GLfloat*) &ud->mMatrix.m[0][0] );
  checkError("uniform model matrix");
  // Include camera normal.
  glUniform3fv ( ud->camNormalLoc, 1, &ud->camNormal[0]);
  checkError("uniform cam normal location");
  // Draw everything. Notice the absence of offset params implies indices must exist in a separate array. 
  // That's kind of "duh" though when you think about it.
  glDrawElements ( GL_TRIANGLES, ud->numIndices, GL_UNSIGNED_SHORT, 0 );
  checkError("draw elems");
}

// Cleanup
void ShutDown ( ESContext *esContext )
{
  UserData *ud = &esContext->ud;

  if ( ud->vertices != NULL )
  {
    free ( ud->vertices );
  }

  if ( ud->indices != NULL )
  {
    free ( ud->indices );
  }

  // Delete program object
  glDeleteProgram ( ud->programObject );

  // free(ud);
  XAutoRepeatOn( x_display );
}

// CreateEGLContext()
//    Creates an EGL rendering context and all associated elements
EGLBoolean CreateEGLContext ( EGLNativeWindowType hWnd, EGLDisplay* eglDisplay,
                              EGLContext* eglContext, EGLSurface* eglSurface,
                              EGLint attribList[])
{
   EGLint numConfigs;
   EGLint majorVersion;
   EGLint minorVersion;
   EGLDisplay display;
   EGLContext context;
   EGLSurface surface;
   EGLConfig config;
   EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };

   // Get Display
   display = eglGetDisplay((EGLNativeDisplayType)x_display);
   if ( display == EGL_NO_DISPLAY )
   {
      return EGL_FALSE;
   }

   // Initialize EGL
   if ( !eglInitialize(display, &majorVersion, &minorVersion) )
   {
      return EGL_FALSE;
   }

   // Get configs
   if ( !eglGetConfigs(display, NULL, 0, &numConfigs) )
   {
      return EGL_FALSE;
   }
   void* funcPtr = 0;
   // jfuncPtr = eglGetProcAddress("eglSwapInterval");
   // jprintf("swap interval func ptr: 0x%08x\n", funcPtr);
   // j((*funcPtr)(display, 1);
   eglSwapInterval(display, 2);

   // Choose config
   if ( !eglChooseConfig(display, attribList, &config, 1, &numConfigs) )
   {
      return EGL_FALSE;
   }

   // Create a surface
   surface = eglCreateWindowSurface(display, config, (EGLNativeWindowType)hWnd, NULL);
   if ( surface == EGL_NO_SURFACE )
   {
      return EGL_FALSE;
   }

   // Create a GL context
   context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs );
   if ( context == EGL_NO_CONTEXT )
   {
      return EGL_FALSE;
   }   

   // Make the context current
   if ( !eglMakeCurrent(display, surface, surface, context) )
   {
      return EGL_FALSE;
   }

   *eglDisplay = display;
   *eglSurface = surface;
   *eglContext = context;
   return EGL_TRUE;
} 

///
//  WinCreate()
//
//      This function initialized the native X11 display and window for EGL
//
EGLBoolean WinCreate(ESContext *esContext, const char *title)
{
    Window root;
    XSetWindowAttributes swa;
    XSetWindowAttributes  xattr;
    Atom wm_state;
    XWMHints hints;
    XEvent xev;
    EGLConfig ecfg;
    EGLint num_config;
    Window win;

    /*
     * X11 native display initialization
     */

    x_display = XOpenDisplay(NULL);
    if ( x_display == NULL )
    {
        return EGL_FALSE;
    }
    XAutoRepeatOff(x_display);

    root = DefaultRootWindow(x_display);
    XGrabPointer(x_display, root, False, PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);

    swa.event_mask  =  ExposureMask | PointerMotionMask | KeyPressMask | KeyReleaseMask;
    win = XCreateWindow(
               x_display, root,
               0, 0, esContext->width, esContext->height, 0,
               CopyFromParent, InputOutput,
               CopyFromParent, CWEventMask,
               &swa );

    xattr.override_redirect = GL_FALSE;
    XChangeWindowAttributes ( x_display, win, CWOverrideRedirect, &xattr );

    hints.input = GL_TRUE;
    hints.flags = InputHint;
    XSetWMHints(x_display, win, &hints);

    // make the window visible on the screen
    XMapWindow (x_display, win);
    XStoreName (x_display, win, title);

    // get identifiers for the provided atom name strings
    wm_state = XInternAtom (x_display, "_NET_WM_STATE", GL_FALSE);

    memset ( &xev, 0, sizeof(xev) );
    xev.type                 = ClientMessage;
    xev.xclient.window       = win;
    xev.xclient.message_type = wm_state;
    xev.xclient.format       = 32;
    xev.xclient.data.l[0]    = 1;
    xev.xclient.data.l[1]    = GL_FALSE;
    XSendEvent (
       x_display,
       DefaultRootWindow ( x_display ),
       GL_FALSE,
       SubstructureNotifyMask,
       &xev );

    esContext->hWnd = (EGLNativeWindowType) win;
    return EGL_TRUE;
}

///
//  esCreateWindow()
//
//      title - name for title bar of window
//      width - width of window to create
//      height - height of window to create
//      flags  - bitwise or of window creation flags 
//          ES_WINDOW_ALPHA       - specifies that the framebuffer should have alpha
//          ES_WINDOW_DEPTH       - specifies that a depth buffer should be created
//          ES_WINDOW_STENCIL     - specifies that a stencil buffer should be created
//          ES_WINDOW_MULTISAMPLE - specifies that a multi-sample buffer should be created
//
GLboolean esCreateWindow ( ESContext *esContext, const char* title, GLint width, GLint height, GLuint flags )
{
  // a list of integers arranged in key-value pairs
  EGLint attribList[] =
  {
    EGL_RED_SIZE,       5,
    EGL_GREEN_SIZE,     6,
    EGL_BLUE_SIZE,      5,
    EGL_ALPHA_SIZE,     (flags & ES_WINDOW_ALPHA) ? 8 : EGL_DONT_CARE,
    EGL_DEPTH_SIZE,     (flags & ES_WINDOW_DEPTH) ? 8 : EGL_DONT_CARE,
    EGL_STENCIL_SIZE,   (flags & ES_WINDOW_STENCIL) ? 8 : EGL_DONT_CARE,
    EGL_SAMPLE_BUFFERS, (flags & ES_WINDOW_MULTISAMPLE) ? 1 : 0, EGL_NONE
  };

  if ( esContext == NULL )
  {
    return GL_FALSE;
  }

  esContext->width = width;
  esContext->height = height;

  if ( !WinCreate ( esContext, title) )
  {
    return GL_FALSE;
  }


  if ( !CreateEGLContext ( esContext->hWnd,
        &esContext->eglDisplay,
        &esContext->eglContext,
        &esContext->eglSurface,
        attribList) )
  {
    return GL_FALSE;
  }


  return GL_TRUE;
}

int main ( int argc, char *argv[] )
{
  ESContext esContext = {0};
  GLboolean windowMade = esCreateWindow ( &esContext, "Simple Texture 2D", 480, 240, ES_WINDOW_RGB ); 
  if (!windowMade) {
    return 1;
  }

  if ( !Init ( &esContext ) ) {
    return 1;
  }

  // Main loop
  while(!esContext.ud.itsTimeToExit) {
    Key(&esContext);
    Update(&esContext.ud, 0.01);
    Draw(&esContext);
    eglSwapBuffers(esContext.eglDisplay, esContext.eglSurface);
  }

  ShutDown ( &esContext );
}
