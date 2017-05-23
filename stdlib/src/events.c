#include "events.h"

extern void puts(const char* str);
extern void disableInterrupts();
extern void enableInterrupts();

volatile Event eq[EVENT_QUEUE_MAX];
volatile int eventHead;
volatile int eventSize;

void initEvents()
{
  eventHead = 0;
  eventSize = 0;
}

void addEvent(Event e)
{
  if(eventSize == EVENT_QUEUE_MAX)
  {
    puts("ERROR: Event queue full");
  }
  eq[(eventHead + eventSize) % EVENT_QUEUE_MAX] = e;
  eventSize++;
}

Event getNextEvent()
{
  //wait for an event
  while(eventSize == 0);
  disableInterrupts();
  Event e = eq[eventHead];
  eventHead = (eventHead + 1) % EVENT_QUEUE_MAX;
  eventSize--;
  enableInterrupts();
  return e;
}

bool haveEvent()
{
  return eventSize > 0;
}

