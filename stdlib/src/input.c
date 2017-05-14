#include "input.h"

void drawChar(char c, int x, int y, byte fg, byte bg);

extern void pass();
//delay, sometimes needed after readport/writeport
extern void ioWait();

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

//global declarations
byte shiftPressed = 0;
byte ctrlPressed = 0;
byte altPressed = 0;
byte capsLockOn = 0;

KeyQueue keyQueue;
bool inListener;

#define KB_CAN_READ 1
#define KB_CAN_WRITE 2

//get keyboard status byte
byte getKeyboardStatus()
{
  return readport(0x64);
}

//Whether the keyboard has byte(s) to be read.
bool keyboardHasOutput()
{
  return getKeyboardStatus() & KB_CAN_READ;
}

byte getKeyboardData()
{
  while(!keyboardHasOutput());
  return readport(0x60);
}

//Send a command to the PS/2 controller (NOT the actual keyboard device)
byte keyboardCommand(byte command)
{
  int retry = 64;
  byte result = 0xFE;
  while(result == 0xFE && retry)
  {
    //Wait for bit 1 of status reg to clear, means input buffer is empty
    while(getKeyboardStatus() & KB_CAN_WRITE);
    //issue command
    writeport(0x64, command);
    //wait for response, then read it
    result = getKeyboardData();
    retry--;
  }
  return result;
}

void initKeyboard()
{
  keyQueue.head= 0;
  keyQueue.size= 0;
  inListener = false;
  dword idtAddress;
  dword idtPtr[2];
  //set up keyboard interrupt handler (0x21)
  dword keyboardAddress = (dword) keyboardInterrupt;
  idt[0x20].offsetLower = keyboardAddress & 0xFFFF;
  idt[0x20].selector = 0x08;
  idt[0x20].zero = 0;
  idt[0x20].type_attr = 0x8E;
  idt[0x20].offsetHigher = (keyboardAddress & 0xFFFF0000) >> 16;
  //set up mouse interrupt handler (0x2C)
  dword mouseAddress = (dword) mouseInterrupt; 
  idt[0x2B].offsetLower = mouseAddress & 0xFFFF;
  idt[0x2B].selector = 0x08;
  idt[0x2B].zero = 0;
  idt[0x2B].type_attr = 0x8E;
  idt[0x2B].offsetHigher = (mouseAddress & 0xFFFF0000) >> 16;
  //disable 2nd PS/2 port (mouse)
  while(getKeyboardStatus() & KB_CAN_WRITE);
  writeport(0x64, 0xAD);
  //In PIC, remap master and slave IRQ handlers to 0 (0x20)
  //this makes the keyboard interrupt 0x20 (matching IDT entry above)
  writeport(0x20, 0x11);
  ioWait();
  writeport(0xA0, 0x11);
  ioWait();
  writeport(0x21, 0x20);
  ioWait();
  writeport(0xA1, 0x28);
  ioWait();
  writeport(0x21, 4);
  ioWait();
  writeport(0xA1, 2);
  ioWait();
  writeport(0x21, 1);
  ioWait();
  writeport(0xA1, 1);
  ioWait();
  writeport(0x21, 0);
  ioWait();
  writeport(0xA1, 0);
  ioWait();
  idtAddress = (dword) idt;
  //First 2 bytes are size, next 4 bytes = idtAddress pointer
  idtPtr[0] = (sizeof(idtEntry) * 256 - 1) + ((idtAddress & 0xFFFF) << 16);
  idtPtr[1] = idtAddress >> 16;
  //clear keyboard's data buffer
  while(keyboardHasOutput())
  {
    getKeyboardData();
  }
  loadIDT();
}

void keyboardHandler()
{
  disableInterrupts();
  int dataBytes = 0;
  byte event = getKeyboardData();
  bool pressed = true;
  if(event & 0x80)
  {
    pressed = false;
    event &= 0x7F;
  }
  //now dataIn contains just the keycode, bit 7 clear
  //Process special keys
  switch((Scancode) event)
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
  keyPressed(event, pressed);
  //signal EOI
  writeport(0x20, 0x20);
  writeport(0xA0, 0x20);
  enableInterrupts();
}

void mouseHandler()
{
  puts("Hello from mouse handler!");
  disableInterrupts();
  //read the 3 byte mouse packet through PS/2 controller
  byte packet1 = getKeyboardData();
  byte packet2 = getKeyboardData();
  byte packet3 = getKeyboardData();
  //dx/dy are signed 9 bit values
  int dx = packet2;
  int dy = packet3;
  if(packet1 & (1 << 4))
  {
    //dx is negative, set sign-bit and sign extend
    dx |= (0xFF) << 24;
    dx |= (0xFF) << 16;
    dx |= (0xFF) << 8;
  }
  if(packet1 & (1 << 5))
  {
    //dy is negative, set sign-bit and sign extend
    dy |= (0xFF) << 24;
    dy |= (0xFF) << 16;
    dy |= (0xFF) << 8;
  }
  if(dx != 0 || dy != 0)
  {
    printf("dx: %i, dy: %i\n", dx, dy);
  }
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
    if((capsLockOn == 1 && shiftPressed == 0) || (capsLockOn == 0 && shiftPressed == 1))
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

