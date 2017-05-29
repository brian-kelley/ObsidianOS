#include "oc.h"

//Have 10x6 inventory
static Stack* inv;
//Have 4x4x4 chunks (64^3 world)
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
*/
//Colors of blocks
//dim 1 is block ID [0,15)
//dim 2 is light level [0,5); 0 is brightest, 4 is darkest

//For now, light level 0 is top, 1 is north/south, 2 is west/east, 3 is bottom, 4 is unused
static byte blockColor[15][5] = {
{0, 0, 0, 0, 0},                    //Air (colorless)
{0x1A, 0x19, 0x18, 0x17, 0x16},     //Stone
{0x02, 0x06, 0x72, 0x71, 0x70},     //Dirt
{0x12, 0x11, 0x10, 0x10, 0x10},     //Coal
{0x17, 0x16, 0x15, 0x14, 0x13},     //Iron
{0x0E, 0x2C, 0x44, 0x74, 0x8C},     //Gold
{0x4C, 0x0B, 0x4D, 0x03, 0x7C},     //Diamond
{0x42, 0x06, 0x73, 0x72, 0x71},     //Log
{0x02, 0x79, 0x77, 0xC0, 0xBF},     //Leaf
{0x37, 0x21, 0x22, 0x01, 0x68},     //Water
{0x44, 0x45, 0x8C, 0x8B, 0xD3},     //Sand
{0x0F, 0x0F, 0x0F, 0x0F, 0x0F},                    //Glass (colorless)
{0x43, 0x42, 0x06, 0x73, 0x72},     //Chest
{0x42, 0x41, 0x8B, 0x8A, 0x89},     //Granite
{0x1F, 0x1F, 0x1E, 0x1E, 0x1D}      //Quartz
};

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

static int chunksX = 1;
static int chunksY = 1;
static int chunksZ = 1;

static void pumpEvents();
static void drawCube(float x, float y, float z, float size);
static void terrainGen();
static void processInput();

//player movement configuration
#define PLAYER_SPEED 0.15
#define X_SENSITIVITY 0.04
#define Y_SENSITIVITY 0.04

static void clockSleep(int millis)
{
  clock_t start = clock();
  while(clock() < start + millis);
}

void ocMain()
{
  inv = malloc(60 * sizeof(Stack));
  chunks = malloc(chunksX * chunksY * chunksZ * sizeof(Chunk));
  yaw = 0;
  pitch = 0;
  //start in center of world
  player.v[0] = 32;
  player.v[1] = 34;
  player.v[2] = 32;
  wkey = false;
  akey = false;
  skey = false;
  dkey = false;
  terrainGen();
  setModel(identity());
  int fov = 90;
  setProj(perspective(fov / (180.0f / PI), 1, 1000));
  while(1)
  {
    clock_t cstart = clock();
    pumpEvents();
    processInput();
    glClear(sky);
    //fill depth buf with maximum depth (255)
    memset(depthBuf, 0xFF, 64000);
    renderChunk(0, 0, 0);
    glFlush();
    //vsync();
    while(clock() < cstart + 17);
  }
}

//for a face to be rendered, it must:
//  -be facing the player (i.e. if top face, player y must be > face y)
//  -have a transparent block (air/glass) as a neighbor

void renderChunk(int x, int y, int z)
{
  Chunk* c = &chunks[x + y * chunksX + z * chunksX * chunksY];
  //chunk offset position
  glBegin(GL_QUADS);
  int cx = x * 16;
  int cy = y * 16;
  int cz = z * 16;
  glEnableDepthTest(true);
  for(int i = 0; i < 16; i++)
  {
    for(int j = 0; j < 16; j++)
    {
      for(int k = 0; k < 16; k++)
      {
        byte block = getBlockC(c, i, j, k);
        if(block == AIR)
          continue;
        float bx = cx + i;
        float by = cy + j;
        float bz = cz + k;
        //compute depth value for whole block
        vec3 face = {bx, by, bz};
        vec4 viewSpace = matvec3(viewMat, face);
        glDepth(-viewSpace.v[2] * 4);
        if(block == GLASS)
          glDrawMode(DRAW_WIREFRAME);
        else
          glDrawMode(DRAW_FILL);
        glColor1i(blockColor[block][1]);
        if(player.v[0] < bx)
        {
          //might need to draw low X face
          byte neighbor = getBlock(bx - 1, by, bz);
          if(neighbor == AIR || neighbor == GLASS)
          {
            //need to draw the face
            //do depth testing and update depth values along the way
            glVertex3f(bx, by, bz);
            glVertex3f(bx, by + 1, bz);
            glVertex3f(bx, by + 1, bz + 1);
            glVertex3f(bx, by, bz + 1);
          }
        }
        else if(player.v[0] > bx + 1)
        {
          byte neighbor = getBlock(bx + 1, by, bz);
          if(neighbor == AIR || neighbor == GLASS)
          {
            glVertex3f(bx + 1, by, bz);
            glVertex3f(bx + 1, by + 1, bz);
            glVertex3f(bx + 1, by + 1, bz + 1);
            glVertex3f(bx + 1, by, bz + 1);
          }
        }
        if(player.v[1] < by)
        {
          byte neighbor = getBlock(bx, by - 1, bz);
          if(neighbor == AIR || neighbor == GLASS)
          {
            glColor1i(blockColor[block][3]);
            glVertex3f(bx, by, bz);
            glVertex3f(bx + 1, by, bz);
            glVertex3f(bx + 1, by, bz + 1);
            glVertex3f(bx, by, bz + 1);
          }
        }
        else if(player.v[1] > by + 1)
        {
          byte neighbor = getBlock(bx, by + 1, bz);
          if(neighbor == AIR || neighbor == GLASS)
          {
            glColor1i(blockColor[block][0]);
            glVertex3f(bx, by + 1, bz);
            glVertex3f(bx + 1, by + 1, bz);
            glVertex3f(bx + 1, by + 1, bz + 1);
            glVertex3f(bx, by + 1, bz + 1);
          }
        }
        glColor1i(blockColor[block][2]);
        if(player.v[2] < bz)
        {
          byte neighbor = getBlock(bx, by, bz - 1);
          if(neighbor == AIR || neighbor == GLASS)
          {
            glVertex3f(bx, by, bz);
            glVertex3f(bx + 1, by, bz);
            glVertex3f(bx + 1, by + 1, bz);
            glVertex3f(bx, by + 1, bz);
          }
        }
        else if(player.v[2] > bz + 1)
        {
          byte neighbor = getBlock(bx, by, bz + 1);
          if(neighbor == AIR || neighbor == GLASS)
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

byte getBlock(int x, int y, int z)
{
  if(x < 0 || y < 0 || z < 0 || x > chunksX * 16 || y > chunksY * 16 || z > chunksZ * 16)
    return AIR;
  return getBlockC(&chunks[(x / 16) + 4 * (y / 16) + 16 * (z / 16)], x % 16, y % 16, z % 16);
}

void setBlock(byte b, int x, int y, int z)
{
  if(x < 0 || y < 0 || z < 0 || x > chunksX * 16 || y > chunksY * 16 || z > chunksZ * 16)
    return;
  setBlockC(b, &chunks[(x / 16) + chunksX * (y / 16) + chunksX * chunksY * (z / 16)], x % 16, y % 16, z % 16);
}

byte getBlockC(Chunk* c, int cx, int cy, int cz)
{
  int ind = (cx + (cy << 4) + (cz << 8)) >> 1;
  byte b = c->vals[ind];
  return ind & 1 ? (b >> 4) : (b & 0xF);
}

void setBlockC(byte newBlock, Chunk* c, int cx, int cy, int cz)
{
  int ind = (cx + (cy << 4) + (cz << 8)) >> 1;
  byte b = c->vals[ind];
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
  c->vals[ind] = b;
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

void drawCube(float x, float y, float z, float size)
{
  glBegin(GL_QUADS);
  //Bottom face (low y)
  if(player.v[1] < y)
  {
    glColor1i(0x2);
    glVertex3f(x, y, z);
    glVertex3f(x + size, y, z);
    glVertex3f(x + size, y, z + size);
    glVertex3f(x, y, z + size);
  }
  //Top face (high y)
  if(player.v[1] > y + size)
  {
    glColor1i(0x3);
    glVertex3f(x, y + size, z);
    glVertex3f(x + size, y + size, z);
    glVertex3f(x + size, y + size, z + size);
    glVertex3f(x, y + size, z + size);
  }
  //Left face (low x)
  if(player.v[0] < x)
  {
    glColor1i(0x4);
    glVertex3f(x, y, z);
    glVertex3f(x, y + size, z);
    glVertex3f(x, y + size, z + size);
    glVertex3f(x, y, z + size);
  }
  //Right face (high x)
  if(player.v[0] > x + size)
  {
    glColor1i(0x5);
    glVertex3f(x + size, y, z);
    glVertex3f(x + size, y + size, z);
    glVertex3f(x + size, y + size, z + size);
    glVertex3f(x + size, y, z + size);
  }
  //Back face (low z);
  if(player.v[2] < z)
  {
    glColor1i(0x6);
    glVertex3f(x, y, z);
    glVertex3f(x + size, y, z);
    glVertex3f(x + size, y + size, z);
    glVertex3f(x, y + size, z);
  }
  //Front face (high z);
  if(player.v[2] > z + size)
  {
    glColor1i(0x7);
    glVertex3f(x, y, z + size);
    glVertex3f(x + size, y, z + size);
    glVertex3f(x + size, y + size, z + size);
    glVertex3f(x, y + size, z + size);
  }
  glEnd();
}

void terrainGen()
{
  for(int x = 0; x < 16; x++)
  {
    for(int y = 0; y < 16; y++)
    {
      for(int z = 0; z < 16; z++)
      {
        if(y % 2)
        {
          setBlock(STONE, x, y, z);
        }
        else
        {
          setBlock(AIR, x, y, z);
        }
      }
    }
  }
}

void processInput()
{
  //handle orientation changes via ijkl
  const float pitchLimit = 88 / (180.0f / PI);
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

