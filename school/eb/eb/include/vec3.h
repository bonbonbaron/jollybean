#ifndef VEC3_H
#define VEC3_H

typedef struct {
  float x, y, z;
} Vec3;

void cross(Vec3* a, Vec3* b, Vec3* out);

float dot(Vec3* a, Vec3* b);

void minus(Vec3* a, Vec3* b, Vec3* out);


#endif

