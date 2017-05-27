#include "oc.h"

//Have 10x6 inventory
static Stack inv[60];
//Have 4x4x4 chunks (64^3 world)
static Chunk chunks[64];
static vec3 player;
static float yaw;     //yaw (left-right), radians, left is increasing
static float pitch;   //pitch (up-down), radians, ahead is 0, up is positive
//Solid colors of blocks
static byte blockColor[] = {0x00, 0x16, 0x06, 0x12, 0x1B, 0x2C, 0x0C, 0xF0, 0x77, 0x37, 0x44, 0x00};
//Sky color
static byte sky = 0x35;
static bool viewUpdated;
static bool wkey;
static bool akey;
static bool skey;
static bool dkey;

static void pumpEvents();
static void drawCube(float x, float y, float z, float size);

static void clockSleep(int millis)
{
  clock_t start = clock();
  while(clock() < start + millis);
}

void ocMain()
{
  yaw = 0;
  pitch = 0;
  //start in center of world
  player.v[0] = 32;
  player.v[1] = 32;
  player.v[2] = 32;
  wkey = false;
  akey = false;
  skey = false;
  dkey = false;
  viewUpdated = true;
  setModel(identity());
  int fov = 90;
  setProj(perspective(fov / (180.0f / PI), 0.1, 100));
  while(1)
  {
    //TESTING CAMERA
    pumpEvents();
    clearScreen(sky);
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
      player = vecadd(player, vecscale(target, 0.2 * xvel));
      viewUpdated = true;
    }
    if(yvel)
    {
      player = vecadd(player, vecscale(right, 0.2 * yvel));
      viewUpdated = true;
    }
    if(viewUpdated)
    {
      vec3 up = cross(right, target);
      target = vecadd(target, player);
      setView(lookAt(player, target, up));
    }
    setColor(blockColor[STONE]);
    drawCube(32, 32, 39, 1);
    clockSleep(30);
  }
}

byte getBlock(Chunk* c, int cx, int cy, int cz)
{
  int ind = (cx + (cy << 4) + (cz << 8)) >> 1;
  byte b = c->vals[ind];
  return ind & 1 ? (b >> 4) : (b & 0xF);
}

void setBlock(byte newBlock, Chunk* c, int cx, int cy, int cz)
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
          if(k.scancode == KEY_W)
          {
            wkey = k.pressed;
          }
          if(k.scancode == KEY_A)
          {
            akey = k.pressed;
          }
          if(k.scancode == KEY_S)
          {
            skey = k.pressed;
          }
          if(k.scancode == KEY_D)
          {
            dkey = k.pressed;
          }
        }
        break;
      case MOTION_EVENT:
        {
          const float pitchLimit = 88 / (180.0f / PI);
          const float pitchSens = 0.04;
          const float yawSens = 0.04;
          viewUpdated = true;
          yaw -= yawSens * ev.e.motion.dx;
          //clamp yaw to 0:2pi (but preserve rotation beyond the bounds)
          if(yaw < 0)
            yaw += 2 * PI;
          else if(yaw >= 2 * PI)
            yaw -= 2 * PI;
          pitch -= pitchSens * ev.e.motion.dy;
          //clamp pitch to -pitchLimit:pitchLimit
          if(pitch < -pitchLimit)
            pitch = -pitchLimit;
          else if(pitch > pitchLimit)
            pitch = pitchLimit;
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
    glVertex3f(x, y, z + size);
    glVertex3f(x + size, y, z + size);
  }
  //Top face (high y)
  if(player.v[1] > y + size)
  {
    glColor1i(0x3);
    glVertex3f(x, y + size, z);
    glVertex3f(x + size, y + size, z);
    glVertex3f(x, y + size, z + size);
    glVertex3f(x + size, y + size, z + size);
  }
  //Left face (low x)
  if(player.v[0] < x)
  {
    glColor1i(0x4);
    glVertex3f(x, y, z);
    glVertex3f(x, y + size, z);
    glVertex3f(x, y, z + size);
    glVertex3f(x, y + size, z + size);
  }
  //Right face (high x)
  if(player.v[0] > x + size)
  {
    glColor1i(0x5);
    glVertex3f(x + size, y, z);
    glVertex3f(x + size, y + size, z);
    glVertex3f(x + size, y, z + size);
    glVertex3f(x + size, y + size, z + size);
  }
  //Back face (low z);
  if(player.v[2] < z)
  {
    glColor1i(0x6);
    glVertex3f(x, y, z);
    glVertex3f(x + size, y, z);
    glVertex3f(x, y + size, z);
    glVertex3f(x + size, y + size, z);
  }
  //Front face (high z);
  if(player.v[2] > z + size)
  {
    glColor1i(0x7);
    glVertex3f(x, y, z + size);
    glVertex3f(x + size, y, z + size);
    glVertex3f(x, y + size, z + size);
    glVertex3f(x + size, y + size, z + size);
  }
  glEnd();
}

