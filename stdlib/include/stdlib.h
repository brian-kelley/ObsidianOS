#ifndef __STDLIB_H__
#define __STDLIB_H__

#ifndef NULL
#define NULL ((void*) 0);
#endif
#ifndef RAND_MAX
#define RAND_MAX 0xAFFFFFFF
#endif
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS (0)
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE (-1)
#endif

typedef struct
{
    int quot;
    int rem;
} div_t;

typedef struct
{
    long int quot;
    long int rem;
} ldiv_t;

double atof(const char* str);
int atoi(const char* str);
long int atol(const char* str);
double strtod(const char* str, char** endptr);
long int strtol(const char* str, char** endptr);
unsigned long int strtoul(const char* str, char** endptr);

int rand();
void srand(unsigned int seed);

void* malloc(size_t size);
void* calloc(size_t num, size_t size);
void* realloc(void* ptr, size_t newSize);
void free(void* ptr);

void abort();
int atexit(void (*func) void);
void exit(int status);
char* getenv(const char* name);
int system(const char* command);

void* bsearch(void* key, void* base, size_t num, size_t size, int (*compare) (const void*, const void*));
void* qsort(void* base, size_t num, size_t size, int (*compare) (const void*, const void*));

int abs(int n);
div_t div(int numer, int denom);
long int labs(long int n);
ldiv_t ldiv(long int numer, long int denom);

#endif
