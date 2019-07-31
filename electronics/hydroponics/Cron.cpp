#include "config.h"

#include "RTClib.h"
#include "Arduino.h"
#include "Cron.h"

Cron::Cron() {}

void Cron::begin(alarm_callback onAlarm, tick_callback onTick) {
  _onAlarm = onAlarm;
  _onTick = onTick;

  rtc.begin();
}

bool Cron::checkTime(const char *sentence, const short current) {
  short number;

  if (sentence[0] == '*') { // any
    if (sscanf(sentence, "*/%d", &number) > 0) { // every each
      short numbers = round(60 / number);

      for (short i = 1; i <= numbers; i++){
        if (current == (number * i == 60 ? 0 : number * i)){
          return true;
        }
      }
      return false;
    }
    return true;
  }
  
  return atoi(sentence) == current; // exact
}

String Cron::isTime(const char* sentence, DateTime now) {
  char code[8], hours[8], mins[8], secs[8];
  sscanf(sentence, "%s %4s %4s %4s", &code, &secs, &mins, &hours);

  if (
     Cron::checkTime(secs, now.second()) && 
     Cron::checkTime(mins, now.minute()) && 
     Cron::checkTime(hours, now.hour())
  ) {
    return String(code);
  }

  return "0";
}

void Cron::tick() {
  DateTime now = rtc.now();
  int epoch = now.unixtime();

  if (epoch > lastEpoch) {
    _onTick(now);

    for (short i = 0; i < CRON_RATES_SIZE; i++) {
      const String result = Cron::isTime(CRON_RATES[i], now);
      if (result != "0") {
        _onAlarm(result);
      }
    }
    
    lastEpoch = epoch;
  }
}

DateTime Cron::getTime() {
  return rtc.now();
}

short Cron::getTemp() {
  return rtc.getTemperature();
}