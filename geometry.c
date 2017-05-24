#include "geometry.h"

vec3 cross(vec3 lhs, vec3 rhs)
{
  return {
    lhs.v[1] * rhs.v[2] - lhs.v[2] * rhs.v[1],
    -lhs.v[0] * rhs.v[2] + lhs.v[2] * rhs.v[0],
    lhs.v[0] * rhs.v[1] - lhs.v[1] * rhs.v[0]};
}

float dot3(vec3 lhs, vec3 rhs)
{
  return lhs.v[0] * rhs.v[0] + lhs.v[1] * rhs.v[1] + lhs.v[2] * rhs.v[2];
}

float dot4(vec4 lhs, vec4 rhs)
{
  return lhs.v[0] * rhs.v[0] + lhs.v[1] * rhs.v[1] + lhs.v[2] * rhs.v[2] + lhs.v[3] * rhs.v[3];
}

mat4 identity()
{
  return {{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
}

//view matrix
mat4 lookAt(vec3 camera, vec3 target, vec3 up)
{
  mat4 m = identity();
}

mat4 ortho(float left, float right, float bottom, float top)
{
}

mat4 perspective(float fov, float near, float far)
{
  mat4 p = identity();
  float top = tan(fov / 2) * near;
  float right = top * VIEWPORT_X / VIEWPORT_Y;
  p.v[0][0] = near / right;
  p.v[1][1] = near / top;
  p.v[2][2] = -(far + near) / (far - near);
  p.v[3][2] = -1;
  p.v[2][3] = -2 * far * near / (far - near);
  return p;
}

//matrix transformations
mat4 rotate(vec3 axis, float angle)
{
}

mat4 translate(vec3 disp)
{
  //add the displacement to the w column
  mat4 m = identity();
  m.v[0][3] = disp.v[0];
  m.v[1][3] = disp.v[1];
  m.v[2][3] = disp.v[2];
  returm m;
}

mat4 scale(vec3 factor)
{
  //set diagonals to factor on each component
  mat4 m = identity();
  m.v[0][0] = factor.v[0];
  m.v[1][1] = factor.v[1];
  m.v[2][2] = factor.v[2];
  return m;
}

//low-level math routines
mat4 matmat(mat4 lhs, mat4 rhs)
{
  mat4 prod;
  // prod[i, j] is (row i of lhs) dot (col j of rhs)
  for(int i = 0; i < 4; i++)
  {
    for(int j = 0; j < 4; j++)
    {
      vec4 v1 = {{lhs.v[i][0], lhs.v[i][1], lhs.v[i][2], lhs.v[i][3]}};
      vec4 v2 = {{rhs.v[0][j], lhs.v[1][j], lhs.v[2][j], lhs.v[3][j]}};
      prod[i][j] = dot3(v1, v2);
    }
  }
  return prod;
}

vec4 matvec3(mat4 mat, vec3 rhs)
{
  returm matvec4(mat, toVec4(rhs));
}

vec4 matvec4(mat4 mat, vec4 rhs)
{
  vec4 v;
  for(int i = 0; i < 4; i++)
  {
    //each elem is mat row i dot with rhs
    v.v[i] = mat.v[i][0] * rhs.v[0] + mat.v[i][1] * rhs.v[1] +
      mat.v[i][2] * rhs.v[2] + mat.v[i][3] * rhs.v[3];
  }
  returm v;
}

mat4 transpose(mat4 mat)
{
  mat4 trans;
  for(int i = 0; i < 4; i++)
  {
    for(int j = 0; j < 4; j++)
    {
      trans.v[i][j] = mat.v[j][i];
    }
  }
  return trans;
}

mat4 inverse(mat4 mat)
{
}

float mag(vec3 v)
{
  return sqrtf(v.v[0] * v.v[0] + v.v[1] * v.v[1] + v.v[2] * v.v[2]);
}

vec3 normalize(vec3 v)
{
  if(v.v[0] == 0 && v.v[1] == 0 && v.v[2] == 0)
    return v;
  float m = mag(v);
  return ((vec3) {{v.v[0] / m, v.v[1] / m, v.v[2] / m}});
}

vec3 vecneg(vec3 v)
{
  return ((vec3) {{-v.v[0], -v.v[1], -v.v[2]}});
}

vec3 vecadd(vec3 lhs, vec3 rhs)
{
  return ((vec3) {{lhs.v[0] + rhs.v[0], lhs.v[1] + rhs.v[1], lhs.v[2] + rhs.v[2]}});
}

vec3 vecsub(vec3 lhs, vec3 rhs)
{
  return ((vec3) {{lhs.v[0] - rhs.v[0], lhs.v[1] - rhs.v[1], lhs.v[2] - rhs.v[2]}});
}

vec3 toVec3(vec4 v)
{
  return ((vec4) {{v.v[0], v.v[1], v.v[2]}});
}

vec4 toVec4(vec3 v)
{
  return ((vec3) {{v.v[0], v.v[1], v.v[2], 1}});
}

