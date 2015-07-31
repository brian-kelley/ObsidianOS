#ifndef __ATA_DRV_H__
#define __ATA_DRV_H__

#include "globalDefines.h"

//Sector offset of primary partition (1 MB)
#define PARTITION_SEC 0x800

int ataInit();
int readsector(dword sector, byte* buf);
int writesector(dword sector, byte* buf);

#endif
