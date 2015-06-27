#ifndef __ATA_DRV_H__
#define __ATA_DRV_H__

#include "globalDefines.h"

//need port i/o for this, makefile links libc.o to port.o
extern void writeport(dword portNum, dword value);
extern byte readport(dword portNum);
extern void writeportw(dword portNum, dword value);
extern byte readportw(dword portNum);

int ataInit();
int readsector(dword sector, byte* buf);
int writesector(dword sector, byte* buf);

#endif
