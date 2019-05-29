/*
  Manage LED State Lighting - to see how is the device by a BLKing LED
*/

#ifndef LEDState_h
#define LEDState_h

#include "Arduino.h"

/* STATUS CODES
  0   - [OFF FAULT - OFF STATUS] No status
  100 - [ON FAULT] Initialization: FAULT ON and Status OFF, if it breaks it will stuck in FAULT ON

  20x - [ON  STATUS] READY and SUCCESS states, everything is fine 
  30x - [BLK STATUS] WORKING states, trying something, ie connecting
  40x - [ON  FAULT] ERROR states, something went wrong, mainly to show fatal errors

  x = specification of the status (ie change velocity of the BLK)
*/

enum DeviceState {
  DEVICE_IDLE             = 0,   // OFF FAULT - OFF STATUS
  
  DEVICE_STARTUP          = 100, // ON FAULT
  DEVICE_SERIAL_READY     = 200, // OFF FAULT
  
  DEVICE_WIFI_TRY         = 301, // BLK STATUS - SLOW
  DEVICE_WIFI_ON          = 201, // OFF FAULT - ON STATUS
  DEVICE_WIFI_OFF         = 401, // ON FAULT  - OFF STATUS
  
  DEVICE_MQTT_TRY         = 302, // BLK STATUS - FAST
  DEVICE_MQTT_ON          = 202, // OFF FAULT - ON STATUS
  DEVICE_MQTT_OFF         = 402, // ON FAULT  - OFF STATUS

  WATER_PUMP_ALARM_OFF    = 203, // OFF FAULT
  WATER_PUMP_ALARM_ON     = 403, // BLK FAULT - not enough water - slow
  
  DEVICE_PUBLISH          = 305, // BLK STATUS - LOW
  DEVICE_PUBLISH_ACK      = 205, // BLK STATUS - HIGH
  DEVICE_ERROR            = 405, // BLK FAULT - pub failed, general error - fast
  DEVICE_ERROR_UNKONWN_CMD= 500  // DOES NOTHING
};

class LEDState
{
  public:
    LEDState();
    void setState(DeviceState newState);
    DeviceState getState();
    void loop();

  private:
    DeviceState state;
    
    byte faultValue = 0;
    unsigned short faultInterval = 0;
    unsigned long faultLastTime = 0;
    
    byte stateValue = 0;
    unsigned short stateInterval = 0;
    unsigned long stateLastTime = 0;
    
    const short slowBlink = 500;
    const short fastBlink = 200;
};

#endif
