/*
  Manages the enviroment temperature, humidity, sun light, etc
*/

#ifndef Enviroment_h
#define Enviroment_h

#include "Arduino.h"

struct EnviromentData {
  float temperature;
  unsigned int humidity;
  unsigned int light;
};

class Enviroment
{
  public:
    Enviroment();
    void readData();
    bool isDaylight();
    EnviromentData getData();

  private:
    unsigned int tempPin;
    unsigned int humPin;
    unsigned int photoPin;
    EnviromentData data;
};

#endif