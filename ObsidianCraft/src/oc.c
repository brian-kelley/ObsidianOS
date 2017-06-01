#include "oc.h"

//Seed (TODO: configure or randomize w/ time())
#define SEED 11

//Have 10x6 inventory
static Stack* inv;
//Eventually, have at least a 4x4x4 chunks (64^3 blocks) world
static Chunk* chunks;
static vec3 player;
static float yaw;     //yaw (left-right), radians, left is increasing
static float pitch;   //pitch (up-down), radians, ahead is 0, up is positive
/* Block list:
  AIR (0),
  STONE (1),    //Light grey
  DIRT (2)      //Dark brown, and green on top
  COAL (3)      //Black
  IRON (4)      //Dark grey
  GOLD (5)      //Yellow
  DIAMOND (6)   //Aqua
  LOG (7)       //Light brown
  LEAF (8)      //Medium green
  WATER (9)     //Blue
  SAND (10)     //Tan
  GLASS (11)
  CHEST (12)    //Lighter brown than log
  GRANITE (13)  //Pink
  QUARTZ (14)   //Pure white
  BEDROCK (15)  //Solid black
*/
//Colors of blocks
//dim 1 is block ID [0,15)
//dim 2 is light level [0,5); 0 is brightest, 4 is darkest

//For now, light level 0 is top, 1 is north/south, 2 is west/east, 3 is bottom, 4 is unused
static byte blockColor[16][5] = {
{0, 0, 0, 0, 0},                    //Air (colorless)
{0x1A, 0x19, 0x18, 0x17, 0x16},     //Stone
{0x02, 0x06, 0x72, 0x71, 0x70},     //Dirt
{0x12, 0x11, 0x10, 0x10, 0x10},     //Coal
{0x17, 0x16, 0x15, 0x14, 0x13},     //Iron
{0x0E, 0x2C, 0x44, 0x74, 0x8C},     //Gold
{0x4C, 0x0B, 0x4D, 0x03, 0x7C},     //Diamond
{0x42, 0x06, 0x73, 0x72, 0x71},     //Log
{0x02, 0x79, 0x77, 0xC0, 0xBF},     //Leaf
{0x37, 0x21, 0x21, 0x01, 0x68},     //Water
{0x44, 0x45, 0x8C, 0x8B, 0xD3},     //Sand
{0x0F, 0x0F, 0x0F, 0x0F, 0x0F},                    //Glass (colorless)
{0x43, 0x42, 0x06, 0x73, 0x72},     //Chest
{0x42, 0x41, 0x8B, 0x8A, 0x89},     //Granite
{0x1F, 0x1F, 0x1E, 0x1E, 0x1D},     //Quartz
{0, 0, 0, 0, 0}                     //Bedrock
};

inline bool isTransparent(byte b)
{
  return b == AIR || b == GLASS;
}

//Sky color
static byte sky = 0x35;

static bool wkey;
static bool akey;
static bool skey;
static bool dkey;
static bool ikey;
static bool jkey;
static bool kkey;
static bool lkey;

#define chunksX 4
#define chunksY 4
#define chunksZ 4

static void pumpEvents();
static void terrainGen();
static void initChunks();
static void processInput();

//player movement configuration
#define PLAYER_SPEED 0.30
#define X_SENSITIVITY 0.08
#define Y_SENSITIVITY 0.08

//3D configuration
#define NEAR 0.01f
#define FAR 64.0f
#define FOV 75.0f       //fovy (degrees)

static void clockSleep(int millis)
{
  clock_t start = clock();
  while(clock() < start + millis);
}

static Chunk* getChunk(int x, int y, int z)
{
  if(x < 0 || y < 0 || z < 0 || x >= chunksX || y >= chunksY || z >= chunksZ)
  {
    printf("Request chunk that is out-of-bounds: %i %i %i\n", x, y, z);
    while(1);
  }
  return &chunks[x + y * chunksX + z * chunksX * chunksY];
}

void ocMain()
{
  inv = malloc(60 * sizeof(Stack));
  chunks = malloc(chunksX * chunksY * chunksZ * sizeof(Chunk));
  terrainGen();
  initChunks();
  yaw = 0;
  pitch = -PI / 2 + 0.1;
  player.v[0] = 0;
  player.v[1] = 32;
  player.v[2] = 0;
  wkey = false;
  akey = false;
  skey = false;
  dkey = false;
  setModel(identity());
  setProj(perspective(FOV / (180.0f / PI), NEAR, FAR));
  while(1)
  {
    clock_t cstart = clock();
    pumpEvents();
    processInput();
    glClear(sky);
    //fill depth buf with maximum depth (255)
    memset(depthBuf, 0xFF, 64000);
    for(int i = 0; i < chunksX; i++)
    {
      for(int j = 0; j < chunksY; j++)
      {
        for(int k = 0; k < chunksZ; k++)
        {
          renderChunk(i, j, k);
        }
      }
    }
    glFlush();
    //hit 30 fps (if there is spare time this frame)
    while(clock() < cstart + 34);
  }
}

void renderChunk(int x, int y, int z)
{
  //immediately skip chunk if it is too far from player
  {
    //get distance to center of chunk
    float dx = player.v[0] - (x * 16 + 8);
    float dy = player.v[1] - (y * 16 + 8);
    float dz = player.v[2] - (z * 16 + 8);
    float distSquared = dx * dx + dy * dy + dz * dz;
    if(distSquared > ((FAR + 14) * (FAR + 14)))
      return;
  }
  Chunk* c = getChunk(x, y, z);
  if(c->filled == 0)
  {
    //chunk is completely empty
    return;
  }
  //chunk offset position
  glBegin(GL_QUADS);
  int cx = x * 16;
  int cy = y * 16;
  int cz = z * 16;
  glEnableDepthTest(true);
  //how many blocks have already been drawn
  for(int i = 0; i < 16; i++)
  {
    for(int j = 0; j < 16; j++)
    {
      for(int k = 0; k < 16; k++)
      {
        {
          int ind = i + j * 16 + k * 256;
          if((c->visible[ind >> 3] & (1 << (ind & 0x7))) == 0)
          {
            //block is either air or is fully hidden by opaque blocks
            continue;
          }
        }
        float bx = cx + i;
        float by = cy + j;
        float bz = cz + k;
        byte block = getBlockC(c, i, j, k);
        //byte block = getBlock(bx, by, bz);
        //if a block is certainly not in the player's view, ignore it
        //compute single depth value for whole block (using view space, not perspective)
        //determine the nearest corner to player (for most aggressive depth value)
        vec3 face = {bx, by, bz};
        if(player.v[0] <= bx + 1)
          face.v[0] += 1;
        if(player.v[1] <= by + 1)
          face.v[1] += 1;
        if(player.v[2] <= bz + 1)
          face.v[2] += 1;
        //take the nearest corner and run it through the full projection
        vec4 viewSpace = matvec3(viewMat, face);
        vec4 clip = matvec4(projMat, viewSpace);
        //must divide by w
        float invw = 1.0f / clip.v[3];
        clip.v[0] *= invw;
        clip.v[1] *= invw;
        clip.v[2] *= invw;
        //clip blocks that are > 1 block outside the edge of the viewport
        //conveniently, invw is the clipspace size of a block at the distance of the block
        //z still clips to exactly -1,1
        float clipVal = 1.3 + invw;
        if(clip.v[0] < -clipVal  || clip.v[0] > clipVal ||
            clip.v[1] < -clipVal || clip.v[1] > clipVal || 
            clip.v[2] <= -1 || clip.v[2] >= 1)
        {
          //skip block, as nearest corner to player is outside frustum
          continue;
        }
        //in depth value, far plane is 64, so anything that
        //would overflow the byte has already been clipped
        glDepth(-viewSpace.v[2] * 3);
        if(block == GLASS)
          glDrawMode(DRAW_WIREFRAME);
        else
          glDrawMode(DRAW_FILL);
        glColor1i(blockColor[block][1]);
        if(player.v[0] < bx + 1)
        {
          //might need to draw low X face
          byte neighbor = getBlock(bx - 1, by, bz);
          if(isTransparent(neighbor))
          {
            //need to draw the face
            //do depth testing and update depth values along the way
            glVertex3f(bx, by, bz);
            glVertex3f(bx, by + 1, bz);
            glVertex3f(bx, by + 1, bz + 1);
            glVertex3f(bx, by, bz + 1);
          }
        }
        if(player.v[0] > bx)
        {
          byte neighbor = getBlock(bx + 1, by, bz);
          if(isTransparent(neighbor))
          {
            glVertex3f(bx + 1, by, bz);
            glVertex3f(bx + 1, by + 1, bz);
            glVertex3f(bx + 1, by + 1, bz + 1);
            glVertex3f(bx + 1, by, bz + 1);
          }
        }
        if(player.v[1] < by + 1)
        {
          byte neighbor = getBlock(bx, by - 1, bz);
          if(isTransparent(neighbor))
          {
            glColor1i(blockColor[block][3]);
            glVertex3f(bx, by, bz);
            glVertex3f(bx + 1, by, bz);
            glVertex3f(bx + 1, by, bz + 1);
            glVertex3f(bx, by, bz + 1);
          }
        }
        if(player.v[1] > by)
        {
          byte neighbor = getBlock(bx, by + 1, bz);
          if(isTransparent(neighbor))
          {
            glColor1i(blockColor[block][0]);
            glVertex3f(bx, by + 1, bz);
            glVertex3f(bx + 1, by + 1, bz);
            glVertex3f(bx + 1, by + 1, bz + 1);
            glVertex3f(bx, by + 1, bz + 1);
          }
        }
        glColor1i(blockColor[block][2]);
        if(player.v[2] < bz + 1)
        {
          byte neighbor = getBlock(bx, by, bz - 1);
          if(isTransparent(neighbor))
          {
            glVertex3f(bx, by, bz);
            glVertex3f(bx + 1, by, bz);
            glVertex3f(bx + 1, by + 1, bz);
            glVertex3f(bx, by + 1, bz);
          }
        }
        if(player.v[2] > bz)
        {
          byte neighbor = getBlock(bx, by, bz + 1);
          if(isTransparent(neighbor))
          {
            glVertex3f(bx, by, bz + 1);
            glVertex3f(bx + 1, by, bz + 1);
            glVertex3f(bx + 1, by + 1, bz + 1);
            glVertex3f(bx, by + 1, bz + 1);
          }
        }
      }
    }
  }
  glEnd();
}

//any block outside of the world is treated as air
byte getBlock(int x, int y, int z)
{
  if(x < 0 || y < 0 || z < 0 || x >= chunksX * 16 || y >= chunksY * 16 || z >= chunksZ * 16)
    return AIR;
  return getBlockC(getChunk(x / 16, y / 16, z / 16), x % 16, y % 16, z % 16);
}

void setBlock(byte b, int x, int y, int z)
{
  if(x < 0 || y < 0 || z < 0 || x >= chunksX * 16 || y >= chunksY * 16 || z >= chunksZ * 16)
    return;
  setBlockC(b, getChunk(x / 16, y / 16, z / 16), x % 16, y % 16, z % 16);
}

byte getBlockC(Chunk* c, int cx, int cy, int cz)
{
  int ind = cx + (cy << 4) + (cz << 8);
  byte b = c->vals[ind >> 1];
  return ind & 1 ? (b >> 4) : (b & 0xF);
}

void setBlockC(byte newBlock, Chunk* c, int cx, int cy, int cz)
{
  int ind = cx + (cy << 4) + (cz << 8);
  byte b = c->vals[ind >> 1];
  if(ind & 1)
  {
    b &= 0x0F;
    b |= (newBlock << 4);
  }
  else
  {
    b &= 0xF0;
    b |= newBlock;
  }
  //write back new value
  c->vals[ind >> 1] = b;
}

void pumpEvents()
{
  while(haveEvent())
  {
    Event ev = getNextEvent();
    //call event handler
    switch(ev.type)
    {
      case KEY_EVENT:
        {
          KeyEvent k = ev.e.key;
          switch(k.scancode)
          {
            case KEY_W:
              wkey = k.pressed; break;
            case KEY_A:
              akey = k.pressed; break;
            case KEY_S:
              skey = k.pressed; break;
            case KEY_D:
              dkey = k.pressed; break;
            case KEY_I:
              ikey = k.pressed; break;
            case KEY_J:
              jkey = k.pressed; break;
            case KEY_K:
              kkey = k.pressed; break;
            case KEY_L:
              lkey = k.pressed; break;
            default:;
          }
        }
        break;
      case MOTION_EVENT:
        {
          //TODO: support configuring for either mouse or ijkl for camera movement?
          /*
             const float pitchLimit = 88 / (180.0f / PI);
             viewUpdated = true;
             yaw += X_SENSITIVITY * ev.e.motion.dx;
          //clamp yaw to 0:2pi (but preserve rotation beyond the bounds)
          if(yaw < 0)
          yaw += 2 * PI;
          else if(yaw >= 2 * PI)
          yaw -= 2 * PI;
          pitch -= Y_SENSITIVITY * ev.e.motion.dy;
          //clamp pitch to -pitchLimit:pitchLimit
          if(pitch < -pitchLimit)
          pitch = -pitchLimit;
          else if(pitch > pitchLimit)
          pitch = pitchLimit;
          */
          break;
        }
      case BUTTON_EVENT:
        {
          if(ev.e.button.button == LEFT_BUTTON && ev.e.button.pressed)
          {
            //left click
          }
          else if(ev.e.button.button == RIGHT_BUTTON && ev.e.button.pressed)
          {
            //right click
          }
          break;
        }
      default:;
    }
  }
}

//Seed rng with unique hash of block coordinates, combined with octave value
static void srandBlockHash(int x, int y, int z, int octave)
{
  //srand(4 * (x + y * (chunksX * 16 + 1) + z * (chunksX * 16 * chunksY * 16 + 1)) + octave);
  int hash = 4 * (x + y * (chunksX * 16 + 1) + z * (chunksX * 16 * chunksY * 16 + 1)) + octave;
  //printf("Hash of %i, %i, %i, octave %i: %i\n", x, y, z, octave, hash);
  srand(hash);
}

void terrainGen()
{
  int wx = chunksX * 16;
  int wy = chunksY * 16;
  int wz = chunksZ * 16;
  /*  build some fractal noise in-place (with already allocated 4 bits per block)
      always clamp values to [0, 16)
      first, fill with some random vals (with a small maximum, like 2)
      then, repeatedly sample some small 3D region (-50%), scale its values up (+50%), and add it back to original
  */
  //Base fractal noise
  //Each chunk computed independently (TODO: whole terrain gen should independent of neighbors in final product)
  //sample at 4 octaves: 0, 1, 2, 3
  //octave i has amplitude 2^i and frequency 2^(-i)
  //that way, absolute max value possible is 15 (perfect for 4-bit range)
  //assume octave 0 has sample points every 2 blocks
  for(int c = 0; c < chunksX * chunksY * chunksZ; c++)
  {
    int cx = c % chunksX;
    int cy = (c / chunksX) % chunksY;
    int cz = c / (chunksX * chunksY);
    Chunk* chunk = getChunk(cx, cy, cz);
    memset(chunk, 0, sizeof(Chunk));
    //each chunk is unique
    for(int octave = 0; octave < 4; octave++)
    {
      int amplitude = 1 << octave;
      //period = distance between samples (must evenly divide 16)
      int period = 2 * (1 << octave);
      //freq = samples per chunk length
      int freq = 16 / period;
      //loop over sample cubes, then loop over blocks within sample cubes and lerp its value from this octave
      //printf("Octave %i: %i sample cubes of side length %i\n", octave, freq, period);
      for(int sample = 0; sample < freq * freq * freq; sample++)
      {
        int sx = sample % freq;
        int sy = (sample / freq) % freq;
        int sz = sample / (freq * freq);
        //printf("Sample: %i %i %i\n", sx, sy, sz);
        //clockSleep(500);
        int bx = 16 * cx + sx * period;
        int by = 16 * cy + sy * period;
        int bz = 16 * cz + sz * period;
        //samples at the 8 sampling cube corners
        int samples[8];
        srandBlockHash(bx + period, by + period, bz + period, octave);
        samples[0] = rand() % (amplitude + 1);
        srandBlockHash(bx, by + period, bz + period, octave);
        samples[1] = rand() % (amplitude + 1);
        srandBlockHash(bx + period, by, bz + period, octave);
        samples[2] = rand() % (amplitude + 1);
        srandBlockHash(bx, by, bz + period, octave);
        samples[3] = rand() % (amplitude + 1);
        srandBlockHash(bx + period, by + period, bz, octave);
        samples[4] = rand() % (amplitude + 1);
        srandBlockHash(bx, by + period, bz, octave);
        samples[5] = rand() % (amplitude + 1);
        srandBlockHash(bx + period, by, bz, octave);
        samples[6] = rand() % (amplitude + 1);
        srandBlockHash(bx, by, bz, octave);
        samples[7] = rand() % (amplitude + 1);
        for(int x = 0; x < period; x++)
        {
          for(int y = 0; y < period; y++)
          {
            for(int z = 0; z < period; z++)
            {
              //values proportional volume in cuboid between opposite corner and interpolation point
              int v = 0;
              v += samples[0] * x * y * z;
              v += samples[1] * (period - x) * y * z;
              v += samples[2] * x * (period - y) * z;
              v += samples[3] * (period - x) * (period - y) * z;
              v += samples[4] * x * y * (period - z);
              v += samples[5] * (period - x) * y * (period - z);
              v += samples[6] * x * (period - y) * (period - z);
              v += samples[7] * (period - x) * (period - y) * (period - z);
              int val = getBlock(bx + x, by + y, bz + z);
              //add the weighted average of sample cube corner values
              val += v / (period * period * period);
              if(val > 15)
                val = 15;
              setBlock(val, bx + x, by + y, bz + z);
            }
          }
        }
      }
    }
  }
  /*
  //provide a downward bias for values high above sea level
  for(int x = 0; x < wx; x++)
  {
    for(int y = 0; y < wy; y++)
    {
      for(int z = 0; z < wz; z++)
      {
        float shift = wy / 2 - y;
        if(y > wy / 2)
          shift *= 0.50;
        else
          shift *= 0.10;
        int val = getBlock(x, y, z);
        val += shift;
        if(val < 0)
          val = 0;
        if(val > 15)
          val = 15;
        setBlock(val, x, y, z);
      }
    }
  }
  */
  /*
  //run a few sweeps of a smoothing function
  //basically gaussian blur, or like a game of life automaton
  for(int sweep = 0; sweep < 4; sweep++)
  {
    for(int x = 0; x < wx; x++)
    {
      for(int y = 0; y < wy; y++)
      {
        for(int z = 0; z < wz; z++)
        {
          //test neighbors around
          //note: values outside world have value 0
          //since threshold is 8, dividing sum of neighbor values by 26 and then comparing against same threshold provides smoothing function
          int neighborVals = 0;
          for(int tx = -1; tx <= 1; tx++)
          {
            for(int ty = -1; ty <= 1; ty++)
            {
              for(int tz = -1; tz <= 1; tz++)
              {
                neighborVals += getBlock(x + tx, y + ty, z + tz);
              }
            }
          }
          neighborVals /= 27;
          if(neighborVals >= 8 + rand() % 2)
            setBlock(13, x, y, z);
          else
            setBlock(4, x, y, z);
        }
      }
    }
  }
  */
  //now, set each block above a threshold to stone, and each below to air
  for(int x = 0; x < wx; x++)
  {
    for(int y = 0; y < wy; y++)
    {
      for(int z = 0; z < wz; z++)
      {
        int val = getBlock(x, y, z);
        if(val >= 6)
        {
          setBlock(STONE, x, y, z);
        }
        else
          setBlock(AIR, x, y, z);
      }
    }
  }
  //set the bottom layer of world to bedrock
  for(int i = 0; i < wx; i++)
  {
    for(int j = 0; j < wz; j++)
    {
      setBlock(BEDROCK, i, 0, j);
    }
  }
  //set all surface blocks to dirt
  for(int x = 0; x < wx; x++)
  {
    for(int z = 0; z < wz; z++)
    {
      //trace down from sky to find highest block
      for(int y = wy - 1; y >= 0; y--)
      {
        byte b = getBlock(x, y, z);
        if(b == STONE)
        {
          setBlock(DIRT, x, y, z);
          break;
        }
      }
    }
  }
  //set all air blocks below sea level to water
  for(int x = 0; x < wx; x++)
  {
    for(int y = 0; y < wy / 2; y++)
    {
      for(int z = 0; z < wz; z++)
      {
        if(getBlock(x, y, z) == AIR)
        {
          //setBlock(WATER, x, y, z);
        }
      }
    }
  }
  /*
  //find random places on the surface to plant trees
  for(int tree = 0; tree < 2 * chunksX * chunksZ; tree++)
  {
    //determine if the highest block here is dirt
    int x = rand() % wx;
    int z = rand() % wz;
    bool hitDirt = false;
    int y;
    for(y = wy - 1; y >= wy / 2; y--)
    {
      byte b = getBlock(x, y, z);
      if(b == DIRT)
        hitDirt = true;
      if(b != AIR)
        break;
    }
    if(!hitDirt)
      continue;
    //try to plant the tree on dirt block @ (x, y, z)
    int treeHeight = 4 + rand() % 4;
    for(int i = 0; i < treeHeight; i++)
    {
      setBlock(LOG, x, y + 1 + i, z);
    }
    setBlock(LEAF, x, y + treeHeight, z);
    //do a depth-first strategy to attach leaves to the upper half of the logs
    //send out some "rays" on random walks from a log at least halfway up the trunk,
    //with a limited range and replacing all air with leaf
    //rays can move in any direction except down
    for(int ray = 0; ray < treeHeight * 2; ray++)
    {
      int rayH = (treeHeight / 2) + (rand() % (treeHeight / 2));
      int rx = x;
      int ry = y + 1 + rayH;
      int rz = z;
      int rayLen = 3 + rand() % treeHeight;
      for(int ri = 0; ri < rayLen; ri++)
      {
        switch(rand() % 5)
        {
          case 0: rx--; break;
          case 1: rx++; break;
          case 2: rz--; break;
          case 3: rz++; break;
          case 4: ry++; break;
          default:;
        }
        //test for out of bounds
        if(rx < 0 || rx >= wx || ry >= wy || rz < 0 || rz >= wz)
          break;
        //place leaf if existing is air
        if(getBlock(rx, ry, rz) == AIR)
        {
          //setBlock(LEAF, rx, ry, rz);
        }
      }
    }
  }
  */
}

static void initChunks()
{
  //initialize visible and filled fields of all chunks
  //byte visible[512];
  //short filled;
  for(int cx = 0; cx < chunksX; cx++)
  {
    for(int cy = 0; cy < chunksY; cy++)
    {
      for(int cz = 0; cz < chunksZ; cz++)
      {
        Chunk* c = getChunk(cx, cy, cz);
        c->filled = 0;
        memset(c->visible, 0, 512);
        for(int i = 0; i < 16; i++)
        {
          for(int j = 0; j < 16; j++)
          {
            for(int k = 0; k < 16; k++)
            {
              int bx = cx * 16 + i;
              int by = cy * 16 + j;
              int bz = cz * 16 + k;
              if(getBlockC(c, i, j, k) != AIR)
              {
                c->filled++;
                //check the 6 neighbors for transparent blocks
                if(isTransparent(getBlock(bx - 1, by, bz)) ||
                    isTransparent(getBlock(bx + 1, by, bz)) ||
                    isTransparent(getBlock(bx, by - 1, bz)) ||
                    isTransparent(getBlock(bx, by + 1, bz)) ||
                    isTransparent(getBlock(bx, by, bz - 1)) ||
                    isTransparent(getBlock(bx, by, bz + 1)))
                {
                  //set bit in c->visible
                  int ind = i + j * 16 + k * 256;
                  c->visible[ind >> 3] |= (1 << (ind & 0x7));
                }
              }
            }
          }
        }
      }
    }
  }
}

void processInput()
{
  //handle orientation changes via ijkl
  const float pitchLimit = 89 / (180.0f / PI);
  int dx = (jkey ? -1 : 0) + (lkey ? 1 : 0);
  int dy = (ikey ? -1 : 0) + (kkey ? 1 : 0);
  yaw += X_SENSITIVITY * dx;
  //clamp yaw to 0:2pi (but preserve rotation beyond the bounds)
  if(yaw < 0)
    yaw += 2 * PI;
  else if(yaw >= 2 * PI)
    yaw -= 2 * PI;
  pitch -= Y_SENSITIVITY * dy;
  //clamp pitch to -pitchLimit:pitchLimit
  if(pitch < -pitchLimit)
    pitch = -pitchLimit;
  else if(pitch > pitchLimit)
    pitch = pitchLimit;
  vec3 target = {cosf(pitch) * cosf(yaw), sinf(pitch), cosf(pitch) * sinf(yaw)};
  vec3 right = {-sinf(yaw), 0, cosf(yaw)};
  int xvel = 0;
  int yvel = 0;
  if(wkey)
    xvel++;
  if(skey)
    xvel--;
  if(akey)
    yvel--;
  if(dkey)
    yvel++;
  if(xvel)
  {
    player = vecadd(player, vecscale(target, PLAYER_SPEED * xvel));
  }
  if(yvel)
  {
    player = vecadd(player, vecscale(right, PLAYER_SPEED * yvel));
  }
  if(dx || dy || xvel || yvel)
  {
    //must update view matrix
    vec3 up = cross(right, target);
    target = vecadd(target, player);
    setView(lookAt(player, target, up));
  }
}

