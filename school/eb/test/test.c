#include <stdio.h>
#include <stdint.h>

void testPack(uint16_t* array, int array_size, int bits, uint8_t* result) {
  uint32_t buffer = 0;
  int buffer_bits = 0;
  int result_index = 0;

  for(int idx = 0; idx < array_size; idx++) {
    buffer |= (array[idx] << buffer_bits);
    buffer_bits += bits;

    while(buffer_bits >= 8) {
      result[result_index++] = buffer & 0xFF;
      buffer >>= 8;
      buffer_bits -= 8;
    }
  }

  // Any remaining bits.
  if(buffer_bits > 0) {
    result[result_index++] = buffer;
  }
}

int main() {
  uint16_t array[] = {1, 2, 3, 10, 15, 32};
  int array_size = sizeof(array)/sizeof(uint16_t);
  uint8_t result[100] = {0}; // make sure it's large enough.

  for(int i = 0; i < array_size; i++) {
    printf("%d ", array[i]);
  }

  int bits = 7;
  testPack(array, array_size, bits, result);

  printf("Packed array: ");

  for(int i = 0; i < ((array_size * bits + 7) / 8); i++) {
    printf("%02x ", result[i]);
  }
  putchar('\n');

  return 0;
}

