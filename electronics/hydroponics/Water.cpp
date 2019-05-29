#include "config.h"
#include "Arduino.h"
#include "Water.h"
#include <OneWire.h>
#include <DallasTemperature.h>

Water::Water():oneWire(PIN_WATER_TEMP_DATA), sensors(&oneWire) {  
  pinMode(PIN_WATER_EC, INPUT);
  pinMode(PIN_WATER_EC_POWER, OUTPUT);
  pinMode(PIN_WATER_TEMP_DATA, INPUT);

  digitalWrite(PIN_WATER_EC_POWER, LOW);

  sensors.begin();
}

WaterData Water::getData() {
  return data;
}

void Water::readData() {
  // Temperature
  sensors.requestTemperatures();
  data.temperature = sensors.getTempCByIndex(0);

  // Estimate resistance
  digitalWrite(PIN_WATER_EC_POWER, HIGH);
  float raw = analogRead(PIN_WATER_EC);
  raw = analogRead(PIN_WATER_EC); // This is not a mistake, First reading will be low beause if charged a capacitor
  digitalWrite(PIN_WATER_EC_POWER, LOW);
   
  // Convert to EC
  float Vdrop = (VOLAGE_PSU * raw) / 1024.0;
  float Rc = ( Vdrop * (EC_PPM_R1 + EC_PPM_Ra)) / ( VOLAGE_PSU - Vdrop );
  Rc = Rc - EC_PPM_Ra; //acounting for Digital Pin Resitance
  float EC = 1000 / ( Rc * EC_PPM_K );

  // Compensating For Temperaure
  data.ec = EC / (1+ EC_PPM_TempCoef * (data.temperature - 25.0)); // EC25
  data.tds = data.ec * (EC_PPM_Conversion * 1000); // PPM
}
