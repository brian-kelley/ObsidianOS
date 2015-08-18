#ifndef __FAT_DRV_H__
#define __FAT_DRV_H__

#include "globalDefines.h"
#include "atadrv.h"
#include "string.h"

struct FILE_ATTRIB
{
    const byte READONLY = 1;
    const byte HIDDEN = 2;
    const byte SYSTEM = 4;
    const byte VOLUME_LABEL = 8;
    const byte DIRECTORY = 16;
};

typedef struct //32 byte struct exactly matching layout on disk
{
    char filename[8];
    char fileExt[3];
    byte attributes; //& with FILE_ATTRIB values
    byte PAD[10];
    word time; //5 for hour, 6 for minute, 5 for two-second intervals 0-30
    word date; //7 for year, 4 for month, 5 for day
    word firstCluster;
    dword size;
} DirEntry;

typedef struct
{
    byte data[512];
} Sector;

void initFatDriver();		 //initialize the fatInfo struct from boot sector
bool createDir(const char* path); //not recursive
bool createFile(const char* path);
bool deleteFile(const char* path);
bool deleteDir(const char* path);
void setPermission(DirEntry* entry, byte flags);  //set the FAT16 attribute flags on a file
dword getFreeSpace();
int numFilesInDir(const char* path); //get number of files and directories
DirEntry getRootDirEntry(int index); //get info about the nth root directory entry
bool walkPath(DirEntry* result, const char* path);
bool findFile(DirEntry* result, DirEntry* dir, const char* name, bool allowDir);
Sector getSectorForCluster(word cluster, int n); //load the nth sector
bool isDirectory(DirEntry* entry);
qword getFreeSpace(); //get free disk space in bytes
word getNextCluster(word cluster); //read sector in fat and follow cluster chain
void allocCluster(word last, bool first); //mark a cluster as used and EOF, set previous last to point to new
void reallocChain(word first, size_t newSize); //add or remove clusters at end of file to get new size (bytes)
word allocChain(size_t size);
void deleteChain(word first);  //mark all clusters in chain as free (don't modify corresponding directory entry)
void flushFat(); //update FAT(s) on disk to reflect memory copy
int fatDiff();   //find the first difference (in sectors) between logical and acual, or -1 if they are the same
bool isValidFilename(const char* name); //does it fit in 8-3 filename? (include dot)

#endif
