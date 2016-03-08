#include "stdlib.h"
#include "string.h"
#include "globalDefines.h"

#define DEFINE_LIST(TYPE) \
struct _##TYPE##ListNode \
{ \
    TYPE data; \
    struct _##TYPE##ListNode* prev; \
    struct _##TYPE##ListNode* next; \
}; \
\
typedef struct _##TYPE##ListNode* TYPE##ListIter; \
\
typedef struct \
{ \
    struct _##TYPE##ListNode head; \
    struct _##TYPE##ListNode tail; \
} TYPE##List; \
\
void TYPE##ListInit(TYPE##List* l); \
void TYPE##ListDestroy(TYPE##List* l); \
TYPE##ListIter TYPE##ListBegin(TYPE##List* l); \
TYPE##ListIter TYPE##ListEnd(TYPE##List* l); \
TYPE##ListIter TYPE##ListInc(TYPE##ListIter it); \
TYPE##ListIter TYPE##ListDec(TYPE##ListIter it); \
void TYPE##ListPushFront(TYPE##List* l, TYPE data); \
void TYPE##ListPushBack(TYPE##List* l, TYPE data); \
TYPE TYPE##ListPopFront(TYPE##List* l); \
TYPE TYPE##ListPopBack(TYPE##List* l); \
TYPE##ListIter TYPE##ListInsert(TYPE##ListIter it, TYPE data); \
void TYPE##ListRemove(TYPE##ListIter it); \
int TYPE##ListEmpty(TYPE##List* l);

