#include <stdio.h>

/* formula:
 *  0 --> 1    (1 << 0) | ((1 << 0) - 1)   =   1
 *  1 --> 3    (1 << 1) | ((1 << 1) - 1)   =   3
 *  2 --> 7    (1 << 2) | ((1 << 2) - 1)   =   7
 *  3 --> 15
 *  4 --> 31
 *  5 --> 63
 *  6 --> 127
 *  7 --> 255
 * */
int main(int argc, char **argv) {
	unsigned char i, j, k, cntMask;
	FILE *f;

	f = fopen("./bitCountMaskLUT.h", "w");
	fprintf(f, "#include \"jb.h\"\n\n");
	fprintf(f, "Uint8 bitCountMaskLUT[] = {\n");

	for (i = 0; i <= 0xFF; i++) {
		k = (1 << (i & 0x07));
		cntMask = k | (k - 1);  // bit 0:
		if (i != 0xFF) 
			fprintf(f, "\t0x%02x,\t// key = %d\n", cntMask, i);
		else {
			fprintf(f, "\t0x%02x\t// key = %d\n};", cntMask, i);
			break;
		}
	}
	fclose(f);
}

