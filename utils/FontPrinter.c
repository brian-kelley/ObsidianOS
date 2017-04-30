#include "stdio.h"

int main()
{
  FILE* bin = fopen("../stdlib/src/font.bin", "rb");
  unsigned char glyph[16];
  while(!feof(bin))
  {
    fread(glyph, 1, 16, bin);
    for(int i = 0; i < 16; i++)
    {
      for(unsigned char mask = 1; mask; mask <<= 1)
      {
        if(glyph[i] & mask)
        {
          putchar('*');
        }
        else
        {
          putchar(' ');
        }
      }
      putchar('\n');
    }
    putchar('\n');
  }
  fclose(bin);
  int x = '!';
  printf("%i\n", x);
  return 0;
}

