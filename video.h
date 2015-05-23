#include <stdint.h>

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;
typedef uint64_t qword;

extern void writeport(dword portNum, dword value);
extern byte readport(dword portNum);

void enterMode12H();
void putPixel(uint16_t x, uint16_t y, uint8_t color);
