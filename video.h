#include <stdint.h>

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;
typedef uint64_t qword;

extern void writeport(dword portNum, dword value);
extern byte readport(dword portNum);
//font binary from assembly file
extern void* getFontPtr();

void enterMode12H();
void putPixel(word x, word y, byte color);
void clearScreen(byte color);
//Draw a character in Liberation Mono anywhere on screen, with foreground and background colors given
//Note: x and y are on text grid, 8x16 pixels
void drawChar(char c, word x, word y, byte fg, byte bg);
