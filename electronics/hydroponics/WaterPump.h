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
  long waterFlow; // litres/hour
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
    void loop();
    void flowFrequency();

  private:
    int relayPin;
    int floatSwitchPin;
    WaterPumpData data;

    volatile int flow_frequency;
    unsigned long cloopTime;
};

#endif
