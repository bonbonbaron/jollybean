#include <stdio.h>
#include <stdatomic.h>

int main(int argc, char **argv) {
  atomic_flag k = ATOMIC_FLAG_INIT;
  printf("size of flag: %d byte(s)\n", sizeof(k));
  char oldFlag = atomic_flag_test_and_set(&k);
  printf("old flag: %d; new one: %d\n", oldFlag, k);
  oldFlag = atomic_flag_test_and_set(&k);
  printf("old flag: %d; new one: %d\n", oldFlag, k);

  return 0;
}
