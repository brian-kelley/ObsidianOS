#include "fatdrv.h"

static struct
{
    word sectorSize;
    byte sectorsPerCluster;
    byte numFats;
    word maxRootEntries;
    dword totalSectors;
    word sectorsPerFat;
    dword hiddenSectors; //use as offset for all sector values (seems to be 0)
} fatInfo;

static dword fat1;  //first sector of first FAT
static dword fat2;  //first sector of second FAT
static dword rootStart; //first sector of root directory
static dword dataStart; //first sector of data area
static word* logicalFatBuf; //copy of FAT reflecting actual allocations etc.
static word* actualFatBuf;  //copy of the FAT on disk
static bool doubleFat;      //true if disk has 2 copies of fat, false if only one
static word numClusters;    //actual number of clusters in data area

void initFatDriver()
{
    byte sectorBuf[0x200];
    readsector(0, sectorBuf);
    fatInfo.sectorSize = *((word*) &sectorBuf[11]);
    fatInfo.sectorsPerCluster = sectorBuf[13];
    fatInfo.numFats = sectorBuf[16];
    doubleFat = fatInfo.numFats == 2;
    if(fatInfo.numFats > 2)
    {
	printString("\nError: Disk has more than two copies of the FAT table.\n");
	while(1);
    }
    fatInfo.maxRootEntries = *((word*) &sectorBuf[17]);
    fatInfo.totalSectors = *((word*) &sectorBuf[19]);
    if(fatInfo.totalSectors == 0)
	fatInfo.totalSectors = *((dword*) &sectorBuf[32]);
    fatInfo.sectorsPerFat = *((word*) &sectorBuf[22]);
    fatInfo.hiddenSectors = *((dword*) &sectorBuf[28]);
    if(fatInfo.sectorSize != 512)
    {
	printString("\nError: Disk formatted with sector size other than 512 bytes.\n");
	while(1); //halt, leave that message on the screen
    }
    fat1 = 1; //first sector after boot sector
    fat2 = fat1 + fatInfo.sectorsPerFat;
    rootStart = fat2 + fatInfo.sectorsPerFat;
    dataStart = rootStart + fatInfo.maxRootEntries * 32 / 512;
    numClusters = (fatInfo.totalSectors - dataStart) / fatInfo.sectorsPerCluster;
    logicalFatBuf = (word*) malloc(2 * numClusters);
    actualFatBuf = (word*) malloc(2 * numClusters);
    //load a copy into both buffers
    for(int i = 0; i < fatInfo.sectorsPerFat; i++)
    {
	readsector(fat1 + i, ((byte*) actualFatBuf) + i * 512; 
    }
    memcpy(logicalFatBuf, actualFatBuf, fatInfo.sectorsPerFat * 512);
}

int createDir(const char* path)
{
    //walk components of path and create subdirectories that don't exist
    char* tok = strtok(path, NULL);
}

int createFile(const char* path)
{
    
}

int deleteDir(const char* path)
{
    
}

int deleteFile(const char* path)
{
    
}

void setPermission(DirEntry* entry, byte flags)
{
    entry->attributes = flags;
}

bool isDirectory(DirEntry* entry)
{
    if(entry->attributes & FILE_ATTRIB.DIRECTORY)
	return true;
    else
	return false;
}

qword getFreeSpace()
{
    int numFreeClusters = 0;
    for(int i = 0; i < numClusters; i++)
    {
	if(logicalFatBuf[i] == 0)
	    numFreeClusters++;
    }
    return numFreeClusters * 512 * fatInfo.sectorsPerCluster;
}

word getNextCluster(word cluster)
{
    return logicalSectorBuf[cluster - 2];
}

void allocCluster(word last, bool first)
{
    //scan through FAT and find first free cluster
    int dest = -1;
    for(int i = 0; i < numClusters; i++)
    {
	if(logicalFatBuf[i] == 0)
	{
	    dest = i;
	    break;
	}
    }
    if(dest == -1)
    {
	printString("\nError: Not enough disk space to complete operation.\n");
	while(1);
    }
    logicalFatBuf[dest] = 0xFFFF;
    if(!first)
        logicalFatBuf[last - 2] = dest + 2;
}

int fatDiff()
{
    for(int i = 0; i < numClusters; i++)
    {
	if(logicalFatBuf[i] != actualFatBuf[i])
	{
	    //i / 256 gives the sector # within FAT
	    return i / 256;
	}
    }
    return -1;
}

void flushFat()
{
    //find all sectors containing updates, and write them
    int diff = 1;
    while(1)
    {
	diff = fatDiff();
	if(diff = -1)
	    break;
        writeSector(fat1 + diff, logicalFatBuf[diff * 256]);
	writeSector(fat2 + diff, logicalFatBuf[diff * 256]);
	memcpy(logicalFatBuf[diff * 256], actualFatBuf[diff * 256], 512);
    }
}

void reallocChain(word first, size_t newSize)
{
    size_t bytesPerCluster = 512 * fatInfo.sectorsPerCluster;
    size_t newClusters = newSize / bytesPerCluster;
    if(newSize % bytesPerCluster)
	newClusters++;
    //find the current length of the chain, to see if it will grow, shrink or not change
    int curClusters = 1;
    word iter = first;
    while(logicalFatBuf[iter - 2] != 0xFFFF)
    {
	iter = logicalFatBuf[iter = 2];
	curClusters++;
    }
    //iter now holds last cluster
    if(curClusters == newClusters)
    {
	return;
    }
    else if(curClusters < newClusters)
    {
	//allocate new clusters
	word newLast; //keep track of last cluster as chain grows
	for(int i = 0; i < newClusters - curClusters; i++)
	{
	    newLast = allocCluster(iter, false);
	}
    }
    else
    {
	//free clusters
	iter = first;
	for(int i = 0; i < curClusters - newClusters - 1; i++)
	{
	    iter = logicalFatBuf[iter - 2];
	}
	//iter holds new last cluster
	word temp = logicalFatBuf[iter - 2];
	logicalFatBuf[iter - 2] = 0xFFFF;
	deleteChain(temp);
    }
}

word allocChain(size_t size)
{
    size_t bytesPerCluster = 512 * fatInfo.sectorsPerCluster;
    size_t numClusters = size / bytesPerCluster;
    if(size % bytesPerCluster)
	numClusters++;
    if(numClusters == 0)
	numClusters = 1;
    word first = allocCluster(0, true);
    word iter = first;
    for(int i = 0; i < numClusters - 1; i++)
    {
	iter = allocCluster(iter, false);
    }
    return first;
}

void deleteChain(word first)
{
    word iter = first;
    word temp;
    while(iter != 0xFFFF)
    {
	temp = logicalFatBuf[iter - 2]; //set temp to be cluster after iter
	logicalFatBuf[iter - 2] = 0;    //mark iter as freed
	iter = temp;                    //advance iter
    }
    logicalFatBuf[iter - 2] = 0;        //free last cluster
}

DirEntry getRootDirEntry(int index)
{
    DirEntry rv;
    if(index < 0 || index >= fatInfo.maxRootEntries)
    {
	printString("Error: Root directory entry request out of range.");
	return rv;
    }
    byte sec[512];
    //get the sector #
    int sector = (index * 32) / 512;
    int sectorIndex = (index * 32) % 512;
    readsector(sector, sec);
    memcpy(&rv, sec + sectorIndex, 32);
    return rv;
}

bool walkPath(DirEntry* result, const char* path)
{
    char buf[50];
    char* compIter = (char*) path; //iterates over path components
    DirEntry searchDir;
    while(*compIter)
    {
	char* end = compIter;
	while(*end && *end != '/')
	{
	    end++;
	}
	//now, string in [compIter, end) gives this component
	//copy into buf and use as name string
	int index = 0;
	for(char* citer = compIter; citer < end; citer++)
        {
	    buf[index] = *citer;
	    index++;
	}
	buf[end - compIter] = 0;
	DirEntry temp;
	if(!findFile(&temp, &searchDir, buf, true))
	    return false;
        if(*end == 0)
	{
	    //end of path, temp has result
	    *result = temp;
	    return true;
	}
        else
	    //keep looking with 'temp' as search dir
	    searchDir = temp;
    }
    return false;
}

bool findFile(DirEntry* result, DirEntry* dir, const char* name, bool allowDir)
{
    char searchName[11]; //search for result with matching name
    //"name" must be 12 characters or less (8-1-3)
    if(strlen(name) > 12)                  //name too long
	return false;
    char* dotPos = (char*) strrchr(name, '.');
    if(dotPos == NULL && strlen(name) > 8) //no extension, name too long
	return false;
    else if(strrchr(name, 0) - dotPos > 4) //extension too long
	return false;
    char* iter = (char*) name;
    for(int i = 0; i < 8; i++)
    {
	if(*iter != '.' && *iter)
	{
	    searchName[i] = *iter;
	    iter++;
	}
	else
	    searchName[i] = ' '; //pad if no more actual characters
    }
    if(dotPos == NULL)
    {
	//no extension
	searchName[8] = ' ';
	searchName[9] = ' ';
	searchName[10] = ' ';
    }
    else
    {
	iter = dotPos + 1;
	for(int i = 0; i < 3; i++)
	{
	    if(*iter)
	    {
		searchName[8 + i] = *iter;
 		iter++;
	    } 
	    else
		searchName[8 + i] = ' ';
        }
    }
    //now have name to search for
    //use cluster # of search dir to get entries
    word dirCluster = dir->firstCluster;
    while(1)
    {
	//process this cluster
	for(int i = 0; i < fatInfo.sectorsPerCluster; i++)
	{
	    //process this sector
	    Sector sec = getSectorForCluster(dirCluster, i);
	    for(int j = 0; j < 512; j += 32) //bytes
	    {
		//process this dir entry
		DirEntry* thisEntry = (DirEntry*) &sec.data[j];
		char det = thisEntry->filename[0];
		if(det == 0 || det == 0xE5 || det == 0x2E)
		    continue;
		//compare filename
		if(strncmp(searchName, thisEntry->filename, 11) == 0)
		{
		    if(!allowDir && isDirectory(thisEntry))
			continue;
		    memcpy(result, thisEntry, 32);
		    return true;
	        }
	    }
	}
	if(logicalFatBuf[dirCluster - 2] == 0xFFFF)
	    //can't continue search, out of clusters in dir
	    return false;
	else
	    dirCluster = logicalFatBuf[dirCluster - 2];
    }
    return false;
}

Sector getSectorForCluster(word cluster, int n)
{
    Sector rv;
    dword sectorToRead = dataStart;
    sectorToRead += (dword) (cluster - 2) * fatInfo.sectorsPerCluster;
    sectorToRead += n;
    readsector(sectorToRead, rv.data);
    return rv;
}

bool isValidFilename(const char* name)
{
    char* nameBegin = name;
    if(strpbrk(name, ".\"*+,/:;<=>?\\[]|"))
	return false;
    if(strlen
}
