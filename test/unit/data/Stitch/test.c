#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "data.h"


typedef struct {
  U32 a;
  S8  b;
  S16 c;
} A;  // 8 bytes

typedef struct {
  S8  b;
  S16 d;
  A   a;
  U32 c;
} B;

static void reportDataFacts() {
  printf("size of elems n A and B:\n========================\n \
          \tA.a: %d bytes,\n \
          \tA.b: %d bytes,\n \
          \tA.c: %d bytes,\n\n \
          \tB.b: %d bytes,\n \
          \tB.d: %d bytes,\n \
          \tB.a: %d bytes,\n \
          \tB.c: %d bytes,\n", \
          structMemberSizeof_(A, a), \
          structMemberSizeof_(A, b), \
          structMemberSizeof_(A, c), \
          structMemberSizeof_(B, b), \
          structMemberSizeof_(B, d), \
          structMemberSizeof_(B, a), \
          structMemberSizeof_(B, c));
  printf("offset of elems n A:\n==================== \
          \tA.a: %d bytes from struct's beginning,\n \
          \tA.b: %d bytes from struct's beginning,\n \
          \tA.c: %d bytes from struct's beginning,\n\n \
          \tB.b: %d bytes from struct's beginning,\n \
          \tB.d: %d bytes from struct's beginning,\n \
          \tB.a: %d bytes from struct's beginning,\n \
          \tB.c: %d bytes from struct's beginning,\n", \
          structMemberOffset_(A, a), \
          structMemberOffset_(A, b), \
          structMemberOffset_(A, c), \
          structMemberOffset_(B, b), \
          structMemberOffset_(B, d), \
          structMemberOffset_(B, a), \
          structMemberOffset_(B, c));
}

static void reportValues(char *prelude, A *aP, B *bP) {
  printf("%s\n===============\n", prelude);

  printf("A:\n\ta = %d,\n\tb = %d,\n\tc = %d,\n", aP->a, aP->b, aP->c);
  printf("B:\n\tb = %d,\n\td = %d,\n\ta = {%3d, %3d, %3d},\n\tc = %d\n", 
      bP->b, 
      bP->d,
      bP->a.a, bP->a.b, bP->a.c,
      bP->c
  );
}

int main(int argc, char **argv) {
  A a = {
    .a = 12,
    .b = -34,
    .c = -56
  };
  B b = {
    .b = -12,
    .d = -4,
    .a = {0},
    .c = 2
  };
  B bOrig = b;
  reportDataFacts();
  reportValues("Initialization:", &a, &b);
  memcpy((U8*) &b + structMemberOffset_(B, a), &a, sizeof(A));
  reportValues("Stitching a into b", &a, &b);

  // Assert b's non-a components are unchanged.
  assert(b.b == bOrig.b);
  assert(b.c == bOrig.c);
  assert(b.d == bOrig.d);
  // Assert a got stitched into b successfully.
  assert(!memcmp(&b.a, &a, sizeof(A)));

  return 0;
}
