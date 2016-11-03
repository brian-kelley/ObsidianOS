#include "DataStruct.h"

//types are registered in this table
#define MAX_TYPES 25
static int numTypes;
static TypeEntry typeTable[MAX_TYPES];

static int intCompare(void* lhs, void* rhs)
{
    return *((int*) lhs) < *((int*) rhs);
}

static int uintCompare(void* lhs, void* rhs)
{
    return *((unsigned*) lhs) < *((unsigned*) rhs);
}

static int charCompare(void* lhs, void* rhs) {
    return *((char*) lhs) < *((char*) rhs);
}

static int byteCompare(void* lhs, void* rhs)
{
    return *((byte*) lhs) < *((byte*) rhs);
}

static int floatCompare(void* lhs, void* rhs)
{
    return *((float*) lhs) < *((float*) rhs);
}

static int doubleCompare(void* lhs, void* rhs)
{
    return *((double*) lhs) < *((double*) rhs);
}

static int stringCompare(void* lhs, void* rhs)
{
    const char* lstr = *((const char**) lhs);
    const char* rstr = *((const char**) rhs);
    return strcmp(lstr, rstr) == -1;
}

void initDataStructTypes()
{
    addTypeEntry(intCompare, sizeof(int), "int");
    addTypeEntry(uintCompare, sizeof(int), "unsigned");
    addTypeEntry(charCompare, sizeof(char), "char");
    addTypeEntry(byteCompare, sizeof(byte), "byte");
    addTypeEntry(floatCompare, sizeof(float), "float");
    addTypeEntry(doubleCompare, sizeof(double), "double");
    addTypeEntry(stringCompare, sizeof(char*), "string");
}

int registerType_(int(*compare)(void* lhs, void* rhs),
                  size_t size,
                  const char* type)
{
    if(numTypes == MAX_TYPES)
        return 1;   //error, out of space
    TypeEntry* te = &typeTable[numTypes];
    te->comp = compare;
    te->size = size;
    strncpy(te->name, type, MAX_TYPE_NAME_LENGTH);
    return 0;
}

Array arrMake(int type)
{
    Array arr;
    arr.type = type;
    arr.size = 0;
    arr.capacity = 10;
    arr.data = malloc(arr.capacity * typeTable[type].size);
    return arr;
}

void arrPush(Array* arr, void* elem)
{
    if(arr->size == arr->capacity)
    {
        //expand storage
        size_t newCap = arr->capacity * 2;
    }
}

int arrPop(Array* arr, void* elem)
{
}

int arrInsert(Array* arr, void* elem, size_t pos)
{
}

void arrClear(Array* arr)
{
}

void arrReserve(Array* arr, size_t num)
{
}

void arrShrinkToFit(Array* arr)
{

}
