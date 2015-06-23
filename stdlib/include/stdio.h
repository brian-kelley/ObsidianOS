#ifndef __STDIO_H__
#define __STDIO_H__

#include "stdarg.h"
#include "asmTypes.h"

typedef size_t fpos_t;

typedef struct
{
    int kernelID;      //every stream will have an ID in the kernel
    byte isBuffered;   //1 if buffer points 
    size_t pos;        //position in file/stream, if applicable
    size_t bufSize;    //size of buffer in bytes
    void* buffer;      //buffer, either NULL or a block of memory
} FILE;

#define BUFSIZ 1024
#define EOF -1
#define FILENAME_MAX 8
#define FOPEN_MAX 64
#define L_tmpnam 3
#define TMP_MAX 26 * 26 * 26; //3 letters, starting AAA, then AAB, to ZZZ

int remove(const char* filename);
int rename(const char* oldname, const char* newname);
FILE* tmpfile();			//open temporary file with indeterminate name
char* tmpnam(char* str);	//open temporary file with given name
int fclose(FILE* stream);
int fflush(FILE* stream);
FILE* fopen(const char* filename, const char* mode); //mode is some combo of "r,w,a,+"
FILE* freopen(const char* filename, const char* mode, FILE* stream);
void setbuf(FILE* stream, char* buffer);
int setvbuf(FILE* stream, char* buffer, int mode, size_t size);
int fprintf(FILE* stream, const char* format, ...);
int fscanf(FILE* stream, const char* format, ...);
int printf(const char* format, ...);
int scanf(const char* format, ...);
int sprintf(char* str, const char* format, ...);
int sscanf(const char* s, const char* format, ...);
int vfprintf(FILE* stream, const char* format, va_list arg);
int vprintf(const char* format, va_list arg);
int vsprintf(char* s, const char* format, va_list arg);
int fgetc(FILE* stream);
char* fgets(char* str, int num, FILE* stream);
int fputc(int character, FILE* stream);
int fputs(const char* str, FILE* stream);
int getc(FILE* stream);
int getchar();
char* gets(char* str);
int putc(int character, FILE* stream);
int putchar(int character);
int puts(const char* str);
int ungetc(int character, FILE* stream);
size_t fread(void* ptr, size_t size, size_t count, FILE* stream);
size_t fwrite(const void* ptr, size_t size, size_t count, FILE* stream);
int fgetpos(FILE* stream, fpos_t* pos);
int fseek(FILE* stream, long int offset, int origin);
int fsetpos(FILE* stream, const fpos_t* pos);
long int ftell(FILE* stream);
void rewind(FILE* stream);
void clearerr(FILE* stream);
int feof(FILE* stream);
int ferror(FILE* stream);

#endif
