#include "config.h"
#include "Arduino.h"
#include "WaterPump.h"

WaterPump::WaterPump() { 
  relayPin = PIN_RELAY_PUMP;
  floatSwitchPin = PIN_FLOAT_SWITCH;

  pinMode(relayPin, OUTPUT);
  pinMode(floatSwitchPin, INPUT);

  flow_frequency = 0;
  cloopTime = millis();
  
  off();
}

WaterPumpData WaterPump::getData() {
  return data;
}

bool WaterPump::checkWaterLevel() {
  if (!floatSwitchPin) {
    return true;
  }

  if(digitalRead(floatSwitchPin) == LOW_WATER_LEVEL_STATE) {
    off(WaterPumpState::PUMP_ERR_LEVEL);
    return false;
  }

  return true;
}

bool WaterPump::isOn() {
  return data.pumpState == WaterPumpState::PUMP_ON;
}

bool WaterPump::hasError() {
  return 
    data.pumpState == WaterPumpState::PUMP_ERR_FLOW 
    || data.pumpState == WaterPumpState::PUMP_ERR_LEVEL;
}

void WaterPump::flowFrequency() {
  flow_frequency++;
}

void WaterPump::loop() {
  float currentTime = millis();
  
  // Every second, calculate flow
  if((currentTime - cloopTime) > 1000)
  {
    float flowRate = ((1000.0 / (currentTime - cloopTime)) * flow_frequency) / WATER_FLOW_PULSE_HZ;
    data.waterFlow = (flowRate / 60) * 1000;
    
    flow_frequency = 0; // Reset Counter
    cloopTime = currentTime;
  }
  
  if(isOn()){
    // TODO: check data.waterFlow === 0 and stop if is flow check is enabled
    checkWaterLevel();
  }
}

bool WaterPump::on(bool ignoreFlowSensor) {
  if (ignoreFlowSensor || checkWaterLevel()) {
    digitalWrite(relayPin, HIGH);
    data.pumpState = WaterPumpState::PUMP_ON;
    return true;
  }
  return false;
}

void WaterPump::off(enum WaterPumpState newState) {
  data.pumpState = newState;
  digitalWrite(relayPin, LOW);
}
