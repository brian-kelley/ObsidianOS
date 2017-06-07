#ifndef OC_H
#define OC_H

#include "stdio.h"
#include "assert.h"
#include "time.h"
#include "graphics.h"

//4-bit block IDs
enum Block 
{
  AIR,
  STONE,      //Light grey
  DIRT,       //Dark brown, and green on top
  COAL,       //Black
  IRON,       //Dark grey
  GOLD,       //Yellow
  DIAMOND,    //Aqua
  LOG,        //Light brown
  LEAF,       //Medium green
  WATER,      //Blue
  SAND,       //Tan
  GLASS,
  CHEST,      //Lighter brown than log
  GRANITE,    //Pink
  QUARTZ,     //Pure white
  BEDROCK     //Solid black (darker than coal)
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
  //visible: each bit is whether block has >=1 transparent neighbor AND is not air
  byte visible[512];
  //number of blocks that are not air
  short filled;
} Chunk;

void ocMain();
void renderChunk(int x, int y, int z);
byte getBlock(int x, int y, int z);
void setBlock(byte b, int x, int y, int z);
byte getBlockC(Chunk* c, int cx, int cy, int cz);
void setBlockC(byte b, Chunk* c, int cx, int cy, int cz);

#endif

