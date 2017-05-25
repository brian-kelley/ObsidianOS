#include "geometry.h"

vec3 cross(vec3 lhs, vec3 rhs)
{
  return ((vec3) {
    lhs.v[1] * rhs.v[2] - lhs.v[2] * rhs.v[1],
    -lhs.v[0] * rhs.v[2] + lhs.v[2] * rhs.v[0],
    lhs.v[0] * rhs.v[1] - lhs.v[1] * rhs.v[0]});
}

float dot(vec3 lhs, vec3 rhs)
{
  return lhs.v[0] * rhs.v[0] + lhs.v[1] * rhs.v[1] + lhs.v[2] * rhs.v[2];
}

mat4 identity()
{
  return ((mat4) {{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}});
}

//view matrix
mat4 lookAt(vec3 camera, vec3 target, vec3 up)
{
  mat4 m = identity();
  //view matrix first rotates world so that 
  up = normalize(up);
  vec3 camDir = normalize(vecsub(camera, target));
  vec3 s = cross(camDir, up);
  vec3 u = cross(s, camDir);
  m.v[0][0] = s.v[0];
  m.v[0][1] = s.v[1];
  m.v[0][2] = s.v[2];
  m.v[1][0] = u.v[0];
  m.v[1][1] = u.v[1];
  m.v[1][2] = u.v[2];
  m.v[2][0] = -camDir.v[0];
  m.v[2][1] = -camDir.v[1];
  m.v[2][2] = -camDir.v[2];
  //now translate from eye point to origin
  vec3 trans = vecneg(camera);
  return matmat(m, translate(trans));
}

mat4 ortho(float left, float right, float bottom, float top, float near, float far)
{
  mat4 o = identity();
  o.v[0][0] = 2 / (right - left);
  o.v[1][1] = 2 / (top - bottom);
  o.v[2][2] = -2 / (far - near);
  o.v[3][3] = 1;
  o.v[0][3] = -(right + left) / (right - left);
  o.v[1][3] = -(top + bottom) / (top - bottom);
  o.v[2][3] = -(far + near) / (far - near);
  return o;
}

mat4 perspective(float fov, float near, float far)
{
  //this assumes that top = -bottom and left = -right
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
  mat4 m = identity();
  //http://inside.mines.edu/fs_home/gmurray/ArbitraryAxisRotation/
  float sint = sin(angle);
  float cost = cos(angle);
  float u = axis.v[0];
  float v = axis.v[1];
  float w = axis.v[2];
  axis = normalize(axis);
  m.v[0][0] = u * u + (v * v + w * w) * cost;
  m.v[0][1] = u * v * (1 - cost) - w * sint;
  m.v[0][2] = u * w * (1 - cost) + v * sint;
  m.v[1][0] = u * v * (1 - cost) + w * sint;
  m.v[1][1] = v * v + (u * u + w * w) * cost;
  m.v[1][2] = v * w * (1 - cost) - u * sint;
  m.v[2][0] = u * w * (1 - cost) - v * sint;
  m.v[2][1] = v * w * (1 - cost) + u * sint;
  m.v[2][2] = w * w + (u * u + v * v) * cost;
  return m;
}

mat4 translate(vec3 disp)
{
  //add the displacement to the w column
  mat4 m = identity();
  m.v[0][3] = disp.v[0];
  m.v[1][3] = disp.v[1];
  m.v[2][3] = disp.v[2];
  return m;
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
      prod.v[i][j] = v1.v[0] * v2.v[0] + v1.v[1] * v2.v[1] + v1.v[2] * v2.v[2] + v1.v[3] * v2.v[3];
    }
  }
  return prod;
}

vec4 matvec3(mat4 mat, vec3 rhs)
{
  return matvec4(mat, toVec4(rhs));
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
  return v;
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
  return ((vec3) {{v.v[0], v.v[1], v.v[2]}});
}

vec4 toVec4(vec3 v)
{
  return ((vec4) {{v.v[0], v.v[1], v.v[2], 1}});
}

