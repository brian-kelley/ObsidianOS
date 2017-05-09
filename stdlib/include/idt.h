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

extern idtEntry idt[256];

//pass - interrupt handler that does nothing
extern void pass();
//load the IDT and re-enable interrupts
extern void loadIDT();

#endif

