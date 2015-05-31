#include "terminal.h"

#define TERM_W 80
#define TERM_H 30

static char buffer[TERM_H][TERM_W];		//30 rows of 80 characters
static int cursorX = 0;
static int cursorY = 0;
static char* const prompt = ">> ";
static const char cursor = '_';
static const int promptLen = 3;	//column to put cursor in at prompt
static byte fgColor = 0xF;	//white	text
static byte bgColor = 0x1;	//dark blue background
static const int TAB_WIDTH = 4;
static int commandLen = 0;	//number of chars in command being typed, starting from promptLen++. Div by TERM_W to get # of rows.

void initTerminal()
{
	for(int i = 0; i < TERM_H; i++)
	{
		for(int j = 0; j < TERM_W; j++)
		{
			buffer[i][j] = ' ';
		}
	}
	char* const welcome = "Welcome to GoldOS!";
	for(int i = 0;; i++)
	{
		if(welcome[i] == 0)
			break;
		buffer[0][i] = welcome[i];
	}
	for(int i = 0;; i++)
	{
		if(prompt[i] == 0)
			break;
		buffer[1][i] = prompt[i];
	}
	//Set initial cursor position
	cursorX = promptLen;
	cursorY = 1;
	terminalUpdateScreen();
}

void terminalKeyListener(byte scancode)
{
	Scancode code = (Scancode) scancode;
	switch(code)
	{
		case KEY_TAB:
		{
			//Erase cursor
			drawChar(' ', cursorX, cursorY, fgColor, bgColor);
			if(TERM_W - cursorX < 4)
			{
				//commandLen now divisible by TERM_W so set it to ceil(commandLen / TERM_W) * TERM_W
				commandLen = (commandLen % TERM_W) == 0 ? commandLen : (1 + commandLen / TERM_W) * TERM_W;
				cursorY++;
				cursorX = 0;
				if(cursorY == TERM_H)
				{
					shiftUp();
					//If the tab forces the buffer to shift up, start cursor at start of bottom line
					cursorY--;
				}
			}
			else
			{
				cursorX += TAB_WIDTH;
				commandLen += TAB_WIDTH;
			}
			drawChar(cursor, cursorX, cursorY, fgColor, bgColor);
			break;
		}
		case KEY_ENTER:
		{
			//carriage return and newline
			//Erase cursor
			drawChar(' ', cursorX, cursorY, fgColor, bgColor);
			cursorX = promptLen;
			cursorY++;
			if(cursorY == TERM_H)
			{
				shiftUp();
				cursorY--;
			}
			parseCommand();	//Would normally load a program into RAM, invoke the runtime and jump execution to the program.
			//Does not do that yet.
			//Draw prompt string on cursor's new row
			for(int i = 0;; i++)
			{
				if(prompt[i] == 0)
					break;
				drawChar(prompt[i], i, cursorY, fgColor, bgColor);
			}
			//Redraw cursor
			drawChar(cursor, cursorX, cursorY, fgColor, bgColor);
			commandLen = 0;
			break;
		}
		case KEY_BACKSPACE:
		{
			if(commandLen == 0)
				break;
			//Erase cursor, back up, redraw cursor
			drawChar(' ', cursorX, cursorY, fgColor, bgColor);
			commandLen--;
			if(cursorX == 0)
			{
				cursorX = TERM_W - 1;
				cursorY--;
			}
			else
			{
				cursorX--;
			}
			drawChar(cursor, cursorX, cursorY, fgColor, bgColor);
			break;
		}
		default:
		{
			//Any other scancode is processed by its ASCII value, if any
			char other = getASCII(scancode);
			if(other)
			{
				drawChar(other, cursorX, cursorY, fgColor, bgColor);
				cursorX++;
				commandLen++;
				if(cursorX == TERM_W)
				{
					cursorX = 0;
					//goto next line
					cursorY++;
					if(cursorY == TERM_H)
					{
						shiftUp();
						cursorY--;
					}
				}
				drawChar(cursor, cursorX, cursorY, fgColor, bgColor);
			}
		}
	}
}

void terminalUpdateScreen()
{
	for(int i = 0; i < TERM_H; i++)
	{
		for(int j = 0; j < TERM_W; j++)
		{
			drawChar(buffer[i][j], j, i, fgColor, bgColor);
		}
	}
}

void printString(char* const str)
{
	
}

void shiftUp()
{
	for(int i = 0; i < TERM_H - 1; i++)
	{
		for(int j = 0; j < TERM_W; j++)
		{
			buffer[i][j] = buffer[i + 1][j];
		}
	}
	//Bottom row should be all blank
	for(int i = 0; i < TERM_W; i++)
		buffer[TERM_H - 1][i] = ' ';
	terminalUpdateScreen();
}

void parseCommand()
{
	
}
