#include "input.h"

static idtEntry idt[256];

//Reset the keyboard device
int resetKeyboard()
{
	sendCommand:
	while(readport(0x64) & 2);	    //Wait for input buffer to clear before writing command byte
	writeport(0x60, 0xFF);			//Send reset command to keyboard device, NOT PS/2 controller
	while((readport(0x64) & 1) == 0);	//Wait for bit 0 of status register to be set, then read response byte
	byte result = readport(0x60);
	if(result == 0xFE)
		goto sendCommand;
	else if(result == 0xFA)
	{
		//Command acknowledged, wait for self-test results
		while((readport(0x64) & 1) == 0);
		result = readport(0x60);
	}
	switch(result)
	{
		case 0xAA:
			//Self-test passed, successfully reset keyboard
			return 0;
		case 0xFC:
		case 0xFD:
			//Self-test failed, something wrong with keyboard
			return 1;
		case 0xFE:
			//Try again
			goto sendCommand;
			break;
		default:;	//Result not ready yet, read again
	}
	return 1;	//Dead code
}

//Enable the keyboard to send output
int enableKeyboardScancodes()
{
	if(readport(0x64 & 1))
		readport(0x60);
	int numRetries = 3;	//when reaches 0, time out and report error
	while(numRetries > 0)
	{
		//Wait to write command
		while(readport(0x64) & 2);
		//Write command
		writeport(0x60, 0xF4);
		while((readport(0x64) & 1) == 0);
		byte result = readport(0x60);
		if(result == 0xFA)
			return 0;
		else if(result == 0xFE)
		{
			numRetries--;
		}
	}
	return 1;
}

//Send a command to the PS/2 controller (NOT the actual keyboard device)
void keyboardCommand(byte command)
{
	while(readport(0x64) & 2);	//Wait for bit 1 of status reg to clear, means input buffer is empty
	writeport(0x64, command);
}

void initKeyboard()
{
	dword keyboardAddress = (dword) keyboardHandler;
	dword idtAddress;
	dword idtPtr[2];
	idt[0x21].offsetLower = keyboardAddress & 0xFFFF;
	idt[0x21].selector = 0x10;
	idt[0x21].zero = 0;
	idt[0x21].type_attr = 0x8E;
	idt[0x21].offsetHigher = (keyboardAddress & 0xFFFF0000) >> 16;
	enableKeyboardScancodes();
	puts("Cleared keyboard buffer.", 40, 1);
	//Leave device 2 disabled unless adding mouse support later
	keyboardCommand(0xAE);
	puts("Resetting keyboard.", 40, 3);
	if(resetKeyboard())
		puts("Reset failed.     ", 40, 4);
	else
		puts("Reset succeeded."     , 40, 4);
	writeport(0x20, 0x11);
	writeport(0xA0, 0x11);
	writeport(0x21, 0x20);
	writeport(0xA1, 0x28);
	writeport(0x21, 4);
	writeport(0xA1, 2);
	writeport(0x21, 1);
	writeport(0xA1, 1);
	writeport(0x21, 0);
	writeport(0xA1, 0);
	idtAddress = (dword) idt;
	//First 2 bytes are size, next 4 bytes = idtAddress pointer
	idtPtr[0] = (sizeof(idtEntry) * 256 - 1) + ((idtAddress & 0xFFFF) << 16);
	idtPtr[1] = idtAddress >> 16;
	loadIDT(idtPtr);
	writeport(0x21, 0xFD);
	puts("Keyboard init complete.", 40, 5);
	puts("Status reg:", 0, 5);
	drawNum(readport(0x64), 6);
	enableInterrupts();
}

void keyboardHandler()
{
	disableInterrupts();
	while(!(readport(0x64) & 1));	//Wait until data is available to read from 0x60
	byte pressed;	//Assume pressed, set to 0 if released
	byte dataIn = readport(0x60);
	if(dataIn & 0x80)
	{
		pressed = 0;
		dataIn ^= 0x80;
	}
	else
	{
		pressed = 1;
	}
	if(pressed)
		puts("Key pressed:", 0, 12);
	else
		puts("Key released:", 0, 12);
	drawNum(dataIn, 13);
	//keyPressed(dataIn, pressed);
	//signal EOI
	writeport(0x20, 0x20);
	writeport(0xA0, 0x20);
	enableInterrupts();
}
