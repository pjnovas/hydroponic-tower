#include "config.h"
#include "Arduino.h"
#include "Enviroment.h"
#include "amt1001_ino.h"

Enviroment::Enviroment() {  
  tempPin = PIN_ENV_TEMP;
  humPin = PIN_ENV_HUM;
  photoPin = PIN_PHOTO_CELL;
  
  pinMode(tempPin, INPUT);
  pinMode(humPin, INPUT);
  pinMode(photoPin, INPUT);
}

EnviromentData Enviroment::getData() {
  return data;
}

void Enviroment::readData() {
  if(tempPin) {
    data.temperature = amt1001_gettemperature(analogRead(tempPin));
    data.temperature -= (data.temperature * 5) / 25; // this is a patch fix for the voltage reference (5v == 25°c)
  }
  
  if(humPin) {
    double volt = (double) analogRead(humPin) * (VOLAGE_PSU / 1023.0);
    data.humidity = amt1001_gethumidity(volt);
  }

  if (photoPin) {
    // light - 0 = dark
    data.light = map(analogRead(photoPin), 0, 1023, 0, 100);
  }
}

bool Enviroment::isDaylight() {
  // TODO: should use a clock?
  return data.light < DARK_THRESHOLD ? true : false;
}
