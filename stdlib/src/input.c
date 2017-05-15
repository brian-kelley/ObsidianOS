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
#define KB_CANT_WRITE 2

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
    while(getKeyboardStatus() & KB_CANT_WRITE);
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
  dword passAddr = (dword) pass;
  //first, set all IRQ handlers to "pass"
  for(int i = 0x20; i < 0x30; i++)
  {
    idt[i].offsetLower = passAddr & 0xFFFF;
    idt[i].selector = 0x08;
    idt[i].zero = 0;
    idt[i].type_attr = 0x8E;
    idt[i].offsetHigher = (passAddr & 0xFFFF0000) >> 16;
  }
  //set up keyboard interrupt handler (IRQ 1)
  idt[0x20].offsetLower = keyboardAddress & 0xFFFF;
  idt[0x20].selector = 0x08;
  idt[0x20].zero = 0;
  idt[0x20].type_attr = 0x8E;
  idt[0x20].offsetHigher = (keyboardAddress & 0xFFFF0000) >> 16;
  //set up mouse interrupt handler (IRQ 12)
  dword mouseAddress = (dword) mouseInterrupt; 
  idt[0x2C].offsetLower = mouseAddress & 0xFFFF;
  idt[0x2C].selector = 0x08;
  idt[0x2C].zero = 0;
  idt[0x2C].type_attr = 0x8E;
  idt[0x2C].offsetHigher = (mouseAddress & 0xFFFF0000) >> 16;
  //enable PS/2 mouse packets and IRQ
  {
    //reset mouse
    while(getKeyboardStatus() & KB_CANT_WRITE);
    writeport(0x64, 0xD4);
    while(getKeyboardStatus() & KB_CANT_WRITE);
    writeport(0x60, 0xFF);
    while(getKeyboardStatus() & KB_CAN_READ)
    {
      getKeyboardData();
    }
    while(getKeyboardStatus() & KB_CANT_WRITE);
    writeport(0x64, 0x20);
    byte mouseStatus = getKeyboardData();
    //set bit 1 to enable IRQ
    mouseStatus |= (1 << 1);
    //clear bit 5 to enable mouse clock
    mouseStatus &= ~(1 << 5);
    while(getKeyboardStatus() & KB_CANT_WRITE);
    writeport(0x64, 0x60);
    ioWait();
    while(getKeyboardStatus() & KB_CANT_WRITE);
    writeport(0x60, mouseStatus);
    //get ACK, if it exists
    ioWait();
    if(getKeyboardStatus() & KB_CAN_READ)
    {
      readport(0x60);
    }
    while(getKeyboardStatus() & KB_CANT_WRITE);
    writeport(0x64, 0xA8);
    getKeyboardData();
    //enable packet streaming
    while(getKeyboardStatus() & KB_CANT_WRITE);
    writeport(0x64, 0xD4);
    while(getKeyboardStatus() & KB_CANT_WRITE);
    writeport(0x60, 0xF4);
  }
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
  //load IDT then enable interrupts
  loadIDT();
}

void requestMousePacket(byte* packet)
{
  //flush KB data buffer
  while(getKeyboardStatus() & KB_CAN_READ)
  {
    getKeyboardData();
  }
  while(getKeyboardStatus() & KB_CANT_WRITE);
  writeport(0x64, 0xD4);
  while(getKeyboardStatus() & KB_CANT_WRITE);
  writeport(0x60, 0xEB);
  //get ACK
  getKeyboardData();
  //read the 3 byte packet
  packet[0] = getKeyboardData();
  packet[1] = getKeyboardData();
  packet[2] = getKeyboardData();
}

void keyboardHandler()
{
  /*
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
  */
  writeport(0x20, 0x20);
  writeport(0xA0, 0x20);
  enableInterrupts();
}

int mouseX = 160;
int mouseY = 100;

void mouseHandler()
{
  //disableInterrupts();
  //read the 3 byte mouse packet through PS/2 controller
  byte packet[3];
  packet[0] = getKeyboardData();
  packet[1] = getKeyboardData();
  packet[2] = getKeyboardData();
  //flush extraneous ps/2 data
  while(getKeyboardStatus() & KB_CAN_READ)
  {
    getKeyboardData();
  }
  printf("Mouse packet: %hhx %hhx %hhx\n", packet[0], packet[1], packet[2]);
  //dx/dy are signed 9 bit values
  unsigned short dxs = packet[1];
  unsigned short dys = packet[2];
  if(packet[0] & (1 << 4))
  {
    //dx is negative, set sign-bit and sign extend
    dxs |= (0xFF) << 8;
  }
  if(packet[0] & (1 << 5))
  {
    //dy is negative, set sign-bit and sign extend
    dys |= (0xFF) << 8;
  }
  int dx = (short) dxs;
  int dy = (short) dys;
  if(dx != 0 || dy != 0)
  {
    //printf("%i %i\n", dx, dy);
    byte* fb = (byte*) 0xA0000;
    fb[mouseX + mouseY * 320] = 0x0;
    //clamp to screen
    mouseX += dx;
    mouseY -= dy;
    if(mouseX < 0)
      mouseX = 0;
    if(mouseY < 0)
      mouseY = 0;
    if(mouseX >= 320)
      mouseX = 319;
    if(mouseY >= 200)
      mouseY = 199;
    fb[mouseX + mouseY * 320] = 0xF;
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

