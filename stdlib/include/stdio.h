#ifndef __STDIO_H__
#define __STDIO_H__

#include <stddef.h>
#include "stdarg.h"
#include "string.h"
#include "terminal.h"
#include "ctype.h"
#include "math.h"
#include "stdlib.h"
#include "globalDefines.h"
#include "fatdrv.h"

typedef size_t fpos_t;

enum F_ERROR
{
	NO_ERROR = 0,
	HARD_FAULT = 1,		//serious hardware or ATA driver problem!
	ILLEGAL_WRITE = 2,	//attempted writing to unwritable file
	FILE_NOT_OPEN = 3	//attempted to use a FILE handle not registerd with kernel  
};

typedef struct
{
    size_t pos;        //position in file/stream, if applicable
    byte* buffer;
    size_t bufsize;    //size of buffer, defaults to BUFSIZ but can be set with setvbuf
    int err;           //error state (an F_ERROR value)
    bool canWrite;     //true if mode is write or append
    bool ungetFilled;  //true if unget is holding a value
    byte unget;        //hold value from ungetc()
    bool active;       //false if this entry is not open
    bool eof;          //true if read or write past end attempted
} FILE;

#define BUF_SIZE 512
#define EOF (-1)
#define FILENAME_MAX 8
#define FOPEN_MAX 64
#define L_tmpnam 1
#define TMP_MAX 26 //a...z

#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

extern FILE* stdout;

int remove(const char* filename);
int rename(const char* oldname, const char* newname);
FILE* tmpfile();			//open temporary file with indeterminate name
char* tmpnam(char* str);	//open temporary file with given name
int fclose(FILE* stream);
int fflush(FILE* stream);
FILE* fopen(const char* filename, const char* mode); //mode is some combo of "r,w,a,+"
FILE* freopen(const char* filename, const char* mode, FILE* stream);
void setbuf(FILE* stream, char* buffer);    //does NOT call free on the old buffer!
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
