#include "fatdrv.h"

static struct
{
    unsigned int numSectors; //total sectors on the drive
    unsigned int sectorsPerCluster;
    unsigned int bytesPerSector;
    unsigned int numFats;
    unsigned int sectorsPerFat;
} fatInfo;

void loatFatInfo()
{
    byte sectorBuf[0x200];
    //read the boot record sector into temporary stack buffer
    readsector(0, sectorBuf); //have boot sector loaded
    word temp = (word) sectorBuf[0x13] | ((word) sectorBuf[0x14] << 8);
    if(temp == 0)
    {
	//num sectors is actually an int at 32 byte offset
	fatInfo.numSectors = (dword) sectorBuf[0x20] | ((dword) sectorBuf[0x21] << 8) | ((dword) sectorBuf[0x22] << 16) || ((dword) sectorBuf[0x23] << 24);
    }
    else
    {
	//temp is the # of sectors in volume
	fatInfo.numSectors = temp;
    }
    fatInfo.sectorsPerCluster = sectorBuf[0xD];
    fatInfo.bytesPerSector = (word) sectorBuf[0xB] | ((word) sectorBuf[0xC] << 8);
    fatInfo.numFats = sectorBuf[0x10];
    fatInfo.sectorsPerFat = (word) sectorBuf[0x16] | ((word) sectorBuf[0x17] << 8);
}

