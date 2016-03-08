#include "globalDefines.h"
#include "stdlib.h"
#include "string.h"

#define DEFINE_VECTOR(TYPE) \
typedef struct \
{ \
    TYPE* buf; \
    size_t size; \
    size_t capacity; \
} TYPE##Vector; \
\
void TYPE##VecInit(TYPE##Vector* v); \
void TYPE##VecDestroy(TYPE##Vector* v); \
void TYPE##VecPushBack(TYPE##Vector* v, TYPE t); \
TYPE TYPE##VecPopBack(TYPE##Vector* v); \
void TYPE##VecReserve(TYPE##Vector* v, size_t newCap); \
void TYPE##VecShrink(TYPE##Vector* v); \
TYPE TYPE##VecAt(TYPE##Vector* v, size_t i); \
void TYPE##VecInsert(TYPE##Vector* v, size_t i, TYPE val); \
TYPE TYPE##VecRemove(TYPE##Vector* v, size_t i);
