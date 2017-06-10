#include "graphics.h"

static byte* const framebuf = (byte*) 0xA0000;
byte* renderBuf;
#ifdef DOUBLE_BUFFERED
byte* depthBuf;
#endif
static byte depthVal;
static byte color;
static bool depthTest;
static int drawMode;
mat4 modelMat;
mat4 viewMat;
mat4 projMat;

static bool glDebugOn = false;

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
  int minx = min(min(x1, x2), x3);
  int miny = min(min(y1, y2), y3);
  int maxx = max(max(x1, x2), x3);
  int maxy = max(max(y1, y2), y3);
  //triangle too big to rasterize
  if(maxx - minx > 320 || maxy - miny > 200)
    return;
  int partition;
  /*
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
  */
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
  for(int i = y; i < y + h; i++)
  {
    for(int j = x; j < x + w; j++)
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

//////////////////
// 3D functions //
//////////////////

mat4 fullMat;
static Plane frustum[5];
static float projFar;

static void updateMatrices()
{
  fullMat = matmat(projMat, matmat(viewMat, modelMat));
}

void setModel(mat4 m)
{
  modelMat = m;
  updateMatrices();
}

void setView(mat4 v)
{
  viewMat = v;
  updateMatrices();
}

void setProj(float fovyDeg, float near, float far)
{
  projFar = far;
  projMat = perspective(fovyDeg / (180.0f / PI), near, far);
  updateMatrices();
  getFrustumPlanes(frustum, ((fovyDeg / 2) / (180.0f / PI)), near);
}

vec3 vshade(vec3 vertex)
{
  vec4 result = matvec3(fullMat, vertex);
  //divide by w
  vec3 clip = {{result.v[0] / result.v[3], result.v[1] / result.v[3], result.v[2] / result.v[3]}};
  return clip;
}

point viewport(vec3 clip)
{
  point p;
  p.x = (clip.v[0] + 1) * (VIEWPORT_X / 2);
  if(p.x < 0)
    p.x = 0;
  if(p.x >= 320)
    p.x = 319;
  p.y = (-clip.v[1] + 1) * (VIEWPORT_Y / 2);
  if(p.y < 0)
    p.y = 0;
  if(p.y >= 200)
    p.y = 199;
  return p;
}

//Simple OpenGL implementation

static int geomType = NO_GEOM;
static vec3 vertState[4];
static int numVerts = 0;
static bool enabled3D = true;
static const int vertsPerElement[] = {0, 2, 3, 4};

void glBegin(int type)
{
  geomType = type;
  numVerts = 0;
}

void glEnd()
{
  geomType = NO_GEOM;
}

void glVertex2i(int x, int y)
{
  glVertex3f(x, y, 0);
}

void glVertex3f(float x, float y, float z)
{
  vec3 v = {x, y, z};
  glVertex3fv(v);
}

//all triangles with > 0 vertices past far plane are completely culled
//other triangles are clipped in this order:
//z = -1 (near)
//x = -1
//x = 1
//y = -1
//y = 1

//Each clip-draw function clips one triangle against one plane

vec3 intersect(vec3 pt1, vec3 pt2, int dim, float val)
{
  if(fabsf(pt1.v[dim] - pt2.v[dim]) < 1e-6)
  {
    //can't do intersection formula, so just return the midpt
    //return vecscale(vecadd(pt1, pt2), 0.5);
    return pt1;
  }
  else
  {
    return vecadd(pt1, vecscale(vecsub(pt2, pt1), (val - pt1.v[dim]) / (pt2.v[dim] - pt1.v[dim])));
  }
}

#define VEC_SWAP(pt1, pt2) \
{ \
  vec3 temp = pt1; \
  pt1 = pt2; \
  pt2 = temp; \
}

//Fill a triangle, and clip against frustum
//v1, v2, v3 in view space
//Pass clipPlane = 0 to start (is recursive)
//Pass clipPlane = 5 to do perspective trans and rasterize
//Precondition: already culled triangles beyond the far plane
static void drawClippedTri(vec3 v1, vec3 v2, vec3 v3, int clipPlane)
{
  if(clipPlane == 0)
  {
    if(v1.v[2] < -projFar || v2.v[2] < -projFar || v3.v[3] < -projFar)
      return;
  }
  if(clipPlane == 5)
  {
    vec4 proj1 = matvec3(projMat, v1);
    vec4 proj2 = matvec3(projMat, v2);
    vec4 proj3 = matvec3(projMat, v3);
    point vp1 = viewport(vecscale(toVec3(proj1), 1.0f / proj1.v[3]));
    point vp2 = viewport(vecscale(toVec3(proj2), 1.0f / proj2.v[3]));
    point vp3 = viewport(vecscale(toVec3(proj3), 1.0f / proj3.v[3]));
    fillTriangle(vp1.x, vp1.y, vp2.x, vp2.y, vp3.x, vp3.y);
    return;
  }
  Plane toClip = frustum[clipPlane];
  //test vertices against *toClip
  //note: distance > 0 means "in front" or in same direction as plane normal,
  //which means the point is on the visible side of the plane
  int behind = 0;
  float dist[3];
  dist[0] = planeLineDistance(v1, toClip);
  dist[1] = planeLineDistance(v2, toClip);
  dist[2] = planeLineDistance(v3, toClip);
  //sort vertices by dist, ascending
  if(dist[0] > dist[1])
  {
    VEC_SWAP(v1, v2);
    float temp = dist[0];
    dist[0] = dist[1];
    dist[1] = temp;
  }
  if(dist[1] > dist[2])
  {
    VEC_SWAP(v2, v3);
    float temp = dist[1];
    dist[1] = dist[2];
    dist[2] = temp;
  }
  if(dist[0] > dist[1])
  {
    VEC_SWAP(v1, v2);
    float temp = dist[0];
    dist[0] = dist[1];
    dist[1] = temp;
  }
  int beyond = 0;
  for(int i = 0; i < 3; i++)
  {
    if(dist[i] < 0)
      beyond++;
  }
  switch(beyond)
  {
    case 0:
      //triangle does not intersect plane at all
      drawClippedTri(v1, v2, v3, clipPlane + 1);
      break;
    case 1:
      {
        //one vertex invisible
        //compute two intersection points and draw 2 triangles
        //get v1 as the invisible vertex
        vec3 inter1 = linePlaneIntersect(v1, v2, toClip);
        vec3 inter2 = linePlaneIntersect(v1, v3, toClip);
        drawClippedTri(inter1, v2, v3, clipPlane + 1);
        drawClippedTri(inter1, v3, inter2, clipPlane + 1);
        break;
      }
    case 2:
      {
        //2 vertices invisible
        //compute two intersection points and draw one triangle
        //get v1 as the only visible vertex
        vec3 inter1 = linePlaneIntersect(v1, v3, toClip);
        vec3 inter2 = linePlaneIntersect(v2, v3, toClip);
        drawClippedTri(v3, inter1, inter2, clipPlane + 1);
        break;
      }
    case 3:
    default:;
  }
}

void glVertex3fv(vec3 v)
{
  vertState[numVerts++] = v;
  int verts = vertsPerElement[geomType];
  if(numVerts == verts)
  {
    //run vshader and draw the geometry, then flush vert buffer
    //vertices in screen space
    point screen[4];
    vec3 clip[4];
    if(!enabled3D)
    {
      for(int i = 0; i < verts; i++)
      {
        screen[i] = ((point) {vertState[i].v[0], vertState[i].v[1]});
      }
    }
    else
    {
      for(int i = 0; i < verts; i++)
      {
        clip[i] = vshade(vertState[i]);
      }
    }
    if(geomType == GL_LINES)
    {
      screen[0] = viewport(clip[0]);
      screen[1] = viewport(clip[1]);
      drawLine(screen[0].x, screen[0].y, screen[1].x, screen[1].y);
    }
    else if(geomType == GL_TRIANGLES)
    {
      if(drawMode == DRAW_FILL)
      {
        //do modelview transform only on vertstate[0:3]
        //for now, ignore model
        drawClippedTri(
            toVec3(matvec3(viewMat, vertState[0])),
            toVec3(matvec3(viewMat, vertState[1])),
            toVec3(matvec3(viewMat, vertState[2])), 0);
      }
      else
      {
        screen[0] = viewport(clip[0]);
        screen[1] = viewport(clip[1]);
        screen[2] = viewport(clip[2]);
        drawTriangle(screen[0].x, screen[0].y, screen[1].x, screen[1].y, screen[2].x, screen[2].y);
      }
    }
    else if(geomType == GL_QUADS)
    {
      if(drawMode == DRAW_FILL)
      {
        drawClippedTri(
            toVec3(matvec3(viewMat, vertState[0])),
            toVec3(matvec3(viewMat, vertState[1])),
            toVec3(matvec3(viewMat, vertState[2])), 0);
        drawClippedTri(
            toVec3(matvec3(viewMat, vertState[0])),
            toVec3(matvec3(viewMat, vertState[2])),
            toVec3(matvec3(viewMat, vertState[3])), 0);
      }
      else
      {
        screen[0] = viewport(clip[0]);
        screen[1] = viewport(clip[1]);
        screen[2] = viewport(clip[2]);
        screen[3] = viewport(clip[3]);
        drawLine(screen[0].x, screen[0].y, screen[1].x, screen[1].y);
        drawLine(screen[1].x, screen[1].y, screen[2].x, screen[2].y);
        drawLine(screen[2].x, screen[2].y, screen[3].x, screen[3].y);
        drawLine(screen[0].x, screen[0].y, screen[3].x, screen[3].y);
      }
    }
    numVerts = 0;
  }
}

void glColor1i(byte c)
{
  color = c;
}

void enable2D()
{
  enabled3D = false;
}

void enable3D()
{
  enabled3D = true;
}

void glClear(byte c)
{
  memset(renderBuf, c, 64000);
}

void glDepth(int d)
{
  if(d > 254)
    d = 254;
  if(d < 0)
    d = 0;
  depthVal = d;
}

void glEnableDepthTest(bool enable)
{
  depthTest = enable;
}

void glDrawMode(int mode)
{
  drawMode = mode;
}

void glFlush()
{
#ifdef DOUBLE_BUFFERED
  memcpy(framebuf, renderBuf, 64000);
#endif
}

void glText(const char* text, int x, int y, byte bg)
{
  byte* iter = renderBuf + x + y * 320;
  for(const char* c = text; *c; c++)
  {
    const int stride = 312;
    if(*c < '!' || *c > '~')
    {
      //non-visible character, clear that region
      for(int i = 0; i < 8; i++)
      {
        for(int j = 0; j < 8; j++)
        {
          *iter = bg;
          iter++;
        }
        iter += stride;
      }
    }
    else
    {
      byte* glyph = fontbin + 8 * (*c - '!');
      //stride is from right edge of character to left edge of char on next line
      for(int row = 0; row < 8; row++)
      {
        for(byte mask = 0x1; mask; mask <<= 1)
        {
          if(glyph[row] & mask)
          {
            *iter = color;
          }
          else
          {
            *iter = bg;
          }
          iter++;
        }
        iter += stride;
      }
    }
    iter = iter - 8 * 320 + 8;
  }
}

void glDebug()
{
  glDebugOn = true;
}

