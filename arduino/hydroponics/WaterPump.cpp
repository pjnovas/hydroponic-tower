#include "config.h"
#include "Arduino.h"
#include "WaterPump.h"

WaterPump::WaterPump() { 
  relayPin = PIN_RELAY_PUMP;
  floatSwitchPin = PIN_FLOAT_SWITCH;
  flowSensorPin = PIN_FLOW_SENSOR;

  pinMode(relayPin, OUTPUT);
  pinMode(floatSwitchPin, INPUT);
  pinMode(flowSensorPin, INPUT);

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

void WaterPump::readData() {
  if (checkWaterLevel()) {
    if (flowSensorPin && isOn()) {
      data.waterFlow = pulseIn(flowSensorPin, HIGH);
      // check if flow is not running > off(WaterPumpState::PUMP_ERR_FLOW);
    }
  }
}

bool WaterPump::on(bool ignoreFlowSensor) {
  if (checkWaterLevel()) {
    digitalWrite(relayPin, HIGH);
    data.pumpState = WaterPumpState::PUMP_ON;
    return true;
  }
}

void WaterPump::off(enum WaterPumpState newState) {
  data.pumpState = newState;
  digitalWrite(relayPin, LOW);
}
