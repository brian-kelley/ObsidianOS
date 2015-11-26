#include "stdio.h"

FILE* stdout = NULL;

//utility function to convert between ints and character digits in decimal, hex and octal

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

static int printHex(unsigned int num, FILE* f, bool upper)
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

static int printSciFloat(double num, FILE* f, bool upper)
{
    return 0;
}

static int printShortestFloat(double num, FILE* f, bool upper)
{
    return 0;
}

static int printCstring(const char* str, FILE* f)
{
    return 0;
}

static int printPointer(void* ptr, FILE* f)
{
    //unsigned int ptrVal = (int) ptr;
    return 0;
}

int printf(const char* format, ...)
{
    char* iter = (char*) format;
    int charsPrinted = 0; //caller can request # of characters printed so far with %n
    for(; *iter != 0; iter++)
    {
	//if character is not %, just print the character
	if(*iter != '%' && '!' <= *iter && *iter <= '~')
	{
	    putc(*iter, stdout);
	    charsPrinted++;
	}
	else
	{
	    switch(*(iter + 1))
	    {
		case '%':
		    putc('%', stdout);
		    charsPrinted++;
		    break;
		case 'd':
		    //signed decimal int
		    break;
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
    return 0;
}
