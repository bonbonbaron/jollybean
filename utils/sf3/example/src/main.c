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

/*
// Seeds prevent duplicate data storage in ROM. Only samples are stored identical to runtime format.

typedef struct {
  U8     nZones;
  Zone **zonePA;
} InstrumentSeed;

typedef InstrumentSeed PresetSeed;

typedef struct {
  U8     nPresets;
  Preset **presetPA;
} BankSeed;

typedef struct {
  U8     nBankSeeds;
  Bank  *bankSeedA;
} SoundfontSeed;
*/

// Writing structs
void fWriteStructHeader(FILE *fP, char *type, char *name) {
  fprintf(fP, "%s %s = {\n", type, name);
}

void fWriteStructFooter(FILE *fP) {
  fprintf(fP, "};\n\n");
}

void fWriteStructMemberNum(FILE *fP, char *fieldName, U32 num) {
  char strP[10];
  sprintf(strP, "%d", num);
  fprintf(fP, "\t.%s = %s;\n", fieldName, strP);
}

void fWriteStructMemberStr(FILE *fP, char *fieldName, char *fieldData) {
  fprintf(fP, "\t.%s = %s;\n",fieldName, fieldData);
}

void fWriteStructMemberArrayStart(FILE *fP, char *fieldName) {
  fprintf(fP, "\t.%s = {", fieldName);
}

void fWriteStructMemberArrayEnd(FILE *fP) {
  fprintf(fP, "};\n");
}

void fWriteStructMemberArrayPtrElem(FILE *fP, char *fieldType, char *fieldData, U8 isLastElem) {
  if (isLastElem)
    fprintf(fP, "&%s%s", fieldData, fieldType);
  else
    fprintf(fP, "&%s%s, ", fieldData, fieldType);
}

void fWriteStructMemberArrayStrElem(FILE *fP, char *fieldData, U8 isLastElem) {
  if (isLastElem)
    fprintf(fP, "%s", fieldData);
  else
    fprintf(fP, "%s, ", fieldData);
}

void fWriteStructMemberArrayNumElem(FILE *fP, U32 num, U8 isLastElem) {
  char *fieldData = NULL;
  sprintf(fieldData, "%d", num);
  if (isLastElem)
    fprintf(fP, "%s", fieldData);
  else
    fprintf(fP, "%s, ", fieldData);
  free(fieldData);
}

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

typedef struct {
  FILE *fP;
  char *name;
  char *objType;
} File;

void fileDel(File **fPP) {
  if (fPP) {
    jbFree((void**) &(*fPP)->name);
    fclose((*fPP)->fP);
    jbFree((void**) fPP);
  }
}

Error fileNew(File **fPP, char *objNameP, char *objTypeP, char *fileExtP) {
  if (!fPP) 
    return E_BAD_ARGS;
  
  Error e = jbAlloc((void**) fPP, sizeof(File), 1);
  if (!e)
    e = fileNameNew(&(*fPP)->name, objNameP, objTypeP, fileExtP);
  if (!e)
    (*fPP)->fP = fopen((*fPP)->name, "w");
  if ((*fPP)->fP == NULL) {
    e = E_NO_MEMORY;
    fileDel(fPP);
  }
  if (!e)
    (*fPP)->objType = objTypeP;
  return e;
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
      printf("Usage: %s <soundfont> [<output>]\n", argv[0]);
      return 1;
    }

    // MB: Because of the way fluidlite is written, I can't simply access the
    //     soundfont through here. I have to write the file-writer inside the code itself.
    fluid_settings_t* settings = new_fluid_settings();
    fluid_synth_t* synthP = new_fluid_synth(settings);
    fluid_synth_sfload(synthP, argv[1], 1);

    fluid_sfont_t *sfP = fluid_synth_get_sfont(synthP, 0);
    char *fileNameP;
    File *fileP;
    fluid_preset_t *presetP;
    fluid_def_preset_t *defPresetP;
    U8 maxBankId = 0;
    U8 currIterN = 0;
    U8 nZones    = 0;
    Error e = SUCCESS;
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

    if (!e && sfP) {
      // Soundfont
      e = fileNew(&fileP, "test", "soundfontSeed", "h");
      if (!e && fileP) {
        fprintf(fileP->fP, "#include <botox/data.h>\n\n");
        fWriteStructHeader(fileP->fP, "SoundfontSeed", "WhoKNows");
        fWriteStructMemberStr(fileP->fP, "bankSeedA", "idunnoyet");
        fWriteStructFooter(fileP->fP);
      }
      fileDel(&fileP);
      // Bank
      e = fileNew(&fileP, "myBank", "bankSeed", "h");
      if (!e && fileP) {
        sfP->iteration_start(sfP);
        presetP = sfP->get_preset(sfP, 0, 0);
        while (sfP->iteration_next(sfP, presetP)) {
          defPresetP = presetP->data;
          fprintf(fileP->fP, "#include <botox/data.h>\n\n");
          fWriteStructHeader(fileP->fP, "BankSeed", "myBank");
          fWriteStructMemberNum(fileP->fP, "nPresetSeeds", maxBankId + 1);
          fWriteStructMemberArrayStart(fileP->fP, "presetSeedA");
          fWriteStructMemberArrayPtrElem(fileP->fP, "PresetSeed", defPresetP->name, ++currIterN >= maxBankId);
          fWriteStructMemberArrayEnd(fileP->fP);
          fWriteStructFooter(fileP->fP);
        }
      }
      fileDel(&fileP);
      // Presets
      /*
        typedef struct {
          U8     nZones;
          Zone **zonePA;
        } InstrumentSeed;
      */
      if (!e) {
        sfP->iteration_start(sfP);
        presetP = sfP->get_preset(sfP, 0, 0);
        fluid_preset_zone_t *pZoneP;
        FILE *fP = fileP->fP;
        while (sfP->iteration_next(sfP, presetP)) {
          e = fileNew(&fileP, defPresetP->name, "presetSeed", "h");
          fprintf(fileP->fP, "#include <botox/data.h>\n\n");
          fWriteStructHeader(fileP->fP, "PresetSeed", defPresetP->name);

          // Global zone
          defPresetP = presetP->data;
          pZoneP = defPresetP->global_zone;
          // TODO write zone-writer
          // TODO write mod-writer
          // TODO write gen-writer
          // TODO write sample-writer
          // This is getting ridiculous. Just fprintf the damn members.
          if (pZoneP) {
            fprintf(fP, "\n\t.globalZone = {");
            fprintf(fP, "\n\t\t.zoneA = {\n\t\t.nGenerators = %d;", 5);
            fprintf(fP, "\n\t\t\t.u.instP = &%s;", "idkYet");
            fprintf(fP, "\n\t\t\t.genA = &%s;", "localA");
            fprintf(fP, "\n};"};
          }

          fWriteStructMemberNum(fileP->fP, "nZones", maxBankId + 1);
          fWriteStructMemberStr(fileP->fP, "zoneA", "idunnoyet");
          fWriteStructFooter(fileP->fP);
          fileDel(&fileP);
        }
      }
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

    arrayDel((void**) &presetCountsA);
    delete_fluid_synth(synthP);
    delete_fluid_settings(settings);
}
