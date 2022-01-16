#include "jb.h"

U32 hp = 200;
U32 mp = 300;
typedef struct {
	U8 stat;
	U32 type;
	U16 def;
} Test;
Test test = {1, 2, 3};
BBSeed_(mb, {1, &hp}, {2, &mp}, {5, &test});
