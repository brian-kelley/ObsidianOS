#include "video.h"

void clearScreen(byte color)
{
  //clear 4 bytes (4 pixels) at a time
  dword color4 = color;
  color4 |= (color4 << 8);
  color4 |= (color4 << 16);
  dword* iter = (dword*) 0xA0000;
  for(int i = 0; i < 64000 / 4; i++)
  {
    iter[i] = color4;
  }
}

void drawChar(char c, int x, int y, byte fg, byte bg)
{
  int sx = x * 8;
  int sy = y * 8;
  byte* iter = (byte*) (0xA0000 + sx + sy * 320);
  const int stride = 312;
  if(c < '!' || c > '~')
  {
    //non-visible character, clear that region
    for(int i = 0; i < 8; i++)
    {
      for(int j = 0; j < 8; j++)
      {
        *iter = bg;
        iter++;
      }
      iter += stride;
    }
    return;
  }
  byte* glyph = fontbin + 8 * (c - '!');
  //stride is from right edge of character to left edge of char on next line
  for(int row = 0; row < 8; row++)
  {
    for(byte mask = 0x1; mask; mask <<= 1)
    {
      if(glyph[row] & mask)
      {
        *iter = fg;
      }
      else
      {
        *iter = bg;
      }
      iter++;
    }
    iter += stride;
  }
}

void vsync()
{
  /*
	while((readport(0x3DA) & 8) == 0);	    //Wait to enter vertical refresh interval
	while((readport(0x3DA) & 8) == 1);			//Wait for vertical refresh interval to end
  */
}

