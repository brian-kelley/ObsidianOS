/*
 * ObsidianOS floppy image builder
 * Produces bootable single partition, FAT12 disk image
 *
 * Required arguments (in this order):
 * --boot bootloader.bin  |  bootloader, loaded to 0x7C00
 * --kernel kernel.bin    |  kernel, will be first file in filesystem, and must fit between 
 *
 * Pass list of system files (besides kernel.bin), in order of how they appear on disk
 */

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"

typedef unsigned char byte;

//buffer where disk image is constructed
void* bin;
int binIndex;
const char* bootFile;
const char* kernelFile;

void writeByte(byte b)
{
  *((byte*) (bin + binIndex)) = b;
  binIndex++;
}

void writeChars(const char* chars, size_t n)
{
  for(size_t i = 0; i < n; i++)
  {
    writeByte(chars[i]);
  }
}

void writeShort(unsigned short s)
{
  *((byte*) (bin + binIndex)) = s & 0xFF;
  *((byte*) (bin + binIndex + 1)) = s >> 8;
  binIndex += 2;
}

void writeInt(unsigned i)
{
  *((byte*) (bin + binIndex)) = i & 0xFF;
  *((byte*) (bin + binIndex + 1)) = (i >> 8) & 0xFF;
  *((byte*) (bin + binIndex + 2)) = (i >> 16) & 0xFF;
  *((byte*) (bin + binIndex + 3)) = (i >> 24) & 0xFF;
  binIndex += 4;
}

void writeBuf(void* buf, size_t n)
{
  memcpy(bin + binIndex, buf, n);
  binIndex += n;
}

//add a system, readonly file entry to root directory
void makeRootEntry(int rootStart, const char* name, const char* ext, int size)
{
  void* kentry = bin + rootStart;
  //filename
  int i;
  for(i = 0; name[i]; i++)
  {
    *((char*) (kentry + i)) = name[i];
  }
  for(; i < 8; i++)
  {
    *((char*) (kentry + i)) = ' ';
  }
  for(i = 0; ext[i]; i++)
  {
    *((char*) (kentry + 8 + i)) = ext[i];
  }
  for(; i < 3; i++)
  {
    *((char*) (kentry + 8 + i)) = ' ';
  }
}

void err(const char* msg)
{
  printf("Error: ");
  puts(msg);
  exit(1);
}

int main(int argc, const char** argv)
{
  int sectors = 2880;
  int bytes = sectors * 512;
  int sectorsPerFat = 9;
  int rootDirSectors = 13;
  //with only 1 reserved sector, data area starts at sector 32
  //bootloader max size, will be 448 unless boot sector layout changes
  int bootMax = 448;
  //kernel max size, depends on how and where it is loaded (TBD)
  int kernelMax = 128 * 1024;
  bin = malloc(bytes);
  binIndex = 0;
  bootFile = NULL;
  kernelFile = NULL;
  if(argc == 1)
  {
    puts("Disk image builder requires a bootloader and kernel.");
    puts("Run with \"--help\" for details.");
    return 0;
  }
  else if(argc == 2 && strcmp(argv[1], "--help"))
  {
    puts("*** ObsidianOS Disk Image Builder *** ");
    puts("Note: arguments must be passed in this order.");
    puts("Required arguments:");
    puts("--boot bootloader.bin    | Bootloader (448 bytes max), org'd to 0x7C00");
    puts("--kernel kernel.bin      | Kernel (written as first file in root directory)");
    puts("Optional arguments:");
    puts("file1.bin file2.bin...   | Other files to include in root directory");
    return 0;
  }
  int arg = 1;
  if(arg < argc && strcmp(argv[arg], "--boot") == 0)
  {
    arg++;
    if(arg < argc)
      bootFile = argv[arg++];
    else
      err("Bootloader file not specified.");
  }
  else
    err("Bootloader required.");
  /*
  if(arg < argc && strcmp(argv[arg], "--kernel") == 0)
  {
    arg++;
    if(arg < argc)
      kernelFile = argv[arg++];
    else
      err("Kernel file not specified.");
  }
  else
    err("Kernel required.");
  */
  int miscFilesStart = arg;
  //write BIOS parameter block
  writeByte(0xEB);
  writeByte(0x3C);  //note: rel jump short n jumps to (n + 2) absolute (must be bootloader addr)
  writeByte(0x90);
  writeByte('o');
  writeByte('b');
  writeByte('s');
  writeByte('n');
  writeByte('.');
  writeByte('f');
  writeByte('a');
  writeByte('t');
  writeShort(512);                //bytes per sector
  writeByte(1);                   //sectors per cluster
  writeShort(1);                  //reserved sectors
  writeByte(2);                   //FATs
  int maxRootEntries = rootDirSectors * 512 / 32;
  writeShort(maxRootEntries);     //max root dir entries
  writeShort(sectors);            //total sectors on disk
  writeByte(0xF8);                //media descriptor - 0xF8 = HD
  writeShort(sectorsPerFat);      //sectors per FAT
  writeShort(16);                 //sectors per track
  writeShort(64);                 //heads/sides
  writeInt(0);                    //hidden sectors
  writeInt(0);                    //sectors, if >= 65536
  writeByte(0x80);                //Drive number, 0x80 for primary HD
  writeByte(0);                   //unused
  writeByte(0x29);                //Signature, must be 0x29
  writeInt(0xDEADBEEF);           //Serial number
  writeChars("OBSIDIAN HD", 11);  //volume label string
  writeChars("FAT12   ", 8);      //system identifier string
  //Bootloader goes here, gets up to 448 bytes
  {
    FILE* boot = fopen(bootFile, "rb");
    if(!boot)
      err("Could not open bootloader file.");
    size_t bootSize = 0;
    fseek(boot, 0, SEEK_END);
    bootSize = ftell(boot);
    rewind(boot);
    if(bootSize > bootMax)
      err("Bootloader is bigger than 448 bytes and can't fit in boot sector.");
    for(size_t i = 0; i < bootSize; i++)
    {
      byte b = 0;
      fread(&b, 1, 1, boot);
      writeByte(b);
    }
    fclose(boot);
    //pad bootloader section with 0
    for(size_t i = 0; i < bootMax - bootSize; i++)
      writeByte(0);
  }
  //boot sector signature
  writeByte(0x55);
  writeByte(0xAA);
  if(binIndex != 512)
    err("Boot sector is not 512 bytes");
  /*
  //2 empty FATs
  int fat1 = binIndex;
  int fat2 = fat1 + 512 * sectorsPerFat;
  for(int i = 0; i < 2 * 512 * sectorsPerFat; i++)
    writeByte(0);
  //empty root directory
  int rootStart = binIndex;
  for(int i = 0; i < 512 * rootDirSectors; i++)
  {
    writeByte(0);
  }
  int dataArea = binIndex;
  //kernel data starting at first data cluster
  {
    FILE* kernel = fopen(kernelFile, "rb");
    if(!kernel)
      err("Could not open kernel file.");
    int kernelSize = 0;
    fseek(kernel, 0, SEEK_END);
    kernelSize = ftell(kernel);
    rewind(kernel);
    if(kernelSize > kernelMax)
      err("Kernel is bigger than max size.");
    for(size_t i = 0; i < kernelSize; i++)
    {
      byte b = 0;
      fread(&b, 1, 1, kernel);
      writeByte(b);
    }
    fclose(kernel);
    //now that kernel size is known, create root directory entry for kernel.bin
  }
  */
  //write image from memory
  FILE* out = fopen("obsidian.img", "wb");
  fwrite(bin, 1, bytes, out);
  fclose(out);
  free(bin);
  puts("Bootable floppy image obsidian.img created.");
  return 0;
}

