#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SerialCommands.h>

#define SERIAL_BAULRATE 9600
#define HOSTNAME "ESP-Tower-1"
#define RETRY_MQTT_CONNECT 5000

char* ssid;
char* password;

char* mqtt_server;
unsigned int mqtt_port;
char* clientId;

unsigned long lastTry;

WiFiClient espClient;
PubSubClient client(espClient);

char commands_buffer[64];
SerialCommands commands(&Serial, commands_buffer, sizeof(commands_buffer), "\r\n", ";");

// default handler
void cmd_unrecognized(SerialCommands* sender, const char* cmd)
{
  sender->GetSerial()->print("ERROR;unknown command;");
  sender->GetSerial()->println(cmd);
}

// Restart Module
void cmd_reset(SerialCommands* sender)
{
  ESP.restart();
}

// publish message
void cmd_publish(SerialCommands* sender)
{
  if (!isMQTTReady()) {
    sender->GetSerial()->println("ERROR;PUB;BROKER OFFLINE");
    return;
  }
  
  char* topic = sender->Next();
  if (topic == NULL)
  {
    sender->GetSerial()->println("ERROR;PUB;TOPIC");
    return;
  }
  
  char* value = sender->Next();
  if (value == NULL)
  {
    sender->GetSerial()->println("ERROR;PUB;VALUE");
    return;
  }

  String topic_srt = String(clientId) + "/" + String(topic);
  const char* topic_ = topic_srt.c_str();
  
  if(client.publish(topic_, value, true)) {
    sender->GetSerial()->println("PUBOK");
  } else {
    sender->GetSerial()->println("ERROR:PUB;FAILED");
  }
}

// set configs
void cmd_set(SerialCommands* sender)
{
  char* type = sender->Next();
  
  if (strcmp(type, "WIFI") == 0) {
    ssid = sender->Next();
    password = sender->Next();

    WiFi.begin(ssid, password);
    connectWIFI();
    return;
  }

  if (strcmp(type, "MQTT") == 0) {
    mqtt_server = sender->Next();
    mqtt_port = atoi(sender->Next());
    clientId = sender->Next();

    client.setServer(mqtt_server, mqtt_port);
    connectMQTT();
    return;
  }

  sender->GetSerial()->println("ERROR;SET;TYPE NOT WIFI OR MQTT");
}

// SERIAL COMMANDS
SerialCommand cmd_reset_("RST", cmd_reset);
SerialCommand cmd_publish_("PUB", cmd_publish);
SerialCommand cmd_set_("SET", cmd_set);

void onMessageReceive(char* topic, byte* payload, unsigned int length) {

  // Serial.print("MSG RECEIVED TOPIC: ");
  // Serial.print(topic);
  // Serial.print(" | VALUE: ");
  // Serial.println((char)payload[0]);
  
  if (strcmp(topic, (String(clientId) + "/water/pump/power").c_str()) == 0) {    
    switch((char)payload[0]){
      case '0': // OFF
        Serial.println("PUMP;OFF");
        break;
      case '1': // ON
        Serial.println("PUMP;ON");
        break;
    }
  }

  if (strcmp(topic, (String(clientId) + "/box/reset").c_str()) == 0) {
    if ((char)payload[0] == '1') {
       // publish a 0 so it won't come back again
      if (client.publish((String(clientId) + "/box/reset").c_str(), "0", true)) {
        Serial.println("RESET"); // send arduino a reset command
      }
    }
  }
}

bool isWifiReady() {
  return WiFi.status() == WL_CONNECTED;
}

bool isMQTTReady() {
  return client.connected();
}

bool checkWifi() {
  if (isWifiReady()) return true;

  if (WiFi.status() == WL_CONNECTION_LOST) {
    Serial.print("WIFI;OFF;");
    Serial.println(WiFi.status());
  }
  
  return false;
}

void connectWIFI() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("WIFI;TRY");

    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.print("WIFI;OFF;");
      Serial.println(WiFi.status());
      return;
    }
  
    delay(1000);
  }

  Serial.println("WIFI;ON");
}

void connectMQTT() {
  if (millis() > lastTry + RETRY_MQTT_CONNECT) {
    lastTry = millis();
    
    if (isWifiReady() && mqtt_server) {
      
      if (isMQTTReady()) Serial.println("MQTT;ON");
      else {
        String willTopic = String(clientId) + "/status";
        const char* willTopic_ = willTopic.c_str();
        
        if (client.connect(clientId, NULL, NULL, willTopic_, 0, 1, "OFFLINE", true)) {
          Serial.println("MQTT;ON");
          client.publish(willTopic_, "ONLINE", true);

          client.subscribe((String(clientId) + "/water/pump/power").c_str());
          client.subscribe((String(clientId) + "/box/reset").c_str());
        }
        else {
          Serial.print("MQTT;TRY;");
          Serial.println(client.state());
        }
      }
    }
  }
}

void setup() {
  WiFi.mode(WIFI_STA);
  WiFi.hostname(HOSTNAME);
  Serial.begin(SERIAL_BAULRATE);

  client.setCallback(onMessageReceive);

  commands.SetDefaultHandler(cmd_unrecognized);
  commands.AddCommand(&cmd_reset_);
  commands.AddCommand(&cmd_publish_);
  commands.AddCommand(&cmd_set_);

  Serial.println("READY"); // Used to know when the Module is ready
}

void loop() {
  commands.ReadSerial();

  if (checkWifi()) {
    client.loop();
    if (!isMQTTReady()) connectMQTT();
  }
}
