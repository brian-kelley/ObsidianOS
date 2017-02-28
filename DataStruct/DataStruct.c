#include "DataStruct.h"

#define CMP_FUNC(name, type) \
int compare##name(void* lhs, void* rhs) \
{ \
  type l = *((type*) lhs); \
  type r = *((type*) rhs); \
  if(l == r) \
    return 0; \
  if(l < r) \
    return -1; \
  else \
    return 1; \
}

CMP_FUNC(Char, char);
CMP_FUNC(UChar, unsigned char);
CMP_FUNC(Short, short);
CMP_FUNC(UShort, unsigned short);
CMP_FUNC(Int, int);
CMP_FUNC(UInt, unsigned);

int compareString(void* lhs, void* rhs)
{
  return strcmp((const char*) lhs, (const char*) rhs);
}

Array arrayInit_(size_t esize)
{
  Array arr;
  arr.capacity = 8;
  arr.esize = esize;
  arr.n = 0;
  arr.buf = malloc(8 * esize);
  return arr;
}

void arrayPush_(Array* arr)
{
  if(arr->n == arr->capacity)
  {
    arrayReserve_(arr, arr->capacity + (arr->capacity >> 1));
    printf("Expanded array to %lu elems.\n", arr->capacity);
  }
  arr->n++;
}

void arrayReserve_(Array* arr, size_t newCap)
{
  arr->capacity = newCap;
  arr->buf = realloc(arr->buf, arr->capacity * arr->esize);
}

//BST node format: {left ptr, right ptr, key,       value}
//        offsets: {0,        PSIZE,     2 * PSIZE, 2 * PSIZE + sizeof(vtype)}

//note: BST node layout: {void* left, void* right, ktype key, vtype data}
BST bstInit_(size_t ksize, size_t vsize, Comparator kcmp)
{
  BST bst;
  bst.elems = arrayInit_(2 * PSIZE + ksize + vsize);
  bst.ksize = ksize;
  bst.vsize = vsize;
  bst.keycmp = kcmp;
  return bst;
}

void* bstLookup_(BST* bst, void* key)
{
  //get offset of value within node
  void* iter = bst->elems.buf;
  while(1)
  {
    void* iterKey = iter + 2 * PSIZE;
    int cmp = bst->keycmp(iterKey, key);
    if(cmp == 0)
    {
      //return pointer to node
      return iter;
    }
    else if(cmp > 0)
    {
      iter = *((void**) (iter + PSIZE));
    }
    else
    {
      //newIter => left subtree
      iter = *((void**) iter);
    }
    if(!iter)
      break;
  }
  return NULL;
}

void bstInsert_(BST* bst, void* key, void* val)
{
  void* newNode;
  int alreadyInTree = 0;
  if(bst->elems.n == 0)
  {
    bst->elems.n = 1;
    newNode = bst->elems.buf;
  }
  else
  {
    //root is always first node
    void* iter = bst->elems.buf;
    void* newIter;
    while(1)
    {
      void* iterKey = iter + 2 * PSIZE;
      int cmp = bst->keycmp(iterKey, key);
      if(cmp == 0)
      {
        newNode = iter;
        alreadyInTree = 1;
        break;
      }
      else if(cmp > 0)
      {
        newIter = *((void**) (iter + PSIZE));
        if(newIter == NULL)
        {
          //insert here
          arrayPush_(&bst->elems);
          newNode = bst->elems.buf + (bst->elems.n - 1) * bst->elems.esize;
          //link right subtree of iter to newNode
          *((void**) (iter + PSIZE)) = newNode;
          break;
        }
      }
      else
      {
        newIter = *((void**) iter);
        if(newIter == NULL)
        {
          //insert here
          arrayPush_(&bst->elems);
          newNode = bst->elems.buf + (bst->elems.n - 1) * bst->elems.esize;
          *((void**) iter) = newNode;
          break;
        }
      }
      iter = newIter;
    }
  }
  //set up new node
  if(!alreadyInTree)
  {
    *((void**) newNode) = NULL;
    *((void**) (newNode + PSIZE)) = NULL;
    memcpy(newNode + 2 * PSIZE, key, bst->ksize);
  }
  memcpy(newNode + 2 * PSIZE + bst->ksize, val, bst->vsize);
}

