/*
  Manages the Water temperature, EC, PPM, etc
*/

#ifndef Water_h
#define Water_h

#include "Arduino.h"
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>

struct WaterData {
  float temperature;
  float ec;
  float tds;
};

class Water
{
  public:
    Water();
    void readData();
    WaterData getData();

  private:    
    WaterData data;
    OneWire oneWire;
    DallasTemperature sensors;
};

#endif
