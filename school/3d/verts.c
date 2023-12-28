#include <GLES2/gl2.h>
// The below lists 24 vertices with 4 different attributes per vertex:
//   1) Position
//   2) Normal
//   3) Color
//   4) Texel coordinate
//
//   24 * 11 floats per vertex = 264 floats.
GLfloat cubeVerts[] = {
  -0.5f, -0.5f, -0.5f,
  0.0f, -1.0f, 0.0f,
  0.0f, 1.0f, 0.0f,
  0.0f, 0.0f,
  -0.5f, -0.5f,  0.5f,
  0.0f, -1.0f, 0.0f,
  0.0f, 1.0f, 0.0f,
  0.0f, 1.0f,
  0.5f, -0.5f,  0.5f,
  0.0f, -1.0f, 0.0f,
  0.0f, 1.0f, 0.0f,
  1.0f, 1.0f,
  0.5f, -0.5f, -0.5f,
  0.0f, -1.0f, 0.0f,
  0.0f, 1.0f, 0.0f,
  1.0f, 0.0f,
  -0.5f,  0.5f, -0.5f,
  0.0f, 1.0f, 0.0f,
  0.0f, 1.0f, 0.0f,
  1.0f, 0.0f,
  -0.5f,  0.5f,  0.5f,
  0.0f, 1.0f, 0.0f,
  0.0f, 1.0f, 0.0f,
  1.0f, 1.0f,
  0.5f,  0.5f,  0.5f,
  0.0f, 1.0f, 0.0f,
  0.0f, 1.0f, 0.0f,
  0.0f, 1.0f,
  0.5f,  0.5f, -0.5f,
  0.0f, 1.0f, 0.0f,
  0.0f, 1.0f, 0.0f,
  0.0f, 0.0f,
  -0.5f, -0.5f, -0.5f,
  0.0f, 0.0f, -1.0f,
  0.0f, 0.0f, 1.0f,
  0.0f, 0.0f,
  -0.5f,  0.5f, -0.5f,
  0.0f, 0.0f, -1.0f,
  0.0f, 0.0f, 1.0f,
  0.0f, 1.0f,
  0.5f,  0.5f, -0.5f,
  0.0f, 0.0f, -1.0f,
  0.0f, 0.0f, 1.0f,
  1.0f, 1.0f,
  0.5f, -0.5f, -0.5f,
  0.0f, 0.0f, -1.0f,
  0.0f, 0.0f, 1.0f,
  1.0f, 0.0f,
  -0.5f, -0.5f, 0.5f,
  0.0f, 0.0f, 1.0f,
  0.0f, 0.0f, 1.0f,
  0.0f, 0.0f,
  -0.5f,  0.5f, 0.5f,
  0.0f, 0.0f, 1.0f,
  0.0f, 0.0f, 1.0f,
  0.0f, 1.0f,
  0.5f,  0.5f, 0.5f, 
  0.0f, 0.0f, 1.0f,
  0.0f, 0.0f, 1.0f,
  1.0f, 1.0f,
  0.5f, -0.5f, 0.5f,
  0.0f, 0.0f, 1.0f,
  0.0f, 0.0f, 1.0f,
  1.0f, 0.0f,
  -0.5f, -0.5f, -0.5f,
  -1.0f, 0.0f, 0.0f,
  1.0f, 0.0f, 0.0f,
  0.0f, 0.0f,
  -0.5f, -0.5f,  0.5f,
  -1.0f, 0.0f, 0.0f,
  1.0f, 0.0f, 0.0f,
  0.0f, 1.0f,
  -0.5f,  0.5f,  0.5f,
  -1.0f, 0.0f, 0.0f,
  1.0f, 0.0f, 0.0f,
  1.0f, 1.0f,
  -0.5f,  0.5f, -0.5f,
  -1.0f, 0.0f, 0.0f,
  1.0f, 0.0f, 0.0f,
  1.0f, 0.0f,
  0.5f, -0.5f, -0.5f,
  1.0f, 0.0f, 0.0f,
  1.0f, 0.0f, 0.0f,
  0.0f, 0.0f,
  0.5f, -0.5f,  0.5f,
  1.0f, 0.0f, 0.0f,
  1.0f, 0.0f, 0.0f,
  0.0f, 1.0f,
  0.5f,  0.5f,  0.5f,
  1.0f, 0.0f, 0.0f,
  1.0f, 0.0f, 0.0f,
  1.0f, 1.0f,
  0.5f,  0.5f, -0.5f,
  1.0f, 0.0f, 0.0f,
  1.0f, 0.0f, 0.0f,
  1.0f, 0.0f
};
// The reason why some vertex positions get copied is because of separate normals and colors for each side.
// It's just faster in terms of memory access to keep them together when the GPU's processing each one.
// So this means when we store these, we're going to want to compress them even further.
GLushort cubeIndices[] =  
{                    
  0, 2, 1,            
  0, 3, 2,             
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

        </source>
        <source id="Cube-mesh-colors-Attribute" name="Attribute">
