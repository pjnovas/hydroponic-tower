/*
  Manages the water pump
*/

#ifndef WaterPump_h
#define WaterPump_h

#include "Arduino.h"

enum WaterPumpState {
  PUMP_IDLE       = 0,
  PUMP_ON         = 1,
  PUMP_ERR_LEVEL  = 2,
  PUMP_ERR_FLOW   = 3
};

struct WaterPumpData {
  WaterPumpState pumpState;
  unsigned long waterFlow;
};

class WaterPump
{
  public:
    WaterPump();
    bool on(bool ignoreFlowSensor = false);
    void off(enum WaterPumpState = WaterPumpState::PUMP_IDLE);
    bool checkWaterLevel();
    bool isOn();
    bool hasError();
    WaterPumpData getData();
    void readData();
    void loop();

  private:
    int relayPin;
    int floatSwitchPin;
    int flowSensorPin;
    WaterPumpData data;
};

#endif
