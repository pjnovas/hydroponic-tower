/*
  Manages the enviroment temperature, humidity, sun light, etc
*/

#ifndef Enviroment_h
#define Enviroment_h

#include "Arduino.h"

struct EnviromentData {
  float temperature;
  byte humidity;
  byte light;
};

class Enviroment
{
  public:
    Enviroment();
    void readData();
    EnviromentData getData();

  private:
    EnviromentData data;
};

#endif
