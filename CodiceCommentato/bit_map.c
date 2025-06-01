#include <assert.h>
#include "bit_map.h"
#include <stdio.h>



// Funzione che converte bit in byte
int BitMap_getBytes(int bits){
  return bits/8 + ((bits%8)!=0);
}


// Inizializzazione della bitmap, assegna buffer, numero bits gestiti e bytes gestiti
void BitMap_init(BitMap* bit_map, int num_bits, char* buffer){
  bit_map->buffer = buffer;
  bit_map->num_bits=num_bits;
  bit_map->buffer_size=BitMap_getBytes(num_bits);
}


// Modifica il bit in posizione passata bit_num, al valore passato status
void BitMap_setBit(BitMap* bit_map, int bit_num, int status){
  int byte_num=bit_num>>3;
  assert(byte_num<bit_map->buffer_size);
  int bit_in_byte = bit_num & 0x07; // Prima era errato e settato a & 0x03
  if (status) {
    bit_map->buffer[byte_num] |= (1<<bit_in_byte);
  } else {
    bit_map->buffer[byte_num] &= ~(1<<bit_in_byte);
  }
}

// Riporta il valore del bit in posizione passata bit_num
int BitMap_bit(const BitMap* bit_map, int bit_num){
  int byte_num=bit_num>>3; 
  assert(byte_num<bit_map->buffer_size);
  int bit_in_byte = bit_num & 0x07; // Anche qua prima era errato e settato a & 0x03
  return (bit_map->buffer[byte_num] & (1<<bit_in_byte))!=0;
}


// Funzione ausiliaria di print di tutto lo stato dei bit, usata nel main
void BitmapMain_print(BitMap* bit_map) {
  printf("[");
  for (int i = 0; i < bit_map->num_bits; i++) {
    printf(" %d ", BitMap_bit(bit_map, i));
  }
  printf("]\n");
}
