#include "stdio.h"

FILE* stdout = NULL;

//utility function to convert between ints and character digits in decimal, hex and octal

typedef enum
{
	NONE,	//int
	HH,	//byte and char
	H,	//short
	L,	//long int
	LL,	//long long int	
	J,	//intmax_t
        Z,	//size_t
	T,	//ptrdiff_t
	LD	//long double
} SizeSpec;

static void byteToStream(byte b, FILE* f)
{

}

int setvbuf(FILE* stream, char* buffer, int mode, size_t size)
{
    return 0;
}

int fprintf(FILE* stream, const char* format, ...)
{
	va_list arg;
	va_start(arg, format);
	int rv = vfprintf(stream, format, arg);
	va_end(arg);
	return rv;
}

int fscanf(FILE* stream, const char* format, ...)
{
    return 0;
}

int printf(const char* format, ...)
{
	va_list arg;
	va_start(arg, format);
	int rv = vfprintf(stdout, format, arg);
	va_end(arg);
	return rv;
}

int scanf(const char* format, ...)
{
    return 0;
}

int sprintf(char* str, const char* format, ...)
{
	va_list arg;
	va_start(arg, format);
	int rv = sprintf(str, format, arg);
	va_end(arg);
	return rv;
}

int sscanf(const char* str, const char* format, ...)
{
	return 0;
}



int vprintf(const char* format, va_list arg)
{
    return vfprintf(stdout, format, arg);
}

int vsprintf(char* s, const char* format, va_list arg)
{
    return 0;
}

int fgetc(FILE* stream)
{
    return 0;
}

char* fgets(char* str, int num, FILE* stream)
{
    return 0;
}

int fputc(int character, FILE* stream)
{
    return 0;
}

int fputs(const char* str, FILE* stream)
{
    return 0;
}

int getc(FILE* stream)
{
    return 0;
}

int getchar()
{
    return 0;
}

char* gets(char* str)
{
    return 0;
}

int putc(int character, FILE* stream)
{
    return 0;
}

int putchar(int character)
{
    return 0;
}

int puts(const char* str)
{
    return 0;
}

int ungetc(int character, FILE* stream)
{
    return 0;
}

size_t fread(void* ptr, size_t size, size_t count, FILE* stream)
{
    return 0;
}

size_t fwrite(const void* ptr, size_t size, size_t count, FILE* stream)
{
    return 0;
}

int fgetpos(FILE* stream, fpos_t* pos)
{

}

int fseek(FILE* stream, long int offset, int origin)
{

}

int fsetpos(FILE* stream, const fpos_t* pos)
{
    return 0;
}

long int ftell(FILE* stream)
{
    return 0;
}

void rewind(FILE* stream)
{
    return 0;
}

void clearerr(FILE* stream)
{
    return 0;
}

int feof(FILE* stream)
{
    return 0;
}

int ferror(FILE* stream)
{
    return 0;
}

