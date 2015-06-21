#ifndef TERMINAL_H
#define TERMINAL_H

#include "asmTypes.h"
#include "video.h"
#include "input.h"

void initTerminal();
void terminalKeyListener(byte scancode);
void terminalUpdateScreen();
void printString(char* const str);      //"echo", 
void shiftUp();	//Shift the buffer contents up one line
void parseCommand(int startRow);	//Command parser needs to know which row in buffer command starts
void clearTerminal(byte commandMode);	//commandMode: 1 if should draw cursor/prompt after clear, 0 otherwise
const char* readString();	        //read in string from stdin buffer

#endif
