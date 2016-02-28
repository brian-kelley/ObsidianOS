#include "stdio.h"

FILE* stdout = NULL;

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
static int printUnsignedDec(unsigned long long int num, FILE* f, PrintFlags* pf);
static int printSignedDec(long long int num, FILE* f, PrintFlags* pf);
static int printOctal(unsigned long long int num, FILE* f, PrintFlags* pf);
static int printHex(unsigned long long int num, FILE* f, PrintFlags* pf);
static int printCstring(const char* str, FILE* f, PrintFlags* pf);
static int printDecFloat(long double num, FILE* f, PrintFlags* pf);
static int printSciFloat(long double num, FILE* f, PrintFlags* pf);
static int printShortestFloat(long double num, FILE* f, PrintFlags* pf);
static int printHexFloat(long double num, FILE* f, PrintFlags* pf);
static int printPointer(void* ptr, FILE* f, PrintFlags* pf);
static int paddedString(char* str, FILE* f, bool ljust, char pad, int width);
static int paddedStringSignExtend(char* str, FILE* f, bool ljust, char pad, int width);
//Parse format flags. fmt is right after '%', iter points to caller's iterator
static PrintFlags parseFlags(char* fmt, char** iter);

static void byteToStream(byte b, FILE* stream)
{
    if(stream == NULL)	//stdout
    {
        //stdout, write directly to terminal
        printChar(b);
    }
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

static int printUnsignedDec(unsigned long long int num, FILE* f, PrintFlags* pf)
{
    char buf[32];
    char* iter = buf;
    char* numStart = iter;
    if(num == 0)
        *(iter++) = '0';
    else
    {
        while(num > 0)
        {
            *(iter++) = decToChar(num % 10);
            num /= 10;
        }
    }
    //now iter points to char after the last digit
    *iter = 0;
    char* numEnd = iter - 1;
    //reverse all chars between numStart and numEnd (inclusive)
    while(numStart < numEnd)
    {
        char temp = *numStart;
        *numStart = *numEnd;
        *numEnd = temp;
        numStart++;
        numEnd--;
    }
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
        *(iter++) = '-';
    else if(pf->spaceForSign)
        *(iter++) = ' ';
    else if(pf->forceSign)
        *(iter++) = '+';
    if(num < 0)
        num = -num;
    char* numStart = iter;
    if(num == 0)
        *(iter++) = '0';
    else
    {
        while(num > 0)
        {
            *(iter++) = decToChar(num % 10);
            num /= 10;
        }
    }
    //now iter points to char after the last digit
    *iter = 0;
    char* numEnd = iter - 1;
    //reverse all chars between numStart and numEnd (inclusive)
    while(numStart < numEnd)
    {
        char temp = *numStart;
        *numStart = *numEnd;
        *numEnd = temp;
        numStart++;
        numEnd--;
    }
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
    return paddedString(buf, f, pf->ljust, pad, pf->width);
}

static int printCstring(const char* str, FILE* f, PrintFlags* pf)
{
    return paddedString(str, f, pf->ljust, ' ', pf->width);
}

static int printDecFloat(long double num, FILE* f, PrintFlags* pf)
{
    char buf[32];
    char* iter = buf;
    if(num < 0)
    {
        *(iter++) = '-';
        num = -num;
    }
    else if(pf->forceSign)
        *(iter++) = '+';
    else if(pf->spaceForSign)
        *(iter++) = ' ';
    //print integer part of the number
    unsigned long long ipart = num;
    if(ipart == 0)
        *(iter++) = '0';
    else
    {
        char* iStart = iter;
        while(num > 0)
        {
            *(iter++) = decToChar(num % 10);
            num /= 10;
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
    num -= ipart;
    if((pf->precision == 0 || num == 0) && pf->pound)
        //force decimal point
        *(iter++) = '.';
    else
    {
        for(int i = 0; i < pf->precision; i++)
        {
            num *= 10;
            *(iter++) = decToChar(num);
        }
    }
    char pad = ' ';
    if(!pf->ljust && pf->zeroPad)
        pad = '0';
    *iter = 0;
    return paddedStringSignExtend(buf, f, pf->ljust, pad, pf->width);
}

static int printSciFloat(long double num, FILE* f, PrintFlags* pf)
{
    char buf[100];
    char* iter = buf;
    bool sign = num < 0;
    num = fabsl(num);
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
    if(precision != 0 || pf->pound)
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
    if(pf->zeroPad && !ljust)
        pad = '0';
    return paddedStringSignExtend(buf, f, pf->ljust, pad, pf->width);
}

static int printShortestFloat(long double num, FILE* f, PrintFlags* pf)
{
    //Only use sci notation if the number is very big or very small
    if(abs((long long int) num) > 1e10 || fabsl(num * 1e10) < 10)
        return printSciFloat(num, f, upper);
    else
        return printDecFloat(num, f);
}

static int printHexFloat(long double num, FILE* f, PrintFlags* pf)
{
    //TODO
    return 0;
}

static int printPointer(void* ptr, FILE* f, PrintFlags* pf)
{
    char str[15];
    unsigned int val = (int) ptr;
    //32 bit integer in hex is always 8 digits
    str[0] = '0';
    str[1] = 'x';
    char* iter = &str[2];
    for(int i = 0; i < 8; i++)
    {
        *iter = hexToChar(val & 0xF0000000, pf->uppercase);
        //shift the next 4 bits into the same position
        val <<= 4;
        iter++;
    }
    *iter = 0;
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
    int success = fflush
        return 0;
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
        stream->buffer = buffer;
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
    return vfprintf(stdout, format, arg);
}

int scanf(const char* format, ...)
{
    return 0;
}

int sprintf(char* str, const char* format, ...)
{
    return 0;
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
    for(; *iter != 0; iter++)
    {
        //if character is not % and is printable, print it
        if(*iter != '%')
        {
            byteToStream(*iter, stream);
            charsPrinted++;
        }
        else
        {
            iter++;
            if(*iter == '%')
                byteToStream(*iter, stream);
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
                        int val = va_arg(arg, char);
                        charsPrinted += printSigned(val, stream, &pf);
                        break;
                    }
                    case U8:
                    {
                        unsigned val = va_arg(arg, unsigned char);
                        charsPrinted += printUnsigned(val, stream, &pf);
                        break;
                    }
                    case S16:
                    {
                        int val = va_arg(arg, short);
                        charsPrinted += printSigned(val, stream, &pf);
                        break;
                    }
                    case U16:
                    {
                        unsigned val = va_arg(arg, unsigned short);
                        charsPrinted += printUnsigned(val, stream, &pf);
                        break;
                    }
                    case S32:
                    {
                        int val = va_arg(arg, int);
                        charsPrinted += printSigned(val, stream, &pf);
                        break;
                    }
                    case U32:
                    {
                        unsigned val = va_arg(arg, unsigned int);
                        charsPrinted += printUnsigned(val, stream, &pf);
                        break;
                    }
                    case S64:
                    {
                        long long val = va_arg(arg, long long);
                        charsPrinted += printSigned(val, stream, &pf);
                        break;
                    }
                    case U64:
                    {
                        unsigned long long val = va_arg(arg, unsigned long long);
                        charsPrinted += printUnsigned(val, stream, &pf);
                        break;
                    }
                    case F64:
                    case F80:
                    {
                        long double val;
                        if(pf.type == F64)
                            val = va_arg(arg, double);
                        else
                            val = va_arg(arg, long double);
                        return printFloat(val, stream, &pf);
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
    return 0;
}

int vprintf(const char* format, va_list arg)
{
    return vfprintf(stdout, format, arg);
}

int vsprintf(char* s, const char* format, va_list arg)
{
    return 0;
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
    byte* iter = ptr;
    
    for(int i = 0; i < size; i++)
    {
        for(int j = 0; j < count; j++)
        {
            *(iter++) = byteFromStream()
        }
        if(!)
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
    return 0;
}

void clearerr(FILE* stream)
{
    if(stream->active)
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
    PrintFlags pf = getDefaultFlags();
    //get a char* pointing to the last char of the flags (type specifier)
    const char* terminals = "diuoxXfFeEgGaAcspn";
    char* end = fmt + strcspn(fmt, terminals);
    char* iter = fmt;
    bool pastDecimal = false;
    const char* digits = "0123456789";
    int sizeSpec = NONE;
    while(iter != end)
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
        if(*iter == '0')
            pf.zeroPad = true;
        if(*iter == '.')
            pastDecimal = true;
        if(*iter == '*' && !pastDecimal)
            pf.deferredWidth = true;
        if(*iter == '*' && pastDecimal)
            pf.deferredPrec = true;
        if(strchr(digits, *iter))
        {
            if(!pastDecimal)
            {
                //parse width number
                int width = 0;
                while(strchr(digits, *iter))
                {
                    width *= 10;
                    width += charToDec(*iter);
                    iter++;
                }
                pf.width = width;
            }
            else
            {
                //parse precision number
                int precision = 0;
                while(strchr(digits, *iter))
                {
                    precision *= 10;
                    precision += charToDec(*iter);
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
    *callerIter = end + 1;
    //Left-justified numbers must have spaces for padding
    if(pf.ljust)
        pf.zeroPad = false;
    return pf;
}

static int paddedString(char* str, FILE* f, bool ljust, char pad, int width)
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
    for(char* iter = str; *iter; iter++)
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

static int paddedStringSignExtend(char* str, FILE* f, bool ljust, char pad, int width)
{
    //If pad is '0', move sign char (str[0]) to front
    int num = 0;
    int len = strlen(str);
    char* newString = str;
    if(!ljust && pad == '0' && (str[0] == '+' || str[0] == '-' || str[0] == ' '))
    {
        byteToStream(str[0]);
        num++;
        newString++;
        width--;
    }
    return num + paddedString(str, f, ljust, pad, width);
}

static int printSigned(long long int val, FILE* f, PrintFlags* pf)
{
    if(pf.base == DEC)
        return printSignedDec(val, f, pf);
    else
    {
        //explicitly convert to unsigned for hex/octal
        unsigned long long uval = *((unsigned long long*) val);
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
