#include <stdio.h>

typedef enum EA { A, B, C } EA;

typedef struct K { char k; } K;

typedef struct ABC {
  char a, b, c;
} ABC;

typedef struct ABCK {
  K k;
  ABC abc;
} ABCK;

#define size( x_ ) printf( "sizeof %s: %d\n", #x_, sizeof(x_))

void main () {
  ABCK abck = {
    .k = {
      .k = 55
    },
    .abc = {
      .a = 12,
      .b = 34,
      .c = 56
    }
  };
  size(ABCK);
  printf("sizeof k is %d\n", sizeof(abck.k));
  printf("sizeof abc is %d\n", sizeof(abck.abc));
}
