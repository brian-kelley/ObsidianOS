#ifndef INPUT_H
#define INPUT_H

#include "globalDefines.h"
#include "video.h"
#include "terminal.h"
#include "idt.h"

void keyboardHandler();
void initKeyboard();
char getASCII(byte scancode);
void initASCIIValues();
extern byte shiftPressed;
extern byte ctrlPressed;
extern byte altPressed;
extern byte capsLockOn;

extern void keyboardInterrupt();
extern void drawNum(dword num, int row);
extern void keyPressed(byte scancode, byte pressed);
extern void enableInterrupts();	//sti
extern void disableInterrupts();	//cli

#define KEY_QUEUE_MAX 16

//Ring buffer for key events
typedef struct
{
  byte data[KEY_QUEUE_MAX];
  int head;
  int size;
} KeyQueue;

extern KeyQueue keyQueue;

//if inListener, don't call high-level key event listener, just add key to queue and return
//the previously running listener will return and then see that there are more events to process
//this preserves order of events visible to programs, while keeping key events responsive
extern bool inListener;

//define Scancode type
typedef enum
{
	NONE0,	//scancode 0 doesn't exist, KEY_ESCAPE = 1
	KEY_ESCAPE,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_0,
	KEY_DASH,
	KEY_EQUALS,
	KEY_BACKSPACE,
	KEY_TAB,
	KEY_Q,
	KEY_W,
	KEY_E,
	KEY_R,
	KEY_T,
	KEY_Y,
	KEY_U,
	KEY_I,
	KEY_O,
	KEY_P,
	KEY_LEFTBRACKET,
	KEY_RIGHTBRACKET,
	KEY_ENTER,
	KEY_LEFTCONTROL,
	KEY_A,
	KEY_S,
	KEY_D,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_SEMICOLON,
	KEY_QUOTE,
	KEY_TILDE,
	KEY_LEFTSHIFT,
	KEY_BACKSLASH,
	KEY_Z,
	KEY_X,
	KEY_C,
	KEY_V,
	KEY_B,
	KEY_N,
	KEY_M,
	KEY_COMMA,
	KEY_PERIOD,
	KEY_SLASH,
	KEY_RIGHTSHIFT,
	KEY_NUMPAD_ASTERISK,
	KEY_LEFTALT,
	KEY_SPACE,
	KEY_CAPSLOCK,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_NUMLOCK,
	KEY_SCROLLLOCK,
	KEY_NUMPAD_7,
	KEY_NUMPAD_8,
	KEY_NUMPAD_9,
	KEY_NUMPAD_DASH,
	KEY_NUMPAD_4,
	KEY_NUMPAD_5,
	KEY_NUMPAD_6,
	KEY_NUMPAD_PLUS,
	KEY_NUMPAD_1,
	KEY_NUMPAD_2,
	KEY_NUMPAD_3,
	KEY_NUMPAD_0,
	KEY_NUMPAD_PERIOD,
	NONE1, NONE2, NONE3,	//3 skipped values in scancode set
	KEY_F11,
	KEY_F12
} Scancode;

#endif
