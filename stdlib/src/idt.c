#include "idt.h"

void loadEmptyIDT()
{
  dword passAddr = (dword) pass;
  for(int i = 0; i < 256; i++)
  {
    idt[i].offsetLower = passAddr & 0xFFFF;
    idt[i].selector = 0x08;
    idt[i].zero = 0;
    idt[i].type_attr = 0x8E;
    idt[i].offsetHigher = (passAddr & 0xFFFF0000) >> 16;
  }
  loadIDT(idt);
}

