#define IMPLEMENT_VECTOR(TYPE) \
\
void TYPE##VecInit(TYPE##Vector* v) \
{ \
    v->size = 0; \
    v->capacity = 10; \
    v->buf = malloc(10 * sizeof(TYPE)); \
} \
\
void TYPE##VecDestroy(TYPE##Vector* v) \
{ \
    free(v->buf); \
} \
\
void TYPE##VecPushBack(TYPE##Vector* v, TYPE t) \
{ \
    if(v->size < v->capacity) \
    { \
	v->buf[v->size] = t; \
	v->size++; \
	return; \
    } \
    v->capacity *= 1.5; \
    v->buf = realloc(v->buf, v->capacity * sizeof(TYPE)); \
    v->buf[v->size] = t; \
    v->size++; \
} \
\
TYPE TYPE##VecPopBack(TYPE##Vector* v) \
{ \
    v->size--; \
    return v->buf[v->size]; \
} \
\
void TYPE##VecReserve(TYPE##Vector* v, size_t newCap) \
{ \
    if(newCap > v->capacity) \
	v->buf = realloc(v->buf, newCap * sizeof(TYPE)); \
} \
\
void TYPE##VecShrink(TYPE##Vector* v) \
{ \
    size_t target = v->size * 1.5; \
    if(v->capacity > target) \
	v->buf = realloc(v->buf, target * sizeof(TYPE)); \
} \
TYPE TYPE##VecAt(TYPE##Vector* v, size_t i) \
{ \
    return v->buf[i]; \
} \
\
void TYPE##VecInsert(TYPE##Vector* v, size_t i, TYPE val) \
{ \
    if(i > v->size) \
	return; \
    TYPE##VecPushBack(v, val); \
    if(i == v->size) \
	return; \
    for(size_t it = v->size - 2; it >= i; it--) \
	v->buf[it + 1] = v->buf[i]; \
    v->buf[i] = val; \
} \
\
TYPE TYPE##VecRemove(TYPE##Vector* v, size_t i) \
{ \
    if(i >= v->size) \
    { \
	TYPE empty; \
	memset(&empty, 0, sizeof(TYPE)); \
	return empty; \
    } \
    v->size--; \
    for(size_t it = i; it < v->size; it++) \
	v->buf[it] = v->buf[it + 1]; \
}
