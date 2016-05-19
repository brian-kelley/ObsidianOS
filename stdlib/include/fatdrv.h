#ifndef __FAT_DRV_H__
#define __FAT_DRV_H__

#include "globalDefines.h"
#include "atadrv.h"
#include "string.h"
#include "terminal.h"
#include "stdlib.h"

enum FILE_ATTRIB
{
    FA_READONLY = 1,
    FA_HIDDEN = 2,
    FA_SYSTEM = 4,
    FA_VOLUME_LABEL = 8,
    FA_DIRECTORY = 16
};

typedef struct //32 byte struct exactly matching layout on disk
{
    char filename[8];
    char fileExt[3];
    byte attributes; //& with FILE_ATTRIB values
    byte _PAD[10];
    word time; //5 for hour, 6 for minute, 5 for two-second intervals 0-30
    word date; //7 for year, 4 for month, 5 for day
    word firstCluster;
    word size;
} DirEntry;

typedef struct
{
    word cluster;
    word sector;
    word index;
} EntrySlot;

void initFatDriver();		 //initialize the fatInfo struct from boot sector
bool createFile(const char* path, bool dir);
bool deleteFile(const char* path);
bool deleteDir(const char* path);
void setPermission(DirEntry* entry, byte flags);  //set the FAT16 attribute flags on a file
qword getFreeSpace();
int numFilesInEntry(DirEntry* dir);  //get number of entries in dir
int numFilesInDir(const char* path);
int getMaxRootEntries();
DirEntry getRootDirEntry(int index); //get info about the nth root directory entry
bool walkPath(DirEntry* result, const char* path);
bool findFile(DirEntry* result, DirEntry* dir, const char* name, bool allowDir);
bool readClusterSec(word cluster, int n, Sector* sec); //load the nth sector
bool writeClusterSec(word cluster, int n, Sector* sec); //write out nth sector
bool isDirectory(DirEntry* entry);
qword getFreeSpace(); //get free disk space in bytes
word getNextCluster(word cluster); //read sector in fat and follow cluster chain
word allocCluster(word last, bool first); //mark a cluster as used and EOF, set previous last to point to new
void reallocChain(word first, size_t newSize); //add or remove clusters at end of file to get new size (bytes)
word allocChain(size_t size);
void deleteChain(word first);  //mark all clusters in chain as free (don't modify corresponding directory entry)
void flushFat(); //update FAT(s) on disk to reflect logical memory copy
int fatDiff();   //find the first difference (in sectors) between logical and acual, or -1 if they are the same
bool isValidFilename(const char* name); //does it fit in 8-3 filename? (include dot)
bool entryExists(DirEntry* entry); //is the entry a file or directory?
void compressRoot();
void compressDir(DirEntry* dir);
bool findFreeSlot(DirEntry* dir, EntrySlot* result);
dword getNextSector(dword prevSec);         //next sector in file, or 0 if at end
bool load83Name(char* shortName, const char* longName);
void loadDirEntryDefaults(DirEntry* dir);
bool createEntry(DirEntry* parent, DirEntry* newFile);  //NULL parent means root; can only fail if disk full
bool isLongNameEntry(DirEntry* entry);

int fileSecond(const char* name);
int fileMinute(const char* name);
int fileHour(const char* name);
int fileDay(const char* name);
int fileMonth(const char* name);
int fileYear(const char* name);

#endif
