#include "config.h"
#include "Arduino.h"
#include "LEDState.h"

LEDState::LEDState() {  
  pinMode(PIN_LED_FAULT, OUTPUT);
  pinMode(PIN_LED_STATE, OUTPUT);

  digitalWrite(PIN_LED_FAULT, LOW);
  digitalWrite(PIN_LED_STATE, LOW);

  state = DeviceState::DEVICE_IDLE;
}

DeviceState LEDState::getState() {
  return state;
}

void LEDState::setState(DeviceState newState) {
  state = newState;

  switch(state){
    case DeviceState::DEVICE_IDLE: 
      faultValue = LOW;
      stateValue = LOW;
      break;
    case DeviceState::DEVICE_STARTUP: 
      faultValue = HIGH;
      break;
    case DeviceState::DEVICE_SERIAL_READY: 
      faultValue = LOW;
      break;
    case DeviceState::DEVICE_WIFI_TRY: 
      stateValue = HIGH;
      stateInterval = slowBlink;
      break;
    case DeviceState::DEVICE_MQTT_TRY: 
      stateValue = HIGH;
      stateInterval = fastBlink;
      break;
    case DeviceState::DEVICE_WIFI_ON: 
    case DeviceState::DEVICE_MQTT_ON: 
      stateValue = LOW;
      stateInterval = 0; // blink off
      break;
    case DeviceState::DEVICE_WIFI_OFF:
    case DeviceState::DEVICE_MQTT_OFF: 
      faultValue = HIGH;
      faultInterval = 0; // blink off
      stateValue = LOW;
      break;
    case DeviceState::DEVICE_PUBLISH: 
      stateValue = HIGH;
      stateInterval = 0;
      break;
    case DeviceState::DEVICE_PUBLISH_ACK: 
      stateValue = LOW;
      stateInterval = 0;
      break;
    case DeviceState::DEVICE_ERROR: 
      faultValue = HIGH;
      faultInterval = fastBlink;
      break;
    case DeviceState::WATER_PUMP_ALARM_ON:
      faultValue = HIGH;
      faultInterval = slowBlink;
      break;
    case DeviceState::WATER_PUMP_ALARM_OFF:
      faultValue = LOW;
      faultInterval = 0; // blink off
      break;
    case DeviceState::DEVICE_ERROR_UNKONWN_CMD:
      // Do nothing
      break;
    default: 
      faultValue = HIGH;
      break;
  }
}

void LEDState::loop() {
  unsigned long curMillis = millis();
  
  if(faultInterval > 0 && curMillis - faultLastTime >= faultInterval) {
    faultLastTime = curMillis;
      
    if (faultValue == HIGH) faultValue = LOW;
    else faultValue = HIGH;
  }

  digitalWrite(PIN_LED_FAULT, faultValue);

  ////////////////
  
  if(stateInterval > 0 && curMillis - stateLastTime >= stateInterval) {
    stateLastTime = curMillis;
      
    if (stateValue == HIGH) stateValue = LOW;
    else stateValue = HIGH;
  }

  digitalWrite(PIN_LED_STATE, stateValue);
}
