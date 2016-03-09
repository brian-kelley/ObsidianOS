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
    if(v->size + 1 == v->capacity) \
    { \
    	v->capacity *= 1.5; \
	v->buf = realloc(v->buf, sizeof(TYPE) * v->capacity); \
    } \
    memmove(&v->buf[i + 1], &v->buf[i], sizeof(TYPE) * (v->size - i)); \
    v->buf[i] = val; \
    v->size++; \
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
    TYPE val = v->buf[i]; \
    memmove(&v->buf[i], &v->buf[i + 1], sizeof(TYPE) * (v->size - i)); \
    v->size--; \
    return val; \
}
