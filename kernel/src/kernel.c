#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "video.h"
#include "globalDefines.h"
#include "input.h"
#include "terminal.h"
#include "atadrv.h"
#include "memory.h"
#include "string.h"
#include "stdio.h"

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

//Verify correctness of dynamic allocator
void memtest()
{
    int* ptrs[500];
    int count = 0;
    for(int i = 0; i < 500; i++)
    {
	//allocate 1, 2, 3, ... 500 int arrays
	ptrs[i] = malloc((i + 1) * sizeof(int));
	for(int j = 0; j < i + 1; j++)
	{
	    ptrs[i][j] = count++;
	}
    }
    //now verify that the values are all what they were assigned
    count = 0;
    for(int i = 0; i < 500; i++)
    {
	for(int j = 0; j < i + 1; j++)
	{
	    if(ptrs[i][j] != count)
	    {
		printString("FATAL MEMORY ERROR!!!!\n");
		return;
	    }
	    count++;
	}
    }
}

void kernel_main()
{
	enterMode12H();
	clearScreen(4);
	initKeyboard();
	initTerminal();
	double val = 3;
	printChar('\n');
	printf("Should be 003.00: %06.2f\n", val);
	printf("Should be 3.: %#.0f\n", val);
	printf("Should be 3: %.0f\n", val);
	printf("Should be 3.00: %.2f\n", val);
	double val2 = E;
	printf("Should be E with 10 digits: %.10f\n", val2);
	printf("Should be 0003.000000: %011f\n", val);
	printf("Should be 3.000000: %f\n", val);
	printf("Should be 3.0000: %.4f\n", val);
	printf("Should be 0003.000: %08.3f\n", val);
	resetTermCursor();
	//initmm();
	while(1);   //Kernel setup done, everything else triggered by interrupts
}
