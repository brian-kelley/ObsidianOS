#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "video.h"
#include "globalDefines.h"
#include "input.h"
#include "terminal.h"
#include "atadrv.h"

/* Note on GCC for i386 types:

short: 16 bits
int: 32 bits
long int: 32 bits
long long int: 64 bits
float: 32 bits
double: 64 bits
long double: 96 bits in memory for 8-byte alignment but is actually 80 bit precision
*/

extern byte getFontVal();
byte userProc = 0;	//0 if in terminal and 1 if running user program

char hexFromBits(byte bits)	//put in low 4
{
	switch(bits & 0xF)
	{
		case 0:
			return '0';
		case 1:
			return '1';
		case 2:
			return '2';
		case 3:
			return '3';
		case 4:
			return '4';
		case 5:
			return '5';
		case 6:
			return '6';
		case 7:
			return '7';
		case 8:
			return '8';
		case 9:
			return '9';
		case 10:
			return 'A';
		case 11:
			return 'B';
		case 12:
			return 'C';
		case 13:
			return 'D';
		case 14:
			return 'E';
		case 15:
			return 'F';
		default:
			return '?';
	}
}
void drawNum(dword num, int row)
{
	char* mystr = "0x........";	//all dots will be filled in
	for(int i = 0; i < 8; i++)
	{
		mystr[9 - i] = hexFromBits(num >> (i * 4));
	}
	puts((const char*) mystr, 0, row);
}

void keyPressed(byte scancode, byte pressed)
{
    //Keyboard interrupts send data here. Send data to user applications etc.
	if(userProc)
	{
		//give event to user key listeners...
	}
	else
	{
		//terminal is the current focus
		//terminal doesn't care about key releases
		if(pressed)
			terminalKeyListener(scancode);
	}
}

void kernel_main()
{
	enterMode12H();
	clearScreen(4);
	initKeyboard();
	initTerminal();
	while(1);
}
