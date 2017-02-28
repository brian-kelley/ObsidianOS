#include "stdlib.h"
#include "string.h"
#include "stdio.h"

typedef int (*Comparator)(void* lhs, void* rhs);

int compareChar(void* lhs, void* rhs);
int compareUChar(void* lhs, void* rhs);
int compareShort(void* lhs, void* rhs);
int compareUShort(void* lhs, void* rhs);
int compareInt(void* lhs, void* rhs);
int compareUInt(void* lhs, void* rhs);
int compareString(void* lhs, void* rhs); //calls strcmp

#define PSIZE sizeof(void*)

//Contiguous array/stack
typedef struct
{
  void* buf;      //single contiguous buffer of elements
  size_t esize;   //size of each element
  size_t n;          //number of elements
  size_t capacity;   //allocated capacity
} Array;

Array arrayInit_(size_t esize);

#define ArrayCreate(type, name) \
  Array name = arrayInit_(sizeof(type));

#define ArrayInit(type, name) \
  name = arrayInit_(sizeof(type));

#define ArrayDestroy(name) \
  free(name.buf);

void arrayPush_(Array* arr);

// data needs to implicitly convert to type
#define ArrayPush(type, name, data) \
  arrayPush_(&name); \
  ((type*) name.buf)[name.n - 1] = (data);

// ArrayPop produces expression for top data
#define ArrayPop(type, name) \
  ((type*) name.buf)[--name.n]

#define ArrayGet(type, name, index) \
  ((type*) name.buf)[index]

void arrayReserve_(Array* arr, size_t newCap);

#define ArrayReserve(name, newCapacity) \
  arrayReserve_(&name, newCapacity);

typedef struct
{
  Array elems;
  size_t ksize;
  size_t vsize;
  Comparator keycmp;
} BST;

BST bstInit_(size_t ksize, size_t vsize, Comparator kcmp);

#define BSTCreate(ktype, vtype, name, compare) \
  BST name = bstInit_(sizeof(ktype), sizeof(vtype), compare);

#define BSTInit(ktype, vtype, name) \
  name = bstInit_(sizeof(ktype), sizeof(vtype));

void* bstLookup_(BST* bst, void* key);

#define BSTGet(ktype, vtype, name, key) \
({ktype k_ = (key); *((vtype*) (bstLookup_(&name, &k_) + 2 * PSIZE + name.ksize));})

void bstInsert_(BST* bst, void* key, void* val);

#define BSTInsert(ktype, vtype, name, key, val) \
{ \
  ktype k_ = key; \
  vtype v_ = val; \
  bstInsert_(&name, &k_, &v_); \
}

//Clear will clear all elements, but doesn't shrink elems buffer
#define BSTClear(name) \
  (name).elems.n = 0;

