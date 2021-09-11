#include <stdio.h>

int main(int argc, char **argv) {
	unsigned char i, j, k, bitFlag;
	FILE *f;

	f = fopen("./bitFlagLUT.h", "w");
	fprintf(f, "#include \"jb.h\"\n\n");
	fprintf(f, "Uint8 bitFlagLUT[] = {\n");

	for (i = 0; i <= 0xFF; i++) {
		bitFlag = (1 << (i & 0x07));
		if (i != 0xFF) 
			fprintf(f, "\t0x%02x,\t// key = %d\n", bitFlag, i);
		else {
			fprintf(f, "\t0x%02x\t// key = %d\n};", bitFlag, i);
			break;
		}
	}
	fclose(f);
}

