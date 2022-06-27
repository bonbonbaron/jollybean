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

char genNamesA[][100] = {
        "GEN_STARTADDROFS",         
        "GEN_ENDADDROFS",           
        "GEN_STARTLOOPADDROFS",     
        "GEN_ENDLOOPADDROFS",       
        "GEN_STARTADDRCOARSEOFS",   
        "GEN_MODLFOTOPITCH",        
        "GEN_VIBLFOTOPITCH",        
        "GEN_MODENVTOPITCH",        
        "GEN_FILTERFC",             
        "GEN_FILTERQ",              
        "GEN_MODLFOTOFILTERFC",     
        "GEN_MODENVTOFILTERFC",     
        "GEN_ENDADDRCOARSEOFS",     
        "GEN_MODLFOTOVOL",          
        "GEN_UNUSED1",              
        "GEN_CHORUSSEND",           
        "GEN_REVERBSEND",           
        "GEN_PAN",                  
        "GEN_UNUSED2",              
        "GEN_UNUSED3",              
        "GEN_UNUSED4",              
        "GEN_MODLFODELAY",          
        "GEN_MODLFOFREQ",           
        "GEN_VIBLFODELAY",          
        "GEN_VIBLFOFREQ",           
        "GEN_MODENVDELAY",          
        "GEN_MODENVATTACK",         
        "GEN_MODENVHOLD",           
        "GEN_MODENVDECAY",          
        "GEN_MODENVSUSTAIN",        
        "GEN_MODENVRELEASE",        
        "GEN_KEYTOMODENVHOLD",      
        "GEN_KEYTOMODENVDECAY",     
        "GEN_VOLENVDELAY",          
        "GEN_VOLENVATTACK",         
        "GEN_VOLENVHOLD",           
        "GEN_VOLENVDECAY",          
        "GEN_VOLENVSUSTAIN",        
        "GEN_VOLENVRELEASE",        
        "GEN_KEYTOVOLENVHOLD",      
        "GEN_KEYTOVOLENVDECAY",     
        "GEN_INSTRUMENT",           
        "GEN_RESERVED1",            
        "GEN_KEYRANGE",             
        "GEN_VELRANGE",             
        "GEN_STARTLOOPADDRCOARSEOFS",
        "GEN_KEYNUM",               
        "GEN_VELOCITY",             
        "GEN_ATTENUATION",          
        "GEN_RESERVED2",            
        "GEN_ENDLOOPADDRCOARSEOFS", 
        "GEN_COARSETUNE",           
        "GEN_FINETUNE",             
        "GEN_SAMPLEID",             
        "GEN_SAMPLEMODE",           
        "GEN_RESERVED3",            
        "GEN_SCALETUNE",            
        "GEN_EXCLUSIVECLASS",       
        "GEN_OVERRIDEROOTKEY",      
        "GEN_PITCH",                
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


static U32 instAddrA[65536] = {0};

S32 getAddrIdx(U32 addr, U32 *addrA, U32 nElems) {
  U32 *currAddrP = addrA;
  U32 *addrEndP = addrA + nElems;

  // If addr's in there, get its index.
  for (; currAddrP < addrEndP && *currAddrP; ++currAddrP) 
    if (*currAddrP == addr)
      return (S32) (((U32) currAddrP - (U32) addrA) / sizeof(U32)); 
  
  // If addr's not in there, just add it and get its new index.
  if (currAddrP < addrEndP) {
    *currAddrP = addr;
    return (S32) (((U32) currAddrP - (U32) addrA) / sizeof(U32)); 
  }

  // Not found.
  return -1;
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

    char INCLUDE_DIRECTIVES[] = "#include \"botox/data.h\"\n#include \"../soundfont.h\"\n\n";

    if (!e && sfP) {
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
      
      
      // Standardize strings to lessen the chance of mistyping any.
      char BANK_ARR_FORMAT[]         = "sf%sBankA";  // %s = sfName
      char PRESET_ARR_FORMAT[]       = "sf%sBank%dPresetA";  // %s = sfName, %d bank number
      char GLOBAL_IMOD_ARR_FORMAT[]  = "sf%sBank%dPreset%dGlobalIZoneModA";  // sfont name, bank num, preset num
      char GLOBAL_IGEN_ARR_FORMAT[]  = "sf%sBank%dPreset%dGlobalIZoneGenA";  // sfont name, bank num, preset num
      char GLOBAL_PMOD_ARR_FORMAT[]  = "sf%sBank%dPreset%dGlobalPZoneModA";  // sfont name, bank num, preset num
      char GLOBAL_PGEN_ARR_FORMAT[]  = "sf%sBank%dPreset%dGlobalPZoneGenA";  // sfont name, bank num, preset num
      char IMOD_ARR_FORMAT[]         = "sf%sBank%dPreset%dInst%dIZone%dIModA";  // sfont name, bank num, preset num, zone num
      char IGEN_ARR_FORMAT[]         = "sf%sBank%dPreset%dInst%dIZone%dIGenA";  // sf, bank, preset, inst, izone
      char IZONE_ARR_FORMAT[]        = "sf%sBank%dPreset%dInst%dIZoneA";  // sfont name, bank num, preset num
      char PMOD_ARR_FORMAT[]         = "sf%sBank%dPreset%dPZone%dPModA";  // sfont name, bank num, preset num, zone num
      char PGEN_ARR_FORMAT[]         = "sf%sBank%dPreset%dPZone%dPGenA";  // sfont name, bank num, preset num, zone num, gennum
      char PZONE_ARR_FORMAT[]        = "sf%sBank%dPreset%dPZoneA";  // sfont name, bank num, preset num
      char INST_FORMAT[]             = "inst%s";  // Instruments are reusable; the zones that specify their attributes aren't.

      // Soundfont
      char soundfontFilename[500];
      sprintf(soundfontFilename, "sf%s.h", sfName);
      FILE *sfFP = fopen(soundfontFilename, "w");
      if (!e && sfFP) {
        fprintf(sfFP, INCLUDE_DIRECTIVES);
        U8 nPresets = 0;
        U8 currBankIdx = 0;
        U8 currPresetIdx = 0;
        // Banks
          if (!e && sfFP) {
            sfP->iteration_start(sfP);
            presetP = sfP->get_preset(sfP, 0, 0);
            // Count the number of presets in this bank.
            defPresetP = presetP->data;
          
            for (fluid_def_preset_t *currPresetP = defPresetP; currPresetP; currPresetP = currPresetP->next) 
              if (currPresetP->bank == currBankIdx)
                ++nPresets;
            // For each preset in the current bank...
            fluid_preset_zone_t *pZoneP;
            fluid_inst_zone_t *iZoneP;
            // So tell each zone whether it has a modulator array (we'll know what the name is thanks to our string constants)
            // with an array of booleans (U8).
            U8 zoneIGenNumModsA[256][64] = {0};
            U8 zonePGenNumModsA[256][64] = {0};

            presetP = sfP->get_preset(sfP, 0, 0);
            while (sfP->iteration_next(sfP, presetP)) {
              fluid_def_preset_t *currPresetP = presetP->data;
              if (defPresetP->bank == currBankIdx) {
                // Instruments
                // Global zone
                fluid_inst_zone_t *globalIZoneP = defPresetP->global_zone->inst->global_zone;
                // Global zone's modulators
                // Modulators are read one by one in a whole array and tested for amounts before processing. 
                U8 imodCounter = 0;
                U8 globalIZoneNMods = 0;
                // Each preset can have up to 256 zones, and each zone has 60 generators.
                if (globalIZoneP->mod) {
                  fprintf(sfFP, "Modulator ");
                  fprintf(sfFP, GLOBAL_IMOD_ARR_FORMAT, sfName, currBankIdx, currPresetIdx);
                  fprintf(sfFP, "= {\n");
                  for (fluid_mod_t *modP = globalIZoneP->mod; modP; modP = modP->next) {
                    globalIZoneNMods++;
                    fprintf(sfFP, "\t{\n");
                    fprintf(sfFP, "\t\t.src1 = %d;\n", modP->src1);
                    fprintf(sfFP, "\t\t.src2 = %d;\n", modP->src2);
                    fprintf(sfFP, "\t\t.xformType1 = %d;\n", modP->flags1);
                    fprintf(sfFP, "\t\t.xformType2 = %d;\n", modP->flags2);
                    fprintf(sfFP, "\t\t.productScale = %d;\n", (S16) modP->amount);
                    fprintf(sfFP, "\t},\n");
                  }
                  fprintf(sfFP, "};");  // end global modulator array
                }
                // Individual zones' generators
                // Loop through all zone's modulators can determine which generators own any modulators. 
                // Fluidsynth does this a little backwards.
                U8 currIZoneIdx = 0;
#define MAX_N_ZONES 256
#define MAX_N_GENS   60
                // Keeps track of whether a zone's gen has any modulators.
                for (pZoneP = defPresetP->zone->inst->zone; pZoneP; pZoneP = pZoneP->next, ++currIZoneIdx) 
                  for (fluid_mod_t *modP = pZoneP->mod; modP; modP = modP->next) 
                    ++zoneIGenNumModsA[currIZoneIdx][modP->dest];  // avoids setting this zone's modA to NULL
                // Now that we know which generators have modulators, we can make a mod array for each generator
                // Iterate through all modulators. Determine which gens have any modulators too.
                // before writing the gens.
                currIZoneIdx = 0;
                U8 izoneIdx = 0;
                U8 currGenIdx= 0;
                // Write a modulator array for each generator that has any modulators. What a horrible algo!! LOL
                for (pZoneP = defPresetP->zone->inst->zone; pZoneP; pZoneP = pZoneP->next, ++currIZoneIdx) {
                  for (U8 igenIdx = 0; igenIdx < 60; ++igenIdx) {
                    if (zoneIGenNumModsA[currIZoneIdx][igenIdx]) {
                      // Start the array for this generator's modulators.
                      fprintf(sfFP, "Modulator ");
                      fprintf(sfFP, IMOD_ARR_FORMAT, sfName, currBankIdx, currPresetIdx, izoneIdx++);
                      fprintf(sfFP, "[] = {\n");
                      for (fluid_mod_t *modP = pZoneP->mod; modP; modP = modP->next) {
                        if (modP->dest == igenIdx) {
                          fprintf(sfFP, "\t{\n");
                          fprintf(sfFP, "\t\t.src1 = %d;\n", modP->src1);
                          fprintf(sfFP, "\t\t.src2 = %d;\n", modP->src2);
                          fprintf(sfFP, "\t\t.xformType1 = %d;\n", modP->flags1);
                          fprintf(sfFP, "\t\t.xformType2 = %d;\n", modP->flags2);
                          fprintf(sfFP, "\t\t.productScale = %d;\n", (S16) modP->amount);
                          fprintf(sfFP, "\t},\n");
                        }  // if this modulator belongs to the current generator
                      }  // for each modulator in this zone
                      fprintf(sfFP, "};\n\n");  // Close this generator's modulator array.
                    }  // if gen has any mods
                  }  // for each of this zone's 60 generators
                }  // for each zone in this preset
                // Generator Arrays for global zone
                globalIZoneP = defPresetP->global_zone;
                // Count number of generators in the global zone.
                U8 nGlobalZoneGens = 0;
                currIZoneIdx = 0;
                for (iZoneP = defPresetP->zone->inst->zone; pZoneP; pZoneP = pZoneP->next, ++currIZoneIdx) 
                  for (U8 genIdx = 0; genIdx < 60; ++genIdx)
                    if (iZoneP->gen[genIdx].flags)   // if this generator exists (TODO: determine how to tell if it exists)
                      ++nGlobalZoneGens;

                // Write the global zone generators.
                U8 nGlobalGens = 0;
                if (nGlobalZoneGens) {
                  fprintf(sfFP, "Generator ");
                  fprintf(sfFP, GLOBAL_IGEN_ARR_FORMAT, sfName, currBankIdx, currPresetIdx);
                  fprintf(sfFP, "[] = {\n");
                  for (U8 genIdx = 0; genIdx < 60; ++genIdx) {
                    if (globalIZoneP->gen[genIdx].flags) {  // if this generator exists (TODO: determine how to tell if it exists)
                      ++nGlobalGens;
                      fprintf(sfFP, "\t.genType = %s;\n", genNamesA[genIdx]);
                      fprintf(sfFP, "\t.nMods = %d;\n", zoneIGenNumModsA[currIZoneIdx][genIdx]);
                      fprintf(sfFP, "\t.amount = %d;\n", (S16) globalIZoneP->gen[genIdx].val);
                    }  // if this zone is being used...
                  }  // for each generator of the global zone...
                  fprintf(sfFP, "};\n\n");  // Close the current generator structure.
                }
                // Generator Arrays for individual zones
                // Count number of generators per zone.
                U8 nZoneGensA[256] = {0};
                currIZoneIdx = 0;
                for (pZoneP = defPresetP->zone; pZoneP; pZoneP = pZoneP->next, ++currIZoneIdx) 
                  for (U8 genIdx = 0; genIdx < 60; ++genIdx)
                    if (pZoneP->gen[genIdx].flags)   // if this generator exists (TODO: determine how to tell if it exists)
                      ++nZoneGensA[currIZoneIdx];

                // Write the individual zone generators.
                currIZoneIdx = 0;
                for (pZoneP = defPresetP->zone; pZoneP; pZoneP = pZoneP->next, ++currIZoneIdx) {
                  if (nZoneGensA[currIZoneIdx]) {
                    fprintf(sfFP, "Generator ");
                    fprintf(sfFP, GEN_ARR_FORMAT, sfName, currBankIdx, currPresetIdx, currIZoneIdx);
                    fprintf(sfFP, "[] = {\n");
                    for (U8 genIdx = 0; genIdx < 60; ++genIdx) {
                      if (pZoneP->gen[genIdx].flags) {  // if this generator exists (TODO: determine how to tell if it exists)
                        fprintf(sfFP, "\t.genType = %s;\n", genNamesA[genIdx]);
                        fprintf(sfFP, "\t.nMods = %d;\n", zoneGenNumModsA[currZoneIdx][genIdx]);
                        fprintf(sfFP, "\t.amount = %d;\n", (S16) pZoneP->gen[genIdx].val);
                      }  // if this zone is being used...
                    }  // for each generator of the current zone...
                    fprintf(sfFP, "};\n\n");  // Close the current generator structure.
                  }
                }  // for each zone
                // Global zone
                if (nGlobalZoneGens || defPresetP->global_zone->inst) {
                  presetGlobalZoneP = defPresetP->global_zone;
                  fprintf(sfFP, "Zone ");
                  //fprintf(sfFP, GLOBAL_PZONE_FORMAT, sfName, currBankIdx, currPresetIdx);
                }
                
                // Preset Zones 
                currZoneIdx = 0;
                fprintf(sfFP, "Zone ");
                fprintf(sfFP, PZONE_ARR_FORMAT, sfName, currBankIdx, currPresetIdx);
                fprintf(sfFP, "[] = {\n");
                for (pZoneP = defPresetP->zone; pZoneP; pZoneP = pZoneP->next, ++currZoneIdx) {
                  if (nZoneGensA[currZoneIdx] || pZoneP->inst) {
                    fprintf(sfFP, "\t{\n");
                    fprintf(sfFP, "\t\t.nGens = %d;\n", nZoneGensA[currZoneIdx]);
                    fprintf(sfFP, "\t\t.u.instP = &");
                    fprintf(sfFP, INST_FORMAT, pZoneP->inst->name);
                    fprintf(sfFP, ";\n");
                    fprintf(sfFP, "\t\t.genA = ");
                    if (nZoneGensA[currZoneIdx]) {
                      fprintf(sfFP, GEN_ARR_FORMAT, sfName, currBankIdx, currPresetIdx);
                    }
                    else
                      fprintf(sfFP, "NULL;");
                    if (currZoneIdx <= nZones)
                      fprintf(sfFP, "\n\t}\n");
                    else
                      fprintf(sfFP, ",\n");
                  }
                }
                fprintf(sfFP, "};\n\n");  // close the array of individual zones

            presetP = sfP->get_preset(sfP, 0, 0);
            // Instruments
            while (sfP->iteration_next(sfP, presetP)) {
              fluid_def_preset_t *currPresetP = presetP->data;
                // Global zone
                fluid_preset_zone_t *presetGlobalZoneP = defPresetP->global_zone;
                // Global zone's modulators
                // Modulators are read one by one in a whole array and tested for amounts before processing. 
                U8 pmodCounter = 0;
                U8 globalZoneNMods = 0;
                // Each preset can have up to 256 zones, and each zone has 60 generators.
                if (presetGlobalZoneP->mod) {
                  fprintf(sfFP, "Modulator ");
                  fprintf(sfFP, GLOBAL_MOD_ARR_FORMAT, sfName, currBankIdx, currPresetIdx);
                  fprintf(sfFP, "= {\n");
                  for (fluid_mod_t *modP = presetGlobalZoneP->mod; modP; modP = modP->next) {
                    globalZoneNMods++;
                    fprintf(sfFP, "\t{\n");
                    fprintf(sfFP, "\t\t.src1 = %d;\n", modP->src1);
                    fprintf(sfFP, "\t\t.src2 = %d;\n", modP->src2);
                    fprintf(sfFP, "\t\t.xformType1 = %d;\n", modP->flags1);
                    fprintf(sfFP, "\t\t.xformType2 = %d;\n", modP->flags2);
                    fprintf(sfFP, "\t\t.productScale = %d;\n", (S16) modP->amount);
                    fprintf(sfFP, "\t},\n");
                  }
                  fprintf(sfFP, "};");  // end global modulator array
                }
                // Individual zones' generators
                // Loop through all zone's modulators can determine which generators own any modulators. 
                // Fluidsynth does this a little backwards.
                U8 currZoneIdx = 0;
#define MAX_N_ZONES 256
#define MAX_N_GENS   60
                // Keeps track of whether a zone's gen has any modulators.
                for (pZoneP = defPresetP->zone; pZoneP; pZoneP = pZoneP->next, ++currZoneIdx) 
                  for (fluid_mod_t *modP = pZoneP->mod; modP; modP = modP->next) 
                    ++zoneGenNumModsA[currZoneIdx][modP->dest];  // avoids setting this zone's modA to NULL
                // Now that we know which generators have modulators, we can make a mod array for each generator
                // Iterate through all modulators. Determine which gens have any modulators too.
                // before writing the gens.
                currZoneIdx = 0;
                U8 zoneIdx = 0;
                U8 currGenIdx= 0;
                // Write a modulator array for each generator that has any modulators. What a horrible algo!! LOL
                for (pZoneP = defPresetP->zone; pZoneP; pZoneP = pZoneP->next, ++currZoneIdx) {
                  for (U8 genIdx = 0; genIdx < 60; ++genIdx) {
                    if (zoneGenNumModsA[currZoneIdx][genIdx]) {
                      // Start the array for this generator's modulators.
                      fprintf(sfFP, "Modulator ");
                      fprintf(sfFP, MOD_ARR_FORMAT, sfName, currBankIdx, currPresetIdx, zoneIdx++);
                      fprintf(sfFP, "[] = {\n");
                      for (fluid_mod_t *modP = pZoneP->mod; modP; modP = modP->next) {
                        if (modP->dest == genIdx) {
                          fprintf(sfFP, "\t{\n");
                          fprintf(sfFP, "\t\t.src1 = %d;\n", modP->src1);
                          fprintf(sfFP, "\t\t.src2 = %d;\n", modP->src2);
                          fprintf(sfFP, "\t\t.xformType1 = %d;\n", modP->flags1);
                          fprintf(sfFP, "\t\t.xformType2 = %d;\n", modP->flags2);
                          fprintf(sfFP, "\t\t.productScale = %d;\n", (S16) modP->amount);
                          fprintf(sfFP, "\t},\n");
                        }  // if this modulator belongs to the current generator
                      }  // for each modulator in this zone
                      fprintf(sfFP, "};\n\n");  // Close this generator's modulator array.
                    }  // if gen has any mods
                  }  // for each of this zone's 60 generators
                }  // for each zone in this preset
                // Generator Arrays for global zone
                presetGlobalZoneP = defPresetP->global_zone;
                // Count number of generators in the global zone.
                U8 nGlobalZoneGens = 0;
                currZoneIdx = 0;
                for (pZoneP = defPresetP->zone; pZoneP; pZoneP = pZoneP->next, ++currZoneIdx) 
                  for (U8 genIdx = 0; genIdx < 60; ++genIdx)
                    if (pZoneP->gen[genIdx].flags)   // if this generator exists (TODO: determine how to tell if it exists)
                      ++nGlobalZoneGens;

                // Write the global zone generators.
                U8 nGlobalGens = 0;
                if (nGlobalZoneGens) {
                  fprintf(sfFP, "Generator ");
                  fprintf(sfFP, GLOBAL_GEN_ARR_FORMAT, sfName, currBankIdx, currPresetIdx);
                  fprintf(sfFP, "[] = {\n");
                  for (U8 genIdx = 0; genIdx < 60; ++genIdx) {
                    if (presetGlobalZoneP->gen[genIdx].flags) {  // if this generator exists (TODO: determine how to tell if it exists)
                      ++nGlobalGens;
                      fprintf(sfFP, "\t.genType = %s;\n", genNamesA[genIdx]);
                      fprintf(sfFP, "\t.nMods = %d;\n", zoneGenNumModsA[currZoneIdx][genIdx]);
                      fprintf(sfFP, "\t.amount = %d;\n", (S16) presetGlobalZoneP->gen[genIdx].val);
                    }  // if this zone is being used...
                  }  // for each generator of the global zone...
                  fprintf(sfFP, "};\n\n");  // Close the current generator structure.
                }
                // Generator Arrays for individual zones
                // Count number of generators per zone.
                U8 nZoneGensA[256] = {0};
                currZoneIdx = 0;
                for (pZoneP = defPresetP->zone; pZoneP; pZoneP = pZoneP->next, ++currZoneIdx) 
                  for (U8 genIdx = 0; genIdx < 60; ++genIdx)
                    if (pZoneP->gen[genIdx].flags)   // if this generator exists (TODO: determine how to tell if it exists)
                      ++nZoneGensA[currZoneIdx];

                // Write the individual zone generators.
                currZoneIdx = 0;
                for (pZoneP = defPresetP->zone; pZoneP; pZoneP = pZoneP->next, ++currZoneIdx) {
                  if (nZoneGensA[currZoneIdx]) {
                    fprintf(sfFP, "Generator ");
                    fprintf(sfFP, GEN_ARR_FORMAT, sfName, currBankIdx, currPresetIdx, currZoneIdx);
                    fprintf(sfFP, "[] = {\n");
                    for (U8 genIdx = 0; genIdx < 60; ++genIdx) {
                      if (pZoneP->gen[genIdx].flags) {  // if this generator exists (TODO: determine how to tell if it exists)
                        fprintf(sfFP, "\t.genType = %s;\n", genNamesA[genIdx]);
                        fprintf(sfFP, "\t.nMods = %d;\n", zoneGenNumModsA[currZoneIdx][genIdx]);
                        fprintf(sfFP, "\t.amount = %d;\n", (S16) pZoneP->gen[genIdx].val);
                      }  // if this zone is being used...
                    }  // for each generator of the current zone...
                    fprintf(sfFP, "};\n\n");  // Close the current generator structure.
                  }
                }  // for each zone
                // Global zone
                if (nGlobalZoneGens || defPresetP->global_zone->inst) {
                  presetGlobalZoneP = defPresetP->global_zone;
                  fprintf(sfFP, "Zone ");
                  //fprintf(sfFP, GLOBAL_PZONE_FORMAT, sfName, currBankIdx, currPresetIdx);
                }
                
                // Preset Zones 
                currZoneIdx = 0;
                fprintf(sfFP, "Zone ");
                fprintf(sfFP, PZONE_ARR_FORMAT, sfName, currBankIdx, currPresetIdx);
                fprintf(sfFP, "[] = {\n");
                for (pZoneP = defPresetP->zone; pZoneP; pZoneP = pZoneP->next, ++currZoneIdx) {
                  if (nZoneGensA[currZoneIdx] || pZoneP->inst) {
                    fprintf(sfFP, "\t{\n");
                    fprintf(sfFP, "\t\t.nGens = %d;\n", nZoneGensA[currZoneIdx]);
                    fprintf(sfFP, "\t\t.u.instP = &");
                    fprintf(sfFP, INST_FORMAT, pZoneP->inst->name);
                    fprintf(sfFP, ";\n");
                    fprintf(sfFP, "\t\t.genA = ");
                    if (nZoneGensA[currZoneIdx]) {
                      fprintf(sfFP, GEN_ARR_FORMAT, sfName, currBankIdx, currPresetIdx);
                    }
                    else
                      fprintf(sfFP, "NULL;");
                    if (currZoneIdx <= nZones)
                      fprintf(sfFP, "\n\t}\n");
                    else
                      fprintf(sfFP, ",\n");
                  }
                }
                fprintf(sfFP, "};\n\n");  // close the array of individual zones

                // Preset
                // Write bank's array of preset seeds.
                U8 currPresetIdx = 0;
                fprintf(sfFP, "Preset ");
                fprintf(sfFP, PRESET_ARR_FORMAT, sfName, currBankIdx);
                fprintf(sfFP, "[] = {\n");
                // Write each preset element of bank's preset seed array.
                for (fluid_def_preset_t *currPresetP = defPresetP; currPresetP; currPresetP = currPresetP->next) {
                  if (currPresetP->bank == currBankIdx) { // if this preset belongs to this bank...
                    fprintf(sfFP, "\t{\n");
                    fprintf(sfFP, "\t\t.nZones = %d;\n", nZones);
                    fprintf(sfFP, "\t\t.globalZoneP = ");
                    if (defPresetP->global_zone->inst || nGlobalZoneGens) {
                      fprintf(sfFP, " = {\n");
                      fprintf(sfFP, "\t\t\t.nGens = %d;\n", nGlobalGens);
                      fprintf(sfFP, "\t\t\t.u.instP = &");
                      fprintf(sfFP, INST_FORMAT, presetGlobalZoneP->inst->name);
                      fprintf(sfFP, ";\n");
                      fprintf(sfFP, "\t\t.genA = ");
                      if (nGlobalGens) {
                        fprintf(sfFP, GLOBAL_GEN_ARR_FORMAT, sfName, currBankIdx, currPresetIdx);
                        fprintf(sfFP, "\n\t}\n");
                      }
                      else 
                        fprintf(sfFP, "NULL;\n\t};\n");
                      //fprintf(sfFP, GLOBAL_PZONE_FORMAT, sfName, currBankIdx, currPresetIdx);
                      fprintf(sfFP, ";\n");
                    }
                    else 
                      fprintf(sfFP, " = NULL;\n");
                    
                    fprintf(sfFP, "\t\t.zoneA = ");
                    fprintf(sfFP, PZONE_ARR_FORMAT, sfName, currBankIdx, currPresetIdx);
                    fprintf(sfFP, ";\n\t}\n");
                    if (++currPresetIdx == nPresets) 
                      break;
                    fprintf(sfFP, ", ");
                  }
                }
                fprintf(sfFP, "};\n\n");  // end current bank's preset seed array

              }  // if this preset belongs to the current bank
              ++currPresetIdx;
            }  // for each preset in the soundfont
          }  // if bank file opened successfully 
          // Write the bank seed pointer array first.
          currPresetIdx = 0;
          fprintf(sfFP, "Bank ");
          fprintf(sfFP, BANK_ARR_FORMAT, sfName);
          fprintf(sfFP, "[] = {\n");
        }  // for each bank in this soundfont
        // Write the soundfont seed.
        fprintf(sfFP, "Soundfont %sSoundfont = {\n", sfName);
        fprintf(sfFP, "\t.nBanks = %d;\n", maxBankId + 1);
        fprintf(sfFP, "\t.bankA  = ");
        fprintf(sfFP, BANK_ARR_FORMAT, sfName, currBankIdx);
        fprintf(sfFP, ";\n");
        fprintf(sfFP, "};\n\n");
        fclose(sfFP);
    }
    delete_fluid_synth(synthP);
    delete_fluid_settings(settings);
}
