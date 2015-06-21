#ifndef __STD_MATH_H__
#define __STD_MATH_H__

//Constants (as doubles, but written with more than double precision)
#define PI 3.1415926535897932384626
#define E  2.7182818284523536028747

static const unsigned int __nanVal__ = 0x7FC00000;
static const unsigned long long int __hugeNumberVal__ = 0x7FFFFFFFFFFFFFFF;

const float NAN = *((float*) &__nanVal__);
const double HUGE_VAL = *((double*) &__hugeNumberVal__)

//Trig functions
double sin(double x);
float sinf(float x);
long double sinl(long double x);
double cos(double x);
float cosf(float x);
long double cosl(long double x);
double tan(double x);
float tanf(float x);
long double tanl(long double x);
double asin(double x);
float asinf(float x);
long double asinl(long double x);
double acos(double x);
float acosf(float x);
long double acosl(long double x);
double atan(double x);
float atanf(float x);
long double atanl(long double x);
double atan2(double y, double x);	//y is the vertical part, x is horizontal
float atan2f(float y, float x);
long double atan2l(long double y, long double x);

//Hyperbolic functions
double sinh(double x);
float sinhf(float x);
long double sinhl(long double x);
double cosh(double x);
float coshf(float x);
long double coshl(long double x);
double tanh(double x);
float tanhf(float x);
long double tanhl(long double x);

//Exponential and logarithmic functions
double exp(double x);						//e^x
float expf(float x);
long double expl(long double x);
double frexp(double x, int* exp);			//breaks x into significand and
float frexpf(float x, int* exp);			//exponent, significand in [.5, 1)
long double frexpl(long double x, int* exp);
double ldexp(double x, int exp);			//build float out of significand
float ldexpf(float x, int exp);				//and exponent
long double ldexpl(long double x, int exp);
double log(double x);						//ln(x)
float logf(float x);
long double logl(long double x);
double log10(double x);						//log(x), base 10
float log10f(float x);
long double log10l(long double x);
double modf(double x, double* intpart);		//returns fractional part, integer
float modff(float x, float* intpart);		//part in intpart. Both parts w/
long double modfl(long double x, long double* intpart);	//same sign as x.

//Power functions
double pow(double base, double exponent);	//base^exponent
float powf(float base, float exponent);
long double powl(long double base, long double exponent);
double sqrt(double x);
float sqrtf(float x);
long double sqrtl(long double x);

//Rounding and remainder functions
double ceil(double x);
float ceilf(float x);
long double ceill(long double x);
double floor(double x);
float floorf(float x);
long double floorl(long double x);
double fmod(double numer, double denom);
float fmodf(float numer, float denom);
long double fmodl(long double numer, long double denom);

//Absolute value functions
double fabs(double x);
float fabsf(float x);
long double fabsl(long double x);

#endif