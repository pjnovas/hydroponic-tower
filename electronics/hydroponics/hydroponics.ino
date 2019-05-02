#include "config.h"

#include "WaterPump.h"
#include "Enviroment.h"
#include "Water.h"
#include "SimpleTimer.h"
#include <ArduinoJson.h>

WaterPump waterPump;
Enviroment enviroment;
Water water;
SimpleTimer timer;

unsigned int timer_EnvRead;
unsigned int timer_WaterRead;
unsigned int timer_WaterPumpRead;

//////////

// Interrupt function
void flow () {
  waterPump.flowFrequency();
}

void setupFlowSensor() {
  pinMode(PIN_FLOW_SENSOR, INPUT);
  digitalWrite(PIN_FLOW_SENSOR, HIGH);
  attachInterrupt(PIN_FLOW_INTERRUPT, flow, FALLING);
  sei(); // Enable interrupts
}

void startWaterPump() {
  if(waterPump.on()) {
    timer.setTimeout(RATE_WATER_PUMP_FLOW, []() {
      waterPump.off();
      timer.setTimeout(RATE_WATER_PUMP_START, startWaterPump);
    });
  }
  else {
    // Could not start, RETRY later
    timer.setTimeout(RATE_WATER_PUMP_START_RETRY, startWaterPump);
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Started");

  timer_EnvRead = timer.setInterval(RATE_ENV_READ, []() {
    enviroment.readData();
    serializeJson(enviroment.toJSON(), Serial);
    Serial.println("");
  });

/*
  timer_WaterRead = timer.setInterval(RATE_WATER_READ, []() {
    water.readData();
    serializeJson(water.toJSON(), Serial);
    Serial.println("");
  });
*/

  timer_WaterPumpRead = timer.setInterval(RATE_WATER_PUMP_READ, []() {
    serializeJson(waterPump.toJSON(), Serial);
    Serial.println("");
  });

  setupFlowSensor();
  startWaterPump();

  // if enviroment.isDaylight() ? setup waterPump : stop waterPump

  /*
    timer.setInterval(RATE_POST_SERVER, []() {
    waterPump.getData();
    enviroment.getData();
    });
  */

}


void loop() {
  timer.run();
  waterPump.loop();
}
