#ifndef __STDIO_H__
#define __STDIO_H__

int remove(const char* filename);
int rename(const char* oldname, const char* newname);
FILE* tmpfile();			//open temporary file with indeterminate name
char* tmpnam(char* str);	//open temporary file with given name
int fclose(FILE* stream);
int fflush(FILE* stream);
FILE* fopen(const char* filename, const char* mode); //mode is some combo of "r,w,a,+"
FILE* freopen(const char* filename, const char* mode, FILE* stream);
void setbuf(FILE* stream, char* buffer);
int setvbuf(FILE* stream, char* buffer, int mode, size_t size);
int fprintf(FILE* stream, const char* format, ...);

#endif