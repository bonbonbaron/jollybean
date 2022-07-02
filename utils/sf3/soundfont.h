#include <botox/data.h>

/**
 * Generator (effect) numbers (Soundfont 2.01 specifications section 8.1.3)
 */
enum fluid_gen_type {
  GEN_STARTADDROFS,		/**< Sample start address offset (0-32767) */
  GEN_ENDADDROFS,		/**< Sample end address offset (-32767-0) */
  GEN_STARTLOOPADDROFS,		/**< Sample loop start address offset (-32767-32767) */
  GEN_ENDLOOPADDROFS,		/**< Sample loop end address offset (-32767-32767) */
  GEN_STARTADDRCOARSEOFS,	/**< Sample start address coarse offset (X 32768) */
  GEN_MODLFOTOPITCH,		/**< Modulation LFO to pitch */
  GEN_VIBLFOTOPITCH,		/**< Vibrato LFO to pitch */
  GEN_MODENVTOPITCH,		/**< Modulation envelope to pitch */
  GEN_FILTERFC,			/**< Filter cutoff */
  GEN_FILTERQ,			/**< Filter Q */
  GEN_MODLFOTOFILTERFC,		/**< Modulation LFO to filter cutoff */
  GEN_MODENVTOFILTERFC,		/**< Modulation envelope to filter cutoff */
  GEN_ENDADDRCOARSEOFS,		/**< Sample end address coarse offset (X 32768) */
  GEN_MODLFOTOVOL,		/**< Modulation LFO to volume */
  GEN_UNUSED1,			/**< Unused */
  GEN_CHORUSSEND,		/**< Chorus send amount */
  GEN_REVERBSEND,		/**< Reverb send amount */
  GEN_PAN,			/**< Stereo panning */
  GEN_UNUSED2,			/**< Unused */
  GEN_UNUSED3,			/**< Unused */
  GEN_UNUSED4,			/**< Unused */
  GEN_MODLFODELAY,		/**< Modulation LFO delay */
  GEN_MODLFOFREQ,		/**< Modulation LFO frequency */
  GEN_VIBLFODELAY,		/**< Vibrato LFO delay */
  GEN_VIBLFOFREQ,		/**< Vibrato LFO frequency */
  GEN_MODENVDELAY,		/**< Modulation envelope delay */
  GEN_MODENVATTACK,		/**< Modulation envelope attack */
  GEN_MODENVHOLD,		/**< Modulation envelope hold */
  GEN_MODENVDECAY,		/**< Modulation envelope decay */
  GEN_MODENVSUSTAIN,		/**< Modulation envelope sustain */
  GEN_MODENVRELEASE,		/**< Modulation envelope release */
  GEN_KEYTOMODENVHOLD,		/**< Key to modulation envelope hold */
  GEN_KEYTOMODENVDECAY,		/**< Key to modulation envelope decay */
  GEN_VOLENVDELAY,		/**< Volume envelope delay */
  GEN_VOLENVATTACK,		/**< Volume envelope attack */
  GEN_VOLENVHOLD,		/**< Volume envelope hold */
  GEN_VOLENVDECAY,		/**< Volume envelope decay */
  GEN_VOLENVSUSTAIN,		/**< Volume envelope sustain */
  GEN_VOLENVRELEASE,		/**< Volume envelope release */
  GEN_KEYTOVOLENVHOLD,		/**< Key to volume envelope hold */
  GEN_KEYTOVOLENVDECAY,		/**< Key to volume envelope decay */
  GEN_INSTRUMENT,		/**< Instrument ID (shouldn't be set by user) */
  GEN_RESERVED1,		/**< Reserved */
  GEN_KEYRANGE,			/**< MIDI note range */
  GEN_VELRANGE,			/**< MIDI velocity range */
  GEN_STARTLOOPADDRCOARSEOFS,	/**< Sample start loop address coarse offset (X 32768) */
  GEN_KEYNUM,			/**< Fixed MIDI note number */
  GEN_VELOCITY,			/**< Fixed MIDI velocity value */
  GEN_ATTENUATION,		/**< Initial volume attenuation */
  GEN_RESERVED2,		/**< Reserved */
  GEN_ENDLOOPADDRCOARSEOFS,	/**< Sample end loop address coarse offset (X 32768) */
  GEN_COARSETUNE,		/**< Coarse tuning */
  GEN_FINETUNE,			/**< Fine tuning */
  GEN_SAMPLEID,			/**< Sample ID (shouldn't be set by user) */
  GEN_SAMPLEMODE,		/**< Sample mode flags */
  GEN_RESERVED3,		/**< Reserved */
  GEN_SCALETUNE,		/**< Scale tuning */
  GEN_EXCLUSIVECLASS,		/**< Exclusive class number */
  GEN_OVERRIDEROOTKEY,		/**< Sample root note override */

  /* the initial pitch is not a "standard" generator. It is not
   * mentioned in the list of generator in the SF2 specifications. It
   * is used, however, as the destination for the default pitch wheel
   * modulator. */
  GEN_PITCH,			/**< Pitch (NOTE: Not a real SoundFont generator) */
  GEN_LAST			/**< Value defines the count of generators (#fluid_gen_type) */
};



typedef struct {
  U8   origPitch;
  U8   origPitchAdj;
  U32  startIdx;      // I've yet to see a good reason for this if we have a pointer already.
  U32  endIdx;
  U32  loopStartIdx;
  U32  loopEndIdx;
  S16 *pcmDataP;      // Split pcm data apart to avoid duplicates.
} Sample;      // 20 bytes

// Modulator adjusts any given generator. Don't need to tell it which generator since the gen in question owns it.
typedef struct {	
  U8  src1;		
  U8  src2;   // This is kind of the second controller that controls the degree of the first.
  U8  xformType1;	
  U8  xformType2;	
  S16 productScale;	  // Scales the product of src1 transformed and src2 transformed.
} Modulator;   // 6 bytes 

typedef union {
  S16 s16;			
  U16 u16;		
  struct {
    unsigned char u8Lo;			
    unsigned char u8Hi;			
  } range;
} GenAmount;  // 2 bytes

// Generator is a component of a sound; e.g. vibrato LFO, 
// key/velocity range, filter cutoff/Q, source sample, etc.
typedef struct {
  U8 genType;	
  U8 nMods;
  GenAmount amount;	
  Modulator *modA;  // I get the mod array appeal, but data accessed together should stay together.
} Generator;   // 8 bytes

struct _Zone;

typedef struct {
  U8 nZones;  // only pertains to individual zones. Global zone is just tested for NULL.
  struct _Zone *globalZoneP;
  struct _Zone *zoneA;
} Instrument;  // 12 bytes

typedef Instrument Preset;  // Both have the same structure; so 4 bytes again!

// Zone contains all gens for an instrument or preset.
// Union has inst/sample because presets are made of 
// instruments, and instruments are made of samples.
typedef struct _Zone {
  U8           nGens;
  union {   // Finally found a good use case for unions: avoiding void* while restricting purpose.
    Sample     *sampleP;
    Instrument *instP;
  }             u;
  Generator    *genA;
} Zone;  // 12 bytes

typedef struct {
  U8 nPresets;
  Preset *presetA;
} Bank;  // 8 bytes

typedef struct {
  U8 nBanks;
  Bank *bankA;
} Soundfont;  // 8 bytes
