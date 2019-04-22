/*
  Manages the enviroment temperature, humidity, sun light, etc
*/

#ifndef Enviroment_h
#define Enviroment_h

#include "Arduino.h"

struct EnviromentData {
  int temperature;
  int humidity;
  int light;
};

class Enviroment
{
  public:
    Enviroment();
    void readData();
    bool isDaylight();
    EnviromentData getData();

  private:
    int tempPin;
    int humPin;
    int photoPin;
    EnviromentData data;
};

#endif
