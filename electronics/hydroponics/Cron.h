#ifndef Cron_h
#define Cron_h

#include "RTClib.h"
#include "Arduino.h"

typedef void (*alarm_callback) (const String code);
typedef void (*tick_callback) (const DateTime now);

class Cron
{
  public:
    Cron();
    void begin(alarm_callback onAlarm, tick_callback onTick);
    void tick();
    DateTime getTime();
    short getTemp();

  private:
    RTC_DS3231 rtc;
    alarm_callback _onAlarm;
    tick_callback _onTick;
    unsigned short _timerSize;
    char * _timers;
    unsigned long lastEpoch = 0;
    bool checkTime(const char *sentence, const short current);
    String isTime(const char* sentence, DateTime now);
};

#endif

