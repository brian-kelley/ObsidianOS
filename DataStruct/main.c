#include "DataStruct.h"

int main()
{
  BSTCreate(const char*, int, tree, compareString)
  BSTInsert(const char*, int, tree, "hello", 5)
  BSTInsert(const char*, int, tree, "world", 3)
  BSTInsert(const char*, int, tree, "sailor", 7)
  //test lookup
  puts("");
  int v = BSTGet(const char*, int, tree, "world");
  printf("Looked up world, got %i\n", v);
  v = BSTGet(const char*, int, tree, "sailor");
  printf("Looked up sailor, got %i\n", v);
  v = BSTGet(const char*, int, tree, "hello");
  printf("Looked up hello, got %i\n", v);
  return 0;
}
