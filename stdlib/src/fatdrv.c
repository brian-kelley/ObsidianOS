#include "fatdrv.h"

static struct
{
  word sectorSize;
  byte spc;		//sectors per cluster
  byte numFats;
  word maxRootEntries;
  dword totalSectors;
  word sectorsPerFat;
  dword hiddenSectors; //first sector of partition
  dword reserved;      //# of reserved sectors between BPB and 1st FAT
} fatInfo;

static dword fat1;  //first sector of first FAT
static dword fat2;  //first sector of second FAT
static dword rootStart; //first sector of root directory
static dword dataStart; //first sector of data area
static word* logicalFatBuf; //copy of FAT reflecting actual allocations etc.
static word* actualFatBuf;  //copy of the FAT on disk
static bool doubleFat;      //true if disk has 2 copies of fat, false if only one
static word numClusters;    //actual number of clusters in data area
static const char* invalidChars = ".\"*+,/:;<=>?\\[]|";

void initFatDriver()
{
  byte sectorBuf[0x200];
  readsector(0, sectorBuf);
  fatInfo.sectorSize = *((word*) &sectorBuf[11]);
  fatInfo.spc = sectorBuf[13];
  fatInfo.reserved = *((word*) &sectorBuf[14]);
  fatInfo.numFats = sectorBuf[16];
  doubleFat = fatInfo.numFats == 2;
  if(fatInfo.numFats != 2)
  {
    //TODO: Handle >2 fats if there is ever a use case for that
    puts("Error: The FAT16 driver requires exactly 2 FATs.");
    while(1);   //deliberately hang the system
  }
  fatInfo.maxRootEntries = *((word*) &sectorBuf[17]);
  fatInfo.totalSectors = *((word*) &sectorBuf[19]);
  if(fatInfo.totalSectors == 0)
    fatInfo.totalSectors = *((dword*) &sectorBuf[32]);
  fatInfo.sectorsPerFat = *((word*) &sectorBuf[22]);
  fatInfo.hiddenSectors = *((dword*) &sectorBuf[28]);
  if(fatInfo.sectorSize != 512)
  {
    puts("\nError: Disk formatted with sector size other than 512 bytes.\n");
    while(1); //halt, leave that message on the screen
  }
  //printf("%i hidden sectors\n", fatInfo.hiddenSectors);
  //printf("%i reserved sectors\n", fatInfo.reserved);
  fat1 = fatInfo.reserved; //first sector after boot sector
  fat2 = fat1 + fatInfo.sectorsPerFat;
  rootStart = fat2 + fatInfo.sectorsPerFat;
  int offset = rootStart * 512;
  /*
  printf("root starts at byte %#x\n", offset);
  printf("fat1 starts at %i\n", fat1);
  printf("fat2 starts at %i\n", fat2);
  printf("root entries start at sector %i\n", rootStart);
  printf("clusters start at %i\n", dataStart);
  */
  dataStart = rootStart + fatInfo.maxRootEntries * 32 / 512;
  numClusters = (fatInfo.totalSectors - dataStart) / fatInfo.spc;
  logicalFatBuf = (word*) malloc(2 * numClusters);
  actualFatBuf = (word*) malloc(2 * numClusters);
  //load a copy into both buffers
  for(int i = 0; i < fatInfo.sectorsPerFat; i++)
  {
    readsector(fat1 + i, sectorBuf);
    for(int j = 0; j < 256; j++)
    {
      if(i * 256 + j < numClusters)
        logicalFatBuf[i * 256 + j] = *((word*) &sectorBuf[j * 2]);
      else
        break;
    }
  }
  memcpy(logicalFatBuf, actualFatBuf, 2 * numClusters);
}

bool createFile(const char* path, bool dir)
{
  //walk components of path and create necessary subdirectories
  char copy[512];
  size_t plen = strlen(path);
  if(plen > 511)
  {
    puts("Error creating directory: path too long.");
    return false;
  }
  memcpy(copy, path, plen);
  copy[plen] = 0;
  int numElem = 1;
  for(char* iter = copy; *iter; iter++)
  {
    if(*iter == '/')
      numElem++;
  }
  const char* delim = "/";
  char* tok = strtok((char*) copy, delim);
  byte sec[512];
  DirEntry iter;
  DirEntry check;
  for(int i = 0; i < numElem; i++)
  {
    if(tok == NULL)
      break;
    char shortName[11];
    if(!load83Name(shortName, tok))
    {
      puts("Invalid filename!");
      return false;
    }
    //Determine if <iter>/<next> exists
    dword secNum = dataStart + (iter.firstCluster - 2) * fatInfo.spc;
    readsector(secNum, sec);
    bool match = false;
    do
    {
      for(int j = 0; j < 512; j += 32)
      {
        check = *((DirEntry*) &sec[j]);
        if(entryExists(&iter) && memcmp(&check.filename, shortName, 11) == 0)
        {
          //match!
          iter = check;
          match = true;
          break;
        }
      }
      if(match)
        break;
      secNum = getNextSector(secNum);
      if(secNum == 0)
        break;
      readsector(secNum, sec);
    }
    while(true);
    //end of directory, target was not found, create it
    if(!match)
    {
      DirEntry newEntry;
      //memcpy(shortName, 11);
    }
  }
  return true;
}

bool deleteDir(const char* path)
{
  return true;
}

bool deleteFile(const char* path)
{
  return true;
}

void setPermission(DirEntry* entry, byte flags)
{
  entry->attributes = flags;
}

bool isDirectory(DirEntry* entry)
{
  if(entry->attributes & FA_DIRECTORY)
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
  return numFreeClusters * 512 * fatInfo.spc;
}

word getNextCluster(word cluster)
{
  return logicalFatBuf[cluster - 2];
}

word allocCluster(word last, bool first)
{
  //scan through FAT and find first free cluster
  int dest = -1;
  for(int i = 0; i < (int) numClusters; i++)
  {
    if(logicalFatBuf[i] == 0)
    {
      dest = i;
      break;
    }
  }
  if(dest == -1)
  {
    puts("\nError: Not enough disk space to complete operation.\n");
    return 0xFFFF;
  }
  logicalFatBuf[dest] = 0xFFFF;
  if(!first)
    logicalFatBuf[last - 2] = (word) (dest + 2);
  return dest + 2;
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
    if(diff == -1)
      break;
    writesector(fat1 + diff, (byte*) &logicalFatBuf[diff * 256]);
    writesector(fat2 + diff, (byte*) &logicalFatBuf[diff * 256]);
    memcpy(&logicalFatBuf[diff * 256], &actualFatBuf[diff * 256], 512);
  }
}

void reallocChain(word first, size_t newSize)
{
  size_t bytesPerCluster = 512 * fatInfo.spc;
  word newClusters = (word) (newSize / bytesPerCluster);
  if(newSize % bytesPerCluster)
    newClusters++;
  //find the current length of the chain, to see if it will grow, shrink or not change
  word curClusters = 1;
  word iter = first;
  while(logicalFatBuf[iter - 2] != 0xFFFF)
  {
    iter = logicalFatBuf[iter - 2];
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
    for(int i = 0; i < newClusters - curClusters; i++)
      iter = allocCluster(iter, false);
  }
  else
  {
    //free clusters
    iter = first;
    for(word i = 0; i < curClusters - newClusters - 1; i++)
      iter = logicalFatBuf[iter - 2];
    //iter holds new last cluster
    word temp = logicalFatBuf[iter - 2];
    logicalFatBuf[iter - 2] = 0xFFFF;
    deleteChain(temp);
  }
}

word allocChain(size_t size)
{
  size_t bytesPerCluster = 512 * fatInfo.spc;
  word chainClusters = (word) (size / bytesPerCluster);
  if(size % bytesPerCluster)
    chainClusters++;
  //file of size 0 bytes still has gets one cluster
  if(chainClusters == 0)
    chainClusters = 1;
  word first = allocCluster(0, true);
  word iter = first;
  for(word i = 0; i < chainClusters - 1; i++)
    iter = allocCluster(iter, false);
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
    puts("Error: Root directory entry request out of range.");
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
  Sector sec;
  while(1)
  {
    //process this cluster
    for(int i = 0; i < fatInfo.spc; i++)
    {
      //process this sector
      readClusterSec(dirCluster, i, &sec);
      for(int j = 0; j < 512; j += 32) //bytes
      {
        //process this dir entry
        DirEntry* thisEntry = (DirEntry*) &sec.data[j];
        //byte det = thisEntry->filename[0];
        if(!entryExists(thisEntry))

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
    if(logicalFatBuf[dirCluster - 2] >= 0xFFF8)
      //can't continue search, out of clusters in dir
      return false;
    else
      dirCluster = logicalFatBuf[dirCluster - 2];
  }
  return false;
}

bool readClusterSec(word cluster, int n, Sector* sec)
{
  //bounds check the given cluster/sector #
  if(cluster < 2 || cluster > numClusters + 2)
    return false;
  if(n < 0 || n > fatInfo.spc)
    return false;
  dword sectorToRead = dataStart;
  sectorToRead += (dword) (cluster - 2) * fatInfo.spc;
  sectorToRead += n;
  readsector(sectorToRead, sec->data);
  return true;
}

bool writeClusterSec(word cluster, int n, Sector* sec)
{
  if(cluster < 2 || cluster > numClusters + 2)
    return false;
  if(n < 0 || n > fatInfo.spc)
    return false;
  dword sectorToWrite = dataStart;
  sectorToWrite += (dword) (cluster - 2) * fatInfo.spc;
  sectorToWrite += n;
  writesector(sectorToWrite, sec->data);
  return true;
}

bool isValidFilename(const char* name)
{
  if(strpbrk((char*) name, invalidChars))
    return false;
  return true;
}

int numFilesInEntry(DirEntry* dir)
{
  if(!entryExists(dir) || !isDirectory(dir))
    return 0;
  //find cluster
  word cluster = dir->firstCluster;
  Sector sec;
  int num = 0;
  while(1)
  {
    for(int i = 0; i < fatInfo.spc; i++)
    {
      readClusterSec(cluster, i, &sec);
      for(int j = 0; j < 512; j += 32)
      {
        DirEntry* entry = (DirEntry*) &sec.data[j];
        if(entryExists(entry))
          num++;
      }
    }
    //decide if there is another cluster to search
    if(logicalFatBuf[cluster - 2] != 0xFFFF)
      cluster = logicalFatBuf[cluster - 2];
    else
      break;
  }
  return num;
}

int numFilesInDir(const char* path)
{
  DirEntry dir;
  if(!walkPath(&dir, path))
    return 0;
  return numFilesInEntry(&dir);
}

int fileSecond(const char* name)
{
  DirEntry dir;
  if(!walkPath(&dir, name))
    return -1;
  return ((int) dir.time & 0b11111) * 2;
}

int fileMinute(const char* name)
{
  DirEntry dir;
  if(!walkPath(&dir, name))
    return -1;
  return ((int) dir.time & 0b11111100000) >> 5;
}

int fileHour(const char* name)
{
  DirEntry dir;
  if(!walkPath(&dir, name))
    return -1;
  return ((int) dir.time & 0b1111100000000000) >> 11;
}

int fileDay(const char* name)
{
  DirEntry dir;
  if(!walkPath(&dir, name))
    return -1;
  return (int) dir.date & 0b11111;
}

int fileMonth(const char* name)
{
  DirEntry dir;
  if(!walkPath(&dir, name))
    return -1;
  return ((int) dir.date & 0b111100000) >> 5;
}

int fileYear(const char* name)
{
  DirEntry dir;
  if(!walkPath(&dir, name))
    return -1;
  return 1980 + (((int) dir.date & 0b1111111000000000) >> 9);
}

bool entryExists(DirEntry* entry)
{
  byte det = entry->filename[0];
  if(det == 0 || det == 0xE5 || det == 0x2E)
    return false;
  return true;
}

void compressDir(DirEntry* dir)
{
  //first, see if a cluster could be saved (if not, don't do anything)
  if(!entryExists(dir) || !isDirectory(dir))
    return;
  int dirClusters = 1;
  word cluster = dir->firstCluster;
  while(1)
  {
    if(logicalFatBuf[cluster - 2] != 0xFFFF)
    {
      cluster = logicalFatBuf[cluster - 2];
      dirClusters++;
    }
    else
      break;
  }
  //now get the smallest possible size, in bytes
  size_t idealSize = numFilesInEntry(dir) * 32;
  int bytesPerCluster = 512 * fatInfo.spc;
  int idealClusters = idealSize / bytesPerCluster;
  if(idealSize % bytesPerCluster)
    idealClusters++;
  if(idealClusters == dirClusters)
    return; //no space will be saved
  //actually do the compression
  //make a copy of numClusters (curClusters)
  //while the last cluster is not empty, take DirEntries from the last cluster and move them into empty slots found in the first clusters.
  //decrement curClusters
  //if curClusters is still greater than idealClusters, repeat process for new last cluster
  //call reallocChain to remove the freed sectors (given by numClusters - curClusters)
  //flush FAT so clusters can't be 'leaked' in case of power loss etc
  int curClusters = dirClusters;
  //DirEntry entryToCopy; //hold space for entries being moved into other clusters
  Sector srcSec; //sector in cluster that will be freed
  Sector origSrcSec; //copy of src as it is on disk, compare to see if updated
  Sector dstSec; //sector in cluster earlier in chain with free slots
  while(curClusters > idealClusters)
  {
    //seek to last cluster
    word lastCluster = dir->firstCluster;
    while(logicalFatBuf[lastCluster - 2] != 0xFFFF)
      lastCluster = logicalFatBuf[lastCluster - 2];
    for(int i = 0; i < fatInfo.spc; i++)
    {
      readClusterSec(lastCluster, i, &srcSec);
      memcpy(origSrcSec.data, srcSec.data, 512);
      for(int j = 0; j < 512; j += 32)
      {
        DirEntry* thisEntry = (DirEntry*) &srcSec.data[j];
        if(entryExists(thisEntry))
        {
          //now start at first cluster again, and find free entry slot
          EntrySlot slot;
          if(!findFreeSlot(dir, &slot))
            return; //this shouldn't happen, but abort if it does
          //load the dest sector, and copy the entry
          readClusterSec(slot.cluster, slot.sector, &dstSec);
          memcpy((void*) &dstSec.data[slot.index * 16], (void*) thisEntry, 32);
          //write back the dest sector (TODO: cache it in case it's the same for multiple entries?)
          writeClusterSec(slot.cluster, slot.sector, &dstSec);
          //zero out the slot in the src sector
          memset((void*) thisEntry, 0, 32);
        }
      }
      //done with cluster, write it back if it changed at all
      if(!memcmp(srcSec.data, origSrcSec.data, 512))
        writeClusterSec(lastCluster, i, &srcSec);
    }
    curClusters--;
  }
}

bool findFreeSlot(DirEntry* dir, EntrySlot* result)
{
  if(!entryExists(dir) || !isDirectory(dir))
    return false;
  word cluster = dir->firstCluster;
  Sector sec;
  while(1)
  {
    for(word i = 0; i < (word) fatInfo.spc; i++)
    {
      readClusterSec(cluster, i, &sec);
      for(int j = 0; j < 512; j += 32)
      {
        DirEntry* thisDest = (DirEntry*) &sec.data[j];
        if(!entryExists(thisDest))
        {
          result->cluster = cluster;
          result->sector = (word) i;
          result->index = (word) (j / 32);
          return true;
        }
      }
    }
    if(logicalFatBuf[cluster - 2] == 0xFFFF)
      return false;
    else
      cluster = logicalFatBuf[cluster - 2];
  }
  return false;
}

void compressRoot()
{
  //Iterate through root dir entries (backward) and move to front empty slots
  //Read entire root directory
  byte* root = malloc(fatInfo.maxRootEntries * 32);
  if(root == NULL)
  {
    puts("Out of memory!");
    return;
  }
  int numSec = fatInfo.maxRootEntries * 32 / 512;
  for(int i = 0; i < numSec; i++)
  {
    readsector(rootStart + i, &root[i * 512]);
  }
  bool done = false;        //have all entries been placed in optimal slot?
  bool modified = false;    //do the root sectors need to be written back?
  for(int i = fatInfo.maxRootEntries - 1; i >= 0; i--)
  {
    DirEntry* entry = (DirEntry*) &root[i * 32];
    if(entryExists(entry))
    {
      //find first empty slot in root
      for(int j = 0; j < fatInfo.maxRootEntries; j++)
      {
        if(j >= i)
        {
          done = true;
          break;
        }
        DirEntry* possibleSlot = (DirEntry*) &root[j * 32];
        if(!entryExists(possibleSlot))
        {
          //Found empty slot, move entry to it
          memcpy(possibleSlot, entry, 32);
          memset(entry, 0, 32);       //mark old slot as free
          modified = true;
        }
      }
    }
    if(done)
      break;
  }
  if(modified)
  {
    //write back sectors
    for(int i = 0; i < numSec; i++)
    {
      writesector(rootStart + i, &root[i * 512]);
    }
  }
  free(root);
}

dword getNextSector(dword prevSec)
{
  dword offset = prevSec - dataStart;
  if(offset % fatInfo.spc < (dword) fatInfo.spc - 1)
    return prevSec + 1;	  //next sector in same cluster
  dword cluster = (prevSec - dataStart) / fatInfo.spc;
  dword nextCluster = logicalFatBuf[cluster - 2];
  if(nextCluster >= 0xFFF8)
    return 0;		  //end of file
  return nextCluster * fatInfo.spc;
}

bool load83Name(char* shortName, const char* longName)
{
  //check for illegal characters
  if(!isValidFilename(longName))
    return false;
  size_t longLen = strlen(longName);
  const char* dot = strrchr(longName, '.'); //find rightmost dot for ext separator
  size_t stemLen = longLen - (dot - longName);
  //If > 3 chars after '.', assume '.' is part of the stem
  if(dot == NULL || longLen - stemLen > 3)
  {
    //no extension; longName is only stem
    if(longLen > 8)
      return false;
    int i;
    for(i = 0; i < longLen; i++)
      shortName[i] = toupper(longName[i]);
    for(; i < 11; i++)
      shortName[i] = ' ';
    return true;
  }
  //Otherwise, assume '.' is between stem and ext
  if(stemLen > 8)
    return false;
  size_t extLen = longLen - 1 - stemLen;
  int i;
  for(i = 0; i < stemLen; i++)
    shortName[i] = toupper(longName[i]);
  for(; i < 8; i++)
    shortName[i] = ' ';
  for(i = 0; i < extLen; i++)
    shortName[8 + i] = toupper(*(dot + 1 + i));
  for(; i < 3; i++)
    shortName[8 + i] = ' ';
  return true;
}

bool createEntry(DirEntry* parent, DirEntry* newFile)
{
  EntrySlot slot;
  if(!findFreeSlot(parent, &slot))
  {
    int newCluster = allocCluster(0, true);
    int iter = logicalFatBuf[parent->firstCluster];
    while(true)
    {
      int next = logicalFatBuf[iter];
      if(next >= 0xFFF8)
        break;
      else
        iter = next;
    }
    logicalFatBuf[iter] = newCluster;
    logicalFatBuf[newCluster] = 0xFFFF;
    findFreeSlot(parent, &slot);
  }
  return true;
}

int getMaxRootEntries()
{
  return fatInfo.maxRootEntries;
}

