#include "math.h"

//utility assembly function for 2^x

double asin(double x)
{
    return 2 * atan(x / (1 + sqrt(1 - x * x)));
}

float asinf(float x)
{
    return 2 * atanf(x / (1 + sqrtf(1 - x * x)));
}

long double asinl(long double x)
{
    return 2 * atanl(x / (1 + sqrtl(1 - x * x)));
}

double acos(double x)
{
    return 2 * atan(sqrt(1 - x * x) / (1 + x));
}

float acosf(float x)
{
    return 2 * atanf(sqrtf(1 - x * x) / (1 + x));
}

long double acosl(long double x)
{
    return 2 * atanl(sqrtl(1 - x * x) / (1 + x));
}

//Hyperbolic trig functions
double sinh(double x)
{
	double eX = exp(x);	//save this to avoid doing exp twice, worse than mult
	return (eX * eX - 1) / 2 * eX;
}

float sinhf(float x)
{
	float eX = expf(x);
	return (eX * eX - 1) / 2 * eX;
}

long double sinhl(long double x)
{
	long double eX = expl(x);
	return (eX * eX - 1) / 2 * eX;
}

double cosh(double x)
{
	double eX = exp(x);
	return (eX * eX + 1) / 2 * eX;
}

float coshf(float x)
{
	float eX = expf(x);
	return (eX * eX + 1) / 2 * eX;
}

long double coshl(long double x)
{
	long double eX = expl(x);
	return (eX * eX + 1) / 2 * eX;
}

double tanh(double x)
{
	double e2X = exp(x);
	e2X *= e2X;
	return (e2X + 1) / (e2X - 1);
}

float tanhf(float x)
{
	float e2X = expf(x);
	e2X *= e2X;
	return (e2X + 1) / (e2X - 1);
}

long double tanhl(long double x)
{
	long double e2X = expl(x);
	e2X *= e2X;
	return (e2X + 1) / (e2X - 1);
}


