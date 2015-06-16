#include "fatdrv.h"

void loatFatInfo()
{
    byte sectorBuf[512];
    //read the boot record sector into temporary stack buffer
    readsector(0, sectorBuf);
    word temp = *((word*) &sectorBuf[19]);
    if(temp == 0)
    {
	//num sectors is actually an int at 32 byte offset
	fatInto.numSectors = *((dword*) &sectorBuf[32]);
    }
    else
    {
	//temp is the # of sectors in volume
	fatInfo.numSectors = temp;
    }
    fatInfo.sectorsPerCluster = sectorBuf[13];
    fatInfo.bytesPerSector = *((word*) &sectorBuf[11]);
    fatInfo.numFats = sectorBuf[16];
    fatInfo.sectorsPerFat = *((dword*) &sectorBuf[36]);
    fatInfo.rootCluster = *((dword*) &sectorBuf[44]);
}