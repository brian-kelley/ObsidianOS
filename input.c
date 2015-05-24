#include "input.h"

void initIDT()
{
	dword keyboardAddress;
	dword idtAddress;
	dword idtPtr[2];
	keyboardAddress = (dword) keyboardHandler;
	idt[0x21].offsetLower = keyboardAddress & 0xFFFF;
	idt[0x21].selector = 0x8;
	idt[0x21].zero = 0;
	idt[0x21].type_attr = 0x8E;
	idt[0x21].offsetHigher = (keyboardAddress & 0xFFFF0000) >> 16;
	setupIDT();
	idtAddress = (dword) idt;
	idtPtr[0] = (sizeof(idtEntry) * 256) + ((idtAddress & 0xFFFF) << 16);
	idtPtr[1] = idtAddress >> 16;
	loadIDT(idtPtr);
	enableKeyboard();
}
