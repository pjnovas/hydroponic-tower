#include <Wire.h>
#include <CronAlarms.h>
#include "Sodaq_DS3231.h"

// DOCS: https://www.nongnu.org/avr-libc/user-manual/group__avr__time.html
#include <time.h>

char weekDay[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

void setup () {
  Serial.begin(57600);
  while (!Serial) ; // wait for Arduino Serial Monitor
  
  Serial.println("Starting setup...");
  
  Wire.begin();
  rtc.begin();

  DateTime now = rtc.now();

  set_zone(0);
  set_dst(0);
  set_system_time(now.getEpoch());

  Cron.create("*/15 * * * * *", on, false);
  Cron.create("*/30 * * * * *", temp, false);
}

void showTime() {
  DateTime now = rtc.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.date(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.print(' ');
  Serial.print(weekDay[now.dayOfWeek()]);
  Serial.println();
}

void temp() {
  showTime();
  rtc.convertTemperature();
  Serial.print(rtc.getTemperature());
  Serial.println("°");
}

void off() {
  showTime();
  Serial.println("OFF");
}

void on() {
  showTime();
  Serial.println("ON");
  Cron.create("*/5 * * * * *", off, true);
}

void loop () {
  system_tick();
  Cron.delay(1000);
}
