#ifndef EVENTS_H
#define EVENTS_H

#define EVENT_QUEUE_MAX 64

//Obsidian event system

enum EventType
{
  KEY_EVENT,        //Key press or release
  MOTION_EVENT,     //Mouse motion
  BUTTON_EVENT,     //Mouse button press or release
  CLOCK_EVENT       //RTC 1024 Hz clock
};

typedef struct KeyEvent
{
  byte scancode;
  bool pressed;
};

typedef struct MotionEvent
{
  short dx;
  short dy;
};

typedef struct ButtonEvent
{
  short button;
  bool pressed;
};

typedef struct ClockEvent {};

typedef struct Event
{
  int type;
  union
  {
    KeyEvent key;
    MotionEvent motion;
    ButtonEvent button;
    ClockEvent clock;
  } e;
};

void initEvents();
void addEvent(Event e);
Event getNextEvent();

#endif

