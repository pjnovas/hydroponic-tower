/*
 * Configuration File
 */

// SOFT SETTINGS /////////////////////////////////////

// #define HOSTNAME "Hydroponics"

#define WIFI_SSID "******"
#define WIFI_PASSWORD "******"

#define SERIAL_BAULRATE 115200 // communication with ESP-01

// IoT MQTT Settings
// hostname, token, etc

// Rates (milliseconds)
#define RATE_ENV_READ                 5000 // enviroment sensor refresh rates
#define RATE_WATER_READ               5000 // EC and water temp refresh rates
#define RATE_WATER_PUMP_READ          1000 // Water Flow Meter refresh rates
#define RATE_WATER_PUMP_START         3000 // time the pump gets started
#define RATE_WATER_PUMP_START_RETRY   1000 // retry start time
#define RATE_WATER_PUMP_FLOW          6000 // time the pump is on

// WaterFlow Pulse frequency (Hz): is flow rate in L/min.
#define WATER_FLOW_PULSE_HZ   1

#define DARK_THRESHOLD        30 // below this value is considered night (0 to 100)
#define LOW_WATER_LEVEL_STATE HIGH // from the float switch to stop pump

// EC PPM Measurement Sensor (check ./calibration)
#define EC_PPM_R1             500 // (Ohms)
#define EC_PPM_Ra             25 // (Ohms)
#define EC_PPM_Conversion     0.7
#define EC_PPM_TempCoef       0.019
#define EC_PPM_K              2.7

// HARD SETTINGS /////////////////////////////////////

/////////////////////////////////////
// Digital Pins /////////////////////

#define PIN_RELAY_PUMP        4  // Any relay module for the Water Pump
#define PIN_FLOAT_SWITCH      3  // Any buoy float switch
#define PIN_FLOW_SENSOR       2  // YF-S201 Flow Meter (must support interrupts)
#define PIN_FLOW_INTERRUPT    0  // Interrupt of the above pin, ie. 0 = digital pin 2

#define PIN_WATER_TEMP_DATA   10 // DS18B20 Encapsulated Data Pin

/////////////////////////////////////
// Analog Pins //////////////////////

// Electric Conductance
#define PIN_WATER_EC          A0 // EC Pin INPUT from 110v
#define PIN_WATER_EC_POWER    A1 // EC Pin Power from 110v

#define PIN_PHOTO_CELL        A5 

// AMT1001 module
#define PIN_ENV_HUM           A6 // YELLOW
#define PIN_ENV_TEMP          A7 // WHITE > 10k resistor to GND
