#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

extern uint16_t getchar();

void kernel_main()
{
	const int WIDTH = 80;
	const int HEIGHT = 25;
	int cursorX = 0;
	int cursorY = 0;
	uint16_t* textbuf = (uint16_t*) 0xB8000;
	uint8_t textColor = 0x4A;	//light blue on dark red background
	uint16_t blank = textColor << 8 | (uint16_t) ' ';
	for(uint16_t i = 0; i < WIDTH; i++)
	{
		for(uint16_t j = 0; j < HEIGHT; j++)
		{
			textbuf[i + j * WIDTH] = blank;
		}
	}
	const char* hello = "Hello World!";
	char* iter = (char*) hello;
	while(*iter != '\0')
	{
		uint16_t entry = textColor << 8 | (uint16_t) (*iter);
		textbuf[cursorX + cursorY * WIDTH] = entry;
		cursorX++;
		iter++;
	}
	uint16_t in = getchar();
	textbuf[WIDTH] = in | (textColor << 8);
}
