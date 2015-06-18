#include "terminal.h"

#define TERM_W 80
#define TERM_H 30
#define STDIN_BUF_SIZE 512;

static char buffer[TERM_H][TERM_W];        //30 rows of 80 characters
static char stdinBuf[STDIN_BUF_SIZE];      //linear buffer of cstrings that are typed in
static int cursorX = 0;
static int cursorY = 0;
static char* const welcomeStr = "Welcome to GoldOS!\n";
static char* const prompt = ">> ";
static const char cursor = '_';
static const int promptLen = 3;	//column to put cursor in at prompt
static byte fgColor = 0xF;	//foreground
static byte bgColor = 0x4;	//background
static const int TAB_WIDTH = 4;
static int commandLen = 0;	//number of chars in command being typed, starting from promptLen.
static byte stdinWait = 0;	//true if inputting stdin string from libc, false if in shell

void initTerminal()
{
	clearTerminal(0);
	cursorX = 0;
	cursorY = 0;
	printString(welcomeStr);
	printString(prompt);
	cursorX = promptLen;
	drawChar(cursor, cursorX, cursorY, fgColor, bgColor);
	//terminalUpdateScreen();
}

void terminalKeyListener(byte scancode)
{
	if(ctrlPressed || altPressed)
		return;
	Scancode code = (Scancode) scancode;
	switch(code)
	{
		case KEY_ENTER:
		{
			buffer[cursorY][cursorX] = ' ';
			drawChar(' ', cursorX, cursorY, fgColor, bgColor);
			//Immediately run the command
			//Get cursor on a new line before running command
			cursorX = 0;
			int cmdStart = -1 + cursorY - ((commandLen + promptLen) / TERM_W);
			cursorY++;
			cmdStart++;
			if(cursorY == TERM_H)
			{
				shiftUp();
				cursorY--;
				cmdStart--;
			}
			if(stdinWait)
			{
			    stdinWait = 0;  //if here then in loop to read string for stdin
		            char* commandStart = (char*) &buffer[row][promptLen];
			    if(commandLen > STDIN_BUF_SIZE - 1)
				commandLen = STDIN_BUF_SIZE - 1;
			    for(int i = 0; i < commandLen; i++)
			    {
				stdinBuf[i] = commandStart[i];
			    }
			    stdinBuf[commandLen] = 0;
			    //now reset invisible cursor appropriately for libc application
			    cursorX = 0;
			    cursorY++;
			    if(cursorY == TERM_H - 1)
			    {
				shiftUp();
				cursorY--;
			    }
			    cursorX = 0;
			    break;          //setting that to false is signal to return there
			}
			parseCommand(cmdStart);	//Would normally load a program into RAM, invoke the runtime and jump execution to the program.
			//carriage return and newline
			//Erase cursor
			drawChar(' ', cursorX, cursorY, fgColor, bgColor);
			buffer[cursorY][cursorX] = ' ';
			cursorX = promptLen;
			cursorY++;
			if(cursorY == TERM_H)
			{
				shiftUp();
				cursorY--;
			}
			//Does not do that yet.
			//Draw prompt string on cursor's new row
			for(int i = 0;; i++)
			{
				if(prompt[i] == 0)
					break;
				drawChar(prompt[i], i, cursorY, fgColor, bgColor);
				buffer[cursorY][i] = prompt[i];
			}
			//Redraw cursor
			drawChar(cursor, cursorX, cursorY, fgColor, bgColor);
			buffer[cursorY][cursorX] = cursor;
			commandLen = 0;
			break;
		}
		case KEY_BACKSPACE:
		{
			if(commandLen == 0)
				break;
			//Erase cursor, back up, redraw cursor
			drawChar(' ', cursorX, cursorY, fgColor, bgColor);
			buffer[cursorY][cursorX] = ' ';
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
			buffer[cursorY][cursorX] = cursor;
			break;
		}
		default:
		{
			//Any other scancode is processed by its ASCII value, if any
			char other = getASCII(scancode);
			if(other)
			{
				drawChar(other, cursorX, cursorY, fgColor, bgColor);
				buffer[cursorY][cursorX] = other;
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
				buffer[cursorY][cursorX] = cursor;
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

//Prints the string at the cursor position
//Does NOT automatically newline, that needs to be in the string
void printString(char* const str)
{
	for(int i = 0;; i++)
	{
		switch(str[i])
		{
			case '\0':
				return;
			case '\t':	//tab
				//round X up to a TAB_WIDTH-column boundary
				if(cursorX % TAB_WIDTH == 0)
					cursorX += TAB_WIDTH;
				else
					cursorX = (1 + (cursorX / TAB_WIDTH)) * TAB_WIDTH;
				//put cursor at start of next line if past end of line
				if(cursorX >= TERM_W)
				{
					cursorX = 0;
					cursorY++;
					if(cursorY == TERM_H)
					{
						cursorY--;
						shiftUp();
					}
				}
				break;
			case '\n':	//newline
				cursorX = 0;
				cursorY++;
				if(cursorY == TERM_H)
				{
					cursorY--;
					shiftUp();
				}
				break;
			case '\b': //backspace
				if(cursorX > 0)
				{
					buffer[cursorY][cursorX] = ' ';
					drawChar(' ', cursorX, cursorY, fgColor, bgColor);
					cursorX--;
				}
				break;
			case '\r':	//Carriage return, go back to start of line
				cursorX = 0;
				break;
			case '\f':	//Form feed
				clearScreen(0);
				break;
			default:
				//Just draw the ASCII char
				drawChar(str[i], cursorX, cursorY, fgColor, bgColor);
				cursorX++;
				if(cursorX == TERM_W)
				{
					cursorX = 0;
					cursorY++;
					if(cursorY == TERM_H)
					{
						cursorY--;
						shiftUp();
					}
				}
		}
	}
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

void parseCommand(int row)
{
	//Stop drawing the cursor while the command is running
	//Figure out exactly where the command starts
	//commandLen gives the number of characters in command string, so (commandLen + promptLen) / TERM_W gives the number of complete lines in command
	//that means that command starts in the buffer at x = promptLen, y = cursorY - (commandLen + promptLen) / TERM_W
	//Rows stored contiguously.
	//When Enter is pressed, parseCommand is the first thing the terminal does so that cursor position preserved
	char* commandStart = (char*) &buffer[row][promptLen];
	commandStart[commandLen] = 0;	//Add a null-terminator in the buffer 
	printString(commandStart);
}

void clearTerminal(byte commandMode)
{
	//Clear buffer
	for(int i = 0; i < TERM_H; i++)
	{
		for(int j = 0; j < TERM_W; j++)
		{
			buffer[i][j] = ' ';
		}
	}
	cursorY = 0;
	if(commandMode)
	{
		for(int i = 0;; i++)
		{
			if(prompt[i] == 0)
				break;
			drawChar(prompt[i], i, 0, fgColor, bgColor);
		}
		cursorX = promptLen;
	}
	else
	{
		cursorX = 0;
	}
	terminalUpdateScreen();
}

const char* readString()
{
    stdinWait = 1;
    //wait for this to be set to 0 when ENTER is pressed
    //keyboard interrupts trigger terminal typing, this doesn't block that
    while(stdinWait);
    //string is in terminal buffer, copy into stdin buffer and return
    return (const char*) &stdinBuf[0];
}
