#include "input.h"

static idtEntry idt[256];

void initKeyboard()
{
	dword keyboardAddress = (dword) keyboardHandler;
	drawNum(keyboardHandler, 0);
	dword idtAddress;
	dword idtPtr[2];
	idt[0x21].offsetLower = keyboardAddress & 0xFFFF;
	idt[0x21].selector = 0x10;
	idt[0x21].zero = 0;
	idt[0x21].type_attr = 0x8E;
	idt[0x21].offsetHigher = (keyboardAddress & 0xFFFF0000) >> 16;
	writeport(0x20, 0x11);
	writeport(0xA0, 0x11);
	writeport(0x21, 0x20);
	writeport(0xA1, 0x28);
	writeport(0x21, 0);
	writeport(0xA1, 0);
	writeport(0x21, 1);
	writeport(0xA1, 1);
	writeport(0x21, 0xFF);
	writeport(0xA1, 0xFF);
	idtAddress = (dword) idt;
	//First 2 bytes are size, next 4 bytes = idtAddress pointer
	idtPtr[0] = (sizeof(idtEntry) * 256 - 1) + ((idtAddress & 0xFFFF) << 16);
	idtPtr[1] = idtAddress >> 16;
	loadIDT(idtPtr);
	writeport(0x21, 0xFD);
}

void keyboardHandler()
{
	writeport(0x20, 0x20);
	byte isData = readport(0x64);
	if(isData & 1)
	{
		keyPressed(readport(0x60));
	}
}
