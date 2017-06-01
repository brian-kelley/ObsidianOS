#include "video.h"

void clearScreen(byte color)
{
  //clear 4 bytes (4 pixels) at a time
  memset((void*) 0xA0000, color, 64000);
}


void drawChar(char c, int x, int y, byte fg, byte bg)
{
  /*
  if(x < 0 || y < 0 || x >= 40 || y >= 25)
  {
    const char* msg = "drawChar bound error!";
    unsigned n = strlen(msg);
    for(unsigned i = 0; i < n; i++)
    {
      drawChar(msg[i], i, 0, 0xF, 0x0);
    }
    while(1);
  }
  */
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
	while((readport(0x3DA) & 8) == 0);	    //Wait to enter vertical refresh interval
	while((readport(0x3DA) & 8) == 1);			//Wait for vertical refresh interval to end
}

