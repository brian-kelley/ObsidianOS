#ifndef IDT_H
#define IDT_H

#include "globalDefines.h"

typedef struct
{
  word offsetLower;
  word selector;
  byte zero;
  byte type_attr;
  word offsetHigher;
} idtEntry;

idtEntry idt[256];

void loadEmptyIDT();

//pass - interrupt handler that does nothing
extern void pass();
extern void loadIDT(void* idtAddr);

#endif

