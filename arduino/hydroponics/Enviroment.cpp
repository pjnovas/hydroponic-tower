#include "config.h"
#include "Arduino.h"
#include "Enviroment.h"
#include "amt1001_ino.h"
#include <ArduinoJson.h>

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

DynamicJsonDocument Enviroment::toJSON() {
  // More info at https://arduinojson.org/v6/assistant/

  const size_t capacity = JSON_OBJECT_SIZE(3);
  DynamicJsonDocument doc(capacity);
  
  doc["temperature"] = data.temperature;
  doc["humidity"] = data.humidity;
  doc["light"] = data.light;

  return doc;
}

void Enviroment::readData() {
  if(tempPin) {
    data.temperature = amt1001_gettemperature(analogRead(tempPin));
  }
  
  if(humPin) {
    double volt = (double) analogRead(humPin) * (5.0 / 1023.0);
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
