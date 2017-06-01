#include "string.h"

static char* lastTok;	//used to store the \0 where last token ended

//memcpy implementation that does work with overlapping src/dst arrays
void* memcpy(void* dst, const void* src, size_t num)
{
  byte* srcArr = (byte*) src;
  byte* dstArr = (byte*) dst;
  if(src > dst)
  {
    for(size_t i = 0; i < num; i++)
      dstArr[i] = srcArr[i];
  }
  else if(src < dst)
  {
    for(size_t i = 0; i < num; i++)
      dstArr[num - i - 1] = srcArr[num - i - 1];
  }
  return dst;
}

void* memmove(void* dst, const void* src, size_t num)
{
  return memcpy(dst, src, num);
}

char* strcpy(char* dst, const char* src)
{
  for(int i = 0;; i++)
  {
    if(src[i] == 0)
    {
      dst[i] = 0;
      break;
    }
    dst[i] = src[i];
  }
  return dst;
}

char* strncpy(char* dst, const char* src, size_t num)
{
  size_t total = 0;
  for(; total < num; total++)
  {
    dst[total] = src[total];
    if(!src[total])
      break;
  }
  for(; total < num; total++)
    dst[total] = 0;
  return dst;
}

char* strcat(char* dst, const char* src)
{
  char* iter = dst;
  for(; *iter; iter++);
  const char* siter = src;
  for(; *siter; siter++)
    *(iter++) = *siter;
  *iter = 0;
  return dst;
}

char* strncat(char* dst, const char* src, size_t num)
{
  char* iter = dst;
  for(; *iter; iter++);
  const char* siter = src;
  size_t added = 0;
  for(; *siter; siter++)
  {
    if(added++ >= num)
      break;
    *(iter++) = *siter;
  }
  *iter = 0;
  return dst;
}

int memcmp(const void* ptr1, const void* ptr2, size_t num)
{	
  const byte* arr1 = (const byte*) ptr1;
  const byte* arr2 = (const byte*) ptr2;
  for(int i = 0; i < (int) num; i++)
  {
    if(arr1[i] < arr2[i])
      return -1;
    if(arr1[i] > arr2[i])
      return 1;
  }
  return 0;
}

int strcmp(const char* str1, const char* str2)
{
  for(size_t i = 0;; i++)
  {
    if(str1[i] == 0 && str2[i] == 0) //strings match exactly
      break;
    if(str1[i] < str2[i])
      return -1;
    if(str1[i] > str2[i])
      return 1;
  }
  return 0;
}

int strncmp(const char* str1, const char* str2, size_t num)
{
  for(size_t i = 0; i < num; i++)
  {
    if(str1[i] == 0 || str2[i] == 0)
      return 0;
    if(str1[i] < str2[i])
      return -1;
    if(str1[i] > str2[i])
      return 1;
  }
  return 0;
}

void* memchr(void* ptr, int value, size_t num)
{
  byte* arr = (byte*) ptr;
  byte search = value;
  for(size_t i = 0; i < num; i++)
  {
    if(arr[i] == search)
      return ptr + i;
  }
  return NULL;
}

char* strchr(char* str, int character)
{
  for(int i = 0;; i++)
  {
    if(str[i] == 0)
      return NULL;
    if(str[i] == character)
      return str + i;
  }
}

size_t strcspn(const char* str1, const char* str2)
{
  for(size_t i = 0;; i++)
  {
    if(str1[i] == 0)
      return i;
    for(size_t j = 0; str2[j]; j++)
    {
      if(str1[i] == str2[j])
        return i;
    }
  }
}

char* strpbrk(char* str1, const char* str2)
{
  size_t span = strcspn(str1, str2);
  if(str1[span])
    return str1 + span;
  return NULL;
}

const char* strrchr(const char* str, int character)
{
  //first find end of string
  size_t stringLen = 0;
  for(;; stringLen++)
  {
    if(str[stringLen] == 0)
      break;
  }
  for(int i = stringLen - 1; i >= 0; i--)
  {
    if(str[i] == (char) character)
      return str + i;
  }
  return NULL;
}

size_t strspn(const char* str1, const char* str2)
{
  size_t i = 0;
  for(; str1[i]; i++)
  {
    bool found = false;
    for(size_t j = 0; str2[j]; j++)
    {
      if(str1[i] == str2[j])
      {
        found = true;
        break;
      }
    }
    if(!found)
      break;
  }
  return i;
}

char* strstr(const char* str1, const char* str2)
{
  for(int i = 0;; i++)
  {
    if(str1[i] == 0)
      break;
    //check if str2 is part of str1 starting at str1[i]
    byte matches = 1;
    for(int j = 0;; j++)
    {
      if(str2[j] == 0)
        break;
      if(str1[i + j] != str2[j])
      {
        matches = 0;
        break;
      }
    }
    if(matches)
      return (char*) str1 + i;
  }
  return NULL;
}

char* strtok(char* str, const char* delimiters)
{
  char* start = str ? str : lastTok;
  if(start == NULL)
    return NULL;
  start += strspn(start, delimiters);      //scan to the first non-delim char
  char* end = start + strcspn(start, delimiters);
  if(*end == 0)	    //hit the end of original string
    lastTok = NULL;
  else
    lastTok = end + 1;
  *end = 0;
  return start;
}

void* memset(void* ptr, int value, size_t num)
{
  dword val = (byte) value;
  val |= val << 8;
  val |= val << 16;
  dword* dptr = (dword*) ptr;
  size_t i = 0;
  for(; i < num >> 2; i++)
  {
    dptr[i] = val;
  }
  byte* bptr = (byte*) ptr;
  for(; i < num & 3; i++)
  {
    bptr[i] = (byte) value;
  }
  return ptr;
}

size_t strlen(const char* str)
{
  for(int i = 0;; i++)
  {
    if(str[i] == 0)
      return i;
  }
  return 0;
}

