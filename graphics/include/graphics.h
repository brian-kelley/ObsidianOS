#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "globalDefines.h"
#include "stdio.h"

extern byte* const framebuf;
//drawing color
extern byte color;

#define drawPixel(x, y) framebuf[x + y * 320] = color;

void setColor(byte c);
void drawLine(int x1, int y1, int x2, int y2);

#endif
