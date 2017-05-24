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

void kernel_main()
{
  initTime();
  initTerminal();
  initFPU();
  ataInit();
  initFatDriver();
  initKeyboard();
  resetTermCursor();
  setColor(0xF);
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

