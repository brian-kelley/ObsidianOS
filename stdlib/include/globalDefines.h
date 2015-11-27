#ifndef __GLOBAL_DEFINES__
#define __GLOBAL_DEFINES__

#include <stdint.h>

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;
typedef uint64_t qword;
typedef dword size_t;
typedef int intmax_t;
typedef int ptrdiff_t;

//Boolean type
#undef bool //don't want C99 _Bool, use unsigned char instead
#define bool byte
#define true 1
#define false 0

#ifndef NULL
#define NULL ((void*) 0)
#endif
#ifndef RAND_MAX
#define RAND_MAX 0xAFFFFFFF
#endif
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS (0)
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE (-1)
#endif

//Generic port I/O
extern byte readport(dword port);
extern void writeport(dword port, dword data);
extern word readportw(dword port);
extern void writeportw(dword port, dword data);

#endif
