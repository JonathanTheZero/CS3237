#include "Arduino.h"
#include <WiFi.h>
#include "ESP32MQTTClient.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 2     // Digital pin connected to the DHT sensor 
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment the type of sensor in use:
//#define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

const char *ssid = "Galaxy A53 5G 1EBE";
const char *pass = "jxjw7723";

// Test Mosquitto server, see: https://test.mosquitto.org
char *server = "mqtt://192.168.137.224:1883";

char *subscribeTopic = "receiving/esp";
char *publishTopic = "hello/esp";

ESP32MQTTClient mqttClient;  // all params are set later

void setup() {
  Serial.begin(115200);
  log_i();
  log_i("setup, ESP.getSdkVersion(): ");
  log_i("%s", ESP.getSdkVersion());

  mqttClient.enableDebuggingMessages();

  mqttClient.setURI(server);
  mqttClient.enableLastWillMessage("lwt", "I am going offline");
  mqttClient.setKeepAlive(30);
  WiFi.begin(ssid, pass);
  WiFi.setHostname("c3test");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  mqttClient.loopStart();
}

int pubCount = 0;

void loop() {
  String msg = "Hello: " + String(pubCount++);
  mqttClient.publish(publishTopic, msg, 0, false);
  delay(2000);
}

void onMqttConnect(esp_mqtt_client_handle_t client) {
  if (mqttClient.isMyTurn(client))  // can be omitted if only one client
  {
    Serial.println("Connected to the MQTT broker and subscribing to topics...");

    mqttClient.subscribe(subscribeTopic, [](const String &payload) {
      Serial.println(String(subscribeTopic) + String(" ") + String(payload.c_str()));
    });

    mqttClient.subscribe("bar/#", [](const String &topic, const String &payload) {
      Serial.println(String(topic) + String(" ") + String(payload.c_str()));
    });
  }
}

void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
  mqttClient.onEventCallback(event);
}