#ifndef INPUT_H
#define INPUT_H
#include "asmTypes.h"
#include "video.h"

void keyboardHandler();
extern void keyboardInterrupt();
extern void loadIDT(dword* idtPtr);
extern void drawNum(dword num, int row);
extern void keyPressed(byte scancode, byte pressed);
extern void enableInterrupts();	//sti
extern void disableInterrupts();	//cli

void initKeyboard();

typedef struct
{
	word offsetLower;
	word selector;
	byte zero;
	byte type_attr;
	word offsetHigher;
} idtEntry /*__attribute__((packed)) */;

#endif
