#ifndef OC_H
#define OC_H

#include "stdio.h"
#include "graphics.h"

//4-bit block IDs
enum Block 
{
  AIR,
  STONE,
  DIRT,
  COAL,
  IRON,
  GOLD,
  DIAMOND,
  LOG,
  LEAF,
  WATER,
  SAND,
  GLASS
};

typedef struct
{
  byte item;
  byte count;
} Stack; 

typedef struct
{
  //4096 blocks (16^3), each 4 bits
  byte vals[2048];
} Chunk;

void ocMain();
byte getBlock(Chunk* c, int cx, int cy, int cz);
void setBlock(byte b, Chunk* c, int cx, int cy, int cz);

#endif

