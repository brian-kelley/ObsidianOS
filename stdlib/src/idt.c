#include "idt.h"

idtEntry idt[256];
idtPtr idtDesc = {1023, (dword) &idt[0]};

