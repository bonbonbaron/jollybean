/* Goals:
 * X) Rearrange code to be more easily hackable
 * *) Control the camera motion
 * *) Make object levitate and rotate like a magical object
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
#define DBG (1)

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

// TEMPORARY: get rid of this when you become sophisticated enough to load your own models.
int esGenCube ( float scale, GLfloat **vertices, GLfloat **normals,
                           GLfloat **texCoords, GLuint **indices )
{
   int i;
   int numVertices = 24;
   int numIndices = 36;
   
   GLfloat cubeVerts[] =
   {
      -0.5f, -0.5f, -0.5f,
      -0.5f, -0.5f,  0.5f,
      0.5f, -0.5f,  0.5f,
      0.5f, -0.5f, -0.5f,
      -0.5f,  0.5f, -0.5f,
      -0.5f,  0.5f,  0.5f,
      0.5f,  0.5f,  0.5f,
      0.5f,  0.5f, -0.5f,
      -0.5f, -0.5f, -0.5f,
      -0.5f,  0.5f, -0.5f,
      0.5f,  0.5f, -0.5f,
      0.5f, -0.5f, -0.5f,
      -0.5f, -0.5f, 0.5f,
      -0.5f,  0.5f, 0.5f,
      0.5f,  0.5f, 0.5f, 
      0.5f, -0.5f, 0.5f,
      -0.5f, -0.5f, -0.5f,
      -0.5f, -0.5f,  0.5f,
      -0.5f,  0.5f,  0.5f,
      -0.5f,  0.5f, -0.5f,
      0.5f, -0.5f, -0.5f,
      0.5f, -0.5f,  0.5f,
      0.5f,  0.5f,  0.5f,
      0.5f,  0.5f, -0.5f,
   };

   GLfloat cubeNormals[] =
   {
      0.0f, -1.0f, 0.0f,
      0.0f, -1.0f, 0.0f,
      0.0f, -1.0f, 0.0f,
      0.0f, -1.0f, 0.0f,
      0.0f, 1.0f, 0.0f,
      0.0f, 1.0f, 0.0f,
      0.0f, 1.0f, 0.0f,
      0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, -1.0f,
      0.0f, 0.0f, -1.0f,
      0.0f, 0.0f, -1.0f,
      0.0f, 0.0f, -1.0f,
      0.0f, 0.0f, 1.0f,
      0.0f, 0.0f, 1.0f,
      0.0f, 0.0f, 1.0f,
      0.0f, 0.0f, 1.0f,
      -1.0f, 0.0f, 0.0f,
      -1.0f, 0.0f, 0.0f,
      -1.0f, 0.0f, 0.0f,
      -1.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 0.0f,
   };

   GLfloat cubeTex[] =
   {
      0.0f, 0.0f,
      0.0f, 1.0f,
      1.0f, 1.0f,
      1.0f, 0.0f,
      1.0f, 0.0f,
      1.0f, 1.0f,
      0.0f, 1.0f,
      0.0f, 0.0f,
      0.0f, 0.0f,
      0.0f, 1.0f,
      1.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 0.0f,
      0.0f, 1.0f,
      1.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 0.0f,
      0.0f, 1.0f,
      1.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 0.0f,
      0.0f, 1.0f,
      1.0f, 1.0f,
      1.0f, 0.0f,
   };
   
   // Allocate memory for buffers
   if ( vertices != NULL )
   {
      *vertices = malloc ( sizeof(GLfloat) * 3 * numVertices );
      memcpy( *vertices, cubeVerts, sizeof( cubeVerts ) );
      for ( i = 0; i < numVertices * 3; i++ )
      {
         (*vertices)[i] *= scale;
      }
   }

   if ( normals != NULL )
   {
      *normals = malloc ( sizeof(GLfloat) * 3 * numVertices );
      memcpy( *normals, cubeNormals, sizeof( cubeNormals ) );
   }

   if ( texCoords != NULL )
   {
      *texCoords = malloc ( sizeof(GLfloat) * 2 * numVertices );
      memcpy( *texCoords, cubeTex, sizeof( cubeTex ) ) ;
   }

   // TODO what role do these play?
   // Generate the indices
   if ( indices != NULL )
   {
      GLuint cubeIndices[] =  // 6 squares are halved into 2 triangles each,
      {                       // making 12 triangles, so 36 indexing into the
         0, 2, 1,             // total of 24... what? we only need 8 points so
         0, 3, 2,             // why not just index 8 vertices? TODO learn 3D point formatting.
         4, 5, 6,
         4, 6, 7,

         8, 9, 10,
         8, 10, 11, 
         12, 15, 14,
         12, 14, 13, 

         16, 17, 18,
         16, 18, 19, 
         20, 23, 22,
         20, 22, 21
      };

      *indices = malloc ( sizeof(GLuint) * numIndices );
      memcpy( *indices, cubeIndices, sizeof( cubeIndices ) );
   }

   return numIndices;
}

// X11 related local variables
static Display *x_display = NULL;

GLint getCompilerStatus( GLuint shader ) {
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
      printf( "Error compiling shader:\n%s\n", infoLog );
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
  if (getCompilerStatus(shader)) {
    return 0;
  }
  return shader;
}

void checkError( char* title ) {
  GLenum err = glGetError();
  if ( err == GL_NO_ERROR ) {
    return;
  }
  printf( "Error at %s: ", title );
  switch( err ) {
    case GL_INVALID_VALUE:
      printf( "Invalid value.\n" );
      break;
    case GL_INVALID_ENUM:
      printf( "Invalid enum.\n" );
      break;
    case GL_INVALID_OPERATION:
      printf( "Invalid operation.\n");
      break;
    case GL_OUT_OF_MEMORY:
      printf( "Out of memory.\n");
      break;
  }
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
    "uniform mat4 u_mvpMatrix;                   \n"
    "attribute vec4 a_position;                  \n"
    "void main()                                 \n"
    "{                                           \n"
    "   gl_Position = a_position * u_mvpMatrix;  \n"
    "}                                           \n";

  GLchar fShaderStr[] =  
    "precision mediump float;                                                     \n"
    "void main()                                                                  \n"
    "{                                                                            \n"
    "  gl_FragColor = vec4( 1.0, 0.0, 0.0, 1.0 );        \n"
    "}                                                                            \n";

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
  // Uniforms
  ud->mvpLoc = glGetUniformLocation( ud->programObject, "u_mvpMatrix" );  
  checkError( "Getting MVP matrix location" );

  // Scream if any locations are bad-looking.
  if (
      ud->positionLoc < 0 || 
      ud->mvpLoc < 0 
     ) {
    printf("programObject is %d\n", ud->programObject);
    printf("ATTRIBUTES:\n-----------\n");
    printf("positionLoc is %d\n", ud->positionLoc);
    printf("\nUNIFORMS:\n-----------\n");
    printf("mvpLoc is %d\n", ud->mvpLoc);
    return 0;
  }


  // Generate the vertex data
  ud->numIndices = esGenCube( 1.0, &ud->vertices, &ud->normals, NULL, &ud->indices );

  glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );

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
  ESMatrix modelview, proj = {0}; 

  esMatrixLoadIdentity( &modelview );
  esRotateX( &modelview, i += di );       // post-multiply to get from 1 to 2
  esRotateY( &modelview, j += dj );       // again from 2 to 3
  esTranslate( &modelview, x += ud->velX, 0, (z += ud->velZ) - 5.0 );
  esRotateY( &modelview, p += ud->velP ); // Turn camera up and down
  esRotateX( &modelview, r += ud->velR ); // Turn camera right and left
  esPerspective( &proj, 60.0f, ud->aspect, 1.0f, 20.0f );  // post-multiply since this is done on current digidigi
  dispMatrix("proj", &proj);
  dispMatrix("mv", &modelview);
#if 1
  esMatrixMultiply( &ud->mvpMatrix, &proj, &modelview );
#else
  esMatrixMultiply( &ud->mvpMatrix, &modelview, &proj );
#endif
}

// Draw a triangle using the shader pair created in Init()
void Draw ( ESContext *esContext )
{
  UserData *ud = &esContext->ud;
  glViewport ( 0, 0, esContext->width, esContext->height );
  glClear ( GL_COLOR_BUFFER_BIT );
  // TODO see whether this is necessary every time.
  glUseProgram ( ud->programObject );
  // Enable vertex positions.
  glVertexAttribPointer ( ud->positionLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), ud->vertices );
  glEnableVertexAttribArray ( ud->positionLoc );
  //
  // Include the MVP matrix.
  glUniformMatrix4fv( ud->mvpLoc, 1, GL_FALSE, (GLfloat*) &ud->mvpMatrix.m[0][0] );
  // Draw everything.
  glDrawElements ( GL_TRIANGLES, ud->numIndices, GL_UNSIGNED_INT, ud->indices );
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

  free(ud);
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
  GLboolean windowMade = esCreateWindow ( &esContext, "Simple Texture 2D", 720, 480, ES_WINDOW_RGB ); 
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
