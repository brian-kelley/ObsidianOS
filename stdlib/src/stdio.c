#include "stdio.h"

FILE* stdout = NULL;

//utility function to convert between ints and character digits in decimal, hex and octal

enum SizeSpec
{
	NONE,
	HH,
	H,
	L,
	LL,
	J,
	Z,
	T,
	LD
};

typedef struct
{
	bool leftJust;		//left-justify within field if width given (default is right-just)
	bool posPlus;		//show '+' sign for postiive numbers
	bool posSpace;		//show ' ' in place of sign for positive numbers
	bool forceDecPt;	//always show decimal point in float
	bool zeroPad;		//pad with '0' instead of ' '
	bool pad;
	int padSize;		//min # of chars to be printed
	bool 
} PrintFlags;

static PrintFlags getDefaultFlags()
{
	PrintFlags flags;
	flags.leftJust = false;
	flags.posPlus = false;
	flags.posSpace = false;
	
	return flags;
}

static void byteToStream(byte b, FILE* stream)
{
	if(stream == NULL)	//stdout
	{
		//stdout, write directly to terminal with no buffer
		printChar(b);
	}
	else
	{
		//write the byte to the current position in buffer, and flush if
		//buffer is then full
		size_t bufPos = stream->pos % 512;
		*((byte*) stream->buffer + bufPos) = b;
		bufPos++;
		if(bufPos == 512)
		{
			//flush sector
		}
	}
}

static char hexToChar(int digit, bool upper)
{
	if(digit >= 0 && digit < 10)
		return '0' + digit;
	else if(digit >= 10 && digit < 16)
	{
		if(upper)
			return 'A' + (digit - 10);
		else
			return 'a' + (digit - 10);
	}
	else
	{
		printString("Error: invalid hex digit (stdio.c)\n");
		return '!';
	}
}

static int charToHex(char digit)
{
	if(digit >= '0' && digit <= '9')
		return digit - '0';
	digit = tolower(digit);
	if(digit >= 'a' && digit <= 'f')
		return 10 + (digit - 'a');
	puts("Error: invalid hex digit!");
	return -1;
}

static char decToChar(int digit)
{
	if(digit >= 0 && digit < 10)
		return '0' + digit;
	puts("Error: invalid decimal digit!");
	return '!';
}

static int charToDec(char digit)
{
	if(digit >= '0' && digit <= '9')
		return digit - '0';
	puts("Error: invalid decimal digit!");
	return -1;
}

static char octToChar(int digit)
{
	if(digit >= 0 && digit < 8)
		return '0' + digit;
	puts("Error: invalid octal digit!");
	return '!';
}

static int charToOct(char digit)
{
	if(digit >= '0' && digit <= '8')
		return digit - '0';
	puts("Error: invalid octal digit!");
	return '!';
}

//local utility functions for printing different types
//each returns the number of characters actually printed to stdout

static int printUnsignedDec(unsigned long long int num, FILE* f)
{
	int length = 0;
	char digits[20];
	char* iter = digits;
	if(num == 0)
	{
		byteToStream('0', f);
		return 1;
	}
	while(num > 0)
	{
		*iter = decToChar(num % 10);
		num = num / 10;
		iter++;
	}
	//Write the string to file, reversed
	while(iter != digits)
	{
		iter--;
		byteToStream((byte) *iter, f);
		length++;
	}
	return length;
}

static int printSignedDec(long long int num, FILE* f)
{
	int length = 0;
	if(num < 0)
	{
		byteToStream('-', f);
		length++;
		num = -num;
	}
	return length + printUnsignedDec(num, f);
}

static int printOctal(unsigned long long int num, FILE* f)
{
	int length = 1;
	byteToStream('0', f);   //Octal numbers always start with '0'
	char digits[25];
	char* iter = digits;
	if(num == 0)
	{
		byteToStream('0', f);
		return 2;
	}
	while(num > 0)
	{
		*iter = octToChar(num % 8); //least-significant digit, written last
		num /= 8;
		iter++;
	}
	while(iter != digits)
	{
		iter--;
		byteToStream(*iter, f);
		length++;
	}
	return length;
}

static int printHex(unsigned long long int num, FILE* f, bool upper)
{
	int length = 2;
	byteToStream('0', f);   //Hex numbers always start with '0x'
	if(upper)
		byteToStream('X', f);
	else
		byteToStream('x', f);
	if(num == 0)
	{
		byteToStream('0', f);
		return 3;
	}
	char digits[16];
	char* iter = digits;
	while(num > 0)
	{
		*iter = hexToChar(num % 16, upper);
		num /= 16;
		iter++;
	}
	while(iter != digits)
	{
		iter--;
		byteToStream(*iter, f);
		length++;
	}
	return length;
}

static int getLargestDigit(long double num, long double* newNum)
{
	if(num == 0)
	{
		*newNum = 0;
		return 0;
	}
	long double placeVal = powl(10, (int) log10l(num));
	int digit = num / placeVal;
	*newNum = num - placeVal * digit;
	return digit;
}

static int printCstring(const char* str, FILE* f)
{
	int length;
	for(length = 0; str[length] != 0, length++)
	{
		byteToStream(str[length], f);
	}
	return length;
}

static int printFloat(long double num, FILE* f)
{
	int length = 0;
	char str[40];
	bool sign;
	if(num < 0)
	{
		num = -num;
		sign = 1;
	}
	else
		sign = 0;
	return 0;
}

static int printSciFloat(long double num, FILE* f, bool upper)
{
	if(num == 0)
	{
		return printCstring("0.000000e+00");
	}
	int expo = log10l(num);
	num *= pow(10, expo);
	//num is now in range [0, 10)
	//just print that float followed by the exponent
	int length = 0;
	length += printFloat(num, f);
	length += printCstring("e", f);
	length += printSignedInt(expo, f);
	return length;
}

static int printShortestFloat(long double num, FILE* f, bool upper)
{
	//Only use sci notation if the integer part is huge
	if(abs((long long int) num) > 1e10)
		return printSciFloat(num, f, upper);
	else
		return printFloat(num, f);
}

static int printPointer(void* ptr, FILE* f)
{
	unsigned int val = (int) ptr;
	//32 bit integer in hex is always 8 digits
	byteToStream('0', f);
	byteToStream('x', f);
	for(int i = 0; i < 8; i++)
	{
		byteToStream(hexToChar(val & 0xF0000000), f);
		//shift the next 4 bits into the same position
		f <<= 4;
	}
	return 10;	//always the same length
}

int remove(const char* fname)
{

}

int rename(const char* oldname, const char* newname)
{

}

FILE* tmpfile()
{

}

char* tmpnam(char* str)
{

}

int fclose(FILE* stream)
{

}

int fflush(FILE* stream)
{

}

FILE* fopen(const char* fname, const char* mode)
{

}

FILE* freopen(const char* fname, const char* mode, FILE* stream)
{

}

void setbuf(FILE* stream, char* buffer)
{

}

int setvbuf(FILE* stream, char* buffer, int mode, size_t size)
{

}

int fprintf(FILE* stream, const char* format, ...)
{
	va_list arg = va_start(format);
	int rv = vfprintf(stream, format, arg);
	va_end(arg);
	return rv;
}

int fscanf(FILE* stream, const char* format, ...)
{

}

int printf(const char* format, ...)
{
	va_list arg = va_start(format);
	int rv = vprintf(stdout, format, arg);
	va_end(arg);
	return rv;
}

int scanf(const char* format, ...)
{

}

int sprintf(char* str, const char* format, ...)
{
	va_list arg = va_start(format);
	int rv = sprintf(str, format, arg);
	va_end(arg);
	return rv;
}

int sscanf(const char* str, const char* format, ...)
{
	
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
					case ''
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
								long long int* val = va_arg(arg, long int*);
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
						putc(val);
						running = false;
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
								val = va_arg(arg, ptrdiff_t)
							}
						}
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
	
}

int fgetc(FILE* stream)
{

}

char* fgets(char* str, int num, FILE* stream)
{

}

int fputc(int character, FILE* stream)
{

}

int fputs(const char* str, FILE* stream)
{

}

int getc(FILE* stream)
{

}

int getchar()
{

}

char* gets(char* str)
{

}

int putc(int character, FILE* stream)
{

}

int putchar(int character)
{

}

int puts(const char* str)
{

}

int ungetc(int character, FILE* stream)
{

}

size_t fread(void* ptr, size_t size, size_t count, FILE* stream)
{

}

size_t fwrite(const void* ptr, size_t size, size_t count, FILE* stream)
{

}

int fgetpos(FILE* stream, fpos_t* pos)
{

}

int fseek(FILE* stream, long int offset, int origin)
{

}

int fsetpos(FILE* stream, const fpos_t* pos)
{

}

long int ftell(FILE* stream)
{

}

void rewind(FILE* stream)
{

}

void clearerr(FILE* stream)
{

}

int feof(FILE* stream)
{

}

int ferror(FILE* stream)
{

}

