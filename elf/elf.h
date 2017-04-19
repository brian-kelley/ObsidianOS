#ifndef ELF_H
#define ELF_H

#include "sys.h"
#include "types.h"
#include "stdio.h"

typedef unsigned ElfAddr;
typedef unsigned short ElfHalf;
typedef unsigned ElfOffset;
typedef int ElfSword;
typedef unsigned ElfWord;

typedef struct
{
  byte ident[16];
  ElfHalf type;
  ElfHalf machine;
  ElfWord version;
  ElfAddr entry;
  ElfOffset phOffset;   //program header offset
  ElfOffset shOffset;   //section header offset
  ElfWord flags;
  ElfHalf ehSize;
  ElfHalf phEntrySize;
  ElfHalf phNum;
  ElfHalf shEntrySize;
  ElfHalf shNum;
  ElfHalf shStrIndex;
} ElfHeader;

enum ElfType
{
  ET_NONE,
  ET_REL,
  ET_EXEC,
  ET_DYN,
  ET_CORE
};

//i386 machine ID
#define ELF_MACHINE 3

//32-bit class
#define ELF_CLASS 1

//little-endian
#define ELF_DATA 1

//"current version" - ELF 1.0
#define ELF_VERSION 1

void writeElfHeader(FILE* out);

#endif

