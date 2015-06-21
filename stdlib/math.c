#define FLOAT_CONV_TOL 1e-7
#define DOUBLE_CONV_TOL 1e-10
#define LONG_DOUBLE_CONV_TOL 1e-16

//locally defined factorial function, needed for Taylor series evaluation
int factorial(int num)
{
	if(num & 1 == num)
		return 1;
	int rv = 1;
	for(int mult = num; mult > 1; mult--)
		rv *= mult;
	return rv;
}

//Math utility functions (not in header)
//(simpler, faster version of pow for nonnegative integer powers)
double intpow(double base, int exponent)
{
	if(exponent == 0)
		return base;
	double rv = 1;
	for(int i = 0; i < exponent; i++)
		rv *= base;
	return rv;
}

float intpowf(float base, int exponent)
{
	if(exponent == 0)
		return base;
	float rv = 1;
	for(int i = 0; i < exponent; i++)
		rv *= base;
	return rv;
}

long double intpowl(long double base, int exponent)
{
	if(exponent == 0)
		return base;
	long double rv = 1;
	for(int i = 0; i < exponent; i++)
		rv *= base;
	return rv;
}

//Trig functions

double sin(double x)
{
	if(x == NAN)
		return NAN;
	//get domain into (-2pi, 2pi) to improve accuracy
	x -= PI * ((int) (x / PI));
	double rv = 0;
	for(int i = 0;; i++)
	{
		//(i * 4 + 1) gives sequence 1, 5, 9, ... (added terms)
		//(i * 4 + 3) gives sequence 3, 7, 11 ... (subtracted terms)
		double term1 = intpow(x, i * 4 + 1) / factorial(i * 4 + 1);
		double term2 = intpow(x, i * 4 + 3) / factorial(i * 4 + 3);
		//check for convergence if have already done ~6 terms
		double termDiff = term1 - term2;
		if(-DOUBLE_CONV_TOL < termDiff && termDiff < DOUBLE_CONV_TOL)
			break;
		rv += term1;
		rv -= term2;
	}
	return rv;
}

float sinf(float x)
{
	if(x == NAN)
		return NAN;
	x -= (float) PI * ((int) (x / (float)PI));
	float rv = 0;
	for(int i = 0;; i++)
	{
		float term1 = intpowf(x, i * 4 + 1) / factorial(i * 4 + 1);
		float term2 = intpowf(x, i * 4 + 3) / factorial(i * 3 + 1);
		float termDiff = term1 - term2;
		if(-FLOAT_CONV_TOL < termDiff && termDiff < FLOAT_CONV_TOL)
			break;
		rv += term1;
		rv -= term2;
	}
	return rv;
}

long double sinl(long double x)
{
	if(x == NAN)
		return NAN;
	x -= (long double) PI * ((int) (x / (long double) PI));
	float rv = 0;
	for(int i = 0;; i++)
	{
		long double term1 = intpowl(x, i * 4 + 1) / factorial(i * 4 + 1);
		long double term2 = intpowl(x, i * 4 + 3) / factorial(i * 4 + 3);
		long double termDiff = term1 - term2;
		if(-LONG_DOUBLE_CONV_TOL < termDiff && termDiff < LONG_DOUBLE_CONV_TOL)
			break;
		rv += term1;
		rv -= term2;
	}
	return rv;
}

double cos(double x)
{
	if(x == NAN)
		return NAN;
	x -= PI * ((int) (x / PI));
	double rv = 1;
	for(int i = 0;; i++)
	{
		double term1 = intpow(x, i * 4 + 2) / factorial(i * 4 + 2);
		double term2 = intpow(x, i * 4 + 4) / factorial(i * 4 + 4);
		double termDiff = term1 - term2;
		if(-DOUBLE_CONV_TOL < termDiff && termDiff < DOUBLE_CONV_TOL)
			break;
		rv -= term1;
		rv += term2;
	}
	return rv;
}

float cosf(float x)
{
	if(x == NAN)
		return NAN;
	x -= (float) PI * ((int) (x / PI));
	float rv = 1;
	for(int i = 0;; i++)
	{
		float term1 = intpowf(x, i * 4 + 2) / factorial(i * 4 + 2);
		float term2 = intpowf(x, i * 4 + 4) / factorial(i * 4 + 4);
		float termDiff = term1 - term2;
		if(-FLOAT_CONV_TOL < termDiff && termDiff < FLOAT_CONV_TOL)
			break;
		rv -= term1;
		rv += term2;
	}
	return rv;
}

long double cosl(long double x)
{
	if(x == NAN)
		return NAN;
	x -= (long double) PI * ((int) (x / (long double) PI));
	long double rv = 1;
	for(int i = 0;; i++)
	{
		long double term1 = intpowl(x, i * 4 + 2) / factorial(i * 4 + 2);
		long double term2 = intpowl(x, i * 4 + 4) / factorial(i * 4 + 4);
		long double termDiff = term1 - term2;
		if(-LONG_DOUBLE_CONV_TOL < termDiff && termDiff < LONG_DOUBLE_CONV_TOL)
			break;
		rv -= term1;
		rv += term2;
	}
	return rv;
}

double tan(double x)
{
	if(x == NAN)
		return NAN;
	double cosine = cos(x);
	if(-DOUBLE_CONV_TOL < cosine && cosine < DOUBLE_CONV_TOL)
		return NAN;
	return sin(x) / cosine;
}

float tanf(float x)
{
	if(x == NAN)
		return NAN;
	float cosine = cosf(x);
	if(-FLOAT_CONV_TOL < cosine && cosine < DOUBLUE_CONV_TOL)
		return NAN;
	return sinf(x) / cosine;
}

long double tanl(long double x)
{
	if(x == NAN)
		return NAN;
	long double cosine = cosl(x);
	if(-LONG_DOUBLE_CONV_TOL < cosine && cosine < LONG_DOUBLE_CONV_TOL)
		return NAN;
	return sinl(x) / cosine;
}

double asin(double x)
{
	//Check for valid input
	if(x > 1.0 || x < -1.0 || x == NAN)
		return NAN;
	double rv = x;
	for(int i = 0;; i++)
	{
		//Numerator and denominator for the (2n choose n) factor, terms 1 & 2
		long long int chooseTermNumer1 = 1;
		long long int chooseTermDenom1 = 1;
		for(int j = 0; j < (2 * i + 1); j++)	//# of times thru loop 1, 3, 5
		{
			chooseTermNumer1 *= (j * 2 + 1);
			chooseTermDenom1 *= (j * 2 + 2);
		}
		//Calculate these based on term 1 numer/denom, since numer is just
		//multiplied by next odd and denom is multiplied by next even
		long long int chooseTermNumer2 = chooseTermNumer1 * ((i * 2 + 2) * 2 + 1);
		long long int chooseTermDenom2 = chooseTermDenom1 * ((i * 2 + 2) * 2 + 2);
		double chooseTerm1 = (long double) chooseTermNumer1 / chooseTermDenom1;
		double chooseTerm2 = (long double) chooseTermNumer2 / chooseTermDenom2;
		double term1 = chooseTerm1 * intpow(x, i * 4 + 3) / (i * 4 + 3);
		double term2 = chooseTerm2 * intpow(x, i * 4 + 5) / (i * 4 + 5);
		//See if converged
		double termDiff = term1 - term2;
		if(-DOUBLE_CONV_TOL < termDiff && termDiff < DOUBLE_CONV_TOL)
			break;
		rv += term1;
		rv += term2;
	}
	return rv;
}

float asinf(float x)
{
	if(x > 1.0 || x < -1.0 || x == NAN)
		return NAN;
	double rv = x;
	for(int i = 0;; i++)
	{
		//Numerator and denominator for the (2n choose n) factor, terms 1 & 2
		long long int chooseTermNumer1 = 1;
		long long int chooseTermDenom1 = 1;
		for(int j = 0; j < (2 * i + 1); j++)	//# of times thru loop 1, 3, 5
		{
			chooseTermNumer1 *= (j * 2 + 1);
			chooseTermDenom1 *= (j * 2 + 2);
		}
		//Calculate these based on term 1 numer/denom, since numer is just
		//multiplied by next odd and denom is multiplied by next even
		long long int chooseTermNumer2 = chooseTermNumer1 * ((i * 2 + 2) * 2 + 1);
		long long int chooseTermDenom2 = chooseTermDenom1 * ((i * 2 + 2) * 2 + 2);
		float chooseTerm1 = (long double) chooseTermNumer1 / chooseTermDenom1;
		float chooseTerm2 = (long double) chooseTermNumer2 / chooseTermDenom2;
		float term1 = chooseTerm1 * intpow(x, i * 4 + 3) / (i * 4 + 3);
		float term2 = chooseTerm2 * intpow(x, i * 4 + 5) / (i * 4 + 5);
		//See if converged
		float termDiff = term1 - term2;
		if(-FLOAT_CONV_TOL < termDiff && termDiff < FLOAT_CONV_TOL)
			break;
		rv += term1;
		rv += term2;
	}
	return rv;
}

long double asinl(long double x)
{
	if(x > 1.0 || x < -1.0 || x == NAN)
		return NAN;
	double rv = x;
	for(int i = 0;; i++)
	{
		//Numerator and denominator for the (2n choose n) factor, terms 1 & 2
		long long int chooseTermNumer1 = 1;
		long long int chooseTermDenom1 = 1;
		for(int j = 0; j < (2 * i + 1); j++)	//# of times thru loop 1, 3, 5
		{
			chooseTermNumer1 *= (j * 2 + 1);
			chooseTermDenom1 *= (j * 2 + 2);
		}
		//Calculate these based on term 1 numer/denom, since numer is just
		//multiplied by next odd and denom is multiplied by next even
		long long int chooseTermNumer2 = chooseTermNumer1 * ((i * 2 + 2) * 2 + 1);
		long long int chooseTermDenom2 = chooseTermDenom1 * ((i * 2 + 2) * 2 + 2);
		long double chooseTerm1 = (long double) chooseTermNumer1 / chooseTermDenom1;
		long double chooseTerm2 = (long double) chooseTermNumer2 / chooseTermDenom2;
		long double term1 = chooseTerm1 * intpow(x, i * 4 + 3) / (i * 4 + 3);
		long double term2 = chooseTerm2 * intpow(x, i * 4 + 5) / (i * 4 + 5);
		//See if converged
		long double termDiff = term1 - term2;
		if(-LONG_DOUBLE_CONV_TOL < termDiff && termDiff < LONG_DOUBLE_CONV_TOL)
			break;
		rv += term1;
		rv += term2;
	}
	return rv;
}

double acos(double x)
{
	if(x < -1.0 || x > 1.0 || x == NAN)
		return NAN;
	return PI / 2.0 - asin(x);
}

float acosf(float x)
{
	if(x < -1.0 || x > 1.0 || x == NAN)
		return NAN;
	return PI / 2.0 - asinf(x);
}

long double acosl(long double x)
{
	if(x < -1.0 || x > 1.0 || x == NAN)
		return NAN;
	return PI / 2.0 - asinl(x);
}

double atan(double x)
{
	double rv = 0;
	for(int i = 0;; i++)
	{
		double term1 = intpow(x, i * 4 + 1) / (i * 4 + 1);
		double term2 = intpow(x, i * 4 + 3) / (i * 4 + 3);
		double termDiff = term1 - term2;
		if(-DOUBLE_CONV_TOL < termDiff && termDiff < DOUBLE_CONV_TOL)
			break;
		rv += term1;
		rv -= term2;
	}
	return rv;
}

float atanf(float x)
{
	float rv = 0;
	for(int i = 0;; i++)
	{
		float term1 = intpowf(x, i * 4 + 1) / (i * 4 + 1);
		float term2 = intpowf(x, i * 4 + 3) / (i * 4 + 3);
		float termDiff = term1 - term2;
		if(-FLOAT_CONV_TOL < termDiff && termDiff < FLOAT_CONV_TOL)
			break;
		rv += term1;
		rv -= term2;
	}
	return rv;
}

long double atanl(long double x)
{
	long double rv = 0;
	for(int i = 0;; i++)
	{
		long double term1 = intpow(x, i * 4 + 1) / (i * 4 + 1);
		long double term2 = intpow(x, i * 4 + 3) / (i * 4 + 3);
		long double termDiff = term1 - term2;
		if(-LONG_DOUBLE_CONV_TOL < termDiff && termDiff < LONG_DOUBLE_CONV_TOL)
			break;
		rv += term1;
		rv -= term2;
	}
	return rv;
}

double atan2(double y, double x)
{
	if(y == NAN || x == NAN)
		return NAN;
	byte yIsZero = -DOUBLE_CONV_TOL < y && y < DOUBLE_CONV_TOL ? 1 : 0;
	byte xIsZero = -DOUBLE_CONV_TOL < x && x < DOUBLE_CONV_TOL ? 1 : 0;
	//if x is ~0 and y is not, just return -PI/2 or PI/2 depending on sign of y
	//if both x and y 0 or either NAN, return NAN
	if(yIsZero && xIsZero)
		return NAN;
	else if(!yIsZero && xIsZero)
	{
		if(y > 0)
			return PI / 2;
		else
			return -PI / 2;
	}
	//if here, then y / x meaningful and just use atan Taylor
	return atan(y / x);
}

float atan2f(float y, float x)
{
	if(y == NAN || x == NAN)
		return NAN;
	byte yIsZero = -FLOAT_CONV_TOL < y && y < FLOAT_CONV_TOL ? 1 : 0;
	byte xIsZero = -FLOAT_CONV_TOL < x && x < FLOAT_CONV_TOL ? 1 : 0;
	//if x is ~0 and y is not, just return -PI/2 or PI/2 depending on sign of y
	//if both x and y 0 or either NAN, return NAN
	if(yIsZero && xIsZero)
		return NAN;
	else if(!yIsZero && xIsZero)
	{
		if(y > 0)
			return PI / 2;
		else
			return -PI / 2;
	}
	return atanf(y / x);
}

long double atan2l(long double y, long double x)
{
	if(y == NAN || x == NAN)
		return NAN;
	byte yIsZero = -LONG_DOUBLE_CONV_TOL < y && y < LONG_DOUBLE_CONV_TOL ? 1 : 0;
	byte xIsZero = -LONG_DOUBLE_CONV_TOL < x && x < LONG_DOUBLE_CONV_TOL ? 1 : 0;
	//if x is ~0 and y is not, just return -PI/2 or PI/2 depending on sign of y
	//if both x and y 0 or either NAN, return NAN
	if(yIsZero && xIsZero)
		return NAN;
	else if(!yIsZero && xIsZero)
	{
		if(y > 0)
			return PI / 2;
		else
			return -PI / 2;
	}
	return atanl(y / x);
}

//Hyperbolic trig functions
double sinh(double x)
{
	if(x == NAN)
		return NAN;
	double eX = exp(x);	//save this to avoid doing exp twice, worse than mult
	if(-DOUBLE_CONV_TOL < eX && eX < DOUBLE_CONV_TOL) //x so small that would get div by 0
		return NAN;
	return (eX * eX - 1) / 2 * eX;
}

float sinhf(float x)
{
	if(x == NAN)
		return NAN;
	float eX = expf(x);
	if(-FLOAT_CONV_TOL < eX && eX < FLOAT_CONV_TOL)
		return NAN;
	return (eX * eX - 1) / 2 * eX;
}

long double sinhl(long double x)
{
	if(x == NAN)
		return NAN;
	long double eX = expl(x);
	if(-LONG_DOUBLE_CONV_TOL < eX && eX < LONG_DOUBLE_CONV_TOL)
		return NAN;
	return (eX * eX - 1) / 2 * eX;
}

double cosh(double x)
{
	if(x == NAN)
		return NAN;
	double eX = exp(x);
	if(-DOUBLE_CONV_TOL < eX && eX < DOUBLE_CONV_TOL)
		return NAN;
	return (eX * eX + 1) / 2 * eX;
}

float coshf(float x)
{
	if(x == NAN)
		return NAN;
	float eX = expf(x);
	if(-FLOAT_CONV_TOL < eX && eX < FLOAT_CONV_TOL)
		return NAN;
	return (eX * eX + 1) / 2 * eX;
}

long double coshl(long double x)
{
	if(x == NAN)
		return NAN;
	long double eX = expl(x);
	if(-LONG_DOUBLE_CONV_TOL < eX && eX < LONG_DOUBLE_CONV_TOL)
		return NAN;
	return (eX * eX + 1) / 2 * eX;
}

double tanh(double x)
{
	if(x == NAN)
		return NAN;
	double e2X = exp(x);
	if(-DOUBLE_CONV_TOL < e2X && e2X < DOUBLE_CONV_TOL)
		return -1;	//if |x| big, automatically converges to +-1
	else if(e2X == HUGE_VAL)
		return 1;
	e2X *= e2X;
	return (e2X + 1) / (e2X - 1);
}

float tanhf(float x)
{
	if(x == NAN)
		return NAN;
	float e2X = expf(x);
	if(-FLOAT_CONV_TOL < e2X && e2X < FLOAT_CONV_TOL)
		return -1;	//if |x| big, automatically converges to +-1
	else if(e2X == HUGE_VAL)
		return 1;
	e2X *= e2X;
	return (e2X + 1) / (e2X - 1);
}

long double tanhl(long double x)
{
	if(x == NAN)
		return NAN;
	long double e2X = expl(x);
	if(-LONG_DOUBLE_CONV_TOL < e2X && e2X < LONG_DOUBLE_CONV_TOL)
		return -1;	//if |x| big, automatically converges to +-1
	else if(e2X == HUGE_VAL)
		return 1;
	e2X *= e2X;
	return (e2X + 1) / (e2X - 1);
}

//Exponential and logarithmic functions
double exp(double x)
{
	double rv = 0;
	for(int i = 0;; i++)
	{
		double term = intpow(x, i) / factorial(i);
		if(-DOUBLE_CONV_TOL < term && term < DOUBLE_CONV_TOL)
			break;
		rv += term;
	}
	return rv;
}

float expf(float x)
{
	float rv = 0;
	for(int i = 0;; i++)
	{
		float term = intpowf(x, i) / factorial(i);
		if(-FLOAT_CONV_TOL < term && term < FLOAT_CONV_TOL)
			break;
		rv += term;
	}
	return rv;
}

long double expl(long double x)
{
	long double rv = 0;
	for(int i = 0;; i++)
	{
		long double term = intpowl(x, i) / factorial(i);
		if(-LONG_DOUBLE_CONV_TOL < term && term < LONG_DOUBLE_CONV_TOL)
			break;
		rv += term;
	}
	return rv;
}

double frexp(double x, int* exponent)
{
	if(x == NAN)
	{
		*exponent = 0;
		return NAN;
	}
	//64-bit type, bit 63 = sign, next 11 = exp, last 52 = mant
	unsigned long long int binRep = *((unsigned long long int*) &x);
	//Isolate exponent
	unsigned long long int expBits = 0x7FF;	//that's 11 one bits
	*exponent = (binRep >> 52) & expBits;
	binRep &= (~expBits << 52);				//clear all exponent bits, sign/mant stay
	double rv = *((double*) binRep);
	return rv;
}

float frexpf(float x, int* exponent)
{
	if(x == NAN)
	{
		*exponent = 0;
		return NAN;
	}
	unsigned int binRep = *((unsigned int*) &x);
	unsigned int expBits = 0xFF;
	*exponent = (binRep >> 23) & expBits;
	binRep &= (~expBits << 23);
	float rv = *((float*) binRep);
	return rv;
}

long double frexpl(long double x, int* exponent)
{
	if(x == NAN)
	{
		*exponent = 0;
		return NAN;
	}
	byte* binRep = (byte*) &x;	//little endian, so first 8 bytes of this is the mant
	unsigned long long int mant = *((unsigned long long int*) &x);
z	unsigned short top16 = *((unsigned short) &binRep[8]);
	*exponent = (top16 & 0x7FFF) - 16383;	//endianness-independent least-significant 15 bits of top16
	*((unsigned short) &binRep[8]) &= 0x7FFF;	//clear exp bits in place
	return x;
}

double ldexp(double x, int exponent)
{
	unsigned long long int binRep = *((unsigned long long int) &x);
	unsigned long long int expMask = 0x7FF;
	exponent &= expMask;
	binRep &= ~(expMask << 52);	//clear bits 62-52
	binRep |= (exponent << 52);	//put in given exponent
	double rv = *((double*) &binRep);
	return rv;
}

float ldexpf(float x, int exponent)
{
	unsigned int binRep = *((unsigned int*) &x;
	unsigned int expMask = 0xFF;
	binRep &= ~(expMask << 23);
	exponent &= expMask;
	binRep |= exponent << 23;
	float rv = *((float*) &binRep);
	return rv;
}

long double ldexpl(long double x, int exponent)
{
	x *= intpow(2, exponent);
	return x;
}

double log(double x)
{
	if(x < DOUBLE_CONV_TOL)
		return NAN;
	double rv = 0;
	x -= 1.0;
	//now get x in range (0, 1) so Taylor applies correctly
	while(x < 1)
	{
		x /= E;
		rv += 1;
	}
	for(int i = 0;; i++)
	{
		double term1 = intpow(x, i * 2 + 1) / (i * 2 + 1);
		double term2 = intpow(x, i * 2 + 2) / (i * 2 + 2);
		double termDiff = term1 - term2;
		if(-DOUBLE_CONV_TOL < termDiff && termDiff < DOUBLE_CONV_TOL)
		{
			break;
		}
		rv += termDiff;
	}
	return rv;
}

float logf(float x)
{
	if(x < FLOAT_CONV_TOL)
		return NAN;
	float rv = 0;
	x -= 1.0;
	while(x < 1)
	{
		x /= E;
		rv += 1;
	}
	for(int i = 0;; i++)
	{
		float term1 = intpowf(x, i * 2 + 1) / (i * 2 + 1);
		float term2 = intpowf(x, i * 2 + 2) / (i * 2 + 2);
		float termDiff = term1 - term2;
		if(-FLOAT_CONV_TOL < termDiff && termDiff < FLOAT_CONV_TOL)
		{
			break;
		}
		rv += termDiff;
	}
	return rv;
}

long double logl(long double x)
{
	if(x < LONG_DOUBLE_CONV_TOL)
		return NAN;
	long double rv = 0;
	x -= 1.0;
	while(x < 1)
	{
		x /= E;
		rv += 1;
	}
	for(int i = 0;; i++)
	{
		long double term1 = intpow(x, i * 2 + 1) / (i * 2 + 1);
		long double term2 = intpow(x, i * 2 + 2) / (i * 2 + 2);
		long double termDiff = term1 - term2;
		if(-LONG_DOUBLE_CONV_TOL < termDiff && termDiff < LONG_DOUBLE_CONV_TOL)
		{
			break;
		}
		rv += termDiff;
	}
	return rv;
}

double log10(double x)
{
	return log(x) / 2.30258509299;
}

float log10f(float x)
{
	return logf(x) / 2.30258509299f;
}

long double log10l(long double x)
{
	return logl(x) / 2.30258509299;
}

double modf(double x, double* intpart)
{
	*intpart = (double) ((unsigned long long int) x);
	return x - *intpart;
}

float modff(float x, float* intpart)
{
	*intpart = (float) ((unsigned long long int) x);
	return x - *intpart;
}

long double modfl(long double x, long double* intpart)
{
	*intpart = (long double) ((unsigned long long int) x);
	return x - *intpart;
}

double pow(double base, double exponent)
{
	if(base == 0 && exponent == 0)
		return NAN;
	if(base == 0)
		return 0;
	if(exponent == 0)
		return 1;
	byte isExpInt = 0;
	int expIPart;
	modf(exponent, &ipart);
	if((double) expIPart == exponent)
		isExpInt = 1;
	if(base < 0)
	{
		if(isExpInt)
		{
			//negative base, integer exponent
			if(exponent > 0)
			{
				//negative base, integer positive exp
				//(-5.2)^4
				if(exponent % 2 == 0)
				{
					//exp even, result is positive
					return intpow(-base, exponent);
				}
				else
				{
					//exp odd, result is negative
					return -intpow(-base, exponent);
				}
			}
			else
			{
				//negative base, integer negative exp
				//same as when exponent is positive, except take reciprocal
				if((-exponent) % 2 == 0)
				{
					return 1.0 / intpow(-base, exponent);
				}
				else
				{
					return -1.0 / intpow(-base, exponent);
				}
			}
		}
		else
			return NAN;
	}
	else
	{
		//use faster routine for integer exponents
		if(isExpInt)
			return intpow(base, (int) exponent);
		else
			return exp(exponent * log(base));
	}
}

float powf(float base, float exponent)
{
	if(base == 0 && exponent == 0)
		return NAN;
	if(base == 0)
		return 0;
	if(exponent == 0)
		return 1;
	byte isExpInt = 0;
	int expIPart;
	modff(exponent, &ipart);
	if((float) expIPart == exponent)
		isExpInt = 1;
	if(base < 0)
	{
		if(isExpInt)
		{
			//negative base, integer exponent
			if(exponent > 0)
			{
				//negative base, integer positive exp
				//(-5.2)^4
				if(exponent % 2 == 0)
				{
					//exp even, result is positive
					return intpowf(-base, exponent);
				}
				else
				{
					//exp odd, result is negative
					return -intpowf(-base, exponent);
				}
			}
			else
			{
				//negative base, integer negative exp
				//same as when exponent is positive, except take reciprocal
				if((-exponent) % 2 == 0)
				{
					return 1.0 / intpowf(-base, exponent);
				}
				else
				{
					return -1.0 / intpowf(-base, exponent);
				}
			}
		}
		else
			return NAN;
	}
	else
	{
		//use faster routine for integer exponents
		if(isExpInt)
			return intpowf(base, (int) exponent);
		else
			return expf(exponent * logf(base));
	}
}

long double powl(long double base, long double exponent)
{
	if(base == 0 && exponent == 0)
		return NAN;
	if(base == 0)
		return 0;
	if(exponent == 0)
		return 1;
	byte isExpInt = 0;
	int expIPart;
	modfl(exponent, &ipart);
	if((long double) expIPart == exponent)
		isExpInt = 1;
	if(base < 0)
	{
		if(isExpInt)
		{
			//negative base, integer exponent
			if(exponent > 0)
			{
				//negative base, integer positive exp
				//(-5.2)^4
				if(exponent % 2 == 0)
				{
					//exp even, result is positive
					return intpowl(-base, exponent);
				}
				else
				{
					//exp odd, result is negative
					return -intpowl(-base, exponent);
				}
			}
			else
			{
				//negative base, integer negative exp
				//same as when exponent is positive, except take reciprocal
				if((-exponent) % 2 == 0)
				{
					return 1.0 / intpowl(-base, exponent);
				}
				else
				{
					return -1.0 / intpowl(-base, exponent);
				}
			}
		}
		else
			return NAN;
	}
	else
	{
		//use faster routine for integer exponents
		if(isExpInt)
			return intpowl(base, (int) exponent);
		else
			return expl(exponent * logl(base));
	}
}

double sqrt(double x)
{
	if(x < 0)
		return NAN;
	return exp(0.5 * log(x));
}

float sqrtf(float x)
{
	if(x < 0)
		return NAN;
	return expf(0.5f * logf(x));
}

long double sqrtl(long double x)
{
	if(x < 0)
		return NAN;
	return expl(0.5 * logl(x));
}

double ceil(double x)
{
	double ipart;
	double fpart = modf(x, &ipart);
	if(ipart == x)
		return x;
	else
		return (double) ((unsigned long long int) x + 1);
}

float ceilf(float x)
{
	float ipart;
	float fpart = modff(x, &ipart);
	if(ipart == x)
		return x;
	else
		return (float) ((unsigned long long int) x + 1);
}

long double ceill(long double x)
{
	long double ipart;
	long double fpart = modf(x, &ipart);
	if(ipart == x)
		return x;
	else
		return (long double) ((unsigned long long int) x + 1);
}

double floor(double x)
{
	return (double) ((unsigned long long int) x);
}

float floorf(float x)
{
	return (float) ((unsigned long long int) x)
}

long double floorl(long double x)
{
	return (long double) ((unsigned long long int) x)
}

double fmod(double numer, double denom)
{
	if(denom == 0)
		return NAN;
	int quotient = numer / denom;
	return numer - (denom * quotient);
}

float fmodf(float numer, float denom)
{
	if(denom == 0)
		return NAN;
	int quotient = numer / denom;
	return numer - (denom * quotient);
}

long double fmodl(long double numer, long double denom)
{
	if(denom == 0)
		return NAN;
	int quotient = numer / denom;
	return numer - (denom * quotient);
}

double fabs(double x)
{
	return x < 0 ? -x : x;
}

float fabsf(float x)
{
	return x < 0 ? -x : x;
}

long double fabsl(long double x)
{
	return x < 0 ? -x : x;
}