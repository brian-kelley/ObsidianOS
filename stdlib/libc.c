/* String functions */

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;
typedef unsigned long int qword;

#include "string.h"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"

static char* lastTok;	//used to store the \0 where last token ended

//memcpy implementation that does work with overlapping src/dst arrays
void* memcpy(void* dst, const void* src, size_t num)
{
	byte* srcArr = (byte*) src;
	byte* dstArr = (byte*) dst;
	if(src < dst)
	{
		for(int i = 0; i < (int) num; i++)
		{
			dstArr[i] = srcArr[i];
		}
	}
	else if(src > dst)
	{
		for(int i = (int) num - 1; i >= 0; i--)
		{
			dstArr[i] = srcArr[i];
		}
	}
	return destination;
}

void* memmove(void* dst, const void* src, size_t num)
{
	return memcpy(dst, src, num);
}

char* strcpy(char* dst, const char* src)
{
	for(int i = 0;; i++)
	{
		if(src[i] == 0)
			break;
		dst[i] = src[i];
	}
	return dst;
}

char* strncpy(char* dst, const char* src, size_t num)
{
	for(int i = 0; i < num; i++)
	{
		if(src[i] == 0)
			break;
		dst[i] = src[i];
	}
	return dst;
}

char* strcat(char* dst, const char* src)
{
	//Find the end of dst where src will be copied
	int dstLen;
	for(int i = 0;; i++)
	{
		if(dst[i] == 0)
		{
			dstLen = i;
			break;
		}
	}
	//dstLen is the number of non-null chars in dst string
	//so dst[dstLen] is the \0, where src is copied to
	for(int i = 0;; i++)
	{
		if(src[i] == 0)
		{
			//Add the null to the end of the resulting string
			dst[dstLen - 1 + i] = 0;
			break;
		}
		dst[dstLen - 1 + i] = src[i];
	}
	return dst;
}

char* strncat(char* dst, const char* src, size_t num)
{
	int dstLen;
	for(int i = 0;; i++)
	{
		if(dst[i] == 0)
		{
			dstLen = i;
			break;
		}
	}
	int catLen = -1;
	for(int i = 0; i < (int) num; i++)
	{
		if(src[i] == 0)
		{
			catLen = dstLen - 1 + i;
			break;
		}
		dst[dstLen - 1 + i] = src[i];
	}
	if(catLen == -1)	//this means that src string wasn't completely copied
	{
		catLen = dstLen - 1 + num;
	}
	dst[catLen] = 0;
	return dst;
}

int memcmp(const void* ptr1, const void* ptr2, size_t num)
{	
	const byte* arr1 = (const byte*) ptr1;
	const byte* arr2 = (const byte*) ptr2;
	for(int i = 0; i < num; i++)
	{
		if(arr1[i] < arr2[i])
			return -1;
		if(arr1[i] > arr2[i])
			return 1;
	}
	return 0;
}

int strcmp(const char* str1, const char* str2)
{
	for(int i = 0;; i++)
	{
		if(str1[i] == 0 || str2[i] == 0)
			return 0;
		if(str1[i] < str2[i])
			return -1;
		if(str1[i] > str2[i])
			return 1;
	}
	return 0;
}

int strncmp(const char* str1, const char* str2, size_t num)
{
	for(int i = 0; i < (int) num; i++)
	{
		if(str1[i] == 0 || str2[i] == 0)
			return 0;
		if(str1[i] < str2[i])
			return -1;
		if(str1[i] > str2[i])
			return 1;
	}
	return 0;
}

void* memchr(void* ptr, int value, size_t num)
{
	byte* arr = (byte*) ptr;
	byte search = (byte*) value;
	for(int i = 0; i < (int) num; i++)
	{
		if(arr[i] == search)
			return ptr + i;
	}
	return NULL;
}

char* strchr(char* str, int character)
{
	for(int i = 0;; i++)
	{
		if(str[i] == 0)
			break;
		if(str[i] == (char) character)
			return str + i;
	}
	return NULL;
}

size_t strcspn(const char* str1, const char* str2)
{
	size_t strLength = 0;
	for(size_t i = 0;; i++)
	{
		if(str1[i] == 0)
		{
			strLength = i;
		}
		for(size_t j = 0;; j++)
		{
			if(str[i] == str[j])
				return i;
		}
	}
	return strLength;
}

char* strpbrk(char* str1, const char* str2)
{
	for(int i = 0;; i++)
	{
		if(str1[i] == 0)
			break;
		for(int j = 0;; j++)
		{
			if(str2[j] == 0)
				break;
			if(str1[i] == str2[j])
				return str1 + i;
		}
	}
	return NULL;
}

char* strrchr(char* str, int character)
{
	//first find end of string
	int stringLen = 0;
	for(;; stringLen++)
	{
		if(str[stringLen] == 0)
			break;
	}
	for(int i = stringLen - 1; i >= 0; i--)
	{
		if(str[i] == (char) character)
			return str + i;
	}
	return NULL;
}

size_t strspn(const char* str1, const char* str2)
{
	size_t rv = 0;
	for(;; rv++)
	{
		if(str1[rv] == 0)
			return NULL;
		byte found = 0;
		for(int j = 0;; j++)
		{
			if(str1[rv] == str2[j])
			{
				found = 1;
				break;
			}
		}
		if(!found)
			break;
	}
	return rv;
}

char* strstr(const char* str1, const char* str2)
{
	for(int i = 0;; i++)
	{
		if(str1[i] == 0)
			break;
		//check if str2 is part of str1 starting at str1[i]
		bool matches = 1;
		for(int j = 0;; j++)
		{
			if(str2[j] == 0)
				break;
			if(str1[i + j] != str2[j])
			{
				matches = 0;
				break;
			}
		}
		if(matches)
			return str1 + i;
	}
	return NULL;
}

char* strtok(char* str, const char* delimiters)
{
	if(str == NULL && lastTok == NULL)
		return NULL
	//Scan to start of token
	char* tokSearchBegin = str == NULL ? lastTok : str;
	char* tokStart = NULL;
	for(int i = 0;; i++)
	{
		if(tokSearchBegin[i] == 0)
			return NULL;
		byte isDelim = 0;
		for(int j = 0;; j++)
		{
			if(delimiters[j] == 0)
				break;
			if(tokSearchBegin[i] == delimiters[j])
			{
				isDelim = 1;
				break;
			}
		}
		if(!isDelim)
		{
			//token starts here
			tokStart = tokSearchBegin + i;
		}
	}
	//if here then tokStart points to start of token
	//scan for end of token, if reached before \0 then put \0 at end of tok
	for(int i = 0;; i++)
	{
		//check for delimiter in token
		if(tokStart[i] == 0)
			break;
		byte isDelim = 0;
		for(int j = 0;; j++)
		{
			if(delimiters[j] == 0)
				break;
			if(tokStart[i] == delimiters[j])
			{
				isDelim = 1;
				tokStart[i] = 0;
				break;
			}
		}
	}
	return tokStart;
}

void* memset(void* ptr, int value, size_t num)
{
	byte* bptr = (byte*) ptr;
	for(int i = 0; i < (int) num; i++)
	{
		bptr[i] = (byte) value;
	}
	return ptr;
}

size_t strlen(const char* str)
{
	for(int i = 0;; i++)
	{
		if(str[i] == 0)
			return i;
	}
	return 0;
}

/* Math functions */

//Proportion between sum to n - 1 and sum n in taylor series
//must be between (1 - CONV_TOL) and (1 + CONV_TOL)

#define FLOAT_CONV_TOL 1e-8
#define DOUBLE_CONV_TOL 1e-12
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

//simpler, faster version of pow for nonnegative integer powers
//used a lot with the Taylor series
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

double sin(double x)
{
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
	double cosine = cos(x);
	if(-DOUBLE_CONV_TOL < cosine && cosine < DOUBLE_CONV_TOL)
		return NAN;
	return sin(x) / cosine;
}

float tanf(float x)
{
	float cosine = cosf(x);
	if(-FLOAT_CONV_TOL < cosine && cosine < DOUBLUE_CONV_TOL)
		return NAN;
	return sinf(x) / cosine;
}

long double tanl(long double x)
{
	long double cosine = cosl(x);
	if(-LONG_DOUBLE_CONV_TOL < cosine && cosine < LONG_DOUBLE_CONV_TOL)
		return NAN;
	return sinl(x) / cosine;
}

double asin(double x)
{
	//Check for valid input
	if(x > 1.0 || x < -1.0)
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
	//Check for valid input
	if(x > 1.0f || x < -1.0f)
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
	//Check for valid input
	if(x > 1.0 || x < -1.0)
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
	if(x < -1.0 || x > 1.0)
		return NAN;
}

float acosf(float x)
{
	
}

long double acosl(long double x)
{
	
}

double atan(double x)
{
	
}

float atanf(float x)
{
	
}

long double atanl(long double x)
{
	
}

double atan2(double y, double x)
{
	
}

float atan2f(float y, float x)
{
	
}

long double atan2l(long double y, long double x)
{
	
}
