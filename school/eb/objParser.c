/* obj_load.c — Blender OBJ → ObjResult
 * Quads are split 0-1-2 and 0-2-3.
 *
 * Expected neighboring types (adjust names if yours differ):
 *   typedef struct { float x, y; }       Vec2;
 *   typedef struct { float x, y, z; }    Vec3;
 *   typedef struct { float x, y, z, w; } Vec4;
 *   typedef struct { int v[3], vt[3], vn[3]; } Triangle;
 *
 * Usage:
 *   ObjResult pos = {0}, tex = {0}, nrm = {0}, faces = {0};
 *   if (obj_load("model.obj", &pos, &tex, &nrm, &faces) != 0) ...
 *   // pos.u.vec3A[0..pos.count-1]
 *   // faces.u.triA[0..faces.count-1]
 *   obj_free(&pos); obj_free(&tex); obj_free(&nrm); obj_free(&faces);
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <float.h>

#ifndef OBJ_LOAD_TYPES_DEFINED
typedef struct { float x, y; }       Vec2;
typedef struct { float x, y, z; }    Vec3;
typedef struct { float x, y, z, w; } Vec4;
typedef struct { int v[3], vt[3], vn[3]; } Triangle;
#endif

/* Residual / Quantized are caller-owned; leave zeroed. */
#ifndef OBJ_LOAD_RESULT_DEFINED
typedef struct Residual { int unused; } Residual;
typedef struct Quantized { int unused; } Quantized;

typedef struct {
  int count;
  int stride;
  char* valString;
  union {
    int scalar;
    float* scalarA;
    Vec2* vec2A;
    Vec3* vec3A;
    Vec4* vec4A;
    Triangle* triA;
  } u;
  union {
    float scalar;
    Vec2 vec2;
    Vec3 vec3;
    Vec4 vec4;
    Triangle* triA;
  } min;
  union {
    float scalar;
    Vec2 vec2;
    Vec3 vec3;
    Vec4 vec4;
    Triangle* triA;
  } max;
  Residual residual;
  Quantized quantized;
} ObjResult;
#endif

static void* xrealloc(void* p, size_t n) {
  void* q = realloc(p, n);
  if (!q && n) { fprintf(stderr, "obj_load: OOM\n"); exit(1); }
  return q;
}

static char* slurp(const char* path, long* out_len) {
  FILE* f = fopen(path, "rb");
  if (!f) return NULL;
  // Get the number of bytes in this file.
  fseek(f, 0, SEEK_END);  //  repositions the file position indicator
  long n = ftell(f);  // returns the current byte offset of the file position indicator
  fseek(f, 0, SEEK_SET);
  // Allocate for number of bytes, which we'll null-terminate with an extra one at the end.
  char* s = (char*)malloc((size_t)n + 1);
  if (!s) { fclose(f); return NULL; }
  // Read file contents into s.
  if (fread(s, 1, (size_t)n, f) != (size_t)n) { free(s); fclose(f); return NULL; }
  s[n] = 0;
  fclose(f);
  if (out_len) *out_len = n;
  return s;
}

static int parse_index(const char* tok, int count) {
  /* OBJ is 1-based; negatives count from end. 0 = unused. */
  if (!tok || !*tok) return 0;
  int v = atoi(tok);
  if (v > 0) return v - 1;
  if (v < 0) return count + v;
  return 0;
}

static void parse_corner(const char* tok, int nv, int nvt, int nvn,
                         int* v, int* vt, int* vn) {
  *v = *vt = *vn = -1;
  if (!tok || !*tok) return;
  char buf[128];
  strncpy(buf, tok, sizeof(buf) - 1);  // copy a maximum of n characters; pads excess w/ null-terms
  buf[sizeof(buf) - 1] = 0;  // null-term the ending
  char* s1 = strchr(buf, '/');  // finds the first appearance of a specific character
  if (!s1) { *v = parse_index(buf, nv); return; }
  *s1 = 0;
  *v = parse_index(buf, nv);
  char* s2 = strchr(s1 + 1, '/');
  if (!s2) { *vt = parse_index(s1 + 1, nvt); return; }
  *s2 = 0;
  if (s1[1]) *vt = parse_index(s1 + 1, nvt);
  *vn = parse_index(s2 + 1, nvn);
}

void obj_free(ObjResult* r) {
  if (!r) return;
  free(r->valString);
  /* union pointers share storage — free the one that was used */
  if (r->stride == (int)sizeof(Triangle) && r->u.triA) free(r->u.triA);
  else if (r->u.scalarA) free(r->u.scalarA);
  memset(r, 0, sizeof(*r));
}

int obj_load(const char* path,
             ObjResult* pos,   /* v  → vec3A  */
             ObjResult* tex,   /* vt → vec2A, may be NULL */
             ObjResult* nrm,   /* vn → vec3A, may be NULL */
             ObjResult* faces) /* f  → triA (triangulated) */
{
  long len = 0;
  char* text = slurp(path, &len);
  if (!text) return -1;

  if (pos)   { memset(pos,   0, sizeof(*pos));   pos->valString   = NULL; }
  if (tex)   { memset(tex,   0, sizeof(*tex));   }
  if (nrm)   { memset(nrm,   0, sizeof(*nrm));   }
  if (faces) { memset(faces, 0, sizeof(*faces)); }

  /* pass 1: counts */
  int nv = 0, nvt = 0, nvn = 0, ntri = 0;
  for (char* p = text; *p; ) {
    char* line = p;
    char* nl = strchr(p, '\n');
    if (nl) { *nl = 0; p = nl + 1; } else { p += strlen(p); }
    while (*line == ' ' || *line == '\t') line++;
    if (line[0] == 'v' && line[1] == ' ') nv++;
    else if (line[0] == 'v' && line[1] == 't' && (line[2] == ' ' || line[2] == '\t')) nvt++;
    else if (line[0] == 'v' && line[1] == 'n' && (line[2] == ' ' || line[2] == '\t')) nvn++;
    else if (line[0] == 'f' && (line[1] == ' ' || line[1] == '\t')) {
      int corners = 0;
      char* q = line + 1;
      while (*q) {
        while (*q == ' ' || *q == '\t') q++;
        if (!*q) break;
        corners++;
        while (*q && *q != ' ' && *q != '\t') q++;
      }
      if (corners >= 3) ntri += corners - 2; /* fan */
    }
  }

  Vec3* V  = (Vec3*)calloc((size_t)nv,  sizeof(Vec3));
  Vec2* VT = nvt ? (Vec2*)calloc((size_t)nvt, sizeof(Vec2)) : NULL;
  Vec3* VN = nvn ? (Vec3*)calloc((size_t)nvn, sizeof(Vec3)) : NULL;
  Triangle* T = (Triangle*)calloc((size_t)ntri, sizeof(Triangle));
  if ((nv && !V) || (ntri && !T) || (nvt && !VT) || (nvn && !VN)) {
    free(V); free(VT); free(VN); free(T); free(text);
    return -1;
  }

  /* pass 2: fill. Re-walk — we zeroed newlines, so restore by scanning original?
     slurp buffer had newlines replaced with 0; we need a fresh copy. */
  free(text);
  text = slurp(path, &len);
  if (!text) { free(V); free(VT); free(VN); free(T); return -1; }

  int iv = 0, ivt = 0, ivn = 0, it = 0;
  Vec3 pmin = { FLT_MAX, FLT_MAX, FLT_MAX }, pmax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
  Vec2 tmin = { FLT_MAX, FLT_MAX }, tmax = { -FLT_MAX, -FLT_MAX };
  Vec3 nmin = { FLT_MAX, FLT_MAX, FLT_MAX }, nmax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

  for (char* p = text; *p; ) {
    char* line = p;
    char* nl = strchr(p, '\n');
    if (nl) { *nl = 0; p = nl + 1; } else { p += strlen(p); }
    if (line[0] && line[strlen(line)-1] == '\r') line[strlen(line)-1] = 0;
    while (*line == ' ' || *line == '\t') line++;

    if (line[0] == 'v' && line[1] == ' ') {
      Vec3 a;
      if (sscanf(line + 2, "%f %f %f", &a.x, &a.y, &a.z) >= 3 && iv < nv) {
        V[iv++] = a;
        if (a.x < pmin.x) pmin.x = a.x; if (a.x > pmax.x) pmax.x = a.x;
        if (a.y < pmin.y) pmin.y = a.y; if (a.y > pmax.y) pmax.y = a.y;
        if (a.z < pmin.z) pmin.z = a.z; if (a.z > pmax.z) pmax.z = a.z;
      }
    } else if (line[0] == 'v' && line[1] == 't' && (line[2] == ' ' || line[2] == '\t')) {
      Vec2 a = {0, 0};
      sscanf(line + 3, "%f %f", &a.x, &a.y);
      if (ivt < nvt) {
        VT[ivt++] = a;
        if (a.x < tmin.x) tmin.x = a.x; if (a.x > tmax.x) tmax.x = a.x;
        if (a.y < tmin.y) tmin.y = a.y; if (a.y > tmax.y) tmax.y = a.y;
      }
    } else if (line[0] == 'v' && line[1] == 'n' && (line[2] == ' ' || line[2] == '\t')) {
      Vec3 a;
      if (sscanf(line + 3, "%f %f %f", &a.x, &a.y, &a.z) >= 3 && ivn < nvn) {
        VN[ivn++] = a;
        if (a.x < nmin.x) nmin.x = a.x; if (a.x > nmax.x) nmax.x = a.x;
        if (a.y < nmin.y) nmin.y = a.y; if (a.y > nmax.y) nmax.y = a.y;
        if (a.z < nmin.z) nmin.z = a.z; if (a.z > nmax.z) nmax.z = a.z;
      }
    } else if (line[0] == 'f' && (line[1] == ' ' || line[1] == '\t')) {
      /* collect up to 16 corners, fan-triangulate */
      int cv[16], ct[16], cn[16], nc = 0;
      char* q = line + 1;
      while (*q && nc < 16) {
        while (*q == ' ' || *q == '\t') q++;
        if (!*q) break;
        char tok[128]; int ti = 0;
        while (*q && *q != ' ' && *q != '\t' && ti < 127) tok[ti++] = *q++;
        tok[ti] = 0;
        parse_corner(tok, iv, ivt, ivn, &cv[nc], &ct[nc], &cn[nc]);
        nc++;
      }
      for (int k = 1; k + 1 < nc && it < ntri; k++) {
        Triangle t;
        t.v[0]  = cv[0]; t.v[1]  = cv[k]; t.v[2]  = cv[k + 1];
        t.vt[0] = ct[0]; t.vt[1] = ct[k]; t.vt[2] = ct[k + 1];
        t.vn[0] = cn[0]; t.vn[1] = cn[k]; t.vn[2] = cn[k + 1];
        T[it++] = t;
      }
    }
  }

  if (pos) {
    pos->count = iv;
    pos->stride = (int)sizeof(Vec3);
    pos->u.vec3A = V;
    pos->min.vec3 = pmin;
    pos->max.vec3 = pmax;
    pos->valString = NULL;
    V = NULL; /* owned by pos */
  }
  if (tex) {
    tex->count = ivt;
    tex->stride = (int)sizeof(Vec2);
    tex->u.vec2A = VT;
    tex->min.vec2 = tmin;
    tex->max.vec2 = tmax;
    VT = NULL;
  }
  if (nrm) {
    nrm->count = ivn;
    nrm->stride = (int)sizeof(Vec3);
    nrm->u.vec3A = VN;
    nrm->min.vec3 = nmin;
    nrm->max.vec3 = nmax;
    VN = NULL;
  }
  if (faces) {
    faces->count = it;
    faces->stride = (int)sizeof(Triangle);
    faces->u.triA = T;
    T = NULL;
  }

  free(V); free(VT); free(VN); free(T);
  free(text);
  return 0;
}  // obj_load()
