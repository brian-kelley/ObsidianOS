#include "globalDefines.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

//Tiny Container Library
//array, bitset, red-black tree, queue, heap

//type table info:
//  comparison: int(*)(void*, void*)
//  size: int

typedef int(*Compare)(void*, void*);
typedef void(*Dtor)(void*);

#define MAX_TYPE_NAME_LENGTH 20
typedef struct
{
    Compare comp;
    size_t size;
    char name[MAX_TYPE_NAME_LENGTH];
} TypeEntry;

void initDataStructTypes();

#define registerType(compare, typeName) \
    addTypeEntry(compare, sizeof(typeName), #typeName);

int addTypeEntry(int(*compare)(void* lhs, void* rhs),
                 size_t size,
                 const char* type);

typedef struct
{
    void* data;
    size_t size;
    size_t capacity;
    int type;           //index of entry in type table
} Array;

typedef struct
{
    unsigned* data;
    size_t size;    //size in bits
    size_t nwords;  //number of unsigneds alloc'd
} Bitset;

typedef struct
{
    Array data;     //holds the elements (has type user requests) & handles allocs
    Array ptrs;     //holds pointers to nodes (encodes tree structures)
    Bitset color;   //0 = red, 1 = black; corresponds sequentially to elements in data
} Tree; 

typedef struct
{
    Array buf;
    size_t head;    //index of start
    size_t tail;    //index of end (can be <= start)
} Queue;

typedef struct
{
    Array data;
} Heap;

Array arrNew(int type);
void arrDelete(Array* arr);
#define arrGet(arr, pos, type) (*((type*) arr->data[arr->size * pos]));
#define arrSet(arr, pos, val) {*((type*) arr->data[arr->size * pos]) = *((type*) val);}
void arrPush(Array* arr, void* elem);  //push elem on end
int arrPop(Array* arr, void* elem);   //pop last into elem
int arrInsert(Array* arr, void* elem, size_t pos); //insert at pos
void arrClear(Array* arr);
void arrReserve(Array* arr, size_t num);
void arrShrink(Array* arr);
void arrSort(Array* arr);

Bitset bsNew(size_t size);  //zero-initializes
void bsDelete(Bitset* bs);
bool bsGet(Bitset* bs, size_t index);
void bsSet(Bitset* bs, size_t index, bool val);
void bsResize(Bitset* bs, size_t newCapacity);
int bsPopCount(Bitset* bs);

Tree treeNew(int keyType, int valType);
void treeDelete(Tree* tree);
void treeInsert(Tree* tree, void* key, void* data);
#define treeGet(tree, key, type) (*((type*) treeLookup(tree, key)))
