#ifndef __ATA_DRV_H__
#define __ATA_DRV_H__

#include "globalDefines.h"

typedef struct
{
    byte data[512];
} Sector;

int ataInit();
int readsector(dword sector, byte* buf);
int writesector(dword sector, byte* buf);

#endif
