#include "math.h"

//utility assembly function for 2^x

double asin(double x)
{
    return 2 * atanl(x / (1 + sqrtl(1 - x * x)));
}

float asinf(float x)
{
    return 2 * atanl(x / (1 + sqrtl(1 - x * x)));
}

long double asinl(long double x)
{
    return 2 * atanl(x / (1 + sqrtl(1 - x * x)));
}

double acos(double x)
{
    return 2 * atanl(sqrtl(1 - x * x) / (1 + x));
}

float acosf(float x)
{
    return 2 * atanl(sqrtl(1 - x * x) / (1 + x));
}

long double acosl(long double x)
{
    return 2 * atanl(sqrtl(1 - x * x) / (1 + x));
}

//Hyperbolic trig functions
double sinh(double x)
{
	long double eX = expl(x);	//save this to avoid doing exp twice, worse than mult
	return (eX - 1 / eX) / 2;
}

float sinhf(float x)
{
	long double eX = expl(x);
	return (eX - 1 / eX) / 2;	
}

long double sinhl(long double x)
{
	long double eX = expl(x);
	return (eX - 1 / eX) / 2;
}

double cosh(double x)
{
	double eX = exp(x);
	return (eX + 1 / eX) / 2;
}

float coshf(float x)
{
	float eX = expf(x);
	return (eX + 1 / eX) / 2;
}

long double coshl(long double x)
{
	long double eX = expl(x);
	return (eX + 1 / eX) / 2;
}

double tanh(double x)
{
	long double e2X = expl(2 * x);
	return (e2X - 1) / (e2X + 1);
}

float tanhf(float x)
{
	long double e2X = expl(2 * x);
	return (e2X - 1) / (e2X + 1);
}

long double tanhl(long double x)
{
	long double e2X = expl(2 * x);
	return (e2X - 1) / (e2X + 1);
}

//Need to provide custom implementations of libgcc 64-bit math
unsigned long long __umoddi3(unsigned long long dividend, unsigned long long divisor)
{
  unsigned long long q = 0;
  unsigned long long r = 0;
  for(int i = 63; i >= 0; i--)
  {
    r <<= 1;
    unsigned long long mask = 1ULL << i;
    if(dividend & mask)
    {
      r |= 1;
    }
    if(r >= divisor)
    {
      r -= divisor;
      q |= mask;
    }
  }
  return r;
}

unsigned long long __udivdi3(unsigned long long dividend, unsigned long long divisor)
{
  unsigned long long q = 0;
  unsigned long long r = 0;
  for(int i = 63; i >= 0; i--)
  {
    r <<= 1;
    unsigned long long mask = 1ULL << i;
    if(dividend & mask)
    {
      r |= 1;
    }
    if(r >= divisor)
    {
      r -= divisor;
      q |= mask;
    }
  }
  return q;
}

long long __moddi3(long long dividend, long long divisor)
{
  //C behavior: mod is negative if dividend OR divisor is negative
  bool sign = (dividend < 0) || (divisor < 0);
  unsigned long long rem = __umoddi3(dividend < 0 ? -dividend : dividend, divisor < 0 ? -divisor : divisor);
  if(sign)
  {
    return -rem;
  }
  else
  {
    return rem;
  }
}

long long __divdi3(long long dividend, long long divisor)
{
  bool sign = (dividend < 0) ^ (divisor < 0);
  unsigned long long q = __udivdi3(dividend < 0 ? -dividend : dividend, divisor < 0 ? -divisor : divisor);
  if(sign)
  {
    return -q;
  }
  else
  {
    return q;
  }
}

