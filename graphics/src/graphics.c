#include "graphics.h"

byte* const framebuf = (byte*) 0xA0000;
byte color;

void drawLine(int x1, int y1, int x2, int y2)
{
	int changed, x, y, dx, dy, signx, signy, i, e, temp;
	x = x1;
	y = y1;
	dx = x2 - x1;
	if(dx < 0)
		dx = -dx;
	dy = y2 - y1;
	if(dy < 0)
		dy = -dy;
	signx = x2 - x1;
	if(signx < 0)
		signx = -1;
	else
		signx = 1;
	signy = y2 - y1;
	if(signy < 0)
		signy = -1;
	else
		signy = 1;
	if(dy > dx)
	{
		temp = dy;
		dy = dx;
		dx = temp;
		changed = 1;
	}
	else
	{
		changed = 0;
	}
	e = (dy << 1) - dx;
	for(i = 0; i <= dx; i++)
	{
		drawPixel(x, y);
		if(e > 0)
		{
			if(changed)
				x += signx;
			else
				y += signy;
			e -= (dx << 1);
		}
		if(changed)
			y += signy;
		else
			x += signx;
		e += (dy << 1);
	}
}

