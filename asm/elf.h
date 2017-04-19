#ifndef ELF_H
#define ELF_H

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

typedef struct
{
  ElfWord name;
  ElfWord type;
  ElfWord flags;
  ElfAddr addr;
  ElfOffset offset;
  ElfWord size;
  ElfWord link;
  ElfWord info;
  ElfWord align;
  ElfWord entrySize;
} ElfSectionHeader;

typedef struct
{
  ElfWord type;
  ElfOffset offset;
  ElfAddr vaddr;
  ElfAddr paddr;
  ElfWord fileSize;
  ElfWord memSize;
  ElfWord flags;
  ElfWord align;
} ElfProgHeader;

typedef struct
{
  ElfWord name;
  ElfAddr value;
  ElfWord size;
  byte info;
  byte other;
  ElfHalf shIndex;
} ElfSymbol;

enum ElfType
{
  ET_NONE,
  ET_REL,
  ET_EXEC,
  ET_DYN,
  ET_CORE
};

enum ElfSectionType
{
  SHT_NULL,
  SHT_PROGBITS,
  SHT_SYMTAB,
  SHT_STRTAB,
  SHT_RELA,
  SHT_HASH,
  SHT_DYNAMIC,
  SHT_NOTE,
  SHT_NOBITS,
  SHT_REL,
  SHT_SHLIB,
  SHT_DYNSYM
};

//i386 machine ID
#define ELF_MACHINE 3

//32-bit class
#define ELF_CLASS 1

//little-endian
#define ELF_DATA 1

//"current version" - ELF 1.0
#define ELF_VERSION 1

ElfHeader getElfHeader();
void writeElfHeader(FILE* out);

#endif

