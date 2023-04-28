#include <stdio.h>

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;

int main(int argc, char **argv) {

#ifdef __ARM_NEON__
  U32 inputA[8] = {0x12340000, 0x56780000, 0x9abc0000, 0xdeaf0000, 
                0x43210000, 0x87650000, 0xcba90000, 0xfaed0000};
  U32 resultA[8] = {0};
  U32 *inputP = inputA;
  U32 *resultP = resultA;
  asm volatile inline (
              "vmov.i8 q10, 0xfe\n\t"
              // First batch
              "vld1.32 {d0-d1}, [%1]!\n\t"  
              "vand q1, q0, q10\n\t"
              "vshr.u32 q1, #16\n\t"
              "vst1.32 {d2-d3}, [%0]!\n\t"
              // Second batch
              "vld1.32 {d0-d1}, [%1]!\n\t"   
              "vand q1, q0, q10\n\t"
              "vshr.u32 q1, #16\n\t"
              "vst1.32 {d2-d3}, [%0]!"
  : "+r&" (resultP), "+r&" (inputP)
  );
  //for (int i = 0; i < 8; ++i)
    //printf("0x%08x ", inputA[i]);
  printf("\n");
  for (int i = 0; i < 8; ++i)
    printf("0x%08x ", resultA[i]);
  printf("\n");
#endif

  return 0;
}
