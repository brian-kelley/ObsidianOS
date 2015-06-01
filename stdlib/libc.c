/* String functions */

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;
typedef unsigned long int qword;

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
	if(str == NULL)
	{
		//Must use "lastTok" to begin looking for next token
		char* tokBegin = str;
		for(;; tokBegin++)
		{
			if(
			bool endOfToken = 0;
			for(int j = 0;; j++)
			{
				if(delimiters[j] == 0)
					break;
				if(*tokBegin == delimiters[j])
					endOfToken = 1;
			}
		}
	}
	else
	{
		//if lastTok was NULL, no more tokens for this string
		if(lastTok == NULL)
			return NULL;
		//use str to start search
	}
}
