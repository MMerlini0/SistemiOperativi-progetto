#pragma once
#include <stdint.h>

// Struttura della bitmap
typedef struct  {
  uint8_t *buffer;
  int buffer_size;
  int num_bits; 
} BitMap;


// Le funzioni sono spiegate nel file corrispettivo (bit_map.c)
int BitMap_getBytes(int bits);
void BitMap_init(BitMap* bit_map, int num_bits, char* buffer);
void BitMap_setBit(BitMap* bit_map, int bit_num, int status);
int BitMap_bit(const BitMap* bit_map, int bit_num);
void BitmapMain_print(BitMap* bit_map);