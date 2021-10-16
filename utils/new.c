#include <stdio.h>

typedef unsigned char U8;
int main(int argc, char **argv) {
	register U8 i, j, k, bitFlag;

	i = 5;
	i |= (i - 1);
	printf("i = %d\n", i);
	return 0;
}
