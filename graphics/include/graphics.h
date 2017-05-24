#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "globalDefines.h"
#include "stdio.h"
#include "geometry.h"

#define drawPixel(x, y) framebuf[x + y * 320] = color;

void setColor(byte c);
void drawLine(int x1, int y1, int x2, int y2);
void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3);
void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3);
void drawRect(int x, int y, int w, int h);
void fillRect(int x, int y, int w, int h);

#endif

