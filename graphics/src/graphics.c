#include "graphics.h"

static byte* const framebuf = (byte*) 0xA0000;
static byte color;

// (x1, y1) is the top of triangle, and y2 == y3
static int fillFlatBottomTriangle(int x1, int y1, int x2, int y2, int x3, int y3);
// (x3, y3) is the bottom of triangle, and y1 == y2
static void fillFlatTopTriangle(int x1, int y1, int x2, int y2, int x3, int y3);
// rearrange the given values in-place so y1 <= y2 <= y3
static void sortVerticesByY(int* x1, int* y1, int* x2, int* y2, int* x3, int* y3);
// swap vertices in-place (used by sort)
static void swapVertices(int* x1, int* y1, int* x2, int* y2);

void setColor(byte c)
{
  color = c;
}

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

void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3)
{
	drawLine(x1, y1, x2, y2);
	drawLine(x1, y1, x3, y3);
	drawLine(x2, y2, x3, y3);
}

void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3)
{
	int partition;
	if(y1 == y2)
	{
		fillFlatTopTriangle(x1, y1, x2, y2, x3, y3);
		return;
	}
	if(y2 == y3)
	{
		fillFlatBottomTriangle(x1, y1, x2, y2, x3, y3);
		return;
	}
	sortVerticesByY(&x1, &y1, &x2, &y2, &x3, &y3);
	partition = fillFlatBottomTriangle(x1, y1, x2, y2, x3, y3);
	if(x2 < x3)
		fillFlatTopTriangle(partition, y2, x2, y2, x3, y3);
	else
		fillFlatTopTriangle(x2, y2, partition, y2, x3, y3);
}

void drawRect(int x, int y, int w, int h)
{
	int i;
	for(i = x; i <= x + w; i++)
	{
		drawPixel(i, y);
		drawPixel(i, y + h);
	}
	for(i = y + 1; i <= y + h; i++)
	{
		drawPixel(x, i);
		drawPixel(x + w, i);
	}
}

void fillRect(int x, int y, int w, int h)
{
	int i;
	int j;
	for(i = y; i < y + h; i++)
	{
		for(j = x; j < x + w; j++)
		{
			drawPixel(j, i);
		}
	}
}

// point 1 is peak
int fillFlatBottomTriangle(int x1, int y1, int x2, int y2, int x3, int y3)
{
	int changed1, changed2, ix1, iy1, ix2, iy2, dx1, dy1, dx2, dy2, 
	signx1, signy1, signx2, signy2, partition, i, j, e1, e2, leg, temp;
	/* set up bresenham between points 1 and 2 */
	if(x2 > x3)
	{
		swapVertices(&x2, &y2, &x3, &y3);
		leg = 0;
	}
	else
	{
		leg = 1;
	}
	ix1 = x1;
	iy1 = y1;
	dx1 = x2 - x1;
	if(dx1 < 0)
	{
		signx1 = -1;
		dx1 = -dx1;
	}
	else
	{
		signx1 = 1;
	}
	dy1 = y2 - y1;
	if(dy1 < 0)
	{
		signy1 = -1;
		dy1 = -dy1;
	}
	else
	{
		signy1 = 1;
	}
	if(dy1 > dx1)
	{
		changed1 = 1;
		temp = dy1;
		dy1 = dx1;
		dx1 = temp;
	}
	else
	{
		changed1 = 0;
	}
	/* set up bresenham between points 2 and 3 */
	ix2 = x1;
	iy2 = y1;
	dx2 = x3 - x1;
	if(dx2 < 0)
	{
		signx2 = -1;
		dx2 = -dx2;
	}
	else
	{
		signx2 = 1;
	}
	dy2 = y3 - y1;
	if(dy2 < 0)
	{
		signy2 = -1;
		dy2 = -dy2;
	}
	else
	{
		signy2 = 1;
	}
	if(dy2 > dx2)
	{
		changed2 = 1;
		temp = dy2;
		dy2 = dx2;
		dx2 = temp;
	}
	else
	{
		changed2 = 0;
	}
	/* run two instances of bresenham simultaneously */
	e1 = (dy1 << 1) - dx1;
	e2 = (dy2 << 1) - dx2;
	for(i = y1; i < y2 && i < y3; i++)	/* want i to reach min(y2, y3) */
	{
		while(1)					/* step forward by 1 unit in y */
		{
			drawPixel(ix1, iy1);
			if(e1 > 0)
			{
				if(changed1)
				{
					ix1 += signx1;
					e1 -= (dx1 << 1);
				}
				else
				{
					iy1 += signy1;
					e1 -= (dx1 << 1);
					break;
				}
			}
			if(changed1)
			{
				iy1 += signy1;
				e1 += (dy1 << 1);
				break;
			}
			else
			{
				ix1 += signx1;
				e1 += (dy1 << 1);
			}
		}
		while(1)
		{
			drawPixel(ix2, iy2);
			if(e2 > 0)
			{
				if(changed2)
				{
					ix2 += signx2;
					e2 -= (dx2 << 1);
				}
				else
				{
					iy2 += signy2;
					e2 -= (dx2 << 1);
					break;
				}
			}
			if(changed2)
			{
				iy2 += signy2;
				e2 += (dy2 << 1);
				break;
			}
			else
			{
				ix2 += signx2;
				e2 += (dy2 << 1);
			}
		}
		/* now both x and y have increased exactly one unit in y */
		if(ix1 < ix2)
		{
			for(j = ix1; j < ix2; j++)
			{
				drawPixel(j, iy1);
			}
		}
		else
		{
			for(j = ix2; j < ix1; j++)
			{
				drawPixel(j, iy1);
			}
		}
	}
	if(leg == 0)
		return ix1;
	else
		return ix2;
}

//point 3 is bottom
void fillFlatTopTriangle(int x1, int y1, int x2, int y2, int x3, int y3)
{ /* preconditions: y1 == y2 and (x3, y3) is the bottom tip of tri */
	int changed1, changed2, ix1, iy1, ix2, iy2, dx1, dy1, dx2, dy2, 
	signx1, signy1, signx2, signy2, i, j, e1, e2, temp;
	/* set up bresenham between points 1 and 2 */
	if(x1 > x2)
		swapVertices(&x1, &y1, &x2, &y2);
	for(i = x1; i <= x2; i++)
		drawPixel(i, y1);
	ix1 = x1;
	iy1 = y1;
	dx1 = x3 - x1;
	if(dx1 < 0)
	{
		signx1 = -1;
		dx1 = -dx1;
	}
	else
	{
		signx1 = 1;
	}
	dy1 = y3 - y1;
	if(dy1 < 0)
	{
		signy1 = -1;
		dy1 = -dy1;
	}
	else
	{
		signy1 = 1;
	}
	if(dy1 > dx1)
	{
		changed1 = 1;
		temp = dy1;
		dy1 = dx1;
		dx1 = temp;
	}
	else
	{
		changed1 = 0;
	}
	/* set up bresenham between points 2 and 3 */
	ix2 = x2;
	iy2 = y2;
	dx2 = x3 - x2;
	if(dx2 < 0)
	{
		signx2 = -1;
		dx2 = -dx2;
	}
	else
	{
		signx2 = 1;
	}
	dy2 = y3 - y2;
	if(dy2 < 0)
	{
		signy2 = -1;
		dy2 = -dy2;
	}
	else
	{
		signy2 = 1;
	}
	if(dy2 > dx2)
	{
		changed2 = 1;
		temp = dy2;
		dy2 = dx2;
		dx2 = temp;
	}
	else
	{
		changed2 = 0;
	}
	/* run two instances of bresenham simultaneously */
	e1 = (dy1 << 1) - dx1;
	e2 = (dy2 << 1) - dx2;
	for(i = y1; i < y3; i++)
	{
		while(1)					/* step forward by 1 unit in y */
		{
			drawPixel(ix1, iy1);
			if(e1 > 0)
			{
				if(changed1)
				{
					ix1 += signx1;
					e1 -= (dx1 << 1);
				}
				else
				{
					iy1 += signy1;
					e1 -= (dx1 << 1);
					break;
				}
			}
			if(changed1)
			{
				iy1 += signy1;
				e1 += (dy1 << 1);
				break;
			}
			else
			{
				ix1 += signx1;
				e1 += (dy1 << 1);
			}
		}
		while(1)
		{
			drawPixel(ix2, iy2);
			if(e2 > 0)
			{
				if(changed2)
				{
					ix2 += signx2;
					e2 -= (dx2 << 1);
				}
				else
				{
					iy2 += signy2;
					e2 -= (dx2 << 1);
					break;
				}
			}
			if(changed2)
			{
				iy2 += signy2;
				e2 += (dy2 << 1);
				break;
			}
			else
			{
				ix2 += signx2;
				e2 += (dy2 << 1);
			}
		}
		/* now both x and y have increased exactly one unit in y */
		if(ix1 < ix2)
		{
			for(j = ix1; j < ix2; j++)
			{
				drawPixel(j, iy1);
			}
		}
		else
		{
			for(j = ix2; j < ix1; j++)
			{
				drawPixel(j, iy1);
			}
		}
	}
}

void sortVerticesByY(int* x1, int* y1, int* x2, int* y2, int* x3, int* y3)
{
	if(*y1 > *y2)
		swapVertices(x1, y1, x2, y2);
	if(*y2 > *y3)
		swapVertices(x2, y2, x3, y3);
	if(*y1 > *y2)
		swapVertices(x1, y1, x2, y2);
	if(*y2 > *y3)
		swapVertices(x2, y2, x3, y3);
}

void swapVertices(int* x1, int* y1, int* x2, int* y2)
{
	int temp;
	temp = *x1;
	*x1 = *x2;
	*x2 = temp;
	temp = *y1;
	*y1 = *y2;
	*y2 = temp;
}
