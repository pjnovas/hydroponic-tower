#include "config.h"
#include "Arduino.h"
#include "Enviroment.h"
#include "amt1001_ino.h"

Enviroment::Enviroment() {  
  pinMode(PIN_ENV_TEMP, INPUT);
  pinMode(PIN_ENV_HUM, INPUT);
  pinMode(PIN_PHOTO_CELL, INPUT);
}

EnviromentData Enviroment::getData() {
  return data;
}

void Enviroment::readData() {
  if(PIN_ENV_TEMP) {
    data.temperature = amt1001_gettemperature(analogRead(PIN_ENV_TEMP));
    data.temperature -= (data.temperature * 5) / 25; // this is a patch fix for the voltage reference (5v == 25°c)
  }
  
  if(PIN_ENV_HUM) {
    data.humidity = amt1001_gethumidity(analogRead(PIN_ENV_HUM) * (VOLAGE_PSU / 1023.0));
  }

  if (PIN_PHOTO_CELL) {
    // light - 0 = dark
    data.light = map(analogRead(PIN_PHOTO_CELL), 0, 1023, 0, 100);
  }
}
