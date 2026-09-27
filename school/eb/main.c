#include <stdio.h>
#include "eb.h"
#include "objParser.h"

int main(int argc, char** argv) {
  if (argc < 2) { fprintf(stderr, "usage: %s file.obj\n", argv[0]); return 1; }
  ObjResult pos = {0}, tex = {0}, nrm = {0}, faces = {0};
  if (obj_load(argv[1], &pos, &tex, &nrm, &faces) != 0) {
    fprintf(stderr, "failed to load %s\n", argv[1]);
    return 1;
  }
  printf("v=%d vt=%d vn=%d tri=%d\n", pos.count, tex.count, nrm.count, faces.count);
  if (pos.count)
    printf("bbox (%.3f,%.3f,%.3f)-(%.3f,%.3f,%.3f)\n",
           pos.min.vec3.x, pos.min.vec3.y, pos.min.vec3.z,
           pos.max.vec3.x, pos.max.vec3.y, pos.max.vec3.z);
  obj_free(&pos); obj_free(&tex); obj_free(&nrm); obj_free(&faces);
  return 0;
}


