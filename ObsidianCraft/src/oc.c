#include "oc.h"

//Have 10x6 inventory
static Stack* inv;
//Have 4x4x4 chunks (64^3 world)
static Chunk* chunks;
static vec3 player;
static float yaw;     //yaw (left-right), radians, left is increasing
static float pitch;   //pitch (up-down), radians, ahead is 0, up is positive
//Solid colors of blocks (at highest light level)
static byte blockColor[] = {0x00, 0x16, 0x06, 0x12, 0x1B, 0x2C, 0x4D, 0xF0, 0x77, 0x37, 0x44, 0x00};
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
    //TESTING CAMERA
    clock_t cstart = clock();
    pumpEvents();
    processInput();
    glClear(sky);
    //fill depth buf with maximum depth (255)
    memset(depthBuf, 0xFF, 64000);
    //renderChunk(0, 0, 0);
    glBegin(GL_QUADS);
    glColor1i(blockColor[STONE]);
    for(int i = 0; i < 32; i++)
    {
      for(int j = 0; j < 32; j++)
      {
        glVertex3f(i, j, 0);
        glVertex3f(i + 1, j, 0);
        glVertex3f(i + 1, j + 1, 0);
        glVertex3f(i, j + 1, 0);
      }
    }
    glColor1i(blockColor[DIAMOND]);
    for(int i = 0; i < 32; i++)
    {
      for(int j = 0; j < 32; j++)
      {
        glVertex3f(0, i, j);
        glVertex3f(0, i + 1, j);
        glVertex3f(0, i + 1, j + 1);
        glVertex3f(0, i, j + 1);
      }
    }
    glColor1i(blockColor[IRON]);
    for(int i = 0; i < 32; i++)
    {
      for(int j = 0; j < 32; j++)
      {
        glVertex3f(i, 0, j);
        glVertex3f(i + 1, 0, j);
        glVertex3f(i + 1, 0, j + 1);
        glVertex3f(i, 0, j + 1);
      }
    }
    glEnd();
    //drawCube(32, 32, 39, 1);
    //drawCube(33, 33, 40, 1);
    glFlush();
    //vsync();
    while(clock() < cstart + 17);
  }
}

void renderChunk(int x, int y, int z)
{
  Chunk* c = &chunks[x + y * chunksX + z * chunksX * chunksY];
  glBegin(GL_QUADS);
  for(int i = 0; i < 16; i++)
  {
    for(int j = 0; j < 16; j++)
    {
      for(int k = 0; k < 16; k++)
      {
        byte block = getBlock(i, j, k);
        if(block == AIR)
          continue;
        glColor1i(blockColor[block]);
        //test whether faces right(+x), above (+y), and front (+z) need to be drawn
        //right face:
        byte rightNeighbor = getBlock(i + 1, j, k);
        if(i == 15 || rightNeighbor == AIR || rightNeighbor == GLASS)
        {
          //face would need to be drawn, if it is visible
          //check depth buffer at that position
          int bx = x * 16 + i;
          int by = y * 16 + j;
          int bz = z * 16 + k;
          vec3 face = {bx, by, bz};
          vec4 viewSpace = matvec3(viewMat, face);
          float depth = -viewSpace.v[2];
          point screenSpace = viewport(vshade(face));
          //clamp screenspace
          if(screenSpace.x < 0)
            screenSpace.x = 0;
          if(screenSpace.x >= 320)
            screenSpace.x = 319;
          if(screenSpace.y < 0)
            screenSpace.y = 0;
          if(screenSpace.y >= 200)
            screenSpace.y = 199;
          //query depth buffer
          byte existingDepth = depthBuf[screenSpace.x + screenSpace.y * 320];
          if(existingDepth > depth)
          {
            //draw the face
            glDepth(depth);
            glVertex3f(bx + 1, by, bz);
            glVertex3f(bx + 1, by + 1, bz);
            glVertex3f(bx + 1, by + 1, bz + 1);
            glVertex3f(bx + 1, by, bz + 1);
          }
        }
      }
    }
  }
  glEnd();
}

byte getBlock(int x, int y, int z)
{
  return getBlockC(&chunks[(x / 16) + 4 * (y / 16) + 16 * (z / 16)], x % 16, y % 16, z % 16);
}

void setBlock(byte b, int cx, int cy, int cz)
{
  setBlockC(b, &chunks[(cx / 16) + chunksX * (cy / 16) + chunksX * chunksY * (cz / 16)], cx % 16, cy % 16, cz % 16);
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
        setBlock(STONE, x, y, z);
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

