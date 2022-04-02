#include <stdio.h>

int main(int argc, char **argv) {
	int i, j, counter, nibble1; 
	FILE *f;

	f = fopen("./bitCountLUT.h", "w");
	fprintf(f, "#include \"jb.h\"\n\n");
	fprintf(f, "Uint8 bitCountLUT[] = {\n");

	for (i = 0; i <= 0xFF; i++) {
		for (j = counter = 0; j < 8; j++) {
			counter += (i >> j) & 1;
		}
		//counter = ((counter - 1) < 0) ? 0 : (counter - 1);
		if (i != 0xFF) 
			fprintf(f, "\t0x%02x,\n", counter);
		else
			fprintf(f, "\t0x%02x\n};", (counter));
	}
	fclose(f);
}

