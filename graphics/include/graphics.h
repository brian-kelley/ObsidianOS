#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "globalDefines.h"
#include "stdio.h"
#include "geometry.h"

#define drawPixel(x, y) if(x >= 0 && x < 320 && y >= 0 && y < 320) framebuf[x + y * 320] = color;

void setColor(byte c);
void drawLine(int x1, int y1, int x2, int y2);
void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3);
void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3);
void drawRect(int x, int y, int w, int h);
void fillRect(int x, int y, int w, int h);

//3d functions
void setModel(mat4 m);
void setView(mat4 v);
void setProj(mat4 p);

//do model, view, projection transformations
vec3 vshade(vec3 vertex);
//convert clip space to screen space (integer pixels)
point viewport(vec3 clip);

enum GeometryTypes
{
  NO_GEOM,
  GL_LINES,
  GL_TRIANGLES,
  GL_QUADS
};

//OpenGL immediate mode style rendering
void glBegin(int type);
void glEnd();
void glVertex2i(int x, int y);
void glVertex3f(float x, float y, float z);
void glVertex3fv(vec3 v);
void glColor1i(byte c);
//Assume all vertices are already in 2D screen space
void enable2D();
//Do model-view-projection transformations on all vertices
void enable3D();

#endif

