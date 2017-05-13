#ifndef TIME_H
#define TIME_H

#include "globalDefines.h"
#include "string.h"

#define CLOCKS_PER_SEC 1024

//avoid 2038 problem!
typedef unsigned /*long*/ long time_t;
//even with 32-bit clock, wraparound only happens once in 1000 hours
typedef size_t clock_t;

struct tm
{
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;
} ; 

char* asctime(const struct tm* t);          //get human readable time from struct tm
clock_t clock();                            //get current clock ticks
char* ctime(const time_t* t);               //get human readable time from time_t
double difftime(time_t end, time_t start);  //difference in seconds
struct tm* gmtime(const time_t* t);         //convert time_t to tm, in GMT timezone
struct tm* localtime(const time_t* t);      //convert time_t to tm, in local timezone
time_t mktime(struct tm* t);                //convert tm (local) to time_t (inverse of localtime)
size_t strftime(char* str, size_t maxsize, const char* fmt, const struct tm* t);  //formatted time
time_t time(time_t* t);                     //return time, and if t set *t to time also

#endif

