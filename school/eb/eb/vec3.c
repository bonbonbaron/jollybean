#include "vec3.h"

void cross(Vec3* a, Vec3* b, Vec3* out) {
  out->x = a->y * b->z - a->z * b->y;
  out->y = b->x * a->z - b->z * a->x;
  out->z = a->x * b->y - a->y * b->x;
}

float dot(Vec3* a, Vec3* b) {
  return a->x * b->x + a->y * b->y + a->z * b->z;
}

void minus(Vec3* a, Vec3* b, Vec3* out) {
  out->x = a->x - b->x;
  out->y = a->y - b->y;
  out->z = a->z - b->z;
}

