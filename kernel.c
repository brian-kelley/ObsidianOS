#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "video.h"

extern word passthru(dword);

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
	byte color = 0x0F;
	char* mystr = "0x........";	//all dots will be filled in
	for(int i = 0; i < 8; i++)
	{
		mystr[9 - i] = hexFromBits(num >> (i * 4));
	}
	int cursorX = 0;
	int cursorY = row;
	char* iter = (char*) mystr;
	dword* textbuf = (dword*) 0xB8000;
	while(*iter != '\0')
	{
		dword entry = ((dword) color) << 8 | (dword) (*iter);
		textbuf[cursorX + cursorY * 80] = entry;
		cursorX++;
		iter++;
	}
}

void kernel_main()
{
	const int WIDTH = 80;
	const int HEIGHT = 25;
	int cursorX = 0;
	int cursorY = 0;
	uint16_t* textbuf = (uint16_t*) 0xB8000;
	uint8_t textColor = 0x00;
	uint16_t blank = textColor << 8 | (uint16_t) ' ';
	for(uint16_t i = 0; i < WIDTH; i++)
	{
		for(uint16_t j = 0; j < HEIGHT; j++)
		{
			textbuf[i + j * WIDTH] = blank;
		}
	}
	int row = 0;
	for(int i = 0; i < 0x7FFFFFFF; i++)
	{
		row = row > 25 ? 0 : row;
		drawNum(i, row);
		row++;
	}
	//initVideo();
}
