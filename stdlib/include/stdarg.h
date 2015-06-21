#ifndef __STDARG_H__
#define __STDARG_H__

typedef unsigned char* va_list;
#define va_start(ap, last) ap = ((va_list) &(last)) + ((sizeof(last) + 3) & ~3)
#define va_arg(ap, type) (ap += (sizeof(type) + 3) & ~3, *(type*)(ap - ((sizeof(type) + 3) & ~3)))
#define va_copy(dest, src) (dest) = (src)
#define va_end(ap)

#endif
