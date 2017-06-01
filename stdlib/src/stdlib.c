#include "stdlib.h"

static unsigned randState;

//update this as necessary when system image size grows
static size_t membreak = 0x10000;
static size_t minbreak = 0x10000;
static size_t maxbreak = 0x9DD00;

void printMemStats()
{
  printf("%u KB used, %u KB free.\n", (membreak - minbreak) >> 10, (maxbreak - membreak) >> 10);
}

void* malloc(size_t size)
{
  if(membreak + size > maxbreak)
  {
    puts("Out of memory - allocation failed!");
    while(1);
  }
  void* ptr = (void*) membreak;
  membreak += size;
  return ptr;
  //return mmAlloc(size);
}

void* calloc(size_t num, size_t size)
{
  byte* ptr = (byte*) malloc(num * size);
  if(ptr != NULL)
    memset(ptr, 0, size);
  return ptr;
}

//for now just to the trivial thing - free and reallocate
void* realloc(void* ptr, size_t newSize)
{
  void* newPtr = malloc(newSize);
  memcpy(newPtr, ptr, newSize);
  return newPtr;
  //TODO: return mmRealloc(ptr, newSize);
}

void free(void* ptr)
{
  //mmFree(ptr);
}

double atof(const char* str)
{
  double rv = 1;
  //Find the first non-whitespace character
  const char* start = str + strspn(str, " \n\t");
  if(*start == '-')
    rv *= -1;
  start++;
  long long int mantissa;
  long long int pot = 1;
  //find the last decimal digit before the exponent or end of number
  const char* manEnd = start + strspn(start, "0123456789");
  manEnd--;
  const char* digIt = manEnd; //iterator over the digit chars, backwards
  while(digIt <= start)
  {
    int digit = *digIt - '0';
    mantissa += digit * pot;
    pot *= 10;
    digIt--;
  }
  manEnd++;
  if(tolower(*manEnd) == 'e')
  {

  }
  //TODO
  return rv;
}

int rand()
{
  //32-bit xorshift
  randState ^= randState << 13;
  randState ^= randState >> 17;
  randState ^= randState << 5;
  //mask out sign bit: rand() must return positive value
  int intval = randState;
  return intval < 0 ? -intval : intval;
}

void srand(unsigned int seed)
{
  randState = seed;
  //don't return seed immediately
  rand();
}

