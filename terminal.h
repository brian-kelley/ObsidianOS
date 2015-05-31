#ifndef TERMINAL_H
#define TERMINAL_H

#include "asmTypes.h"
#include "video.h"
#include "input.h"

void initTerminal();
void terminalKeyListener(byte scancode);
void terminalUpdateScreen();
void printString(char* const str);
void shiftUp();	//Shift the buffer contents up one line
void parseCommand();

#endif
