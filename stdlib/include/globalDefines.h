#ifndef __GLOBAL_DEFINES__
#define __GLOBAL_DEFINES__

#include <stdint.h>

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;
typedef uint64_t qword;
typedef dword size_t;
typedef byte bool;

#define true 1
#define false 0

//Generic port I/O
extern byte readport(dword port);
extern void writeport(dword port, dword data);
extern word readportw(dword port);
extern void writeportw(dword port, dword data);

#endif
