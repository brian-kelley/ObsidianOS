#include "memory.h"

#define LEVELS 4
#define PARENT_SAVES 20
#define NUM_MEGS 14

typedef struct
{
    int levels[LEVELS];
} Block;

static Block lastParents[LEVELS][PARENT_SAVES]; //just don't use first element (top-level)
static void* megMap = (void*) 0x200000; //location of start of 1 MiB level bitmap
//Offset of first sub-block (4-byte aligned, comes after bitmap table)
static const size_t subblockStart[LEVELS] = {64, 4, 20, 16};
//Block sizes for each level in bytes
static const size_t blockSize[LEVELS] = {0x100000, 65528, 1024, 16};
//Number of sub-blocks within each level
static int numSubblocks[LEVELS] = {NUM_MEGS, 16, 65, 63};

//List of the 2-bit codes in memory bitmap tables
enum BlockStatus
{
    FREE = 0,
    BEGIN = 1,
    MID_END = 2,
    SUBDIV = 3
};

void printBlock(Block b)
{
    //int* start = &b.levels[0];
    //printf("(%d %d %d %d)", start[0], start[1], start[2], start[3]);
}

static bool validBlock(Block b)
{
    for(int i = 0; i < LEVELS; i++)
    {
        if(b.levels[i] >= numSubblocks[i])
            return false;
    }
    if(b.levels[0] == -1)
        return false;
    return true;
}

//Precondition: b is valid. Function doesn't check.
static int getBlockLevel(Block b)
{
    //int level = 0;
    for(int i = 1; i < LEVELS; i++)
    {
        if(b.levels[i] == -1)
        {
            return i - 1;
        }
    }
    //last level, all values in Block struct are used
    return LEVELS - 1;
}

static Block getParentBlock(Block b)
{
    int level = getBlockLevel(b);
    b.levels[level] = -1;
    return b;
}

//Get the memory location referenced by block
//Return NULL for invalid block descriptor
static void* getBlockPtr(Block b)
{
    void* ptr = (void*) megMap;
    if(!validBlock(b))
        return NULL;
    int level = getBlockLevel(b);
    for(int i = 0; i <= level; i++)
    {
        //For each level, add the offset for table as well as the memory for
        //blocks before this one
        ptr += subblockStart[i];
        ptr += b.levels[i] * blockSize[i];
    }
    return ptr;
}

//preconditions: b is a valid block, b can be subdivided (level < LEVELS - 1)
static Block getFirstSubblock(Block b)
{
    Block sub = b;
    int level = getBlockLevel(b);
    sub.levels[level + 1] = 0;
    return sub;
}

//Return next block on the same level
static Block getNextBlock(Block b)
{
    int level = getBlockLevel(b);
    b.levels[level]++;
    return b;
}

static Block getNthSubblock(Block parent, int index)
{
    Block sub = getFirstSubblock(parent);
    int level = getBlockLevel(sub);
    sub.levels[level] = index;
    return sub;
}

//Return a BlockStatus value or -1 on error
static int getBlockStatus(Block b)
{
    if(!validBlock(b))
        return -1;
    int level = getBlockLevel(b);
    byte* table; //start of table for parent blocks, or top-level block.
    byte mask = 0b11;
    int shift;
    //set table to address of block status bitmap of parent
    if(level == 0)
        table = (byte*) megMap;
    else
        table = (byte*) getBlockPtr(getParentBlock(b));
    int baseIndex = b.levels[level];
    byte entry = *(table + (baseIndex / 4)); //2 bits per entry
    shift = 6 - (2 * (baseIndex % 4));  //trust me it works
    return (entry & (mask << shift)) >> shift;
}

static void setBlockStatus(Block b, int code)
{
    int level = getBlockLevel(b);
    byte* table; //start of table for parent blocks, or top-level block.
    byte mask = 0b11;
    if(level == 0)
        table = (byte*) megMap;
    else
        table = (byte*) getBlockPtr(getParentBlock(b));
    int baseIndex = b.levels[level];
    byte* entry = table + (baseIndex / 4); //2 bits per entry
    int shift = 6 - (2 * (baseIndex % 4));  //trust me it works
    byte newVal = *entry & ~(mask << shift);
    newVal |= (code << shift);
    *entry = newVal;
}

//Find a group of free level 0 blocks
static bool findTopLevelGroup(int numBlocks, Block* result)
{
    int bestStart = -1;
    int bestSize = NUM_MEGS + 1;
    Block iter = {{0, -1, -1, -1}};
    while(iter.levels[0] < NUM_MEGS)
    {
        //scan to next free block
        while(getBlockStatus(iter) != FREE && iter.levels[0] < NUM_MEGS)
            iter.levels[0]++;
        //start counting the run
        int runStart = iter.levels[0];
        //scan to end of run
        while(getBlockStatus(iter) == FREE && iter.levels[0] < NUM_MEGS)
            iter.levels[0]++;
        //save the run as best if it fits numBlocks and is smaller than last best
        int runSize = iter.levels[0] - runStart;
        if(runSize < bestSize && runSize >= numBlocks)
        {
            bestStart = runStart;
            bestSize = runSize;
        }
    }
    if(bestStart == -1)
        return false;
    iter.levels[0] = bestStart;
    *result = iter;
    return true;
}

//Precondition: newParent is not LEVELS-1 (can be subdivided)
static void allocForSub(Block newParent)
{
    int parLevel = getBlockLevel(newParent);
    //Mark newParent in its bitmap
    setBlockStatus(newParent, SUBDIV);
    for(int i = 0; i < numSubblocks[parLevel + 1]; i++)
    {
        Block sub = getNthSubblock(newParent, i);
        setBlockStatus(sub, FREE);
    }
}

//Find best contiguous group of subblocks that can hold numBlocks
//Preconditions: parent is already allocated for 
static bool findGroupInBlock(Block parent, int numBlocks, Block* result, int* overshoot)
{
    //printf("Searching for group of free blocks in ");
    //printBlock(parent);
    //puts("");
    if(getBlockStatus(parent) != SUBDIV)
    {
        //puts("FATAL: parent is not subdivided!");
        return false;
    }
    int parentLevel = getBlockLevel(parent);
    if(parentLevel == LEVELS - 1)
    {
        //printf("FATAL ERROR: findGroupInBlock: parent level is level 3!");
        return false;
    }
    int numSub = numSubblocks[parentLevel];
    int bestStart = -1;
    int bestSize = numSub + 1;
    Block iter = getNthSubblock(parent, 0);
    int lvl = parentLevel + 1;
    while(iter.levels[lvl] < numSub)
    {
        //scan to next free block
        while(getBlockStatus(iter) != FREE && iter.levels[lvl] < numSub)
            iter.levels[lvl]++;
        //start counting the run
        int runStart = iter.levels[lvl];
        //scan to end of run
        while(getBlockStatus(iter) == FREE && iter.levels[lvl] < numSub)
            iter.levels[lvl]++;
        //save the run as best if it fits numBlocks and is smaller than last best
        int runSize = iter.levels[lvl] - runStart;
        if(runSize < bestSize && runSize >= numBlocks)
        {
            bestStart = runStart;
            bestSize = runSize;
        }
    }
    if(bestStart == -1)
        return false;
    *result = getNthSubblock(parent, bestStart);
    if(overshoot)
        *overshoot = bestSize - numBlocks;
    return true;
}

static int getMaxGroup(Block b)
{
    int bestNum = 0;
    int num = 0;
    for(int i = 0; i < numSubblocks[getBlockLevel(b)]; i++)
    {
        if(getBlockStatus(b) == FREE)
            num++;
        else
        {
            if(num > bestNum)
                bestNum = num;
            num = 0;
        }
    }
    return bestNum;
}

static void allocGroup(Block first, int num)
{
    if(num == 1)
    {
        setBlockStatus(first, MID_END);
        return;
    }
    else
    {
        int* increment = &first.levels[getBlockLevel(first)];
        setBlockStatus(first, BEGIN);
        for(int i = 0; i < num - 1; i++)
        {
            (*increment)++;
            setBlockStatus(first, MID_END);
        }
    }
}

static bool replaceParentSave(Block* parentSave, int level);

static bool groupSearch(int level, int numBlocks, Block parentBlock, Block* result)
{
    int parLevel = getBlockLevel(parentBlock);
    if(parLevel == level - 1)
    {
        if(findGroupInBlock(parentBlock, numBlocks, result, NULL))
            return true;
    }
    for(int i = 0; i < numSubblocks[parLevel]; i++)
    {
        Block sub = getNthSubblock(parentBlock, i);
        if(getBlockStatus(sub) == SUBDIV)
        {
            if(groupSearch(level, numBlocks, sub, result))
                return true;
        }
    }
    return false;
}

static bool allocBlocks(int level, int numBlocks, Block* result)
{
    //printf("Allocating %d blocks at level %d\n", numBlocks, level);
    if(level == 0)
    {
        //printf("  Will attempt to find top-level group of %d blocks.\n", numBlocks);
        return findTopLevelGroup(numBlocks, result);
    }
    //Need to find subblock(s)
    //Best to worst options:
    //-Use same parent block from last allocation
    //-Allocate new parent block somewhere else
    //-Depth-first search subdivided blocks looking for free blocks
    //-Return false
    //First try lastParents
    int bestOvershootParent = -1;
    int bestOvershoot = 0xFFFF;
    int overshoot = 0;
    for(int i = 0; i < PARENT_SAVES; i++)
    {
        if(validBlock(lastParents[level][i]))
        {
            if(findGroupInBlock(lastParents[level][i], numBlocks, result, &overshoot))
            {
                if(overshoot < bestOvershoot)
                {
                    bestOvershoot = overshoot;
                    bestOvershootParent = i;
                }
            }
        }
    }
    if(bestOvershootParent != -1)
    {
        Block use = lastParents[level][bestOvershootParent];
        if(!findGroupInBlock(use, numBlocks, result, NULL))
        {
            //This should never ever happen
            //puts("VERY FATAL ERROR: CHECK FOUND LAST_PARENT CASE");
            return false;
        }
        //puts("Allocation successful via lastParents");
        //printf("Resulting block group: ");
        //printBlock(*result);
        //puts("");
        allocGroup(*result, numBlocks);
        return true;
    }
    //lastParents must be updated
    //If one is not yet valid, set it to a newly allocated block
    //Immediately use subblocks of that new block to fulfill current request
    //puts("Did not find good cached parent block.");
    bool allValid = true;
    Block* toReplace;
    for(int i = 0; i < PARENT_SAVES; i++)
    {
        if(!validBlock(lastParents[level][i]))
        {
            allValid = false;
            toReplace = &lastParents[level][i];
            break;
        }
    }
    if(!allValid)
    {
        bool success = replaceParentSave(toReplace, level - 1);
        if(success)
        {
            //use *toReplace (now a fresh subdivded block) as parent
            *result = getFirstSubblock(*toReplace);
            allocGroup(*result, numBlocks);
            //puts("Allocation successful via newParent");
            return true;
        }
    }
    else
    {
        int worstGroup = 100;
        int worstBlock = -1;
        for(int i = 0; i < PARENT_SAVES; i++)
        {
            int thisGroup = getMaxGroup(lastParents[level][i]);
            if(thisGroup < worstGroup)
            {
                worstGroup = thisGroup;
                worstBlock = i;
            }
        }
        toReplace = &lastParents[level][worstBlock];
        //replace worstBlock
        bool success = replaceParentSave(toReplace, level - 1);
        if(success)
        {
            *result = getFirstSubblock(*toReplace);
            allocGroup(*result, numBlocks);
            return true;
            //puts("Allocation sucessful via new parent.");
        }
    }
    //If here, then depth-first search for an appropriate parent block
    //that isn't necesarily listed in lastParent.
    Block first;
    for(int i = 0; i < numSubblocks[0]; i++)
    {
        Block topLevel = {{i, -1, -1, -1}};
        if(getBlockStatus(topLevel) == SUBDIV)
        {
            if(groupSearch(level, numBlocks, topLevel, &first))
            {
                *result = first;
                allocGroup(*result, numBlocks);
                return true;
                //puts("Allocation successful via worst-case dfs for parent");
            }
        }
    }
    //puts("Block group allocation failed!");
    return false;
}

static bool replaceParentSave(Block* parentSave, int level)
{
    Block replacement;
    bool success = allocBlocks(level, 1, &replacement);
    setBlockStatus(replacement, SUBDIV);
    if(!success)
        return false;
    *parentSave = replacement;
    allocForSub(*parentSave);
    //have a new (allocated) parent block
    //set the allocation status of that to subdiv
    return true;
}

void* mmAlloc(size_t size)
{
    /*
    puts("");
    for(int j = 0; j < 60; j++)
        putchar('*');
    puts(""); */
    //printf("Allocating %lu bytes\n", size);
    int parentLevel = 2;  //start with smallest block and count up
    while(blockSize[parentLevel] < size)
    {
        parentLevel--;
    }
    int level = parentLevel + 1;
    if(level < 0 || level >= LEVELS)
    {
        //puts("WARNING: Couldn't pick a good block size.");
        level = 0;
    }
    size_t bsize = blockSize[level];
    int numBlocks = size / bsize;
    if(size % bsize)
        numBlocks++;
    //printf("Allocating a group of %d blocks of level %d\n", numBlocks, level);
    Block first;
    bool success = allocBlocks(level, numBlocks, &first);
    if(success)
    {
        //printf("Allocated %lu bytes, block: ", size);
        //printBlock(first);
        //puts("");
        return getBlockPtr(first);
    }
    else
    {
        //puts("Allocation failed!");
        return NULL;
    }
}

void mmFree(void* mem)
{
    Block b = {{-1, -1, -1, -1}};
    size_t offset = (size_t) (mem - megMap); //total offset in memory area
    for(int i = 0; i < LEVELS; i++)
    {
        b.levels[i] = (offset - subblockStart[i]) / blockSize[i];
        if(b.levels[i] * blockSize[i] + subblockStart[i] == offset)
            break;
        offset -= (b.levels[i] * blockSize[i] + subblockStart[i]);
    }
    //b is the first block of group
    bool haveParent = false;
    Block parent;
    if(getBlockLevel(b) != 0)
    {
        haveParent = true;
        parent = getParentBlock(b);
    }
    while(getBlockStatus(b) != MID_END)
    {
        setBlockStatus(b, FREE);
        b = getNextBlock(b);
    }
    setBlockStatus(b, FREE);
    if(haveParent)
    {
        //check if parent can also be freed
        if(getMaxGroup(parent) == numSubblocks[getBlockLevel(parent)])
        {
            setBlockStatus(parent, FREE);
        }
    }
}

/*
Functions in the public interface
*/

//Called by kernel early in initialization
//Mark all dynamic memory as free, set lastParent to invalid blocks
void initMM()
{
    for(int i = 0; i < numSubblocks[0]; i++)
    {
        Block b = {{i, -1, -1, -1}};
        setBlockStatus(b, FREE);
    }
    Block nullBlock = {{-1, -1, -1, -1}};
    for(int i = 0; i < LEVELS; i++)
    {
        for(int j = 0; j < PARENT_SAVES; j++)
        {
            lastParents[i][j] = nullBlock;
        }
    }
}

void initBuf(void* newMegMap)
{
    megMap = newMegMap;
}
