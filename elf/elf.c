#include "elf.h"

void writeElfHeader(FILE* out)
{
  fputs("\x7FELF", out);
  byte b = ELF_CLASS;
  fwrite(&b, 1, 1, out);
  b = ELF_DATA;
  fwrite(&b, 1, 1, out);
  b = ELF_VERSION;
  fwrite(&b, 1, 1, out);
  fputs("\0\0\0\0\0\0\0\0\0", out);
  ElfHalf 
}

