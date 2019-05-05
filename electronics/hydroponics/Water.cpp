#include "config.h"
#include "Arduino.h"
#include "Water.h"
#include <OneWire.h>
#include <DallasTemperature.h>

Water::Water():oneWire(PIN_WATER_TEMP_DATA), sensors(&oneWire) {  
  ECPin = PIN_WATER_EC;
  ECPowerPin = PIN_WATER_EC_POWER;
  tempPin = PIN_WATER_TEMP_DATA;
  
  pinMode(ECPin, INPUT);
  pinMode(ECPowerPin, OUTPUT);
  pinMode(tempPin, INPUT);

  digitalWrite(ECPowerPin, LOW);

  R1 = EC_PPM_Ra;
  Ra = EC_PPM_R1;
  PPMconversion = EC_PPM_Conversion;
  TemperatureCoef = EC_PPM_TempCoef;
  K = EC_PPM_K;
  
  Vin = 5;
  R1 = (R1 + Ra);

  sensors.begin();
}

WaterData Water::getData() {
  return data;
}

void Water::readData() {
  // Temperature
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);

  // Estimate resistance
  digitalWrite(ECPowerPin, HIGH);
  float raw = analogRead(ECPin);
  raw = analogRead(ECPin);// This is not a mistake, First reading will be low beause if charged a capacitor
  digitalWrite(ECPowerPin, LOW);
   
  // Convert to EC
  float Vdrop = (Vin * raw) / 1024.0;
  float Rc = ( Vdrop * R1) / ( Vin - Vdrop );
  Rc = Rc - Ra; //acounting for Digital Pin Resitance
  float EC = 1000 / ( Rc * K );

  // Compensating For Temperaure
  float EC25 = EC / (1+ TemperatureCoef * (temp - 25.0));
  float ppm = EC25 * (PPMconversion * 1000);

  // Set data
  data.temperature = temp;
  data.ec = EC25;
  data.tds = ppm;
}
