#include "stdio.h"

FILE* stdout = NULL;
FILE* stdin = NULL;

FILE stringPrinter;
FILE stringReader;

byte tmpFiles[TMP_MAX] = {0};

enum SizeSpec
{
  NONE,
  HH,
  H,
  L,
  LL,
  J,
  Z,
  T,
  LD
};

enum TypeSpec
{
  CHAR,
  S8,
  U8,
  S16,
  U16,
  S32,
  U32,
  S64,
  U64,
  F64,
  F80,
  STRING,
  POINTER
};

enum BaseSpec
{
  DEC,
  HEX,
  OCT
};

typedef struct
{
  int type;           //a TypeSpec value
  int base;           //a BaseSpec value
  int width;          //0, or the width if custom width
  int precision;
  bool ljust;	        //Wheter to left justify. Default = false
  bool forceSign; 	//Whether to write '+' preceding positive #s, default = false
  bool spaceForSign;  //Whether to write ' ' preceding positive #s, default = false
  bool pound;         //"0x" or "0" prefix, or force "." for floats
  bool uppercase;     //0X vs 0x, etc
  bool zeroPad;
  bool deferredWidth; //Whether to read width as int arg before prec and value
  bool deferredPrec;  //Whether to read precision as int arg right before float
  bool sciNot;        //scientific notation for floats
  bool useShortest;   //shortest repr for floats, true for Gg
  bool numPrinted;    //true for %n
} PrintFlags;

PrintFlags getDefaultFlags()
{
  PrintFlags pf;
  pf.type = S32;
  pf.base = DEC;
  pf.ljust = false;
  pf.forceSign = false;
  pf.spaceForSign = false;
  pf.pound = false;
  pf.uppercase = false;
  pf.zeroPad = false;
  pf.deferredWidth = false;
  pf.deferredPrec = false;
  pf.sciNot = false;
  pf.useShortest = false;
  pf.numPrinted = false;
  pf.width = 0;
  pf.precision = 6;
  return pf;
}

//non-interface functions
static void byteToStream(byte b, FILE* stream);
static byte byteFromStream(FILE* stream);
static char hexToChar(int digit, bool upper);
static int charToHex(char digit);
static char decToChar(int digit);
static int charToDec(char digit);
static char octToChar(int digit);
static int charToOct(char digit);
//Top-level numeric printing functions
static int printSigned(long long int, FILE* f, PrintFlags* pf);
static int printUnsigned(unsigned long long int, FILE* f, PrintFlags* pf);
static int printFloat(long double, FILE* f, PrintFlags* pf);
//Specific cases for different data
static int printUnsignedDec(unsigned long long val, FILE* f, PrintFlags* pf);
static int printSignedDec(long long val, FILE* f, PrintFlags* pf);
static int printOctal(unsigned long long int num, FILE* f, PrintFlags* pf);
static int printHex(unsigned long long int num, FILE* f, PrintFlags* pf);
static int printCstring(const char* str, FILE* f, PrintFlags* pf);
static int printDecFloat(long double num, FILE* f, PrintFlags* pf);
static int printSciFloat(long double num, FILE* f, PrintFlags* pf);
static int printShortestFloat(long double num, FILE* f, PrintFlags* pf);
static int printHexFloat(long double num, FILE* f, PrintFlags* pf);
static int printPointer(void* ptr, FILE* f, PrintFlags* pf);
static int paddedString(const char* str, FILE* f, bool ljust, char pad, int width);
static int paddedStringSignExtend(const char* str, FILE* f, bool ljust, char pad, int width);
//Parse format flags. fmt is right after '%', iter points to caller's iterator
static PrintFlags parseFlags(char* fmt, char** iter);

static void byteToStream(byte b, FILE* stream)
{
  if(stream == NULL)	//stdout (TODO: Make stdout a proper FILE)
    printChar(b);
  else if(stream == &stringPrinter)
    *(stringPrinter.buffer++) = b;
  else
  {
    if(!stream->canWrite)
    {
      stream->err = ILLEGAL_WRITE;
      return;
    }
  }
}

static byte byteFromStream(FILE* stream)
{
  return 0;
}

static char hexToChar(int digit, bool upper)
{
  if(digit >= 0 && digit < 10)
    return '0' + digit;
  else if(digit >= 10 && digit < 16)
  {
    if(upper)
      return 'A' + (digit - 10);
    else
      return 'a' + (digit - 10);
  }
  else
  {
    printString("Error: invalid hex digit (stdio.c)\n");
    return '!';
  }
}

static int charToHex(char digit)
{
  if(digit >= '0' && digit <= '9')
    return digit - '0';
  digit = tolower(digit);
  if(digit >= 'a' && digit <= 'f')
    return 10 + (digit - 'a');
  puts("Error: invalid hex digit!");
  return -1;
}

static char decToChar(int digit)
{
  if(digit >= 0 && digit < 10)
    return '0' + digit;
  puts("Error: invalid decimal digit!");
  return '!';
}

static int charToDec(char digit)
{
  if(digit >= '0' && digit <= '9')
    return digit - '0';
  puts("Error: invalid decimal digit!");
  return -1;
}

static char octToChar(int digit)
{
  if(digit >= 0 && digit < 8)
    return '0' + digit;
  puts("Error: invalid octal digit!");
  return '!';
}

static int charToOct(char digit)
{
  if(digit >= '0' && digit <= '8')
    return digit - '0';
  puts("Error: invalid octal digit!");
  return '!';
}

//local utility functions for printing different types
//each returns the number of characters actually printed to stdout

const unsigned long long tenPowers[19] = {
  1ULL,
  10ULL,
  100ULL,
  1000ULL,
  10000ULL,
  100000ULL,
  1000000ULL,
  10000000ULL,
  100000000ULL,
  1000000000ULL,
  10000000000ULL,
  100000000000ULL,
  1000000000000ULL,
  10000000000000ULL,
  100000000000000ULL,
  1000000000000000ULL,
  10000000000000000ULL,
  100000000000000000ULL,
  1000000000000000000ULL
};

//Print any unsigned int as decimal in buf, returning length (# digits)
static void printDecimal(char* buf, unsigned long long val)
{
  if(val == 0)
  {
    buf[0] = '0';
    buf[1] = 0;
  }
  else
  {
    //Use division by repeated subtraction
    //Definitely faster than using a 64-bit long division/remainder for each digit
    int level = 18;
    while(tenPowers[level] > val)
    {
      level--;
    }
    int charIndex = 0;
    while(level >= 0)
    {
      buf[charIndex] = '0';
      while(val >= tenPowers[level])
      {
        buf[charIndex]++;
        val -= tenPowers[level];
      }
      level--;
      charIndex++;
    }
    buf[charIndex] = 0;
  }
}

static int printUnsignedDec(unsigned long long val, FILE* f, PrintFlags* pf)
{
  char buf[32];
  printDecimal(buf, val);
  char pad = ' ';
  if(pf->zeroPad)
    pad = '0';
  return paddedStringSignExtend(buf, f, pf->ljust, pad, pf->width);
}

static int printSignedDec(long long int num, FILE* f, PrintFlags* pf)
{
  char buf[32];
  char* iter = buf;
  if(num < 0)
  {
    *(iter++) = '-';
    num = -num;
  }
  else if(pf->spaceForSign)
    *(iter++) = ' ';
  else if(pf->forceSign)
    *(iter++) = '+';
  printDecimal(iter, num);
  char pad = ' ';
  if(pf->zeroPad)
    pad = '0';
  return paddedStringSignExtend(buf, f, pf->ljust, pad, pf->width);
}

static int printOctal(unsigned long long int num, FILE* f, PrintFlags* pf)
{
  char buf[32];
  char* iter = buf;
  if(pf->pound)
    *(iter++) = '0';
  char* numStart = iter;
  if(num == 0)
    *(iter++) = '0';
  else
  {
    while(num > 0)
    {
      *(iter++) = octToChar(num & 7);
      num >>= 3;
    }
  }
  char* numEnd = iter - 1;
  while(numStart < numEnd)
  {
    char temp = *numStart;
    *numStart = *numEnd;
    *numEnd = temp;
    numStart++;
    numEnd--;
  }
  char pad = ' ';
  if(pf->zeroPad && !pf->ljust)
    pad = '0';
  return paddedString(buf, f, pf->ljust, pad, pf->width);
}

static int printHex(unsigned long long int num, FILE* f, PrintFlags* pf)
{
  char buf[32];
  char* iter = buf;
  if(pf->pound)
  {
    *(iter++) = '0';
    if(pf->uppercase)
      *(iter++) = 'X';
    else
      *(iter++) = 'x';
  }
  char* numStart = iter;
  if(num == 0)
    *(iter++) = '0';
  else
  {
    while(num > 0)
    {
      *(iter++) = hexToChar(num & 0xF, pf->uppercase);
      num >>= 4;
    }
  }
  *iter = 0;
  char* numEnd = iter - 1;
  while(numStart < numEnd)
  {
    char temp = *numStart;
    *numStart = *numEnd;
    *numEnd = temp;
    numStart++;
    numEnd--;
  }
  char pad = ' ';
  if(!pf->ljust && pf->zeroPad)
    pad = '0';
  return paddedStringSignExtend(buf, f, pf->ljust, pad, pf->width);
}

static int printCstring(const char* str, FILE* f, PrintFlags* pf)
{
  return paddedString(str, f, pf->ljust, ' ', pf->width);
}

static int printDecFloat(long double num, FILE* f, PrintFlags* pf)
{
  bool sign = false;
  if(num < 0)
  {
    sign = true;
    num = -num;
  }
  unsigned long long origIPart = num;
  //First, determine the fractional part string (doesn't include '.')
  char fpart[128];
  memset(fpart, 0, 128);
  fpart[0] = 0;
  long double frac = num - origIPart;
  if(pf->precision > 0)
  {
    //now if frac >= 0.5, propagate rounding up through the number
    char* fiter = fpart;
    for(int i = 0; i < pf->precision; i++)
    {
      frac *= 10;
      *(fiter++) = decToChar(frac);    //int conversion gives the single digit left of decimal
      frac -= (unsigned long long int) frac;
    }
    *(fiter--) = 0;
    //round according to remaining frac, and then propagate the increment through all digits
    bool carry = false;
    if(frac >= 0.5)
    {
      //increment digit at fiter, and continue back to the beginning of the string
      do
      {
        char inc = *fiter;
        if(inc == '9')
        {
          carry = true;
          *fiter = '0';
        }
        else
        {
          (*fiter)++;
          carry = false;
        }
        fiter--;
      }
      while(carry && fiter >= fpart);
    }
    if(carry)
      origIPart++;
  }
  char buf[256];
  memset(buf, 0, 256);
  char* iter = buf;
  if(sign)
    *(iter++) = '-';
  else if(false)
    *(iter++) = '+';
  else if(false)
    *(iter++) = ' ';
  if(origIPart == 0)
    *(iter++) = '0';
  else
  {
    char* iStart = iter;
    unsigned long long ipart = origIPart;   //ipart will be modified here
    while(ipart > 0)
    {
      *(iter++) = decToChar(ipart % 10);
      ipart /= 10;
    }
    char* iEnd = iter - 1;
    while(iStart < iEnd)
    {
      char temp = *iStart;
      *iStart = *iEnd;
      *iEnd = temp;
      iStart++;
      iEnd--;
    }
  }
  if(pf->precision > 0 || pf->pound)
    *(iter++) = '.';
  *iter = 0;
  //If there is a fractional part, append it to buf
  if(pf->precision > 0)
    strcpy(iter, fpart);
  char pad = ' ';
  if(!pf->ljust && pf->zeroPad)
    pad = '0';
  return paddedStringSignExtend(buf, f, pf->ljust, pad, pf->width);
}

static int printSciFloat(long double num, FILE* f, PrintFlags* pf)
{
  char buf[256];
  memset(buf, 0, 256);
  char* iter = buf;
  bool sign = num < 0;
  num = abs(num);
  int expo = log10l(num);
  num /= pow(10, expo);
  //print the significand, with the precision given by pf
  if(sign)
    *(iter++) = '-';
  else if(pf->forceSign)
    *(iter++) = '+';
  else if(!sign && pf->spaceForSign)
    *(iter++) = ' ';
  //sig has the form x.xxxxx...
  //always write the first digit
  *(iter++) = decToChar(num);
  num *= 10;
  if(pf->precision != 0 || pf->pound)
    *(iter++) = '.';
  for(int i = 0; i < pf->precision; i++)
  {
    *(iter++) = decToChar(num);
    num *= 10;
  }
  //now write the exponent
  if(pf->uppercase)
    *(iter++) = 'E';
  else
    *(iter++) = 'e';
  if(expo < 0)
    *(iter++) = '-';
  else
    *(iter++) = '+';
  if(expo < 0)
    expo = -expo;
  if(expo == 0)
  {
    *(iter++) = '0';
    *(iter++) = '0';
  }
  else if(expo < 10)
  {
    *(iter++) = '0';
    *(iter++) = charToDec(expo);
  }
  else
  {
    char* eStart = iter;
    while(expo > 0)
    {
      *(iter++) = charToDec(expo % 10);
      expo /= 10;
    }
    char* eEnd = iter - 1;
    while(eStart < eEnd)
    {
      char temp = *eStart;
      *eStart = *eEnd;
      *eEnd = temp;
      eStart++;
      eEnd--;
    }
  }
  *iter = 0;
  char pad = ' ';
  if(pf->zeroPad && !pf->ljust)
    pad = '0';
  return paddedStringSignExtend(buf, f, pf->ljust, pad, pf->width);
}

static int printShortestFloat(long double num, FILE* f, PrintFlags* pf)
{
  //Only use sci notation if the number is very big or very small
  if(abs((long long int) num) > 1e10 || fabsl(num * 1e10) < 10)
    return printSciFloat(num, f, pf);
  else
    return printDecFloat(num, f, pf);
}

static int printHexFloat(long double num, FILE* f, PrintFlags* pf)
{
  //TODO? very low priority
  return printSciFloat(num, f, pf);
}

static int printPointer(void* ptr, FILE* f, PrintFlags* pf)
{
  char str[11];   //a pointer is always 11 chars
  unsigned val = (unsigned int) ptr;
  unsigned mask = 0xF;
  //32 bit integer in hex is always 8 digits
  str[0] = '0';
  if(pf->uppercase)
    str[1] = 'X';
  else
    str[1] = 'x';
  for(int i = 0; i < 8; i++)
  {
    str[9 - i] = hexToChar(val & mask, pf->uppercase);
    val >>= 4;
  }
  str[10] = 0;
  return paddedString(str, f, pf->ljust, ' ', pf->width);
}

int remove(const char* fname)
{
  return deleteFile(fname);
}

int rename(const char* oldname, const char* newname)
{
  return 0;
}

FILE* tmpfile()
{
  return NULL;
}

char* tmpnam(char* str)
{
  return NULL;
}

int fclose(FILE* stream)
{
  if(!stream)
    return EOF;
  int success = fflush(stream);
  //TODO: Remove this file entry from fileList
  //fileListIter it = 
  return success;
}

int fflush(FILE* stream)
{
  return 0;
}

FILE* fopen(const char* fname, const char* mode)
{
  return NULL;
}

FILE* freopen(const char* fname, const char* mode, FILE* stream)
{
  fclose(stream);
  return fopen(fname, mode);
}

void setbuf(FILE* stream, char* buffer)
{
  //note: buffer can be NULL
  if(stream)
    stream->buffer = (byte*) buffer;
}

int setvbuf(FILE* stream, char* buffer, int mode, size_t size)
{
  if(mode == _IOFBF)
  {

  }
  else if(mode == _IOLBF)
  {

  }
  else if(mode == _IONBF)
  {

  }
  return 0;
}

int fprintf(FILE* stream, const char* format, ...)
{
  return 0;
}

int fscanf(FILE* stream, const char* format, ...)
{
  return 0;
}

int printf(const char* format, ...)
{
  va_list arg;
  va_start(arg, format);
  int num = vfprintf(stdout, format, arg);
  va_end(arg);
  return num;
}

int scanf(const char* format, ...)
{
  return 0;
}

int sprintf(char* str, const char* format, ...)
{
  va_list arg;
  va_start(arg, format);
  stringPrinter.buffer = str;
  int num = vfprintf(&stringPrinter, format, arg);
  va_end(arg);
  //null-terminate
  byteToStream(0, &stringPrinter);
  return num;
}

int sscanf(const char* str, const char* format, ...)
{
  return 0;
}

//note: vfprintf is the most general printing function, so it contains all logic
int vfprintf(FILE* stream, const char* format, va_list arg)
{
  //Make a mutable copy of the format string
  char* iter = (char*) format;
  int charsPrinted = 0; //caller can request # of characters printed so far with %n
  while(*iter)
  {
    //if character is not % and is printable, print it
    if(*iter != '%')
    {
      byteToStream(*iter, stream);
      charsPrinted++;
      iter++;
    }
    else
    {
      iter++;
      if(*iter == '%')
      {
        byteToStream(*iter, stream);
        iter++;
      }
      else
      {
        //this moves iter to the end of the item
        PrintFlags pf = parseFlags(iter, &iter);
        //read width & precision, if specified in print flags
        if(pf.deferredWidth)
          pf.width = va_arg(arg, int);
        if(pf.deferredPrec)
          pf.precision = va_arg(arg, int);
        //get the data type
        switch(pf.type)
        {
          case CHAR:
            {
              char val = va_arg(arg, int);
              char str[2] = {val, 0};
              charsPrinted += printCstring(str, stream, &pf);
              break;
            }
            //Integers:
          case S8:
            {
              i64 val = va_arg(arg, char);
              charsPrinted += printSigned(val, stream, &pf);
              break;
            }
          case U8:
            {
              u64 val = va_arg(arg, unsigned char);
              charsPrinted += printUnsigned(val, stream, &pf);
              break;
            }
          case S16:
            {
              i64 val = va_arg(arg, short);
              charsPrinted += printSigned(val, stream, &pf);
              break;
            }
          case U16:
            {
              u64 val = va_arg(arg, unsigned short);
              charsPrinted += printUnsigned(val, stream, &pf);
              break;
            }
          case S32:
            {
              i64 val = va_arg(arg, int);
              charsPrinted += printSigned(val, stream, &pf);
              break;
            }
          case U32:
            {
              u64 val = va_arg(arg, unsigned int);
              charsPrinted += printUnsigned(val, stream, &pf);
              break;
            }
          case S64:
            {
              i64 val = va_arg(arg, long long);
              charsPrinted += printSigned(val, stream, &pf);
              break;
            }
          case U64:
            {
              u64 val = va_arg(arg, unsigned long long);
              charsPrinted += printUnsigned(val, stream, &pf);
              break;
            }
          case F64:
          case F80:
            {
              long double val;
              if(pf.type == F64)
                val = va_arg(arg, double);
              else if(pf.type == F80)
                val = va_arg(arg, long double);
              charsPrinted += printFloat(val, stream, &pf);
              break;
            }
          case STRING:
            {
              const char* str = va_arg(arg, const char*);
              charsPrinted += printCstring(str, stream, &pf);
              break;
            }
          case POINTER:
            {
              void* ptr = va_arg(arg, void*);
              charsPrinted += printPointer(ptr, stream, &pf);
              break;
            }
          default:;
        }
      }
    }
  }
  return charsPrinted;
}

int vprintf(const char* format, va_list arg)
{
  return vfprintf(stdout, format, arg);
}

int vsprintf(char* s, const char* format, va_list arg)
{
  stringPrinter.buffer = s;
  int num = vfprintf(&stringPrinter, format, arg);
  //null-terminate
  byteToStream(0, &stringPrinter);
  return num;
}

int fgetc(FILE* stream)
{
  return 0;
}

char* fgets(char* str, int num, FILE* stream)
{
  return str;
}

int fputc(int character, FILE* stream)
{
  byteToStream(character, stream);
  return 0;
}

int fputs(const char* str, FILE* stream)
{
  byte* iter = (byte*) str;
  while(*iter)
    byteToStream(*(iter++), stream);
  byteToStream('\n', stream);
  return 0;
}

int getc(FILE* stream)
{
  return 'a';
}

int getchar()
{
  return 'a';
}

char* gets(char* str)
{
  return NULL;
}

int putc(int character, FILE* stream)
{
  byteToStream(character, stream);
  return 0;
}

int putchar(int character)
{
  byteToStream(character, stdout);
  return 0;
}

int puts(const char* str)
{
  byte* iter = (byte*) str;
  while(*iter)
    byteToStream(*(iter++), stdout);
  byteToStream('\n', stdout);
  return 0;
}

int ungetc(int character, FILE* stream)
{
  if(!stream->ungetFilled)
  {
    stream->unget = character;
    return 0;
  }
  else
    return EOF;
}

size_t fread(void* ptr, size_t size, size_t count, FILE* stream)
{
  size_t numRead = 0;
  //byte* iter = ptr;

  for(int i = 0; i < (int) size; i++)
  {
    for(int j = 0; j < (int) count; j++)
    {
      //TODO
      //TODO
      //TODO
    }
  }
  return numRead;
}

size_t fwrite(const void* ptr, size_t size, size_t count, FILE* stream)
{
  size_t numWritten = 0;

  return numWritten;
}

int fgetpos(FILE* stream, fpos_t* pos)
{
  return 0;
}

int fseek(FILE* stream, long int offset, int origin)
{
  return 0; 
}

int fsetpos(FILE* stream, const fpos_t* pos)
{
  return 0;
}

long int ftell(FILE* stream)
{
  return stream->pos;
}

void rewind(FILE* stream)
{
  fseek(stream, 0, 0);
}

void clearerr(FILE* stream)
{
  stream->err = NO_ERROR;
}

int feof(FILE* stream)
{
  return stream->eof;
}

int ferror(FILE* stream)
{
  return stream->err;
}

static PrintFlags parseFlags(char* fmt, char** callerIter)
{
  if(*fmt == '%')
    fmt++;
  PrintFlags pf = getDefaultFlags();
  //get a char* pointing to the last char of the flags (type specifier)
  const char* terminals = "diuoxXfFeEgGaAcspn";
  char* end = strpbrk(fmt, terminals);
  char* iter = fmt;
  bool pastDecimal = false;
  const char* digits = "0123456789";
  int sizeSpec = NONE;
  while(iter <= end)
  {
    //check for size specifier
    if(*iter == 'h')
    {
      if(*(iter + 1) == 'h')
      {
        iter++;
        sizeSpec = HH;
      }
      else
        sizeSpec = H;
    }
    if(*iter == 'l')
    {
      if(*(iter + 1) == 'l')
      {
        iter++;
        sizeSpec = LL;
      }
      else
        sizeSpec = L;
    }
    if(*iter == 'j')
      sizeSpec = J;
    if(*iter == 'z')
      sizeSpec = Z;
    if(*iter == 't')
      sizeSpec = T;
    if(*iter == 'L')
      sizeSpec = LD;
    //check for format options
    if(*iter == '-')
      pf.ljust = true;
    if(*iter == '+')
      pf.forceSign = true;
    if(*iter == ' ')
      pf.spaceForSign = true;
    if(*iter == '#')
      pf.pound = true;
    if(*iter == '.')
      pastDecimal = true;
    if(*iter == '0' && !pastDecimal)
      pf.zeroPad = true;
    if(*iter == '*' && !pastDecimal)
      pf.deferredWidth = true;
    if(*iter == '*' && pastDecimal)
      pf.deferredPrec = true;
    //Check for numerical argument for width or precision
    //Width can't start with 0 but precision can
    //If past '.', any numerical characters interpreted as precision
    if(strchr((char*) digits, *iter) && (*iter != '0' || pastDecimal))
    {
      if(!pastDecimal)
      {
        //parse width number
        pf.width = 0;
        while(strchr((char*) digits, *iter))
        {
          pf.width *= 10;
          pf.width += charToDec(*iter);
          iter++;
        }
      }
      else
      {
        //parse precision number
        pf.precision = 0;
        while(strchr((char*) digits, *iter))
        {
          pf.precision *= 10;
          pf.precision += charToDec(*iter);
          iter++;
        }
      }
      iter--;
    }
    iter++;
  }
  //now end points to the type character
  pf.uppercase = isupper(*end);
  char typeKey = tolower(*end);
  pf.numPrinted = *end == 'n';
  //if(pf.customWidth || pf.customPrecision)
  //printf("Width: %i Prec: %i\n", pf.width, pf.precision);
  switch(sizeSpec)
  {
    case Z:
      {
        //size_t always prints as unsigned
        if(typeKey == 'd' || typeKey == 'i')
          typeKey = 'u';
      }
    case NONE:
    case L:     //long int, size_t, ptrdiff_t all 32 bit
    case T:
      {
        switch(typeKey)
        {
          case 'n':
          case 'd':
          case 'i': pf.type = S32; pf.base = DEC; break;
          case 'u': pf.type = U32; pf.base = DEC; break;
          case 'o': pf.type = U32; pf.base = OCT; break;
          case 'x': pf.type = U32; pf.base = HEX; break;
          case 'f': pf.type = F64; pf.sciNot = false; break;
          case 'e': pf.type = F64; pf.sciNot = true; break;
          case 'g': pf.type = F64; pf.useShortest = true; break;
          case 'a': pf.type = F64; pf.base = HEX; break;
          case 'c': pf.type = CHAR; break;
          case 's': pf.type = STRING; break;
          case 'p': pf.type = POINTER; break;
          default:; // format string incorrect, do nothing
        }
        break;
      }
    case HH:
      {
        switch(typeKey)
        {
          case 'n':
          case 'd':
          case 'i': pf.type = S8; pf.base = DEC; break;
          case 'u': pf.type = U8; pf.base = DEC; break;
          case 'o': pf.type = U8; pf.base = OCT; break;
          case 'x': pf.type = U8; pf.base = HEX; break;
          default:;
        }
        break;
      }
    case H:
      {
        switch(typeKey)
        {
          case 'n':
          case 'd':
          case 'i': pf.type = S16; pf.base = DEC; break;
          case 'u': pf.type = U16; pf.base = DEC; break;
          case 'o': pf.type = U16; pf.base = OCT; break;
          case 'x': pf.type = U16; pf.base = HEX; break;
          default:;
        }
        break;
      }
    case J:
    case LL:
      {
        switch(typeKey)
        {
          case 'n':
          case 'd':
          case 'i': pf.type = S64; pf.base = DEC; break;
          case 'u': pf.type = U64; pf.base = DEC; break;
          case 'o': pf.type = U64; pf.base = OCT; break;
          case 'x': pf.type = U64; pf.base = HEX; break;
          default:;
        }
        break;
      }
    case LD:
      {
        switch(typeKey)
        {
          case 'f': pf.type = F80; pf.sciNot = false; break;
          case 'e': pf.type = F80; pf.sciNot = true; break;
          case 'g': pf.type = F80; pf.useShortest = true; break;
          case 'a': pf.type = F80; pf.base = HEX; break;
          default:;
        }
        break;
      }
  }
  //callerIter should now point to the character after the type specifier
  *callerIter = end + 1;
  //Left-justified numbers must have spaces for padding
  if(pf.ljust)
    pf.zeroPad = false;
  return pf;
}

static int paddedString(const char* str, FILE* f, bool ljust, char pad, int width)
{
  int num = 0;
  int len = strlen(str);
  if(!ljust && len < width)
  {
    for(int i = 0; i < width - len; i++)
    {
      byteToStream(pad, f);
      num++;
    }
  }
  for(char* iter = (char*) str; *iter; iter++)
  {
    byteToStream(*iter, f);
    num++;
  }
  if(ljust && len < width)
  {
    for(int i = 0; i < width - len; i++)
    {
      byteToStream(pad, f);
      num++;
    }
  }
  return num;
}

static int paddedStringSignExtend(const char* str, FILE* f, bool ljust, char pad, int width)
{
  //If pad is '0', move sign char (str[0]) to front
  int num = 0;
  int len = strlen(str);
  char* newString = (char*) str;
  if(!ljust && pad == '0' && (str[0] == '+' || str[0] == '-' || str[0] == ' '))
  {
    byteToStream(str[0], f);
    num++;
    newString++;
    width--;
  }
  else if(!ljust && pad == '0' && (str[0] == '0' && tolower(str[1]) == 'x'))
  {
    byteToStream('0', f);
    byteToStream(str[1], f);
    num += 2;
    newString += 2;
    width -= 2;
  }
  return num + paddedString(newString, f, ljust, pad, width);
}

static int printSigned(long long int val, FILE* f, PrintFlags* pf)
{
  if(pf->base == DEC)
    return printSignedDec(val, f, pf);
  else
  {
    //explicitly convert to unsigned for hex/octal
    unsigned long long uval = *((unsigned long long*) &val);
    return printUnsigned(uval, f, pf);
  }
}

static int printUnsigned(unsigned long long int val, FILE* f, PrintFlags* pf)
{
  if(pf->base == DEC)
    return printUnsignedDec(val, f, pf);
  else if(pf->base == HEX)
    return printHex(val, f, pf);
  else
    return printOctal(val, f, pf);
}

static int printFloat(long double val, FILE* f, PrintFlags* pf)
{
  if(pf->useShortest)
    return printShortestFloat(val, f, pf);
  else if(pf->sciNot)
    return printSciFloat(val, f, pf);
  else if(pf->base == HEX)
    return printHexFloat(val, f, pf);
  else
    return printDecFloat(val, f, pf);
}
