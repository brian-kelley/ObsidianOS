#ifndef INPUT_H
#define INPUT_H
#include "asmTypes.h"

extern void keyboardHandler();
extern void loadIDT(dword* idtPtr);
extern void setupIDT();
extern void enableKeyboard();

void initIDT();

typedef struct
{
	word offsetLower;
	word selector;
	byte zero;
	byte type_attr;
	word offsetHigher;
} idtEntry;

idtEntry idt[256];
#endif
