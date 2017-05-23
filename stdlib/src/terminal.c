#include "terminal.h"

#define STDIN_BUF_SIZE 512
#define COMMAND_MAX 2000

extern void demo();

static char buffer[TERM_H][TERM_W];
static char stdinBuf[STDIN_BUF_SIZE];      //linear buffer of cstrings that are typed in
static int cursorX = 0;
static int cursorY = 0;
static char* const welcomeStr = "ObsidianOS v0.1\n";
static char* const prompt = ">> ";
static const char cursor = '_';
static const int promptLen = 3;	//column to put cursor in at prompt
static byte fgColor = 0xF;	//foreground
static byte bgColor = 0x0;	//background
static const int TAB_WIDTH = 4;
static int commandLen;		//number of chars in command being typed
static byte stdinWait = 0;	//true if inputting stdin string from libc, false if in shell
static volatile byte runningDemo = 0;

void initTerminal()
{
  memset(buffer, ' ', TERM_W * TERM_H);
  cursorX = 0;
  cursorY = 0;
  printString(welcomeStr);
  cursorY = 1;
  printString(prompt);
  cursorX = promptLen;
  commandLen = 0;
}

void terminalKeyListener(byte scancode)
{
  runningDemo = false;
  Scancode code = (Scancode) scancode;
  if(code == KEY_C && ctrlPressed)
  {
    runningDemo = false;
    return;
  }
  switch(code)
  {
    case KEY_ENTER:
      {
        buffer[cursorY][cursorX] = ' ';
        drawChar(' ', cursorX, cursorY, fgColor, bgColor);
        //Immediately run the command
        //Get cursor on a new line before running command
        char* cmdStart = &buffer[cursorY][cursorX] - commandLen;
        cursorX = 0;
        cursorY++;
        if(cursorY == TERM_H)
        {
          shiftUp();
          cursorY--;
          cmdStart -= TERM_W;
        }
        if(stdinWait)
        {
          stdinWait = 0;  //if here then in loop to read string for stdin
          //TODO: Let stdin read from the input string
          break;
        }
        parseCommand(cmdStart);	//Would normally load a program into RAM, invoke the runtime and jump execution to the program. So treat that line as an arbitrary program running.
        //Leave the cursor on the current line if that line is empty.
        resetTermCursor();
        commandLen = 0;
        break;
      }
    case KEY_BACKSPACE:
      {
        if(commandLen == 0)	 //can't back up any further
          break;
        //Erase cursor, back up, redraw cursor
        drawChar(' ', cursorX, cursorY, fgColor, bgColor);
        buffer[cursorY][cursorX] = ' ';
        commandLen--;
        if(cursorX == 0)
        {
          cursorX = TERM_W - 1;
          cursorY--;
        }
        else
        {
          cursorX--;
        }
        drawChar(cursor, cursorX, cursorY, fgColor, bgColor);
        buffer[cursorY][cursorX] = cursor;
        break;
      }
    default:
      {
        //Any other scancode is processed by its ASCII value, if any
        char other = getASCII(scancode);
        if(other)
        {
          drawChar(other, cursorX, cursorY, fgColor, bgColor);
          buffer[cursorY][cursorX] = other;
          cursorX++;
          commandLen++;
          if(cursorX == TERM_W)
          {
            cursorX = 0;
            //goto next line
            cursorY++;
            if(cursorY == TERM_H)
            {
              shiftUp();
              cursorY--;
            }
          }
          drawChar(cursor, cursorX, cursorY, fgColor, bgColor);
          buffer[cursorY][cursorX] = cursor;
        }
      }
  }
}

void terminalUpdateScreen()
{
  for(int i = 0; i < TERM_H; i++)
  {
    for(int j = 0; j < TERM_W; j++)
    {
      drawChar(buffer[i][j], j, i, fgColor, bgColor);
    }
  }
}

//Print a single character or do escape code
void printChar(char ch)
{
  switch(ch)
  {
    case '\0':
      {
        return;
      }
    case '\t':	//tab
      {
        //round X up to a TAB_WIDTH-column boundary
        if(cursorX % TAB_WIDTH == 0)
          cursorX += TAB_WIDTH;
        else
          cursorX = (1 + (cursorX / TAB_WIDTH)) * TAB_WIDTH;
        //put cursor at start of next line if past end of line
        if(cursorX >= TERM_W)
        {
          cursorX = 0;
          cursorY++;
          if(cursorY == TERM_H)
          {
            cursorY--;
            shiftUp();
          }
        }
        break;
      }
    case '\n':	//newline
      {
        cursorX = 0;
        cursorY++;
        if(cursorY == TERM_H)
        {
          cursorY--;
          shiftUp();
        }
        break;
      }
    case '\b': //backspace
      {
        if(cursorX > 0)
        {
          buffer[cursorY][cursorX] = ' ';
          drawChar(' ', cursorX, cursorY, fgColor, bgColor);
          cursorX--;
        }
        break;
      }
    case '\r':	//Carriage return, go back to start of line
      {
        cursorX = 0;
        break;
      }
    case '\f':	//Form feed
      {
        clearScreen(0);
        break;
      }
    case ' ':
      {
        cursorX++;
        if(cursorX == TERM_W)
        {
          cursorX = 0;
          cursorY++;
        }
        if(cursorY == TERM_H)
        {
          cursorY--;
          shiftUp();
        }
        break;
      }
    default:
      {
        //if character is not drawable, return
        if(ch < '!' || ch > '~')
          return;
        drawChar(ch, cursorX, cursorY, fgColor, bgColor);
        buffer[cursorY][cursorX] = ch;
        cursorX++;
        if(cursorX == TERM_W)
        {
          cursorX = 0;
          cursorY++;
          if(cursorY == TERM_H)
          {
            cursorY--;
            shiftUp();
          }
        }
      }
  }
}

//Prints the string at the cursor position
//Does NOT automatically newline, that needs to be in the string
void printString(const char* str)
{
  char* iter = (char*) str;
  while(*iter != 0)
  {
    printChar(*iter);
    iter++;
  }
}

void printChars(const char* str, size_t numChars)
{
  for(int i = 0; i < (int) numChars; i++)
    printChar(str[i]);
}

void shiftUp()
{
  for(int i = 0; i < TERM_H - 1; i++)
  {
    for(int j = 0; j < TERM_W; j++)
    {
      buffer[i][j] = buffer[i + 1][j];
    }
  }
  //Bottom row should be all blank
  for(int i = 0; i < TERM_W; i++)
    buffer[TERM_H - 1][i] = ' ';
  terminalUpdateScreen();
}

void parseCommand(char* start)
{
  char cmd[COMMAND_MAX + 1];
  if(commandLen > COMMAND_MAX)
  {
    puts("Terminal error: command too long to process!");
    return;
  }
  memcpy(cmd, start, commandLen);
  cmd[commandLen] = 0;
  //Stop drawing the cursor while the command is running
  //Figure out exactly where the command starts
  //commandLen gives the number of characters in command string, so (commandLen + promptLen) / TERM_W gives the number of complete lines in command
  //that means that command starts in the buffer at x = promptLen, y = cursorY - (commandLen + promptLen) / TERM_W
  //Rows stored contiguously.
  //When Enter is pressed, parseCommand is the first thing the terminal does so that cursor position preserved
  if(strcmp("ls", cmd) == 0)
    ls(NULL);
  else if(strcmp("cd", cmd) == 0)
    cd(NULL);
  else if(strcmp("clear", cmd) == 0)
    clearTerminal(false);
  else if(strcmp("date", cmd) == 0)
    dateCommand();    //see time.c
  else if(strcmp("uptime", cmd) == 0)
    printf("%.4f minutes\n", clock() / (CLOCKS_PER_SEC * 60.0));
  else if(strcmp("demo", cmd) == 0)
  {
    runningDemo = true;
    demo();
  }
  /* DO THE COMMAND */
}

void clearTerminal(bool showPrompt)
{
  //Clear buffer
  for(int i = 0; i < TERM_H; i++)
  {
    for(int j = 0; j < TERM_W; j++)
    {
      buffer[i][j] = ' ';
    }
  }
  cursorY = 0;
  if(showPrompt)
  {
    printString(prompt);
    cursorX = promptLen;
  }
  else
    cursorX = 0;
  terminalUpdateScreen();
}

const char* readString()
{
  stdinWait = 1;
  //wait for this to be set to 0 when ENTER is pressed
  //keyboard interrupts trigger terminal typing, this doesn't block that
  while(stdinWait);
  //string is in terminal buffer, copy into stdin buffer and return
  return (const char*) &stdinBuf[0];
}

void resetTermCursor()
{
  cursorX = 0;
  printString(prompt);
  cursorX = promptLen;
  drawChar(cursor, cursorX, cursorY, fgColor, bgColor);
}

void ls(const char* args)
{
  //TODO: store the "working directory" and list files in there
  //TODO: cache some portion of the filesystem tree for ls/cd speed
  /* TEMPORARY TEST VERSION */
  //iterate through root directory entries and print populated ones
  printf("max %i entries in root\n", getMaxRootEntries());
  return;
  for(int i = 0; i < getMaxRootEntries(); i++)
  {
    DirEntry ent = getRootDirEntry(i);
    byte indicator = ent.filename[0];
    if(indicator == 0)
      break;
    else if(indicator == 0xE5 || indicator == 0x05)
      continue;
    char name[14];
    name[8] = '.';
    name[13] = 0;
    byte attrib = ent.attributes;
    memcpy(name, &ent.filename[0], 8);
    memcpy(name + 9, &ent.fileExt[0], 3);
    for(char* iter = (char*) &ent; iter < ((char*) &ent) + sizeof(DirEntry); iter++)
    {
      if('!' <= *iter && *iter <= '~')
        putchar(*iter);
      else
        putchar('.');
    }
    printf("\"%s\" ", name);
    if(attrib & 1)
      putchar('R');
    if(attrib & 2)
      putchar('H');
    if(attrib & 4)
      putchar('S');
    if(attrib & 8)
      printString("(volume label)");
    if(attrib & 16)
      putchar('D');
    puts("");
  }
}

void cd(const char* args)
{

}

void hexdump(void* data, size_t num)
{
  byte* arr = data;
  char str[17];
  str[16] = 0;
  for(size_t i = 0; i < num; i++)
  {
    printf("%02hhx ", arr[i]);
    if('!' <= arr[i] && arr[i] <= '~')
      str[i % 16] = arr[i];
    else
      str[i % 16] = '.';
    if(i % 16 == 15)
      printf("%23s\n", str);
    else if(i == num - 1)
    {
      str[i % 16 + 1] = 0;
      printf("%*s\n", 23 + 2 * (15 - i % 16), str);
    }
  }
}

void demo()
{
  puts("");
  double theta = 0;
  double omega = 0.2; //added to theta each line
  double amplitude = 20;
  int h, left, right;
  //a Ctrl+C interrupt will jump out of loop
  for(int j = 0; j < 200; j++)
  {
    h = cos(theta) * amplitude;
    theta += omega;
    if(theta > 2 * PI)
      theta -= 2 * PI;
    left = 20 - h;
    right = 20;
    if(left > right)
    {
      right = left;
      left = 20;
    }
    for(int i = 0; i < left; i++)
      putchar(' ');
    for(int i = left; i < right; i++)
      putchar('-');
    for(int i = right; i < 40; i++)
      putchar(' ');
  }
}

