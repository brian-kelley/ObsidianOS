#include "time.h"

//struct that is populated from RTC values
typedef struct
{
  byte sec;
  byte min;
  byte hour;
  byte mday;
  byte mon;
  byte year;    //only low 2 digits!
} RTC;

static char ctimeBuf[26];
static struct tm tmState;
//RTC interrupt handler increments clockCounter
clock_t clockCounter = 0;

static const char* months[] = {
  "January",
  "February",
  "March",
  "April",
  "May",
  "June",
  "July",
  "August",
  "September",
  "October",
  "November",
  "December"
};

static const char* weeks[] = {
  "Sunday",
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saturday"
};

static const int daysPerMonth[] = {
  31,
  28,
  31,
  30,
  31,
  30,
  31,
  31,
  30,
  31,
  30,
  31
};

static bool bcd;

//read a CMOS reg
//does BCD->binary conversion if needed
static byte readCMOS(byte reg)
{
  writeport(0x70, reg);
  byte val = readport(0x71);
  if(bcd)
  {
    return (val >> 4) * 10 + (val & 0xF);
  }
  else
  {
    return val;
  }
}

static bool rtcUpdating()
{
  return readCMOS(0xA) & 0x80;
}

static bool rtcEqual(RTC* lhs, RTC* rhs)
{
  return memcmp(lhs, rhs, 6) == 0;
}

static RTC getRTC()
{
  RTC rtc;
  RTC temp;
  do
  {
    temp = rtc;
    rtc.sec = readCMOS(0x0);
    rtc.min = readCMOS(0x2);
    rtc.hour = readCMOS(0x4);
    rtc.mday = readCMOS(0x7);
    rtc.mon = readCMOS(0x8);
    rtc.year = readCMOS(0x9);
  }
  while(!rtcEqual(&rtc, &temp));
  return rtc;
}

void initTime()
{
  writeport(0x70, 0x0B);
  bcd = readport(0x71) & 0x4;
}

//get human readable time from struct tm
char* asctime(const struct tm* t)
{
  //Format: "Mmm Www dd hh:mm:ss yyyy"
  strncpy(ctimeBuf, weeks[t->tm_wday], 3);
  ctimeBuf[3] = ' ';
  strncpy(ctimeBuf + 4, months[t->tm_mon], 3);
  ctimeBuf[7] = ' ';
  sprintf(ctimeBuf + 8, "%02i:%02i:%02i %04i\n", t->tm_hour, t->tm_min, t->tm_sec, t->tm_year);
  return ctimeBuf;
}

//get current clock ticks (takes ~1000 hours to overflow)
clock_t clock()
{
  return clockCounter;
}

//get human readable time from time_t
char* ctime(const time_t* t)
{
  //TODO: actually handle timezones
  return asctime(localtime(t));
}

//difference in seconds
double difftime(time_t end, time_t start)
{
  return (double) end - start;
}

//convert time_t to tm, in GMT timezone
struct tm* gmtime(const time_t* t)
{
  return &tmState;
}

//convert time_t to tm, in local timezone
struct tm* localtime(const time_t* t)
{
  return &tmState;
}

//convert tm (local) to time_t (inverse of localtime)
time_t mktime(struct tm* t)                
{
  //TODO: timezones
  time_t days = (t->tm_year - 1970) * 365 + t->tm_yday;
  days += (1 + t->tm_year - 1970) / 4;
  return days * (24 * 3600) + t->tm_hour * 3600 + t->tm_min * 60 + t->tm_sec;
}

//formatted time
size_t strftime(char* str, size_t maxsize, const char* fmt, const struct tm* t)
{
  //TODO
  return 0;
}

//return time, and if t != NULL, also set *t to time
time_t time(time_t* t)
{
  RTC rtc = getRTC();
  //first, count day in year
  unsigned yday = rtc.mday;
  for(int i = 1; i < rtc.mon; i++)
  {
    yday += daysPerMonth[i];
    if(rtc.year % 4 == 0 && i == 2)
    {
      yday++;
    }
  }
  //assume RTC year >= 70 means 19XX, and < 70 means 20XX
  unsigned year = rtc.year >= 70 ? 1900 + rtc.year : 2000 + rtc.year;
  unsigned leapDays = (1 + year - 1970) / 4;
  if(rtc.mon > 2 && rtc.year % 4 == 0)
  {
    //current year also leap year, and time is past February so another leap day passed
    leapDays++;
  }
  time_t unixTime = 365 * year + leapDays;
  unixTime *= (24 * 3600);
  unixTime += rtc.hour * 3600 + rtc.min * 60 + rtc.sec;
  if(t)
  {
    *t = unixTime;
  }
  return unixTime;
}

