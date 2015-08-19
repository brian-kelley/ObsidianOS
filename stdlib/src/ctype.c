#include "ctype.h"

#define inRange(min, max) (min <= c && c <= max)

int isalnum(int c)
{
    if(inRange('0', '9') || inRange('A', 'Z') || inRange('a', 'z'))
	return true;
    return false;
}

int isalpha(int c)
{
    if(inRange('a', 'z') || inRange('A', 'Z'))
	return true;
    return false;
}

int iscntrl(int c)
{
    if(inRange(0, 0x1F) || c == 0x7F)
	return true;
    return false;
}

int isdigit(int c)
{
    if(inRange('0', '9'))
	return true;
    return false;
}

int isgraph(int c)
{
    if(inRange(0x21, 0x7E))
	return true;
    return false;
}

int islower(int c)
{
    if(inRange('a', 'z'))
	return true;
    return false;
}

int isprint(int c)
{
    if(inRange(0x20, 0x7E))
	return true;
    return false;
}

int ispunct(int c)
{
    if(inRange(0x21, 0x2F) || inRange(0x3A, 0x40) || inRange(0x5B, 0x60) || inRange(0x7B, 0x7E))
	return true;
    return false;
}

int isspace(int c)
{
    if(c == ' ' || inRange(0x9, 0xD))
	return true;
    return false;
}

int isupper(int c)
{
    if(inRange('A', 'Z'))
	return true;
    return false;
}

int isxdigit(int c)
{
    if(inRange('0', '9') || inRange('a', 'f') || inRange('A', 'F'))
	return true;
    return false;
}

int tolower(int c)
{
    if(inRange('A', 'Z'))
	return c + 'a' - 'A';
    else
	return c;
}

int toupper(int c)
{
    if(inRange('a', 'z'))
	return c - 'a' - 'A';
    else
	return c;
}
