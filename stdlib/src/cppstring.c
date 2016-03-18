#include "cppstring.h"

#define MINCAP 16               //min capacity, depends on malloc 

static void resizeStr(string* s, size_t newCap)
{
    if(newCap <= MINCAP)
        return;
    s->c_str = realloc(s->c_str, newCap);
    s->cap = newCap;
}

string emptyStr()
{
    string s;
    s.size = 0;
    s.cap = MINCAP;
    s.c_str = malloc(s.cap);
    s.c_str[0] = 0;
    return s;
}

string initStr(const char* c)
{
    string s;
    size_t len = strlen(c);
    s.cap = len + 1;
    if(s.cap < MINCAP)
        s.cap = MINCAP;
    s.c_str = malloc(s.cap);
    memcpy(s.c_str, c, s.cap);
    s.size = len;
    return s;
}

void disposeStr(string* s)
{
    free(s->c_str);
}

void concatStr(string* dst, string* src)
{
    size_t added = strlen(src->c_str);
    if(added == 0)
        return;
    size_t newCap = dst->size + added + 1;
    if(newCap > dst->cap)
        resizeStr(dst, newCap);
    memcpy(dst->c_str + dst->size, src->c_str, added + 1);
    dst->size += added;
}

void concatChar(string* dst, char c)
{
    size_t newCap = dst->size + 2;
    if(newCap > dst->cap)
        resizeStr(dst, newCap);
    dst->c_str[dst->size] = c;
    dst->c_str[dst->size + 1] = 0;
    dst->size++;
}
