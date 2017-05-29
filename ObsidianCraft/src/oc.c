#include "oc.h"

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
{0x37, 0x21, 0x21, 0x01, 0x68},     //Water
{0x44, 0x45, 0x8C, 0x8B, 0xD3},     //Sand
{0x0F, 0x0F, 0x0F, 0x0F, 0x0F},                    //Glass (colorless)
{0x43, 0x42, 0x06, 0x73, 0x72},     //Chest
{0x42, 0x41, 0x8B, 0x8A, 0x89},     //Granite
{0x1F, 0x1F, 0x1E, 0x1E, 0x1D}      //Quartz
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

static int chunksX = 2;
static int chunksY = 2;
static int chunksZ = 2;

static void pumpEvents();
static void terrainGen();
static void initChunks();
static void processInput();

//player movement configuration
#define PLAYER_SPEED 0.30
#define X_SENSITIVITY 0.08
#define Y_SENSITIVITY 0.08

//3D configuration
#define NEAR 0.05f
#define FAR 64.0f
#define FOV 85.0f       //fovy (degrees)

static void clockSleep(int millis)
{
  clock_t start = clock();
  while(clock() < start + millis);
}

static Chunk* getChunk(int x, int y, int z)
{
  return &chunks[x + y * chunksX + z * chunksX * chunksY];
}

void ocMain()
{
  inv = malloc(60 * sizeof(Stack));
  chunks = malloc(chunksX * chunksY * chunksZ * sizeof(Chunk));
  //print mem usage, wait ~1 second, continue
  puts("Generating world");
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
            clip.v[2] < -1 || clip.v[2] > 1)
        {
          //skip block, as nearest corner to player is outside frustum
          continue;
        }
        //in depth value, far plane is 64, so anything that
        //would overflow the byte has already been clipped
        glDepth(-viewSpace.v[2] * 3.9);
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

void terrainGen()
{
  srand(time(NULL));
  //This must assume that the world is a cube, simplifies things
  //sea level is half the world height
  int seaLevel = chunksY * 16 / 2;
  /*  build some fractal noise in-place (with already allocated 4 bits per block)
      always clamp values to [0, 16)
      first, fill with some random vals (with a small maximum, like 2)
      then, repeatedly sample some small 3D region (-50%), scale its values up (+50%), and add it back to original
  */
  for(int x = 0; x < chunksX; x++)
  {
    for(int y = 0; y < chunksY; y++)
    {
      for(int z = 0; z < chunksZ; z++)
      {
        setBlock(rand() % 0x2, x, y, z);
      }
    }
  }
  //first sample, scale, combine
  //sample the high end of each dimension so that those values are sampled before they are changed
  for(int iter = 0; iter < 3; iter++)
  {
    for(int x = 0; x < chunksX; x++)
    {
      for(int y = 0; y < chunksY; y++)
      {
        for(int z = 0; z < chunksZ; z++)
        {
          byte sample = getBlock(seaLevel + x / 2, seaLevel + y / 2, seaLevel + z / 2);
          byte orig = getBlock(x, y, z);
          int new = orig + sample;
          if(new >= 16)
            new = 15;
          setBlock(new, x, y, z);
        }
      }
    }
  }
  int stone = 0;
  //now, set each block above a threshold to stone, and each below to air
  for(int x = 0; x < chunksX; x++)
  {
    for(int y = 0; y < chunksY; y++)
    {
      for(int z = 0; z < chunksZ; z++)
      {
        int val = getBlock(x, y, z);
        if(val >= 1)
        {
          stone++;
          setBlock(STONE, x, y, z);
        }
        else
          setBlock(AIR, x, y, z);
      }
    }
  }
  float solidPercent = ((float) stone) / (4096 * chunksX * chunksY * chunksZ) * 100;
  printf("World is %.2f%% solid.\n", solidPercent);
  time_t t = time(NULL);
  while(time(NULL) < t + 3);
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

