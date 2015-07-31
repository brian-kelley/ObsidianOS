#ifndef TERMINAL_H
#define TERMINAL_H

#include <stddef.h>
#include "globalDefines.h"
#include "video.h"
#include "input.h"

#define TERM_W 80
#define TERM_H 30

void initTerminal();
void terminalKeyListener(byte scancode);
void terminalUpdateScreen();
void printString(const char* str);      //used for puts/putc/printf
void printChars(const char* str, size_t numChars); //print a given number of chars within string
void printChar(char ch);                   //general print character
void shiftUp();	//Shift the buffer contents up one line
void parseCommand(int startRow);	//Command parser needs to know which row in buffer command starts
void clearTerminal(byte commandMode);	//commandMode: 1 if should draw cursor/prompt after clear, 0 otherwise
const char* readString();	        //read in string from stdin buffer

#endif
