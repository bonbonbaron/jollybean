#include <stdlib.h>
#include <stdio.h>

#include "fluidlite.h"

#define SAMPLE_RATE 44100
#define SAMPLE_SIZE sizeof(float)
#define NUM_FRAMES SAMPLE_RATE
#define NUM_CHANNELS 2
#define NUM_SAMPLES (NUM_FRAMES * NUM_CHANNELS)

/*
 *
struct _fluid_defsfont_t {
	U32 samplepos;				// the position in the file at which the sample data starts 
	U32 samplesize;			// the size of the sample data 
	S16 *sampledata;						// the sample data, loaded in ram 
	fluid_list_t *sample;					// the samples in this soundfont 
	fluid_defpreset_t *preset;		// the presets of this soundfont 
	fluid_preset_t iter_preset;		// preset interface used in the iteration 
	fluid_defpreset_t *iter_cur;	// the current preset in the iteration 
};
*/

typedef struct _fluid_defsfont_t SF;

int main(int argc, char *argv[]) {
    if (argc < 2) {
      printf("Usage: %s <soundfont> [<output>]\n", argv[0]);
      return 1;
    }

    // MB: Because of the awkward way fluidlite is written, I can't simply access the
    //     soundfont through here. I have to write the file-writer inside the code itself.
    fluid_settings_t* settings = new_fluid_settings();
    fluid_synth_t* synth = new_fluid_synth(settings);
    fluid_synth_sfload(synth, argv[1], 1);

    //SF *sfP = (SF*) synth->sfont->data;
    //float* buffer = calloc(SAMPLE_SIZE, NUM_SAMPLES);
    //FILE* file = argc > 2 ? fopen(argv[2], "wb") : stdout;
    //fluid_synth_noteon(synth, 0, 60, 127);
    //fluid_synth_write_float(synth, NUM_FRAMES, buffer, 0, NUM_CHANNELS, buffer, 1, NUM_CHANNELS);
    //fwrite(buffer, SAMPLE_SIZE, NUM_SAMPLES, file);

    //fluid_synth_noteoff(synth, 0, 60);
    //fluid_synth_write_float(synth, NUM_FRAMES, buffer, 0, NUM_CHANNELS, buffer, 1, NUM_CHANNELS);
    //fwrite(buffer, SAMPLE_SIZE, NUM_SAMPLES, file);

    //fclose(file);

    //free(buffer);

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}
