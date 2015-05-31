#include "input.h"

typedef struct
{
	word offsetLower;
	word selector;
	byte zero;
	byte type_attr;
	word offsetHigher;
} idtEntry;

static idtEntry idt[256];

static char charVals[] =
{
	//Chars without Shift
	0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0, 0,
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
	'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ',
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	'7', '8', '9', '-', 
	'4', '5', '6', '+', 
	'1', '2', '3', '0', '.', 
	0, 0, 0, 0, 0,
	//Chars with Shift
	0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, 0,
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0, 0,
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0, '|',
	'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ',
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	'7', '8', '9', '-', 
	'4', '5', '6', '+', 
	'1', '2', '3', '0', '.', 
	0, 0, 0, 0, 0
};

byte shiftPressed = 0;
byte ctrlPressed = 0;
byte altPressed = 0;
byte capsLockOn = 0;

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
	keyboardCommand(0xAE);
	if(resetKeyboard())
		puts("Keyboard reset failed.     ", 0, 0);
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
	enableInterrupts();
}

void keyboardHandler()
{
	disableInterrupts();	//Don't want another keyboard IRQ to fire during this function
	while(!(readport(0x64) & 1));	//Wait until data is available to read from 0x60
	byte pressed;	//Assume pressed, set to 0 if released
	byte dataIn = readport(0x60);
	if(dataIn & 0x80)
	{
		pressed = 0;
		dataIn &= 0x7F;
	}
	else
	{
		pressed = 1;
	}
	/*
	if(pressed)
		puts("Key pressed: ", 0, 28);
	else
		puts("Key released:", 0, 28);
	drawNum(dataIn, 29);
	*/
	//now dataIn contains just the keycode, bit 7 clear
	//Process special keys
	switch((Scancode) dataIn)
	{
		case KEY_LEFTSHIFT:
		case KEY_RIGHTSHIFT:
			if(pressed)
				shiftPressed = 1;
			else
				shiftPressed = 0;
			break;
		case KEY_CAPSLOCK:
			if(pressed)
				capsLockOn = capsLockOn ? 0 : 1;
			break;
		case KEY_LEFTALT:
			if(pressed)
				altPressed = 1;
			else
				altPressed = 0;
			break;
		case KEY_LEFTCONTROL:
			if(pressed)
				ctrlPressed = 1;
			else
				ctrlPressed = 0;
			break;
		default:;
	}
	drawNum(KEY_CAPSLOCK, 28);
	drawNum(dataIn, 29);
	keyPressed(dataIn, pressed);
	//signal EOI
	writeport(0x20, 0x20);
	writeport(0xA0, 0x20);
	enableInterrupts();
}

char getASCII(byte scancode)
{
	scancode &= 0x7F;	//Don't care if pressed or released
	//Don't want to go out of bounds in charVals array:
	//Also anything with a scancode bigger than KEY_F12 is a media or ACPI key, ignore them
	const int NUMKEYS = 0x59;	//Number of keys included in charVal array
	if(scancode > NUMKEYS)
		return 0;
	//Result depends on type of character.
	//If alphabetical, capitalization depends on capslock state:
	if(charVals[scancode] >= 'a' && charVals[scancode] <= 'z')
	{
		if((capsLockOn && !shiftPressed) || (!capsLockOn && shiftPressed))
			return charVals[scancode + NUMKEYS];
		else
			return charVals[scancode];
	}
	//Otherwise, don't care about capslock and just use shift to modify
	else
	{
		if(shiftPressed)
			return charVals[scancode + NUMKEYS];
		else
			return charVals[scancode];
	}
}
