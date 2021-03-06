// #include "avr/wdt.h";
#include "config.h"
#include "Cron.h"

#include "WaterPump.h"
#include "Enviroment.h"
#include "Water.h"
#include "LEDState.h"

#include <SerialCommands.h>
#include <SoftwareSerial.h>

WaterPump waterPump;
Enviroment enviroment;
Water water;
LEDState deviceState;
Cron cron;

SoftwareSerial espSerial(PIN_DEBUG_SERIAL_RX, PIN_DEBUG_SERIAL_TX);
// auto& espSerial = Serial; // TODO: hardware serial should be use when is no Debug mode

#ifdef DEBUG
auto& debugSerial = Serial;
#endif

char commands_buffer[64];
SerialCommands commands(&espSerial, commands_buffer, sizeof(commands_buffer), "\r\n", ";");

short delayPUB = 50;
bool espReady = false;
char retry_code = '\0';
unsigned long hearBeatRTC = 0;

//////////

void connectWifi() {
  deviceState.setState(DeviceState::DEVICE_WIFI_TRY);
#ifdef DEBUG
  debugSerial.println("SET;WIFI");
#endif
  
  espSerial.print("SET;WIFI;");
  espSerial.print(WIFI_SSID);
  espSerial.print(";");
  espSerial.println(WIFI_PASSWORD);
}

void connectMQTT() {
  deviceState.setState(DeviceState::DEVICE_MQTT_TRY);
#ifdef DEBUG
  debugSerial.println("SET;MQTT");
#endif
  
  espSerial.print("SET;MQTT;");
  espSerial.print(MQTT_HOST);
  espSerial.print(";");
  espSerial.print(MQTT_PORT);
  espSerial.print(";");
  espSerial.println(MQTT_TOWER_ID);
}

// default handler
void cmd_unrecognized(SerialCommands* sender, const char* cmd) {
  deviceState.setState(DeviceState::DEVICE_ERROR_UNKONWN_CMD);
#ifdef DEBUG
  debugSerial.println(cmd);
#endif
}

void cmd_wifi(SerialCommands* sender) {
  char* state = sender->Next();

#ifdef DEBUG
  debugSerial.print("WIFI:");
  debugSerial.println(state);
#endif
  
  if (strcmp(state, "ON") == 0) {
    deviceState.setState(DeviceState::DEVICE_WIFI_ON);

    connectMQTT();
    return;
  }

  espReady = false;
  
  if (strcmp(state, "TRY") == 0) {
    deviceState.setState(DeviceState::DEVICE_WIFI_TRY);
  }

  if (strcmp(state, "OFF") == 0) {
    deviceState.setState(DeviceState::DEVICE_WIFI_OFF);
    retry_code = 'I';
  }
}

void cmd_mqtt(SerialCommands* sender) {
  char* state = sender->Next();

#ifdef DEBUG
  debugSerial.print("MQTT:");
  debugSerial.println(state);
#endif
  
  if (strcmp(state, "ON") == 0) {
    deviceState.setState(DeviceState::DEVICE_MQTT_ON);
    espReady = true;
    startup();
    return;
  }

  espReady = false;

  if (strcmp(state, "TRY") == 0) {
    deviceState.setState(DeviceState::DEVICE_MQTT_TRY);

    char* reason = sender->Next();

    // There is an error with the lib. Wifi must be restored
    // https://github.com/knolleary/pubsubclient/issues/552
    if (strcmp(reason, "-2") == 0) {
      connectWifi();
    }
  }

  if (strcmp(state, "OFF") == 0) {
    deviceState.setState(DeviceState::DEVICE_MQTT_OFF);
    // ESP will retry by itself
  }
}

void cmd_error(SerialCommands* sender) {
  char* error = sender->Next();
  deviceState.setState(DeviceState::DEVICE_ERROR);
  // Publish into topic /errors (does it make sense?)
}

void cmd_pub_ack(SerialCommands* sender) {
  deviceState.setState(DeviceState::DEVICE_PUBLISH_ACK);
}

void cmd_pump(SerialCommands* sender) {
  char* state = sender->Next();

#ifdef DEBUG
  debugSerial.print("PUMP:");
  debugSerial.println(state);
#endif
  
  if (strcmp(state, "ON") == 0) {
    startWaterPump();
  }

  if (strcmp(state, "OFF") == 0) {
    stopWaterPump();
  }
}

void cmd_reset(SerialCommands* sender) {
#ifdef DEBUG
  debugSerial.print("RESET");
#endif

  digitalWrite(PIN_RESET, LOW);
  delay(3000); // hang for 3 seconds
}

// SERIAL COMMANDS
SerialCommand cmd_wifi_("WIFI", cmd_wifi);
SerialCommand cmd_mqtt_("MQTT", cmd_mqtt);
SerialCommand cmd_pub_ack_("PUBOK", cmd_pub_ack);
SerialCommand cmd_pump_("PUMP", cmd_pump);
SerialCommand cmd_reset_("RESET", cmd_reset);
SerialCommand cmd_error_("ERROR", cmd_error);

// Interrupt function
void flow () {
  waterPump.flowFrequency();
}

void setupFlowSensor() {
  pinMode(PIN_FLOW_SENSOR, INPUT);
  digitalWrite(PIN_FLOW_SENSOR, HIGH);
  attachInterrupt(PIN_FLOW_INTERRUPT, flow, FALLING);
  sei(); // Enable interrupts
}

void publishWaterPumpState() {
  if (espReady) {
    WaterPumpData wpData = waterPump.getData();
    deviceState.setState(DeviceState::DEVICE_PUBLISH);
    
    espSerial.print("PUB;water/pump/state;");
    espSerial.println(wpData.pumpState);
  }
}

void readBox() {
  if (espReady) {
    deviceState.setState(DeviceState::DEVICE_PUBLISH);
    espSerial.print("PUB;box/temp;");
    espSerial.println(cron.getTemp());
    
    delay(delayPUB);
    deviceState.setState(DeviceState::DEVICE_PUBLISH);
    espSerial.print("PUB;box/time;");
    espSerial.println(cron.getTime().unixtime());
  }
}

void readEnviroment() {
  enviroment.readData();
  
  if (espReady) {
    EnviromentData eData = enviroment.getData();
    
    deviceState.setState(DeviceState::DEVICE_PUBLISH);
    espSerial.print("PUB;env/temp;");
    espSerial.println(eData.temperature);

    delay(delayPUB);
    deviceState.setState(DeviceState::DEVICE_PUBLISH);
    espSerial.print("PUB;env/hum;");
    espSerial.println(eData.humidity);
  
    delay(delayPUB);
    deviceState.setState(DeviceState::DEVICE_PUBLISH);
    espSerial.print("PUB;env/light;");
    espSerial.println(eData.light);
  }
}

void readWater() {
  water.readData();

  if (espReady) {
    WaterData wData = water.getData();

    if (wData.temperature < 10) {
      // Failed reading
      retry_code = 'W'; // WATR_RD;
      return;
    }

    deviceState.setState(DeviceState::DEVICE_PUBLISH);
    espSerial.print("PUB;water/temp;");
    espSerial.println(wData.temperature);

    delay(delayPUB);
    deviceState.setState(DeviceState::DEVICE_PUBLISH);
    espSerial.print("PUB;water/ec;");
    espSerial.println(wData.ec);

    delay(delayPUB);
    deviceState.setState(DeviceState::DEVICE_PUBLISH);
    espSerial.print("PUB;water/tds;");
    espSerial.println(wData.tds);
  }
}

void readPump() {
  if (espReady) {
    WaterPumpData wpData2 = waterPump.getData();
    
    // if (wpData2.pumpState == WaterPumpState::PUMP_ON) {
    deviceState.setState(DeviceState::DEVICE_PUBLISH);
    espSerial.print("PUB;water/pump/flow;");
    espSerial.println(wpData2.waterFlow);
    // }
  }
}

void startWaterPump() {
  if(waterPump.on()) {
    deviceState.setState(DeviceState::WATER_PUMP_ALARM_OFF);
  }
  else {
    deviceState.setState(DeviceState::WATER_PUMP_ALARM_ON);
    retry_code = "PUMP_ON";
  }

  publishWaterPumpState();
}

void stopWaterPump() {
  waterPump.off();
  deviceState.setState(DeviceState::WATER_PUMP_ALARM_OFF);
  publishWaterPumpState(); 
}

void onAlarm(const char code) {
#ifdef DEBUG
  Serial.println(code);
#endif

  if(code == 'B') readBox(); // BOX_RD
  if(code == 'E') readEnviroment(); // ENV_RD
  if(code == 'W') readWater(); // WATR_RD
  if(code == 'F') readPump(); // WART_FW
  if(code == 'U') startWaterPump(); // PUMP_ON = UP
  if(code == 'D') stopWaterPump(); // PUMP_OF = DOWN
  if(code == 'I') connectWifi(); // WIFI

  if(code == 'R') { // RETRY
    if (retry_code != '\0') {
      onAlarm(retry_code);
      retry_code = '\0';
    }
  }
}

// Called every second
void onTick(const DateTime now) {
  // wdt_reset();
  hearBeatRTC = millis();

#ifdef DEBUG
  Serial.print("TIME: ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.println(now.second(), DEC);
#endif
}

void startup() {
  // when connected to MQTT and started
  espSerial.println("PUB;box/clock;1"); // set clock on

  readBox();
  delay(delayPUB);
  publishWaterPumpState();
  delay(delayPUB);
  readPump();

  // wdt_enable(WDTO_4S);
}

void checkResetRTC() {
  if (millis() > hearBeatRTC + 3000) {
    // 3 seconds has passed from last call
    // something is wrong > power off RTC
    
    espSerial.println("PUB;box/clock;0");
    digitalWrite(PIN_POWER_RTC, LOW);

    delay(3000); // hang for 3 seconds and reset
    digitalWrite(PIN_RESET, LOW);
  } 
}

void setup() {
  digitalWrite(PIN_RESET, HIGH);
  pinMode(PIN_RESET, OUTPUT);
  
  // wdt_disable(); // start disabled until startup

#ifdef DEBUG
  debugSerial.begin(SERIAL_DEBUG_BAULRATE);
  while (!debugSerial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  debugSerial.println("Connecting to ESP Serial");

  commands.SetDefaultHandler(cmd_unrecognized);
#endif

  // Power ON RTC
  pinMode(PIN_POWER_RTC, OUTPUT);
  digitalWrite(PIN_POWER_RTC, HIGH);
  
  commands.AddCommand(&cmd_wifi_);
  commands.AddCommand(&cmd_mqtt_);
  commands.AddCommand(&cmd_pub_ack_);
  commands.AddCommand(&cmd_pump_);
  commands.AddCommand(&cmd_reset_);
  commands.AddCommand(&cmd_error_);

  espSerial.begin(SERIAL_BAULRATE);
  espSerial.println("RST");
  espSerial.flush();

  deviceState.setState(DeviceState::DEVICE_STARTUP);
  deviceState.loop();
  
  delay(1000);
  
  if(!espSerial.find("READY")) {
#ifdef DEBUG
    debugSerial.println("Module has no response");
#endif

    while(1); // TODO: this is a never start loop
  }

  deviceState.setState(DeviceState::DEVICE_SERIAL_READY);
  deviceState.loop();

  setupFlowSensor();
  cron.begin(onAlarm, onTick);

  espSerial.flush();
  connectWifi();
}

void loop() {
  cron.tick(millis());
  
  commands.ReadSerial();
  
  waterPump.loop();
  deviceState.loop();

  checkResetRTC();
}
