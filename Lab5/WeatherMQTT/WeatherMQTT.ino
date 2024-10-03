#include "Arduino.h"
#include <WiFi.h>
#include "ESP32MQTTClient.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 16
#define DHTTYPE DHT11

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

const char *ssid = "Galaxy A53 5G 1EBE";
const char *pass = "jxjw7723";

// Test Mosquitto server, see: https://test.mosquitto.org
char *server = "mqtt://192.168.137.224:1883";

char *subscribeTopic = "receiving/esp";
char *topic1 = "weather/temp",
     *topic2 = "weather/humidity";

ESP32MQTTClient mqttClient;  // all params are set later

void setup() {
  Serial.begin(115200);
  log_i();
  log_i("setup, ESP.getSdkVersion(): ");
  log_i("%s", ESP.getSdkVersion());
  dht.begin();

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

void loop() {
  sensors_event_t event;
  float temp,
    humidity;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  } else {
    temp = event.temperature;
  }

  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  } else {
    humidity = event.relative_humidity;
  }
  mqttClient.publish(topic1, "Temp: " + String(temp) + " °C", 0, false);
  mqttClient.publish(topic2, "Humidity: " + String(humidity) + " %", 0, false);
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