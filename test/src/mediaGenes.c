#include "xMedia.h"
#include "mediaSysGenes.h"
include "gene/Image.h"

extern Image redImg;
extern Image blehImg;
// Bundle above gene bodies into genes
///typedef struct MediaImageGene {
  ///GeneHdr hdr;
  ///Image* imgP;
///} MediaImageGene;

MediaImageGene
med1Gene = {
  .hdr = {
    .class = IMMUTABLE,
    .u.type = MEDIA_SYS_ID
#ifndef NEDBUG
      ,
    .size = sizeof(MediaImmutableInt),
    .typeName = "MediaImmutableInt"
#endif
  },
  .imgP = &redImg,
},
  med2Gene = {
    .hdr = {
      .class = IMMUTABLE,
      .u.type = MEDIA_SYS_ID
#ifndef NEDBUG
        ,
      .size = sizeof(MediaImmutableInt),
      .typeName = "MediaImmutableInt"
#endif
    },
    .imgP = &blehImg,
  };
