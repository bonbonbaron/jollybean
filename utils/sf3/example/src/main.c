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


char genNamesA[][100] = { "GEN_STARTADDROFS", "GEN_ENDADDROFS", "GEN_STARTLOOPADDROFS", "GEN_ENDLOOPADDROFS", "GEN_STARTADDRCOARSEOFS", "GEN_MODLFOTOPITCH", "GEN_VIBLFOTOPITCH", "GEN_MODENVTOPITCH", "GEN_FILTERFC", "GEN_FILTERQ", "GEN_MODLFOTOFILTERFC", "GEN_MODENVTOFILTERFC", "GEN_ENDADDRCOARSEOFS", "GEN_MODLFOTOVOL", "GEN_UNUSED1", "GEN_CHORUSSEND", "GEN_REVERBSEND", "GEN_PAN", "GEN_UNUSED2", "GEN_UNUSED3", "GEN_UNUSED4", "GEN_MODLFODELAY", "GEN_MODLFOFREQ", "GEN_VIBLFODELAY", "GEN_VIBLFOFREQ", "GEN_MODENVDELAY", "GEN_MODENVATTACK", "GEN_MODENVHOLD", "GEN_MODENVDECAY", "GEN_MODENVSUSTAIN", "GEN_MODENVRELEASE", "GEN_KEYTOMODENVHOLD", "GEN_KEYTOMODENVDECAY", "GEN_VOLENVDELAY", "GEN_VOLENVATTACK", "GEN_VOLENVHOLD", "GEN_VOLENVDECAY", "GEN_VOLENVSUSTAIN", "GEN_VOLENVRELEASE", "GEN_KEYTOVOLENVHOLD", "GEN_KEYTOVOLENVDECAY", "GEN_INSTRUMENT", "GEN_RESERVED1", "GEN_KEYRANGE", "GEN_VELRANGE", "GEN_STARTLOOPADDRCOARSEOFS", "GEN_KEYNUM", "GEN_VELOCITY", "GEN_ATTENUATION", "GEN_RESERVED2", "GEN_ENDLOOPADDRCOARSEOFS", "GEN_COARSETUNE", "GEN_FINETUNE", "GEN_SAMPLEID", "GEN_SAMPLEMODE", "GEN_RESERVED3", "GEN_SCALETUNE", "GEN_EXCLUSIVECLASS", "GEN_OVERRIDEROOTKEY", "GEN_PITCH" };

static char INCLUDE_DIRECTIVES[] = "#include \"botox/data.h\"\n#include \"../soundfont.h\"\n\n";
// Standardize strings to lessen the chance of mistyping any.
char BANK_ARR_FORMAT[]               = "sf%sBankA";  
char PRESET_ARR_FORMAT[]             = "sf%sBank%dPresetA";  

char SAMPLE_FORMAT[] = "sample_%s";
char IMOD_FORMAT[] = "i%s_z%s_g%d_modA";
char IGEN_FORMAT[] = "i%s_z%s_genA";
char IZONE_FORMAT[] = "i%s_zoneA";
char IGZONE_FORMAT[] = "i%s_globalZone";
char INST_FORMAT[] = "inst_%s";
char PMOD_FORMAT[] = "i%s_z%s_g%d_modA";
char PGEN_FORMAT[] = "i%s_z%s_genA";
char PZONE_FORMAT[] = "i%s_zoneA";
char PGZONE_FORMAT[] = "i%s_globalZone";
char PRESET_FORMAT[] = "preset_%d";
char BANK_FORMAT[] = "bank%d";
char SOUNDFONT_FORMAT[] = "soundfont%d";


// Instrument name + zone name + dest + Mod A

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

typedef struct _fluid_defsfont_t {
  char* filename;           /* the filename of this soundfont */
  unsigned int samplepos;   /* the position in the file at which the sample data starts */
  unsigned int samplesize;  /* the size of the sample data */
  short* sampledata;        /* the sample data, loaded in ram */
  fluid_list_t* sample;      /* the samples in this soundfont */
  struct _fluid_defpreset_t* preset; /* the presets of this soundfont */

  fluid_preset_t iter_preset;        /* preset interface used in the iteration */
  struct _fluid_defpreset_t* iter_cur;       /* the current preset in the iteration */
} fluid_defsfont_t;
//
//**************************
//*****  INSTRUMENTS  ******
//**************************

// Pass genIdx = 0 - 59 through this function to see which modulators a generator uses.
void writeIMod(FILE *fP, fluid_mod_t *imodP) {
  fprintf(fP, "\t{\n");
  fprintf(fP, "\t\t.src1 = %d;\n", imodP->src1);
  fprintf(fP, "\t\t.src2 = %d;\n", imodP->src2);
  fprintf(fP, "\t\t.xformType1 = %d;\n", imodP->flags1);
  fprintf(fP, "\t\t.xformType2 = %d;\n", imodP->flags2);
  fprintf(fP, "\t\t.productScale = %d;\n", (S16) imodP->amount);
  fprintf(fP, "\t},\n");
}

void writeImodArray(FILE *fP, char *iname, char *zname, U8 genId, fluid_inst_zone_t *izoneP) {
  if (izoneP->mod) {
    fprintf(fP, "Modulator "); 
    fprintf(fP, IMOD_FORMAT, iname, zname, genId);
    fprintf(fP, "[] = {\n");
    for (fluid_mod_t *imodP = izoneP->mod; imodP; imodP = imodP->next) 
      if (imodP->dest == genId)  // Only write mods that belong to this generator so it has an array of them.
        writeIMod(fP, imodP);
    fprintf(fP, "};\n\n");  
  }
}

void writeImodArraysForAllIzones(FILE *fP, fluid_inst_t *instP) {
  for (fluid_inst_zone_t *izoneP = instP->zone; izoneP; izoneP = izoneP->next) 
    // inst's izones' generators
    for (U8 genId = 0; genId < 60; ++genId) 
      if (izoneP->gen[genId].flags) 
        writeImodArray(fP, instP->name, izoneP->name, genId, izoneP);
}

void writeImodArrays(FILE *fP, fluid_defsfont_t *sfP) {
  // Dig into presets to find all the instruments. This means excluding any unused ones!
  // presets
  for (fluid_def_preset_t *dPresetP = sfP->preset; dPresetP; dPresetP = dPresetP->next) {
    // global pzone's inst
    if (dPresetP->global_zone->inst) 
      writeImodArraysForAllIzones(fP, dPresetP->global_zone->inst);
    // individual pzones' insts
    for (fluid_preset_zone_t *pzoneP = dPresetP->zone; pzoneP; pzoneP = pzoneP->next) {
      writeImodArraysForAllIzones(fP, pzoneP->inst);
    }
  }
}

// TODO make this reusable for presets too. Have a boolean to determine which one.
//      You can do this by making a union between izone and pzone.
void writeIgen(FILE *fP, char *iname, char *zname, U8 genId, U16 genAmt, U8 nMods) {
  fprintf(fP, "\t{\n");
  fprintf(fP, "\t\t.genType = %s,\n", genNamesA[genId]);
  fprintf(fP, "\t\t.nMods = %d,\n", nMods);
  fprintf(fP, "\t\t.amount = %d,\n", genAmt);
  fprintf(fP, "\t\t.modA = ");
  if (nMods) 
    fprintf(fP, IMOD_FORMAT, iname, zname, genId);
  else
    fprintf(fP, "NULL");
  fprintf(fP, "\n\t},\n");
}

U8 countIModsInGen(fluid_inst_zone_t *izoneP, U8 genId) {
  U8 nMods = 0;
  if (izoneP->mod)
    for (fluid_mod_t *modP = izoneP->mod; modP; modP = modP->next)
      nMods += (modP->dest == genId);
  return nMods;
}


void writeIgenArray(FILE *fP, char *pname, char *zname, U8 genId, fluid_inst_zone_t *izoneP) {
  fprintf(fP, "Generator "); 
  fprintf(fP, IGEN_FORMAT, pname, zname);
  fprintf(fP, "[] = {\n");
  U8 nMods = countIModsInGen(izoneP, genId);
  writeIgen(fP, pname, zname, genId, (U16) izoneP->gen[genId].val, nMods);
  fprintf(fP, "};\n\n");
}

void writeIgenArraysForAllIzones(FILE *fP, fluid_inst_t *instP) {
  for (fluid_inst_zone_t *izoneP = instP->zone; izoneP; izoneP = izoneP->next) 
    for (U8 genId = 0; genId < 60; ++genId) 
      if (izoneP->gen[genId].flags) 
        writeIgenArray(fP, instP->name, izoneP->name, genId, izoneP);
}

void writeIgenArrays(FILE *fP, fluid_defsfont_t *sfP) {
  // Dig into presets to find all the instruments. This means excluding any unused ones!
  // presets
  for (fluid_def_preset_t *dPresetP = sfP->preset; dPresetP; dPresetP = dPresetP->next) {
    // global pzone's inst
    if (dPresetP->global_zone->inst) 
      writeImodArraysForAllIzones(fP, dPresetP->global_zone->inst);
    // individual pzones' insts
    for (fluid_preset_zone_t *pzoneP = dPresetP->zone; pzoneP; pzoneP = pzoneP->next) {
      writeIgenArraysForAllIzones(fP, pzoneP->inst);
    }
  }
}

void writeIzone(FILE *fP, U8 nGens, char *iname, char *zname, char *sampleName) {
  fprintf(fP, "\t{\n");
  fprintf(fP, "\t\t.nGens = %d;\n", nGens);
  fprintf(fP, "\t\t.u.sampleP = &%s;\n", sampleName);  // TODO sample name needs format
  fprintf(fP, "\t\t.genA = ");
  if (nGens) 
    fprintf(fP, IGEN_FORMAT, iname, zname);
  else
    fprintf(fP, "NULL");
  fprintf(fP, ";\n\t},\n");
}

U8 countGensInIzone(fluid_inst_zone_t *izoneP) {
  U8 nGens = 0;
  if (izoneP)
    for (U8 genId = 0; genId < 60; ++genId)
      nGens += (izoneP->gen[genId].flags != 0);
  return nGens;
}

void writeIzoneArray(FILE *fP, fluid_inst_t *instP) {
  if (instP) {
    fprintf(fP, "Zone ");
    fprintf(fP, IZONE_FORMAT, instP->name);
    fprintf(fP, "[] = {\n");
    for (fluid_inst_zone_t *izoneP = instP->zone; izoneP; izoneP = izoneP->next) {
      U8 nGens = countGensInIzone(izoneP);
      writeIzone(fP, nGens, instP->name, izoneP->name, izoneP->sample->name);
    }
    fprintf(fP, "};\n\n");
  }
}

void writeGIzone(FILE *fP, fluid_inst_t *instP) {
  if (instP) {
    fprintf(fP, IGZONE_FORMAT, instP->name);
    U8 nGens = countGensInIzone(instP->global_zone);
    writeIzone(fP, nGens, instP->name, instP->global_zone->name, instP->global_zone->sample->name);
  }
}

void writeIzoneArrays(FILE *fP, fluid_defsfont_t *sfP) {
  for (fluid_def_preset_t *presetP = sfP->preset; presetP; presetP = presetP->next) {
    // GLobal izone
    writeGIzone(fP, presetP->global_zone->inst);
    // Normal izones
    for (fluid_preset_zone_t *pzoneP = presetP->zone; pzoneP; pzoneP = pzoneP->next)
      writeIzoneArray(fP, pzoneP->inst);
  }
}

U8 countZonesInInst(fluid_inst_t *instP) {
  U8 nZones = 0;
  for (fluid_inst_zone_t *zoneP = instP->zone; zoneP; zoneP = zoneP->next, ++nZones);
  return nZones;
}

void writeInst(FILE *fP, fluid_inst_t *instP) {
  if (instP) {
    fprintf(fP, "Instrument ");
    fprintf(fP, INST_FORMAT, instP->name);
    fprintf(fP, " = {\n");
    U8 nZones = countZonesInInst(instP);
    fprintf(fP, "\t.nZones = %d,\n", nZones);
    fprintf(fP, "\t.globalZoneP = ");
    fprintf(fP, IGZONE_FORMAT, instP->global_zone->name);
    fprintf(fP, ",\n");
    fprintf(fP, "\t.zoneA = ");
    fprintf(fP, IZONE_FORMAT, instP->name);
    fprintf(fP, "\n};\n\n");
  }
}

void writeInsts(FILE *fP, fluid_defsfont_t *dSfP) {
  for (fluid_def_preset_t *presetP = dSfP->preset; presetP; presetP = presetP->next) {
    // Global zone
    writeInst(fP, presetP->global_zone->inst);
    // Normal zones
    for (fluid_preset_zone_t *pzoneP = presetP->zone; pzoneP; pzoneP = pzoneP->next)
      writeInst(fP, pzoneP->inst);
  }
}

//*****************************
//*****  PRESETS  *************
//*****************************

// Pass genIdx = 0 - 59 through this function to see which modulators a generator uses.
void writePMod(FILE *fP, fluid_mod_t *pmodP) {
  fprintf(fP, "\t{\n");
  fprintf(fP, "\t\t.src1 = %d;\n", pmodP->src1);
  fprintf(fP, "\t\t.src2 = %d;\n", pmodP->src2);
  fprintf(fP, "\t\t.xformType1 = %d;\n", pmodP->flags1);
  fprintf(fP, "\t\t.xformType2 = %d;\n", pmodP->flags2);
  fprintf(fP, "\t\t.productScale = %d;\n", (S16) pmodP->amount);
  fprintf(fP, "\t},\n");
}

void writePmodArray(FILE *fP, char *pname, char *zname, U8 genId, fluid_preset_zone_t*pzoneP) {
  if (pzoneP->mod) {
    fprintf(fP, "Modulator "); 
    fprintf(fP, PMOD_FORMAT, pname, zname, genId);
    fprintf(fP, "[] = {\n");
    for (fluid_mod_t *pmodP = pzoneP->mod; pmodP; pmodP = pmodP->next) 
      if (pmodP->dest == genId)  // Only write mods that belong to this generator so it has an array of them.
        writeIMod(fP, pmodP);
    fprintf(fP, "};\n\n");  
  }
}

void writePmodArraysForAllPzones(FILE *fP, fluid_def_preset_t *presetP) {
  for (fluid_preset_zone_t *pzoneP = presetP->zone; pzoneP; pzoneP = pzoneP->next) 
    // inst's pzones' generators
    for (U8 genId = 0; genId < 60; ++genId) 
      if (pzoneP->gen[genId].flags) 
        writePmodArray(fP, presetP->name, pzoneP->name, genId, pzoneP);
}

void writePmodArrays(FILE *fP, fluid_defsfont_t *sfP) {
  // Dig into presets to find all the instruments. This means excluding any unused ones!
  // presets
  for (fluid_def_preset_t *dPresetP = sfP->preset; dPresetP; dPresetP = dPresetP->next) {
    // global pzone's inst
    if (dPresetP->global_zone->inst) 
      writeImodArraysForAllIzones(fP, dPresetP->global_zone->inst);
    // individual pzones' insts
    for (fluid_preset_zone_t *pzoneP = dPresetP->zone; pzoneP; pzoneP = pzoneP->next) {
      writeImodArraysForAllIzones(fP, pzoneP->inst);
    }
  }
}

// TODO make this reusable for presets too. Have a boolean to determine which one.
//      You can do this by making a union between izone and pzone.
void writePgen(FILE *fP, char *pname, char *zname, U8 genId, U16 genAmt, U8 nMods) {
  fprintf(fP, "\t{\n");
  fprintf(fP, "\t\t.genType = %d;\n", genId);
  fprintf(fP, "\t\t.nMods = %d;\n", nMods);
  fprintf(fP, "\t\t.amount = %d;\n", genAmt);
  fprintf(fP, "\t\t.modA = ");
  if (nMods) 
    fprintf(fP, PMOD_FORMAT, pname, zname, genId);
  else
    fprintf(fP, "NULL");
  fprintf(fP, ";\n\t},\n");
}

U8 countPModsInGen(fluid_preset_zone_t *pzoneP, U8 genId) {
  U8 nMods = 0;
  if (pzoneP->mod)
    for (fluid_mod_t *modP = pzoneP->mod; modP; modP = modP->next)
      nMods += (modP->dest == genId);
  return nMods;
}


void writePgenArray(FILE *fP, char *pname, char *zname, U8 genId, fluid_preset_zone_t *pzoneP) {
  fprintf(fP, "Generator "); 
  fprintf(fP, IGEN_FORMAT, pname, zname);
  fprintf(fP, "[] = {\n");
  U8 nMods = countPModsInGen(pzoneP, genId);
  writeIgen(fP, pname, zname, genId, (U16) pzoneP->gen[genId].val, nMods);
  fprintf(fP, "};\n\n");
}

void writePgenArraysForAllPzones(FILE *fP, fluid_inst_t *instP) {
  for (fluid_inst_zone_t *izoneP = instP->zone; izoneP; izoneP = izoneP->next) 
    for (U8 genId = 0; genId < 60; ++genId) 
      if (izoneP->gen[genId].flags) 
        writeIgenArray(fP, instP->name, izoneP->name, genId, izoneP);
}

void writePgenArrays(FILE *fP, fluid_defsfont_t *sfP) {
  // Dig into presets to find all the instruments. This means excluding any unused ones!
  // presets
  for (fluid_def_preset_t *dPresetP = sfP->preset; dPresetP; dPresetP = dPresetP->next) {
    // global pzone's inst
    if (dPresetP->global_zone->inst) 
      writeImodArraysForAllIzones(fP, dPresetP->global_zone->inst);
    // individual pzones' insts
    for (fluid_preset_zone_t *pzoneP = dPresetP->zone; pzoneP; pzoneP = pzoneP->next) {
      writeImodArraysForAllIzones(fP, pzoneP->inst);
    }
  }
}

void writePzone(FILE *fP, U8 nGens, char *pname, char *zname, char *iname) {
  fprintf(fP, "\t{\n");
  fprintf(fP, "\t\t.nGens = %d;\n", nGens);
  fprintf(fP, "\t\t.u.instP = &%s;\n", iname);
  fprintf(fP, "\t\t.genA = ");
  if (nGens) 
    fprintf(fP, PGEN_FORMAT, iname, zname);
  else
    fprintf(fP, "NULL");
  fprintf(fP, ";\n\t},\n");
}

U8 countGensInPzone(fluid_preset_zone_t *pzoneP) {
  U8 nGens = 0;
  for (U8 genId = 0; genId < 60; ++genId)
    nGens += (pzoneP->gen[genId].flags != 0);
  return nGens;
}

void writePzoneArray(FILE *fP, fluid_def_preset_t *presetP) {
  fprintf(fP, "Zone ");
  fprintf(fP, IZONE_FORMAT, presetP->name);
  fprintf(fP, "[] = {\n");
  for (fluid_preset_zone_t *pzoneP = presetP->zone; pzoneP; pzoneP = pzoneP->next) {
    U8 nGens = countGensInPzone(pzoneP);
    writePzone(fP, nGens, presetP->name, pzoneP->name, pzoneP->inst->name);
  }
  fprintf(fP, "};\n\n");
}

void writePzones(FILE *fP, fluid_def_preset_t *presetP) {
  // Global izone
  fprintf(fP, PGZONE_FORMAT, presetP->name);
  U8 nGens = countGensInPzone(presetP->global_zone);
  writeIzone(fP, nGens, presetP->name, presetP->global_zone->name, presetP->global_zone->inst->name);
  // Normal izones
  writePzoneArray(fP, presetP);
}

U8 countZonesInPreset(fluid_def_preset_t *presetP) {
  U8 nZones = 0;
  for (fluid_preset_zone_t *pzoneP = presetP->zone; pzoneP; pzoneP = pzoneP->next, ++nZones);
  return nZones;
}

void writePreset(FILE *fP, fluid_def_preset_t *presetP) {
  if (presetP) {
    fprintf(fP, "Preset ");
    fprintf(fP, PRESET_FORMAT, presetP->name);
    fprintf(fP, " = {\n");
    U8 nZones = countZonesInPreset(presetP);
    fprintf(fP, "\t.nZones = %d,\n", nZones);
    fprintf(fP, "\t.globalZoneP = ");
    fprintf(fP, PGZONE_FORMAT, presetP->global_zone->name);
    fprintf(fP, ",\n");
    fprintf(fP, "\t.zoneA = ");
    fprintf(fP, PZONE_FORMAT, presetP->name);
    fprintf(fP, "\n};\n\n");
  }
}

void writePresets(FILE *fP, fluid_defsfont_t *sfP) {
  for (fluid_def_preset_t *presetP = sfP->preset; presetP; presetP = presetP->next) 
    writePreset(fP, presetP);
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

#ifdef _WIN32
  char FILE_SEP = '\\';
#else
  char FILE_SEP = '/';
#endif

  if (sfP) {
    fluid_defsfont_t *dSfP = sfP->data;
    // First, get the soundfont's name.
    U8 filenameLen = strlen(argv[1]);
    // Get the sfName of the file without the extension. This'll be the soundfont's name.
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
    char *sfName;
    if (!strcmp(".sf3", fileExtP)) {  // file is sf3.
      sfName = malloc(sizeof(char) * ((basenameLastIdx - basenameFirstIdx) + 1));
      if (!sfName)
        return E_NO_MEMORY;
      char *baseCharP = sfName;
      for (U8 i = basenameFirstIdx; i < basenameLastIdx; ++i)
        *(baseCharP++) = argv[1][i];
      *baseCharP = '\0';
      printf("File is .sf3 format. Its sfName is %s.\n", sfName);
    }
    else {
      printf("File is NOT .sf3 format. It's actually %s.\n", fileExtP);
      return 0;
    }
    
    
    // Soundfont
    char soundfontFilename[500];
    sprintf(soundfontFilename, "sf%s.h", sfName);
    FILE *fP = fopen(soundfontFilename, "w");
    fprintf(fP, INCLUDE_DIRECTIVES);
    writeImodArrays(fP, dSfP);
    writeIgenArrays(fP, dSfP);
    writeIzoneArrays(fP, dSfP);
    // We'll start at the very lowest level (instruments; samples are extern) and work our way up.
  }
}
