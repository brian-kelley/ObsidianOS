#ifndef __FAT_DRV_H__
#define __FAT_DRV_H__

static struct fatInfo
{
    int numSectors; //total sectors on the drive
    int sectorsPerCluster;
    int bytesPerSector;
    int numFats;
    int sectorsPerFat;
    int rootCluster;	//"cluster number of root directory"
};

void loatFatInfo();

#endif