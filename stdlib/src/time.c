#include "time.h"
#include "graphics.h"

//Timezone, in hours offset from UTC/GMT
//TODO: Let user configure in a nice way
//TODO: Also let user set the time
static int timezone= -6;

void setTimezone(int offset)
{
  //sanity check
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
//clockCounter / CLOCKS_PER_SEC gives uptime
extern volatile clock_t clockCounter;

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

static const char* weekdays[] = {
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
  sprintf(ctimeBuf, "%.3s %.3s %02i %02i:%02i:%02i %04i\n", months[t->tm_mon],
      weekdays[t->tm_wday], t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, t->tm_year);
  return ctimeBuf;
}

//get current clock ticks (takes ~1000 hours to overflow)
volatile clock_t clock()
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
  daySeconds /= 60;
  //daySeconds is now minutes
  tmState.tm_min = daySeconds % 60;
  tmState.tm_hour = daySeconds / 60;
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
  return days * (24 * 3600) + (t->tm_hour - timezone) * 3600 + t->tm_min * 60 + t->tm_sec;
}

//formatted time
size_t strftime(char* str, size_t maxsize, const char* fmt, const struct tm* t)
{
  //maxsize includes the \0: ignore that to compare to output directly
  maxsize--;
  size_t output = 0;
  for(const char* iter = fmt; *iter; iter++)
  {
    if(*iter != '%')
    {
      str[output++] = *iter;
      if(output == maxsize)
        return 0;
    }
    else
    {
      //TODO: handle week-based calendar correctly
      //Make a temporary buffer to store just this value
      //that way, bounds checking can happen in one place
      char buf[32];
      iter++;
      switch(*iter)
      {
        case 'a':
          memcpy(buf, weekdays[t->tm_wday], 3);
          buf[3] = 0;
          break;
        case 'A':
          strcpy(buf, weekdays[t->tm_wday]);
          break;
        case 'b':
        case 'h':
          memcpy(buf, months[t->tm_mon], 3);
          buf[3] = 0;
          break;
        case 'B':
          strcpy(buf, months[t->tm_mon]);
          break;
        case 'c':
          {
            char* formatTime = asctime(t);
            strcpy(buf, formatTime);
          }
          break;
        case 'C':
          sprintf(buf, "%02i", t->tm_year / 100);
          break;
        case 'd':
          sprintf(buf, "%02i", t->tm_mday);
          break;
        case 'x':
        case 'D':
          sprintf(buf, "%02i/%02i/%02i", t->tm_mon + 1, t->tm_mday, t->tm_year % 100);
          break;
        case 'e':
          sprintf(buf, "%2i", t->tm_mon);
          break;
        case 'F':
          sprintf(buf, "%04i-%02i-%02i", t->tm_year, t->tm_mon + 1, t->tm_mday);
          break;
        case 'H':
          sprintf(buf, "%02i", t->tm_hour);
          break;
        case 'I':
          sprintf(buf, "%02i", ((t->tm_hour) + 11) % 12 + 1);
          break;
        case 'j':
          sprintf(buf, "%03i", t->tm_yday);
          break;
        case 'm':
          sprintf(buf, "%02i", t->tm_mon + 1);
          break;
        case 'M':
          sprintf(buf, "%02i", t->tm_min);
          break;
        case 'n':
          buf[0] = '\n';
          buf[1] = 0;
          break;
        case 'p':
          {
            buf[1] = 'M';
            buf[2] = 0;
            if(t->tm_hour >= 12)
              buf[0] = 'P';
            else
              buf[0] = 'A';
            break;
          }
        case 'r':
          {
            int hour12 = ((t->tm_hour) + 11) % 12 + 1;
            char amPM = t->tm_hour >= 12 ? 'p' : 'a';
            sprintf(buf, "%02i:%02i:%02i %cm", hour12, t->tm_min, t->tm_sec, amPM);
            break;
          }
        case 'R':
          sprintf(buf, "%02i:%02i", t->tm_hour, t->tm_min);
          break;
        case 'S':
          sprintf(buf, "%02i", t->tm_sec);
          break;
        case 't':
          buf[0] = '\t';
          buf[1] = 0;
          break;
        case 'T':
        case 'X':
          sprintf(buf, "%02i:%02i:%02i", t->tm_hour, t->tm_min, t->tm_sec);
          break;
        case 'u':
          //Monday is 1, and range is 1-7
          sprintf(buf, "%i", t->tm_wday == 0 ? 7 : t->tm_wday);
          break;
        case 'U':
        case 'V':
        case 'W':
          sprintf(buf, "%02i", t->tm_yday / 7);
          break;
        case 'w':
          sprintf(buf, "%i", t->tm_wday);
          break;
        case 'z':
          sprintf(buf, "%+i", 100 * timezone);
          break;
        case 'Z':
          sprintf(buf, "UTC%+i\n", timezone);
          break;
        case 'g':
        case 'y':
          sprintf(buf, "%02i", t->tm_year % 100);
          break;
        case 'G':
        case 'Y':
          sprintf(buf, "%04i", t->tm_year);
          break;
        case '%':
          buf[0] = '%';
          buf[1] = 0;
          break;
      }
      size_t buflen = strlen(buf);
      if(buflen > maxsize - output)
        return 0;
      memcpy(str + output, buf, buflen);
      output += buflen;
    }
  }
  str[output++] = 0;
  return output;
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
  char buf[64];
  strftime(buf, 64, "%a, %b %d %Y, %r %Z", t);
  puts(buf);
}

void sleepS(int s)
{
  int ticks = s * CLOCKS_PER_SEC;
  clock_t start = clock();
  clock_t end = start + ticks;
  while(clock() < end);
}

void sleepMS(int ms)
{
  int ticks = ms * CLOCKS_PER_SEC / 1000;
  clock_t start = clock();
  clock_t end = start + ticks;
  while(clock() < end);
}

void graphicalClock()
{
  byte* fb = (byte*) 0xA0000;
  time_t t = time(NULL);
  while(1)
  {
    memset(fb, 0, 64000);
    if(haveEvent())
    {
      Event e = getNextEvent();
      if(e.type == KEY_EVENT && e.e.key.pressed)
        return;
    }
    struct tm* timeStruct = localtime(&t);
    int sec = timeStruct->tm_sec;
    int minute = timeStruct->tm_min;
    int hour = timeStruct->tm_hour;
    float secTheta = PI / 2 - (2 * PI) / 60 * (sec - 1);
    float minTheta = PI / 2 - (2 * PI) / 60 * (minute - 1);
    float hourTheta = PI / 2 - (2 * PI) / 12 * (hour + minute / 60.0f);
    glClear(0);
    glColor1i(0x1F);
    drawLine(160, 100, 160 + 50 * cos(secTheta), 100 - 50 * sin(secTheta));
    glColor1i(0x1D);
    drawLine(160, 100, 160 + 50 * cos(minTheta), 100 - 50 * sin(minTheta));
    glColor1i(0x1B);
    drawLine(160, 100, 160 + 30 * cos(hourTheta), 100 - 30 * sin(hourTheta));
    //add 60 tick marks
    glColor1i(0xF);
    for(int i = 0; i < 60; i++)
    {
      float theta = PI / 2 - i * (2 * PI / 60);
      int x1 = 160 + 60 * cos(theta);
      int y1 = 100 + 60 * sin(theta);
      int x2, y2;
      if(i % 5)
      {
        x2 = 160 + 63 * cos(theta);
        y2 = 100 + 63 * sin(theta);
      }
      else
      {
        x2 = 160 + 65 * cos(theta);
        y2 = 100 + 65 * sin(theta);
      }
      drawLine(x1, y1, x2, y2);
    }
    for(int i = 1; i <= 12; i++)
    {
      int tx = 160 + 80 * cos(PI / 2 - i * (2 * PI) / 12) - 4;
      int ty = 100 - 80 * sin(PI / 2 - i * (2 * PI) / 12) - 4;
      char buf[4];
      sprintf(buf, "%i", i);
      glText(buf, tx, ty, 0);
    }
    glFlush();
    time_t temp = t;
    while(time(&t) == temp);
  }
}

