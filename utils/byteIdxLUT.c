#include <stdio.h>

int main(int argc, char **argv) {
	int i; 
	FILE *f;

	f = fopen("./byteIdx.h", "w");
	fprintf(f, "#include \"jb.h\"\n\n");
	fprintf(f, "Uint8 byteIdxLUT[] = {\n");

	for (i = 0; i <= 0xFF; i++) {
		if (i != 0xFF) 
			fprintf(f, "\t0x%02x,\n", i >> 3);
		else
			fprintf(f, "\t0x%02x\n};", (i >> 3));
	}
	fclose(f);
}

