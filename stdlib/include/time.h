#ifndef TIME_H
#define TIME_H

#include "globalDefines.h"
#include "string.h"

#define CLOCKS_PER_SEC 1024

typedef u64 time_t;
//32 bit would wraparound in 1000 hours
typedef u64 clock_t;

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
}; 

//Standard functions
char* asctime(const struct tm* t);          //get human readable time from struct tm
volatile clock_t clock();                            //get current clock ticks
char* ctime(const time_t* t);               //get human readable time from time_t
double difftime(time_t end, time_t start);  //difference in seconds
struct tm* gmtime(const time_t* t);         //convert time_t to tm, in GMT timezone
struct tm* localtime(const time_t* t);      //convert time_t to tm, in local timezone
time_t mktime(struct tm* t);                //convert tm (local) to time_t (inverse of localtime)
size_t strftime(char* str, size_t maxsize, const char* fmt, const struct tm* t);  //formatted time
time_t time(time_t* t);                     //return time, and if t set *t to time also

//Custom functions
void setTimezone(int offset);
int getTimezone();
void dateCommand();
//busy wait for given seconds/milliseconds
void sleepS(int s);
void sleepMS(int ms);
void graphicalClock();

#endif

