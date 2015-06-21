#ifndef VIDEO_H
#define VIDEO_H

#include <stdint.h>
#include "asmTypes.h"

extern void writeport(dword portNum, dword value);
extern byte readport(dword portNum);
//font binary from assembly file
extern void* getFontPtr();

void enterMode12H();
void clearScreen(byte color);
//Draw a character in Liberation Mono anywhere on screen, with foreground and background colors given
//Note: x and y are on text grid, 8x16 pixels
void drawChar(char c, int x, int y, byte fg, byte bg);
void vsync();
void puts(const char* str, int x, int y);

#endif