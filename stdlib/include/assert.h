#ifndef ASSERT_H
#define ASSERT_H

extern int printf(const char* fmt, ...);

#define assert(cond) \
{ \
  if(!(cond)) \
  { \
    printf("Assert fail: %s, line %i in %s\n", #cond, __LINE__, __FILE__); \
    while(1); \
  } \
}

#endif

