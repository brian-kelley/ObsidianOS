#include "video.h"

void writeAttributeReg(dword index, dword value)
{
	readport(0x3DA);
	writeport(0x3C0, index);
	writeport(0x3C0, value);
}

void enterMode12H()
{
	//write misc reg
	writeport(0x3C2, 0xE3);
	//write seq regs
	writeport(0x3C4, 0x0);
	writeport(0x3C5, 0x3);
	writeport(0x3C4, 0x1);
	writeport(0x3C5, 0x1);
	writeport(0x3C4, 0x2);
	writeport(0x3C5, 0x8);
	writeport(0x3C4, 0x3);
	writeport(0x3C5, 0x0);
	writeport(0x3C4, 0x4);
	writeport(0x3C5, 0x6);
	//unlock CRTC registers
	writeport(0x3D4, 0x3);
	writeport(0x3D5, readport(0x3D5) | 0x80);
	writeport(0x3D4, 0x11);
	writeport(0x3D5, readport(0x3D5) & ~0x80);
	//note: in future make sure the 0x80 bit is set @ 0x3 and unset @ 0x11
	//write CRTC regs
	writeport(0x3D4, 0x0);
	writeport(0x3D5, 0x5F);
	writeport(0x3D4, 0x1);
	writeport(0x3D5, 0x4F);
	writeport(0x3D4, 0x2);
	writeport(0x3D5, 0x50);
	writeport(0x3D4, 0x3);
	writeport(0x3D5, 0x82 | 0x80);
	writeport(0x3D4, 0x4);
	writeport(0x3D5, 0x54);
	writeport(0x3D4, 0x5);
	writeport(0x3D5, 0x80);
	writeport(0x3D4, 0x6);
	writeport(0x3D5, 0x0B);
	writeport(0x3D4, 0x7);
	writeport(0x3D5, 0x3E);
	writeport(0x3D4, 0x8);
	writeport(0x3D5, 0x00);
	writeport(0x3D4, 0x9);
	writeport(0x3D5, 0x40);
	writeport(0x3D4, 0xA);
	writeport(0x3D5, 0x00);
	writeport(0x3D4, 0xB);
	writeport(0x3D5, 0x00);
	writeport(0x3D4, 0xC);
	writeport(0x3D5, 0x00);
	writeport(0x3D4, 0xD);
	writeport(0x3D5, 0x00);
	writeport(0x3D4, 0xE);
	writeport(0x3D5, 0x00);
	writeport(0x3D4, 0xF);
	writeport(0x3D5, 0x00);
	writeport(0x3D4, 0x10);
	writeport(0x3D5, 0xEA);
	writeport(0x3D4, 0x11 & ~0x80);
	writeport(0x3D5, 0x0C);
	writeport(0x3D4, 0x12);
	writeport(0x3D5, 0xDF);
	writeport(0x3D4, 0x13);
	writeport(0x3D5, 0x28);
	writeport(0x3D4, 0x14);
	writeport(0x3D5, 0x00);
	writeport(0x3D4, 0x15);
	writeport(0x3D5, 0xE7);
	writeport(0x3D4, 0x16);
	writeport(0x3D5, 0x04);
	writeport(0x3D4, 0x17);
	writeport(0x3D5, 0xE3);
	writeport(0x3D4, 0x18);
	writeport(0x3D5, 0xFF);
	//Graphics controller regs
	writeport(0x3CE, 0x0);
	writeport(0x3CF, 0x00);
	writeport(0x3CE, 0x1);
	writeport(0x3CF, 0x00);
	writeport(0x3CE, 0x2);
	writeport(0x3CF, 0x00);
	writeport(0x3CE, 0x3);
	writeport(0x3CF, 0x00);
	writeport(0x3CE, 0x4);
	writeport(0x3CF, 0x03);
	writeport(0x3CE, 0x5);
	writeport(0x3CF, 0x00);
	writeport(0x3CE, 0x6);
	writeport(0x3CF, 0x05);
	writeport(0x3CE, 0x7);
	writeport(0x3CF, 0x0F);
	writeport(0x3CE, 0x8);
	writeport(0x3CF, 0xFF);
	//Attribute controller regs
	writeAttributeReg(0x0, 0x00);
	writeAttributeReg(0x1, 0x01);
	writeAttributeReg(0x2, 0x02);
	writeAttributeReg(0x3, 0x03);
	writeAttributeReg(0x4, 0x04);
	writeAttributeReg(0x5, 0x05);
	writeAttributeReg(0x6, 0x14);
	writeAttributeReg(0x7, 0x07);
	writeAttributeReg(0x8, 0x38);
	writeAttributeReg(0x9, 0x39);
	writeAttributeReg(0xA, 0x3A);
	writeAttributeReg(0xB, 0x3B);
	writeAttributeReg(0xC, 0x3C);
	writeAttributeReg(0xD, 0x3D);
	writeAttributeReg(0xE, 0x3E);
	writeAttributeReg(0xF, 0x3F);
	writeAttributeReg(0x10, 0x01);
	writeAttributeReg(0x11, 0x00);
	writeAttributeReg(0x12, 0x0F);
	writeAttributeReg(0x13, 0x00);
	writeAttributeReg(0x14, 0x00);
	//Might as well leave CRTC regs unlocked but need to restart refreshes
	readport(0x3DA);
	writeport(0x3C0, 0x20);
}

void clearScreen(byte color)
{
	byte* vram = (byte*) 0xA0000;
	writeport(0x3C4, 2);
	//select plane 0
	writeport(0x3C5, 0x1);
	byte plane0 = color & 1 ? 0xFF : 0x0;
	for(int i = 0; i < 0x9600; i++)
	{
		vram[i] = plane0;
	}
	//plane 1
	writeport(0x3C4, 2);
	writeport(0x3C5, 0x2);
	byte plane1 = color & 0x2 ? 0xFF : 0x0;
	for(int i = 0; i < 0x9600; i++)
	{
		vram[i] = plane1;
	}
	//plane 2
	writeport(0x3C4, 2);
	writeport(0x3C5, 0x4);
	byte plane2 = color & 0x4 ? 0xFF : 0x0;
	for(int i = 0; i < 0x9600; i++)
	{
		vram[i] = plane2;
	}
	//plane 3
	writeport(0x3C4, 2);
	writeport(0x3C5, 0x8);
	byte plane3 = color & 0x8 ? 0xFF : 0x0;
	for(int i = 0; i < 0x9600; i++)
	{
		vram[i] = plane3;
	}
}

void drawChar(char c, int x, int y, byte fg, byte bg)
{
  int sx = x * 8;
  int sy = y * 8;
  if(c < '!' || c > '~')
    return;
  byte* glyph = fontbin + 8 * (c - '!');
  byte* base = (byte*) (0xA0000 + sx + sy * 320);
  byte* iter = base;
  const int stride = 312;
  for(int row = 0; row < 8; row++)
  {
    for(byte mask = 0x80; mask; mask <<= 1)
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

/*
void drawChar(char c, int x, int y, byte fg, byte bg)
{
	//Bounds checking
	if(x < 0 || x >= 80 || y < 0 || y >= 30)
		return;
	int offset = 8 * (c - '!');	//# of bytes into font
	byte* glyph = fontbin + offset;
	byte* fb = (byte*) 0xA0000;
	//Plane 0
	writeport(0x3C4, 2);
	writeport(0x3C5, 1);
	byte fgbits = fg & 1 ? 0xFF : 0x0;
	byte bgbits = bg & 1 ? 0xFF : 0x0;
	for(int i = 0; i < 16; i++)
	{
		//Use bits from foreground where font bit is set, and bits from bg when font bit is clear
		//In plane, 80 bytes (1 bit per pixel, 640 pixels) across per pixel row
		//Each char has 16 rows, i represents this from 0 to 15
		fb[80 * (16 * y + i) + x] = (fgbits & reverseTable[glyph[i]]) | (bgbits & ~reverseTable[glyph[i]]);
	}
	//Plane 1
	writeport(0x3C4, 2);
	writeport(0x3C5, 2);
	fgbits = fg & 2 ? 0xFF : 0x0;
	bgbits = bg & 2 ? 0xFF : 0x0;
	for(int i = 0; i < 16; i++)
	{
		//Use bits from foreground where font bit is set, and bits from bg when font bit is clear
		fb[80 * (16 * y + i) + x] = (fgbits & reverseTable[glyph[i]]) | (bgbits & ~reverseTable[glyph[i]]);
	}
	//Plane 2
	writeport(0x3C4, 2);
	writeport(0x3C5, 4);
	fgbits = fg & 4 ? 0xFF : 0x0;
	bgbits = bg & 4 ? 0xFF : 0x0;
	for(int i = 0; i < 16; i++)
	{
		//Use bits from foreground where font bit is set, and bits from bg when font bit is clear
		fb[80 * (16 * y + i) + x] = (fgbits & reverseTable[glyph[i]]) | (bgbits & ~reverseTable[glyph[i]]);
	}
	//Plane 3
	writeport(0x3C4, 2);
	writeport(0x3C5, 8);
	fgbits = fg & 8 ? 0xFF : 0x0;
	bgbits = bg & 8 ? 0xFF : 0x0;
	for(int i = 0; i < 16; i++)
	{
		//Use bits from foreground where font bit is set, and bits from bg when font bit is clear
		fb[80 * (16 * y + i) + x] = (fgbits & reverseTable[glyph[i]]) | (bgbits & ~reverseTable[glyph[i]]);
	}
}
*/

void vsync()
{
	while((readport(0x3DA) & 8) == 0);	    //Wait to enter vertical refresh interval
	while((readport(0x3DA) & 8) == 1);			//Wait for vertical refresh interval to end
}

