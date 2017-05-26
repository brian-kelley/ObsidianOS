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

void clockSleep(int millis)
{
  clock_t start = clock();
  while(clock() < start + millis);
}

void kernel_main()
{
  initTime();
  initTerminal();
  initFPU();
  ataInit();
  initFatDriver();
  initKeyboard();
  resetTermCursor();
  //Graphics testing
  setModel(identity());
  //fov, in degrees
  float fov = 50;
  setProj(perspective(fov / (180.0f / PI), 0.1, 100));
  //camera at origin, upright, pointing towards -Z
  setView(identity());
  int i = 1;
  while(1)
  {
    i++;
    float t = i * (PI / 100.0f);
    clearScreen(0);
    //printf("%i ", i);
    {
      vec3 camera = {0, 0, 5};
      vec3 up = {sin(t), cos(t), 0};
      vec3 target = {0, 0, 0};
      setView(lookAt(camera, target, up));
    }
    vec3 v1 = {-1, -1, 0};
    vec3 v2 = {1, -1, 0};
    vec3 v3 = {-1, 1, 0};
    vec3 v4 = {1, 1, 0};
    v1 = vshade(v1);
    v2 = vshade(v2);
    v3 = vshade(v3);
    v4 = vshade(v4);
    /*
    PRINT_VEC3(v1);
    PRINT_VEC3(v2);
    PRINT_VEC3(v3);
    PRINT_VEC3(v4);
    */
    point p1 = viewport(v1);
    point p2 = viewport(v2);
    point p3 = viewport(v3);
    point p4 = viewport(v4);
    /*
    PRINT_POINT(p1);
    PRINT_POINT(p2);
    PRINT_POINT(p3);
    PRINT_POINT(p4);
    */
    setColor(0x2A);
    fillTriangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
    setColor(0x2C);
    fillTriangle(p2.x, p2.y, p3.x, p3.y, p4.x, p4.y);
    clockSleep(20);
  }
  while(1);
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
          terminalKeyListener(ev.e.key.scancode);
        }
        break;
      case MOTION_EVENT:
        /*
        //putchar('M');
        {
          byte* fb = (byte*) 0xA0000;
          //clear previous cursor pixel (if mouse is not down)
          if(!getButtonState(LEFT_BUTTON))
            fb[mouseX + mouseY * 320] = 0x0;
          //update mouse position
          int oldX = mouseX;
          int oldY = mouseY;
          mouseX += ev.e.motion.dx;
          mouseY += ev.e.motion.dy;
          //clamp mouse pos to screen
          if(mouseX < 0)
            mouseX = 0;
          if(mouseY < 0)
            mouseY = 0;
          if(mouseX >= 320)
            mouseX = 319;
          if(mouseY >= 200)
            mouseY = 199;
          }
          //draw cursor in new position
          fb[mouseX + mouseY * 320] = 0xF;
        }
        */
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

