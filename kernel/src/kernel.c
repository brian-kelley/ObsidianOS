#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "video.h"
#include "globalDefines.h"
#include "terminal.h"
#include "atadrv.h"
#include "memory.h"
#include "string.h"
#include "stdio.h"
#include "math.h"
#include "events.h"
#include "graphics.h"

extern void demo();

/* Note on i386 GCC types:

short: 16 bits
int: 32 bits
long int: 32 bits
long long int: 64 bits
float: 32 bits
double: 64 bits
long double: 96 bits in memory for 8-byte alignment but is actually 80 bit precision
*/

extern byte getFontVal();
extern void initTime();     //time.c
extern void initFPU();	    //mathAsm.asm
extern void inttest();

bool userProc = 0;	//0 if in terminal and 1 if running user program

char hexFromBits(byte bits)	//put in low 4
{
  switch(bits & 0xF)
  {
    case 0:
      return '0';
    case 1:
      return '1';
    case 2:
      return '2';
    case 3:
      return '3';
    case 4:
      return '4';
    case 5:
      return '5';
    case 6:
      return '6';
    case 7:
      return '7';
    case 8:
      return '8';
    case 9:
      return '9';
    case 10:
      return 'A';
    case 11:
      return 'B';
    case 12:
      return 'C';
    case 13:
      return 'D';
    case 14:
      return 'E';
    case 15:
      return 'F';
    default:
      return '?';
  }
}

void keyPressed(byte scancode, bool pressed)
{
  //Keyboard interrupts send data here. Send data to user applications etc.
  if(userProc)
  {
    //TODO: give event to user program key listeners...
  }
  else
  {
    //terminal is the current focus (and ignore releases)
    if(pressed)
    {
      terminalKeyListener(scancode);
    }
  }
}

void showPalette()
{
  for(int i = 0; i < 64000; i++)
  {
    *((byte*) (0xA0000 + i)) = 0x0;
  }
  for(int i = 0; i < 256; i++)
  {
    int bx = (i % 16) * 8;
    int by = (i / 16) * 8;
    for(int x = bx; x < bx + 8; x++)
    {
      for(int y = by; y < by + 8; y++)
      {
        *((byte*) (0xA0000 + x + 8 + 320 * (y + 8))) = i;
      }
    }
  }
  for(int i = 0; i < 10; i++)
  {
    drawChar('0' + i, 1 + i, 0, 0xF, 0);
    drawChar('0' + i, 0, 1 + i, 0xF, 0);
  }
  for(int i = 0; i < 6; i++)
  {
    drawChar('A' + i, 11 + i, 0, 0xF, 0);
    drawChar('A' + i, 0, 11 + i, 0xF, 0);
  }
}

static int mouseX = 160;
static int mouseY = 100;

#define PRINT_VEC3(vec) printf("%s: (%.2f %.2f %.2f)\n", #vec, vec.v[0], vec.v[1], vec.v[2]);
#define PRINT_POINT(p) printf("%s: (%i %i)\n", #p, p.x, p.y);

static vec3 camPos;

void drawCube(float x, float y, float z, float size)
{
  glBegin(GL_TRIANGLES);
  //Bottom face (low y)
  if(camPos.v[1] < y)
  {
    glColor1i(0x28);
    glVertex3f(x, y, z);
    glVertex3f(x + size, y, z);
    glVertex3f(x + size, y, z + size);
    glVertex3f(x + size, y, z + size);
    glVertex3f(x, y, z + size);
    glVertex3f(x, y, z);
  }
  //Top face (high y)
  if(camPos.v[1] > y + size)
  {
    glColor1i(0x29);
    glVertex3f(x, y + size, z);
    glVertex3f(x + size, y + size, z);
    glVertex3f(x + size, y + size, z + size);
    glVertex3f(x + size, y + size, z + size);
    glVertex3f(x, y + size, z + size);
    glVertex3f(x, y + size, z);
  }
  //Left face (low x)
  if(camPos.v[0] < x)
  {
    glColor1i(0x2A);
    glVertex3f(x, y, z);
    glVertex3f(x, y + size, z);
    glVertex3f(x, y + size, z + size);
    glVertex3f(x, y + size, z + size);
    glVertex3f(x, y, z + size);
    glVertex3f(x, y, z);
  }
  //Right face (high x)
  if(camPos.v[0] > x + size)
  {
    glColor1i(0x2B);
    glVertex3f(x + size, y, z);
    glVertex3f(x + size, y + size, z);
    glVertex3f(x + size, y + size, z + size);
    glVertex3f(x + size, y + size, z + size);
    glVertex3f(x + size, y, z + size);
    glVertex3f(x + size, y, z);
  }
  //Back face (low z);
  if(camPos.v[2] < z)
  {
    glColor1i(0x2C);
    glVertex3f(x, y, z);
    glVertex3f(x + size, y, z);
    glVertex3f(x + size, y + size, z);
    glVertex3f(x + size, y + size, z);
    glVertex3f(x, y + size, z);
    glVertex3f(x, y, z);
  }
  //Front face (high z);
  if(camPos.v[2] > z + size)
  {
    glColor1i(0x2D);
    glVertex3f(x, y, z + size);
    glVertex3f(x + size, y, z + size);
    glVertex3f(x + size, y + size, z + size);
    glVertex3f(x + size, y + size, z + size);
    glVertex3f(x, y + size, z + size);
    glVertex3f(x, y, z + size);
  }
  glEnd();
}

//Orbit camera around a stationary cube
void cubeDemo()
{
  //Graphics testing
  setModel(identity());
  //fov, in degrees
  float fov = 90;
  setProj(perspective(fov / (180.0f / PI), 0.1, 100));
  //camera at origin, upright, pointing towards -Z
  int i = 1;
  while(1)
  {
    clock_t start = clock();
    i++;
    float t = i * (PI / 360.0f);
    clearScreen(0);
    {
      camPos = ((vec3) {5 * sin(3 * t) + 0.5, 0.5 + 4 * sin(3 * t / 2), 5 * cos(3 * t) + 0.5});
      vec3 up = {0, 1, 0};
      vec3 target = {0, 0, 0};
      setView(lookAt(camPos, target, up));
    }
    glClear(0x0);
    drawCube(-1, -1, -1, 2);
    glFlush();
    while(clock() < start + 17);
  }
}

extern void ocMain();

void kernel_main()
{
  initTime();
  initTerminal();
  initFPU();
  ataInit();
  //initFatDriver();
  initKeyboard();
  resetTermCursor();
  printMemStats();
  renderBuf = malloc(64000);
#ifdef DOUBLE_BUFFERED
  depthBuf = malloc(64000);
#endif
  ocMain();
  while(1)
  {
    Event ev = getNextEvent();
    //call event handler
    switch(ev.type)
    {
      case KEY_EVENT:
        //putchar('K');
        if(ev.e.key.pressed)
        {
          //terminalKeyListener(ev.e.key.scancode);
        }
        break;
      case MOTION_EVENT:
        break;
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

