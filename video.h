#include <stdint.h>

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;
typedef uint64_t qword;

extern void writeport(dword portNum, dword value);
extern byte readport(dword portNum);

void enterMode12H();
void putPixel(word x, word y, byte color);
void clearScreen(byte color);
