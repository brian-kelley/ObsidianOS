#include "input.h"

static idtEntry idt[256];

int resetKeyboard()
{
	sendCommand:
	while(readport(0x64) & 2 == 1);	//Wait for input buffer to clear before writing command byte
	writeport(0x60, 0xFF);			//Send reset command to keyboard device, NOT PS/2 controller
	puts("Waiting for keyboard to reset...                 ", 0, 14);
	while(readport(0x64) & 1 == 0);	//Wait for bit 0 of status register to be set, then read response byte
	byte result = readport(0x60);
	puts("Got result from keybaord reset:                  ", 0, 14);
	drawNum(result, 15);
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

//Send a command to the keyboard.
int keyboardCommand(byte command)
{
	if(command == 0xFF)
	{
		return resetKeyboard();
	}
	//All commands beside reset use the same 0xFA/0xFE responses
	//Send command byte
	sendCommand:
	while(readport(0x64) & 1);	//Wait for bit 1 of status reg to clear, means input buffer is empty
	writeport(0x64, command);
	int numRetries = 3;	//Number of retry attempts remaining
	//Read back result
	while(readport(0x64) & 1 == 0);
	byte result = readport(0x60);
	puts("Response byte from keyboard:", 0, 7);
	drawNum(result, 8);
	switch(result)
	{
		case 0xFA:
			//Command acknowledged
			return 0;
		case 0xFE:
			//Need to resend command or give up
			if(numRetries == 0)
			{
				//Needed to retry too many times, time out command
				puts("Keyboard command timed out:", 0, 10);
				drawNum(command, 11);
				return 1;
			}
			else
			{
				numRetries--;
				goto sendCommand;
			}
			break;
		default:;	//Just keep reading for the response byte
	}
	return 1;	//Dead code
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
	//puts("Disabling PS2 device 1", 0, 9);
	//keyboardCommand(0xAD);	//Disable device on PS2 channel 1
	puts("Resetting keyboard.", 12, 2);
	if(resetKeyboard())
		puts("Reset failed.", 12, 2);
	else
		puts("Reset succeeded.", 12, 2);
	//Enable keyboard
	keyboardCommand(0xAE);
}

void keyboardHandler()
{
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
}
