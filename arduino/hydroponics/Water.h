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
    DynamicJsonDocument toJSON();

  private:
    unsigned int ECPin;
    unsigned int ECPowerPin;
    unsigned int tempPin;
    
    float Vin;
    unsigned int R1;
    unsigned int Ra;
    float PPMconversion;
    float TemperatureCoef;
    float K;
    
    WaterData data;
    OneWire oneWire;
    DallasTemperature sensors;
};

#endif
