#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "globalDefines.h"
#include "math.h"
#include "stdio.h"

//All angles in radians
//Aspect ratio is always VIEWPORT_X / VIEWPORT_Y

#define VIEWPORT_X 320
#define VIEWPORT_Y 200

typedef struct 
{
  float v[4][4];
} mat4;

typedef struct
{
  float v[4];
} vec4;

typedef struct
{
  float v[3];
} vec3;

typedef struct
{
  float v[2];
} vec2;

//Plane: ax + by + cz = d
//(a, b, c) should be the normalized normal vector
//d is the displacement from origin
typedef struct
{
  float a;
  float b;
  float c;
  float d;
} Plane;

typedef struct
{
  int x;
  int y;
} point;

vec3 cross(vec3 lhs, vec3 rhs);
float dot(vec3 lhs, vec3 rhs);
mat4 identity();
//view matrix
mat4 lookAt(vec3 camera, vec3 target, vec3 up);
//projection matrix
mat4 ortho(float left, float right, float bottom, float top, float near, float far);
mat4 perspective(float fov, float near, float far);

//matrix transformations
mat4 rotate(vec3 axis, float angle);
mat4 translate(vec3 disp);
mat4 scale(vec3 factor);

//low-level math routines
mat4 matmat(mat4 lhs, mat4 rhs);
vec4 matvec3(mat4 mat, vec3 rhs);
vec4 matvec4(mat4 mat, vec4 rhs);
mat4 transpose(mat4 mat);
//TODO: is this really needed?
//mat4 inverse(mat4 mat);

float mag(vec3 v);
vec3 normalize(vec3 v);
vec3 vecneg(vec3 v);
vec3 vecadd(vec3 lhs, vec3 rhs);
vec3 vecsub(vec3 lhs, vec3 rhs);
vec3 vecscale(vec3 v, float scale);
vec3 toVec3(vec4 v);
vec4 toVec4(vec3 v);

//get the minimum distance from p to v
//if rv > 0, v is "in front of" plane
//if rv = 0, v is on plane
//if rv < 0, v is "behind" plane
float planeLineDistance(vec3 v, Plane p);
//precondition: p1, p2 on different sides of plane and neither are on the plane
vec3 linePlaneIntersect(vec3 p1, vec3 p2, Plane plane);
//load the 5 planes constituting the frustum (in view space) - omit far
//frustum is populated with:
//near (-z), left (-x), right (+x), top (+y), bottom (-y)
void getFrustumPlanes(Plane* frustum, float fovyRad, float near);

#endif

