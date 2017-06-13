#include "oc.h"

//Whether to noclip (no gravity, collisions or world boundaries)
//#define NOCLIP

//Seed (TODO: configure or randomize w/ time())
#define SEED 12

//Have INV_W x INV_H inventory grid
//Eventually, have at least a 4x4x4 chunks (64^3 blocks) world
static Chunk* chunks;
//Player position
vec3 player;
//Player velocity
static vec3 vel;
//Whether view mat must be updated this frame
static bool viewStale = true;
//Whether player is standing on the ground
static bool onGround = false;
static float yaw;     //yaw (left-right), radians, left is increasing
static float pitch;   //pitch (up-down), radians, ahead is 0, up is positive
static int breakFrames; //how many frames player has been breaking (breakX, breakY, breakZ)
static int breakX;
static int breakY;
static int breakZ;
static bool invView;
vec3 lookdir;
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
   GLASS (11)    //Transparent but with white edges
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
  {0x42, 0x06, 0x73, 0x42, 0x71},     //Log
  {0x02, 0x79, 0x77, 0xC0, 0xBF},     //Leaf
  {0x37, 0x21, 0x21, 0x01, 0x68},     //Water
  {0x44, 0x45, 0x8C, 0x8B, 0xD3},     //Sand
  {0x0F, 0x0F, 0x0F, 0x0F, 0x0F},     //Glass (colorless)
  {0x43, 0x42, 0x06, 0x73, 0x72},     //Chest
  {0x42, 0x41, 0x8B, 0x8A, 0x89},     //Granite
  {0x1F, 0x1F, 0x1E, 0x1E, 0x1D},     //Quartz
  {0, 0, 0, 0, 0}                     //Bedrock
};

//Sky color
static byte sky = 0x35;

static bool wkey; //move forward
static bool akey; //move left
static bool skey; //move backward
static bool dkey; //move right
static bool ikey; //look up
static bool jkey; //look left
static bool kkey; //look down
static bool lkey; //look right
static bool rkey; //break block
static bool fkey; //place block

#define chunksX 5
#define chunksY 5
#define chunksZ 5

static void pumpEvents();
static void terrainGen();
static void initChunks();
static void updatePhysics();
static void updateViewMat();
static bool getTargetBlock(int* bx, int* by, int* bz, int* px, int* py, int* pz, bool* canPlace);
static void drawInv();

//player movement configuration
#define PLAYER_SPEED 0.15         //horizontal movement speed
#define JUMP_SPEED 0.5            //vertical takeoff speed of jump
#define GRAVITY 0.06              //gravitational acceleration (blocks per frame per frame)
#define TERMINAL_VELOCITY 100
#define PLAYER_HEIGHT 1.8
#define PLAYER_EYE 1.5
#define PLAYER_WIDTH 0.8
#define PLAYER_REACH 5            //how far away the player can place and destroy blocks
#define BREAK_TIME 18             //how many frames it takes to break a block
#define X_SENSITIVITY 0.11
#define Y_SENSITIVITY 0.11

//Inventory
#define INV_W 9
#define INV_H 4
static Stack inv[INV_W * INV_H];

//3D configuration
#define NEAR 0.1f
#define FAR 50.0f
#define FOV 80.0f

inline bool isTransparent(byte b)
{
  return b == AIR || b == GLASS;
}

inline bool blockInBounds(int x, int y, int z)
{
  return x >= 0 && x < chunksX * 16 && y >= 0 && y < chunksY * 16 && z >= 0 && z < chunksZ * 16;
}

Chunk* getChunk(int x, int y, int z)
{
  if(x < 0 || y < 0 || z < 0 || x >= chunksX || y >= chunksY || z >= chunksZ)
  {
    printf("Request chunk that is out-of-bounds: %i %i %i\n", x, y, z);
    while(1);
  }
  return &chunks[x + y * chunksX + z * chunksX * chunksY];
}

//any block outside of the world is treated as air
byte getBlock(int x, int y, int z)
{
  if(blockInBounds(x, y, z))
    return getBlockC(getChunk(x / 16, y / 16, z / 16), x % 16, y % 16, z % 16);
  return AIR;
}

void setBlock(byte b, int x, int y, int z)
{
  if(blockInBounds(x, y, z))
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

static void setVisible(bool value, int x, int y, int z)
{
  if(!blockInBounds(x, y, z))
    return;
  Chunk* c = getChunk(x / 16, y / 16, z / 16);
  x %= 16;
  y %= 16;
  z %= 16;
  int ind = x + y * 16 + z * 256;
  byte b = c->visible[ind >> 3];
  b &= ~(1 << (ind & 0x7));
  if(value)
    b |= (1 << (ind & 0x7));
  c->visible[ind >> 3] = b;
}

static void updateVisible(int x, int y, int z)
{
  //a block is visible if and only if it is not air AND at least one neighbor is transparent
  byte val = getBlock(x, y, z) != AIR && (
      isTransparent(getBlock(x - 1, y, z)) ||
      isTransparent(getBlock(x + 1, y, z)) ||
      isTransparent(getBlock(x, y - 1, z)) ||
      isTransparent(getBlock(x, y + 1, z)) ||
      isTransparent(getBlock(x, y, z - 1)) ||
      isTransparent(getBlock(x, y, z + 1)));
  setVisible(val, x, y, z);
}

static void breakBlock(int x, int y, int z)
{
  int cx = x / 16;
  int cy = y / 16;
  int cz = z / 16;
  Chunk* chunk = getChunk(cx, cy, cz);
  //can't break bedrock
  if(getBlockC(chunk, x % 16, y % 16, z % 16) == BEDROCK)
    return;
  //set the block in chunk to air
  setBlockC(AIR, chunk, x % 16, y % 16, z % 16);
  setVisible(false, x, y, z);
  //set all non-air neighbors to visible
  if(getBlock(x - 1, y, z))
    setVisible(true, x - 1, y, z);
  if(getBlock(x + 1, y, z))
    setVisible(true, x + 1, y, z);
  if(getBlock(x, y - 1, z))
    setVisible(true, x, y - 1, z);
  if(getBlock(x, y + 1, z))
    setVisible(true, x, y + 1, z);
  if(getBlock(x, y, z - 1))
    setVisible(true, x, y, z - 1);
  if(getBlock(x, y, z + 1))
    setVisible(true, x, y, z + 1);
  chunk->filled--;
}

static void placeBlock(byte b, int x, int y, int z)
{
  int cx = x / 16;
  int cy = y / 16;
  int cz = z / 16;
  Chunk* chunk = getChunk(cx, cy, cz);
  setBlockC(b, chunk, x % 16, y % 16, z % 16);
  //set new block to visible
  setVisible(true, x, y, z);
  chunk->filled++;
  //if block is opaque, check if neighbors are now hidden
  if(!isTransparent(b))
  {
    if(!getBlock(x - 1, y, z))
      updateVisible(x - 1, y, z);
    if(!getBlock(x + 1, y, z))
      updateVisible(x + 1, y, z);
    if(!getBlock(x, y - 1, z))
      updateVisible(x, y - 1, z);
    if(!getBlock(x, y + 1, z))
      updateVisible(x, y + 1, z);
    if(!getBlock(x, y, z - 1))
      updateVisible(x, y, z - 1);
    if(!getBlock(x, y, z + 1))
      updateVisible(x, y, z + 1);
  }
}

//Draw a cubic shell of chunks, centered at x,y,z and with radius rad
//rad = 0: just render chunk x,y,z
static void renderShell(int rad, int x, int y, int z)
{
  //draw front and back
  for(int i = x - rad; i <= x + rad; i++)
  {
    for(int j = y - rad; j <= y + rad; j++)
    {
      renderChunk(i, j, z + rad);
      if(rad != 0)
        renderChunk(i, j, z - rad);
    }
  }
  //draw left and right
  for(int i = z - rad + 1; i <= z + rad - 1; i++)
  {
    for(int j = y - rad; j <= y + rad; j++)
    {
      renderChunk(x - rad, j, i);
      renderChunk(x + rad, j, i);
    }
  }
  //draw top and bottom
  for(int i = x - rad + 1; i <= x + rad - 1; i++)
  {
    for(int j = z - rad; j <= z + rad; j++)
    {
      renderChunk(i, y + rad, j);
      renderChunk(i, y - rad, j);
    }
  }
}

void ocMain()
{
  chunks = malloc(chunksX * chunksY * chunksZ * sizeof(Chunk));
  invView = false;
  terrainGen();
  initChunks();
  yaw = 0;
  pitch = 0;
  //spawn player in horizontal center of world, at top
  player.v[0] = chunksX * 16 / 2;
  player.v[1] = chunksY * 16;
  player.v[2] = chunksZ * 16 / 2;
  onGround = false;
  vel.v[0] = 0;
  vel.v[1] = 0;
  vel.v[2] = 0;
  wkey = false;
  akey = false;
  skey = false;
  dkey = false;
  rkey = false;
  fkey = false;
  setModel(identity());
  setProj(FOV, NEAR, FAR);
  breakFrames = 0;
  for(int i = 0; i < INV_W * INV_H; i++)
  {
    //stacks of AIR are considered empty/clear
    if(rand() % 2)
      inv[i].item = 1 + rand() % 15;
    inv[i].count = rand() % 256;
  }
  while(1)
  {
    //measure time spent in frame to synchronize at end
    clock_t cstart = clock();
    pumpEvents();
    if(invView)
    {
      drawInv();
    }
    else
    {
      updatePhysics();
      int targx = breakX;
      int targy = breakY;
      int targz = breakZ;
      int placex = -1;
      int placey = -1;
      int placez = -1;
      if(viewStale)
      {
        updateViewMat();
        viewStale = false;
        //view updated, now check for block target
      }
      bool hit = false;
      bool canPlace = false;
      hit = getTargetBlock(&targx, &targy, &targz, &placex, &placey, &placez, &canPlace);
      //if(!rkey || !hit || (breakFrames > 0 && (targx != breakX || targy != breakY || targz != breakZ)))
      if(!rkey || !hit)
      {
        //cancel breaking block if it was previously happening
        breakFrames = 0;
      }
      else if(rkey && hit)
      {
        //start breaking block currently pointed at
        breakX = targx;
        breakY = targy;
        breakZ = targz;
        breakFrames++;
        if(breakFrames == BREAK_TIME)
        {
          breakBlock(targx, targy, targz);
          breakFrames = 0;
        }
      }
      if(hit && fkey && canPlace)
      {
        //only out-of-bounds block player can reach is the top of world
        if(placey < chunksY * 16)
          placeBlock(DIAMOND, placex, placey, placez);
        //only place block on rising edge of f
        fkey = false;
      }
      glClear(sky);
      glEnableDepthTest(true);
      //fill depth buf with maximum depth (255)
      memset(depthBuf, 0xFF, 64000);
      //get chunk containing player
      //fill cubic shells starting at player position and moving out
      //this maximizes the chunks that are occlusion culled
      for(int rad = 0; rad < (FAR + 15) / 16; rad++)
      {
        renderShell(rad, player.v[0] / 16, player.v[1] / 16, player.v[2] / 16);
      }
      glEnableDepthTest(false);
      glColor1i(0xF);
      fillRect(160, 96, 1, 9);
      fillRect(156, 100, 9, 1);
    }
    glFlush();
    //hit 30 fps (if there is spare time this frame)
    sleepMS(34 - (clock() - cstart));
  }
}

static byte getDepth(float viewZ)
{
  viewZ += 1;
  byte d;
  if(viewZ <= 8)
    d = viewZ * 12;
  else
    d = 96 + (viewZ - 8) * 3.8;
  return d;
}

//Returns true if a cuboid (world space) is currently invisible to the player
//  Uses frustum culling and occlusion culling
//also if minDepth, sets *minDepth to the minimum depth of the eight corners (in view space)
//  minDepth can be used as the depth value for drawing the cuboid
static bool cullCuboid(float xlo, float xhi, float ylo, float yhi, float zlo, float zhi, byte* minDepth)
{
  //allow minDepth to be NULL, but still compute the depth value for local use
  byte temp;
  if(!minDepth)
    minDepth = &temp;
  vec3 cube[8] = {
    {{xlo, ylo, zlo}},
    {{xhi, ylo, zlo}},
    {{xlo, yhi, zlo}},
    {{xhi, yhi, zlo}},
    {{xlo, ylo, zhi}},
    {{xhi, ylo, zhi}},
    {{xlo, yhi, zhi}},
    {{xhi, yhi, zhi}}
  };
  //take the best clip coordinate (min absolute value) of each dimension
  int clippedNear = 0;
  int clippedFar = 0;
  int clippedLeft = 0;
  int clippedRight = 0;
  int clippedTop = 0;
  int clippedBottom = 0;
  float blockDepth = 255;
  vec4 clip[8];
  for(int i = 0; i < 8; i++)
  {
    vec4 view = matvec3(viewMat, cube[i]);
    clip[i] = matvec4(projMat, view);
    float w = clip[i].v[3];
    if(clip[i].v[2] > w)
    {
      clippedFar++;
      break;
    }
    if(clip[i].v[0] < -w)
      clippedLeft++;
    if(clip[i].v[0] > w)
      clippedRight++;
    if(clip[i].v[1] < -w)
      clippedBottom++;
    if(clip[i].v[1] > w)
      clippedTop++;
    if(clip[i].v[2] < -w)
      clippedNear++;
    blockDepth = min(blockDepth, -view.v[2]);
  }
  *minDepth = getDepth(blockDepth);
  //If block has any vertices past far plane, skip it entirely
  //Otherwise, cube is fully invisible if (iff?) all 8 corners are clipped by the same frustum plane
  if(clippedFar > 0 || clippedLeft == 8 || clippedRight == 8 || clippedTop == 8 || clippedBottom == 8 || clippedNear == 8)
    return true;
  //Now try occlusion culling
  //Already have clip coordinates, so convert to viewport and get upper bounds on x,y
  int minx = 320;
  int maxx = 0;
  int miny = 200;
  int maxy = 0;
  for(int i = 0; i < 8; i++)
  {
    vec3 ndc = vecscale(toVec3(clip[i]), 1.0f / clip[i].v[3]);
    point p = viewport(ndc);
    if(p.x < minx)
      minx = p.x;
    if(p.x > maxx)
      maxx = p.x;
    if(p.y < miny)
      miny = p.y;
    if(p.y > maxy)
      maxy = p.y;
  }
  //clamp bounding box to screen
  if(minx < 0)
    minx = 0;
  if(maxx > 320)
    maxx = 320;
  if(miny < 0)
    miny = 0;
  if(maxy > 200)
    maxy = 200;
  //now compare depth values in buffer to computed depth value
  //note: max is an exclusive upper bound
  int stride = 320 - (maxx - minx);
  int ind = minx + miny * 320;
  bool allBehind = true;
  for(int y = miny; y < maxy; y++)
  {
    for(int x = minx; x < maxx; x++)
    {
      if(depthBuf[ind] > *minDepth)
      {
        allBehind = false;
        break;
      }
      ind++;
    }
    ind += stride;
  }
  if(allBehind)
    return true;
  return false;
}

void renderChunk(int x, int y, int z)
{
  //bounds check
  if(x < 0 || x >= chunksX || y < 0 || y >= chunksY || z < 0 || z >= chunksZ)
    return;
  //frustum cull whole chunk if possible
  Chunk* c = getChunk(x, y, z);
  if(c->filled == 0)
  {
    //chunk is completely empty
    return;
  }
  //chunk offset position
  int cx = x * 16;
  int cy = y * 16;
  int cz = z * 16;
  if(cullCuboid(cx, cx + 16, cy, cy + 16, cz, cz + 16, NULL))
    return;
  glBegin(GL_QUADS);
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
        byte blockDepth;
        if(cullCuboid(bx, bx + 1, by, by + 1, bz, bz + 1, &blockDepth))
          continue;
        glDepth(blockDepth);
        if(block == GLASS)
          glDrawMode(DRAW_WIREFRAME);
        else
          glDrawMode(DRAW_FILL);
        glColor1i(blockColor[block][1]);
        if(player.v[0] < bx)
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
        if(player.v[0] > bx + 1)
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
        if(player.v[1] < by)
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
        if(player.v[1] > by + 1)
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
        if(player.v[2] < bz)
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
        if(player.v[2] > bz + 1)
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

static void drawInv()
{
  glEnableDepthTest(false);
  const int cellSize = 28;
  const int gridw = INV_W;    //configured at top
  const int gridh = INV_H;
  int gridx = (320 - cellSize * gridw) / 2;
  int gridy = (200 - cellSize * gridh) / 2;
  glColor1i(0x1D);
  fillRect(gridx, gridy, gridw * cellSize, gridh * cellSize);
  for(int i = 0; i < gridw; i++)
  {
    for(int j = 0; j < gridh; j++)
    {
      glColor1i(0x19);
      drawRect(gridx + i * cellSize, gridy + j * cellSize, cellSize, cellSize);
      Stack* stack = inv + (i + j * INV_W);
      if(stack->item != AIR && stack->count != 0)
      {
        byte bg = 0x1D;
        if(stack->item != GLASS)
        {
          bg = blockColor[stack->item][1];
          glColor1i(bg);
          fillRect(gridx + i * cellSize + 4, gridy + j * cellSize + 4, cellSize - 9, cellSize - 9);
        }
        glColor1i(0x19);
        char buf[8];
        sprintf(buf, "%i", stack->count);
        glText(buf, gridx + i * cellSize + 1, gridy + j * cellSize + (cellSize - 8), 0x1D);
      }
    }
  }
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
            case KEY_R:
              rkey = k.pressed; break;
            case KEY_F:
              fkey = k.pressed; break;
            case KEY_E:
              if(k.pressed)
                invView = !invView; break;
            case KEY_SPACE:
              if(k.pressed && onGround)
              {
                onGround = false;
                vel.v[1] += JUMP_SPEED;
              }
              break;
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
void srandBlockHash(int x, int y, int z, int octave)
{
  srand(SEED ^ (4 * (x + y * (chunksX * 16 + 1) + z * (chunksX * 16 * chunksY * 16 + 1)) + octave));
}

void printBlockAbundance()
{
  int counts[16];
  memset(counts, 0, 16 * sizeof(int));
  for(int i = 0; i < chunksX * chunksY * chunksZ; i++)
  {
    Chunk* chunk = &chunks[i];
    for(int j = 0; j < 2048; j++)
    {
      counts[chunk->vals[j] & 0xF]++;
      counts[chunk->vals[j] >> 4]++;
    }
  }
  float total = chunksX * chunksY * chunksZ * 4096;
  for(int i = 0; i < 16; i++)
  {
    printf("World is %.2f%% block %i\n", 100.0f * counts[i] / total, i);
  }
}

//replace all "replace" blocks with "with", in ellipsoidal region
void replaceEllipsoid(byte replace, byte with, int x, int y, int z, float rx, float ry, float rz)
{
  for(int lx = x - rx; lx <= x + rx + 1; lx++)
  {
    for(int ly = y - ry; ly <= y + ry + 1; ly++)
    {
      for(int lz = z - rz; lz <= z + rz + 1; lz++)
      {
        if(!blockInBounds(lx, ly, lz))
          continue;
        if(getBlock(lx, ly, lz) != replace)
          continue;
        //compute weighted distance squared from ellipsoid center to block center
        float dx = lx - x;
        float dy = ly - y;
        float dz = lz - z;
        float distSq = (dx * dx / (rx * rx)) + (dy * dy / (ry * ry)) + (dz * dz / (rz * rz));
        if(distSq > 1)
          continue;
        setBlock(with, lx, ly, lz);
      }
    }
  }
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
    memset(chunk, 0x44, sizeof(Chunk));
    for(int octave = 2; octave < 4; octave++)
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
              val += 0.5 + v / (period * period * period);
              if(val > 15)
                val = 15;
              setBlock(val, bx + x, by + y, bz + z);
            }
          }
        }
      }
    }
  }
  //add a small adjustment value that decreases with altitude
  //underground should be mostly solid, above sea level should be mostly empty
  for(int x = 0; x < wx; x++)
  {
    for(int y = 0; y < wy; y++)
    {
      for(int z = 0; z < wz; z++)
      {
        float shift = wy / 2 - y;
        if(y > wy / 2)
          shift = 1 + shift * 0.1;
        else
          shift = shift * 0.7;
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
  //run a few sweeps of a smoothing function
  //basically gaussian blur, but in-place
  for(int sweep = 0; sweep < 8; sweep++)
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
          int samples = 0;
          for(int tx = -1; tx <= 1; tx++)
          {
            for(int ty = -1; ty <= 1; ty++)
            {
              for(int tz = -1; tz <= 1; tz++)
              {
                if(blockInBounds(tx, ty, tz))
                {
                  neighborVals += getBlock(x + tx, y + ty, z + tz);
                  samples++;
                }
              }
            }
          }
          //get sample value as rounded-to-nearest average of samples
          //kill tiny floating islands
          if(samples < 5)
            neighborVals = 0;
          neighborVals = (neighborVals + samples / 2) / samples;
          if(neighborVals >= 8 + rand() % 2)
            setBlock(13, x, y, z);
          else
            setBlock(4, x, y, z);
        }
      }
    }
  }
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
  //set all air blocks below sea level to water
  for(int x = 0; x < wx; x++)
  {
    for(int y = 0; y < wy / 2; y++)
    {
      for(int z = 0; z < wz; z++)
      {
        if(getBlock(x, y, z) == AIR)
        {
          setBlock(WATER, x, y, z);
        }
      }
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
  //set all solid blocks near water to sand
  for(int x = 0; x < wx; x++)
  {
    for(int y = 0; y < wy; y++)
    {
      for(int z = 0; z < wz; z++)
      {
        if(getBlock(x, y, z) != AIR && getBlock(x, y, z) != WATER)
        {
          //look around a 3x1x3 region for water blocks
          bool nearWater = false;
          for(int i = -2; i <= 2; i++)
          {
            for(int k = -2; k <= 2; k++)
            {
              nearWater |= (getBlock(x + i, y, z + k) == WATER);
            }
          }
          if(nearWater)
          {
            setBlock(SAND, x, y, z);
          }
        }
      }
    }
  }
  //replace some stone with ores
  //note: veins attribute is average veins per chunk in the depth range
  //configuration:
#define GRANITE_MIN_SIZE 6
#define GRANITE_MAX_SIZE 12
#define GRANITE_MIN_DEPTH wy
#define GRANITE_VEINS 0.1
#define QUARTZ_MIN_SIZE 5
#define QUARTZ_MAX_SIZE 10
#define QUARTZ_MIN_DEPTH wy / 2
#define QUARTZ_VEINS 0.1
#define COAL_MIN_SIZE 3
#define COAL_MAX_SIZE 6
#define COAL_MIN_DEPTH wy
#define COAL_VEINS 0.1
#define IRON_MIN_SIZE 2
#define IRON_MAX_SIZE 4
#define IRON_MIN_DEPTH wy / 2
#define IRON_VEINS 0.3
#define GOLD_MIN_SIZE 2
#define GOLD_MAX_SIZE 3
#define GOLD_MIN_DEPTH wy / 3
#define GOLD_VEINS 0.2
#define DIAMOND_MIN_SIZE 1
#define DIAMOND_MAX_SIZE 3
#define DIAMOND_MIN_DEPTH wy / 5
#define DIAMOND_VEINS 0.1
#define GEN_VEINS(block, minsize, maxsize, mindepth, veins) \
  { \
    float freq = veins * ((float) mindepth / wy); \
    for(int v = 0; v < chunksX * chunksY * chunksZ * freq; v++) \
    { \
      int x = rand() % wx; \
      int y = rand() % mindepth; \
      int z = rand() % wz; \
      int rx = minsize + rand() % (maxsize - minsize + 1); \
      int ry = minsize + rand() % (maxsize - minsize + 1); \
      int rz = minsize + rand() % (maxsize - minsize + 1); \
      replaceEllipsoid(STONE, block, x, y, z, rx, ry, rz); \
    } \
  }
  GEN_VEINS(GRANITE, GRANITE_MIN_SIZE, GRANITE_MAX_SIZE, GRANITE_MIN_DEPTH, GRANITE_VEINS);
  GEN_VEINS(QUARTZ, QUARTZ_MIN_SIZE, QUARTZ_MAX_SIZE, QUARTZ_MIN_DEPTH, QUARTZ_VEINS);
  GEN_VEINS(COAL, COAL_MIN_SIZE, COAL_MAX_SIZE, COAL_MIN_DEPTH, COAL_VEINS);
  GEN_VEINS(IRON, IRON_MIN_SIZE, IRON_MAX_SIZE, IRON_MIN_DEPTH, IRON_VEINS);
  GEN_VEINS(GOLD, GOLD_MIN_SIZE, GOLD_MAX_SIZE, GOLD_MIN_DEPTH, GOLD_VEINS);
  GEN_VEINS(DIAMOND, DIAMOND_MIN_SIZE, DIAMOND_MAX_SIZE, DIAMOND_MIN_DEPTH, DIAMOND_VEINS);
  //find random places on the surface to plant trees
  for(int tree = 0; tree < chunksX * chunksZ; tree++)
  {
    //determine if the highest block here is dirt
    int x = rand() % wx;
    int z = rand() % wz;
    bool hitDirt = false;
    int y;
    for(y = wy - 1; y >= 1; y--)
    {
      byte b = getBlock(x, y, z);
      if(b == DIRT)
        hitDirt = true;
      if(b != AIR)
        break;
    }
    if(!hitDirt)
    {
      continue;
    }
    //try to plant the tree on dirt block @ (x, y, z)
    int treeHeight = 4 + rand() % 4;
    for(int i = 0; i < treeHeight; i++)
    {
      setBlock(LOG, x, y + 1 + i, z);
    }
    //fill in vertical ellipsoid of leaves around the trunk
    //cover the top 2/3 of trunk, and extend another 1/3 above it
    //have x/z radius be half the y radius
    //loop over bounding box of the leaves
    float ellY = y + 1 + (5.0 / 6.0) * treeHeight;
    int ry = 2 * treeHeight / 3;
    float rxz = 2 * ry / 3;
    replaceEllipsoid(AIR, LEAF, x, y + 1 + 0.833 * treeHeight, z, treeHeight * 0.4, treeHeight * 0.5, treeHeight * 0.4);
  }
}

void initChunks()
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
                  setVisible(true, bx, by, bz);
                }
              }
            }
          }
        }
      }
    }
  }
}

typedef struct
{
  float x;
  float y;
  float z;
  float l;  //x size
  float h;  //y size
  float w;  //z size
} Hitbox;

//6 directions: plus/minus x/y/z
enum
{
  HB_MX,
  HB_PX,
  HB_MY,
  HB_PY,
  HB_MZ,
  HB_PZ
};

//Attempt to move a hitbox the given distance in the given cardinal direction
//Return true iff a collision occurred during this movement
//Resulting position stored in *hb
static bool moveHitbox(Hitbox* hb, int dir, float d)
{
  //make sure d is positive
  if(d < 0)
  {
    dir--;
    d = -d;
  }
  if(d > 0.5)
  {
    bool hit = moveHitbox(hb, dir, d - 0.5);
    if(hit)
      return true;
    d = 0.5;
  }
  switch(dir)
  {
    case HB_MX:
      hb->x -= d;
      break;
    case HB_PX:
      hb->x += d;
      break;
    case HB_MY:
      hb->y -= d;
      break;
    case HB_PY:
      hb->y += d;
      break;
    case HB_MZ:
      hb->z -= d;
      break;
    case HB_PZ:
      hb->z += d;
      break;
    default:;
  }
  //scan for collision between hitbox and blocks
  bool collide = false;
  for(int bx = hb->x; bx <= hb->x + hb->l; bx++)
  {
    for(int by = hb->y; by <= hb->y + hb->h; by++)
    {
      for(int bz = hb->z; bz <= hb->z + hb->w; bz++)
      {
        if(getBlock(bx, by, bz))
        {
          collide = true;
        }
      }
    }
  }
#ifdef NOCLIP
  collide = false;
#endif
  if(collide)
  {
    float eps = 1e-5;
    //depending on movement direction, nudge in opposite direction so not colliding
    switch(dir)
    {
      case HB_MX:
        hb->x = ((int) hb->x) + 1 + eps;
        break;
      case HB_PX:
        hb->x = ((int) (hb->x + hb->l)) - hb->l - eps;
        break;
      case HB_MY:
        hb->y = ((int) hb->y) + 1 + eps;
        break;
      case HB_PY:
        hb->y = ((int) (hb->y + hb->h)) - hb->h - eps;
        break;
      case HB_MZ:
        hb->z = ((int) hb->z) + 1 + eps;
        break;
      case HB_PZ:
        hb->z = ((int) (hb->z + hb->w)) - hb->w - eps;
        break;
      default:;
    }
  }
  return collide;
}

static void updatePhysics()
{
  //handle orientation changes via ijkl
  const float pitchLimit = 90 / (180.0f / PI);
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
  vel.v[0] = 0;
  vel.v[2] = 0;
  if(xvel)
  {
    vel.v[0] += xvel * PLAYER_SPEED * cosf(yaw);
    vel.v[2] += xvel * PLAYER_SPEED * sinf(yaw);
  }
  if(yvel)
  {
    vel.v[0] += yvel * PLAYER_SPEED * sinf(-yaw);
    vel.v[2] += yvel * PLAYER_SPEED * cosf(-yaw);
  }
  if(dx || dy)
  {
    //player look direction changed, so view matrix must be updated
    viewStale = true;
  }
  vec3 old = player;
  Hitbox hb = {player.v[0] - PLAYER_WIDTH / 2, player.v[1] - PLAYER_EYE, player.v[2] - PLAYER_WIDTH / 2, PLAYER_WIDTH, PLAYER_HEIGHT, PLAYER_WIDTH};
  //gravitational acceleration
#ifndef NOCLIP
  if(!onGround)
  {
    vel.v[1] -= GRAVITY;
    if(vel.v[1] < -TERMINAL_VELOCITY)
      vel.v[1] = -TERMINAL_VELOCITY;
  }
  if(vel.v[1] > 0)
  {
    bool hit = moveHitbox(&hb, HB_PY, vel.v[1]);
    if(hit)
      vel.v[1] = 0;
  }
  else if(vel.v[1] < 0)
  {
    bool hit = moveHitbox(&hb, HB_MY, -vel.v[1]);
    if(hit)
    {
      vel.v[1] = 0;
      onGround = true;
    }
  }
#endif
  //move horizontally
  moveHitbox(&hb, vel.v[0] > 0 ? HB_PX : HB_MX, fabsf(vel.v[0]));
  moveHitbox(&hb, vel.v[2] > 0 ? HB_PZ : HB_MZ, fabsf(vel.v[2]));
  //check if player is suspended in the air (clear onGround if so)
  if(onGround)
  {
    if(!moveHitbox(&hb, HB_MY, 0.001))
      onGround = false;
  }
#ifndef NOCLIP
  //clamp player to world boundaries (can't fall out of world)
  if(hb.x < 0)
    hb.x = 0;
  if(hb.x > chunksX * 16 - PLAYER_WIDTH)
    hb.x = chunksX * 16 - PLAYER_WIDTH;
  //bedrock is at y = 1, so clamp there
  if(hb.y < 1)
    hb.y = 1;
  //don't need to clamp on sky limit, because gravity will always bring player back
  if(hb.z < 0)
    hb.z = 0;
  if(hb.z > chunksZ * 16 - PLAYER_WIDTH)
    hb.z = chunksZ * 16 - PLAYER_WIDTH;
#endif
  //copy position back to player vector
  player.v[0] = hb.x + PLAYER_WIDTH / 2;
  player.v[1] = hb.y + PLAYER_EYE;
  player.v[2] = hb.z + PLAYER_WIDTH / 2;
  //test whether player position actually changed this update
  if(memcmp(&old, &player, 3 * sizeof(float)))
    viewStale = true;
}

static void updateViewMat()
{
  lookdir = ((vec3) {cosf(pitch) * cosf(yaw), sinf(pitch), cosf(pitch) * sinf(yaw)});
  vec3 right = {-sinf(yaw), 0, cosf(yaw)};
  vec3 up = cross(right, lookdir);
  vec3 target = vecadd(player, lookdir);
  setView(lookAt(player, target, up));
}

//returns true if the player is looking at a block within reach
//*bx, *by, *bz is set to the block that can be broken
//*px, *py, *pz is set to position where player can place a block
static bool getTargetBlock(int* bx, int* by, int* bz, int* px, int* py, int* pz, bool* canPlace)
{
  vec3 camPos = player;
  vec3 camDir = lookdir;
  vec3 blockIter = {{(int) camPos.v[0], (int) camPos.v[1], (int) camPos.v[2]}};
  //iterate through blocks, finding the faces that player is looking through
  while(true)
  {
    vec3 nextBlock = blockIter;
    bool haveNext = false;
    if(camDir.v[0] > 0)
    {
      //does camPos + t * camDir pass through the +x face?
      //note: +x face is at blockIter.x + 1
      //just solve for y, z at the point the ray gets to blockIter.x + 1
      vec3 intersect = vecadd(camPos, vecscale(camDir, ((blockIter.v[0] + 1 - camPos.v[0]) / camDir.v[0])));
      if(intersect.v[1] >= blockIter.v[1] && intersect.v[1] < blockIter.v[1] + 1 &&
         intersect.v[2] >= blockIter.v[2] && intersect.v[2] < blockIter.v[2] + 1)
      {
        nextBlock.v[0] += 1;
        haveNext = true;
      }
    }
    if(camDir.v[0] < 0 && !haveNext)
    {
      //-x face
      vec3 intersect = vecadd(camPos, vecscale(camDir, ((blockIter.v[0] - camPos.v[0]) / camDir.v[0])));
      if(intersect.v[1] >= blockIter.v[1] && intersect.v[1] < blockIter.v[1] + 1 &&
         intersect.v[2] >= blockIter.v[2] && intersect.v[2] < blockIter.v[2] + 1)
      {
        nextBlock.v[0] -= 1;
        haveNext = true;
      }
    }
    if(camDir.v[1] > 0 && !haveNext)
    {
      //+y face
      vec3 intersect = vecadd(camPos, vecscale(camDir, (blockIter.v[1] + 1 - camPos.v[1]) / camDir.v[1]));
      if(intersect.v[0] >= blockIter.v[0] && intersect.v[0] < blockIter.v[0] + 1 &&
         intersect.v[2] >= blockIter.v[2] && intersect.v[2] < blockIter.v[2] + 1)
      {
        nextBlock.v[1] += 1;
        haveNext = true;
      }
    }
    if(camDir.v[1] < 0 && !haveNext)
    {
      //-y face
      vec3 intersect = vecadd(camPos, vecscale(camDir, ((blockIter.v[1] - camPos.v[1]) / camDir.v[1])));
      if(intersect.v[0] >= blockIter.v[0] && intersect.v[0] < blockIter.v[0] + 1 &&
         intersect.v[2] >= blockIter.v[2] && intersect.v[2] < blockIter.v[2] + 1)
      {
        nextBlock.v[1] -= 1;
        haveNext = true;
      }
    }
    if(camDir.v[2] > 0 && !haveNext)
    {
      //+z face
      vec3 intersect = vecadd(camPos, vecscale(camDir, ((blockIter.v[2] + 1 - camPos.v[2]) / camDir.v[2])));
      if(intersect.v[0] >= blockIter.v[0] && intersect.v[0] < blockIter.v[0] + 1 &&
         intersect.v[1] >= blockIter.v[1] && intersect.v[1] < blockIter.v[1] + 1)
      {
        nextBlock.v[2] += 1;
        haveNext = true;
      }
    }
    if(camDir.v[2] < 0 && !haveNext)
    {
      //-z face
      vec3 intersect = vecadd(camPos, vecscale(camDir, ((blockIter.v[2] - camPos.v[2]) / camDir.v[2])));
      if(intersect.v[0] >= blockIter.v[0] && intersect.v[0] < blockIter.v[0] + 1 &&
         intersect.v[1] >= blockIter.v[1] && intersect.v[1] < blockIter.v[1] + 1)
      {
        nextBlock.v[2] -= 1;
        haveNext = true;
      }
    }
    if(!haveNext)
    {
      return false;
    }
    if(getBlock(nextBlock.v[0], nextBlock.v[1], nextBlock.v[2]))
    {
      //ray ended in a solid block
      //go through all blocks that player fully or partially occupies, and if any of them match, return false
      //otherwise set output parameters to block coords and return true
      int xlo = floor(player.v[0] - PLAYER_WIDTH / 2);
      int xhi = ceil(player.v[0] + PLAYER_WIDTH / 2);
      int ylo = floor(player.v[1] - PLAYER_EYE);
      int yhi = ceil(player.v[1] + PLAYER_HEIGHT - PLAYER_EYE);
      int zlo = floor(player.v[2] - PLAYER_WIDTH / 2);
      int zhi = ceil(player.v[2] + PLAYER_WIDTH / 2);
      if(xlo <= blockIter.v[0] && blockIter.v[0] <= xhi &&
         ylo <= blockIter.v[1] && blockIter.v[1] <= yhi &&
         zlo <= blockIter.v[2] && blockIter.v[2] <= zhi)
      {
        *canPlace = false;
      }
      else
      {
        *canPlace = true;
      }
      *px = blockIter.v[0];
      *py = blockIter.v[1];
      *pz = blockIter.v[2];
      *bx = nextBlock.v[0];
      *by = nextBlock.v[1];
      *bz = nextBlock.v[2];
      return true;
    }
    if(((nextBlock.v[0] - camPos.v[0]) * (nextBlock.v[0] - camPos.v[0]) +
          (nextBlock.v[1] - camPos.v[1]) * (nextBlock.v[1] - camPos.v[1]) +
          (nextBlock.v[2] - camPos.v[2]) * (nextBlock.v[2] - camPos.v[2])) > (PLAYER_REACH * PLAYER_REACH))
    {
      //no block in reach
      return false;
    }
    blockIter = nextBlock;
  }
}
