#define IMPLEMENT_LIST(TYPE) \
void TYPE##ListInit(TYPE##List* l) \
{ \
    l->head.prev = NULL; \
    l->head.next = &(l->tail); \
    l->tail.prev = &(l->head); \
    l->tail.next = NULL; \
} \
\
void TYPE##ListDestroy(TYPE##List* l) \
{ \
    while(l->head.next != &(l->tail)) \
	TYPE##ListPopFront(l); \
} \
\
TYPE##ListIter TYPE##ListBegin(TYPE##List* l) \
{ \
    return l->head.next; \
} \
\
TYPE##ListIter TYPE##ListEnd(TYPE##List* l) \
{ \
    return &(l->tail); \
} \
\
TYPE##ListIter TYPE##ListInc(TYPE##ListIter it) \
{ \
    return it->next; \
} \
\
TYPE##ListIter TYPE##ListDec(TYPE##ListIter it) \
{ \
    return it->prev; \
} \
\
void TYPE##ListPushFront(TYPE##List* l, TYPE data) \
{ \
    TYPE##ListIter newNode = malloc(sizeof(l->head)); \
    newNode->prev = &(l->head); \
    newNode->next = l->head.next; \
    newNode->data = data; \
    l->head.next->prev = newNode; \
    l->head.next = newNode; \
} \
\
void TYPE##ListPushBack(TYPE##List* l, TYPE data) \
{ \
    TYPE##ListIter newNode = malloc(sizeof(l->head)); \
    newNode->next = &(l->tail); \
    newNode->prev = l->tail.prev; \
    newNode->data = data; \
    l->tail.prev->next = newNode; \
    l->tail.prev = newNode; \
} \
\
TYPE TYPE##ListPopFront(TYPE##List* l) \
{ \
    if(l->head.next == &(l->tail)) \
    { \
	TYPE empty; \
	memset(&empty, 0, sizeof(empty)); \
	return empty; \
    } \
    TYPE data = l->head.next->data; \
    TYPE##ListIter toDelete = l->head.next; \
    l->head.next = toDelete->next; \
    toDelete->next->prev = &(l->head); \
    free(toDelete); \
    return data; \
} \
\
TYPE TYPE##ListPopBack(TYPE##List* l) \
{ \
    if(l->head.next == &(l->tail)) \
    { \
	TYPE empty; \
	memset(&empty, 0, sizeof(empty)); \
	return empty; \
    } \
    TYPE data = l->head.next->data; \
    TYPE##ListIter toDelete = l->tail.prev; \
    l->tail.prev = toDelete->prev; \
    toDelete->prev->next = &(l->tail); \
    free(toDelete); \
    return data; \
} \
\
TYPE##ListIter TYPE##ListInsert(TYPE##ListIter it, TYPE data) \
{ \
    TYPE##ListIter newNode = malloc(sizeof(*it)); \
    newNode->prev = it->prev; \
    newNode->next = it; \
    it->prev = newNode; \
    newNode->prev->next = newNode; \
    newNode->data = data; \
    return newNode; \
} \
\
void TYPE##ListRemove(TYPE##ListIter it) \
{ \
    it->prev->next = it->next; \
    it->next->prev = it->prev; \
    free(it); \
} \
\
int TYPE##ListEmpty(TYPE##List* l) \
{ \
    if(l->head.next == &(l->tail)) \
	return 1; \
    return 0; \
}
