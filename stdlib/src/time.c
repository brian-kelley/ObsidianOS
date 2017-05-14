#include "time.h"

//Timezone, in hours offset from UTC/GMT
//TODO: Let user configure in a nice way
//TODO: Also let user set the time
static int timezone= -6;

void setTimezone(int offset)
{
  //sanity check?
  if(offset > 12 || offset < -12)
  {
    puts("ERROR: Timezone out of range -12 to 12");
    while(1);
  }
  timezone = offset;
}

int getTimezone()
{
  return timezone;
}

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

static char ctimeBuf[32];
static struct tm tmState;
//RTC interrupt handler increments clockCounter
clock_t clockCounter = 0;

enum {
  JANUARY,
  FEBRUARY,
  MARCH,
  APRIL,
  MAY,
  JUNE,
  JULY,
  AUGUST,
  SEPTEMBER,
  OCTOBER,
  NOVEMBER,
  DECEMBER
};

enum {
  SUNDAY,
  MONDAY,
  TUESDAY,
  WEDNESDAY,
  THURSDAY,
  FRIDAY,
  SATURDAY
};

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

static bool bcd = true;

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
  while(rtcUpdating());
  do
  {
    //note: values adjusted as needed so all start at zero
    //i.e. January 1 has mon = 0 and mday = 0
    temp = rtc;
    rtc.sec = readCMOS(0x0);
    rtc.min = readCMOS(0x2);
    rtc.hour = readCMOS(0x4);
    rtc.mday = readCMOS(0x7) - 1;
    rtc.mon = readCMOS(0x8) - 1;
    rtc.year = readCMOS(0x9);
  }
  while(!rtcEqual(&rtc, &temp));
  return rtc;
}

void initTime()
{
  writeport(0x70, 0x0B);
  //According to OSDev, this should get BCD flag
  //But, doesn't work in QEMU (is 0 but values are BCD)
  //bcd = readport(0x71) & 0x4;
  bcd = true;
}

//get human readable time from struct tm
char* asctime(const struct tm* t)
{
  //Format: "Mmm Www dd hh:mm:ss yyyy"
  sprintf(ctimeBuf, "%.3s %.3s %02i %02i:%02i%02i %04i\n", months[t->tm_mon],
      weeks[t->tm_wday], t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, t->tm_year);
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
  //convert local time to GMT (as struct tm), then just use asctime
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
  //all days have exactly 24 * 3600 seconds, so take away seconds in current day
  time_t daySeconds = *t % (24 * 3600);
  time_t days = *t / (24 * 3600);
  tmState.tm_sec = daySeconds % 60;
  daySeconds -= tmState.tm_sec;
  tmState.tm_min = daySeconds / 60;
  daySeconds -= 60 * tmState.tm_min;
  tmState.tm_hour = daySeconds / 3600;
  //get exact number of complete leap years since 1970
  //subtract 2 regular years (for 1970 and 1971), then add one for 1972,
  //  plus one for every complete 4-year cycle after that
  time_t leapYears = 1 + ((days - 365 * 2) / 1461);
  //subtract the leap days, then get whole years
  tmState.tm_year = (days - leapYears) / 365;
  //now make days just the complete days in current year, so far
  tmState.tm_yday = days - tmState.tm_year * 365 - leapYears;
  //count up through the months until sum of whole months >= yday
  tmState.tm_mon = 0;
  tmState.tm_mday = tmState.tm_yday;
  for(int i = 0; i < 12; i++)
  {
    int thisMonth = daysPerMonth[i];
    if(i == FEBRUARY && tmState.tm_year % 4 == 0)
    {
      thisMonth++;
    }
    if(tmState.tm_mday < thisMonth)
    {
      break;
    }
    tmState.tm_mday -= thisMonth;
    tmState.tm_mon++;
  }
  //day of week: 1-1-1970 is a Thursday
  tmState.tm_wday = (THURSDAY + days) % 7;
  //Final corrections:
  //only mday range starts at 1
  tmState.tm_mday++;
  //year is since 1970, make absolute
  tmState.tm_year += 1970;
  return &tmState;
}

//convert time_t to tm, in local timezone
struct tm* localtime(const time_t* t)
{
  //convert from local to GMT
  time_t gmt = *t + (3600 * timezone);
  return gmtime(&gmt);
}

//convert tm (local) to time_t (inverse of localtime)
time_t mktime(struct tm* t)                
{
  time_t days = (t->tm_year - 1970) * 365 + t->tm_yday;
  days += (1 + t->tm_year - 1970) / 4;
  return days * (24 * 3600) + (t->tm_hour + timezone) * 3600 + t->tm_min * 60 + t->tm_sec;
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
  for(int i = 0; i < rtc.mon; i++)
  {
    yday += daysPerMonth[i];
    if(rtc.year % 4 == 0 && i == FEBRUARY)
    {
      yday++;
    }
  }
  //assume RTC year >= 70 means 19XX, and < 70 means 20XX
  //note: year is in relation to 1970, not 0 AD
  unsigned year = rtc.year >= 70 ? rtc.year : 30 + rtc.year;
  unsigned leapDays = (1 + year) / 4;
  if(rtc.mon > 2 && rtc.year % 4 == 0)
  {
    //current year also leap year, and time is past February so another leap day passed
    leapDays++;
  }
  time_t unixTime = 365 * year + leapDays + yday;
  unixTime *= (24 * 3600);
  unixTime += (rtc.hour * 3600) + (rtc.min * 60) + rtc.sec;
  if(t)
  {
    *t = unixTime;
  }
  return unixTime;
}

void dateCommand()
{
  //Date command, modeled after OS X version
  time_t ut = time(NULL);
  struct tm* t = localtime(&ut);
  printf("%.3s %.3s %02i %02i:%02i:%02i UTC%+i %04i\n", months[t->tm_mon],
      weeks[t->tm_wday], t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, getTimezone(), t->tm_year);
}

