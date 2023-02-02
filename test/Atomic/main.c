#include <stdio.h>
#include <stdatomic.h>

typedef struct {
  char a;
  char b;
  char c;
} ABC;

ABC abc = {
  .a = 10,
  .b = 25,
  .c = 230
};

int main(int argc, char **argv) {
  atomic_int i = 1;
  atomic_uchar j = 1;
  int c = 1;
  void *abcP = &abc;
  void **abcPP = &abcP;

  printf("abc.a is %d\n", **((char**) abcPP));

  printf("atomic int is %d bytes; char %d byte(s)\n", sizeof(i), sizeof(j));
  int out = atomic_compare_exchange_weak(&i, &c, 12);
  printf("i is %d; out is %d\n", i, out);

  i = 5;

  return 0;
}
