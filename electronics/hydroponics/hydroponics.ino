#include "config.h"

#include "WaterPump.h"
#include "Enviroment.h"
#include "Water.h"
#include "LEDState.h"
#include "SimpleTimer.h"
#include <SerialCommands.h>
#include <SoftwareSerial.h>

WaterPump waterPump;
Enviroment enviroment;
Water water;
SimpleTimer timer;
LEDState deviceState;

SoftwareSerial espSerial(PIN_DEBUG_SERIAL_RX, PIN_DEBUG_SERIAL_TX);

// auto& espSerial = Serial; // TODO: hardware serial should be use when is no Debug mode

#ifdef DEBUG
auto& debugSerial = Serial;
#endif

char commands_buffer[128];
SerialCommands commands(&espSerial, commands_buffer, sizeof(commands_buffer), "\r\n", ";");

bool espReady = false;
unsigned int timer_EnvRead;
unsigned int timer_WaterRead;
unsigned int timer_WaterPumpRead;

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
    timer.setTimeout(5000, connectWifi); // Retry WIFI in 5 seconds
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

// SERIAL COMMANDS
SerialCommand cmd_wifi_("WIFI", cmd_wifi);
SerialCommand cmd_mqtt_("MQTT", cmd_mqtt);
SerialCommand cmd_pub_ack_("PUBOK", cmd_pub_ack);
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

void startWaterPump() {
  if(waterPump.on()) {
    timer.setTimeout(RATE_WATER_PUMP_FLOW, []() {
      waterPump.off();
      publishWaterPumpState();
      timer.setTimeout(RATE_WATER_PUMP_START, startWaterPump);
    });
    
    deviceState.setState(DeviceState::WATER_PUMP_ALARM_OFF);
  }
  else {
    deviceState.setState(DeviceState::WATER_PUMP_ALARM_ON);
    
    // Could not start, RETRY later
    timer.setTimeout(RATE_WATER_PUMP_START_RETRY, startWaterPump);
  }

  publishWaterPumpState();
}

void setUpReadTimers() {
  timer_EnvRead = timer.setInterval(RATE_ENV_READ, []() {
    enviroment.readData();

    timer.setTimeout(1000, []() {
      if (espReady) {
        deviceState.setState(DeviceState::DEVICE_PUBLISH);
        EnviromentData eData = enviroment.getData();
        espSerial.print("PUB;env/temp;");
        espSerial.println(eData.temperature);
      }
    });
  
    timer.setTimeout(2000, []() {
      if (espReady) {
        deviceState.setState(DeviceState::DEVICE_PUBLISH);
        EnviromentData eData = enviroment.getData();
        espSerial.print("PUB;env/hum;");
        espSerial.println(eData.humidity);
      }
    });
  
    timer.setTimeout(3000, []() {
      if (espReady) {
        deviceState.setState(DeviceState::DEVICE_PUBLISH);
        EnviromentData eData = enviroment.getData();
        espSerial.print("PUB;env/light;");
        espSerial.println(eData.light);
      }
    });
  });

  timer_WaterRead = timer.setInterval(RATE_WATER_READ, []() {
    water.readData();

    timer.setTimeout(1000, []() {
      if (espReady) {
        deviceState.setState(DeviceState::DEVICE_PUBLISH);
        WaterData wData = water.getData();
        espSerial.print("PUB;water/temp;");
        espSerial.println(wData.temperature);
      }
    });
    
    timer.setTimeout(2000, []() {
      if (espReady) {
        deviceState.setState(DeviceState::DEVICE_PUBLISH);
        WaterData wData = water.getData();
        espSerial.print("PUB;water/ec;");
        espSerial.println(wData.ec);
      }
    });

    timer.setTimeout(3000, []() {
      if (espReady) {
        deviceState.setState(DeviceState::DEVICE_PUBLISH);
        WaterData wData = water.getData();
        espSerial.print("PUB;water/tds;");
      espSerial.println(wData.tds);
      }
    });
  });

  timer_WaterPumpRead = timer.setInterval(RATE_WATER_PUMP_READ, []() {
    if (espReady) {
      WaterPumpData wpData = waterPump.getData();
        
      if (wpData.pumpState == WaterPumpState::PUMP_ON) {
        deviceState.setState(DeviceState::DEVICE_PUBLISH);
        
        espSerial.print("PUB;water/pump/flow;");
        espSerial.println(wpData.waterFlow);
      }
    }
  });

  setupFlowSensor();
  startWaterPump();
}

void setup() {
#ifdef DEBUG
  debugSerial.begin(9600);
  while (!debugSerial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  debugSerial.println("Connecting to ESP Serial");

  commands.SetDefaultHandler(cmd_unrecognized);
#endif
  
  commands.AddCommand(&cmd_wifi_);
  commands.AddCommand(&cmd_mqtt_);
  commands.AddCommand(&cmd_pub_ack_);
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
  
  setUpReadTimers();

  delay(1000);
  espSerial.flush();
  connectWifi();
}

void loop() {
  commands.ReadSerial();
  timer.run();
  waterPump.loop();
  deviceState.loop();
}
