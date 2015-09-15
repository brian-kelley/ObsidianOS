#include "stdio.h"

//utility function to convert between ints and character digits in decimal, hex and octal

static void byteToStream(FILE* stream, byte b)
{
    if(stream->kernelID == 0)
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

static char hexToChar(int digit)
{
    if(digit >= 0 && digit < 10)
	return '0' + digit;
    else if(digit >= 10 && digit < 16)
	return 'a' + (digit - 10);
    else
	fputs("Error: invalid hex digit!");
        return '!';
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

static int printSignedDec(long long int num, char* dest)
{
    int length = 0;
    char digits[20];
    char* iter = dest;
    if(num < 0)
    {
	length++;
	*iter = '-';
	num = -num;
	iter++;
    }
    if(num == 0)
    {
	*dest = '0';
	return 1;
    }
    int digIndex = 19;
    while(num > 0)
    {
	digits[digIndex] = decToChar(num % 10);
	num = num / 10;
	length++;
	digIndex--;
    }
    digIndex++;
    for(int i = 0; i < 
}

static int printUnsignedDec(unsigned int num, char* dest)
{

}

static int printOctal(unsigned int num, char* dest)
{
    
}

static int printHex(unsigned int num, char* dest, bool upper)
{

}

static int printFloat(double num, char* dest)
{

}

static int printSciFloat(double num, char* dest, bool upper)
{

}

static int printShortestFloat(double num, char* dest, bool upper)
{

}

static int printString(const char* str, char* dest)
{

}

static int printPointer(void* ptr, char* dest)
{
    unsigned int ptrVal = (int) ptr;
    
}

int printf(const char* format, ...)
{
    char* iter = (char*) format;
    int charsPrinted = 0; //caller can request # of characters printed so far with %n
    for(; *iter != 0; iter++)
    {
	//if character is not %, just print the character
	if(*iter != % && '!' <= *iter && *iter <= '~')
	{
	    putc(*iter);
	    charsPrinted++;
	}
	else
	{
	    switch(*(iter + 1))
	    {
		case '%':
		    putc('%');
		    charsPrinted++;
		    break;
		case 'd':
		    //signed decimal int
		    
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
