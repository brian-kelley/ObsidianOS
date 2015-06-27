#ifndef __FAT_DRV_H__
#define __FAT_DRV_H__

#include "globalDefines.h"
#include "atadrv.h"

void loatFatInfo();		 //initialize the fatInfo struct from boot sector
int createDir(const char* path); //these are recursive, creating creates all folders
int createFile(const char* path);//to get the entire path and deleting is recursive delete
int deleteFile(const char* path);
int deleteDir(const char* path);
unsigned long long int  getFreeSpace(); //get free space, in bytes, of filesystem
int setPermission(const char* path, byte flags);  //set the FAT16 attribute flags on a file

#endif
