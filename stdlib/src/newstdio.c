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

int vfprintf(FILE* stream, const char* format, va_list arg)
{
	//Make a mutable copy of the format string
	char buf[250];
	strcpy(buf, format);
	char* iter = (char*) format;
	int charsPrinted = 0; //caller can request # of characters printed so far with %n
	//initialize variadic arg list
	for(; *iter != 0; iter++)
	{
		//if character is not % and is printable, print it
		if(*iter != '%' && '!' <= *iter && *iter <= '~')
		{
			byteToStream(*iter, stream);
			charsPrinted++;
		}
		else
		{
			iter++;
			//behavior depends on specifier in format stringp
			SizeSpec ss = NONE;
			bool reading = true;
			while(reading)
			{
				switch(*iter)
				{
					case '%':
					{
						byteToStream('%', stream);
						charsPrinted++;
						reading = false;
						break;
					}
					//Size specifiers
					case ' ':
		                                break;
					case 'n':
					{
						//provide the current # of charsPrinted
						switch(ss)
						{
							case HH:
							{
								char* val = va_arg(arg, char*);
								*val = charsPrinted;
								break;
							}
							case H:
							{
								short* val = va_arg(arg, short*);
								*val = charsPrinted;
								break;
							}
							case NONE:
							{
								int* val = va_arg(arg, int*);
								*val = charsPrinted;
								break;
							}
							case L:
							{
								long int* val = va_arg(arg, long int*);
								*val = charsPrinted;
								break;
							}
							case LL:
							{
								long long int* val = va_arg(arg, long long int*);
								*val = charsPrinted;
								break;
							}
							case J:
							{
								intmax_t* val = va_arg(arg, intmax_t*);
								*val = charsPrinted;
								break;
							}
							case Z:
							{
								size_t* val = va_arg(arg, size_t*);
								*val = charsPrinted;
								break;
							}
							case T:
							{
								ptrdiff_t* val = va_arg(arg, ptrdiff_t*);
								*val = charsPrinted;
								break;
							}
							default:
							printString("<ERROR: Invalid %n type.>\n");
						}
						reading = false;
						break;
					}
					case 'c':
					{
						//character
						char val = va_arg(arg, char);
						putchar(val);
						reading = false;
						charsPrinted++;
						break;
					}
					case 'd':
					case 'i':
					{
						//signed decimal int
						long long int val;
						switch(ss)
						{
							case HH:
							{
								val = va_arg(arg, char);
								break;
							}
							case H:
							{
								val = va_arg(arg, short);
								break;
							}
							case NONE:
							{
								val = va_arg(arg, int);
								break;
							}
							case L:
							{
								val = va_arg(arg, long int);
								break;
							}
							case LL:
							{
								val = va_arg(arg, long long int);
								break;
							}
							case J:
							{
								val = va_arg(arg, intmax_t);
								break;
							}
							case Z:
							{
								val = va_arg(arg, size_t);
								break;
							}
							case T:
							{
								val = va_arg(arg, ptrdiff_t);
				                                break;
							}
                                                        default:
				                            break;
						}
				                reading = false;
						charsPrinted += printSignedDec(val, f);
					}
					case 'u':
					{
						//unsigned decimal int
						unsigned long long int val;
						switch(ss)
						{
							case HH:
							{
								val = va_arg(arg, unsigned char);
								break;
							}
							case H:
							{
								val = va_arg(arg, unsigned short);
								break;
							}
							case NONE:
							{
								val = va_arg(arg, unsigned int);
								break;
							}
							case L:
							{
								val = va_arg(arg, unsigned long int);
								break;
							}
							case LL:
							{
								val  va_arg(arg, unsigned long long int);
								break;
							}
							case J:
							{
								val = va_arg(arg, intmax_t);
								break;
							}
							case Z:
							{
								val = va_arg(arg, size_t);
								break;
							}
							case T:
							{
								val = va_arg(arg, ptrdiff_t);
								break;
							}
						}
						charsPrinted += printUnsignedDec(val, f);
					}
					case 'f':
					{
						
					}
					iter++;
				}
			}
		}
		while(*iter != 0)
		{
			//print everything up to '%', or end of string if no % remaining
			iter = strchr(iter, '%');
			if(*(iter + 1) == '%')
			{
				//just print a %
			}
		}
	}
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

