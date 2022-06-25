#include <stdlib.h>
#include <stdio.h>
#include "data.h"

#include "fluidlite.h"
#include "../../src/fluid_list.h"

#define SAMPLE_RATE 44100
#define SAMPLE_SIZE sizeof(float)
#define NUM_FRAMES SAMPLE_RATE
#define NUM_CHANNELS 2
#define NUM_SAMPLES (NUM_FRAMES * NUM_CHANNELS)

typedef struct _fluid_defsfont_t SF;
struct _fluid_inst_zone_t;
struct _fluid_preset_zone_t;

typedef struct _fluid_inst_zone_t {
  struct _fluid_inst_zone_t* next;
  char* name;
  fluid_sample_t* sample;
  int keylo;
  int keyhi;
  int vello;
  int velhi;
  fluid_gen_t gen[GEN_LAST];
  fluid_mod_t * mod; /* List of modulators */
} fluid_inst_zone_t;

typedef struct _fluid_inst_t {
  char name[21];
  fluid_inst_zone_t* global_zone;
  fluid_inst_zone_t* zone;
} fluid_inst_t;

typedef struct _fluid_preset_zone_t {
  struct _fluid_preset_zone_t* next;
  char* name;
  fluid_inst_t* inst;
  int keylo;
  int keyhi;
  int vello;
  int velhi;
  fluid_gen_t gen[GEN_LAST];
  fluid_mod_t * mod; /* List of modulators */
} fluid_preset_zone_t;


struct _fluid_defpreset_t;
typedef struct _fluid_defpreset_t {
  struct _fluid_defpreset_t* next;
  struct _fluid_defsfont_t* sfont;                  /* the soundfont this preset belongs to */
  char name[21];                        /* the name of the preset */
  unsigned int bank;                    /* the bank number */
  unsigned int num;                     /* the preset number */
  fluid_preset_zone_t* global_zone;        /* the global zone of the preset */
  fluid_preset_zone_t* zone;               /* the chained list of preset zones */
} fluid_def_preset_t;

struct _fluid_defsfont_t
{
  char* filename;           /* the filename of this soundfont */
  unsigned int samplepos;   /* the position in the file at which the sample data starts */
  unsigned int samplesize;  /* the size of the sample data */
  short* sampledata;        /* the sample data, loaded in ram */
  fluid_list_t* sample;      /* the samples in this soundfont */
  struct _fluid_defpreset_t* preset; /* the presets of this soundfont */

  fluid_preset_t iter_preset;        /* preset interface used in the iteration */
  struct _fluid_defpreset_t* iter_cur;       /* the current preset in the iteration */
};

Error fileNameNew(char **fileNamePP, char *objName, char *objType, char *fileExt) {
  U8 nameLen = strlen(objType) + strlen(objName) + strlen(fileExt) + 3;  // +2 for '_', '.', and '\0'
  Error e = jbAlloc((void**) fileNamePP, nameLen, 1);
  if (!e) {
    memset(*fileNamePP, 0, nameLen);
    strcpy(*fileNamePP, objType);
    strcat(*fileNamePP, "_");
    strcat(*fileNamePP, objName);
    strcat(*fileNamePP, ".");
    strcat(*fileNamePP, fileExt);
    (*fileNamePP)[nameLen - 1] = '\0';
  }
  return e;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
      printf("Usage: %s <soundfont> [<output>]\n", argv[0]);
      return 1;
    }

    // Load the soundfont in the fluidsynth format.
    // MB: Because of the way fluidlite is written, I can't simply access the
    //     soundfont through here. I have to write the file-writer inside the code itself.
    fluid_settings_t* settings = new_fluid_settings();
    fluid_synth_t* synthP = new_fluid_synth(settings);
    fluid_synth_sfload(synthP, argv[1], 1);

    fluid_sfont_t *sfP = fluid_synth_get_sfont(synthP, 0);

    // Vars to use through the rest of this hack-a-daisy code.
    fluid_preset_t *presetP;
    fluid_def_preset_t *defPresetP;
    U8 maxBankId = 0;
    U8 currIterN = 0;
    U8 nZones    = 0;
    Error e = SUCCESS;
    //
    // Count banks.
    sfP->iteration_start(sfP);
    presetP = sfP->get_preset(sfP, 0, 0);
    while (sfP->iteration_next(sfP, presetP)) {
      defPresetP = presetP->data;
      if (defPresetP->bank > maxBankId) 
        maxBankId = defPresetP->bank;
    }
    // Count presets in each bank.
    U8 *presetCountsA;
    e = arrayNew((void**) &presetCountsA, sizeof(U8), maxBankId + 1);
    if (!e) {
      sfP->iteration_start(sfP);
      presetP = sfP->get_preset(sfP, 0, 0);
      while (sfP->iteration_next(sfP, presetP)) {
        defPresetP = presetP->data;
        ++presetCountsA[defPresetP->bank];
      }
    }

    // The goal is to get rid of dest field in modulators. 
    if (!e) {
      sfP->iteration_start(sfP);
      presetP = sfP->get_preset(sfP, 0, 0);
      defPresetP = presetP->data;
      // Think this through first: SFs can have many banks; banks can have many presets, and presets many insts/zones.
      // You can't only have *one* preset array. In fact, how come this source code looks like you're
      // dealing with only one preset? This should be a loop to keep the array sizes down.
      for (fluid_preset_zone_t *zoneP = defPresetP->zone; zoneP; zoneP = zoneP->next) ;
        //zoneHasAnyModulatorsA[[->mod != NULL)
    }

#ifdef _WIN32
    char FILE_SEP = '\\';
#else
    char FILE_SEP = '/';
#endif

    char INCLUDE_DIRECTIVES[] = "#include \"botox/data.h\"\n#include \"soundfont.h\"\n\n";

    if (!e && sfP) {
      // First, get the soundfont's name.
      U8 filenameLen = strlen(argv[1]);
      // Get the basename of the file without the extension. This'll be the soundfont's name.
      U8 basenameLastIdx = filenameLen - 1;
      U8 basenameFirstIdx = 0;
      for (U8 i = filenameLen - 1; i >= 0; --i) {
        char currChar = argv[1][i];
        if (currChar == FILE_SEP) {
          basenameFirstIdx = i + 1;
          break;
        }
        // This may not look rigorous, but we'll strcmp to ensure substring is ".sf3" afterward!
        else if (currChar == '.' && basenameLastIdx == (filenameLen - 1))  // Only capture the last '.' position.
          basenameLastIdx = i;
      }

      char *fileExtP = argv[1] + basenameLastIdx;
      char *basename;
      if (!strcmp(".sf3", fileExtP)) {  // file is sf3.
        basename = malloc(sizeof(char) * ((basenameLastIdx - basenameFirstIdx) + 1));
        if (!basename)
          return E_NO_MEMORY;
        char *baseCharP = basename;
        for (U8 i = basenameFirstIdx; i < basenameLastIdx; ++i)
          *(baseCharP++) = argv[1][i];
        *baseCharP = '\0';
        printf("File is .sf3 format. Its basename is %s.\n", basename);
      }
      else {
        printf("File is NOT .sf3 format. It's actually %s.\n", fileExtP);
        return 0;
      }
      //TODO make a standard set of include directives to prepend to each file.
      // Soundfont
      char soundfontFilename[500];
      sprintf(soundfontFilename, "sf%s.h", basename);
      FILE *soundfontFP = fopen(soundfontFilename, "w");
      
      // Standardize strings to lessen the chance of mistyping any.*
      char BANK_SEED_ARR_FORMAT[] = "%sBankSeedA";  // %s = basename
      char BANK_SEED_OBJ_FORMAT[] = "sf%sBank%d";   // %s = basename, %d = bank number
      char PRESET_SEED_ARR_FORMAT[] = "sf%sBank%dPresetA";  // %s = basename, %d bank number
      char PRESET_SEED_OBJ_FORMAT[] = "sf%sBank%dPreset%d";  // %s = basename, %d = bank number, %d = preset number

      if (!e && soundfontFP) {
        fprintf(soundfontFP, INCLUDE_DIRECTIVES);
        // Write the bank seed pointer array first.
        fprintf(soundfontFP, "BankSeed");
        fprintf(soundfontFP, BANK_SEED_ARR_FORMAT, basename);
        fprintf(soundfontFP, " = {\n");
        for (U8 bankCounter = 0; bankCounter <= maxBankId; ++bankCounter) {
          fprintf(soundfontFP, "&");
          fprintf(soundfontFP, BANK_SEED_OBJ_FORMAT, basename, bankCounter);
          if (bankCounter < maxBankId)
            fprintf(soundfontFP, ", ");
        fprintf(soundfontFP, "};\n\n");
        // Write the soundfont seed.
        fprintf(soundfontFP, "Soundfont %sSoundfont = {\n", basename);
        fprintf(soundfontFP, "\t.nBankSeeds = %d;\n", maxBankId + 1);
        fprintf(soundfontFP, "\t.bankSeedA  = %sBankSeedA;\n", basename);
        fprintf(soundfontFP, "};\n\n");
        fclose(soundfontFP);
        // Banks
        for (U8 bankIdx = 0; bankIdx <= maxBankId; ++bankIdx) {
          char bankFileName[500] = {0};
          sprintf(bankFileName, "sf%sBank%d.h", basename, bankIdx);
          FILE *bankFP = fopen(bankFileName, "w");
          if (!e && bankFP) {
            fprintf(bankFP, INCLUDE_DIRECTIVES);
            sfP->iteration_start(sfP);
            presetP = sfP->get_preset(sfP, 0, 0);
            // Count the number of presets in this bank.
            U8 nPresets = 0;
            defPresetP = presetP->data;
            for (fluid_def_preset_t *currPresetP = defPresetP; currPresetP; currPresetP = currPresetP->next) 
              if (currPresetP->bank == bankIdx)
                ++nPresets;
            // Write bank's array of preset seeds.
            U8 currPresetIdx = 0;
            defPresetP = presetP->data;
            fprintf(bankFP, "PresetSeed *");
            fprintf(bankFP, PRESET_SEED_ARR_FORMAT, basename, bankIdx);
            fprintf(bankFP, " = {\n");
            // Write each preset seed element of bank's preset seed array.
            for (fluid_def_preset_t *currPresetP = defPresetP; currPresetP; currPresetP = currPresetP->next) {
              if (currPresetP->bank == bankIdx) { // if this preset belongs to this bank...
                fprintf(bankFP, PRESET_SEED_OBJ_FORMAT, basename, bankIdx, currPresetIdx);
                if (++currPresetIdx == nPresets) 
                  break;
                fprintf(bankFP, ", ");
              }
            }
            fprintf(bankFP, "};\n\n");  // end current bank's preset seed array
            // Write the bank seed structure to the current file.
            fprintf(bankFP, "BankSeed *");
            fprintf(bankFP, BANK_SEED_ARR_FORMAT, basename, bankIdx);
            fprintf(bankFP, " = {\n");
            fprintf(bankFP, "\t.nPresetSeeds = %d", nPresets);
            fprintf(bankFP, "\t.presetSeedA  = {");
            fclose(bankFP);
            // For each preset in the current bank...
            sfP->iteration_start(sfP);
            presetP = sfP->get_preset(sfP, 0, 0);
            fluid_preset_zone_t *pZoneP;
            currPresetIdx = 0;
            // So tell each zone whether it has a modulator array (we'll know what the name is by zone #)
            // with an array of booleans (U8).
            U8 zoneGenHasAnyModulatorsA[256][64] = {0};

            char PRESET_GLOBAL_ZONE_MOD_ARR_FORMAT[] = "%sBank%dPreset%dGlobalZoneModA";  // sfont name, bank num, preset num
            char PRESET_ZONE_MOD_ARR_FORMAT[] = "%sBank%dPreset%dZone%dModA";  // sfont name, bank num, preset num, zone num
            while (sfP->iteration_next(sfP, presetP)) {
              fluid_def_preset_t *currPresetP = presetP->data;
              if (defPresetP->bank == bankIdx) {
                // Create preset seed file.
                char presetFilename[500];
                sprintf(presetFilename, "sf%sBank%dPreset%d.h", basename, bankIdx, currPresetIdx++);
                FILE *presetFP = fopen(presetFilename, "w");
                // Write includes.
                fprintf(presetFP, INCLUDE_DIRECTIVES);
                // Global zone
                fluid_preset_zone_t *presetGlobalZoneP = defPresetP->global_zone;
                // Global zone's modulators
                // Modulators are read one by one in a whole array and tested for amounts before processing. 
                U8 modCounter = 0;
                U8 presetGlobalZoneGenHasAnyModulators = 0;
                // Each preset can have up to 256 zones, and each zone has 60 generators.
                if (presetGlobalZoneP->mod) {
                  presetGlobalZoneGenHasAnyModulators = 0;
                  fprintf(presetFP, "Modulator ");
                  fprintf(presetFP, PRESET_GLOBAL_ZONE_MOD_ARR_FORMAT, basename, bankIdx, currPresetIdx);
                  fprintf(presetFP, "= {\n");
                  for (fluid_mod_t *modP = presetGlobalZoneP->mod; modP; modP = modP->next) {
                    fprintf(presetFP, "\t{\n");
                    fprintf(presetFP, "\t\t.src1 = %d;\n", modP->src1);
                    fprintf(presetFP, "\t\t.src2 = %d;\n", modP->src2);
                    fprintf(presetFP, "\t\t.xformType1 = %d;\n", modP->flags1);
                    fprintf(presetFP, "\t\t.xformType2 = %d;\n", modP->flags2);
                    fprintf(presetFP, "\t\t.productScale = %d;\n", (S16) modP->amount);
                    fprintf(presetFP, "\t},\n");
                  }
                  fprintf(presetFP, "};");  // end global modulator array
                }
                // Individual zones' generators
                // Loop through all zone's modulators can determine which generators own any modulators. 
                // Fluidsynth does this a little backwards.
                U8 zoneIdx = 0;
                U8 currZoneIdx = 0;
                U8 currGenIdx= 0;
#define MAX_N_ZONES 256
#define MAX_N_GENS   60
                // Keeps track of whether a zone's gen has any modulators.
                U8 presetZoneGenHasAnyModulators[MAX_N_ZONES][MAX_N_GENS] = {0};  
                for (pZoneP = defPresetP->zone; pZoneP; pZoneP = pZoneP->next, ++currZoneIdx) 
                  for (fluid_mod_t *modP = pZoneP->mod; modP; modP = modP->next) 
                    zoneGenHasAnyModulatorsA[currZoneIdx][modP->dest] = 1;  // avoids setting this zone's modA to NULL
                // Now that we know which generators have modulators, we can make a mod array for each generator
                // Iterate through all modulators. Determine which gens have any modulators too.
                // before writing the gens.
                zoneIdx = 0;
                currZoneIdx = 0;
                currGenIdx= 0;
                // Write a modulator array for each generator that has any modulators. What a horrible algo!! LOL
                for (pZoneP = defPresetP->zone; pZoneP; pZoneP = pZoneP->next, ++currZoneIdx) {
                  for (U8 genIdx = 0; genIdx < 60; ++genIdx) {
                    if (zoneGenHasAnyModulatorsA[currZoneIdx][genIdx]) {
                      // Start the array for this generator's modulators.
                      fprintf(presetFP, "Modulator ");
                      fprintf(presetFP, PRESET_ZONE_MOD_ARR_FORMAT, basename, bankIdx, currPresetIdx, zoneIdx++);
                      fprintf(presetFP, "[] = {\n");
                      for (fluid_mod_t *modP = pZoneP->mod; modP; modP = modP->next) {
                        if (modP->dest == genIdx) {
                          fprintf(presetFP, "\t{\n");
                          fprintf(presetFP, "\t\t.src1 = %d;\n", modP->src1);
                          fprintf(presetFP, "\t\t.src2 = %d;\n", modP->src2);
                          fprintf(presetFP, "\t\t.xformType1 = %d;\n", modP->flags1);
                          fprintf(presetFP, "\t\t.xformType2 = %d;\n", modP->flags2);
                          fprintf(presetFP, "\t\t.productScale = %d;\n", (S16) modP->amount);
                          fprintf(presetFP, "\t},\n");
                        }  // if this modulator belongs to the current generator
                      }  // for each modulator in this zone
                      fprintf(presetFP, "};\n\n");  // Close this generator's modulator array.
                    }  // if gen has any mods
                  }  // for each of this zone's 60 generators
                }  // for each zone in this preset
                // Now that we wrote all the mod arrays (PHEW!!), let's create the generators that own them.
                for (pZoneP = defPresetP->zone; pZoneP; pZoneP = pZoneP->next, ++currZoneIdx) {
                  for (U8 genIdx = 0; genIdx < 60; ++genIdx) {
                    if (zoneGenHasAnyModulatorsA[currZoneIdx][genIdx]) {

                }
              }  // if this preset belongs to the current bank
            }  // for each preset in the soundfont
          }  // if bank file opened successfully 
        }  // for each bank in this soundfont
#if 0
      // Instruments
      e = fileNew(&fileP, "myInstr", "InstrumentSeed", "h");
      if (!e && fileP) {
        fprintf(fileP->fP, "#include <botox/data.h>\n\n");
        fWriteStructHeader(fileP->fP, "InstrumentSeed", "myInstrSeed");
        fWriteStructMemberStr(fileP->fP, "bankSeedA", "idunnoyet");
        fWriteStructFooter(fileP->fP);
      }
      fileDel(&fileP);
      // Samples
      e = fileNew(&fileP, "mySample", "Sample", "h");
      if (!e && fileP) {
        fprintf(fileP->fP, "#include <botox/data.h>\n\n");
        fWriteStructHeader(fileP->fP, "Sample", "sample");
        fWriteStructMemberStr(fileP->fP, "bankSeedA", "idunnoyet");
        fWriteStructFooter(fileP->fP);
      }
      fileDel(&fileP);
#endif
    }

    //SF *sfP = (SF*) synthP->sfont->data;
    //float* buffer = calloc(SAMPLE_SIZE, NUM_SAMPLES);
    //FILE* file = argc > 2 ? fopen(argv[2], "wb") : stdout;
    //fluid_synth_noteon(synthP, 0, 60, 127);
    //fluid_synth_write_float(synthP, NUM_FRAMES, buffer, 0, NUM_CHANNELS, buffer, 1, NUM_CHANNELS);
    //fwrite(buffer, SAMPLE_SIZE, NUM_SAMPLES, file);

    //fluid_synth_noteoff(synthP, 0, 60);
    //fluid_synth_write_float(synthP, NUM_FRAMES, buffer, 0, NUM_CHANNELS, buffer, 1, NUM_CHANNELS);
    //fwrite(buffer, SAMPLE_SIZE, NUM_SAMPLES, file);

    //fclose(file);

    //
    //free(buffer);

    //arrayDel((void**) &presetCountsA);
    delete_fluid_synth(synthP);
    delete_fluid_settings(settings);
}
