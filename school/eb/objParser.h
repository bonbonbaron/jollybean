#ifndef OBJ_PARSER_H
#define OBJ_PARSER_H

int obj_load(const char* path,
             ObjResult* pos,    /* v  → vec3A  */
             ObjResult* tex,    /* vt → vec2A, may be NULL */
             ObjResult* nrm,    /* vn → vec3A, may be NULL */
             ObjResult* faces); /* f  → triA (triangulated) */

void obj_free(ObjResult* r);

#endif
