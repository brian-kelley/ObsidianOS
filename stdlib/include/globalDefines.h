#ifndef __GLOBAL_DEFINES__
#define __GLOBAL_DEFINES__

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;
typedef unsigned long long qword;

//Port io that is used all over the place
extern byte readport(dword port);
extern void writeport(dword port, dword data);
extern word readportw(dword port);
extern void writeportw(dword port, dword data);

#endif
