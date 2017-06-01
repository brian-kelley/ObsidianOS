#include "input.h"

u64 clockCounter = 0;

void drawChar(char c, int x, int y, byte fg, byte bg);

//ISR that does nothing except signal end-of-interrupt to the PIC
extern void pass();
//ISR that prints the error code and faulting eip, and then hangs
extern void generalISR();
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

static byte packet[3];
static int packetBytes = 0;
static void processMousePacket();

//global declarations
byte shiftPressed = 0;
byte ctrlPressed = 0;
byte altPressed = 0;
byte capsLockOn = 0;

static bool buttonDown[2];

bool getButtonState(int button)
{
  return buttonDown[button];
}

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

byte readControllerOutputPort()
{
  while(getKeyboardStatus() & KB_CANT_WRITE);
  writeport(0x64, 0xD0);
  return getKeyboardData();
}

void flushKeyboardData()
{
  while(keyboardHasOutput())
  {
    getKeyboardData();
  }
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
  dword idtAddress;
  //Set up a general handler for all exceptions first
  dword generalAddr = (dword) generalISR;
  for(int i = 0; i < 0x100; i++)
  {
    idt[i].offsetLower = generalAddr & 0xFFFF;
    idt[i].selector = 0x08;
    idt[i].zero = 0;
    idt[i].type_attr = 0x8E;
    idt[i].offsetHigher = (generalAddr & 0xFFFF0000) >> 16;
  }
  dword passAddr = (dword) pass;
  //set up PIT (system timer) interrupt handler (IRQ 0)
  idt[0x20].offsetLower = passAddr & 0xFFFF;
  idt[0x20].selector = 0x08;
  idt[0x20].zero = 0;
  idt[0x20].type_attr = 0x8E;
  idt[0x20].offsetHigher = (passAddr & 0xFFFF0000) >> 16;
  //set up keyboard interrupt handler (IRQ 1)
  dword keyboardAddress = (dword) keyboardISR;
  idt[0x21].offsetLower = keyboardAddress & 0xFFFF;
  idt[0x21].selector = 0x08;
  idt[0x21].zero = 0;
  idt[0x21].type_attr = 0x8E;
  idt[0x21].offsetHigher = (keyboardAddress & 0xFFFF0000) >> 16;
  //set up RTC interrupt handler (IRQ 8)
  dword rtcAddress = (dword) rtcISR;
  idt[0x28].offsetLower = rtcAddress & 0xFFFF;
  idt[0x28].selector = 0x08;
  idt[0x28].zero = 0;
  idt[0x28].type_attr = 0x8E;
  idt[0x28].offsetHigher = (rtcAddress & 0xFFFF0000) >> 16;
  //set up mouse isr (IRQ 12)
  dword mouseAddress = (dword) mouseISR;
  idt[0x2C].offsetLower = mouseAddress & 0xFFFF;
  idt[0x2C].selector = 0x08;
  idt[0x2C].zero = 0;
  idt[0x2C].type_attr = 0x8E;
  idt[0x2C].offsetHigher = (mouseAddress & 0xFFFF0000) >> 16;

  //enable PS/2 mouse packets and IRQ
  {
    byte ack;
    //enable second PS/2 port
    while(getKeyboardStatus() & KB_CANT_WRITE);
    writeport(0x64, 0xA8);
    getKeyboardData();
    //reset mouse
    while(getKeyboardStatus() & KB_CANT_WRITE);
    writeport(0x64, 0xD4);
    while(getKeyboardStatus() & KB_CANT_WRITE);
    writeport(0x60, 0xFF);
    while(getKeyboardStatus() & KB_CAN_READ)
    {
      getKeyboardData();
    }
    //get compaq status byte
    while(getKeyboardStatus() & KB_CANT_WRITE);
    writeport(0x64, 0x20);
    byte mouseStatus = getKeyboardData();
    //set bit 1 to enable IRQ 12 for mouse
    mouseStatus |= (1 << 1);
    //clear bit 5 to enable mouse internal clock
    mouseStatus &= ~(1 << 5);
    //send updated mouse status byte
    while(getKeyboardStatus() & KB_CANT_WRITE);
    writeport(0x64, 0x60);
    ioWait();
    while(getKeyboardStatus() & KB_CANT_WRITE);
    writeport(0x60, mouseStatus);
    //get ACK, if it was sent
    ioWait();
    flushKeyboardData();
    //enable mouse packet streaming
    while(getKeyboardStatus() & KB_CANT_WRITE);
    writeport(0x64, 0xD4);
    while(getKeyboardStatus() & KB_CANT_WRITE);
    writeport(0x60, 0xF4);
    if((ack = getKeyboardData()) != 0xFA)
    {
      printf("Failed to disable packet streaming: code %hhx\n", ack);
      while(1);
    }
    for(int i = 0; i < 2; i++)
    {
      buttonDown[i] = false;
    }
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
  //clear keyboard's data buffer
  while(keyboardHasOutput())
  {
    getKeyboardData();
  }
  loadIDT();
  disableInterrupts();
  //load IDT then enable interrupts
  //enable RTC interrupts
  writeport(0x70, 0x8B);
  byte prev = readport(0x71);
  writeport(0x70, 0x8B);
  writeport(0x71, prev | 0x40);
  //read register C to clear any previous interrupts
  //this also re-enables NMI
  writeport(0x70, 0xC);
  readport(0x71);
  enableInterrupts();
}

void keyboardHandler()
{
  byte status = getKeyboardStatus();
  byte event = getKeyboardData();
  bool pressed = true;
  if(event & 0x80)
  {
    pressed = false;
    event &= 0x7F;
  }
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
    default:
      {
        Event ev;
        ev.type = KEY_EVENT;
        ev.e.key.scancode = event;
        ev.e.key.pressed = pressed;
        addEvent(ev);
      }
  }
  //signal EOI
  writeport(0x20, 0x20);
  writeport(0xA0, 0x20);
}

void mouseHandler()
{
  packet[packetBytes++] = getKeyboardData();
  if(packetBytes == 3)
  {
    processMousePacket();
  }
  writeport(0x20, 0x20);
  writeport(0xA0, 0x20);
}

static void processMousePacket()
{
  packetBytes = 0;
  //check if mouse has a packet
  //dx/dy are signed 9 bit values
  unsigned short dxs = packet[1];
  unsigned short dys = packet[2];
  if(packet[0] & (1 << 4))
  {
    //dx is negative, set sign-bit and sign extend
    dxs |= 0xFF << 8;
  }
  if(packet[0] & (1 << 5))
  {
    //dy is negative, set sign-bit and sign extend
    dys |= 0xFF << 8;
  }
  //test for motion
  int dx = (short) dxs;
  int dy = (short) dys;
  if(dx != 0 || dy != 0)
  {
    //create mouse motion event
    Event ev;
    ev.type = MOTION_EVENT;
    ev.e.motion.dx = dx / 2;
    ev.e.motion.dy = -dy / 2;
    addEvent(ev);
  }
  //test for button state change
  byte mask = 1;
  for(int i = 0; i < 2; i++)
  {
    bool down = packet[0] & mask;
    mask <<= 1;
    if(down != buttonDown[i])
    {
      buttonDown[i] = down;
      Event ev;
      ev.type = BUTTON_EVENT;
      ev.e.button.button = LEFT_BUTTON;
      ev.e.button.pressed = down;
      addEvent(ev);
    }
  }
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

void rtcHandler()
{
  clockCounter++;
  //read RTC register C to enable the next tick
  writeport(0x70, 0xC);
  ioWait();
  readport(0x71);
  writeport(0x20, 0x20);
  writeport(0xA0, 0x20);
}

