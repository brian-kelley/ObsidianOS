#include "geometry.h"

vec3 cross(vec3 lhs, vec3 rhs)
{
  vec3 c = {
    lhs.v[1] * rhs.v[2] - lhs.v[2] * rhs.v[1],
    -lhs.v[0] * rhs.v[2] + lhs.v[2] * rhs.v[0],
    lhs.v[0] * rhs.v[1] - lhs.v[1] * rhs.v[0]
  };
  return c;
}

float dot(vec3 lhs, vec3 rhs)
{
  return lhs.v[0] * rhs.v[0] + lhs.v[1] * rhs.v[1] + lhs.v[2] * rhs.v[2];
}

mat4 identity()
{
  mat4 i = {{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
  return i;
}

//view matrix
mat4 lookAt(vec3 camera, vec3 target, vec3 up)
{
  mat4 m = identity();
  //view matrix first rotates world so that 
  vec3 D = normalize(vecsub(camera, target));
  vec3 R = normalize(cross(up, D));
  vec3 U = cross(D, R);
  m.v[0][0] = R.v[0];
  m.v[0][1] = R.v[1];
  m.v[0][2] = R.v[2];
  m.v[1][0] = U.v[0];
  m.v[1][1] = U.v[1];
  m.v[1][2] = U.v[2];
  m.v[2][0] = D.v[0];
  m.v[2][1] = D.v[1];
  m.v[2][2] = D.v[2];
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
      vec4 v2 = {{rhs.v[0][j], rhs.v[1][j], rhs.v[2][j], rhs.v[3][j]}};
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
  vec3 n = {{v.v[0] / m, v.v[1] / m, v.v[2] / m}};
  return n;
}

vec3 vecneg(vec3 v)
{
  vec3 nv = {{-v.v[0], -v.v[1], -v.v[2]}};
  return nv;
}

vec3 vecadd(vec3 lhs, vec3 rhs)
{
  vec3 sum = {{lhs.v[0] + rhs.v[0], lhs.v[1] + rhs.v[1], lhs.v[2] + rhs.v[2]}};
  return sum;
}

vec3 vecsub(vec3 lhs, vec3 rhs)
{
  vec3 diff = {{lhs.v[0] - rhs.v[0], lhs.v[1] - rhs.v[1], lhs.v[2] - rhs.v[2]}};
  return diff;
}

vec3 vecscale(vec3 v, float scale)
{
  v.v[0] *= scale;
  v.v[1] *= scale;
  v.v[2] *= scale;
  return v;
}

vec3 toVec3(vec4 v)
{
  vec3 v3 = {{v.v[0], v.v[1], v.v[2]}};
  return v3;
}

vec4 toVec4(vec3 v)
{
  vec4 v4 = {{v.v[0], v.v[1], v.v[2], 1}};
  return v4;
}

