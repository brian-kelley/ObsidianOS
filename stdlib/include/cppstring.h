#ifndef CPP_STRING_H
#define CPP_STRING_H

#include "globalDefines.h"
#include "string.h"
#include "stdlib.h"

typedef struct
{
    char* c_str;
    size_t size;
    size_t cap;
} string;

string emptyStr();
string initStr(const char* s);
void disposeStr(string* s);
void concatStr(string* dst, string* src);
void concatChar(string* dst, char c);

#endif
